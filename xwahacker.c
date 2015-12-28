/*
 * XWAHacker: collection of binary patches for X-Wing Alliance.
 * Copyright (C) 2009-2015 Reimar Döffinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#define DEBUG 0

#define SHORT_ENUM __attribute__((packed))

#define NUM_RES 4
static const struct {
  int offset;
  int fov_offset;
  int width;
  int height;
} resdes[NUM_RES] = {
  {0x10a3dd, 0x10f422,  640,  480}, // 0
  {0x10a334, 0x10f44f,  800,  600}, // 1
// 2: 1024x768
  {0x10a37c, 0x10f4a6, 1152,  864}, // 3
// 4: 1280x1024
  {0x10a3bb, 0x10f4fa, 1600, 1200}, // 5
};

static uint32_t RL32(const void *ptr) {
  const uint8_t *p = (const uint8_t *)ptr;
  return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

static void WL32(void *ptr, uint32_t v) {
  uint8_t *p = (uint8_t *)ptr;
  p[3] = v >> 24;
  p[2] = v >> 16;
  p[1] = v >>  8;
  p[0] = v;
}

#define BUFFER_SZ 1024

/**
 * \return 0 if an error occurred while reading or seeking
 */
static int read_buffer(uint8_t *buffer, FILE *f, int offset, int size) {
  assert(size <= BUFFER_SZ);
  memset(buffer, 0, size);
  if (fseek(f, offset, SEEK_SET))
    return 0;
  if (fread(buffer, 1, size, f) != size)
    return 0;
  return 1;
}

static int write_buffer(const uint8_t *buffer, FILE *f, int offset, int size) {
  assert(size <= BUFFER_SZ);
  if (fseek(f, offset, SEEK_SET))
    return 0;
  if (fwrite(buffer, 1, size, f) != size)
    return 0;
  return 1;
}

enum PATCHES {
  NO_PATCH = -1,
  FIRST_PATCH = 0,
  PATCH_16BIT_FB = 0,
  PATCH_32BIT_FB,
  PATCH_ZDEPTH_AUTO,
  PATCH_ZDEPTH_16,
  PATCH_ZDEPTH_24,
  PATCH_ZDEPTH_32,
  PATCH_BLT_CLEAR,
  PATCH_CLEAR2,
  PATCH_CLEAR_Z_16,
  PATCH_CLEAR_Z_24,
  PATCH_CLEAR_Z_32,
  PATCH_CLEAR_Z_F,
  PATCH_STARS_ON,
  PATCH_STARS_OFF,
  PATCH_STAR_16_1,
  PATCH_STAR_32_1,
  PATCH_STAR_32_1_2,
  PATCH_STAR_16_2,
  PATCH_STAR_32_2,
  PATCH_STAR_16_3,
  PATCH_STAR_32_3,
  PATCH_STAR_16_4,
  PATCH_STAR_32_4,
  PATCH_STAR_16_5,
  PATCH_STAR_32_5,
  PATCH_STAR_16_6,
  PATCH_STAR_32_6,
  PATCH_STAR_16_7,
  PATCH_STAR_32_7,
  PATCH_STAR_16_8,
  PATCH_STAR_32_8,
  PATCH_STAR_16_9,
  PATCH_STAR_32_9,
  PATCH_STAR_16_10,
  PATCH_STAR_32_10,
  PATCH_STAR_16_11,
  PATCH_STAR_32_11,
  PATCH_STAR_16_12,
  PATCH_STAR_32_12,
  PATCH_STAR_16_13,
  PATCH_STAR_32_13,
  PATCH_CD_CHECK,
  PATCH_NO_CD_CHECK,
  PATCH_CD_VOICE,
  PATCH_HD_VOICE,
  PATCH_SELECT_RES,
  PATCH_FORCE_RES,
  PATCH_SELECT_RES_2,
  PATCH_FORCE_RES_2,
  PATCH_PRTSCR_8_1,
  PATCH_PRTSCR_32_1,
  PATCH_PRTSCR_8_2,
  PATCH_PRTSCR_32_2,

  PATCH_TIE95_BLT_CLEAR,
  PATCH_TIE95_CLEAR2,

  PATCH_XWING95_BLT_CLEAR,
  PATCH_XWING95_CLEAR2,

  PATCH_XVTBOP_BLT_CLEAR,
  PATCH_XVTBOP_BROKEN,
  PATCH_XVTBOP_CLEAR2,
  PATCH_XVTBOP_JS_CHECK,
  PATCH_XVTBOP_NO_JS_CHECK,

  PATCH_BOP_BLT_CLEAR,
  PATCH_BOP_CLEAR2,
  PATCH_BOP_RANDCD,
  PATCH_BOP_XVTCD,

  PATCH_BOP_EN_BLT_CLEAR,
  PATCH_BOP_EN_CLEAR2,
  PATCH_BOP_EN_RANDCD,
  PATCH_BOP_EN_XVTCD,

  PATCH_NO_MSGLOOP,
  PATCH_ADD_MSGLOOP,

  PATCH_NO_HIDE_CD_DRIVES,
  PATCH_HIDE_CD_DRIVES,

  PATCH_BOP_GOG_DISABLE_3D,
  PATCH_BOP_GOG_ENABLE_3D,
  PATCH_XVT_GOG_DISABLE_3D,
  PATCH_XVT_GOG_ENABLE_3D,

  NUM_PATCHES
} SHORT_ENUM;

