/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "EventsDefinitions.h"
#include <string>
#include <json/json.h>

/********* Interface for composite types to be used as part of event payload *********/
void JsonSerializable::ToJson(Json::Value& root, const char* keyName) const
{
    Json::Value local;
    ToJsonImpl(local);
    root[keyName] = local;
}

/***************** Base class for all events to be pushed to DMTools *****************/
HostManagerEventBase::HostManagerEventBase(const std::string& deviceName) :
    m_timestampLocal(Utils::GetCurrentLocalTimeString()),
    m_deviceName(deviceName)
{}

// Serialization entry point
std::string HostManagerEventBase::ToJson() const
{
    Json::Value root;
    root["$type"] = GetTypeName();
    root["TimestampLocal"] = m_timestampLocal;
    root["DeviceName"] = m_deviceName;

    ToJsonInternal(root);

    Json::FastWriter writer; // prefer over Json::StyledWriter since \r\n can be used for events separation
    return writer.write(root);
}

/*********************************** NewDeviceDiscoveredEvent *****************************************************/
void SerializableFwVersion::ToJsonImpl(Json::Value& valueToUpdate) const
{
    valueToUpdate["Major"] = static_cast<uint32_t>(m_major);
    valueToUpdate["Minor"] = static_cast<uint32_t>(m_minor);
    valueToUpdate["SubMinor"] = static_cast<uint32_t>(m_subMinor);
    valueToUpdate["Build"] = static_cast<uint32_t>(m_build);
}

void SerializableFwTimestamp::ToJsonImpl(Json::Value& valueToUpdate) const
{
    valueToUpdate["Hour"] = static_cast<uint32_t>(m_hour);
    valueToUpdate["Min"] = static_cast<uint32_t>(m_min);
    valueToUpdate["Sec"] = static_cast<uint32_t>(m_sec);
    valueToUpdate["Day"] = static_cast<uint32_t>(m_day);
    valueToUpdate["Month"] = static_cast<uint32_t>(m_month);
    valueToUpdate["Year"] = static_cast<uint32_t>(m_year);
}

NewDeviceDiscoveredEvent::NewDeviceDiscoveredEvent(const std::string& deviceName, const FwVersion& fwVersion, const FwTimestamp& fwTimestamp) :
    HostManagerEventBase(deviceName),
    m_fwVersion(fwVersion),
    m_fwTimestamp(fwTimestamp)
{}

void NewDeviceDiscoveredEvent::ToJsonInternal(Json::Value& root) const
{
    m_fwVersion.ToJson(root, "FwVersion");
    m_fwTimestamp.ToJson(root, "FwTimestamp");
}

/*********************************** ClientConnectedEvent *****************************************************/
ClientConnectedEvent::ClientConnectedEvent(const std::set<std::string> & newClientIPSet, const std::string & newClient) :
    HostManagerEventBase(""),
    m_ExistingClientIPSet(newClientIPSet),
    m_newClient(newClient)
{}

void ClientConnectedEvent::ToJsonInternal(Json::Value& root) const
{
    root["NewConnectedUser"] = m_newClient;
    Json::Value existingClientIps(Json::arrayValue);
    for (const auto& ip : m_ExistingClientIPSet)
    {
        existingClientIps.append(ip);
    }
    root["ExistingConnectedUsers"] = existingClientIps;
}

ClientDisconnectedEvent::ClientDisconnectedEvent(const std::set<std::string> & newClientIPSet, const std::string & newClient) :
    HostManagerEventBase(""),
    m_ExistingClientIPSet(newClientIPSet),
    m_newClient(newClient)
{}

void ClientDisconnectedEvent::ToJsonInternal(Json::Value& root) const
{
    root["DisconnectedUser"] = m_newClient;

    Json::Value existingClientIps(Json::arrayValue);
    for (const auto& ip : m_ExistingClientIPSet)
    {
        existingClientIps.append(ip);
    }
    root["ExistingConnectedUsers"] = existingClientIps;
}

