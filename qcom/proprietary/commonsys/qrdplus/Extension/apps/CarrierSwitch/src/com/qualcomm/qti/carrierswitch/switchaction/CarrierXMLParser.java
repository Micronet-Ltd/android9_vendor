/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch.switchaction;

import android.content.Context;
import android.content.res.XmlResourceParser;

import com.qualcomm.qti.carrierswitch.LogUtil;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public abstract class CarrierXMLParser {

    static List<ParsedInfo> parseFrom(Context context, int resId)
            throws XmlPullParserException, IOException {
        List<ParsedInfo> infos = new ArrayList<>(10);
        XmlResourceParser parser =
                context.getResources().getXml(resId);
        int eventType = parser.getEventType();

        while (eventType != XmlPullParser.END_DOCUMENT) {
            switch (eventType) {
                case XmlPullParser.START_DOCUMENT:
                    LogUtil.v(CarrierXMLParser.class, "parse start doc = " + parser.getName());
                    break;

                case XmlPullParser.START_TAG:
                    LogUtil.v(CarrierXMLParser.class, "parse start tag = " + parser.getName());
                    if (parser.getName().equals(OverlayParsedInfo.PARSER_TAG)) {
                        ParsedInfo info = OverlayParsedInfo.getParser().parse(parser);
                        infos.add(info);
                    } else if (parser.getName().equals(ApplicationParsedInfo.PARSER_TAG)) {
                        ParsedInfo info = ApplicationParsedInfo.getParser().parse(parser);
                        infos.add(info);
                    }
                    break;
                case XmlPullParser.END_TAG:
                    LogUtil.v(CarrierXMLParser.class, "parse end tag = " + parser.getName());
                    break;
            }

            eventType = parser.next();
        }
        return infos;
    }
}
