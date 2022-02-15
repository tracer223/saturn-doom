/*
  CALICO

  Renderer phase 1 - BSP traversal
*/

#include "doomdef.h"
#include "r_local.h"

typedef struct cliprange_s
{
   int first;
   int last;
} cliprange_t;

#define MAXSEGS 32

cliprange_t *newend;
cliprange_t  solidsegs[MAXSEGS];
seg_t       *curline;
angle_t      lineangle1;
sector_t    *frontsector;

//
// To get a global angle from Cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system,
// then the y (<= x) is scaled and divided by x to get a tangent (slope)
// value which is looked up in the tantoangle table.
//
angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
   x -= viewx;
   y -= viewy;

   if(!x && !y)
      return 0;

   if(x >= 0)
   {
      if(y >= 0)
      {
         if(x > y)
            return tantoangle[SlopeDiv(y, x)]; // octant 0
         else
            return ANG90 - 1 - tantoangle[SlopeDiv(x, y)]; // octant 1
      }
      else
      {
         y = -y;

         if(x > y)
            return -tantoangle[SlopeDiv(y, x)]; // octant 7
         else
            return ANG270 + tantoangle[SlopeDiv(x, y)]; // octant 6
      }
   }
   else
   {
      x = -x;

      if(y >= 0)
      {
         if(x > y)
            return ANG180 - 1 - tantoangle[SlopeDiv(y, x)]; // octant 3
         else
            return ANG90 + tantoangle[SlopeDiv(x, y)]; // octant 2
      }
      else
      {
         y = -y;

         if(x > y)
            return ANG180 + tantoangle[SlopeDiv(y, x)]; // octant 4
         else
            return ANG270 - 1 - tantoangle[SlopeDiv(x, y)]; // octant 5
      }
   }

   return 0;
}

static int checkcoord[12][4] =
{
   { 3, 0, 2, 1 },
   { 3, 0, 2, 0 },
   { 3, 1, 2, 0 },
   { 0, 0, 0, 0 },
   { 2, 0, 2, 1 },
   { 0, 0, 0, 0 },
   { 3, 1, 3, 0 },
   { 0, 0, 0, 0 },
   { 2, 0, 3, 1 },
   { 2, 1, 3, 1 },
   { 2, 1, 3, 0 },
   { 0, 0, 0, 0 }
};

//
// Checks BSP node/subtree bounding box. Returns true if some part of the bbox
// might be visible.
//
boolean R_CheckBBox(fixed_t bspcoord[4])
{
   int boxx;
   int boxy;
   int boxpos;

   fixed_t x1, y1, x2, y2;

   angle_t angle1, angle2, span, tspan;

   cliprange_t *start;

   int sx1, sx2;

   // find the corners of the box that define the edges from current viewpoint
   if(viewx <= bspcoord[BOXLEFT])
      boxx = 0;
   else if(viewx < bspcoord[BOXRIGHT])
      boxx = 1;
   else
      boxx = 2;

   if(viewy >= bspcoord[BOXTOP])
      boxy = 0;
   else if(viewy > bspcoord[BOXBOTTOM])
      boxy = 1;
   else
      boxy = 2;

   boxpos = (boxy << 2) + boxx;
   if(boxpos == 5)
      return true;

   x1 = bspcoord[checkcoord[boxpos][0]];
   y1 = bspcoord[checkcoord[boxpos][1]];
   x2 = bspcoord[checkcoord[boxpos][2]];
   y2 = bspcoord[checkcoord[boxpos][3]];

   // check clip list for an open space
   angle1 = R_PointToAngle(x1, y1) - viewangle;
   angle2 = R_PointToAngle(x2, y2) - viewangle;

   span = angle1 - angle2;

   // sitting on a line?
   if(span >= ANG180)
      return true;
   
   tspan = angle1 + clipangle;
   if(tspan > doubleclipangle)
   {
      tspan -= doubleclipangle;

      // totally off the left edge?
      if(tspan >= span)
         return false;

      angle1 = clipangle;
   }

   tspan = clipangle - angle2;
   if(tspan > doubleclipangle)
   {
      tspan -= doubleclipangle;

      // totally off the left edge?
      if(tspan >= span)
         return false;

      angle2 = 0 - clipangle;
   }

   // find the first clippost that touches the source post (adjacent pixels
   // are touching).
   angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
   angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
   sx1    = viewangletox[angle1];
   sx2    = viewangletox[angle2];

   // does not cross a pixel?
   if(sx1 == sx2)
      return false;
   --sx2;

   start = solidsegs;
   while(start->last < sx2)
      ++start;

   // does the clippost contain the new span?
   if(sx1 >= start->first && sx2 <= start->last)
      return false;

   return true;
}

//
// Store information about the clipped seg range into the viswall array.
//
void R_StoreWallRange(int start, int stop)
{
   viswall_t *rw;

   rw = lastwallcmd++;

   rw->seg    = curline;
   rw->start  = start;
   rw->stop   = stop;
   rw->angle1 = lineangle1;
}

//
// Clips the given range of columns, but does not include it in the clip list.
// Does handle windows, e.g., linedefs with upper and lower textures.
//
void R_ClipPassWallSegment(fixed_t first, fixed_t last)
{
   fixed_t      scratch;
   cliprange_t *start;

   // find the first range that touches the range (adjacent pixels are touching)
   scratch = first - 1;
   start   = solidsegs;
   while(start->last < scratch)
      ++start;

   // cut up the seg around solid pieces
   if(first < start->first)
   {
      if(last < start->first - 1)
      {
         // post is entirely visible (above start)
         R_StoreWallRange(first, last);
         return;
      }

      // there is a fragment above *start
      R_StoreWallRange(first, start->first - 1);
   }

   // bottom contained in start?
   if(last <= start->last)
      return;

   while(last >= (start+1)->first - 1)
   {
      // there is a fragment between two posts.
      R_StoreWallRange(start->last + 1, (start+1)->first - 1);
      ++start;

      if(last <= start->last)
         return;
   }

   // there is a fragment after *next
   R_StoreWallRange(start->last + 1, last);
}

