/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include "bta_ag_twsp_dev.h"
#include "bta_ag_twsp.h"
#include "internal_include/bt_trace.h"
#include "bta_ag_int.h"


#if (TWS_AG_ENABLED == TRUE)
//forward declarations
void select_microphone_path(tBTA_AG_SCB *best_scb);


tTWSPLUS_DEVICE twsp_devices[MAX_TWSPLUS_DEVICES];

uint8_t g_latest_selected_eb_role = TWSPLUS_EB_ROLE_LEFT;

uint8_t get_lat_selected_mic_eb_role() {
    return g_latest_selected_eb_role;
}

tBTA_AG_SCB* get_twsp_with_role(uint8_t role) {
   int i;
   for (i=0; i<MAX_TWSPLUS_DEVICES; i++) {
      if (twsp_devices[i].p_scb != NULL &&
          twsp_devices[i].role == role) {
          return twsp_devices[i].p_scb;
      }
   }
   return NULL;
}

void reset_twsp_device(int  eb_idx) {
    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return;
    }

    if (get_lat_selected_mic_eb_role() == twsp_devices[eb_idx].role) {
        //Trigger Microphone Switch
        uint8_t other_twsp_role =
            (twsp_devices[eb_idx].role == TWSPLUS_EB_ROLE_LEFT) ?
                        TWSPLUS_EB_ROLE_RIGHT : TWSPLUS_EB_ROLE_LEFT;
        tBTA_AG_SCB *peer_scb = get_twsp_with_role(other_twsp_role);
        if (peer_scb != NULL) {
             select_microphone_path(peer_scb);
        } else {
             APPL_TRACE_WARNING("%s: peer_scb is NULL, No mic switch", __func__);
        }
     }

    twsp_devices[eb_idx].p_scb = NULL;
    twsp_devices[eb_idx].battery_charge = TWSPLUS_MIN_BATTERY_CHARGE;
    twsp_devices[eb_idx].state = TWSPLUS_EB_STATE_OFF;
    twsp_devices[eb_idx].role =  TWSPLUS_EB_ROLE_INVALID;
    twsp_devices[eb_idx].mic_path_delay = TWSPLUS_INVALID_MICPATH_DELAY;
    twsp_devices[eb_idx].mic_quality = TWSPLUS_MIN_MIC_QUALITY;
    twsp_devices[eb_idx].qdsp_nr = TWSPLUS_INVALID_QDSP_VALUE;
    twsp_devices[eb_idx].qdsp_ec = TWSPLUS_INVALID_QDSP_VALUE;
    twsp_devices[eb_idx].ring_sent = false;
}

void update_twsp_device(tBTA_AG_SCB* p_scb) {
    for (int i=0; i<MAX_TWSPLUS_DEVICES; i++) {
        if (twsp_devices[i].p_scb == NULL) {
            APPL_TRACE_WARNING("%s: idx: %d, p_scb: %x", __func__, i, p_scb);
            twsp_devices[i].p_scb = p_scb;
            twsp_devices[i].battery_charge = TWSPLUS_MIN_BATTERY_CHARGE;
            twsp_devices[i].state = TWSPLUS_EB_STATE_OFF;

            int other_idx = (i == PRIMARY_EB_IDX) ? SECONDARY_EB_IDX : PRIMARY_EB_IDX;
            if (twsp_devices[other_idx].p_scb != NULL &&
                    twsp_devices[other_idx].role == TWSPLUS_EB_ROLE_LEFT) {
                twsp_devices[i].role = TWSPLUS_EB_ROLE_RIGHT;
            } else {
                twsp_devices[i].role = TWSPLUS_EB_ROLE_LEFT;
            }

            APPL_TRACE_WARNING("%s: idx: %d, role: %d", __func__, i, twsp_devices[i].role);
            twsp_devices[i].mic_path_delay = TWSPLUS_INVALID_MICPATH_DELAY;
            twsp_devices[i].mic_quality = TWSPLUS_MIN_MIC_QUALITY;
            twsp_devices[i].qdsp_nr = TWSPLUS_INVALID_QDSP_VALUE;
            twsp_devices[i].qdsp_ec = TWSPLUS_INVALID_QDSP_VALUE;
            twsp_devices[i].ring_sent = false;
            return;
        }
    }

    APPL_TRACE_WARNING("%s: Invalid p_scb %d\n", __func__);
    return;
}

