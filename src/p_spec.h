/* P_spec.h */

#ifndef P_SPEC_H__
#define P_SPEC_H__

/*
===============================================================================

P_SPEC

===============================================================================
*/

/* */
/* Animating textures and planes */
/* */
typedef struct
{
   boolean istexture;
   int     picnum;
   int     basepic;
   int     numpics;
   int     current;
} anim_t;

/* */
/* source animation definition */
/* */
typedef struct
{
   boolean istexture; /* if false, it's a flat */
   char    endname[9];
   char    startname[9];
   int     speed;
} animdef_t;

#define	MAXANIMS 32

extern anim_t anims[MAXANIMS], *lastanim;

/* */
/* Animating line specials */
/* */
#define	MAXLINEANIMS 64
extern int numlinespecials;
extern line_t *linespeciallist[MAXLINEANIMS];


/* Define values for map objects */
#define	MO_TELEPORTMAN 14

/* at game start */
void P_InitPicAnims(void);

/* at map load */
void P_SpawnSpecials(void);

/* every tic */
void P_UpdateSpecials(void);

/* when needed */
boolean P_UseSpecialLine(mobj_t *thing, line_t *line);
void    P_ShootSpecialLine(mobj_t *thing, line_t *line);
void    P_CrossSpecialLine(line_t *line, mobj_t *thing);
void    P_PlayerInSpecialSector(player_t *player);