void R_ClipSolidWallSegment(fixed_t first, fixed_t last)
{
   fixed_t      scratch;
   cliprange_t *start, *next;

   // find the first range that touches the range (adjacent pixels are touching)
   scratch = first - 1;
   start   = solidsegs;
   while(start->last < scratch)
      ++start;

   // add visible pieces and close up holes
   if(first < start->first)
   {
      if(last < start->first - 1)
      {
         // post is entirely visible (above start)
         R_StoreWallRange(first, last);
         next = newend;
         ++newend;

         while(next != start)
         {
            *next = *(next - 1);
            --next;
         }

         next->first = first;
         next->last  = last;
         return;
      }

      // there is a fragment above *start
      R_StoreWallRange(first, start->first - 1);

      // now adjust the clip size
      start->first = first;
   }

   // bottom contained in start?
   if(last <= start->last)
      return;

   next = start;
   while(last >= (next + 1)->first - 1)
   {
      // there is a fragment between two posts
      R_StoreWallRange(next->last + 1, (next + 1)->first - 1);
      ++next;
      
      if(last <= next->last)
      {
         // bottom is contained in next, adjust the clip size
         start->last = next->last;
         goto crunch;
      }
   }

   // there is a fragment after *next
   R_StoreWallRange(next->last + 1, last);
   // adjust the clip size
   start->last = last;

   // remove start+1 to next from the clip list, because start now covers
   // their area
crunch:
   if(next == start) // post just extended past the bottom of one post
      return;

   while(next++ != newend)
      *++start = *next;

   newend = start + 1;
}

//
// Clips the given segment and adds any visible pieces to the line list.
//
void R_AddLine(seg_t *line)
{
   angle_t angle1, angle2, span, tspan;
   fixed_t x1, x2;
   sector_t *backsector;

   curline = line;

   angle1 = R_PointToAngle(line->v1->x, line->v1->y);
   angle2 = R_PointToAngle(line->v2->x, line->v2->y);

   // clip to view edges
   span = angle1 - angle2;

   if(span >= ANG180)
      return;

   lineangle1 = angle1;
   angle1 -= viewangle;
   angle2 -= viewangle;

   tspan = angle1 + clipangle;
   if(tspan > doubleclipangle)
   {
      tspan -= doubleclipangle;
      if(tspan >= span)
         return;
      angle1 = clipangle;
   }

   tspan = clipangle - angle2;
   if(tspan > doubleclipangle)
   {
      tspan -= doubleclipangle;
      if(tspan >= span)
         return;
      angle2 = 0 - clipangle;
   }

   // the seg is in the view range, but not necessarily visible

   angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
   angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;

   x1 = viewangletox[angle1];
   x2 = viewangletox[angle2];

   if(x1 == x2) // doesn't cross a pixel
      return;
   --x2;

   // decide which clip routine to use

   backsector = line->backsector;

   if(!backsector || 
      backsector->ceilingheight <= frontsector->floorheight ||
      backsector->floorheight   >= frontsector->ceilingheight)
      goto clipsolid;

   if(backsector->ceilingheight != frontsector->ceilingheight ||
      backsector->floorheight   != frontsector->floorheight)
      goto clippass;

   // reject empty lines used for triggers and special events
   if(backsector->ceilingpic == frontsector->ceilingpic &&
      backsector->floorpic   == frontsector->floorpic   &&
      backsector->lightlevel == frontsector->lightlevel &&
      curline->sidedef->midtexture == 0)
      return;

clippass:
   R_ClipPassWallSegment(x1, x2);
   return;

clipsolid:
   R_ClipSolidWallSegment(x1, x2);
}

//
// Determine floor/ceiling planes, add sprites of things in sector,
// draw one or more segments.
//
void R_Subsector(int num)
{
   subsector_t *sub = &subsectors[num];
   seg_t       *line, *stopline;
   int          count;
   
   frontsector = sub->sector;
   
   *lastvissubsector = sub;
   ++lastvissubsector;

   line     = &segs[sub->firstline];
   count    = sub->numlines;
   stopline = line + count;

   while(line != stopline)
      R_AddLine(line++);
}

//
// Recursively descend through the BSP, classifying nodes according to the
// player's point of view, and render subsectors in view.
//
void R_RenderBSPNode(int bspnum)
{
   node_t *bsp;
   int     side;

   if(bspnum & NF_SUBSECTOR) // reached a subsector leaf?
   {
      if(bspnum == -1)
         R_Subsector(0);
      else
         R_Subsector(bspnum & ~NF_SUBSECTOR);
      return;
   }

   bsp = &nodes[bspnum];

   // decide which side the view point is on
   side = R_PointOnSide(viewx, viewy, bsp);

   // recursively render front space
   R_RenderBSPNode(bsp->children[side]);

   // possibly divide back space
   if(R_CheckBBox(bsp->bbox[side^1]))
      R_RenderBSPNode(bsp->children[side^1]);
}

//
// Kick off the rendering process by initializing the solidsegs array and then
// starting the BSP traversal.
//
void R_BSP(void)
{
   solidsegs[0].first = -2;
   solidsegs[0].last  = -1;
   solidsegs[1].first = SCREENWIDTH;
   solidsegs[1].last  = SCREENWIDTH+1;
   newend = &solidsegs[2];

   R_RenderBSPNode(numnodes - 1);
}

// EOF

