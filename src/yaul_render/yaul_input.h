/*
  CALICO
  
  YAUL Input Implementation
  
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

#ifndef YAUL_INPUT_H__
#define YAUL_INPUT_H__

#ifdef USE_YAUL

#ifdef __cplusplus
extern "C" {
#endif

void     YAUL_SetGrabState(hal_bool state);
// hal_bool YAUL_MouseShouldBeGrabbed(void);
// void     YAUL_UpdateGrab(void);
// void     YAUL_UpdateFocus(void);
// void     YAUL_InitInput(void);
// int      YAUL_GetEvents(void);
// void     YAUL_ResetInput(void);
// void     YAUL_GetMouseMotion(int *x, int *y);

#ifdef __cplusplus
}
#endif

#endif

#endif

// EOF

