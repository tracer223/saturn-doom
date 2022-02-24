/*
  CALICO
  
  YAUL Audio Implementation
  
  The MIT License (MIT)
  
  Copyright (c) 2017 James Haley
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef YAUL_SOUND_H__
#define YAUL_SOUND_H__

#ifdef USE_YAUL

#include "../hal/hal_types.h"

#define SAMPLERATE 44100

#ifdef __cplusplus
extern "C" {
#endif

int      YAULSfx_StartSound(float *data, size_t numsamples, int volume, hal_bool loop);
void     YAULSfx_StopSound(int handle);
hal_bool YAULSfx_IsSamplePlaying(int handle);
hal_bool YAULSfx_IsSampleAtStart(int handle);
void     YAULSfx_StopAllChannels(void);
void     YAULSfx_UpdateEQParams(void);
hal_bool YAULSfx_IsInit(void);
hal_bool YAULSfx_MixerInit(void);
int      YAULSfx_GetSampleRate(void);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF

