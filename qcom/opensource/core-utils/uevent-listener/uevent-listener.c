/* Copyright (c) 2018- 2019, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of The Linux Foundation nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
* Copyright (C) 2017 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "uevent-listener.h"

static void parse_event (const char *msg, struct uevent *uevent) {
    uevent_debug = false;
    uevent->action = "";
    uevent->path = "";
    uevent->subsystem = "";
    uevent->firmware = "";
    uevent->partition_name = NULL;
    uevent->partition_num = -1;
    uevent->device_name = NULL;
    uevent->notification = -1;
    while (*msg) {
        if (!strncmp(msg, "ACTION=", 7)) {
            msg +=7;
            uevent->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg +=8;
            uevent->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg +=10;
            uevent->subsystem = msg;
        } else if (!strncmp(msg, "FIRMWARE", 9)) {
            msg +=9;
            uevent->firmware = msg;
        } else if (!strncmp(msg, "MAJOR=", 6)) {
            msg += 6;
            uevent->major = atoi(msg);
        } else if (!strncmp(msg, "MINOR=", 6)) {
            msg += 6;
            uevent->minor = atoi(msg);
        } else if (!strncmp(msg, "PARTN=", 6)) {
            msg += 6;
            uevent->partition_num = atoi(msg);
        } else if (!strncmp(msg, "PARTNAME=", 9)) {
            msg +=9;
            uevent->partition_name = msg;
        } else if (!strncmp(msg, "NOTIFICATION=",13)) {
            msg +=13;
            uevent->notification = atoi(msg);
        } else if (!strncmp(msg, "DEVNAME=", 8)) {
            msg += 8;
            uevent->device_name = msg;
        }
        /*advance after the next \0 */
        while (*msg++);
    }
    if (uevent_debug) {
    ALOGI("parse uevent { '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d, %d }\n", uevent->action, \
        uevent->path, uevent->subsystem, uevent->firmware, uevent->partition_name, uevent->device_name, \
        uevent->notification, uevent->partition_num, uevent->major, uevent->minor);
    }
}

int main () {
    int err = 0;
    int n = 0;
    char msg[UEVENT_MSG_LENGTH + 2];
    struct pollfd uevent_fd;
    ALOGI("Starting up Uevent listener....");
    uevent_fd.events = POLLIN;
    uevent_fd.fd = uevent_open_socket(256 * 1024, true);
    if (uevent_fd.fd == -1) {
        ALOGW("Uevent socket fd null");
        return 0;
    }
    while (1) {
        uevent_fd.revents = 0;
        err = poll(&uevent_fd, 1, -1);
        if (err == -1) {
            ALOGI("Error in Polling due to error : %s.(Error no : %d)\n",strerror(errno), errno);
            break;
        }
        if (uevent_fd.revents & POLLIN) {
            n = uevent_kernel_multicast_recv(uevent_fd.fd, msg, sizeof(msg));
            if (n <= 0) {
                ALOGE("Netlink Socket read failed %d\n", n);
                continue;
            }
            if (n >= UEVENT_MSG_LENGTH)
                continue;
            msg[n] = '\0';
            msg[n+1] = '\0';
            struct uevent uevent;
            parse_event(msg, &uevent);
            if (uevent.notification == SUBSYS_AFTER_POWERUP) {
                ALOGI("Uevent received for SUBSYS_AFTER_POWERUP notification for '%s' subsystem, uevent object = { '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d, %d }\n", uevent.subsystem, \
        uevent.action, uevent.path, uevent.subsystem, uevent.firmware, uevent.partition_name, uevent.device_name, \
        uevent.notification, uevent.partition_num, uevent.major, uevent.minor);
                property_set("vendor.notify_after_powerup.subsys", uevent.subsystem);
            }
        }
    }
    return 0;
}