int twoSided(int sector, int line);
doom_sector_t *getSector(int currentSector,int line,int side);
side_t   *getSide(int currentSector,int line, int side);
fixed_t P_FindLowestFloorSurrounding(doom_sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(doom_sector_t *sec);
fixed_t P_FindNextHighestFloor(doom_sector_t *sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(doom_sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(doom_sector_t *sec);
int     P_FindSectorFromLineTag(line_t *line,int start);
int     P_FindMinSurroundingLight(doom_sector_t *sector,int max);
doom_sector_t *getNextSector(line_t *line,doom_sector_t *sec);

/* */
/* SPECIAL */
/* */
int EV_DoDonut(line_t *line);

/*
===============================================================================

P_LIGHTS

===============================================================================
*/
typedef struct
{
   thinker_t  thinker;
   doom_sector_t  *sector;
   int        count;
   int        maxlight;
   int        minlight;
   int        maxtime;
   int        mintime;
} lightflash_t;

typedef struct
{
   thinker_t  thinker;
   doom_sector_t  *sector;
   int        count;
   int        minlight;
   int        maxlight;
   int        darktime;
   int        brighttime;
} strobe_t;

typedef struct
{
   thinker_t  thinker;
   doom_sector_t  *sector;
   int        minlight;
   int        maxlight;
   int        direction;
} glow_t;

#define GLOWSPEED    16
#define	STROBEBRIGHT 3
#define	FASTDARK     8
#define	SLOWDARK     15

void T_LightFlash(lightflash_t *flash);
void P_SpawnLightFlash(doom_sector_t *sector);
void T_StrobeFlash(strobe_t *flash);
void P_SpawnStrobeFlash(doom_sector_t *sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing(line_t *line);
void EV_TurnTagLightsOff(line_t	*line);
void EV_LightTurnOn(line_t *line, int bright);
void T_Glow(glow_t *g);
void P_SpawnGlowingLight(doom_sector_t *sector);

/*
===============================================================================

P_SWITCH

===============================================================================
*/
typedef struct
{
    char name1[9];
    char name2[9];
} switchlist_t;

typedef enum
{
   top,
   middle,
   bottom
} bwhere_e;

typedef struct
{
   line_t   *line;
   bwhere_e  where;
   int       btexture;
   int       btimer;
   mobj_t   *soundorg;
} button_t;

#define	MAXSWITCHES 50 /* max # of wall switches in a level */
#define	MAXBUTTONS  16 /* 4 players, 4 buttons each at once, max. */
#define BUTTONTIME  15 /* 1 second */

extern button_t buttonlist[MAXBUTTONS];	

void P_ChangeSwitchTexture(line_t *line,int useAgain);
void P_InitSwitchList(void);

/*
===============================================================================

P_PLATS

===============================================================================
*/
typedef enum
{
   up,
   down,
   waiting,
   in_stasis
} plat_e;

typedef enum
{
   perpetualRaise,
   downWaitUpStay,
   raiseAndChange,
   raiseToNearestAndChange
} plattype_e;

typedef struct
{
   thinker_t  thinker;
   doom_sector_t  *sector;
   fixed_t    speed;
   fixed_t    low;
   fixed_t    high;
   int        wait;
   int        count;
   plat_e     status;
   plat_e     oldstatus;
   boolean    crush;
   int        tag;
   plattype_e type;
} plat_t;

#define	PLATWAIT  3             /* seconds */
#define	PLATSPEED (FRACUNIT*2)
#define	MAXPLATS  30

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t	*plat);
int  EV_DoPlat(line_t *line,plattype_e type,int amount);
void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void EV_StopPlat(line_t *line);
void P_ActivateInStasis(int tag);

/*
===============================================================================

P_DOORS

===============================================================================
*/
typedef enum
{
   normal,
   close30ThenOpen,
   close,
   open,
   raiseIn5Mins
} vldoor_e;

typedef struct
{
   thinker_t  thinker;
   vldoor_e   type;
   doom_sector_t  *sector;
   fixed_t    topheight;
   fixed_t    speed;
   int        direction;    /* 1 = up, 0 = waiting at top, -1 = down */
   int        topwait;      /* tics to wait at the top */
   /* (keep in case a door going down is reset) */
   int        topcountdown; /* when it reaches 0, start going down */
} vldoor_t;
	
#define	VDOORSPEED FRACUNIT*6
#define	VDOORWAIT  70

void EV_VerticalDoor(line_t *line, mobj_t *thing);
int  EV_DoDoor(line_t *line, vldoor_e  type);
void T_VerticalDoor(vldoor_t *door);
void P_SpawnDoorCloseIn30(doom_sector_t *sec);
void P_SpawnDoorRaiseIn5Mins(doom_sector_t *sec, int secnum);

/*
===============================================================================

P_CEILNG

===============================================================================
*/
typedef enum
{
   lowerToFloor,
   raiseToHighest,
   lowerAndCrush,
   crushAndRaise,
   fastCrushAndRaise
} ceiling_e;

typedef struct
{
   thinker_t  thinker;
   ceiling_e  type;
   doom_sector_t  *sector;
   fixed_t    bottomheight, topheight;
   fixed_t    speed;
   boolean    crush;
   int        direction; /* 1 = up, 0 = waiting, -1 = down */
   int        tag;       /* ID */
   int        olddirection;
} ceiling_t;

#define	CEILSPEED   FRACUNIT*2
#define MAXCEILINGS 30

extern ceiling_t *activeceilings[MAXCEILINGS];

int  EV_DoCeiling(line_t *line, ceiling_e  type);
void T_MoveCeiling(ceiling_t *ceiling);
void P_AddActiveCeiling(ceiling_t *c);
void P_RemoveActiveCeiling(ceiling_t *c);
int  EV_CeilingCrushStop(line_t	*line);
void P_ActivateInStasisCeiling(line_t *line);

/*
===============================================================================

P_FLOOR

===============================================================================
*/
typedef enum
{
   lowerFloor,          /* lower floor to highest surrounding floor */
   lowerFloorToLowest,  /* lower floor to lowest surrounding floor */
   turboLower,          /* lower floor to highest surrounding floor VERY FAST */
   raiseFloor,          /* raise floor to lowest surrounding CEILING */
   raiseFloorToNearest, /* raise floor to next highest surrounding floor */
   raiseToTexture,      /* raise floor to shortest height texture around it */
   lowerAndChange,      /* lower floor to lowest surrounding floor and change floorpic */
   raiseFloor24,
   raiseFloor24AndChange,
   raiseFloorCrush,
   donutRaise
} floor_e;

typedef struct
{
   thinker_t  thinker;
   floor_e    type;
   boolean    crush;
   doom_sector_t  *sector;
   int        direction;
   int        newspecial;
   short      texture;
   fixed_t    floordestheight;
   fixed_t    speed;
} floormove_t;

#define	FLOORSPEED FRACUNIT*3

typedef enum
{
   ok,
   crushed,
   pastdest
} result_e;

result_e T_MovePlane(doom_sector_t *sector, fixed_t speed, fixed_t dest, boolean crush,
                     int floorOrCeiling, int direction);

int  EV_BuildStairs(line_t *line);
int  EV_DoFloor(line_t *line,floor_e floortype);
void T_MoveFloor(floormove_t *floor);

/*
===============================================================================

P_TELEPT

===============================================================================
*/

int EV_Teleport(line_t *line, mobj_t *thing);

#endif

// EOF

