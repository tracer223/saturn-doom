/*
  CALICO
  
  Core mobj thinking, movement, and clipping checks.
  Derived from Doom 64 EX, used under MIT by permission.

  The MIT License (MIT)

  Copyright (C) 2016 Samuel Villarreal, James Haley
  
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

static mobj_t      *checkthing, *hitthing;
static fixed_t      testx, testy;
static fixed_t      testfloorz, testceilingz, testdropoffz;
static subsector_t *testsubsec;
static line_t      *ceilingline;
static fixed_t      testbbox[4];
static int          testflags;

// CALICO: current mobj in P_RunMobjBase2
static mobj_t *currentmobj;

//
// Check for collision against another mobj in one of the blockmap cells.
//
static boolean PB_CheckThing(mobj_t *thing)
{
   fixed_t  blockdist;
   int      delta;
   mobj_t  *mo;

   if(!(thing->flags & MF_SOLID))
      return true; // not blocking

   mo = checkthing;
   blockdist = thing->radius + mo->radius;

   delta = thing->x - testx;
   if(delta < 0)
      delta = -delta;
   if(delta >= blockdist)
      return true; // didn't hit it

   delta = thing->y - testy;
   if(delta < 0)
      delta = -delta;
   if(delta >= blockdist)
      return true; // didn't hit it

   if(thing == mo)
      return true; // don't clip against self

   // check for skulls slamming into things
   if(testflags & MF_SKULLFLY)
   {
      hitthing = thing;
      return false;
   }

   // missiles can hit other things
   if(testflags & MF_MISSILE)
   {
      if(mo->z > thing->z + thing->height)
         return true; // went over
      if(mo->z + mo->height < thing->z)
         return true; // went underneath
      if(mo->target->type == thing->type) // don't hit same species as originator
      {
         if(thing == mo->target)
            return true; // don't explode on shooter
         if(thing->type != MT_PLAYER) // let players missile other players
            return false; // explode but do no damage
      }

      if(!(thing->flags & MF_SHOOTABLE))
         return !(thing->flags & MF_SOLID); // didn't do any damage

      // damage/explode
      hitthing = thing;
      return false;
   }

   return !(thing->flags & MF_SOLID);
}

//
// Test for a bounding box collision with a linedef.
//
static boolean PB_BoxCrossLine(line_t *ld)
{
   fixed_t x1, x2;
   fixed_t lx, ly;
   fixed_t ldx, ldy;
   fixed_t dx1, dy1, dx2, dy2;
   boolean side1, side2;

   // entirely outside bounding box of line?
   if(testbbox[BOXRIGHT ] <= ld->bbox[BOXLEFT  ] ||
      testbbox[BOXLEFT  ] >= ld->bbox[BOXRIGHT ] ||
      testbbox[BOXTOP   ] <= ld->bbox[BOXBOTTOM] ||
      testbbox[BOXBOTTOM] >= ld->bbox[BOXTOP   ])
   {
      return false;
   }

   if(ld->slopetype == ST_POSITIVE)
   {
      x1 = testbbox[BOXLEFT ];
      x2 = testbbox[BOXRIGHT];
   }
   else
   {
      x1 = testbbox[BOXRIGHT];
      x2 = testbbox[BOXLEFT ];
   }

   lx  = ld->v1->x;
   ly  = ld->v1->y;
   ldx = (ld->v2->x - ld->v1->x) >> FRACBITS;
   ldy = (ld->v2->y - ld->v1->y) >> FRACBITS;

   dx1 = (x1 - lx) >> FRACBITS;
   dy1 = (testbbox[BOXTOP] - ly) >> FRACBITS;
   dx2 = (x2 - lx) >> FRACBITS;
   dy2 = (testbbox[BOXBOTTOM] - ly) >> FRACBITS;

   side1 = (ldy * dx1 < dy1 * ldx);
   side2 = (ldy * dx2 < dy2 * ldx);

   return (side1 != side2);
}

//
// Adjusts testfloorz and testceilingz as lines are contacted.
//
static boolean PB_CheckLine(line_t *ld)
{
   fixed_t   opentop, openbottom, lowfloor;
   sector_t *front, *back;

   // The moving thing's destination position will cross the given line.
   // if this should not be allowed, return false.
   if(!ld->backsector)
      return false; // one-sided line

   if(!(testflags & MF_MISSILE) && (ld->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
      return false; // explicitly blocking

   front = ld->frontsector;
   back  = ld->backsector;

   if(front->ceilingheight < back->ceilingheight)
      opentop = front->ceilingheight;
   else
      opentop = back->ceilingheight;

   if(front->floorheight > back->floorheight)
   {
      openbottom = front->floorheight;
      lowfloor   = back->floorheight;
   }
   else
   {
      openbottom = back->floorheight;
      lowfloor   = front->floorheight;
   }

   // adjust floor/ceiling heights
   if(opentop < testceilingz)
   {
      testceilingz = opentop;
      ceilingline  = ld;
   }
   if(openbottom > testfloorz)
      testfloorz = openbottom;
   if(lowfloor < testdropoffz)
      testdropoffz = lowfloor;

   return true;
}

//
// Check a thing against a linedef in one of the blockmap cells.
//
static boolean PB_CrossCheck(line_t *ld)
{
   if(PB_BoxCrossLine(ld))
   {
      if(!PB_CheckLine(ld))
         return false;
   }
   return true;
}

//
// Check an mobj's position for validity against lines and other mobjs
//
static boolean PB_CheckPosition(mobj_t *mo)
{
   int xl, xh, yl, yh, bx, by;

   testflags = mo->flags;

   testbbox[BOXTOP   ] = testy + mo->radius;
   testbbox[BOXBOTTOM] = testy - mo->radius;
   testbbox[BOXRIGHT ] = testx + mo->radius;
   testbbox[BOXLEFT  ] = testx - mo->radius;

   // the base floor / ceiling is from the subsector that contains the point.
   // Any contacted lines the step closer together will adjust them.
   testsubsec   = R_PointInSubsector(testx, testy);
   testfloorz   = testdropoffz = testsubsec->sector->floorheight;
   testceilingz = testsubsec->sector->ceilingheight;

   ++validcount;

   ceilingline = NULL;
   hitthing    = NULL;

   // the bounding box is extended by MAXRADIUS because mobj_ts are grouped into
   // mapblocks based on their origin point, and can overlap into adjacent blocks
   // by up to MAXRADIUS units
   xl = (testbbox[BOXLEFT  ] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
   xh = (testbbox[BOXRIGHT ] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
   yl = (testbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
   yh = (testbbox[BOXTOP   ] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

   if(xl < 0)
      xl = 0;
   if(yl < 0)
      yl = 0;
   if(xh >= bmapwidth)
      xh = bmapwidth - 1;
   if(yh >= bmapheight)
      yh = bmapheight - 1;

   checkthing = mo; // store for PB_CheckThing
   for(bx = xl; bx <= xh; bx++)
   {
      for(by = yl; by <= yh; by++)
      {
         if(!P_BlockThingsIterator(bx, by, PB_CheckThing))
            return false;
         if(!P_BlockLinesIterator(bx, by, PB_CrossCheck))
            return false;
      }
   }

   return true;
}

// 
// Try to move to the new position, and relink the mobj to the new position if
// successful.
//
static boolean PB_TryMove(mobj_t *mo, fixed_t tryx, fixed_t tryy)
{
   testx = tryx;
   testy = tryy;

   if(!PB_CheckPosition(mo))
      return false; // solid wall or thing

   if(testceilingz - testfloorz < mo->height)
      return false; // doesn't fit
   if(testceilingz - mo->z < mo->height)
      return false; // mobj must lower itself to fit
   if(testfloorz - mo->z > 24*FRACUNIT)
      return false; // too big a step up
   if(!(testflags & (MF_DROPOFF|MF_FLOAT)) && testfloorz - testdropoffz > 24*FRACUNIT)
      return false; // don't stand over a dropoff

   // the move is ok, so link the thing into its new position
   P_UnsetThingPosition(mo);
   mo->floorz   = testfloorz;
   mo->ceilingz = testceilingz;
   mo->x        = tryx;
   mo->y        = tryy;
   P_SetThingPosition(mo);

   return true;
}

#define STOPSPEED 0x1000
#define FRICTION  0xd240

//
// Do horizontal movement.
//
void P_XYMovement(mobj_t *mo)
{
   fixed_t xleft, yleft, xuse, yuse;

   xleft = xuse = mo->momx & ~7;
   yleft = yuse = mo->momy & ~7;

   while(xuse > MAXMOVE || xuse < -MAXMOVE || yuse > MAXMOVE || yuse < -MAXMOVE)
   {
      xuse >>= 1;
      yuse >>= 1;
   }

   while(xleft || yleft)
   {
      xleft -= xuse;
      yleft -= yuse;

      if(!PB_TryMove(mo, mo->x + xuse, mo->y + yuse))
      {
         // blocked move

         // flying skull?
         if(mo->flags & MF_SKULLFLY)
         {
            P_SetTarget(&mo->extramobj, hitthing);
            mo->latecall = L_SkullBash;
            return;
         }

         // explode a missile?
         if(mo->flags & MF_MISSILE)
         {
            if(ceilingline && ceilingline->backsector && ceilingline->backsector->ceilingpic == -1)
            {
               mo->latecall = P_RemoveMobj;
               return;
            }

            P_SetTarget(&mo->extramobj, hitthing);
            mo->latecall = L_MissileHit;
            return;
         }

         mo->momx = mo->momy = 0;
         return;
      }
   }

   // slow down

   if(mo->flags & (MF_MISSILE|MF_SKULLFLY))
      return; // no friction for missiles or flying skulls ever

   if(mo->z > mo->floorz)
      return; // no friction when airborne

   if(mo->flags & MF_CORPSE && mo->floorz != mo->subsector->sector->floorheight)
      return; // sliding corpse: don't stop halfway off a step

   if(mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
      mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
   {
      mo->momx = 0;
      mo->momy = 0;
   }
   else
   {
      mo->momx = (mo->momx >> 8) * (FRICTION >> 8);
      mo->momy = (mo->momy >> 8) * (FRICTION >> 8);
   }
}

//
// Float a flying monster up or down.
//
static void P_FloatChange(mobj_t *mo)
{
   mobj_t *target;
   fixed_t dist, delta;

   target = mo->target;                              // get the target object
   delta  = (target->z + (mo->height >> 1)) - mo->z; // get the height difference
   
   dist   = P_AproxDistance(target->x - mo->x, target->y - mo->y);
   delta *= 3;

   if(delta < 0)
   {
      if(dist < -delta)
         mo->z -= FLOATSPEED; // adjust height downward
   }
   else if(dist < delta)
      mo->z += FLOATSPEED;    // adjust height upward
}

//
// Do movement on the z axis.
//
void P_ZMovement(mobj_t *mo)
{
   mo->z += mo->momz;

   if((mo->flags & MF_FLOAT) && mo->target)
   {
      // float toward target if too close
      P_FloatChange(mo);
   }

   // clip movement
   if(mo->z <= mo->floorz)
   {
      if(mo->momz < 0)
         mo->momz = 0;
      mo->z = mo->floorz; // hit the floor
      if(mo->flags & MF_MISSILE)
      {
         mo->latecall = P_ExplodeMissile;
         return;
      }
   }
   else if(!(mo->flags & MF_NOGRAVITY))
   {
      // apply gravity
      if(!mo->momz)
         mo->momz = -GRAVITY*2;
      else
         mo->momz -= GRAVITY;
   }

   if(mo->z + mo->height > mo->ceilingz)
   {
      if(mo->momz > 0)
         mo->momz = 0;
      mo->z = mo->ceilingz - mo->height; // hit the ceiling
      if(mo->flags & MF_MISSILE)
         mo->latecall = P_ExplodeMissile;
   }
}

//
// Perform main thinking logic for a single mobj per tic.
//
void P_MobjThinker(mobj_t *mobj)
{
   state_t* st;
   statenum_t state;

   // momentum movement
   if(mobj->momx || mobj->momy)
      P_XYMovement(mobj);

   // removed or has a special action to perform?
   if(mobj->latecall)
      return;

   if(mobj->z != mobj->floorz || mobj->momz)
      P_ZMovement(mobj);

   // removed or has a special action to perform?
   if(mobj->latecall)
      return;

   // cycle through states
   if(mobj->tics != -1)
   {
      mobj->tics--;

      // you can cycle through multiple states in a tic
      if(!mobj->tics)
      {
          //if (mobj->state)
          //    P_SetMobjState(mobj, mobj->state->nextstate);

          state = mobj->state->nextstate;
          if (state == S_NULL) {
              mobj->latecall = P_RemoveMobj;
          }
          else
          {
              st = &states[state];

              mobj->state = st;
              mobj->tics = st->tics;
              mobj->sprite = st->sprite;
              mobj->frame = st->frame;
              mobj->latecall = st->action;
          }
      }
   }
}

//
// Assign an mobj to another mobj's target field, maintaining mobj reference
// counts.
// haleyjd: CALICO - use mobj reference counting
//
void P_SetTarget(mobj_t **mop, mobj_t *targ)
{
   if(*mop)
      (*mop)->references--;

   if((*mop = targ))
      targ->references++;
}

//
// CALICO: Link an mobj to the mobj list.
//
void P_LinkMobj(mobj_t *mobj)
{
   mobjhead.prev->next = mobj;
   mobj->next = &mobjhead;
   mobj->prev = mobjhead.prev;
   mobjhead.prev = mobj;
}

//
// CALICO: Remove an mobj from the mobj list.
//
void P_UnlinkMobj(mobj_t *mobj)
{
   mobj_t *next = currentmobj->next;

   (next->prev = currentmobj = mobj->prev)->next = next;
   
   mobj->next = NULL;
   mobj->prev = NULL;
}

//
// CALICO: Remove an unreferenced mobj when it is safe to do so.
//
void P_RemoveMobjDeferred(mobj_t *mobj)
{
   if(!mobj->references)
   {
      // remove from list and free self
      P_UnlinkMobj(mobj);
      Z_Free(mobj);
   }
}

//
// Do the main thinking logic for mobjs during a gametic.
//
void P_RunMobjBase2()
{
   for(currentmobj = mobjhead.next; currentmobj != &mobjhead; currentmobj = currentmobj->next)
   {
      if(currentmobj->latecall == P_RemoveMobjDeferred)
         continue; // CALICO: not if about to be removed.

      if (!currentmobj->player) {
          // clear any latecall from the previous frame
          currentmobj->latecall = NULL;

          P_MobjThinker(currentmobj);
      }
   }
}

//
// Do extra "late" thinking logic for mobjs during a gametic.
// CALICO: moved logic here from p_tick so that it can use currentmobj.
//
void P_RunMobjExtra()
{
   // CALICO: if the thing removes itself, currentmobj will be adjusted automatically.
   for(currentmobj = mobjhead.next; currentmobj != &mobjhead; currentmobj = currentmobj->next)
   {
      if(currentmobj->latecall)
         currentmobj->latecall(currentmobj);
   }
}

// EOF

