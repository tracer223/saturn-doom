/*
  CALICO
  
  YAUL 2 basic initialization
  
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

#include <stdlib.h>
#include "yaul.h"
#include "../elib/atexit.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../hal/hal_video.h"
#include "../yaul_video.h"

static hal_bool isExiting;

//
// Initialize the YAUL library
//
hal_bool YAUL_Init(void)
{
   if(YAUL_Init(YAUL_INIT_EVERYTHING) != 0)
      return HAL_FALSE;

   atexit(YAUL_Quit);
   return HAL_TRUE;
}

//
// Exit the program normally
//
void YAUL_Exit(void)
{
   if(isExiting)
      return;

   isExiting = HAL_TRUE;

   E_RunAtExitFuncs(0);

   exit(0);
}

//
// Display a message box; returns 0 on success, -1 on error
//
int YAUL_MsgBox(const char *title, const char *msg, hal_bool isError)
{
   Uint32 flags = isError ? YAUL_MESSAGEBOX_ERROR : YAUL_MESSAGEBOX_INFORMATION;
   return YAUL_ShowSimpleMessageBox(flags, title, msg, mainwindow);
}

//
// Exit due to an error
//
void YAUL_Error(void)
{
   if(isExiting)
      return;

   isExiting = HAL_TRUE;

   E_RunAtExitFuncs(1);

   exit(-1);
}

//
// Return whether the program is in an exiting state or not
//
hal_bool YAUL_IsExiting(void)
{
   return isExiting;
}

//
// Get application base directory
//
const char *YAUL_GetBaseDirectory(void)
{
    static char *pszPath = NULL;
    
    if(!pszPath)
    {
        pszPath = YAUL_GetBasePath();
        if(!pszPath)
            pszPath = "./"; // probably will not work
    }

    return pszPath;
}

//
// Get a suitable write location for the application
//
const char *YAUL_GetWriteDirectory(const char *app)
{
   static char *pszPath = NULL;

   if(!pszPath)
   {
      pszPath = YAUL_GetPrefPath("Team Eternity", app);
      if(!pszPath)
         pszPath = "./"; // probably will not work.
   }

   return pszPath;
}

#endif

// EOF


