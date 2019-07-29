/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "WDSModemEndPoint.h"
#include "sync/GetAttachListMessage.h"
#include "sync/SetAttachListSyncMessage.h"
#include <sstream>

using std::to_string;
constexpr const char *WDSModemEndPoint::NAME;

void WDSModemEndPoint::requestSetup(string clientToken,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, callback);
  mModule->dispatch(shared_setupMsg);
}

Message::Callback::Status WDSModemEndPoint::getAttachListSync
(
  std::shared_ptr<std::list<uint16_t>>& attach_list
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync] "
      "dispatching message GetAttachListMessage");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetAttachListMessage>(nullptr);
    Message::Callback::Status apiStatus;
    apiStatus = msg->dispatchSync(attach_list);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync]"
      " Failed to send message GetAttachListMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::setAttachListSync
(
  const std::shared_ptr<std::list<uint16_t>>& attach_list,
  const SetAttachListSyncMessage::AttachListAction action
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "dispatching message SetAttachListSyncMessage");
    // Note that callback is not required for 'dispatchSync' calls.
    auto msg =
        std::make_shared<SetAttachListSyncMessage>(nullptr);
    if(attach_list)
    {
      msg->setParams(*attach_list, action);
    } else {
      Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "attach_list is NULL.Returning FAILURE");
      return Message::Callback::Status::FAILURE;
    }

    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::setAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "Failed to send message SetAttachListSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::getAttachListCapabilitySync
(
  std::shared_ptr<AttachListCap>& cap
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      " dispatching message GetAttachListCapabilitySyncMessage");

    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<GetAttachListCapabilitySyncMessage>(nullptr);
    Message::Callback::Status apiStatus;
    apiStatus = msg->dispatchSync(cap);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListCapabilitySync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;

  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      "Failed to send message GetAttachListCapabilitySyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

