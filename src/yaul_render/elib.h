/*
  CALICO
  
  ELib
  
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

#ifndef ELIB_H__
#define ELIB_H__

// C++ standard library
#if defined(__cplusplus)
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
// ctypes replacement
#include "m_ctype.h"
#else
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#include <stddef.h>
#include <stdint.h>

// Memory handling
#include "zone.h"

#if defined(__cplusplus)
// Smart pointer types
#include "esmartptr.h"
#endif

// Global config
#define ELIB_APPNAME "Calico"

// String case comparison
#if defined(_MSC_VER) && !defined(strcasecmp)
#define strcasecmp _stricmp
#endif

#if defined(_MSC_VER) && !defined(strncasecmp)
#define strncasecmp _strnicmp
#endif

// Get the size of a static array
#define earrlen(a) (sizeof(a) / sizeof(*a))

// Define a struct var and ensure it is fully initialized
#define edefstructvar(type, name)  \
   type name;                      \
   std::memset(&name, 0, sizeof(name))

// Empty-or-null testing
#define estrempty(str) ((str) == NULL || *(str) == '\0')

// Types
typedef uint8_t byte;
typedef int32_t efixed_t;

// Fixed-point defines
#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

#endif

// EOF

