/*
  CALICO
  
  SDL 2 Video
  
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

#ifndef YAUL_VIDEO_H__
#define YAUL_VIDEO_H__

#ifdef USE_YAUL

#include "../hal/hal_types.h"
#include "../hal/hal_video.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SDL_Window *mainwindow;

void          YAUL_GetWindowSize(int *width, int *height);
void         *YAUL_GetGLProcAddress(const char *proc);
void          YAUL_InitVideo(void);
hal_bool      YAUL_SetNewVideoMode(int w, int h, int fs, int mnum);
void          YAUL_TransformFBCoord(int x, int y, int *tx, int *ty);
void          YAUL_TransformGameCoord2i(int x, int y, int *tx, int *ty);
void          YAUL_TransformGameCoord2f(int x, int y, float *tx, float *ty);
unsigned int  YAUL_TransformWidth(unsigned int w);
unsigned int  YAUL_TransformHeight(unsigned int h);
int           YAUL_ToggleGLSwap(hal_bool swap);
int           YAUL_IsFullScreen(void);
int           YAUL_GetCurrentDisplay(void);
unsigned int  YAUL_GetWindowFlags(void);
void          YAUL_SetGrab(hal_bool grab);
void          YAUL_WarpMouse(int x, int y);
void          YAUL_EndFrame(void);
void         *YAUL_GetWindowHandle(void);
hal_aspect_t  YAUL_GetAspectRatioType(void);
void          YAUL_GetSubscreenExtents(int *x, int *y, int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF


