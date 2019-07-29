/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
*
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcStopHandler.h"
#include "PmcRecordingState.h"
#include "PmcSequence.h"
#include "PmcService.h"
#include "PmcRecordingFileCreator.h"

void PmcStopHandler::HandleRequest(const PmcStopRequest& jsonRequest, PmcStopResponse& jsonResponse)
{
    const std::string& deviceName = jsonRequest.GetDeviceName();
    LOG_DEBUG << "PMC stop request for Device: " << jsonRequest.GetDeviceName() << std::endl;
    BasebandType type;
    auto nameTypeValidRes = PmcSequence::ValidateDeviceNameAndType(deviceName, type);
    if(!nameTypeValidRes.IsSuccess())
    {
        jsonResponse.Fail(nameTypeValidRes.GetStatusMessage());
        return;
    }
    //SLAVE RECORDING NOT SUPPORTED

    auto res = PmcSequence::PausePmcRecording(deviceName);
    if(!res.IsSuccess())
    {
        jsonResponse.Fail(res.GetStatusMessage());
        return;
    }
    PmcService::GetInstance().GetPmcDeviceContext(deviceName).GetDeviceRecordingState().SetState(PMC_RECORDING_STATE::PMC_RECORDING_STATE_STOPPED);
    PmcRecordingFileCreator pmcFilesCreator = PmcRecordingFileCreator(deviceName, type);
    std::string pmcDataPath;
    auto filesRes = pmcFilesCreator.CreatePmcFiles(pmcDataPath);
    if (!filesRes.IsSuccess())
    {
        jsonResponse.Fail(filesRes.GetStatusMessage());
        return;
    }
    jsonResponse.SetRecordingDirectory(pmcDataPath.c_str());
    // When the PMC is stopped after SysAssert, the ring deallocation request returns error (-11).
    // We should ignore this error to have the graph rendered.
    PmcSequence::FreePmcRing(deviceName, false);
}

