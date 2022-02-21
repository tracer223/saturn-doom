/*
  CALICO
  
  Line-of-sight checking

  The MIT License (MIT)
  
  Copyright (c) 2015 James Haley, Olde Skuul, id Software and ZeniMax Media
  
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

#include "doomdef.h"
#include "p_local.h"

static fixed_t sightzstart;           // eye z of looker
static fixed_t topslope, bottomslope; // slopes to top and bottom of target

static divline_t strace;  // from t1 to t2
static fixed_t t2x, t2y;

//
// Returns side 0 (front), 1 (back), or 2 (on).
//
int P_DivlineSide(fixed_t x, fixed_t y, divline_t *node)
{
   fixed_t dx;
   fixed_t dy;
   fixed_t left;
   fixed_t right;

   if(!node->dx)
   {
      if(x == node->x)
         return 2;

      if(x <= node->x)
         return node->dy > 0;

      return node->dy < 0;
   }
    
   if(!node->dy)
   {
      // CALICO: Since Jag didn't have P_DivlineSide, it probably did not have 
      // the bug here either where x was compared to node->y. Pre-emptively fixed.
      if(y == node->y) 
         return 2;

      if (y <= node->y)
         return node->dx < 0;

      return node->dx > 0;
   }

   dx = (x - node->x);
   dy = (y - node->y);

   left  = (node->dy>>FRACBITS) * (dx>>FRACBITS);
   right = (dy>>FRACBITS) * (node->dx>>FRACBITS);

   if(right < left)
      return 0;    // front side

   if(left == right)
      return 2;
   
   return 1;       // back side
}

//
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings and addlines traversers.
//
fixed_t P_InterceptVector2(divline_t *v2, divline_t *v1)
{
   fixed_t frac;
   fixed_t num;
   fixed_t den;

   den = FixedMul(v1->dy>>8,v2->dx) - FixedMul(v1->dx>>8,v2->dy);

   if(den == 0)
      return 0;

   num  = FixedMul((v1->x - v2->x)>>8 ,v1->dy) + FixedMul((v2->y - v1->y)>>8 , v1->dx);
   frac = FixedDiv (num , den);

   return frac;
}

/*
=================
=
= PS_CrossSubsector
=
= Returns true if strace crosses the given subsector successfuly
=================
*/

static boolean PS_CrossSubsector(int num)
{
   seg_t       *seg;
   line_t      *line;
   int          s1;
   int          s2;
   int          count;
   subsector_t *sub;
   doom_sector_t    *front;
   doom_sector_t    *back;
   fixed_t      opentop;
   fixed_t      openbottom;
   divline_t    divl;
   vertex_t    *v1;
   vertex_t    *v2;
   fixed_t      frac;
   fixed_t      slope;

   sub = &subsectors[num];

   // check lines
   count = sub->numlines;
   seg   = &segs[sub->firstline];

   for( ; count; seg++, count--)
   {
      line = seg->linedef;

      // allready checked other side?
      if(line->validcount == validcount)
         continue;

      line->validcount = validcount;

      v1 = line->v1;
      v2 = line->v2;
      s1 = P_DivlineSide(v1->x, v1->y, &strace);
      s2 = P_DivlineSide(v2->x, v2->y, &strace);

      // line isn't crossed?
      if (s1 == s2)
         continue;

      divl.x = v1->x;
      divl.y = v1->y;
      divl.dx = v2->x - v1->x;
      divl.dy = v2->y - v1->y;
      s1 = P_DivlineSide (strace.x, strace.y, &divl);
      s2 = P_DivlineSide (t2x, t2y, &divl);

      // line isn't crossed?
      if (s1 == s2)
         continue;

      // stop because it is not two sided anyway
      // might do this after updating validcount?
      if(!(line->flags & ML_TWOSIDED))
         return false;

      // crosses a two sided line
      front = seg->frontsector;
      back = seg->backsector;

      // no wall to block sight with?
      if(front->floorheight == back->floorheight && front->ceilingheight == back->ceilingheight)
         continue;

      // possible occluder
      // because of ceiling height differences
      if (front->ceilingheight < back->ceilingheight)
         opentop = front->ceilingheight;
      else
         opentop = back->ceilingheight;

      // because of ceiling height differences
      if (front->floorheight > back->floorheight)
         openbottom = front->floorheight;
      else
         openbottom = back->floorheight;

      // quick test for totally closed doors
      if(openbottom >= opentop)
         return false; // stop

      frac = P_InterceptVector2(&strace, &divl);

      if(front->floorheight != back->floorheight)
      {
         slope = FixedDiv(openbottom - sightzstart , frac);
         if(slope > bottomslope)
            bottomslope = slope;
      }

      if(front->ceilingheight != back->ceilingheight)
      {
         slope = FixedDiv (opentop - sightzstart , frac);
         if(slope < topslope)
            topslope = slope;
      }

      if(topslope <= bottomslope)
         return false;    // stop
   }

   // passed the subsector ok
   return true;
}

