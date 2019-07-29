/******************************************************************************
@file    qcril_data_connectionservice.h
@brief   qcril data connection service

DESCRIPTION
Basic implementaton of IDataConnection interface.
Register as data connection HAL service

******************************************************************************/

/*=============================================================================
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include <vendor/qti/hardware/data/connection/1.0/IDataConnection.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using ::android::hardware::Return;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::sp;
using ::android::wp;

namespace vendor {
namespace qti {
namespace hardware {
namespace data {
namespace connection {
namespace V1_0 {

class DataConnectionServiceImpl : public IDataConnection {
public:
  DataConnectionServiceImpl() {}
  Return<StatusCode> getBearerAllocation(int32_t, const sp<IDataConnectionResponse>&) override {return StatusCode::NOT_SUPPORTED;}
  Return<StatusCode> getAllBearerAllocations(const sp<IDataConnectionResponse>&) override {return StatusCode::NOT_SUPPORTED;}
  Return<StatusCode> registerForAllBearerAllocationUpdates(const sp<IDataConnectionIndication>&) override {return StatusCode::NOT_SUPPORTED;}
  void setInstanceId(int instanceId);

private:
  int mInstanceId;

  // Functions from hidl_death_recipient
  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);
};

}
}
}
}
}
}
