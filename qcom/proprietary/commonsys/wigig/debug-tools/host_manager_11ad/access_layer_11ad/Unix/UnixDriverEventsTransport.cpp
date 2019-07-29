/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "UnixDriverEventsTransport.h"
#include "EventsDefinitions.h"
#include "DebugLogger.h"
#include "Host.h"
#include "nl_services.h"

UnixDriverEventsTransport::UnixDriverEventsTransport() :
    m_nlState(nullptr),
    m_exitSockets{ { -1, -1 } }, // aggregate initialization
    m_stopRunningDriverControlEventsThread(false)
{}

UnixDriverEventsTransport::~UnixDriverEventsTransport()
{
    StopDriverControlEventsThread();
}

// Registration for Driver Events through the Operational Mailbox
// \returns false when Driver does not support this mode or in case of failure
bool UnixDriverEventsTransport::RegisterForDriverEvents(const std::string& shortInterfaceName, const std::string& fullInterfaceName)
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitSockets.data()) == -1)
    {
        LOG_ERROR << "Failed to create cancellation socket pair, abort registration for driver events...\n";
        return false;
    }

    nl_status res = nl_initialize(shortInterfaceName.c_str(), &m_nlState);
    if (!(res == NL_SUCCESS && m_nlState))
    {
        CloseSocketPair(); // release the exit sockets
        // display warning instead of error to prevent SI test failure
        LOG_WARNING << "Driver Events are not supported, [nl_status=" << res << "], reason: " << nl_convert_status_to_string(res) << endl;
        return false;
    }

    // Enable commands and events transport on the Driver side
    if (nl_enable_driver_commands_transport(m_nlState, true /*enable*/) != NL_SUCCESS)
    {
        // Do not halt the flow on error because of backward compatibility with previous Driver version before this feature
        LOG_WARNING << "Setup of commands and events transport is not supported on the Driver side. Upgrade to the new Driver version." << endl;
    }

    // Creating a thread to wait on the netlink blocking poll, pass the interface name
    // Note: move assignment operator is used to initialize the member thread
    m_waitOnSignalThread = thread(&UnixDriverEventsTransport::DriverControlEventsThread, this, fullInterfaceName);

    return true;
}

// Workaround, explicit request to enable commands and events transport on the Driver side
// TODO: remove when new Enumeration mechanism is implemented
bool UnixDriverEventsTransport::ReEnableDriverEventsTransport()
{
    if (!m_nlState)
    {   // Driver events are disabled, working with Debug Mailbox. Nothing to be done.
        return true;
    }

    if (nl_enable_driver_commands_transport(m_nlState, true /*enable*/) != NL_SUCCESS)
    {
        LOG_INFO << __FUNCTION__ << ": failed to re-enable Driver events, device will be invalidated." << endl;
        return false;
    }

    return true;
}

// Send event to the driver
bool UnixDriverEventsTransport::SendDriverCommand(uint32_t id, const void *pInBuf, uint32_t inBufSize, void *pOutBuf, uint32_t outBufSize, DWORD* pLastError)
{
    (void)pLastError;   // not in use, netlink mechanism does not update it

    LOG_ASSERT(pInBuf); // just in case, pInBuf validity should be tested by the caller

    if (pOutBuf && outBufSize != sizeof(uint32_t))
    {   // when response required, the only supported response size is 32 bit
        LOG_WARNING << __FUNCTION__ << ": given output buffer size " << outBufSize << " does not match expected " << sizeof(uint32_t) << std::endl;
        return false;
    }

    nl_status res = nl_send_driver_command(m_nlState, id, inBufSize, pInBuf, reinterpret_cast<uint32_t*>(pOutBuf));

    if (res != NL_SUCCESS)
    {
        LOG_ERROR << "Failed to send driver command, [nl_status=" << res << "],error: " << nl_convert_status_to_string(res) << std::endl;
        return false;
    }

    return true;
}

// Main loop of polling for incoming driver events
void UnixDriverEventsTransport::DriverControlEventsThread(const std::string& fullInterfaceName)
{
    struct driver_event_report driverEvent;

    while (!m_stopRunningDriverControlEventsThread)
    {
        memset(&driverEvent, 0, sizeof(driverEvent));

        // blocking call to fetch the next event
        nl_status res = nl_get_driver_event(m_nlState, m_exitSockets[1] /* cancellation socket file descriptor */, &driverEvent);

        if (m_stopRunningDriverControlEventsThread) // asked to quit
        {
            // Disable commands and events transport on the Driver side
            // Ignore the possible error because of backward compatibility with previous Driver version before this feature
            nl_enable_driver_commands_transport(m_nlState, false /*disable*/);
            break;
        }

        if (res != NL_SUCCESS)
        {
            LOG_ERROR << "Failed to get driver event [nl_status=" << res << "],error: " << nl_convert_status_to_string(res) << std::endl;
            continue;
        }

        if (driverEvent.buf_len > DRIVER_MSG_MAX_LEN)
        {
            // shouldn't happen
            LOG_ERROR << "Got invalid driver event, buffer length is " << driverEvent.buf_len << endl;
            continue;
        }

        Host::GetHost().PushEvent(DriverEvent(fullInterfaceName, static_cast<int>(driverEvent.evt_type),
            driverEvent.evt_id, driverEvent.listener_id, driverEvent.buf_len, driverEvent.buf));
    }

    LOG_INFO << "DriverControlEventsThread for device " << fullInterfaceName << " was asked to quit." << std::endl;
}

// Cancel blocking polling for incoming events to allow graceful thread shutdown
void UnixDriverEventsTransport::StopDriverControlEventsThread()
{
    // Stop driver control event thread:
    m_stopRunningDriverControlEventsThread = true;

    // unblocks the monitor receive socket for termination
    // sockets pair serves as a pipe - a value written to one of its sockets, is also written to the second one
    // actual value written has no importance
    if (m_exitSockets[0] >= 0)
    {
        write(m_exitSockets[0], "T", 1);
    }

    if (m_waitOnSignalThread.joinable())
    {
        m_waitOnSignalThread.join();
    }

    // release netlink state
    nl_release(m_nlState);

    // release sockets from the sockets pair used to stop the driver events thread
    CloseSocketPair();
}

// Release sockets from the sockets pair used to stop the driver events thread
void UnixDriverEventsTransport::CloseSocketPair()
{
    if (m_exitSockets[0] >= 0)
    {
        close(m_exitSockets[0]);
        m_exitSockets[0] = -1;
    }
    if (m_exitSockets[1] >= 0)
    {
        close(m_exitSockets[1]);
        m_exitSockets[1] = -1;
    }
}
