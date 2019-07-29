/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2011-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <loc_cfg.h>
#include <loc_pla.h>
#include "GardenType.h"
#include "GardenFramework.h"
#include "GnssCase.h"
#include "GnssMultiClientCase.h"
#include "GeofenceCase.h"
#include "FlpCase.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

using namespace garden;
using namespace std;

typedef struct _GardenLib {
    string name;
    string args;
} GardenLib;

static vector<GardenLib> sLibs;


/** Default values for position/location */
static LocGpsLocation sPositionDefaultValues = {
    sizeof(LocGpsLocation),
    0,
    32.90285,
    -117.202185,
    0,
    0,
    0,
    10000,
    0.0,
    (LocGpsUtcTime)0,
};


// Default values
static CommandLineOptionsType sOptions = {
    LOC_GPS_POSITION_RECURRENCE_SINGLE,
    1,
    60, // Time to stop fix.
    1, // Android frame work (AFW)AGpsBearerType
    FALSE, // TRUE to use remote Api
    0, // ULP test case number
    0, // By default don't delete aiding data.
    LOC_GPS_POSITION_MODE_STANDALONE, // Standalone mode.
    1000, // 1000 millis between fixes
    0, // Accuracy
    0, // 1 millisecond?
    sPositionDefaultValues,
    {0}, // Invalid for NI response
    0, // Defaults to 0 back to back NI test
    0, // Number of elements in NI response pattern
    0, // Test Ril Interface
    {0}, // Default path to config file, will be set in main
    0, // Print NMEA info. By default does not get printed
    0, // SV info. Does not print the detaul by default
    20, // Default value of 20 seconds for time to first fix
    0, // ZPP test case number
    0, // Minimum number of SVs option off by default
    0, // Minimum number of SNR option off by default
    0, //Tracking Session status off by default
    0, // Start GPS tracking session using HW_FLP. Default 60s
    false, // Keep garden-app alive till Ctrl-C is pressed
    {0} //Path to geofence input file
};


