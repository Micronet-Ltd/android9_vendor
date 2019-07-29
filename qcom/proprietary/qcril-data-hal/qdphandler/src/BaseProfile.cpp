/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>

#include "framework/Log.h"

#include "BaseProfile.h"
#include "qdp_adapter.h"
#include "qdp.h"

unsigned int BaseProfile::lookup() const
{
   Log::getInstance().d(mName + "Not yet implemented");
   return INVALID_PROFILE_ID;
}

unsigned int BaseProfile::findMatch(const std::list<uint16_t>& profile_list) const
{
  (void) profile_list;
  Log::getInstance().d(mName + "Not yet implemented");
  return INVALID_PROFILE_ID;
}

//TO BE ENABLED LATER
/*bool BaseProfile::matchHomeProtocolStrict(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchHomeProtocolLoose(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchRoamProtocolStrict(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}

bool BaseProfile::matchRoamProtocolLoose(std::string other) const
{
  (void) other;
  Log::getInstance().d(mName + "Not yet implemented");
  return false;
}*/