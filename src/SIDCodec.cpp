/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "kodi/libXBMC_addon.h"

#include "sidplay/sidplay2.h"
#include "sidplay/SidTune.h"
#include "sidplay/builders/resid.h"
#include "kodi_audiodec_dll.h"
#include "AEChannelData.h"

ADDON::CHelper_libXBMC_addon *XBMC           = NULL;

extern "C" {

//-- Create -------------------------------------------------------------------
// Called on load. Addon should fully initalize or return error status
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!XBMC)
    XBMC = new ADDON::CHelper_libXBMC_addon;

  if (!XBMC->RegisterMe(hdl))
  {
    delete XBMC, XBMC=NULL;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  return ADDON_STATUS_OK;
}

//-- Stop ---------------------------------------------------------------------
// This dll must cease all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Stop()
{
}

//-- Destroy ------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Destroy()
{
  XBMC=NULL;
}

//-- HasSettings --------------------------------------------------------------
// Returns true if this add-on use settings
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
bool ADDON_HasSettings()
{
  return false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

//-- GetSettings --------------------------------------------------------------
// Return the settings for XBMC to display
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

//-- FreeSettings --------------------------------------------------------------
// Free the settings struct passed from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------

void ADDON_FreeSettings()
{
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void* value)
{
  return ADDON_STATUS_OK;
}

//-- Announce -----------------------------------------------------------------
// Receive announcements from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

struct SIDContext
{
  sidplay2 player;
  sid2_config_t config;
  SidTune* tune;
  int64_t pos;
  int track;
};

void* Init(const char* strFile, unsigned int filecache, int* channels,
           int* samplerate, int* bitspersample, int64_t* totaltime,
           int* bitrate, AEDataFormat* format, const AEChannel** channelinfo)
{
  int track=1;
  std::string toLoad(strFile);
  if (toLoad.find(".sidstream") != std::string::npos)
  {
    size_t iStart=toLoad.rfind('-') + 1;
    track = atoi(toLoad.substr(iStart, toLoad.size()-iStart-10).c_str());
    //  The directory we are in, is the file
    //  that contains the bitstream to play,
    //  so extract it
    size_t slash = toLoad.rfind('\\');
    if (slash == std::string::npos)
      slash = toLoad.rfind('/');
    toLoad = toLoad.substr(0, slash);
  }

  void* file = XBMC->OpenFile(toLoad.c_str(), 0);
  if (!file)
    return NULL;

  int len = XBMC->GetFileLength(file);
  uint8_t* data = new uint8_t[len];
  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  SIDContext* result = new SIDContext;

  // Now load the module
  result->tune = new SidTune(data, len);
  delete[] data;

  if (!result->tune)
    return NULL;

  result->tune->selectSong(track);
  result->player.load(result->tune);
  result->config.clockDefault = SID2_CLOCK_PAL;
  result->config.clockForced = false;
  result->config.clockSpeed = SID2_CLOCK_CORRECT;
  result->config.emulateStereo = false;
  result->config.environment = sid2_envR;
  result->config.forceDualSids = false;
  result->config.frequency = 48000;
  result->config.leftVolume = 255;
  result->config.optimisation = SID2_DEFAULT_OPTIMISATION;
  result->config.playback = sid2_mono;
  result->config.powerOnDelay = SID2_DEFAULT_POWER_ON_DELAY;
  result->config.precision = 16;
  result->config.rightVolume = 255;
  result->config.sampleFormat = SID2_LITTLE_SIGNED;
  ReSIDBuilder* rs = new ReSIDBuilder("Resid Builder");
  rs->create (result->player.info().maxsids);
  rs->filter(true);
  rs->sampling(48000);
  result->config.sidEmulation = rs;
  result->pos = 0;
  result->track = track;

  result->player.config(result->config);

  *channels = 1;
  *samplerate = 48000;
  *bitspersample = 16;
  *totaltime = 4*60*1000;
  *format = AE_FMT_S16NE;

  static enum AEChannel map[1][2] = {
    {AE_CH_FC, AE_CH_NULL}
  };

  *channelinfo = map[0];
  *bitrate = 0;

  return result;
}

int ReadPCM(void* context, uint8_t* pBuffer, int size, int *actualsize)
{
  if (!context)
    return 1;

  SIDContext* ctx = (SIDContext*)context;

  if ((*actualsize = ctx->player.play(pBuffer, size)))
  {
    ctx->pos += *actualsize;
    return 0;
  }
  
  return 1;
}

int64_t Seek(void* context, int64_t time)
{
  if (!context)
    return 0;

  SIDContext* ctx = (SIDContext*)context;
  uint8_t temp[3840*2];
  if (ctx->pos > time/1000*48000*2)
  {
    ctx->tune->selectSong(ctx->track);
    ctx->player.load(ctx->tune);
    ctx->player.config(ctx->config);
    ctx->pos = 0;
  }

  while (ctx->pos < time/1000*48000*2)
  {
    int64_t iRead = time/1000*48000*2-ctx->pos;
    if (iRead > 3840*2)
    {
      ctx->player.fastForward(32*100);
      iRead = 3840*2;
    }
    else
      ctx->player.fastForward(100);

    int dummy;
    ReadPCM(ctx, temp, int(iRead), &dummy);
    iRead = dummy;
    if (!iRead)
      break; // get out of here
    if (iRead == 3840*2)
      ctx->pos += iRead*32;
    else ctx->pos += iRead;
  }
  return time;
}

bool DeInit(void* context)
{
  delete (SIDContext*)context;

  return true;
}

bool ReadTag(const char* strFile, char* title, char* artist,
             int* length)
{
  return true;
}

int TrackCount(const char* strFile)
{
  void* file = XBMC->OpenFile(strFile, 0);
  if (!file)
    return 1;

  int len = XBMC->GetFileLength(file);
  uint8_t* data = new uint8_t[len];
  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  SidTune tune(data, len);
  delete[] data;

  return tune.getInfo().songs;
}
}