void readConfigFile(char* path) {
    char networkInitResp[1024];
    char* tokenPtr;
    char* argPtr;
    double locationAccu, minSnr=sOptions.stopOnMinSnr;
    uint8_t locationAccu_valid = 0;
    loc_param_s_type cfg_parameter_table[] =
    {
      {"RECUR_MODE",    &sOptions.r,                                        NULL, 'n'},
      {"SESSION_NUM",   &sOptions.l,                                        NULL, 'n'},
      {"STOP_TIME",     &sOptions.t,                                        NULL, 'n'},
      {"REMOTE_API",    &sOptions.remoteAPI,                                NULL, 'n'},
      {"TEST_STACKS",   &sOptions.s,                                        NULL, 'n'},
      {"ULP_NUM",       &sOptions.ulpTestCaseNumber,                        NULL, 'n'},
      {"DEL_AIDING",    &sOptions.deleteAidingDataMask,                     NULL, 'n'},
      {"POS_MODE",      &sOptions.positionMode,                             NULL, 'n'},
      {"INTERVAL",      &sOptions.interval,                                 NULL, 'n'},
      {"ACCURACY",      &sOptions.accuracy,                                 NULL, 'n'},
      {"RESP_TIME",     &sOptions.responseTime,                             NULL, 'n'},
      {"LAT",           &sOptions.location.latitude,                        NULL, 'f'},
      {"LONG",          &sOptions.location.longitude,                       NULL, 'f'},
      {"LOC_ACCURACY",  &locationAccu,                       &locationAccu_valid, 'f'},
      {"RESP_PAT",      networkInitResp,                                    NULL, 's'},
      {"NI_NUM",        &sOptions.niCount,                                  NULL, 'n'},
      {"RESP_NUM",      &sOptions.niResPatCount,                            NULL, 'n'},
      {"RIL_IFACE",     &sOptions.rilInterface,                             NULL, 'n'},
      {"GPSCONF_PATH",  &sOptions.gpsConfPath,                              NULL, 's'},
      {"NMEA_PRINT",    &sOptions.printNmea,                                NULL, 'n'},
      {"SVINFO_DETAIL", &sOptions.satelliteDetails,                         NULL, 'n'},
      {"FIX_THR",       &sOptions.fixThresholdInSecs,                       NULL, 'n'},
      {"ZPP_NUM",       &sOptions.zppTestCaseNumber,                        NULL, 'n'},
      {"SV_MINNUM",     &sOptions.stopOnMinSvs,                             NULL, 'n'},
      {"MIN_SNR",       &minSnr,                                            NULL, 'f'},
      {"TRACK",         &sOptions.tracking,                                 NULL, 'n'},
      {"HW_FLP",        &sOptions.trackUsingFlpInSecs,                      NULL, 'n'},
    };

    strlcpy(sOptions.gpsConfPath, path, 256);
    // Initialize by reading the gps.conf file
    UTIL_READ_CONF(sOptions.gpsConfPath, cfg_parameter_table);
    if (locationAccu_valid) {
        sOptions.location.accuracy = (float)locationAccu;
    }
    sOptions.stopOnMinSnr = (float)minSnr;
    tokenPtr = strtok_r(networkInitResp, ",", &argPtr);
    if(sOptions.niResPatCount > 0) {
        for(int i=0; i<sOptions.niResPatCount; ++i) {
            if(tokenPtr != NULL) {
                sOptions.networkInitiatedResponse[i] = atoi(tokenPtr);
            }
            tokenPtr = strtok_r(NULL, ",", &argPtr);
        }
    }

    gardenPrint("Parameters read from the config file :");
    gardenPrint("**************************************");
    gardenPrint("RECUR_MODE    : %d", sOptions.r);
    gardenPrint("SESSION_NUM   : %d", sOptions.l);
    gardenPrint("STOP_TIME     : %d", sOptions.t);
    gardenPrint("REMOTE_API    : %d", sOptions.remoteAPI);
    gardenPrint("TEST_STACKS   : %d", sOptions.s);
    gardenPrint("ULP_NUM       : %d", sOptions.ulpTestCaseNumber);
    gardenPrint("DEL_AIDING    : %d", sOptions.deleteAidingDataMask);
    gardenPrint("POS_MODE      : %d", sOptions.positionMode);
    gardenPrint("INTERVAL      : %d", sOptions.interval);
    gardenPrint("ACCURACY      : %d", sOptions.accuracy);
    gardenPrint("RESP_TIME     : %d", sOptions.responseTime);
    gardenPrint("LAT           : %f", sOptions.location.latitude);
    gardenPrint("LONG          : %f", sOptions.location.longitude);
    gardenPrint("LOC_ACCURACY  : %f", sOptions.location.accuracy);
    gardenPrint("RESP_PAT      :");
    for(int i = 0; i<sOptions.niResPatCount; ++i) {
        gardenPrint("%12d",sOptions.networkInitiatedResponse[i]); }
    gardenPrint("NI_NUM        : %d", sOptions.niCount);
    gardenPrint("RESP_NUM      : %d", sOptions.niResPatCount);
    gardenPrint("RIL_IFACE     : %d", sOptions.rilInterface);
    gardenPrint("GPSCONF_PATH  : %s", sOptions.gpsConfPath);
    gardenPrint("NMEA_PRINT    : %d", sOptions.printNmea);
    gardenPrint("SVINFO_DETAIL : %d", sOptions.satelliteDetails);
    gardenPrint("FIX_THR       : %d", sOptions.fixThresholdInSecs);
    gardenPrint("ZPP_NUM       : %d", sOptions.zppTestCaseNumber);
    gardenPrint("SV_MINNUM     : %d", sOptions.stopOnMinSvs);
    gardenPrint("MIN_SNR       : %f", sOptions.stopOnMinSnr);
    gardenPrint("TRACK         : %d", sOptions.tracking);
    gardenPrint("HW_FLP        : %d", sOptions.trackUsingFlpInSecs);
    gardenPrint("**************************************");
}


static void printHelp(char **arg) {
     gardenPrint("usage: %s -r <1|0> -t <xxx> -u <1-12>", arg[0]);
     gardenPrint("    -r:  RECURRENCE 1:SINGLE; 0:PERIODIC; Defaults: %d", sOptions.r);
     gardenPrint("    -l:  Number of sessions to loop through. Takes an argument. An argument of 0 means no sessions. Defaults:%d", sOptions.l);
     gardenPrint("    -t:  User defined length of time to issue stop navigation. Takes an argument. Time in seconds Defaults: %d", sOptions.t);
     gardenPrint("    -R:  Use remote api. Default: %d", sOptions.remoteAPI);
#ifdef TEST_ULP
     gardenPrint("    -u:  run specified ULPLite test case Defaults: %d", sOptions.ulpTestCaseNumber);
#endif
     gardenPrint("    -s:  stacks to test: 1:afw; 2:vzw; 3:(afw | vzw, default) Defaults: %d",sOptions.s);
     gardenPrint("    -d:  Delete aiding data: Takes a hexadecimal mask as an argument as given in gps.h Defaults: 0x%X ",sOptions.deleteAidingDataMask);
     gardenPrint("    -m:  Position Mode. Takes an argument 0:LOC_GPS_POSITION_MODE_STANDALONE, 1:LOC_GPS_POSITION_MODE_MS_BASED, 2:LOC_GPS_POSITION_MODE_MS_ASSISTED Defaults: %d ", sOptions.positionMode);
     gardenPrint("    -i:  Interval. Takes an argument. Time in milliseconds between fixes Defaults: %d", sOptions.interval);
     gardenPrint("    -a:  Accuracy. Takes an argument. Accuracy in meters Defaults: %d ", sOptions.accuracy);
     gardenPrint("    -x:  Response Time. Takes an argument. Requested time to first fix in milliseconds Defaults: %d" , sOptions.responseTime);
     gardenPrint("    -P:  Inject Position. Takes 3 arguments seperated by a COMMA. Latitude, Longitude, and accuracy Defaults: %f,%f,%d ",sOptions.location.latitude,sOptions.location.longitude,(int)sOptions.location.accuracy);
     gardenPrint("    -N:  Network Initiated. Takes 2 arguments separated by COMMA. First being the number of back to back NI tests and second being a COMMA separated pattern of  1:Accept, 2:Deny,or 3:No Response Defaults: %d:%d",sOptions.niCount,0);
     for(int i = 0; i<sOptions.niResPatCount; ++i) { gardenPrint("%12d",sOptions.networkInitiatedResponse[i]); }
     gardenPrint("    -g:  Test Ril Interface. Takes an argument 0 or 1. Defaults: %d",sOptions.rilInterface);
     gardenPrint("    -c:  gps.conf file. Takes an argument. The argument is the path to gps.conf file. Defaults: %s",sOptions.gpsConfPath);
     gardenPrint("    -n:  Use this option to print nmea string, timestamp and length. Takes no arguments. Defaults:%d",sOptions.printNmea);
     gardenPrint("    -y:  Use this option to print detailed info on satellites in view. Defaults:%d",sOptions.satelliteDetails);
     gardenPrint("    -o:  This option, when used, will enforce a check to determine if time to first fix is within a given threshold value. Takes one argument, the threshold value in seconds. Defaults: %d",sOptions.fixThresholdInSecs);
     gardenPrint("    -A:  Minimum number of SVs seen in combination with -B option to determine when to stop the test without actually getting a position report to save test time");
     gardenPrint("    -B:  Minimum SNR for each SV seen in -A option to determine when to stop the test  without actually getting a position report to save test time");
#ifdef TEST_ULP
     gardenPrint("    -T:  Start a tracking session *WARNING* this tracking session will run until process exit. Ctrl-C to exit");
     gardenPrint("    -Z:  ZPP Test case number 1 to 5 - Default: 4");
#endif
     gardenPrint("    -K:  Keep garden-app alive. Press Ctrl-C to exit.");
     gardenPrint("    -L:  Load additional cases from dynamic library. e.g: -L \"example.so -l -t\"");
     gardenPrint("    -G:  Load Geofence command from a file. e.g: -R tombestone_07");
     gardenPrint("    -h:  print this help");
}

static GARDEN_RESULT parse_args(int argc, char *argv[]) {
    GARDEN_RESULT ret = GARDEN_RESULT_PASSED;
    int opt;
    extern char *optarg;
    char *argPtr;
    char *tokenPtr;

    string name;
    string args;
    string s;
    string::size_type p;
    GardenLib gardenLib;

    // Initialize gps conf path
    strlcpy(sOptions.gpsConfPath, LOC_PATH_GPS_CONF, sizeof(sOptions.gpsConfPath));
    sOptions.geofenceInput[0] = 0;
    // Handle '-c' option first, and the other options take precedence over it
    if (argc > 1) {
        for (int i=0; i<argc-1; ++i) {
            if (strcmp(argv[i], "-c") == 0) {
                readConfigFile(argv[i+1]);
                break;
            }
        }
    }

    while ((opt = getopt(argc, argv,
                    "r:l:t:u:hR:s:d:m:i:a:x:P:N:g:D:w:c:nye:o:z:A:B:T::Z:F:L:G:K")) != -1) {
        switch (opt) {
        case 'r':
            sOptions.r = atoi(optarg);
            gardenPrint("Recurrence:%d",sOptions.r);
            break;
        case 'l':
            sOptions.l = atoi(optarg);
            gardenPrint("Number of Sessions to loop through:%d",sOptions.l);
            break;
        case 't':
            sOptions.t = atoi(optarg);
            gardenPrint("User defined length of time to issue stop navigation:%d",sOptions.t);
            break;
        case 'R':
            sOptions.remoteAPI = atoi(optarg);
            gardenPrint("Use remote Api: %d",sOptions.remoteAPI);
            break;
#ifdef TEST_ULP
        case 'u':
            sOptions.ulpTestCaseNumber = atoi(optarg);
            gardenPrint("ulptestCase number: %d \n",sOptions.ulpTestCaseNumber);
            break;
#endif
        case 's':
            sOptions.s = atoi(optarg);
            gardenPrint("Stacks to test:%d",sOptions.s);
            break;
        case 'd':
            sOptions.deleteAidingDataMask = strtoll(optarg,NULL,16);
            gardenPrint("Delete Aiding Mask:%x",sOptions.deleteAidingDataMask);
            break;
        case 'm':
            sOptions.positionMode = atoi(optarg);
            gardenPrint("Position Mode:%d",sOptions.positionMode);
            break;
        case 'i':
            sOptions.interval = atoi(optarg);
            gardenPrint("Interval:%d",sOptions.interval);
            break;
        case 'a':
            sOptions.accuracy = atoi(optarg);
            gardenPrint("Accuracy:%d",sOptions.accuracy);
            break;
        case 'x':
            sOptions.responseTime = atoi(optarg);
            gardenPrint("Response Time:%d",sOptions.responseTime);
            break;
        case 'P':
            sOptions.location.flags = 0x0011;
            tokenPtr = strtok_r(optarg, ",", &argPtr);
            if(tokenPtr != NULL) {
                sOptions.location.latitude = atof(tokenPtr);
                tokenPtr = strtok_r(NULL, ",", &argPtr);
                if(tokenPtr != NULL) {
                    sOptions.location.longitude = atof(tokenPtr);
                    tokenPtr = strtok_r(NULL, ",", &argPtr);
                    if(tokenPtr != NULL) {
                        sOptions.location.accuracy = atoi(tokenPtr);
                    }
                }
            }
            gardenPrint("Inject Position:: flags:%x, lat:%f, lon:%f, acc:%f",
                    sOptions.location.flags, sOptions.location.latitude,
                    sOptions.location.longitude,sOptions.location.accuracy);
            break;
        case 'N':
            // Number of back to back tests
            tokenPtr = strtok_r(optarg, ",", &argPtr);
            if(tokenPtr != NULL) {
                sOptions.niCount = atoi(tokenPtr);
                if(sOptions.niCount > 0)
                {
                   char *ret;
                   while((ret = strtok_r(NULL, ",", &argPtr)) != NULL)
                   {
                      sOptions.networkInitiatedResponse[sOptions.niResPatCount++] = atoi(ret);
                   }
                }
            }
            gardenPrint("Number of back to back NI tests : %d",sOptions.niCount);
            break;
        case 'g':
            sOptions.rilInterface = atoi(optarg);
            gardenPrint("Test Ril Interface:%d", sOptions.rilInterface);
            break;
        case 'c': // '-c' is handled above, and just ignore it here to avoid override other options
            break;
        case 'n':
            sOptions.printNmea = 1;
            gardenPrint("Print NMEA info:%d",sOptions.printNmea);
            break;
        case 'y':
            sOptions.satelliteDetails = 1;
            gardenPrint("Print Details Satellites in View info:%d",sOptions.satelliteDetails);
            break;
        case 'o':
            sOptions.fixThresholdInSecs = atoi(optarg);
            gardenPrint("Time to first fix threshold value in seconds : %d",
                    sOptions.fixThresholdInSecs);
            break;
#ifdef TEST_ULP
        case 'z':
            sOptions.zppTestCaseNumber = atoi(optarg);
            gardenPrint("ZPP testCase: %d", sOptions.zppTestCaseNumber);
            break;
#endif
        case 'A':
            sOptions.stopOnMinSvs = atoi(optarg);
            gardenPrint("Stop on Minimum Svs: %d", sOptions.stopOnMinSvs);
            break;
        case 'B':
            sOptions.stopOnMinSnr = atof(optarg);
            gardenPrint("Stop on Minimum SNR: %f", sOptions.stopOnMinSnr);
            break;
#ifdef TEST_ULP
        case 'T':
            sOptions.tracking = 1;
            gardenPrint("%s%s",
                    "Start Tracking Session -- ",
                    "continuous untill processs is alive press Ctrl-C to exit");
            break;
        case 'Z':
            sOptions.zppTestCaseNumber =  atoi(optarg);
            gardenPrint("ZPP Test number: %d", sOptions.zppTestCaseNumber);
            break;
#endif
        case 'F':
            sOptions.trackUsingFlpInSecs = atoi(optarg);
            if (0 == sOptions.trackUsingFlpInSecs) {
                // no value passed via -F option
                sOptions.trackUsingFlpInSecs = 60;
            }
            gardenPrint("HW_FLP timeout: %d", sOptions.trackUsingFlpInSecs);
            break;
        case 'K':
            sOptions.keepAlive = true;
            gardenPrint("Keep garden_app alive. Press Ctrl-C to exit");
            break;
        case 'L':
            strlcpy(sOptions.gpsConfPath, optarg, 256);
            gardenLib.name = optarg;
            gardenLib.args = "";;
            s = optarg;
            p = s.find(" ");
            if (string::npos != p) {
                gardenLib.name = s.substr(0, p);
                gardenLib.args = optarg + p + 1;
            }
            sLibs.push_back(gardenLib);
            gardenPrint("Additional lib: %s: %s", gardenLib.name.c_str(), gardenLib.args.c_str());
            break;
        case 'G':
            strlcpy(sOptions.geofenceInput, optarg, 256);
            gardenPrint("Geofence input file: %s", sOptions.geofenceInput);
            break;
        case 'h':
            printHelp(argv);
            // abort when we meet -h option
            return GARDEN_RESULT_ABORT;
        default:
            printHelp(argv);
            GARDEN_ERROR("Invalid option: %c", opt);
        }
    }

    return GARDEN_RESULT_PASSED;
}

