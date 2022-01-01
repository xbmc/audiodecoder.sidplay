/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "sidplay/SidTune.h"
#include "sidplay/builders/resid.h"
#include "sidplay/sidplay2.h"
#include "sidplay/utils/SidDatabase.h"

#include <kodi/addon-instance/AudioDecoder.h>

class ATTR_DLL_LOCAL CSIDCodec : public kodi::addon::CInstanceAudioDecoder
{
public:
  CSIDCodec(const kodi::addon::IInstanceInfo& instance);
  virtual ~CSIDCodec();

  bool Init(const std::string& filename,
            unsigned int filecache,
            int& channels,
            int& samplerate,
            int& bitspersample,
            int64_t& totaltime,
            int& bitrate,
            AudioEngineDataFormat& format,
            std::vector<AudioEngineChannel>& channellist) override;
  int ReadPCM(uint8_t* buffer, size_t size, size_t& actualsize) override;
  int64_t Seek(int64_t time) override;
  int TrackCount(const std::string& fileName) override;
  bool ReadTag(const std::string& filename, kodi::addon::AudioDecoderInfoTag& tag) override;

private:
  sidplay2 player;
  sid2_config_t config;
  SidTune* tune = nullptr;
  int64_t pos;
  int track;
};
