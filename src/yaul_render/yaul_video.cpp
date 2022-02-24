/*
  CALICO
  
  YAUL 2 Video
  
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

#include "YAUL.h"
#include "YAUL_syswm.h"

#include "yaul_video.h"
#include "../elib/configfile.h"
#include "../hal/hal_types.h"
#include "../hal/hal_input.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../rb/rb_main.h"
#include "../rb/valloc.h"
#include "../renderintr/ri_interface.h"
#include "../gl/gl_render.h"
#include "../gl4/gl4_render.h"

//=============================================================================
//
// Globals
// 

// Main window object
YAUL_Window    *mainwindow;

// GL context
YAUL_GLContext  glcontext;

//=============================================================================
//
// Configuration options
//

enum renderer_e
{
    RENDERER_GL1_1, // GL 1.1 renderer
    RENDERER_GL4,   // GL 4 renderer

    RENDERER_MIN = RENDERER_GL1_1,
    RENDERER_MAX = RENDERER_GL4
};

static int screenwidth     = CALICO_ORIG_SCREENWIDTH;
static int screenheight    = CALICO_ORIG_SCREENHEIGHT;
static int fullscreen      = 0;
static int monitornum      = 0;
static hal_aspect_t aspect = HAL_ASPECT_NOMINAL;
static int aspectNum       = 4;
static int aspectDenom     = 3;
static int renderer        = RENDERER_GL4;

static cfgrange_t<int> swRange = { 320, 32768 };
static cfgrange_t<int> shRange = { 224, 32768 };
static cfgrange_t<int> fsRange = { -1,  1     };
static cfgrange_t<int> anRange = {  3,  100   };
static cfgrange_t<int> adRange = {  2,  100   };

static cfgrange_t<int> rnRange = { RENDERER_MIN, RENDERER_MAX };

static CfgItem cfgScreenWidth  { "screenwidth",  &screenwidth,  &swRange };
static CfgItem cfgScreenHeight { "screenheight", &screenheight, &shRange };
static CfgItem cfgFullScreen   { "fullscreen",   &fullscreen,   &fsRange };
static CfgItem cfgMonitorNum   { "monitornum",   &monitornum };
static CfgItem cfgAspectNum    { "aspectnum",    &aspectNum,    &anRange };
static CfgItem cfgAspectDenom  { "aspectdenom",  &aspectDenom,  &adRange };
static CfgItem cfgRenderer     { "renderer",     &renderer,     &rnRange };

//=============================================================================
//
// Current state
//

// window geometry
static int curscreenwidth;
static int curscreenheight;
static int curfullscreen;
static int curmonitornum;

// 4:3 subscreen
static rbScissor_t subscreen;

// scale factors
static float screenxscale;
static float screenyscale;
static float screenxiscale;
static float screenyiscale;

//=============================================================================
//
// Mode setting
//

//
// Select the proper renderer
//
static void YAUL_setRenderer()
{
    switch(renderer)
    {
    case RENDERER_GL1_1:
        GL_SelectRenderer();
        break;
    case RENDERER_GL4:
        GL4_SelectRenderer();
        break;
    default:
        hal_platform.fatalError("Unknown value for renderer (%d)", renderer);
    }
    g_renderer->InitRenderer(curscreenwidth, curscreenheight);
}

//
// Calculate the game area's subrect on the framebuffer
//
static void YAUL_calcSubRect()
{
   const rbfixed aspectRatio    = curscreenwidth * RBFRACUNIT / curscreenheight;
   const rbfixed nomAspectRatio = aspectNum * RBFRACUNIT / aspectDenom;

   if(aspectRatio == nomAspectRatio) // nominal
   {
      subscreen.width  = curscreenwidth;
      subscreen.height = curscreenheight;
      subscreen.x      = 0;
      subscreen.y      = 0;
      aspect           = HAL_ASPECT_NOMINAL;
   }
   else if(aspectRatio > nomAspectRatio) // widescreen
   {
      subscreen.width  = curscreenheight * aspectNum / aspectDenom;
      subscreen.height = curscreenheight;
      subscreen.x      = (curscreenwidth - subscreen.width) / 2;
      subscreen.y      = 0;
      aspect           = HAL_ASPECT_WIDE;
   }
   else // narrow
   {
      subscreen.width  = curscreenwidth;
      subscreen.height = curscreenwidth * aspectDenom / aspectNum;
      subscreen.x      = 0;
      subscreen.y      = (curscreenheight - subscreen.height) / 2;
      aspect           = HAL_ASPECT_NARROW;
   }

   screenxscale  = float(subscreen.width ) / CALICO_ORIG_SCREENWIDTH;
   screenyscale  = float(subscreen.height) / CALICO_ORIG_SCREENHEIGHT;
   screenxiscale = 1.0f / screenxscale;
   screenyiscale = 1.0f / screenyscale;
}

//
// Set video mode
//
hal_bool YAUL_SetVideoMode(int width, int height, int fs, int mnum)
{
   if(glcontext)
   {
      YAUL_GL_DeleteContext(glcontext);
      glcontext = nullptr;
   }
   if(mainwindow)
   {
      YAUL_DestroyWindow(mainwindow);
      mainwindow = nullptr;
   }

   Uint32 flags = YAUL_WINDOW_OPENGL | YAUL_WINDOW_INPUT_FOCUS | YAUL_WINDOW_MOUSE_FOCUS | YAUL_WINDOW_SHOWN;
   int    x     = YAUL_WINDOWPOS_CENTERED_DISPLAY(mnum);
   int    y     = YAUL_WINDOWPOS_CENTERED_DISPLAY(mnum);

   if(fs == 1)
      flags |= YAUL_WINDOW_FULLSCREEN;

   // if dimensions are zero, then use fullscreen desktop
   if(!(width && height) || fs == -1)
   {
      x = YAUL_WINDOWPOS_UNDEFINED_DISPLAY(mnum);
      y = YAUL_WINDOWPOS_UNDEFINED_DISPLAY(mnum);
      flags |= YAUL_WINDOW_FULLSCREEN_DESKTOP;
      fs = -1;
   }

   // set GL attributes
   YAUL_GL_SetAttribute(YAUL_GL_RED_SIZE,      8);
   YAUL_GL_SetAttribute(YAUL_GL_GREEN_SIZE,    8);
   YAUL_GL_SetAttribute(YAUL_GL_BLUE_SIZE,     8);
   YAUL_GL_SetAttribute(YAUL_GL_ALPHA_SIZE,    8);
   YAUL_GL_SetAttribute(YAUL_GL_BUFFER_SIZE,  32);
   YAUL_GL_SetAttribute(YAUL_GL_DOUBLEBUFFER,  1);

   if(renderer == RENDERER_GL4)
   {
       YAUL_GL_SetAttribute(YAUL_GL_CONTEXT_PROFILE_MASK, YAUL_GL_CONTEXT_PROFILE_CORE);
       YAUL_GL_SetAttribute(YAUL_GL_CONTEXT_MAJOR_VERSION, 4);
       YAUL_GL_SetAttribute(YAUL_GL_CONTEXT_MINOR_VERSION, 0);
   }

   if(!(mainwindow = YAUL_CreateWindow("Calico", x, y, width, height, flags)))
   {
      // Try resetting previous mode
      if(width != curscreenwidth || height != curscreenheight || fs != curfullscreen ||
         mnum != curmonitornum)
      {
         return YAUL_SetVideoMode(curscreenwidth, curscreenheight, curfullscreen, curmonitornum);
      }
      else
      {
         hal_platform.fatalError("Failed to set video mode %dx%d (%s)", width, height, 
                                 YAUL_GetError());
      }
   }

   // set window icon
   hal_platform.setIcon();

   // create GL context
   if(!(glcontext = YAUL_GL_CreateContext(mainwindow)))
   {
      hal_platform.fatalError("Failed to create GL context (%s)", YAUL_GetError());
   }

   // remember set state
   YAUL_GetWindowSize(mainwindow, &curscreenwidth, &curscreenheight);
   curfullscreen = fs;
   curmonitornum = mnum;

   // calculate game subscreen geometry
   YAUL_calcSubRect();

   // make current and set swap
   YAUL_GL_MakeCurrent(mainwindow, glcontext);
   YAUL_ToggleGLSwap(HAL_TRUE);

   // wake up RB system
   RB_InitDefaultState(renderer == RENDERER_GL4 ? 4 : 1);

   // update appstate maintenance
   hal_appstate.updateFocus();
   hal_appstate.updateGrab();

   return HAL_TRUE;
}

//
// Remember the current video mode in the configuration file
//
static void YAUL_saveVideoMode()
{
   screenwidth  = curscreenwidth;
   screenheight = curscreenheight;
   fullscreen   = curfullscreen;
   monitornum   = curmonitornum;
}

//
// Set initial video mode at startup using configuration file settings
//
void YAUL_InitVideo()
{
   YAUL_SetVideoMode(screenwidth, screenheight, fullscreen, monitornum);
   YAUL_saveVideoMode(); // remember settings

   // initialize renderer
   YAUL_setRenderer();
}

//
// Change video mode at runtime
//
hal_bool YAUL_SetNewVideoMode(int w, int h, int fs, int mnum)
{
   hal_bool res;

   if((res = YAUL_SetVideoMode(w, h, fs, mnum)))
      YAUL_saveVideoMode(); // remember settings

   // notify all registered game objects and subsystems of the change
   VAllocItem::ModeChanging();

   // initialize renderer
   YAUL_setRenderer();

   return res;
}

//=============================================================================
//
// Coordinate transforms
//

//
// Transform a framebuffer or client window coordinate into the game's 
// 640x480 subscreen space.
//
void YAUL_TransformFBCoord(int x, int y, int *tx, int *ty)
{
   *tx = int((x - subscreen.x) * screenxiscale);
   *ty = int((y - subscreen.y) * screenyiscale);
}

//
// Transform a coordinate in the game's 640x480 subscreen space into
// framebuffer space.
//
void YAUL_TransformGameCoord2i(int x, int y, int *tx, int *ty)
{
   *tx = int(x * screenxscale + subscreen.x);
   *ty = int(y * screenyscale + subscreen.y);
}

//
// Float version of TransformGameCoord
//
void YAUL_TransformGameCoord2f(int x, int y, float *tx, float *ty)
{
   *tx = x * screenxscale + subscreen.x;
   *ty = y * screenyscale + subscreen.y;
}

//
// Scale up a width value
//
unsigned int YAUL_TransformWidth(unsigned int w)
{
   return static_cast<unsigned int>(w * screenxscale);
}

//
// Scale up a height value
//
unsigned int YAUL_TransformHeight(unsigned int h)
{
   return static_cast<unsigned int>(h * screenyscale);
}

//=============================================================================
//
// State retrieval
//

//
// Get the size of the video window
//
void YAUL_GetWindowSize(int *width, int *height)
{
   if(mainwindow)
      YAUL_GetWindowSize(mainwindow, width, height);
}

//
// Check if application is running in fullscreen
//
int YAUL_IsFullScreen()
{
   return curfullscreen;
}

//
// Get current display number
//
int YAUL_GetCurrentDisplay()
{
   return curmonitornum;
}

//
// Get main window's state flags
//
unsigned int YAUL_GetWindowFlags()
{
   return (mainwindow ? YAUL_GetWindowFlags(mainwindow) : 0);
}

//
// Toggle mouse pointer grabbing
//
void YAUL_SetGrab(hal_bool grab)
{
   if(mainwindow)
      YAUL_SetWindowGrab(mainwindow, grab ? YAUL_TRUE : YAUL_FALSE);
}

//
// Warp mouse
//
void YAUL_WarpMouse(int x, int y)
{
   if(mainwindow)
      YAUL_WarpMouseInWindow(mainwindow, x, y);
}

//
// Get a GL function pointer
//
void *YAUL_GetGLProcAddress(const char *proc)
{
   return YAUL_GL_GetProcAddress(proc);
}

//
// Toggle swapping
//
int YAUL_ToggleGLSwap(hal_bool swap)
{
   static hal_bool swapState;

   if(swap == swapState)
      return 0;
   swapState = swap;

   return YAUL_GL_SetSwapInterval(!!swap);
}

//
// End frame and swap buffers
//
void YAUL_EndFrame()
{
   if(mainwindow)
      YAUL_GL_SwapWindow(mainwindow);
}

//
// Get platform-specific window handle
//
void *YAUL_GetWindowHandle()
{
#ifdef _WIN32
   YAUL_SysWMinfo info;
   YAUL_VERSION(&info.version);

   if(mainwindow && YAUL_GetWindowWMInfo(mainwindow, &info))
      return info.info.win.window;
#endif

   return nullptr;
}

//
// Get aspect ratio type
//
hal_aspect_t YAUL_GetAspectRatioType()
{
   return aspect;
}

//
// Get location and size of subscreen.
//
void YAUL_GetSubscreenExtents(int *x, int *y, int *w, int *h)
{
   if(x)
      *x = subscreen.x;
   if(y)
      *y = subscreen.y;
   if(w)
      *w = subscreen.width;
   if(h)
      *h = subscreen.height;
}

#endif

// EOF

