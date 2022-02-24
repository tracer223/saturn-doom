/*
  CALICO
  
  SDL 2 HAL setup
  
  The MIT License (MIT)
  
  Copyright (c) 2016 James Haley
  
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

#ifdef USE_YAUL

#include "yaul.h"
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_sfx.h"
#include "../hal/hal_timer.h"
#include "../hal/hal_video.h"
#include "yaul_init.h"
#include "yaul_input.h"
#include "yaul_sound.h"
#include "yaul_timer.h"
#include "yaul_video.h"

//
// Initialize the HAL layer with YAUL implementation function pointers
// Might be useful to write YAUL constants that correspond to different YAUL functionality,
// This is the pre-existing code but replacing SDL with YAUL

void YAUL_InitHAL(void)
{
   // Basic interface
   hal_medialayer.init              = YAUL_Init;
   hal_medialayer.exit              = YAUL_Exit;
   hal_medialayer.error             = YAUL_Error;
   hal_medialayer.msgbox            = YAUL_MsgBox;
   hal_medialayer.isExiting         = YAUL_IsExiting;
   hal_medialayer.getBaseDirectory  = YAUL_GetBaseDirectory;
   hal_medialayer.getWriteDirectory = YAUL_GetWriteDirectory;

   // Video functions
   hal_video.initVideo            = YAUL_InitVideo;
   hal_video.setNewVideoMode      = YAUL_SetNewVideoMode;
   hal_video.getWindowSize        = YAUL_GetWindowSize;
   hal_video.getGLProcAddress     = YAUL_GetGLProcAddress;
   hal_video.transformFBCoord     = YAUL_TransformFBCoord;
   hal_video.transformGameCoord2i = YAUL_TransformGameCoord2i;
   hal_video.transformGameCoord2f = YAUL_TransformGameCoord2f;
   hal_video.transformWidth       = YAUL_TransformWidth;
   hal_video.transformHeight      = YAUL_TransformHeight;
   hal_video.toggleGLSwap         = YAUL_ToggleGLSwap;
   hal_video.endFrame             = YAUL_EndFrame;
   hal_video.isFullScreen         = YAUL_IsFullScreen;
   hal_video.getCurrentDisplay    = YAUL_GetCurrentDisplay;
   hal_video.getWindowFlags       = YAUL_GetWindowFlags;
   hal_video.setGrab              = YAUL_SetGrab;
   hal_video.warpMouse            = YAUL_WarpMouse;
   hal_video.getWindowHandle      = YAUL_GetWindowHandle;
   hal_video.getAspectRatioType   = YAUL_GetAspectRatioType;
   hal_video.getSubscreenExtents  = YAUL_GetSubscreenExtents;

   // App state maintenance
   hal_appstate.mouseShouldBeGrabbed = YAUL_MouseShouldBeGrabbed;
   hal_appstate.updateGrab           = YAUL_UpdateGrab;
   hal_appstate.updateFocus          = YAUL_UpdateFocus;
   hal_appstate.setGrabState         = YAUL_SetGrabState;

   // Input
   hal_input.initInput      = YAUL_InitInput;
   hal_input.getEvents      = YAUL_GetEvents;
   hal_input.resetInput     = YAUL_ResetInput;
   hal_input.getMouseMotion = YAUL_GetMouseMotion;

   // Sound
   hal_sound.initSound       = YAULSfx_MixerInit;
   hal_sound.isInit          = YAULSfx_IsInit;
   hal_sound.startSound      = YAULSfx_StartSound;
   hal_sound.stopSound       = YAULSfx_StopSound;
   hal_sound.isSamplePlaying = YAULSfx_IsSamplePlaying;
   hal_sound.isSampleAtStart = YAULSfx_IsSampleAtStart;
   hal_sound.stopAllChannels = YAULSfx_StopAllChannels;
   hal_sound.updateEQParams  = YAULSfx_UpdateEQParams;
   hal_sound.getSampleRate   = YAULSfx_GetSampleRate;

   // Timer
   hal_timer.delay     = YAUL_Delay;
   hal_timer.getTime   = YAUL_GetTime;
   hal_timer.getTimeMS = YAUL_GetTimeMS;
}

#endif

// EOF