// patch groups help ensure that all patching will be reversible
static const enum PATCHES xwa_patchgroups[] = {
  PATCH_16BIT_FB, PATCH_32BIT_FB, NO_PATCH,
  PATCH_ZDEPTH_AUTO, PATCH_ZDEPTH_16, PATCH_ZDEPTH_24, PATCH_ZDEPTH_32, NO_PATCH,
  PATCH_BLT_CLEAR, PATCH_CLEAR2, NO_PATCH,
  PATCH_CLEAR_Z_16, PATCH_CLEAR_Z_24, PATCH_CLEAR_Z_32, PATCH_CLEAR_Z_F, NO_PATCH,
  PATCH_STARS_ON, PATCH_STARS_OFF, NO_PATCH,
  PATCH_STAR_16_1, PATCH_STAR_32_1, PATCH_STAR_32_1_2, NO_PATCH,
  PATCH_STAR_16_2, PATCH_STAR_32_2, NO_PATCH,
  PATCH_STAR_16_3, PATCH_STAR_32_3, NO_PATCH,
  PATCH_STAR_16_4, PATCH_STAR_32_4, NO_PATCH,
  PATCH_STAR_16_5, PATCH_STAR_32_5, NO_PATCH,
  PATCH_STAR_16_6, PATCH_STAR_32_6, NO_PATCH,
  PATCH_STAR_16_7, PATCH_STAR_32_7, NO_PATCH,
  PATCH_STAR_16_8, PATCH_STAR_32_8, NO_PATCH,
  PATCH_STAR_16_9, PATCH_STAR_32_9, NO_PATCH,
  PATCH_STAR_16_10, PATCH_STAR_32_10, NO_PATCH,
  PATCH_STAR_16_11, PATCH_STAR_32_11, NO_PATCH,
  PATCH_STAR_16_12, PATCH_STAR_32_12, NO_PATCH,
  PATCH_STAR_16_13, PATCH_STAR_32_13, NO_PATCH,
  PATCH_CD_CHECK, PATCH_NO_CD_CHECK, NO_PATCH,
  PATCH_CD_VOICE, PATCH_HD_VOICE, NO_PATCH,
  PATCH_SELECT_RES, PATCH_FORCE_RES, NO_PATCH,
  PATCH_SELECT_RES_2, PATCH_FORCE_RES_2, NO_PATCH,
  PATCH_PRTSCR_8_1, PATCH_PRTSCR_32_1, NO_PATCH,
  PATCH_PRTSCR_8_2, PATCH_PRTSCR_32_2, NO_PATCH,
  PATCH_NO_MSGLOOP, PATCH_ADD_MSGLOOP, NO_PATCH,
  PATCH_NO_HIDE_CD_DRIVES, PATCH_HIDE_CD_DRIVES, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES tie95_patchgroups[] = {
  PATCH_TIE95_BLT_CLEAR, PATCH_TIE95_CLEAR2, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES xwing95_patchgroups[] = {
  PATCH_XWING95_BLT_CLEAR, PATCH_XWING95_CLEAR2, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES xvtbop_patchgroups[] = {
  PATCH_XVTBOP_BLT_CLEAR, PATCH_XVTBOP_BROKEN, PATCH_XVTBOP_CLEAR2, NO_PATCH,
  PATCH_XVTBOP_JS_CHECK, PATCH_XVTBOP_NO_JS_CHECK, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES bop_patchgroups[] = {
  PATCH_BOP_BLT_CLEAR, PATCH_BOP_CLEAR2, NO_PATCH,
  PATCH_BOP_RANDCD, PATCH_BOP_XVTCD, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES bop_en_patchgroups[] = {
  PATCH_BOP_EN_BLT_CLEAR, PATCH_BOP_EN_CLEAR2, NO_PATCH,
  PATCH_BOP_EN_RANDCD, PATCH_BOP_EN_XVTCD, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES bop_gog_patchgroups[] = {
  PATCH_BOP_GOG_DISABLE_3D, PATCH_BOP_GOG_ENABLE_3D, NO_PATCH,
  PATCH_BOP_EN_BLT_CLEAR, PATCH_BOP_EN_CLEAR2, NO_PATCH,
  PATCH_BOP_EN_RANDCD, PATCH_BOP_EN_XVTCD, NO_PATCH,
  NO_PATCH
};

static const enum PATCHES xvt_gog_patchgroups[] = {
  PATCH_XVT_GOG_DISABLE_3D, PATCH_XVT_GOG_ENABLE_3D, NO_PATCH,
  PATCH_XVTBOP_BLT_CLEAR, PATCH_XVTBOP_BROKEN, PATCH_XVTBOP_CLEAR2, NO_PATCH,
  PATCH_XVTBOP_JS_CHECK, PATCH_XVTBOP_NO_JS_CHECK, NO_PATCH,
  NO_PATCH
};

static const char * const patchnames[NUM_PATCHES] = {
  [PATCH_16BIT_FB]    = "16 bit framebuffer",
  [PATCH_32BIT_FB]    = "32 bit framebuffer",
  [PATCH_ZDEPTH_AUTO] = "automatic Z-buffer depth",
  [PATCH_ZDEPTH_16]   = "16 bit Z-buffer depth",
  [PATCH_ZDEPTH_24]   = "24 bit Z-buffer depth",
  [PATCH_ZDEPTH_32]   = "32 bit Z-buffer depth",
  [PATCH_BLT_CLEAR]   = "Z-buffer clear via Surface::Blt",
  [PATCH_CLEAR2]      = "Z-buffer clear via Viewport::Clear2",
  [PATCH_CLEAR_Z_16]  = "clear Z-buffer with 16 bit value",
  [PATCH_CLEAR_Z_24]  = "clear Z-buffer with 24 bit value",
  [PATCH_CLEAR_Z_32]  = "clear Z-buffer with 32 bit value",
  [PATCH_CLEAR_Z_F]   = "clear Z-buffer with floating-point value",
  [PATCH_STARS_ON]    = "starfield on",
  [PATCH_STARS_OFF]   = "starfield off (Linux/Wine only)",
  [PATCH_STAR_16_1]   = "16 bit starfield part 1",
  [PATCH_STAR_32_1]   = "32 bit starfield part 1 (broken)",
  [PATCH_STAR_32_1_2] = "32 bit starfield part 1 (fixed)",
  [PATCH_STAR_16_2]   = "16 bit starfield part 2",
  [PATCH_STAR_32_2]   = "32 bit starfield part 2",
  [PATCH_STAR_16_3]   = "16 bit starfield part 3",
  [PATCH_STAR_32_3]   = "32 bit starfield part 3",
  [PATCH_STAR_16_4]   = "16 bit starfield part 4",
  [PATCH_STAR_32_4]   = "32 bit starfield part 4",
  [PATCH_STAR_16_5]   = "16 bit starfield part 5",
  [PATCH_STAR_32_5]   = "32 bit starfield part 5",
  [PATCH_STAR_16_6]   = "16 bit starfield part 6",
  [PATCH_STAR_32_6]   = "32 bit starfield part 6",
  [PATCH_STAR_16_7]   = "16 bit starfield part 7",
  [PATCH_STAR_32_7]   = "32 bit starfield part 7",
  [PATCH_STAR_16_8]   = "16 bit starfield part 8",
  [PATCH_STAR_32_8]   = "32 bit starfield part 8",
  [PATCH_STAR_16_9]   = "16 bit starfield part 9",
  [PATCH_STAR_32_9]   = "32 bit starfield part 9",
  [PATCH_STAR_16_10]  = "16 bit starfield part 10",
  [PATCH_STAR_32_10]  = "32 bit starfield part 10",
  [PATCH_STAR_16_11]  = "16 bit starfield part 11",
  [PATCH_STAR_32_11]  = "32 bit starfield part 11",
  [PATCH_STAR_16_12]  = "16 bit starfield part 12",
  [PATCH_STAR_32_12]  = "32 bit starfield part 12",
  [PATCH_STAR_16_13]  = "16 bit starfield part 13 (anti-blink hack)",
  [PATCH_STAR_32_13]  = "32 bit starfield part 13 (anti-blink hack)",
  [PATCH_CD_CHECK]    = "check if CD is in drive",
  [PATCH_NO_CD_CHECK] = "skip check if CD is in drive",
  [PATCH_CD_VOICE]    = "play briefing voice only from CD",
  [PATCH_HD_VOICE]    = "play briefing voice from a HD copy",
  [PATCH_SELECT_RES]  = "allow selecting resolution (part 1)",
  [PATCH_FORCE_RES]   = "force the 800x600 resolution (part 1)",
  [PATCH_SELECT_RES_2]  = "allow selecting resolution (part 2, FOV)",
  [PATCH_FORCE_RES_2]   = "force the 800x600 resolution (part 2, FOV)",
  [PATCH_PRTSCR_8_1]  = "support 8 bit screenshots part 1",
  [PATCH_PRTSCR_32_1] = "support 32 bit screenshots part 1",
  [PATCH_PRTSCR_8_2]  = "support 8 bit screenshots part 2",
  [PATCH_PRTSCR_32_2] = "support 32 bit screenshots part 2",

  [PATCH_TIE95_BLT_CLEAR]   = "Z-buffer clear via Surface::Blt",
  [PATCH_TIE95_CLEAR2]      = "Z-buffer clear via Viewport::Clear2",

  [PATCH_XWING95_BLT_CLEAR]   = "Z-buffer clear via Surface::Blt",
  [PATCH_XWING95_CLEAR2]      = "Z-buffer clear via Viewport::Clear2",

  [PATCH_XVTBOP_BLT_CLEAR]   = "Z-buffer clear via Surface::Blt",
  [PATCH_XVTBOP_BROKEN]      = "Z-buffer clear first try (broken)",
  [PATCH_XVTBOP_CLEAR2]      = "Z-buffer clear via Viewport::Clear2",
  [PATCH_XVTBOP_JS_CHECK]    = "Check for joystick",
  [PATCH_XVTBOP_NO_JS_CHECK] = "Skip check for joystick",

  [PATCH_BOP_BLT_CLEAR]      = "Z-buffer clear via Surface::Blt",
  [PATCH_BOP_CLEAR2]         = "Z-buffer clear via Viewport::Clear2",
  [PATCH_BOP_RANDCD]         = "Ask for random CD for check",
  [PATCH_BOP_XVTCD]          = "Ask for XvT CD for check",

  [PATCH_BOP_EN_BLT_CLEAR]   = "Z-buffer clear via Surface::Blt",
  [PATCH_BOP_EN_CLEAR2]      = "Z-buffer clear via Viewport::Clear2",
  [PATCH_BOP_EN_RANDCD]      = "Ask for random CD for check",
  [PATCH_BOP_EN_XVTCD]       = "Ask for XvT CD for check",

  [PATCH_NO_MSGLOOP]         = "Don't run the message loop",
  [PATCH_ADD_MSGLOOP]        = "Always run message loop, for Linux/Wine",

  [PATCH_NO_HIDE_CD_DRIVES]  = "Scan for CD drives",
  [PATCH_HIDE_CD_DRIVES]     = "Disable scanning for CD drives to avoid error messages",

  [PATCH_BOP_GOG_DISABLE_3D] = "Hardware 3D mode disabled",
  [PATCH_BOP_GOG_ENABLE_3D]  = "Hardware 3D mode enabled",
  [PATCH_XVT_GOG_DISABLE_3D] = "Hardware 3D mode disabled",
  [PATCH_XVT_GOG_ENABLE_3D]  = "Hardware 3D mode enabled",
};

static const struct patchdesc {
  int offset;
  int len;
  int original;
  const uint8_t *value;
} patchdescs[NUM_PATCHES] = {
  [PATCH_16BIT_FB]    = {0x1a90a4, 16, 1,
      (const uint8_t [16]){0x00, 0x00, 0xf0, 0x3e, 0x01, 0x00, 0x00, 0x00,
                           0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f}},
  [PATCH_32BIT_FB]    = {0x1a90a4, 16, 0,
      (const uint8_t [16]){0x00, 0x00, 0xf0, 0x3e, 0x01, 0x00, 0x00, 0x00,
                           0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f}},
  [PATCH_ZDEPTH_AUTO] = {0x19833c, 12, 1,
      (const uint8_t [12]){0x74, 0x0f, 0x8b, 0x15, 0x30, 0x1d, 0x7b, 0x00,
                           0xc7, 0x42, 0x20, 0x20}},
  [PATCH_ZDEPTH_16]   = {0x19833c, 12, 0,
      (const uint8_t [12]){0x90, 0x90, 0x8b, 0x15, 0x30, 0x1d, 0x7b, 0x00,
                           0xc7, 0x42, 0x20, 0x10}},
  [PATCH_ZDEPTH_24]   = {0x19833c, 12, 0,
      (const uint8_t [12]){0x90, 0x90, 0x8b, 0x15, 0x30, 0x1d, 0x7b, 0x00,
                           0xc7, 0x42, 0x20, 0x18}},
  [PATCH_ZDEPTH_32]   = {0x19833c, 12, 0,
      (const uint8_t [12]){0x90, 0x90, 0x8b, 0x15, 0x30, 0x1d, 0x7b, 0x00,
                           0xc7, 0x42, 0x20, 0x20}},
  [PATCH_BLT_CLEAR]   = {0x197410, 31, 1,
      (const uint8_t [31]){0x50, 0x68, 0x00, 0x00, 0x00, 0x03, 0x6a, 0x00,
                           0x6a, 0x00, 0x8d, 0x4d, 0x88, 0x51, 0x8b, 0x15,
                           0xa4, 0xe7, 0xb0, 0x00, 0x52, 0xa1, 0xa4, 0xe7,
                           0xb0, 0x00, 0x8b, 0x08, 0xff, 0x51, 0x14}},
  [PATCH_CLEAR2]      = {0x197410, 31, 0,
      (const uint8_t [31]){0x90, 0x6a, 0x00, 0xff, 0x75, 0xec, 0x6a, 0x00,
                           0x6a, 0x02, 0x8d, 0x4d, 0x88, 0x51, 0x90, 0x90,
                           0x90, 0x90, 0x6a, 0x01, 0xa1, 0xbc, 0x15, 0x7b,
                           0x00, 0x50, 0x8b, 0x08, 0xff, 0x51, 0x50}},
  [PATCH_CLEAR_Z_16]  = {0x1973ce,  7, 1,
      (const uint8_t [ 7]){0xc7, 0x45, 0xec, 0xff, 0xff, 0x00, 0x00}},
  [PATCH_CLEAR_Z_24]  = {0x1973ce,  7, 0,
      (const uint8_t [ 7]){0xc7, 0x45, 0xec, 0xff, 0xff, 0xff, 0x00}},
  [PATCH_CLEAR_Z_32]  = {0x1973ce,  7, 0,
      (const uint8_t [ 7]){0xc7, 0x45, 0xec, 0xff, 0xff, 0xff, 0xff}},
  [PATCH_CLEAR_Z_F]   = {0x1973ce,  7, 0,
      (const uint8_t [ 7]){0xc7, 0x45, 0xec, 0x00, 0x00, 0x80, 0x3f}},
  [PATCH_STARS_ON]    = {0x10c8b7, 18, 1,
      (const uint8_t [18]){0xa1, 0x48, 0x33, 0x77, 0x00, 0x6a, 0x00, 0x8d,
                           0x4c, 0x24, 0x10, 0x6a, 0x00, 0x8b, 0x10, 0x51,
                           0x6a, 0x00}},
  [PATCH_STARS_OFF]   = {0x10c8b7, 18, 0,
      (const uint8_t [18]){0xa1, 0x48, 0x33, 0x77, 0x00, 0x6a, 0x00, 0x8d,
                           0x4c, 0x24, 0x10, 0x6a, 0x10, 0x8b, 0x10, 0x51,
                           0x6a, 0x00}},
  [PATCH_STAR_16_1]   = {0x0dc86b, 21, 1,
      (const uint8_t [21]){0x74, 0x15, 0x66, 0xb8, 0x21, 0x04, 0x66, 0xc7,
                           0x05, 0xdc, 0x1d, 0x76, 0x00, 0x00, 0x7c, 0x66,
                           0xa3, 0xd8, 0x1d, 0x76, 0x00}},
  [PATCH_STAR_32_1]   = {0x0dc86b, 21, 0,
      (const uint8_t [21]){0x90, 0xb8, 0x08, 0x08, 0x08, 0x00, 0x66, 0xc7,
                           0x05, 0xdc, 0x1d, 0x76, 0x00, 0x00, 0xff, 0x90,
                           0xa3, 0xd8, 0x1d, 0x76, 0x00}},
  [PATCH_STAR_32_1_2] = {0x0dc86b, 21, 0,
      (const uint8_t [21]){0x90, 0xb8, 0x08, 0x08, 0x08, 0x00, 0x66, 0xc7,
                           0x05, 0xdc, 0x1d, 0x76, 0x00, 0x00, 0xf8, 0x90,
                           0xa3, 0xd8, 0x1d, 0x76, 0x00}},
  [PATCH_STAR_16_2]   = {0x0dd14c,  5, 1,
      (const uint8_t [ 5]){0x03, 0xcf, 0x8d, 0x04, 0x41}},
  [PATCH_STAR_32_2]   = {0x0dd14c,  5, 0,
      (const uint8_t [ 5]){0x03, 0xcf, 0x8d, 0x04, 0x81}},
  [PATCH_STAR_16_3]   = {0x0dd176, 51, 1,
      (const uint8_t [51]){0x33, 0xc9, 0x8a, 0x8e, 0xd8, 0x6b, 0x75, 0x00,
                           0x8b, 0x0c, 0x8f, 0x33, 0xff, 0x66, 0x8b, 0x3c,
                           0x75, 0xd8, 0xd5, 0x74, 0x00, 0x8b, 0x69, 0x08,
                           0x8b, 0x49, 0x04, 0x03, 0xfd, 0x66, 0x8b, 0x0c,
                           0x79, 0x66, 0x8b, 0x3c, 0x75, 0xd8, 0x75, 0x75,
                           0x00, 0x51, 0x50, 0x66, 0x03, 0xf9, 0x52, 0x56,
                           0x66, 0x89, 0x38}},
  [PATCH_STAR_32_3]   = {0x0dd176, 51, 0,
      (const uint8_t [51]){0x0f, 0xb6, 0x8e, 0xd8, 0x6b, 0x75, 0x00, 0x8b,
                           0x0c, 0x8f, 0x0f, 0xb7, 0xbc, 0x36, 0xd8, 0xd5,
                           0x74, 0x00, 0x8b, 0x69, 0x08, 0x8b, 0x49, 0x04,
                           0x03, 0xfd, 0x66, 0x8b, 0x0c, 0x79, 0x66, 0x8b,
                           0x3c, 0x75, 0xd8, 0x75, 0x75, 0x00, 0x51, 0x50,
                           0x03, 0xf9, 0x52, 0x56, 0x66, 0x89, 0x38, 0x66,
                           0x89, 0x78, 0x02}},
  [PATCH_STAR_16_4]   = {0x0dd2b3,  5, 1,
      (const uint8_t [ 5]){0x03, 0xce, 0x8d, 0x04, 0x41}},
  [PATCH_STAR_32_4]   = {0x0dd2b3,  5, 0,
      (const uint8_t [ 5]){0x03, 0xce, 0x8d, 0x04, 0x81}},
  [PATCH_STAR_16_5]   = {0x0dd2dd, 51, 1,
      (const uint8_t [51]){0x33, 0xc9, 0x8a, 0x8d, 0xd8, 0x6b, 0x75, 0x00,
                           0x8b, 0x0c, 0x8e, 0x33, 0xf6, 0x66, 0x8b, 0x34,
                           0x6d, 0xd8, 0xd5, 0x74, 0x00, 0x8b, 0x59, 0x08,
                           0x8b, 0x49, 0x04, 0x03, 0xf3, 0x66, 0x8b, 0x0c,
                           0x71, 0x66, 0x8b, 0x34, 0x6d, 0xd8, 0x75, 0x75,
                           0x00, 0x51, 0x50, 0x66, 0x03, 0xf1, 0x52, 0x55,
                           0x66, 0x89, 0x30}},
  [PATCH_STAR_32_5]   = {0x0dd2dd, 51, 0,
      (const uint8_t [51]){0x0f, 0xb6, 0x8d, 0xd8, 0x6b, 0x75, 0x00, 0x8b,
                           0x0c, 0x8e, 0x0f, 0xb7, 0xb4, 0x2d, 0xd8, 0xd5,
                           0x74, 0x00, 0x8b, 0x59, 0x08, 0x8b, 0x49, 0x04,
                           0x03, 0xf3, 0x66, 0x8b, 0x0c, 0x71, 0x66, 0x8b,
                           0x34, 0x6d, 0xd8, 0x75, 0x75, 0x00, 0x51, 0x50,
                           0x03, 0xf1, 0x52, 0x55, 0x66, 0x89, 0x30, 0x66,
                           0x89, 0x70, 0x02}},
  [PATCH_STAR_16_6]   = {0x0dd4c0,  5, 1,
      (const uint8_t [ 5]){0x03, 0xcf, 0x8d, 0x04, 0x41}},
  [PATCH_STAR_32_6]   = {0x0dd4c0,  5, 0,
      (const uint8_t [ 5]){0x03, 0xcf, 0x8d, 0x04, 0x81}},
  [PATCH_STAR_16_7]   = {0x0dd4ea, 51, 1,
      (const uint8_t [51]){0x33, 0xc9, 0x8a, 0x8e, 0xd8, 0x6b, 0x75, 0x00,
                           0x8b, 0x0c, 0x8f, 0x33, 0xff, 0x66, 0x8b, 0x3c,
                           0x75, 0xd8, 0xd5, 0x74, 0x00, 0x8b, 0x69, 0x08,
                           0x8b, 0x49, 0x04, 0x03, 0xfd, 0x66, 0x8b, 0x0c,
                           0x79, 0x66, 0x8b, 0x3c, 0x75, 0xd8, 0x75, 0x75,
                           0x00, 0x51, 0x50, 0x66, 0x03, 0xf9, 0x52, 0x56,
                           0x66, 0x89, 0x38}},
  [PATCH_STAR_32_7]   = {0x0dd4ea, 51, 0,
      (const uint8_t [51]){0x0f, 0xb6, 0x8e, 0xd8, 0x6b, 0x75, 0x00, 0x8b,
                           0x0c, 0x8f, 0x0f, 0xb7, 0xbc, 0x36, 0xd8, 0xd5,
                           0x74, 0x00, 0x8b, 0x69, 0x08, 0x8b, 0x49, 0x04,
                           0x03, 0xfd, 0x66, 0x8b, 0x0c, 0x79, 0x66, 0x8b,
                           0x3c, 0x75, 0xd8, 0x75, 0x75, 0x00, 0x51, 0x50,
                           0x03, 0xf9, 0x52, 0x56, 0x66, 0x89, 0x38, 0x66,
                           0x89, 0x78, 0x02}},
  [PATCH_STAR_16_8]   = {0x0dd5d8, 13, 1,
      (const uint8_t [13]){0x66, 0x89, 0x41, 0x02, 0x66, 0x85, 0xd2, 0x76,
                           0x04, 0x66, 0x89, 0x41, 0xfe}},
  [PATCH_STAR_32_8]   = {0x0dd5d8, 13, 0,
      (const uint8_t [13]){0x90, 0x89, 0x41, 0x04, 0x66, 0x85, 0xd2, 0x76,
                           0x04, 0x90, 0x89, 0x41, 0xfc}},
  [PATCH_STAR_16_9]   = {0x0dd607,  26, 1,
      (const uint8_t [26]){0xd1, 0xee, 0x66, 0x89, 0x04, 0x71, 0x66, 0x85,
                           0xd2, 0x76, 0x72, 0x8b, 0x15, 0x58, 0xdc, 0x80,
                           0x00, 0xd1, 0xfa, 0xd1, 0xe2, 0x2b, 0xca, 0x66,
                           0x89, 0x01}},
  [PATCH_STAR_32_9]   = {0x0dd607,  26, 0,
      (const uint8_t [26]){0x83, 0xe6, 0xfc, 0x89, 0x04, 0x31, 0x66, 0x85,
                           0xd2, 0x76, 0x72, 0x8b, 0x15, 0x58, 0xdc, 0x80,
                           0x00, 0x83, 0xe2, 0xfc, 0x90, 0x2b, 0xca, 0x90,
                           0x89, 0x01}},
  [PATCH_STAR_16_10]  = {0x0dd636, 23, 1,
      (const uint8_t [23]){0x66, 0x89, 0x41, 0xfe, 0x76, 0x48, 0x8b, 0x15,
                           0x58, 0xdc, 0x80, 0x00, 0x8b, 0xf1, 0xd1, 0xfa,
                           0xd1, 0xe2, 0x2b, 0xf2, 0x66, 0x89, 0x06}},
  [PATCH_STAR_32_10]  = {0x0dd636, 23, 0,
      (const uint8_t [23]){0x90, 0x89, 0x41, 0xfc, 0x76, 0x48, 0x8b, 0x15,
                           0x58, 0xdc, 0x80, 0x00, 0x8b, 0xf1, 0x83, 0xe2,
                           0xfc, 0x90, 0x2b, 0xf2, 0x90, 0x89, 0x06}},
  [PATCH_STAR_16_11]  = {0x0dd65b, 11, 1,
      (const uint8_t [11]){0xd1, 0xfa, 0x8d, 0x54, 0x12, 0x02, 0x2b, 0xca,
                           0x66, 0x89, 0x01}},
  [PATCH_STAR_32_11]  = {0x0dd65b, 11, 0,
      (const uint8_t [11]){0x83, 0xe2, 0xfc, 0x90, 0x90, 0x90, 0x2b, 0xca,
                           0x89, 0x41, 0xfc}},
  [PATCH_STAR_16_12]  = {0x0dd67b,  8, 1,
      (const uint8_t [ 8]){0xd1, 0xf9, 0xd1, 0xe1, 0x2b, 0xd1, 0x66, 0x89}},
  [PATCH_STAR_32_12]  = {0x0dd67b,  8, 0,
      (const uint8_t [ 8]){0x83, 0xe1, 0xfc, 0x90, 0x2b, 0xd1, 0x90, 0x89}},
  [PATCH_STAR_16_13]  = {0x0dd653,  4, 1,
      (const uint8_t [ 4]){0x2b, 0xc2, 0x8b, 0x15}},
  [PATCH_STAR_32_13]  = {0x0dd653,  4, 0,
      (const uint8_t [ 4]){0x90, 0x90, 0x8b, 0x15}},
  [PATCH_CD_CHECK]    = {0x12a3e5, 11, 1,
      (const uint8_t [11]){0x83, 0xec, 0x50, 0x84, 0xc0, 0x56, 0x57, 0x75,
                           0x08, 0x33, 0xc0}},
  [PATCH_NO_CD_CHECK] = {0x12a3e5, 11, 0,
      (const uint8_t [11]){0x83, 0xec, 0x50, 0x84, 0xc0, 0x56, 0x57, 0x33,
                           0xc0, 0x40, 0x90}},
  [PATCH_CD_VOICE]    = {0x15772e, 4, 1,
      (const uint8_t [ 4]){0x75, 0x32, 0xe8, 0xdb}},
  [PATCH_HD_VOICE]    = {0x15772e, 4, 0,
      (const uint8_t [ 4]){0x90, 0x90, 0xe8, 0xdb}},
  [PATCH_SELECT_RES]  = {0x10a327, 13, 1,
      (const uint8_t [13]){0x0f, 0x87, 0xb0, 0x00, 0x00, 0x00, 0xff, 0x24,
                           0x85, 0xa0, 0xb5, 0x50, 0x00}},
  [PATCH_FORCE_RES]   = {0x10a327, 13, 0,
      (const uint8_t [13]){0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
                           0x90, 0x90, 0x90, 0x90, 0x90}},
  [PATCH_SELECT_RES_2]= {0x10f415, 6, 1,
      (const uint8_t [ 6]){0x0f, 0x87, 0x07, 0x01, 0x00, 0x00}},
  [PATCH_FORCE_RES_2] = {0x10f415, 6, 0,
      (const uint8_t [ 6]){0xeb, 0x38, 0x90, 0x90, 0x90, 0x90}},
  [PATCH_PRTSCR_8_1]  = {0x136a42,  3, 1,
      (const uint8_t [ 3]){0x83, 0xfb, 0x08}},
  [PATCH_PRTSCR_32_1] = {0x136a42,  3, 0,
      (const uint8_t [ 3]){0x83, 0xfb, 0x20}},
  [PATCH_PRTSCR_8_2]  = {0x136bd3, 32, 1,
      (const uint8_t [32]){0x8a, 0x14, 0x3b, 0x33, 0xc0, 0x8a, 0x44, 0x95,
                           0x00, 0x50, 0x56, 0xe8, 0x3d, 0x37, 0xff, 0xff,
                           0x0f, 0xbf, 0xc0, 0x83, 0xc4, 0x08, 0x85, 0xc0,
                           0x0f, 0x84, 0x38, 0x01, 0x00, 0x00, 0x33, 0xc9}},
  [PATCH_PRTSCR_32_2] = {0x136bd3, 32, 0,
      (const uint8_t [32]){0x8d, 0x04, 0x9f, 0x90, 0x90, 0x56, 0x6a, 0x01,
                           0x6a, 0x03, 0x50, 0xe8, 0x7d, 0x44, 0x06, 0x00,
                           0x0f, 0xbf, 0xc0, 0x83, 0xc4, 0x10, 0x85, 0xc0,
                           0x0f, 0x84, 0x38, 0x01, 0x00, 0x00, 0xeb, 0x3e}},

  [PATCH_TIE95_BLT_CLEAR]   = {0x0c76b8, 30, 1,
      (const uint8_t [30]){0xa1, 0x04, 0x1e, 0x6b, 0x00, 0x8d, 0x54, 0x24,
                           0x14, 0x52, 0x68, 0x00, 0x00, 0x00, 0x03, 0x8b,
                           0x08, 0x6a, 0x00, 0x8d, 0x54, 0x24, 0x10, 0x6a,
                           0x00, 0x52, 0x50, 0xff, 0x51, 0x14}},
  [PATCH_TIE95_CLEAR2]      = {0x0c76b8, 30, 0,
      (const uint8_t [30]){0xa1, 0x1c, 0xc1, 0x58, 0x00, 0x8d, 0x54, 0x24,
                           0x14, 0x6a, 0x00, 0x6a, 0x00, 0x6a, 0x00, 0x8b,
                           0x08, 0x6a, 0x02, 0x8d, 0x54, 0x24, 0x14, 0x52,
                           0x6a, 0x01, 0x50, 0xff, 0x51, 0x50}},

  [PATCH_XWING95_BLT_CLEAR]   = {0x0b318b, 31, 1,
      (const uint8_t [31]){0x8d, 0x44, 0x24, 0x14, 0x8d, 0x4c, 0x24, 0x04,
                           0x50, 0x8b, 0x15, 0xa4, 0xce, 0x63, 0x00, 0x68,
                           0x00, 0x00, 0x00, 0x03, 0x6a, 0x00, 0x6a, 0x00,
                           0x8b, 0x02, 0x51, 0x52, 0xff, 0x50, 0x14}},
  [PATCH_XWING95_CLEAR2]      = {0x0b318b, 31, 0,
      (const uint8_t [31]){0x31, 0xc0, 0x90, 0x90, 0x8d, 0x4c, 0x24, 0x04,
                           0x50, 0x8b, 0x15, 0x24, 0x70, 0x56, 0x00, 0x90,
                           0x6a, 0x00, 0x6a, 0x00, 0x6a, 0x02, 0x51, 0x6a,
                           0x01, 0x8b, 0x02, 0x52, 0xff, 0x50, 0x50}},

  [PATCH_XVTBOP_BLT_CLEAR]   = {0x0b5e1d, 42, 1,
      (const uint8_t [42]){0x64, 0x00, 0x00, 0x00, 0xc7, 0x45, 0xe8, 0xff,
                           0xff, 0x00, 0x00, 0x8d, 0x45, 0x98, 0x50, 0x68,
                           0x00, 0x00, 0x00, 0x03, 0x6a, 0x00, 0x6a, 0x00,
                           0x6a, 0x00, 0xa1, 0xcc, 0xee, 0x64, 0x00, 0x50,
                           0xa1, 0xcc, 0xee, 0x64, 0x00, 0x8b, 0x00, 0xff,
                           0x50, 0x14}},
  [PATCH_XVTBOP_BROKEN]      = {0x0b5e1d, 42, 0,
      (const uint8_t [42]){0x64, 0x00, 0x00, 0x00, 0xc7, 0x45, 0xe8, 0xff,
                           0xff, 0x00, 0x00, 0x8d, 0x45, 0x98, 0x6a, 0x00,
                           0x6a, 0x00, 0x6a, 0x00, 0x6a, 0x02, 0x6a, 0x00,
                           0x6a, 0x00, 0xa1, 0x44, 0xee, 0x64, 0x00, 0x50,
                           0xa1, 0x44, 0xee, 0x64, 0x00, 0x8b, 0x00, 0xff,
                           0x50, 0x50}},
  [PATCH_XVTBOP_CLEAR2]      = {0x0b5e1d, 42, 0,
      (const uint8_t [42]){0xb9, 0x00, 0x40, 0x00, 0x00, 0x89, 0x48, 0x0c,
                           0x89, 0x48, 0x08, 0x31, 0xc9, 0x89, 0x48, 0x04,
                           0x89, 0x08, 0x51, 0x68, 0x00, 0x00, 0x80, 0x3f,
                           0x90, 0x51, 0x6a, 0x02, 0x50, 0x6a, 0x01, 0xa1,
                           0x44, 0xee, 0x64, 0x00, 0x50, 0x8b, 0x00, 0xff,
                           0x50, 0x50}},
  [PATCH_XVTBOP_JS_CHECK]    = {0x0be33d, 4, 1,
      (const uint8_t [ 4]){0x85, 0xc0, 0x75, 0x47}},
  [PATCH_XVTBOP_NO_JS_CHECK] = {0x0be33d, 4, 0,
      (const uint8_t [ 4]){0x85, 0xc0, 0xeb, 0x47}},

  [PATCH_BOP_BLT_CLEAR]      = {0x10ddf3, 23, 1,
      (const uint8_t [23]){0x50, 0x8b, 0x15, 0xe4, 0x6b, 0x88, 0x00, 0x68,
                           0x00, 0x00, 0x00, 0x03, 0x6a, 0x00, 0x6a, 0x00,
                           0x8b, 0x02, 0x51, 0x52, 0xff, 0x50, 0x14}},
  [PATCH_BOP_CLEAR2]         = {0x10ddf3, 23, 0,
      (const uint8_t [23]){0x90, 0x8b, 0x15, 0xc4, 0x6a, 0x66, 0x00, 0x31,
                           0xc0, 0x50, 0x50, 0x50, 0x6a, 0x02, 0x51, 0x6a,
                           0x01, 0x8b, 0x02, 0x52, 0xff, 0x50, 0x50}},
  [PATCH_BOP_RANDCD]         = {0x0bfcec, 6, 1,
      (const uint8_t [ 6]){0x0f, 0x85, 0xd4, 0x00, 0x00, 0x00}},
  [PATCH_BOP_XVTCD]          = {0x0bfcec, 6, 0,
      (const uint8_t [ 6]){0x67, 0xe9, 0xd4, 0x00, 0x00, 0x00}},

  [PATCH_BOP_EN_BLT_CLEAR]   = {0x0b2713, 23, 1,
      (const uint8_t [23]){0x50, 0x8b, 0x15, 0x34, 0x1a, 0xa9, 0x00, 0x68,
                           0x00, 0x00, 0x00, 0x03, 0x6a, 0x00, 0x6a, 0x00,
                           0x8b, 0x02, 0x51, 0x52, 0xff, 0x50, 0x14}},
  [PATCH_BOP_EN_CLEAR2]      = {0x0b2713, 23, 0,
      (const uint8_t [23]){0x90, 0x8b, 0x15, 0x74, 0x49, 0x66, 0x00, 0x31,
                           0xc0, 0x50, 0x50, 0x50, 0x6a, 0x02, 0x51, 0x6a,
                           0x01, 0x8b, 0x02, 0x52, 0xff, 0x50, 0x50}},
  [PATCH_BOP_EN_RANDCD]      = {0x0bd7dc, 6, 1,
      (const uint8_t [ 6]){0x0f, 0x85, 0xd4, 0x00, 0x00, 0x00}},
  [PATCH_BOP_EN_XVTCD]       = {0x0bd7dc, 6, 0,
      (const uint8_t [ 6]){0x67, 0xe9, 0xd4, 0x00, 0x00, 0x00}},

  [PATCH_NO_MSGLOOP]         = {0x10ce98, 8, 1,
      (const uint8_t [ 8]){0x5d, 0xc3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90}},
  [PATCH_ADD_MSGLOOP]        = {0x10ce98, 8, 0,
      (const uint8_t [ 8]){0x5d, 0x67, 0xe8, 0xe1, 0x2f, 0x03, 0x00, 0xc3}},

  [PATCH_NO_HIDE_CD_DRIVES]  = {0x12a55b, 8, 1,
      (const uint8_t [ 8]){0x83, 0xf8, 0x05, 0x75, 0x70, 0x83, 0xc9, 0xff}},
  [PATCH_HIDE_CD_DRIVES]     = {0x12a55b, 8, 0,
      (const uint8_t [ 8]){0x83, 0xf8, 0x05, 0xeb, 0x70, 0x83, 0xc9, 0xff}},

  [PATCH_BOP_GOG_DISABLE_3D] = {0xd36b7, 20, 1,
      (const uint8_t [20]){0xe8, 0x64, 0x19, 0x00, 0x00, 0x6a, 0x00, 0x68,
                           0x2e, 0x7b, 0xaa, 0x00, 0x6a, 0x02, 0x90, 0xe8,
                           0xe5, 0x2b, 0x04, 0x00}},
  [PATCH_BOP_GOG_ENABLE_3D] = {0xd36b7, 20, 0,
      (const uint8_t [20]){0xe8, 0x64, 0x19, 0x00, 0x00, 0x8b, 0xf0, 0x6a,
                           0x00, 0x68, 0x2e, 0x7b, 0xaa, 0x00, 0x56, 0xe8,
                           0xe5, 0x2b, 0x04, 0x00}},
  [PATCH_XVT_GOG_DISABLE_3D] = {0xd3ae2, 20, 1,
      (const uint8_t [20]){0xe8, 0xf9, 0x19, 0x00, 0x00, 0x6a, 0x00, 0x68,
                           0x63, 0x96, 0x75, 0x00, 0x6a, 0x02, 0x90, 0xe8,
                           0x5e, 0x57, 0x04, 0x00}},
  [PATCH_XVT_GOG_ENABLE_3D] = {0xd3ae2, 20, 0,
      (const uint8_t [20]){0xe8, 0xf9, 0x19, 0x00, 0x00, 0x8b, 0xf0, 0x6a,
                           0x00, 0x68, 0x63, 0x96, 0x75, 0x00, 0x56, 0xe8,
                           0x5e, 0x57, 0x04, 0x00}},
};

struct collection {
  const char *name;
  enum PATCHES patches[18];
};

static const struct collection xwa_collections[] = {
  {"16 bit rendering",
    {PATCH_16BIT_FB, PATCH_STAR_16_1, PATCH_STAR_16_2, PATCH_STAR_16_3, PATCH_STAR_16_4,
     PATCH_STAR_16_5, PATCH_STAR_16_6, PATCH_STAR_16_7, PATCH_STAR_16_8, PATCH_STAR_16_9,
     PATCH_STAR_16_10, PATCH_STAR_16_11, PATCH_STAR_16_12, PATCH_STAR_16_13,
     PATCH_PRTSCR_8_1, PATCH_PRTSCR_8_2, NO_PATCH}},
  {"32 bit rendering",
    {PATCH_32BIT_FB, PATCH_STAR_32_1_2, PATCH_STAR_32_2, PATCH_STAR_32_3, PATCH_STAR_32_4,
     PATCH_STAR_32_5, PATCH_STAR_32_6, PATCH_STAR_32_7, PATCH_STAR_32_8, PATCH_STAR_32_9,
     PATCH_STAR_32_10, PATCH_STAR_32_11, PATCH_STAR_32_12, PATCH_STAR_32_13,
     PATCH_PRTSCR_32_1, PATCH_PRTSCR_32_2, NO_PATCH}},
  {"original Z-buffer clear",
    {PATCH_BLT_CLEAR, PATCH_CLEAR_Z_16, NO_PATCH}},
  {"fixed Z-buffer clear",
    {PATCH_CLEAR2, PATCH_CLEAR_Z_F, NO_PATCH}},
  {"allow choosing resolution",
    {PATCH_SELECT_RES, PATCH_SELECT_RES_2, NO_PATCH}},
  {"force 800x600 resolution",
    {PATCH_FORCE_RES, PATCH_FORCE_RES_2, NO_PATCH}},
  {"use CD",
    {PATCH_CD_CHECK, PATCH_CD_VOICE, PATCH_NO_HIDE_CD_DRIVES, NO_PATCH}},
  {"play from installation directory",
    {PATCH_NO_CD_CHECK, PATCH_HD_VOICE, PATCH_HIDE_CD_DRIVES, NO_PATCH}},
  {NULL}
};

static const struct binary {
  const char *name;
  const char *filename;
  const enum PATCHES *patchgroups;
  const struct collection *collections;
} binaries[] = {
  {"X-Wing Alliance 2.02", "xwingalliance.exe", xwa_patchgroups, xwa_collections},
  {"TIE Fighter 95", "TIE95.EXE", tie95_patchgroups, NULL},
  {"X-Wing 95", "XWING95.EXE", xwing95_patchgroups, NULL},
  {"X-Wing vs. TIE Fighter", "Z_XVT__.EXE", xvtbop_patchgroups, NULL},
  {"Balance of Power (DE)", "z_xvt__.EXE", bop_patchgroups, NULL},
  {"Balance of Power (EN)", "z_xvt__.EXE", bop_en_patchgroups, NULL},
  {"X-Wing vs. TIE Fighter (GOG)", "Z_XVT__.EXE", xvt_gog_patchgroups, NULL},
  {"Balance of Power (GOG)", "z_xvt__.EXE", bop_gog_patchgroups, NULL},
  {NULL}
};

static const struct metapatch {
  const char *name;
  enum PATCHES patches[10];
} metapatches[] = {
  {"Z-buffer clear via Surface::Blt",
    {PATCH_BLT_CLEAR, PATCH_TIE95_BLT_CLEAR, PATCH_XWING95_BLT_CLEAR,
     PATCH_XVTBOP_BLT_CLEAR, PATCH_BOP_BLT_CLEAR, PATCH_BOP_EN_BLT_CLEAR,
     NO_PATCH}
  },
  {"Z-buffer clear via Viewport::Clear2",
    {PATCH_CLEAR2, PATCH_TIE95_CLEAR2, PATCH_XWING95_CLEAR2,
     PATCH_XVTBOP_CLEAR2, PATCH_BOP_CLEAR2, PATCH_BOP_EN_CLEAR2,
     NO_PATCH}
  },
  {"Hardware 3D disabled",
    {PATCH_BOP_GOG_DISABLE_3D, PATCH_XVT_GOG_DISABLE_3D, NO_PATCH}
  },
  {"Hardware 3D enabled",
    {PATCH_BOP_GOG_ENABLE_3D, PATCH_XVT_GOG_ENABLE_3D, NO_PATCH}
  },
  {NULL}
};

static int check_patch(uint8_t *buffer, FILE *f, enum PATCHES patch, int silent) {
  const struct patchdesc *p = &patchdescs[patch];
  int match;
  if (DEBUG) printf("Checking for patch %i\n", patch);
  if (!read_buffer(buffer, f, p->offset, p->len)) {
    if (!silent)
      printf("Read error while checking for patch %i\n", patch);
    return 0;
  }
  match = memcmp(buffer, p->value, p->len) == 0;
  if (DEBUG && !match) {
    int i = 0;
    while (buffer[i] == p->value[i]) i++;
    printf("Differing byte %i: 0x%x instead of 0x%x\n", i, buffer[i], p->value[i]);
  }
  return match;
}

static int count_patches(uint8_t *buffer, FILE *f, const enum PATCHES *patchgroups) {
  int i = 0;
  int count = 0;
  while (patchgroups[i] != NO_PATCH) {
    for (; patchgroups[i] != NO_PATCH; i++)
      if (check_patch(buffer, f, patchgroups[i], 1))
        count++;
    i++;
  }
  return count;
}

static int num_patchgroups(const enum PATCHES *patchgroups) {
  int i = 0;
  int count = 0;
  while (patchgroups[i] != NO_PATCH) {
    for (; patchgroups[i] != NO_PATCH; i++) /* */;
    count++;
    i++;
  }
  return count;
}

static void list_patches(const enum PATCHES *patchgroups) {
  int i = 0;
  int group = 1;
  printf("number : description\n");
  while (patchgroups[i] != NO_PATCH) {
    printf("Patch group %i:\n", group);
    for (; patchgroups[i] != NO_PATCH; i++) {
      enum PATCHES p = patchgroups[i];
      const char *comment = patchdescs[p].original ? " (unmodified original)" : "";
      printf("%4i : %s%s\n", p, patchnames[p], comment);
    }
    printf("\n");
    i++;
    group++;
  }
}

static int num_collections(const struct collection *collections) {
  int i = 0;
  while (collections[i].name) i++;
  return i;
}

static void list_collections(const struct collection *collections) {
  int i, j;
  for (i = 0; collections[i].name; i++) {
    printf("%3i : %s : %i", i, collections[i].name, collections[i].patches[0]);
    for (j = 1; collections[i].patches[j] != NO_PATCH; j++)
      printf(", %i", collections[i].patches[j]);
    printf("\n");
  }
}

static int num_metapatches(void) {
  int i = 0;
  while (metapatches[i].name) i++;
  return i;
}

static void list_metapatches(void) {
  int i;
  for (i = 0; metapatches[i].name; i++)
    printf("%3i : %s\n", i, metapatches[i].name);
}

static const enum PATCHES *find_patchgroup(const enum PATCHES *patchgroups, enum PATCHES p) {
  int i = 0;
  while (patchgroups[i] != NO_PATCH) {
    const enum PATCHES *group = &patchgroups[i];
    for (; patchgroups[i] != NO_PATCH; i++)
      if (patchgroups[i] == p) return group;
    i++;
  }
  return NULL;
}

static int apply_patch(uint8_t *buffer, FILE *f, const enum PATCHES *patchgroups, enum PATCHES patch) {
  const struct patchdesc *p = &patchdescs[patch];
  enum PATCHES previous = NO_PATCH;
  int i;
  const enum PATCHES *group = find_patchgroup(patchgroups, patch);
  assert(group);
  for (i = 0; group[i] != NO_PATCH; i++) {
    if (check_patch(buffer, f, group[i], 0)) {
      previous = group[i];
      break;
    }
  }
  if (previous == NO_PATCH) {
    printf("Could not find the previous patch state in patch group, no changes made\n");
    goto fail;
  }
  if (!write_buffer(p->value, f, p->offset, p->len)) {
    printf("Write failed while patching\n");
    goto fail;
  }
  printf("Patched from %i to %i\n", previous, patch);
  return 1;

fail:
  printf("Failed to apply patch %i\n", patch);
  return 0;
}

static int apply_collection(uint8_t *buffer, FILE *f, const struct binary *binary, int c) {
  int i;
  const struct collection *collections = binary->collections;
  for (i = 0; collections[c].patches[i] != NO_PATCH; i++) {
    if (!apply_patch(buffer, f, binary->patchgroups, collections[c].patches[i]))
      return 0;
  }
  return 1;
}

static int apply_metapatch(uint8_t *buffer, FILE *f, const struct binary *binary, int m) {
  int i;
  int applied = 0;
  int valid = 0;
  for (i = 0; metapatches[m].patches[i] != NO_PATCH; i++) {
    enum PATCHES p = metapatches[m].patches[i];
    if (!find_patchgroup(binary->patchgroups, p))
      continue;
    valid++;
    if (apply_patch(buffer, f, binary->patchgroups, p))
      applied++;
  }
  return valid && applied == valid;
}

static const int max_fps_offset = 0x13d402;
static const int max_fps_idx[] = {0x13d403 - 0x13d402, 0x13d418 - 0x13d402};
static int get_max_fps(uint8_t *buffer, FILE *f) {
  if (!read_buffer(buffer, f, max_fps_offset, max_fps_idx[1] + 1))
    return -1;
  if (buffer[max_fps_idx[0] - 1] != 0x6a ||
      buffer[max_fps_idx[1] - 1] != 0x6a)
    return -1;
  if (buffer[max_fps_idx[0]] != buffer[max_fps_idx[1]])
    return -1;
  return buffer[max_fps_idx[0]];
}

static int set_max_fps(uint8_t *buffer, FILE *f, int fps) {
  if (get_max_fps(buffer, f) < 0)
    return 0;
  if (fps < 0 || fps > 255)
    return 0;
  buffer[max_fps_idx[0]] = fps;
  buffer[max_fps_idx[1]] = fps;
  if (!write_buffer(buffer, f, max_fps_offset, max_fps_idx[1] + 1))
    return 0;
  return 1;
}

struct resopts {
  int w;
  int h;
  union {
    uint32_t i;
    float f;
  } hud_scale;
  int fov;
};

static const float pi = 3.14159265358979323846;

static int default_fov(int height) {
  int fov = height * 1.0666 + 0.5;
  return fov > 0 ? fov : 1;
}

static float default_hud_scale(int height) {
  float res = height / 600.0;
  return res > 1.0 ? res : 1.0;
}

static float fov2deg(int fov, int height) {
  return atan((float)height / fov) * 2 / pi * 180;
}

static int deg2fov(float deg, int height) {
  int fov = height / tan(deg / 180 * pi / 2) + 0.5;
  return fov > 0 ? fov : 1;
}

static void read_res(uint8_t *buffer, FILE *f, struct resopts res[NUM_RES]) {
  int i;
  for (i = 0; i < NUM_RES; i++) {
    res[i].w = res[i].h = res[i].fov = -1;
    res[i].hud_scale.i = 0xffffffffu;
    if (read_buffer(buffer, f, resdes[i].offset, 10)) {
      if (buffer[0] == 0xb8) res[i].w = RL32(buffer + 1);
      if (buffer[5] == 0xb9) res[i].h = RL32(buffer + 6);
    }
    if (read_buffer(buffer, f, resdes[i].fov_offset, 20)) {
      static const uint8_t instr[2][6] = {
        {0xc7, 0x05, 0xb8, 0x02, 0x60, 0x00},
        {0xc7, 0x05, 0x6c, 0xab, 0x91, 0x00}
      };
      if (memcmp(buffer, instr[0], 6) == 0)
        res[i].hud_scale.i = RL32(buffer + 6);
      if (memcmp(buffer + 10, instr[1], 6) == 0)
        res[i].fov = RL32(buffer + 16);
    }
  }
}

static int write_res(uint8_t *buffer, FILE *f, const struct resopts *newval, int num,
                     int skip_hud_scale, int skip_deg) {
  buffer[0] = 0xb8; buffer[5] = 0xb9;
  WL32(buffer + 1, newval->w); WL32(buffer + 6, newval->h);
  if (!write_buffer(buffer, f, resdes[num].offset, 10)) {
    printf("Error writing new resolutions to file\n");
    return 0;
  }
  WL32(buffer, newval->hud_scale.i);
  WL32(buffer + 4, newval->fov);
  if ((!skip_hud_scale && !write_buffer(buffer, f, resdes[num].fov_offset + 6, 4)) ||
      (!skip_deg && !write_buffer(buffer + 4, f, resdes[num].fov_offset + 16, 4))) {
    printf("Error fixing up fov/HUD scale\n");
    return 0;
  }
  return 1;
}

#ifndef GUI
static const char optionhelp[] =
  "Options:\n"
  "  -l             : List available patches\n"
  "  -m             : List available metapatches\n"
  "  -m <n>         : Apply metapatch number <n>\n"
  "  -c             : List available patch collections (XWA only)\n"
  "  -c <n>         : Apply patch collection <n> (XWA only)\n"
  "  -p <n>         : Apply patch number <n>\n"
  "  -r             : List resolution settings (XWA only)\n"
  "  -r <n> <w> <h> [<s> [<f>]]\n"
  "                 : Redirect resolution <n> to <w>x<h> (XWA only)\n"
  "                   optionally specify a HUD scale factor (s)\n"
  "                   and the vertical field of view (f)\n"
  "  -f             : Show current max FPS limit (XWA only)\n"
  "  -f <f>         : Set max FPS limit to <f> (XWA only)\n"
;

static void print_help(const char *prog) {
  printf("Usage: %s xwingalliance.exe [option]\n", prog);
  printf(optionhelp);
}

static int parse_num(const char *s, int limit) {
  char *end;
  long int num = strtol(s, &end, 10);
  if (*end || num < 0 || num >= limit)
    return -1;
  return num;
}

static float parse_float(const char *s, float min, float max) {
  char *end;
  double num = strtod(s, &end);
  if (*end || num < min || num > max)
    return -1.0;
  return num;
}

int main(int argc, char *argv[]) {
  int binary_best_pos = 0;
  int binary_best_count = 0;
  struct resopts resolutions[NUM_RES];
  int detected_patches[NUM_PATCHES];
  uint8_t buffer[BUFFER_SZ];
  FILE *xwa = 0;
  int is_xwa;
  int i;
  int res = 1;
  enum PATCHES p;
  const struct binary *binary;
  const char *prog = argc > 0 ? argv[0] : "xwahacker";

  if (argc < 2) {
    print_help(prog);
    return 1;
  }

  xwa = fopen(argv[1], "r+b");
  if (!xwa) {
    printf("Could not open file %s: %s\n", argv[1], strerror(errno));
    return 1;
  }

  // check with which binary description we get the most patch matches
  for (i = 0; binaries[i].name; i++) {
    int count = count_patches(buffer, xwa, binaries[i].patchgroups);
    if (count > binary_best_count) {
      binary_best_count = count;
      binary_best_pos   = i;
    }
  }
  binary = &binaries[binary_best_pos];
  if (binary_best_count > 0)
    printf("Detected file as %s with %i matches (of %i)\n",
           binary->name, binary_best_count, num_patchgroups(binary->patchgroups));
  else
    printf("Could not detect file, assuming it is %s\n", binary->name);
  rewind(xwa);
  is_xwa = binary_best_pos == 0;

  read_res(buffer, xwa, resolutions);

  if (argc >= 3) {
    const char *opt = argv[2];
    if (argc == 3 && strcmp(opt, "-l") == 0) {
      list_patches(binary->patchgroups);
      res = 0;
      goto cleanup;
    } else if (argc == 3 && strcmp(opt, "-c") == 0 && binary->collections) {
      list_collections(binary->collections);
      res = 0;
      goto cleanup;
    } else if (argc == 3 && strcmp(opt, "-m") == 0) {
      list_metapatches();
      res = 0;
      goto cleanup;
    } else if (argc == 3 && strcmp(opt, "-f") == 0 && is_xwa) {
      int fps = get_max_fps(buffer, xwa);
      if (fps < 0)
        printf("Could not read out current max FPS limit\n");
      else
        printf("Current max FPS limit is %i (unmodified original: 24)\n", fps);
      goto cleanup;
    } else if (argc == 4 && strcmp(opt, "-f") == 0 && is_xwa) {
      int fps = parse_num(argv[3], 255);
      if (fps <= 0)
        printf("Invalid new max FPS limit value\n");
      else if (!set_max_fps(buffer, xwa, fps))
        printf("Failed setting new max FPS limit value\n");
      goto cleanup;
    } else if (argc == 3 && strcmp(opt, "-r") == 0 && is_xwa) {
      printf("Resolutions:\n");
      for (i = 0; i < NUM_RES; i++)
        printf("%i: %5i x %5i mapped to %5i x %5i, fov: %.2f deg%s, HUD scale %f%s\n",
               i, resdes[i].width, resdes[i].height,
               resolutions[i].w, resolutions[i].h,
               fov2deg(resolutions[i].fov, resolutions[i].h),
               resolutions[i].fov == default_fov(resolutions[i].h) ? " (default)" : "",
               resolutions[i].hud_scale.f,
               resolutions[i].hud_scale.f == default_hud_scale(resolutions[i].h) ? " (default)" : "");
      res = 0;
      goto cleanup;
    } else if (argc >= 6 && argc <= 8 && strcmp(opt, "-r") == 0 && is_xwa) {
      int num = parse_num(argv[3], NUM_RES);
      int w = parse_num(argv[4], 10000);
      int h = parse_num(argv[5], 10000);
      float hud_scale = argc > 6 ? parse_float(argv[6], 0.1, 10) : -1;
      int skip_hud_scale = argc > 6 && argv[6][0] == 'k';
      float deg = argc > 7 ? parse_float(argv[7], 10, 170) : -1;
      int skip_deg = argc > 7 && argv[7][0] == 'k';
      if (num < 0 || w < -1 || h < -1) {
        printf("Incorrect resolution values\n");
        goto cleanup;
      }
      if (resolutions[num].w < 0 || resolutions[num].h < 0 ||
          resolutions[num].fov < 0 || resolutions[num].hud_scale.i == 0xffffffffu) {
        printf("Could not detect current values, aborting\n");
        goto cleanup;
      }
      if (w > 0) resolutions[num].w = w;
      if (h > 0) resolutions[num].h = h;
      resolutions[num].hud_scale.f = hud_scale > 0 ? hud_scale : default_hud_scale(h);
      resolutions[num].fov = deg > 0 ? deg2fov(deg, h) : default_fov(h);

      if (!write_res(buffer, xwa, resolutions + num, num, skip_hud_scale, skip_deg))
        goto cleanup;
      printf("Updated resolution %i to map to %5i x %5i\n", num, w, h);
      res = 0;
      goto cleanup;
    } else if (argc == 4 && strcmp(opt, "-p") == 0) {
      int num = parse_num(argv[3], NUM_PATCHES);
      if (num < 0) {
        printf("Incorrect patch number\n");
        goto cleanup;
      }
      if (!apply_patch(buffer, xwa, binary->patchgroups, num))
        printf("Patching failed\n");
      else
        res = 0;
      goto cleanup;
    } else if (argc == 4 && strcmp(opt, "-c") == 0 && binary->collections) {
      int num = parse_num(argv[3], num_collections(binary->collections));
      if (num < 0) {
        printf("Incorrect collection number\n");
        goto cleanup;
      }
      if (!apply_collection(buffer, xwa, binary, num))
        printf("Patching failed\n");
      else
        res = 0;
      goto cleanup;
    } else if (argc == 4 && strcmp(opt, "-m") == 0) {
      int num = parse_num(argv[3], num_metapatches());
      if (num < 0) {
        printf("Incorrect metapatch number\n");
        goto cleanup;
      }
      if (!apply_metapatch(buffer, xwa, binary, num))
        printf("Patching failed\n");
      else
        res = 0;
      goto cleanup;
    } else {
      printf("Wrong option %s\n", opt);
      print_help(prog);
      goto cleanup;
    }
  }

  for (p = FIRST_PATCH; p < NUM_PATCHES; p++)
    detected_patches[p] = check_patch(buffer, xwa, p, 1);
  printf("Detected patches:\n");
  for (p = FIRST_PATCH; p < NUM_PATCHES; p++) {
    if (detected_patches[p]) {
      printf("%s", patchnames[p]);
      if (patchdescs[p].original) printf(" (i.e. unmodified)");
      printf("\n");
    }
  }

  res = 0;

cleanup:
  fclose(xwa);
  return res;
}
#endif
