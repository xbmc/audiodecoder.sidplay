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

#include <kodi/addon-instance/AudioDecoder.h>
#include <kodi/Filesystem.h>

#include "sidplay/sidplay2.h"
#include "sidplay/SidTune.h"
#include "sidplay/builders/resid.h"

class ATTRIBUTE_HIDDEN CSIDCodec : public kodi::addon::CInstanceAudioDecoder
{
public:
  CSIDCodec(KODI_HANDLE instance) :
    CInstanceAudioDecoder(instance) {}

  virtual ~CSIDCodec()
  {
    if (tune)
      delete tune;
  }

  virtual bool Init(const std::string& filename, unsigned int filecache,
                    int& channels, int& samplerate,
                    int& bitspersample, int64_t& totaltime,
                    int& bitrate, AEDataFormat& format,
                    std::vector<AEChannel>& channellist) override
  {
    int track=1;
    std::string toLoad(filename);
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

    kodi::vfs::CFile file;
    if (!file.OpenFile(toLoad.c_str(), 0))
      return false;

    int len = file.GetLength();
    uint8_t* data = new uint8_t[len];
    file.Read(data, len);
    file.Close();

    // Now load the module
    tune = new SidTune(data, len);
    delete[] data;

    if (!tune)
      return false;

    tune->selectSong(track);
    player.load(tune);
    config.clockDefault = SID2_CLOCK_PAL;
    config.clockForced = false;
    config.clockSpeed = SID2_CLOCK_CORRECT;
    config.emulateStereo = false;
    config.environment = sid2_envR;
    config.forceDualSids = false;
    config.frequency = 48000;
    config.leftVolume = 255;
    config.optimisation = SID2_DEFAULT_OPTIMISATION;
    config.playback = sid2_mono;
    config.powerOnDelay = SID2_DEFAULT_POWER_ON_DELAY;
    config.precision = 16;
    config.rightVolume = 255;
    config.sampleFormat = SID2_LITTLE_SIGNED;
    ReSIDBuilder* rs = new ReSIDBuilder("Resid Builder");
    rs->create(player.info().maxsids);
    rs->filter(true);
    rs->sampling(48000);
    config.sidEmulation = rs;
    pos = 0;
    track = track;

    player.config(config);

    channels = 1;
    samplerate = 48000;
    bitspersample = 16;
    totaltime = 4*60*1000;
    format = AE_FMT_S16NE;
    channellist = { AE_CH_FC };
    bitrate = 0;

    return true;
  }

  virtual int ReadPCM(uint8_t* buffer, int size, int& actualsize) override
  {
    if ((actualsize = player.play(buffer, size)))
    {
      pos += actualsize;
      return 0;
    }

    return 1;
  }

  virtual int64_t Seek(int64_t time) override
  {
    uint8_t temp[3840*2];
    if (pos > time/1000*48000*2)
    {
      tune->selectSong(track);
      player.load(tune);
      player.config(config);
      pos = 0;
    }

    while (pos < time/1000*48000*2)
    {
      int64_t iRead = time/1000*48000*2-pos;
      if (iRead > 3840*2)
      {
        player.fastForward(32*100);
        iRead = 3840*2;
      }
      else
        player.fastForward(100);

      int dummy;
      ReadPCM(temp, int(iRead), dummy);
      if (!dummy)
        break; // get out of here
    }
    return time;
  }

  virtual int TrackCount(const std::string& fileName) override
  {
    kodi::vfs::CFile file;
    if (!file.OpenFile(fileName, 0))
      return 1;

    int len = file.GetLength();
    uint8_t* data = new uint8_t[len];
    file.Read(data, len);
    file.Close();

    SidTune tune(data, len);
    delete[] data;

    return tune.getInfo().songs;
  }

private:
  sidplay2 player;
  sid2_config_t config;
  SidTune* tune = nullptr;
  int64_t pos;
  int track;
};


class ATTRIBUTE_HIDDEN CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() { }
  virtual ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    addonInstance = new CSIDCodec(instance);
    return ADDON_STATUS_OK;
  }
  virtual ~CMyAddon()
  {
  }
};


ADDONCREATOR(CMyAddon);
