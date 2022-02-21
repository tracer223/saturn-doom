ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/pre.common.mk

SH_PROGRAM:= doom
SH_SRCS:= \
    ../src/elib/m_argv.c \
    ../src/yaul/yaul_main.c \
    ../src/yaul/y_stdlib.c \
    ../src/am_main.c \
    ../src/comnjag.c \
    ../src/d_main.c \
    ../src/f_main.c \
    ../src/g_game.c \
    ../src/in_main.c \
    ../src/info.c \
    ../src/jagonly.c \
    ../src/m_main.c \
    ../src/o_main.c \
    ../src/p_base.c \
    ../src/p_ceilng.c \
    ../src/p_change.c \
    ../src/p_doors.c \
    ../src/p_enemy.c \
    ../src/p_floor.c \
    ../src/p_inter.c \
    ../src/p_lights.c \
    ../src/p_map.c \
    ../src/p_maputl.c \
    ../src/p_mobj.c \
    ../src/p_move.c \
    ../src/p_plats.c \
    ../src/p_pspr.c \
    ../src/p_setup.c \
    ../src/p_shoot.c \
    ../src/p_sight.c \
    ../src/p_slide.c \
    ../src/p_spec.c \
    ../src/p_switch.c \
    ../src/p_telept.c \
    ../src/p_tick.c \
    ../src/p_user.c \
    ../src/r_data.c \
    ../src/r_main.c \
    ../src/r_phase1.c \
    ../src/r_phase2.c \
    ../src/r_phase3.c \
    ../src/r_phase4.c \
    ../src/r_phase5.c \
    ../src/r_phase6.c \
    ../src/r_phase7.c \
    ../src/r_phase8.c \
    ../src/r_phase9.c \
    ../src/s_sound.c \
    ../src/sounds.c \
    ../src/sprinfo.c \
    ../src/st_main.c \
    ../src/tables.c \
    ../src/vsprintf.c \
    ../src/w_iwad.c \
    ../src/w_wad.c \
    ../src/z_zone.c

SH_LIBRARIES:=
# DISABLING OF ERRORS IS TEMPORARY!
SH_CFLAGS+= -O2 -I../src -save-temps=obj -DYAUL_DOOM \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wno-sign-compare \
    -Wno-incompatible-pointer-types \
    -Wno-implicit-fallthrough \
    -Wno-missing-field-initializers \
    -Wno-type-limits \
    -Wno-maybe-uninitialized \
    -Wno-parentheses \
    -Wno-implicit-function-declaration \
    -Wno-int-conversion


IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= MIC 3D
IP_MASTER_STACK_ADDR:= 0x06004000
IP_SLAVE_STACK_ADDR:= 0x06001E00
IP_1ST_READ_ADDR:= 0x06004000

M68K_PROGRAM:=
M68K_OBJECTS:=

include $(YAUL_INSTALL_ROOT)/share/post.common.mk
