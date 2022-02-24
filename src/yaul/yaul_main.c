
#include <yaul.h>
#include <stdio.h>

static void _vdp1_init(void);
static void _vdp2_init(void);

int main(void)
{
   // run application main program
   Jag68k_main();
   return 0;
}

void
user_init(void)
{
   vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
      VDP2_TVMD_VERT_224);

   vdp1_env_t env;
        vdp1_env_default_init(&env);

   vdp1_env_set(&env);

   vdp2_tvmd_display_set();

   vdp2_sync();
   vdp2_sync_wait();

}

extern void Jag68k_main();