void init_twsp_devices() {
  int i;
  for (i=0; i<MAX_TWSPLUS_DEVICES; i++) {
      reset_twsp_device(i);
  }
}

void print_twsp_device_status(int eb_idx) {
    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return;
    }

    APPL_TRACE_DEBUG("%s: idx : %d", __func__, eb_idx);
    APPL_TRACE_DEBUG("%s: p_scb : %x", __func__, twsp_devices[eb_idx].p_scb);
    APPL_TRACE_DEBUG("%s: mic_quality : %d", __func__,
                               twsp_devices[eb_idx].mic_quality);
    APPL_TRACE_DEBUG("%s: battery_charge : %d", __func__,
                               twsp_devices[eb_idx].battery_charge);
    APPL_TRACE_DEBUG("%s: mic_path_delay : %d", __func__,
                               twsp_devices[eb_idx].mic_path_delay);
    APPL_TRACE_DEBUG("%s: state : %d", __func__, twsp_devices[eb_idx].state);
    APPL_TRACE_DEBUG("%s: role : %d", __func__, twsp_devices[eb_idx].role);
    APPL_TRACE_DEBUG("%s: QDSP_EC : %d", __func__,
                               twsp_devices[eb_idx].qdsp_ec);
    APPL_TRACE_DEBUG("%s: QDSP_NR : %d", __func__,
                               twsp_devices[eb_idx].qdsp_nr);
    APPL_TRACE_DEBUG("%s: ring sent : %d", __func__,
                               twsp_devices[eb_idx].ring_sent);
}

void print_twsp_devices_status() {
    int i;
    for (i=0; i<MAX_TWSPLUS_DEVICES; i++) {
         print_twsp_device_status(i);
    }
}

bool twsp_get_left_eb_addr(RawAddress& addr) {
  int i;
  for (i=0; i<MAX_TWSPLUS_DEVICES; i++) {
     if (twsp_devices[i].p_scb != NULL &&
          twsp_devices[i].role == TWSPLUS_EB_ROLE_LEFT) {
         addr =  twsp_devices[i].p_scb->peer_addr;
         return true;
     }
  }
  return false;
}

bool twsp_get_right_eb_addr(RawAddress& addr) {
  int i;
  for (i=0; i<MAX_TWSPLUS_DEVICES; i++) {
     if (twsp_devices[i].p_scb != NULL &&
          twsp_devices[i].role == TWSPLUS_EB_ROLE_RIGHT) {
         addr = twsp_devices[i].p_scb->peer_addr;
         return true;
     }
  }
  return false;
}

tBTA_AG_SCB* twsp_get_best_mic_scb () {
    int best_mic_quality = -1/*TWSPLUS_MIN_MIC_QUALITY*/;
    int selected_idx = -1;

    for (int i=0; i<MAX_TWSPLUS_DEVICES; i++) {
        if (twsp_devices[i].p_scb != NULL) {
            if (twsp_devices[i].mic_quality > best_mic_quality) {
                selected_idx = i;
                best_mic_quality = twsp_devices[i].mic_quality;
            }
        }
    }
    APPL_TRACE_DEBUG("%s: selected idx is : %d", __func__, selected_idx);
    if (selected_idx == -1) {
        return nullptr;
    } else {
        return twsp_devices[selected_idx].p_scb;
    }
}

int get_best_mic_quality_eb_role () {
    int best_mic_quality = -1/*TWSPLUS_MIN_MIC_QUALITY*/;
    int selected_idx = -1;

    for (int i=0; i<MAX_TWSPLUS_DEVICES; i++) {
        if (twsp_devices[i].mic_quality > best_mic_quality) {
            selected_idx = i;
            best_mic_quality = twsp_devices[i].mic_quality;
        }
    }
    APPL_TRACE_DEBUG("%s: selected idx is : %d", __func__, selected_idx);
    return twsp_devices[selected_idx].role;
}

