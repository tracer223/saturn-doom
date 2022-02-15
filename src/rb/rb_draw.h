/*
  CALICO

  OpenGL Renderer Primitives

  The MIT License (MIT)

  Copyright (C) 2007-2014 Samuel Villarreal
  Copyright (C) 2016 James Haley

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

#ifndef RB_DRAW_H__
#define RB_DRAW_H__

#include "SDL_opengl.h"
#include "rb_main.h"
#include "rb_texture.h"

void RB_BindDrawPointers(vtx_t *vtx);
void RB_AddTriangle(uint16_t v0, uint16_t v1, uint16_t v2);
void RB_AddLine(uint16_t v0, uint16_t v1);
void RB_DrawElements(int mode);
void RB_ResetElements();

//
// Draw a simple quad in immediate mode.
//
static inline void RB_DrawVtxQuadImmediate(vtx_t *v)
{
   glBegin(GL_QUADS);
   glColor4ubv(v[0].colors);
   glTexCoord2fv(v[0].txcoords); glVertex3fv(v[0].coords);
   glTexCoord2fv(v[1].txcoords); glVertex3fv(v[1].coords);
   glTexCoord2fv(v[3].txcoords); glVertex3fv(v[3].coords);
   glTexCoord2fv(v[2].txcoords); glVertex3fv(v[2].coords);
   glEnd();
}

//
// Set a group of one or more vertices' color components.
//
static inline void RB_SetVertexColors(vtx_t *v, int count, 
                                      rbbyte r, rbbyte g, rbbyte b, rbbyte a)
{
   for(int i = 0; i < count; i++)
   {
      v[i].colors[VTX_R] = r;
      v[i].colors[VTX_G] = g;
      v[i].colors[VTX_B] = b;
      v[i].colors[VTX_A] = a;
   }
}

static inline void RB_SetVertexColors(vtx_t *v, int count, unsigned int color)
{
   for(int i = 0; i < count; i++)
   {
      v[i].colors[VTX_R] = (color >>  0) & 0xff;
      v[i].colors[VTX_G] = (color >>  8) & 0xff;
      v[i].colors[VTX_B] = (color >> 16) & 0xff;
      v[i].colors[VTX_A] = (color >> 24) & 0xff;
   }
}

static inline void RB_DefTexCoords(vtx_t v[4], const rbTexture *tex)
{
   v[0].txcoords[VTX_U] = v[2].txcoords[VTX_U] = 0.0f;
   v[0].txcoords[VTX_V] = v[1].txcoords[VTX_V] = 0.0f;
   v[1].txcoords[VTX_U] = v[3].txcoords[VTX_U] = 1.0f;
   v[2].txcoords[VTX_V] = v[3].txcoords[VTX_V] = 1.0f;
}

static inline void RB_SetVertexZCoords(vtx_t *v, int count, float z)
{
   for(int i = 0; i < count; i++)
      v[i].coords[VTX_Z] = z;
}

#endif

// EOF

