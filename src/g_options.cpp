/*
  CALICO
 
  Advanced game configuration options

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

#include "elib/elib.h"
#include "elib/configfile.h"
#include "g_options.h"

extern "C" {

struct gamesettings_t gGameSettings; // current settings

// defaults in config file
struct gamesettings_t gDefaultSettings = 
{
    1, // autorun
};

// settings for running the built-in demos
struct gamesettings_t gDemoSettings =
{
    0, // autorun
};

void G_OptionsNewGame(void)
{
    // restore defaults from config
    gGameSettings = gDefaultSettings;
}

void G_OptionsStartDemo(void)
{
    // set the proper settings for demo playback
    gGameSettings = gDemoSettings;
}

// other settings (not sync-critical)
int g_allowexit = 1;

} // end extern "C"

static cfgrange_t<int> boolRange = { 0, 1 };

static CfgItem cfgAutorun   { "g_autorun",   &gDefaultSettings.autorun, &boolRange };
static CfgItem cfgAllowExit { "g_allowexit", &g_allowexit,              &boolRange };

// EOF

