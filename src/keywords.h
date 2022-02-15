/*
  CALICO

  C/C++ compiler-specific keywords and support macros

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

#ifndef KEYWORDS_H__
#define KEYWORDS_H__

//
// Types
//
typedef enum { false, true } boolean;
typedef unsigned char byte;

//
// Function names
//
#if defined(_MSC_VER)
#if !defined(strcasecmp)
#define strcasecmp _stricmp
#endif
#if !defined(strncasecmp)
#define strncasecmp _strnicmp
#endif
#endif

//
// Basic macros
//

// calculate static array length
#define earrlen(a) (sizeof(a) / sizeof(*a))

// min/max
#define emin(a, b) ((a) < (b) ? (a) : (b))
#define emax(a, b) ((a) > (b) ? (a) : (b))

//
// Structure packing via #pragma pack
//
#if defined(_MSC_VER) || defined(__GNUC__)
#define CALICO_HAS_PACKING
#endif

//
// Is 64-bit processor?
// Add your own processor define here if it's not covered.
//
#if defined(__x86_64__) || defined(__LP64__) || defined(_M_AMD64)
#define CALICO_IS_X64
#endif

#endif

// EOF