int twsp_get_idx_by_scb(tBTA_AG_SCB* p_scb) {
    int res = -1;
    if (p_scb == NULL) {
        APPL_TRACE_ERROR("%s: scb is NULL", __func__);
        return res;
    }
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
        if (twsp_devices[i].p_scb == p_scb) {
            res = i;
            break;
        }
    }
    return res;
}

void select_microphone_path(tBTA_AG_SCB *best_scb) {
    APPL_TRACE_DEBUG("%s: best_scb : %x\n", __func__, best_scb);
    tBTA_AG_SCB *peer_scb = get_other_twsp_scb(best_scb->peer_addr);
    twsp_update_microphone_selection(peer_scb, best_scb);

    int idx = twsp_get_idx_by_scb(best_scb);
    if (idx != -1) {
        //Update the earbud role
        g_latest_selected_eb_role = twsp_devices[idx].role;
    }

    APPL_TRACE_DEBUG("%s: g_latest_selected_eb_role : %d\n", __func__, g_latest_selected_eb_role);
}

void process_mic_quality_change(int eb_idx, uint8_t mic_quality) {
    int last_sel_role;
    APPL_TRACE_DEBUG("%s: >> : %d %d\n", __func__, eb_idx, mic_quality);

    last_sel_role = get_lat_selected_mic_eb_role();
    APPL_TRACE_DEBUG("%s: last_sel_role : %d\n", __func__, last_sel_role);
    if (get_best_mic_quality_eb_role() == twsp_devices[eb_idx].role &&
            last_sel_role != twsp_devices[eb_idx].role) {
            //Trigger the swap
            APPL_TRACE_DEBUG("%s: Select EB%d  mic, SWAP", __func__, eb_idx);
            select_microphone_path(twsp_devices[eb_idx].p_scb);
    } else {
            APPL_TRACE_DEBUG("%s: EB%d is not of best mic quality for now", __func__, eb_idx);
    }

    print_twsp_devices_status();
}

bool set_twsp_mic_quality(int eb_idx, uint8_t mic_quality) {
    APPL_TRACE_DEBUG("%s: mic_qual : %d\n", __func__, mic_quality);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    if (mic_quality < TWSPLUS_MIN_MIC_QUALITY || mic_quality > TWSPLUS_MAX_MIC_QUALITY) {
        APPL_TRACE_WARNING("%s: Invalid mic_quality: %d\n", __func__, mic_quality);
        return false;
    }

    twsp_devices[eb_idx].mic_quality = mic_quality;
    process_mic_quality_change(eb_idx, mic_quality);
    return true;
}

bool set_twsp_mic_path_delay(int eb_idx, uint16_t mic_path_delay) {
    APPL_TRACE_DEBUG("%s: mic_path_delay : %d\n", __func__, mic_path_delay);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    if (mic_path_delay < TWSPLUS_MIN_MICPATH_DELAY || mic_path_delay > TWSPLUS_MAX_MICPATH_DELAY) {
        APPL_TRACE_WARNING("%s: Invalid mic_path_delay: %d\n", __func__, mic_path_delay);
        return false;
    }

    twsp_devices[eb_idx].mic_path_delay = mic_path_delay;
    return true;
}

uint16_t get_twsp_mic_path_delay(int eb_idx) {
    APPL_TRACE_DEBUG("%s:  %d\n", __func__);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    return twsp_devices[eb_idx].mic_path_delay;
}

uint8_t get_twsp_qdsp_nr(int eb_idx) {
    APPL_TRACE_DEBUG("%s:  %d\n", __func__);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    return twsp_devices[eb_idx].qdsp_nr;
}

uint8_t get_twsp_qdsp_ec(int eb_idx) {
    APPL_TRACE_DEBUG("%s:  %d\n", __func__);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    return twsp_devices[eb_idx].qdsp_ec;
}

bool set_twsp_battery_charge(int eb_idx, uint8_t battery_charge) {
    APPL_TRACE_DEBUG("%s: battery_charge : %d\n", __func__, battery_charge);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    if (battery_charge < TWSPLUS_MIN_BATTERY_CHARGE || battery_charge > TWSPLUS_MAX_BATTERY_CHARGE) {
        APPL_TRACE_WARNING("%s: Invalid battery_charge: %d\n", __func__, battery_charge);
        return false;
    }

    twsp_devices[eb_idx].battery_charge = battery_charge;
    return true;
}

