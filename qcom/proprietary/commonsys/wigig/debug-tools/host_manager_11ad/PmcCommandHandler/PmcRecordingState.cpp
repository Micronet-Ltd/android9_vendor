/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcRecordingState.h"
#include "Utils.h"

void PmcRecordingState::SetState(PMC_RECORDING_STATE newState)
{
    LOG_INFO << "Changing PMC recording state: " << GetState() << " to " << newState << std::endl;
    switch (newState)
    {
    case PMC_RECORDING_STATE::PMC_RECORDING_STATE_SLAVE_ACTIVE:
    case PMC_RECORDING_STATE::PMC_RECORDING_STATE_ACTIVE:
        m_startRecordingTime = Utils::GetCurrentDotNetDateTimeString();
        break;

    case PMC_RECORDING_STATE::PMC_RECORDING_STATE_SLAVE_STOPPED:
    case PMC_RECORDING_STATE::PMC_RECORDING_STATE_STOPPED:
        m_stopRecordingTime = Utils::GetCurrentDotNetDateTimeString();
        break;

    default:
        break;
    }
    m_state = newState;
    LOG_INFO << this;
}

