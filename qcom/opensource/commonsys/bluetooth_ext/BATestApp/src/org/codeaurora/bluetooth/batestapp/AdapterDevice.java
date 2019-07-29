/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

package org.codeaurora.bluetooth.batestapp;

import android.bluetooth.BluetoothDevice;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class AdapterDevice extends RecyclerView.Adapter<AdapterDevice.ViewHolder> {

    private List<BroadcastAudioDevice> mList;
    private LayoutInflater mInflater;
    private RecyclerViewListener mListener;
    private static final String TAG=Utils.TAG +"AdapterDevice";

    public AdapterDevice(List<BroadcastAudioDevice> list, LayoutInflater inflater,
            RecyclerViewListener listener) {
        mList = list;
        mInflater = inflater;
        mListener = listener;
        Log.v(TAG," AdapterDevice ");
    }

    public void refreshDevices(List<BroadcastAudioDevice> list) {
        if (list == null) {
            mList = new ArrayList<BroadcastAudioDevice>();
        } else {
            mList = list;
        }
        Log.v(TAG," refreshDevices mList :"+mList.size());
        notifyDataSetChanged();
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, final int position) {

        BroadcastAudioDevice device = mList.get(position);
        if (device != null && device.getName() != null){
            holder.textViewName.setText(device.getName());
        }

        if (device != null && device.getBluetoothDevice() != null){
            holder.textViewAddress.setText(device.getBluetoothDevice()
                                                 .getAddress());
        }
        holder.mLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.v(TAG," onClick position  :" + position);
                mListener.onRecylerViewItemClicked(position);
            }
        });
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = mInflater.inflate(R.layout.layout_row_device, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public int getItemCount() {
        return mList.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        private TextView textViewName, textViewAddress;
        private LinearLayout mLayout;

        public ViewHolder(View v) {
            super(v);
            textViewName = (TextView) v.findViewById(R.id.id_tv_devicename);
            textViewAddress = (TextView) v.findViewById(R.id.id_tv_devieaddress);
            mLayout = (LinearLayout) v.findViewById(R.id.id_linearlayout_row);
        }
    }
}