void process_twsp_state_change (int eb_idx, uint8_t state) {
    APPL_TRACE_DEBUG("%s: state : %d\n", __func__, state);
    if (state == TWSPLUS_EB_STATE_OUT_OF_EAR) {
        //Delay for XXX
        if (get_lat_selected_mic_eb_role() == twsp_devices[eb_idx].role) {
          //Trigger Microphone Switch
          tBTA_AG_SCB *p_scb =  twsp_devices[eb_idx].p_scb;
          if (p_scb != NULL) {
              tBTA_AG_SCB *peer_scb = get_other_twsp_scb(p_scb->peer_addr);
              if (peer_scb != NULL) {
                 select_microphone_path(peer_scb);
              } else {
                 APPL_TRACE_WARNING("%s: peer_scb is NULL, No mic switch", __func__);
              }
          }
        }
    }
}

void process_twsp_role_change (int eb_idx, uint8_t role) {
    APPL_TRACE_DEBUG("%s: role : %d\n", __func__, role);
    if (get_lat_selected_mic_eb_role() == twsp_devices[eb_idx].role) {
        //Trigger Microphone Switch
        g_latest_selected_eb_role = role;
        APPL_TRACE_DEBUG("%s: g_latest_selected_eb_role updated: %d\n", __func__, g_latest_selected_eb_role);
    }
}

bool set_twsp_state(int eb_idx, uint8_t state) {
    APPL_TRACE_DEBUG("%s: current state: %d new state : %d\n", __func__, twsp_devices[eb_idx].state, state);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    if (state < TWSPLUS_EB_STATE_OFF || state > TWSPLUS_EB_STATE_INEAR) {
        APPL_TRACE_WARNING("%s: Invalid state: %d\n", __func__, state);
        return false;
    }

    if (twsp_devices[eb_idx].state == TWSPLUS_EB_STATE_INEAR &&
        state != TWSPLUS_EB_STATE_INEAR) {
        //If It is tranistioning away from InEar,
        //Clear the ring_set flag
        twsp_devices[eb_idx].ring_sent = false;
    }
    twsp_devices[eb_idx].state = state;
    process_twsp_state_change(eb_idx, state);
    return true;
}

bool set_twsp_role(int eb_idx, uint8_t role) {
    APPL_TRACE_DEBUG("%s: role : %d\n", __func__, role);

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    if (role < TWSPLUS_EB_ROLE_LEFT || role > TWSPLUS_EB_ROLE_MONO) {
        APPL_TRACE_WARNING("%s: Invalid role: %d\n", __func__, role);
        return false;
    }

    //if (twsp_devices[eb_idx].role != role)
    //    process_twsp_role_change(eb_idx, role);
    twsp_devices[eb_idx].role = role;
    return true;
}

bool set_twsp_qdsp_features(int eb_idx, uint8_t features_mask) {
    APPL_TRACE_DEBUG("%s: features mask: %x\n", __func__, features_mask);
    uint8_t feature = 0;
    uint8_t value = 0;

    if (eb_idx < PRIMARY_EB_IDX || eb_idx > SECONDARY_EB_IDX) {
        APPL_TRACE_WARNING("%s: Invalid eb_idx: %d\n", __func__, eb_idx);
        return false;
    }

    feature = (features_mask&TWSPLUS_QDSP_FEATURE_MASK) > 1;
    value = (features_mask&TWSPLUS_QDSP_VALUE_MASK);
    APPL_TRACE_DEBUG ("%s: feature: %d, value :%d", __func__, feature, value);
    switch (feature) {
        case TWSPLUS_QDSP_NOISE_REDUCTION:
            twsp_devices[eb_idx].qdsp_nr = value;
        break;
        case TWSPLUS_QDSP_ECHO_CANCELLATION:
            twsp_devices[eb_idx].qdsp_ec = value;
        break;
    }
    return true;
}

uint8_t get_twsp_role(tBTA_AG_SCB *p_scb) {
    APPL_TRACE_DEBUG("%s: p_scb : %d\n", __func__, p_scb);
    uint8_t role = 0;
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
        if (p_scb == twsp_devices[i].p_scb)
        {
           role = twsp_devices[i].role;
           break;
        }
    }
    return role;
}

