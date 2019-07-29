/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch.switchaction;

import com.qualcomm.qti.carrierswitch.LogUtil;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

public class ParsedInfo {
    public static final String TAG_packageName = "packageName";
    public static final String TAG_needReboot = "needReboot";
    public static final String TAG_needFactoryReset = "needFactoryReset";
    public static final String TAG_needSecondReboot = "needSecondReboot";
    public static final String TAG_mode = "mode";
    public static final String TAG_carrierId = "carrierId";

    String packageName;
    boolean needReboot;
    boolean needFactoryReset;
    boolean needSecondReboot;
    String mode;
    int carrierId = -1;

    static abstract class Parser {
        abstract public ParsedInfo parse(XmlPullParser parser) throws
                XmlPullParserException, IOException;

        boolean fillBody(ParsedInfo info, XmlPullParser parser) throws
                XmlPullParserException, IOException {
            boolean ret = false;
            if (TAG_packageName.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.packageName = parser.getText();
                LogUtil.v(ParsedInfo.class, "parse package name: " + info.packageName);
            } else if (TAG_needReboot.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.needReboot = parser.getText().trim().equals("true");
                LogUtil.v(ParsedInfo.class,  "parse reboot: " + info.needReboot);
            } else if (TAG_needFactoryReset.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.needFactoryReset = parser.getText().trim().equals("true");
                LogUtil.v(ParsedInfo.class, "parse factory reset: " + info.needFactoryReset);
            } else if (TAG_needSecondReboot.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.needSecondReboot = parser.getText().trim().equals("true");
                LogUtil.v(ParsedInfo.class, "parse second reboot: " + info.needSecondReboot);
            } else if (TAG_mode.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.mode = parser.getText();
                LogUtil.v(ParsedInfo.class, "parse mode: " + info.mode);
            } else if (TAG_carrierId.equals(parser.getName())) {
                parser.next();
                ret = true;
                info.carrierId = Integer.valueOf(parser.getText());
                LogUtil.v(ParsedInfo.class, "parse carrier Id: " + info.carrierId);
            }
            return ret;
        }
    }
}