static GARDEN_RESULT autoTest() {
    GARDEN_RESULT ret = GARDEN_RESULT_PASSED;
    GARDEN_ADD(IGardenCase, "Setup",
            {
                GnssCase::setAPIMode(sOptions.remoteAPI);
                GnssCase::setSvInfo(sOptions.printNmea, sOptions.satelliteDetails);
            });

    for (auto gardenLib : sLibs) {
        GARDEN_ADD_PLUGIN(gardenLib.name, gardenLib.args);
    }

    if (sOptions.deleteAidingDataMask != 0) {
        GARDEN_ADD(GnssCase, "DeleteAdingData",
            {
                GnssCase* gnssCase = (GnssCase*)icase;
                IGnssAPI* api = gnssCase->getGnssAPI();
                GARDEN_ASSERT_NEQ(api, nullptr, "GnssAPI is nullptr.");
                api->gnssDeleteAidingData(sOptions.deleteAidingDataMask);
            });
    }

    if (sOptions.location.flags != 0) {
        GARDEN_ADD(GnssCase, "InjectLocation",
            {
                GnssCase* gnssCase = (GnssCase*)icase;
                IGnssAPI* api = gnssCase->getGnssAPI();
                GARDEN_ASSERT_NEQ(api, nullptr, "GnssAPI is nullptr.");
                api->gnssInjectLocation(sOptions.location.latitude,
                    sOptions.location.longitude,
                    sOptions.location.accuracy);
            });
    }
#ifdef TEST_ULP
    if (sOptions.ulpTestCaseNumber > 0) {
        stringstream ss;
        ss.str("");
        ss << sOptions.ulpTestCaseNumber;
        GARDEN_ADD_PLUGIN(ULPLIB, ss.str());
    }
    if (sOptions.zppTestCaseNumber > 0) {
        stringstream ss;
        ss.str("");
        ss << sOptions.zppTestCaseNumber;
        GARDEN_ADD_PLUGIN(ZPPLIB, ss.str());
    }
#endif

    for(int i = 0; i < sOptions.l; i++) {
        GARDEN_ADD(GnssCase, "SessionLoop",
            {
                gardenPrint("Session %d:", i);
                GnssCase* gnssCase = (GnssCase*)icase;
                IGnssAPI* api = gnssCase->getGnssAPI();
                GARDEN_ASSERT_NEQ(api, nullptr, "GnssAPI is nullptr.");

                int rc = 0;
                // set SUPL_MODE based on position mode.
                // For MSB(1) set 0x1,For MSA(2) 0x3,For standalone(0) 0x0.
                char config_data[20];
                // default to LOC_GPS_POSITION_MODE_STANDALONE
                int supl_mode = 0;
                if (sOptions.positionMode == LOC_GPS_POSITION_MODE_MS_BASED) {
                    supl_mode = 1;
                } else if (sOptions.positionMode == LOC_GPS_POSITION_MODE_MS_ASSISTED) {
                    supl_mode = 3;
                }

                snprintf(config_data, sizeof(config_data), "%s = %x", "SUPL_MODE", supl_mode);
                gardenPrint ("invoke gnss configuration_update: %s", config_data);
                api->configurationUpdate(config_data, strlen(config_data));

                rc = api->gnssSetPositionMode(sOptions.positionMode,
                        sOptions.r, sOptions.interval,
                        sOptions.accuracy,sOptions.responseTime);
                gardenPrint("set_position_mode returned %d", rc);

                if (sOptions.r == LOC_GPS_POSITION_RECURRENCE_SINGLE) {
                    gardenPrint ("Waiting for location callback...");
                    gnssCase->setStartParam(sOptions.t,
                            sOptions.stopOnMinSvs, sOptions.stopOnMinSnr, sOptions.r);
                } else {
                    gnssCase->setStartParam(sOptions.t);
                }
                rc = gnssCase->gnssStart();
                gardenPrint("start GPS interface returned %d", rc);

#ifdef USE_GLIB
                //In LE, we have to push the phone settings after first start fix request
                if(i == 0){
                    gnssCase->sendMsg((LocMsg*)GARDEN_GET_PHONE_CONTEXT_MSG());
                }
#endif
                gardenPrint("waiting for timeout or stop condition is met...");
                gnssCase->wait();

                if(sOptions.rilInterface) {
                    api->updateNetworkAvailability(88, "MY_APN_FOR_RIL_TEST");
                    gardenPrint ("Ril Interface test Done...");
                }

                rc = gnssCase->gnssStop();
                gardenPrint ("stop GPS interface returned %d", rc);
                GARDEN_ASSERT_EQ(rc, 0, "gnssStop failed");
            });

    }

    if (sOptions.niCount > 0) {
        GARDEN_ADD(GnssCase, "NI BackToBack",
                ((GnssCase*)icase)->niBackToBack(sOptions.niCount,
                    sOptions.niResPatCount,
                    sOptions.networkInitiatedResponse);
                );
    }

    if (0 != sOptions.trackUsingFlpInSecs) {
        GARDEN_ADD(FlpCase, "Flp",
            {
                int rc = 0;
                FlpCase* flpCase = (FlpCase*)icase;
                LocationOptions options;
                options.size = sizeof(LocationOptions);
                options.minInterval = sOptions.interval;
                options.minDistance = 0;
                options.mode = GNSS_SUPL_MODE_STANDALONE;
                flpCase->startFlpSession(SESSION_MODE_ON_FULL, options,
                    sOptions.trackUsingFlpInSecs);
            });
    }

    if (strlen(sOptions.geofenceInput) > 0) {
        GARDEN_ADD(GeofenceCase, "Load command from file",
            {
                GeofenceCase* geofenceCase = (GeofenceCase*)icase;
                geofenceCase->loadFromFileCase(sOptions.geofenceInput);
            });
    }

    if (sOptions.keepAlive) {
        GARDEN_ADD(IGardenCase, "Garden App keep alive",
            {
                gardenPrint("Garden App is alive. Hit Ctrl -C to exit");
                sleep(3600);
            });
    }

#if (defined __ANDROID__) && (defined TEST_ULP)
    /*********
     *
     *   ## IMPORTANT ##
     *   This tracking option will start a tracking session in continous loop
     *   Always keep this as the last test case and add new test cases before this one
     *
     ******* */
    if (sOptions.tracking == 1) {
        GARDEN_ADD_PLUGIN(TRACKINGLIB, "");
    }
#endif //__ANDROID__

    if (sOptions.fixThresholdInSecs > 0) {
        GARDEN_ADD(GnssCase, "TTFF Threshold",
            {
                GnssCase* gnssCase = (GnssCase*)icase;
                GARDEN_ASSERT_NEQ(gnssCase->getTTFF(), 0, "no TTFF");
                GARDEN_ASSERT_NEQ(gnssCase->getTTFF() > (uint64_t)sOptions.fixThresholdInSecs, true,
                    "TTFF Threshold failed");
            });
    }

    GARDEN_ADD(GnssCase, "Cleanup", ((GnssCase*)icase)->destroyGnssAPI(););

    ret = GARDEN_RUN();

    GARDEN_DESTROY();

    sLibs.clear();

    return ret;
}