bool twsp_is_ring_sent(tBTA_AG_SCB *p_scb) {
    int sel_idx = -1;
    bool ret = false;
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
        if (twsp_devices[i].p_scb == p_scb) {
            sel_idx = i;
            break;
        }
    }

    if (sel_idx == -1) {
        ret = false;
    } else {
        ret = twsp_devices[sel_idx].ring_sent;
    }
    APPL_TRACE_DEBUG("%s: returning: %d", __func__, ret);
    return ret;
}

bool twsp_set_ring_sent(tBTA_AG_SCB *p_scb, bool ring_sent) {
    int sel_idx = -1;
    bool ret = false;
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
        if (twsp_devices[i].p_scb == p_scb) {
            sel_idx = i;
            break;
        }
    }

    if (sel_idx == -1) {
        ret = false;
    } else {
        twsp_devices[sel_idx].ring_sent = ring_sent;
        ret = true;
    }
    APPL_TRACE_DEBUG("%s: returning: %d", __func__, ret);
    return ret;
}

bool twsp_ring_needed(tBTA_AG_SCB *p_scb) {
    int sel_idx = -1;
    bool ring_already_sent = false;
    int prev_sent_idx = -1;
    bool ret = false;
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
        if (twsp_devices[i].ring_sent) {
            ring_already_sent = true;
            if (twsp_devices[i].p_scb == p_scb) {
                //If ring was send to same scb before
                //should send it to same scb again
                prev_sent_idx = i;
            }
        }
        else if (twsp_devices[i].p_scb == p_scb) {
            sel_idx = i;
        }
    }
    if (ring_already_sent) {
        if (prev_sent_idx == -1) {
            ret = false;
        } else {
            APPL_TRACE_DEBUG("%s: resent timer to same scb", __func__);
            ret = true;
        }
    } else {
        if (sel_idx == -1) {
            APPL_TRACE_ERROR("%s: Invalid sel_idx: %d", __func__, sel_idx);
            ret = false;
        } else {
            //If ring is not sent to any of them
            //check if it is in ear
            if (twsp_devices[sel_idx].state == TWSPLUS_EB_STATE_INEAR) {
                ret = true;
            } else {
                ret = false;
            }
        }
    }

    APPL_TRACE_DEBUG("%s: returning: %d", __func__, ret);
    return ret;
}

void twsp_clr_all_ring_sent () {
    for (int i=0; i<=SECONDARY_EB_IDX; i++) {
            twsp_devices[i].ring_sent = false;
    }
}

void twsp_handle_vs_at_events(tBTA_AG_SCB* p_scb, uint16_t cmd, int16_t int_arg)
{
    APPL_TRACE_DEBUG("%s: p_scb : %x cmd : %d", __func__, p_scb, cmd);

    int idx = twsp_get_idx_by_scb(p_scb);
    if (idx < 0) {
        APPL_TRACE_ERROR("%s: Invalid SCB handle: %x", __func__, p_scb);
        //bta_ag_send_error(p_scb, BTA_AG_ERR_OP_NOT_SUPPORTED);
        return;
    }

    //bta_ag_send_ok(p_scb);
    switch(cmd) {
       case BTA_AG_TWSP_AT_QMQ_EVT: {
            set_twsp_mic_quality(idx, int_arg);
       } break;
       case BTA_AG_TWSP_AT_QES_EVT: {
            set_twsp_state(idx, int_arg);
       } break;
       case BTA_AG_TWSP_AT_QER_EVT: {
            set_twsp_role(idx, int_arg);
       } break;
       case BTA_AG_TWSP_AT_QBC_EVT: {
            set_twsp_battery_charge(idx, int_arg);
       } break;
       case BTA_AG_TWSP_AT_QMD_EVT: {
            set_twsp_mic_path_delay(idx, int_arg);
       } break;
       case BTA_AG_TWSP_AT_QDSP_EVT: {
            set_twsp_qdsp_features(idx, int_arg);
       } break;
    }
}

#endif //#if (TWS_AG_ENABLED == TRUE)

