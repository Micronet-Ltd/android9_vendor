/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <string>
#include <framework/legacy.h>
struct MessageContext
{
    uint32_t serial; // For legacy purposes
    qcril_instance_id_e_type instance_id;

    MessageContext(qcril_instance_id_e_type instance_id): instance_id(instance_id)
    {}
    MessageContext(qcril_instance_id_e_type instance_id, uint32_t serial):
        serial(serial),
        instance_id(instance_id)
    {}
    qcril_instance_id_e_type getInstanceId() {
        return instance_id;
    }

};

template <class T>
struct MessageContextBase: public MessageContext
{

    MessageContextBase(qcril_instance_id_e_type instance_id): MessageContext(instance_id) {}
    MessageContextBase(qcril_instance_id_e_type instance_id, uint32_t serial):
        MessageContext(instance_id, serial) {}
     std::string toString() {
         return T::toString();
     }
     qcril_instance_id_e_type getInstanceId() { return MessageContext::instance_id; }
};