/*********************************** Enumeration Status Changed *****************************************************/
IsEnumaratingChangedEvent::IsEnumaratingChangedEvent(bool IsEnumarating) :
    HostManagerEventBase(""),
    m_IsEnumarating(IsEnumarating)
{}

void IsEnumaratingChangedEvent::ToJsonInternal(Json::Value& root) const
{
    root["IsEnumarating"] = m_IsEnumarating;
}

/*********************************** Silence Status Changed *****************************************************/
SilenceStatusChangedEvent::SilenceStatusChangedEvent(const std::string& deviceName, bool isSilent) :
    HostManagerEventBase(deviceName),
    m_isSilent(isSilent)
{}

void SilenceStatusChangedEvent::ToJsonInternal(Json::Value& root) const
{
    root["IsSilenced"] = m_isSilent;
}

/************************************FW uCode status changed *****************************************************/
FwUcodeStatusChangedEvent::FwUcodeStatusChangedEvent(const std::string& deviceName, FwUcodeState fwUcodeState) :
    HostManagerEventBase(deviceName),
    m_fwUcodeState(fwUcodeState)
{}

void FwUcodeStatusChangedEvent::ToJsonInternal(Json::Value& root) const
{
    root["FwError"] = static_cast<uint32_t>(m_fwUcodeState.fwError);
    root["UcodeError"] = static_cast<uint32_t>(m_fwUcodeState.uCodeError);
    root["FwState"] = static_cast<uint32_t>(m_fwUcodeState.fwState);
}

//********************************** New Log Lines event **********************************************//
void SerializableLogLine::ToJsonImpl(Json::Value& valueToUpdate) const
{
    valueToUpdate["Module"] = m_module;
    valueToUpdate["Level"] = m_level;
    valueToUpdate["StringOffset"] = m_strOffset;
    valueToUpdate["Signature"] = m_signature;
    valueToUpdate["IsString"] = m_isString;
    valueToUpdate["NumberOfParameters"] = m_dwordArray.size();
    Json::Value parametersArray(Json::arrayValue);
    for (auto parameter : m_dwordArray)
    {
        parametersArray.append(parameter);
    }
    valueToUpdate["Parameters"] = parametersArray;
    valueToUpdate["MLLR"] = m_missingLogsReason;
    valueToUpdate["NOMLLS"] = m_numOfMissingLogDwords;
}

NewLogsEvent::NewLogsEvent(const std::string& deviceName, const CpuType& cpuType, const std::vector<log_collector::RawLogLine>& logLines) :
    HostManagerEventBase(deviceName),
    m_cpuType(cpuType)
{
    m_logLines.reserve(logLines.size());
    for (const auto& logLine : logLines)
    {
        m_logLines.emplace_back(new SerializableLogLine(logLine));
    }
}

void NewLogsEvent::ToJsonInternal(Json::Value& root) const
{
    root["TracerType"] = CPU_TYPE_TO_STRING[m_cpuType];

    Json::Value linesArray(Json::arrayValue);
    for (const auto & line : m_logLines)
    {
        Json::Value lineValue;
        line->ToJsonImpl(lineValue);
        linesArray.append(lineValue);
    }
    root["LogLines"] = linesArray;
}

//********************************** DriverEvent **********************************************//
DriverEvent::DriverEvent(const std::string& deviceName, int driverEventType,
                         unsigned driverEventId, unsigned listenId,
                         unsigned bufferLength, const unsigned char* binaryData) :
    HostManagerEventBase(deviceName),
    m_driverEventType(driverEventType),
    m_driverEventId(driverEventId),
    m_listenId(listenId),
    m_bufferLength(bufferLength),
    m_binaryData(binaryData)
{}

void DriverEvent::ToJsonInternal(Json::Value& root) const
{
    root["DriverEventType"] = m_driverEventType;
    root["DriverEventId"] = m_driverEventId;
    root["ListenId"] = m_listenId;
    root["BinaryDataBase64"] = Utils::Base64Encode(m_binaryData, m_bufferLength); // bufferLength = 0 if buffer is empty
}