//
// Returns true if strace crosses the given node successfuly
//
static boolean PS_CrossBSPNode(int bspnum)
{
   node_t *bsp;
   int side;

   if(bspnum & NF_SUBSECTOR)
   {
      if(bspnum == -1)
         return PS_CrossSubsector(0);
      else
         return PS_CrossSubsector(bspnum & ~NF_SUBSECTOR);
   }

   bsp = &nodes[bspnum];

   // decide which side the start point is on
   side = P_DivlineSide(strace.x, strace.y, (divline_t *)bsp);
   if(side == 2)
      side = 0;

   // cross the starting side
   if(!PS_CrossBSPNode(bsp->children[side]))
      return false;

   // the partition plane is crossed here
   if(side == P_DivlineSide(t2x, t2y, (divline_t *)bsp))
      return true; // the line doesn't touch the other side

   // cross the ending side
   return PS_CrossBSPNode(bsp->children[side^1]);
}

//
// Returns true if a straight line between t1 and t2 is unobstructed
//
boolean PS_CheckSight(mobj_t *t1, mobj_t *t2)
{
   int s1, s2;
   int pnum, bytenum, bitnum;

   // First check for trivial rejection
   s1 = (int)(t1->subsector->sector - sectors);
   s2 = (int)(t2->subsector->sector - sectors);
   pnum = s1*numsectors + s2;
   bytenum = pnum>>3;
   bitnum = 1 << (pnum&7);

   if(rejectmatrix[bytenum] & bitnum) 
   {
      return false; // can't possibly be connected
   }

   // look from eyes of t1 to any part of t2
   ++validcount;

   sightzstart = t1->z + t1->height - (t1->height >> 2);
   topslope    = (t2->z + t2->height) - sightzstart;
   bottomslope = (t2->z) - sightzstart;

   // make sure it never lies exactly on a vertex coordinate
   strace.x = (t1->x & ~0x1ffff) | 0x10000;
   strace.y = (t1->y & ~0x1ffff) | 0x10000;
   t2x = (t2->x & ~0x1ffff) | 0x10000;
   t2y = (t2->y & ~0x1ffff) | 0x10000;
   strace.dx = t2x - strace.x;
   strace.dy = t2y - strace.y;

   return PS_CrossBSPNode(numnodes-1);
}

//
// Optimal mobj sight checking that checks sights in the main tick loop rather
// than from multiple mobj action routines.
//
void P_CheckSights2(void)
{
   mobj_t *mobj;

   for(mobj = mobjhead.next; mobj != &mobjhead; mobj = mobj->next)
   {
      // CALICO: skip removed mobjs
      if(mobj->latecall == P_RemoveMobjDeferred)
         continue;

      // must be killable
      if(!(mobj->flags & MF_COUNTKILL))
         continue;

      // must be about to change states
      if(mobj->tics != 1)
         continue;

      mobj->flags &= ~MF_SEETARGET;

      // must have a target
      if(!mobj->target)
         continue;

      if(PS_CheckSight(mobj, mobj->target))
         mobj->flags |= MF_SEETARGET;
   }
}

// EOF

