/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "framework/SingleDispatchRestrictionHandleDuplicate.h"
#include "framework/Dispatcher.h"
#include "framework/SolicitedMessage.h"
#include "framework/legacy.h"
#include "framework/Log.h"
#include "modules/android/qcril_android_event_context.h"

using std::lock_guard;
using std::mutex;

SingleDispatchRestrictionHandleDuplicate::SingleDispatchRestrictionHandleDuplicate() {
  mName = "SingleDispatchRestrictionHandleDuplicate";
  mHandlingMsg = nullptr;
  mRedispatchedMsg = nullptr;
}

SingleDispatchRestrictionHandleDuplicate::~SingleDispatchRestrictionHandleDuplicate() {}

/*
 * Below logic ensures that msg which are pending in a separate queue due to restriction
 * are processed exactly in the order they arrived. This algo addresses a particular
 * race-condition where pending msg is re-enqueued to main dispatcher queue just after
 * a new msg from telephony is enqueued and hence ends up with incorrect order.
 * Here is the flow:
 * - Forward the incoming msg to module if msg dispatch is not restricted. Set mHandlingMsg
 *   to this msg.
 * - Append the incoming msg to mRestrictedMessagesQueue if restriction is applicable (There
 *   is current handling msg or re-dispatched pending msg or queue is not empty)
 * - Upon msg's callback execution by respective module, pop the head of mRestrictedMessagesQueue
 *   and redispatch. While doing that save the refrence to this msg in mRedispatchedMsg.
 * - If the incoming msg is mRedispatchedMsg then set mHandlingMsg to mRedispatchedMsg and
 *   mRedispatchedMsg to nullptr. Now forward this msg to respective module.
 */
bool SingleDispatchRestrictionHandleDuplicate::isDispatchAllowed(
    std::shared_ptr<Message> msg) {
  lock_guard<mutex> lock(mMutex);

  string msgStr = msg->to_string();
  bool okToDispatch = false;

  if (msg == mRedispatchedMsg) {
    /* case 1: if the msg is the expected re-dispatched msg, handle it directly */
    assert(mHandlingMsg == nullptr);
    mHandlingMsg = msg;
    mRedispatchedMsg = nullptr;
    okToDispatch = true;
  } else if (!mHandlingMsg && !mRedispatchedMsg && !mRestrictedMessagesQueue.size()) {
    /* case 2: if nothing is under handling and nothing is in the waiting queue, and
     * no re-dispatched message is waiting execution, handle it immediately */
    mHandlingMsg = msg;
    okToDispatch = true;
  } else {
    /* Add this msg to the waiting list */
    mRestrictedMessagesQueue.push_back(msg);
    okToDispatch = false;
  }

  if (okToDispatch) {
    /* Message is restricted but no message is outstanding. Clear to go * ahead.*/
    Log::getInstance().d(
        "[" + mName + "]: Restrictions enabled for " +
        msg->to_string() + " but allowed to dispatch.");
    return true;
  } else {
    /* One message from the groupd is still inprogress and callback is not yet
     * executed. Wait.*/
    Log::getInstance().d("[" + mName + "]: Can not forward msg = " +
                         msg->to_string() + " until restriction is cleared");
    return false;
  }
}

void SingleDispatchRestrictionHandleDuplicate::onMessageCallbackCompletion(
    std::shared_ptr<Message> msg) {
  std::unique_lock<mutex> lock(mMutex);

  string msgStr = msg->to_string();
  Log::getInstance().d("[" + mName + "]: onMessageCallbackCompletion msg = " +
                       msg->dump());

  if (msg != mHandlingMsg) {
    Log::getInstance().d("[" + mName + "]: msg mismatch.. maybe duplicated message");
    return;
  }

  std::vector<std::shared_ptr<Message>> duplicatedMsg;
  /* go over the queue and find duplicated messages */
  for (auto it = mRestrictedMessagesQueue.begin();
            it != mRestrictedMessagesQueue.end();) {
    if (msg->isSameMessage(*it)) {
        duplicatedMsg.push_back(*it);
        it = mRestrictedMessagesQueue.erase(it);
    } else {
        it++;
    }
  }
  lock.unlock();

  auto solicitedMsg = std::static_pointer_cast<SolicitedMessage<generic_callback_payload>>(msg);
  /* Send the same response for duplicated messages
   * Currently it only supports soliciated message generated by RIL request */
  for (auto const& dup: duplicatedMsg) {
    if(dup != nullptr) {
      Log::getInstance().d("[" + mName + "]: send duplicated " + msg->dump());
      auto item = std::static_pointer_cast<SolicitedMessage<generic_callback_payload>>(dup);
      item->sendResponse(item, Message::Callback::Status::SUCCESS,
              item->duplicateResponseData(solicitedMsg->getResponseData()));
    }
  }

  std::shared_ptr<Message> redispatchMsg = nullptr;
  lock.lock();
  mHandlingMsg = nullptr;
  /* fetch the front msg in the queue and dispatch */
  if (!mRestrictedMessagesQueue.empty()) {
    mRedispatchedMsg = mRestrictedMessagesQueue.front();
    redispatchMsg = mRedispatchedMsg;
    mRestrictedMessagesQueue.pop_front();
    Log::getInstance().d("[" + mName + "]: Requeue msg = " +
                         mRedispatchedMsg->dump());
  }
  lock.unlock();

  if (redispatchMsg != nullptr) {
    Dispatcher::getInstance().dispatch(redispatchMsg);
  }
}

void SingleDispatchRestrictionHandleDuplicate::abandonPendingMessages() {
  lock_guard<mutex> lock(mMutex);
  Log::getInstance().d("[" + mName + "]: abandonPendingMessages");

  if (!mRestrictedMessagesQueue.empty()) {
    for (const auto &pendingMsg : mRestrictedMessagesQueue) {
      if (pendingMsg) {
        Log::getInstance().d("[" + mName + "]: informing CANCELLED");
        pendingMsg->informDispatchFailure(pendingMsg, Message::Callback::Status::CANCELLED);
      }
    }
    mRestrictedMessagesQueue.clear();
  }
}

string SingleDispatchRestrictionHandleDuplicate::to_string() { return mName; }
