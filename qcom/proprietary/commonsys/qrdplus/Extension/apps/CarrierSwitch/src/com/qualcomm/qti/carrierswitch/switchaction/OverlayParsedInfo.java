/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch.switchaction;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

public class OverlayParsedInfo extends ParsedInfo {
    static final String PARSER_TAG = "overlay";


    static private Parser mParser = new Parser();

    static ParsedInfo.Parser getParser() {
        return mParser;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("OverlayParsedInfo: [");
        sb.append("packageName:" + packageName);
        sb.append(", needReboot:" + needReboot);
        sb.append(", needFactoryReset:" + needFactoryReset);
        sb.append(", needSecondReboot:" + needSecondReboot);
        sb.append(", mode:" + mode);
        sb.append(", carrierId:" + carrierId);
        sb.append("]");
        return sb.toString();
    }

    static class Parser extends ParsedInfo.Parser {
        @Override
        public OverlayParsedInfo parse(XmlPullParser parser)
                throws XmlPullParserException, IOException {
            final int groupDepth = parser.getDepth();
            int type;
            OverlayParsedInfo info = new OverlayParsedInfo();
            while ((type = parser.next()) != XmlPullParser.END_TAG ||
                    parser.getDepth() > groupDepth) {
                if (type != XmlPullParser.START_TAG) {
                    continue;
                }
                fillBody(info, parser);
            }
            return info;
        }

        @Override
        boolean fillBody(ParsedInfo info, XmlPullParser parser) throws
                XmlPullParserException, IOException {
            boolean handled = super.fillBody(info, parser);
            if (!handled) {
                // TODO
            }
            return handled;
        }
    }
}
