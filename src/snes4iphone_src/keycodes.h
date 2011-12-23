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
#define SCANCODE_K 167
#define SCANCODE_ESC 197
#define SCANCODE_CURSORBLOCKRIGHT 175
#define SCANCODE_CURSORBLOCKLEFT 173
#define SCANCODE_CURSORBLOCKUP 190
#define SCANCODE_CURSORBLOCKDOWN 158
#define SCANCODE_H 165
#define SCANCODE_N 182
#define SCANCODE_J 166
#define SCANCODE_U 150
#define SCANCODE_ENTER 196
#define SCANCODE_SPACE 192
#define SCANCODE_A 160
#define SCANCODE_V 180
#define SCANCODE_Q 144
#define SCANCODE_Z 149
#define SCANCODE_B 181
#define SCANCODE_W 145
#define SCANCODE_S 161
#define SCANCODE_M 183
#define SCANCODE_E 146
#define SCANCODE_X 178
#define SCANCODE_COMMA 184
#define SCANCODE_R 147
#define SCANCODE_D 162
#define SCANCODE_PERIOD 185
#define SCANCODE_T 148
#define SCANCODE_C 179
#define SCANCODE_SLASH 186
#define SCANCODE_Y 177
#define SCANCODE_CURSORRIGHT 206
#define SCANCODE_CURSORLEFT 207
#define SCANCODE_CURSORDOWN 205
#define SCANCODE_CURSORUP 204
#define SCANCODE_KEYPADENTER 195
#define SCANCODE_KEYPADPLUS 222
#define SCANCODE_INSERT 143
#define SCANCODE_REMOVE 188
#define SCANCODE_HOME 189
#define SCANCODE_END 157
#define SCANCODE_PAGEUP 191
#define SCANCODE_PAGEDOWN 159
#define SCANCODE_0 138
#define SCANCODE_1 129
#define SCANCODE_2 130
#define SCANCODE_3 131
#define SCANCODE_4 132
#define SCANCODE_5 133
#define SCANCODE_6 134
#define SCANCODE_7 135
#define SCANCODE_8 136
#define SCANCODE_9 137
#define SCANCODE_BACKSPACE 193
#define SCANCODE_F1 208
#define SCANCODE_F2 209
#define SCANCODE_F3 210
#define SCANCODE_F4 211
#define SCANCODE_F5 212
#define SCANCODE_F6 213
#define SCANCODE_F7 214
#define SCANCODE_F8 215
#define SCANCODE_F9 216
#define SCANCODE_F10 217
#define SCANCODE_F11 198
#define SCANCODE_F12 223
#define SCANCODE_P 153
#define SCANCODE_LESSER 176
#define SCANCODE_PLUS 155
