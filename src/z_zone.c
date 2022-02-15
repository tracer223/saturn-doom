/* Z_zone.c */

#include <stdint.h>
#include "doomdef.h"

/* 
============================================================================== 
 
ZONE MEMORY ALLOCATION 
 
There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

It is of no value to free a cachable block, because it will get overwritten
automatically if needed

============================================================================== 
*/ 
 
memzone_t *mainzone;
memzone_t *refzone;
 
/*
========================
=
= Z_InitZone
=
========================
*/

memzone_t *Z_InitZone(byte *base, int size)
{
   memzone_t *zone;

   zone = (memzone_t *)base;

   zone->size = size;
   zone->rover = &zone->blocklist;
   zone->blocklist.size = size - (int)((byte *)&zone->blocklist - (byte *)zone);
   zone->blocklist.user = NULL;
   zone->blocklist.tag = 0;
   zone->blocklist.id = ZONEID;
   zone->blocklist.next = NULL;
   zone->blocklist.prev = NULL;
   zone->blocklist.lockframe = -1;

   return zone;
}

/*
========================
=
= Z_Init
=
========================
*/

void Z_Init(void)
{
   byte *mem;
   int   size;
   int   chunk = 0x80000;

   mem = I_ZoneBase(&size);

#if defined(CALICO_IS_X64)
   // use double chunk size on x64
   chunk *= 2;
#endif

   mainzone = Z_InitZone(mem, chunk);
   refzone  = Z_InitZone(mem + chunk, size - chunk);
}

/*
========================
=
= Z_Free2
=
========================
*/

void Z_Free2(memzone_t *mainzone, void *ptr)
{
   memblock_t *block;

   block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
   if(block->id != ZONEID)
      I_Error("Z_Free: freed a pointer without ZONEID");

   // CALICO_TODO: non-portable pointer comparison
   if(block->user > (void **)0x100) // smaller values are not pointers
      *block->user = 0;             // clear the user's mark
   block->user = NULL; // mark as free
   block->tag = 0;
   block->id = 0;
}
 
/*
========================
=
= Z_Malloc2
=
= You can pass a NULL user if the tag is < PU_PURGELEVEL
========================
*/

#define MINFRAGMENT 64

void *Z_Malloc2(memzone_t *mainzone, int size, int tag, void *user)
{
   int         extra;
   memblock_t *start, *rover, *newblock, *base;

   //
   // scan through the block list looking for the first free block
   // of sufficient size, throwing out any purgable blocks along the way
   //
   size += sizeof(memblock_t); // account for size of block header
   size = (size + 7) & ~7;     // phrase align everything

   start = base = mainzone->rover;

   while(base->user || base->size < size)
   {
      if(base->user)
         rover = base;
      else
         rover = base->next;

      if(!rover)
         goto backtostart;

      if(rover->user)
      {
         // hit an in use block, so move base past it
         base = rover->next;
         if(!base)
         {
backtostart:
            base = &mainzone->blocklist;
         }

         if(base == start) // scaned all the way around the list
            I_Error("Z_Malloc: failed on %i", size);
         continue;
      }

      //
      // free the rover block (adding the size to base)
      //
      rover->id = 0;
      rover->user = NULL; // mark as free

      if(base != rover)
      { 
         // merge with base
         base->size += rover->size;
         base->next = rover->next;
         if(rover->next)
            rover->next->prev = base;
      }
   }

   //
   // found a block big enough
   //
   extra = base->size - size;
   if(extra >  MINFRAGMENT)
   { 
      // there will be a free fragment after the allocated block
      newblock = (memblock_t *)((byte *)base + size);
      newblock->size = extra;
      newblock->user = NULL; // free block
      newblock->tag = 0;
      newblock->prev = base;
      newblock->next = base->next;
      if(newblock->next)
         newblock->next->prev = newblock;
      base->next = newblock;
      base->size = size;
   }

   if(user)
   {
      base->user = user; // mark as an in use block
      *(void **)user = (void *)((byte *)base + sizeof(memblock_t));
   }
   else
   {
      if(tag >= PU_PURGELEVEL)
         I_Error("Z_Malloc: an owner is required for purgable blocks");
      // CALICO_FIXME: non-portable idiom...
      base->user = (void **)2; // mark as in use, but unowned
   }
   base->tag = tag;
   base->id = ZONEID;
   base->lockframe = -1;

   mainzone->rover = base->next; // next allocation will start looking here
   if(!mainzone->rover)
      mainzone->rover = &mainzone->blocklist;

   return (void *)((byte *)base + sizeof(memblock_t));
}

/*
========================
=
= Z_FreeTags
=
========================
*/

void Z_FreeTags(memzone_t *mainzone)
{
   memblock_t *block, *next;

   for(block = &mainzone->blocklist; block; block = next)
   {
      next = block->next; // get link before freeing
      if(!block->user)
         continue;        // free block
      if(block->tag == PU_LEVEL || block->tag == PU_LEVSPEC)
         Z_Free2(mainzone, (byte *)block + sizeof(memblock_t));
   }
}

/*
========================
=
= Z_CheckHeap
=
========================
*/

memblock_t *checkblock;

void Z_CheckHeap(memzone_t *mainzone)
{
   for(checkblock = &mainzone->blocklist; checkblock; checkblock = checkblock->next)
   {
      if(!checkblock->next)
      {
         if((byte *)checkblock + checkblock->size - (byte *)mainzone != mainzone->size)
            I_Error("Z_CheckHeap: zone size changed\n");
         continue;
      }

      if((byte *)checkblock + checkblock->size != (byte *)checkblock->next)
         I_Error("Z_CheckHeap: block size does not touch the next block\n");
      if(checkblock->next->prev != checkblock)
         I_Error("Z_CheckHeap: next block doesn't have proper back link\n");
   }
}

/*
========================
=
= Z_ChangeTag
=
========================
*/

void Z_ChangeTag(void *ptr, int tag)
{
   memblock_t *block;

   block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
   if(block->id != ZONEID)
      I_Error("Z_ChangeTag: freed a pointer without ZONEID");
   if(tag >= PU_PURGELEVEL && (intptr_t)block->user < 0x100) // CALICO_FIXME: non-portable comparison
      I_Error("Z_ChangeTag: an owner is required for purgable blocks");
   block->tag = tag;
}

/*
========================
=
= Z_FreeMemory
=
========================
*/

int Z_FreeMemory(memzone_t *mainzone)
{
   memblock_t *block;
   int         free;

   free = 0;
   for(block = &mainzone->blocklist; block; block = block->next)
   {
      if(!block->user)
         free += block->size;
   }
   return free;
}

// EOF

