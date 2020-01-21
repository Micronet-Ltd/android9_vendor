package com.android.incallui.util;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;

public class LovdreamCallUtil {
	
	private static LovdreamCallUtil mLovdreamCallUtil;
	HeadSetStreamReceiver mReceiver ;
	
	CallBack mCallBack;
	
    //add by xxf
    private static final int BT_CLOSE=0;
    private static final int BT_OPEN_DISCONNECT=1;
    private static final int BT_OPEN_CONNECT=2;
    private static final String BT_ACTION = "lovdream.bt.contect";
  //add by xxf
	
	public void setCallBack(CallBack callBack){
		this.mCallBack = callBack;
	}
	
	private LovdreamCallUtil(){}
	public static LovdreamCallUtil getCallUtil(){ 
		if(mLovdreamCallUtil==null){
			mLovdreamCallUtil = new LovdreamCallUtil();
		}
		return mLovdreamCallUtil;
	}
	
	private class HeadSetStreamReceiver extends BroadcastReceiver{
		 
        @Override
        public void onReceive(Context context, Intent intent) {
        	if(mCallBack==null) return;
             if (intent.getAction().equals(Intent.ACTION_HEADSET_PLUG)) {
                    int state = intent.getIntExtra("state", -1);
                    mCallBack.headSetChanged(state==1);
                }else if(intent.getAction().equals(BT_ACTION)){
                	 int state = intent.getIntExtra("state", BT_CLOSE);
                	 mCallBack.BtHeadSetChanged(state>=BT_OPEN_CONNECT);
                }
             
             
        }
         
    }
	
	
	public LovdreamCallUtil registerHeadSetBroadcast(Context mContext){
		if(mReceiver!=null) mReceiver =null;
		mReceiver= new HeadSetStreamReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        mContext.registerReceiver(mReceiver, filter);
        return this;
	}
	public void unRegisterHeadSetBroadcast(Context mContext){
		try {
			 mContext.unregisterReceiver(mReceiver);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	
	public interface CallBack{
		void headSetChanged(boolean isPlugin);
		void BtHeadSetChanged(boolean isPlugin);
	}

}
