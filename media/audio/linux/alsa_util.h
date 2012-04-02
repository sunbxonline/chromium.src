// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_LINUX_ALSA_UTIL_H_
#define MEDIA_AUDIO_LINUX_ALSA_UTIL_H_

#include <alsa/asoundlib.h>
#include <string>

class AlsaWrapper;

namespace alsa_util {

snd_pcm_format_t BitsToFormat(int bits_per_sample);

snd_pcm_t* OpenCaptureDevice(AlsaWrapper* wrapper,
                             const char* device_name,
                             int channels,
                             int sample_rate,
                             snd_pcm_format_t pcm_format,
                             int latency_us);

snd_pcm_t* OpenPlaybackDevice(AlsaWrapper* wrapper,
                              const char* device_name,
                              int channels,
                              int sample_rate,
                              snd_pcm_format_t pcm_format,
                              int latency_us);

int CloseDevice(AlsaWrapper* wrapper, snd_pcm_t* handle);

snd_mixer_t* OpenMixer(AlsaWrapper* wrapper, const std::string& device_name);

void CloseMixer(AlsaWrapper* wrapper,
                snd_mixer_t* mixer,
                const std::string& device_name);

snd_mixer_elem_t* LoadCaptureMixerElement(AlsaWrapper* wrapper,
                                          snd_mixer_t* mixer);

}

#endif  // MEDIA_AUDIO_LINUX_ALSA_UTIL_H_