static GARDEN_RESULT menuTest() {
    GARDEN_RESULT ret = GARDEN_RESULT_PASSED;
    char buf[16], *p;
    bool exit_loop = false;

    // select api
    menuPrint("Use remote API? (y/n) (default n) :");
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        GARDEN_ERROR("Error: fgets returned nullptr !!");
    }
    if (p[0] == 'y') {
        GARDEN_ADD(IGardenCase, "API Mode",
            {
                GnssCase::setAPIMode(true);
            });
    }

    while (!exit_loop) {
        bool invalid = false;
        usleep(100000);
        menuPrint("\n\n"
                "1: Gnss Verify Menu\n"
                "2: Gnss Positioning Menu\n"
                "3: Flp Batching Menu\n"
                "4: Geofence Menu\n"
                "5: Geofence Stress: Random\n"
                "6: Geofence Stress: Add Remove\n"
                "7: Gnss Multi Client Menu\n"
                "q: quit\n\n"
                "Enter Command:");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            GARDEN_ERROR("Error: fgets returned nullptr !!");
        }

        switch (p[0]) {
        case '1':
            GARDEN_ADD(GnssCase, "Gnss Verify Menu",
                {
                    GnssCase* gnssCase = (GnssCase*)icase;
                    gnssCase->verifyMenuCase();
                });
            break;
        case '2':
            GARDEN_ADD(GnssCase, "Gnss Positioning Menu",
                {
                    GnssCase* gnssCase = (GnssCase*)icase;
                    gnssCase->positioningMenuCase();
                });
            break;
        case '3':
            GARDEN_ADD(FlpCase, "Flp Batching Menu",
                {
                    FlpCase* flpCase = (FlpCase*)icase;
                    flpCase->menuCase();
                });
            break;
        case '4':
            GARDEN_ADD(GeofenceCase, "Geofence Menu",
                {
                    GeofenceCase* geofenceCase = (GeofenceCase*)icase;
                    geofenceCase->menuCase();
                });
            break;
        case '5':
            GARDEN_ADD(GeofenceCase, "Geofence Stress: Random",
                {
                    GeofenceCase* geofenceCase = (GeofenceCase*)icase;
                    geofenceCase->stressRandomCase();
                });
            break;
        case '6':
            GARDEN_ADD(GeofenceCase, "Geofence Stress: Add Remove",
                {
                    GeofenceCase* geofenceCase = (GeofenceCase*)icase;
                    geofenceCase->stressAddRemoveCase();
                });
            break;
        case '7':
            GARDEN_ADD(GnssMultiClientCase, "Gnss Multi Client Menu",
                {
                    GnssMultiClientCase* gnssMultiClientCase = (GnssMultiClientCase*)icase;
                    gnssMultiClientCase->menuTest();
                });
            break;
        case 'q':
            exit_loop = true;
            break;
        default:
            invalid = true;
            menuPrint("\ninvalid command\n");
        }

        if (!exit_loop && !invalid) {
            ret = GARDEN_RUN();
            GARDEN_DESTROY();
        }
    }
    return ret;
}

int main(int argc, char *argv[]) {
    GARDEN_RESULT ret = parse_args(argc, argv);
    if (ret != GARDEN_RESULT_PASSED) {
        if (ret == GARDEN_RESULT_ABORT) {
            // return GARDEN_RESULT_PASSED for -h option
            ret = GARDEN_RESULT_PASSED;
        }
        return ret;
    }

    if (argc == 1) {
        gardenPrint("Optional command line arguments to start %s without menu:", argv[0]);
        printHelp(argv);
        ret = menuTest();
    } else {
        ret = autoTest();
    }

    return ret;
}

