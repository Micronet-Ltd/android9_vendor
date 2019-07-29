#! /system/bin/sh

# Copyright (c) 2018, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

function set_for_system_server() {
    # Check if another instance of this script for system_server is runnning
    num_of_process=`ps -Af | grep "init.qti.khung.sh system_server" | wc -l`

    # Above command returns 1 entry for grep command we use and 1 entry
    # for the current process , if there is any other instance running
    # it shows extra entries .. So if there are more than 2 entries it
    # implies that we have another instance of same script running

    if [ $num_of_process -gt 2 ]; then
        log -t KHUNG -p w "Another instance is trying set ..Exiting here.. "
        return
    fi

    cnt=20
    while [ $cnt -gt 0 ]; do
        proc_id=`pidof system_server`
        if [ "$proc_id" == "" ]; then
            cnt=`expr $cnt - 1`
            log -t KHUNG -p i "system_server is not up yet ..Try $cnt more times"
            sleep 0.5
        else
            hang_detection_file="/proc/"$proc_id"/hang_detection_enabled"
            echo 1 > $hang_detection_file
            break
        fi
    done
}

function set_hang_detection_enabled() {
    log -t KHUNG -p i "Setting hang_detection_enabled for processes "$1
    prop_value=`getprop persist.vendor.khung.disable`
    if [ "$prop_value" == "1" ]; then
        log -t KHUNG -p i "Persist property is disabled"
        return
    fi
    if [ $1 == "system_server" ]; then
        set_for_system_server
    else
        proc_id=`pidof $1`
        if [ "$proc_id" == "" ]; then
            log -t KHUNG -p i "No such process "$1
        else
            hang_detection_file="/proc/"$proc_id"/hang_detection_enabled"
            echo 1 > $hang_detection_file
        fi
    fi
}

function set_hang_detection() {
    log -t KHUNG -p i "Set hang detection for all processes $1"
    for pname in "${process_names[@]}"
    do
        proc_id=`pidof $pname`
        echo $1 > "/proc/"$proc_id"/hang_detection_enabled"
    done
}

function process_arguments() {
    case "$1" in
    "enable")
        set_hang_detection 1
        ;;
    "disable")
        set_hang_detection 0
        ;;
    *)
        set_hang_detection_enabled $1
        ;;
    esac
}
#List of processes
process_names=(system_server cameraserver audioserver surfaceflinger vold)

target=`getprop ro.board.platform`

case "$target" in
    "sm6150")
        process_arguments $1
        ;;
    *)
       log -t KHUNG -p w " Khung is not enabled for $target"
       ;;
esac
