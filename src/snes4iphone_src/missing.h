/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _MISSING_H_
#define _MISSING_H_

struct HDMA
{
    uint8 used;
    uint8 bbus_address;
    uint8 abus_bank;
    uint16 abus_address;
    uint8 indirect_address;
    uint8 force_table_address_write;
    uint8 force_table_address_read;
    uint8 line_count_write;
    uint8 line_count_read;
};

struct Missing
{
    uint8 emulate6502;
    uint8 decimal_mode;
    uint8 mv_8bit_index;
    uint8 mv_8bit_acc;
    uint8 interlace;
    uint8 lines_239;
    uint8 pseudo_512;
    struct HDMA hdma [8];
    uint8 modes [8];
    uint8 mode7_fx;
    uint8 mode7_flip;
    uint8 mode7_bgmode;
    uint8 direct;
    uint8 matrix_multiply;
    uint8 oam_read;
    uint8 vram_read;
    uint8 cgram_read;
    uint8 wram_read;
    uint8 dma_read;
    uint8 vram_inc;
    uint8 vram_full_graphic_inc;
    uint8 virq;
    uint8 hirq;
    uint16 virq_pos;
    uint16 hirq_pos;
    uint8 h_v_latch;
    uint8 h_counter_read;
    uint8 v_counter_read;
    uint8 fast_rom;
    uint8 window1 [6];
    uint8 window2 [6];
    uint8 sprite_priority_rotation;
    uint8 subscreen;
    uint8 subscreen_add;
    uint8 subscreen_sub;
    uint8 fixed_colour_add;
    uint8 fixed_colour_sub;
    uint8 mosaic;
    uint8 sprite_double_height;
    uint8 dma_channels;
    uint8 dma_this_frame;
    uint8 oam_address_read;
    uint8 bg_offset_read;
    uint8 matrix_read;
    uint8 hdma_channels;
    uint8 hdma_this_frame;
    uint16 unknownppu_read;
    uint16 unknownppu_write;
    uint16 unknowncpu_read;
    uint16 unknowncpu_write;
    uint16 unknowndsp_read;
    uint16 unknowndsp_write;
};

EXTERN_C struct Missing missing;
#endif
