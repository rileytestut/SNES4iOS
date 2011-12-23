
.text
	
@  notaz's SPC700 Emulator v0.11 - Assembler Output

@ (c) Copyright 2006 notaz, All rights reserved.

@ this is a rewrite of spc700.cpp in ARM asm, inspired by other asm CPU cores like
@ Cyclone and DrZ80. It is meant to be used in Snes9x emulator ports for ARM platforms.

@ the code is released under Snes9x license. See spcgen.c or any other source file
@ from Snes9x source tree.


  .globl _S9xAPUGetByte
  .globl _S9xAPUSetByte
  .globl _S9xAPUGetByteZ
  .globl _S9xAPUSetByteZ

  .globl _spc700_execute @ int cycles
  .globl _Spc700JumpTab

  opcode  .req r3
  cycles  .req r4
  context .req r5
  opcodes .req r6
  spc_pc  .req r7
  spc_ya  .req r8
  spc_p   .req r9
  spc_x   .req r10
  spc_s   .req r11
  spc_ram .req lr

#define iapu_directpage    0x00
#define iapu_ram           0x44
#define iapu_extraram      0x48
#define iapu_allregs_load  0x30
#define iapu_allregs_save  0x34

#define flag_c             0x01
#define flag_z             0x02
#define flag_i             0x04
#define flag_h             0x08
#define flag_b             0x10
#define flag_d             0x20
#define flag_o             0x40
#define flag_n             0x80

#define call_c_function(function)                                            ;\
  ldr r12, 1f   							                                               ;\
	str r9, [r12]																				                       ;\
  ldr r12, 2f   							                                               ;\
	ldr r9, [r12]    							                                             ;\
  bl _##function	                                                           ;\
  ldr r12, 1f   							                                               ;\
	ldr r9, [r12]    							                                             ;\
  b 3f                                                                       ;\
1:                                                                           ;\
  .long _regR9                                                               ;\
2:                                                                           ;\
  .long _regR9s                                                              ;\
3:                                                                           ;\
                                                                             ;\
                                                                             
1:
    .long _IAPU
2:
    .long _CPU
3:
    .long _regR9s
        
@ --------------------------- Framework --------------------------
_spc700_execute: @ int cycles
  stmfd sp!,{r4-r11,lr}
  ldr   r1, 3b
  str   r9, [r1]
  ldr   context, 1b               @ Pointer to SIAPU struct
  mov   cycles,r0                   @ Cycles
  add   r0,context,#iapu_allregs_load
  ldmia r0,{opcodes,spc_pc,spc_ya,spc_p,spc_x,spc_ram}
  mov   spc_s,spc_x,lsr #8
  and   spc_x,spc_x,#0xff

  ldrb  opcode,[spc_pc],#1          @ Fetch first opcode
  ldr   pc,[opcodes,opcode,lsl #2]  @ Jump to opcode handler


@ We come back here after execution
spc700End:
  orr   spc_x,spc_x,spc_s,lsl #8
  add   r0,context,#iapu_allregs_save
  stmia r0,{spc_pc,spc_ya,spc_p,spc_x}
  mov   r0,cycles
  ldmfd sp!,{r4-r11,pc}

@  .ltorg



Apu00:
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu01:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x1e]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu02:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x01
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu03:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x01
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu04:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu05:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu06:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu07:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu08:
  ldrb  r0,[spc_pc],#1
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu09:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  orr   spc_x,spc_x,r0,lsl #24 @ save from harm
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,spc_x,lsr #24
  and   spc_x,spc_x,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0A:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  orrne spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0B:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  tst   r0,#0x80
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsl #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0C:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  tst   r0,#0x80
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsl #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0D:
  mov   r0,spc_p,lsr #24
  and   r1,r0,#0x80
  tst   r0,r0
  orreq r1,r1,#flag_z
  and   spc_p,spc_p,#0x7d @ clear N & Z
  orr   spc_p,spc_p,r1
  add   r1,spc_ram,spc_s
  strb  spc_p,[r1,#0x100]
  sub   spc_s,spc_s,#1
  orr   spc_p,spc_p,r0,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0E:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  orr   spc_x,spc_x,r0,lsl #16 @ save from memhandler
  call_c_function(S9xAPUGetByte)
  and   r2,r0,spc_ya
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r2,lsl #24
  orr   r0,r0,spc_ya
  mov   r1,spc_x,lsr #16
  and   spc_x,spc_x,#0xff
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu0F:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  mov   r0,spc_p,lsr #24
  and   r1,r0,#0x80
  tst   r0,r0
  orrne r1,r1,#flag_z
  and   spc_p,spc_p,#0x7d @ clear N & Z
  orr   spc_p,spc_p,r1
  add   r1,spc_ram,spc_s
  strb  spc_p,[r1,#0x100]
  sub   spc_s,spc_s,#1
  orr   spc_p,spc_p,#flag_b
  bic   spc_p,spc_p,#flag_i
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x20]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu10:
  tst   spc_p,#0x80000000
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu11:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x1c]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu12:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x01
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu13:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x01
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu14:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu15:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu16:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu17:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu18:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#1
  orr   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu19:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  orr   spc_x,spc_x,r0,lsl #24
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,spc_x,lsr #24
  and   spc_x,spc_x,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1A:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  orr   r1,r1,r0,lsl #8
  sub   r0,r1,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #16
  tst   r0,#0xff
  orrne spc_p,spc_p,#0x01000000
  stmfd sp!,{r0}
  ldrb  r1,[spc_pc]
  call_c_function(S9xAPUSetByteZ)
  ldmfd sp!,{r0}
  mov   r0,r0,lsr #8
  ldrb  r1,[spc_pc],#1
  add   r1,r1,#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1B:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByteZ)
  tst   r0,#0x80
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsl #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1C:
  tst   spc_ya,#0x80
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  and   r0,spc_ya,#0x7f
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0,lsl #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1D:
  sub   spc_x,spc_x,#1
  and   spc_x,spc_x,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1E:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs  r12,spc_x,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu1F:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  sub   sp,sp,#8
  str   r0,[sp,#4]
  call_c_function(S9xAPUGetByte)
  str   r0,[sp]
  ldr   r0,[sp,#4]
  add   r0,r0,#1
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  ldr   r1,[sp],#8
  orr   r0,r1,r0,lsl #8
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu20:
  bic   spc_p,spc_p,#flag_d
  str   spc_ram,[context,#iapu_directpage]
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu21:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x1a]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu22:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x02
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu23:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x02
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu24:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu25:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu26:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu27:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu28:
  ldrb  r0,[spc_pc],#1
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu29:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  and   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2A:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  orreq spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2B:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  mov   r0,r0,lsl #1
  tst   spc_p,#flag_c
  orrne r0,r0,#1
  tst   r0,#0x100
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2C:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  mov   r0,r0,lsl #1
  tst   spc_p,#flag_c
  orrne r0,r0,#1
  tst   r0,#0x100
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2D:
  add   r1,spc_ram,spc_s
  strb  spc_ya,[r1,#0x100]
  sub   spc_s,spc_s,#1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2E:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff
  cmp   r0,r1
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu2F:
  ldrsb r0,[spc_pc],#1
  add   spc_pc,spc_pc,r0
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu30:
  tst   spc_p,#0x80000000
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu31:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x18]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu32:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x02
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu33:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x02
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu34:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu35:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu36:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu37:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  orr   r0,r0,#0xff00
  and   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu38:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#2
  and   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc,#-1]
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu39:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  and   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3A:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  orr   r1,r1,r0,lsl #8
  add   r0,r1,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #16
  tst   r0,#0xff
  orrne spc_p,spc_p,#0x01000000
  stmfd sp!,{r0}
  ldrb  r1,[spc_pc]
  call_c_function(S9xAPUSetByteZ)
  ldmfd sp!,{r0}
  mov   r0,r0,lsr #8
  ldrb  r1,[spc_pc],#1
  add   r1,r1,#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3B:
  ldrb  r0,[spc_pc]
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  mov   r0,r0,lsl #1
  tst   spc_p,#flag_c
  orrne r0,r0,#1
  tst   r0,#0x100
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3C:
  and   r0,spc_ya,#0xff
  mov   r0,r0,lsl #1
  tst   spc_p,#flag_c
  orrne r0,r0,#1
  tst   r0,#0x100
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  and   r0,r0,#0xff
  mov   spc_ya,spc_ya,lsr #8
  orr   spc_ya,r0,spc_ya,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3D:
  add   spc_x,spc_x,#1
  and   spc_x,spc_x,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3E:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs  r12,spc_x,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu3F:
  ldrb  r2,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r2,r2,r12,lsl #8
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  add   spc_pc,spc_ram,r2
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu40:
  orr   spc_p,spc_p,#flag_d
  add   r0,spc_ram,#0x100
  str   r0,[context,#iapu_directpage]
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu41:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x16]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu42:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x04
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu43:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x04
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu44:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu45:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu46:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu47:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu48:
  ldrb  r0,[spc_pc],#1
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu49:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  eor   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4A:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  biceq spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4B:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  tst   r0,#0x01
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsr #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4C:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  tst   r0,#0x01
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsr #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4D:
  add   r1,spc_ram,spc_s
  strb  spc_x,[r1,#0x100]
  sub   spc_s,spc_s,#1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4E:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  orr   spc_x,spc_x,r0,lsl #16 @ save from memhandler
  call_c_function(S9xAPUGetByte)
  and   r2,r0,spc_ya
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r2,lsl #24
  bic   r0,r0,spc_ya
  mov   r1,spc_x,lsr #16
  and   spc_x,spc_x,#0xff
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu4F:
  ldrb  r2,[spc_pc],#1
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  add   spc_pc,spc_ram,r2
  add   spc_pc,spc_pc,#0xff00
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu50:
  tst   spc_p,#0x00000040
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu51:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x14]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu52:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x04
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu53:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x04
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu54:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu55:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu56:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu57:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  eor   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu58:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#2
  eor   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc,#-1]
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu59:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  eor   r0,r0,r1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5A:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc],#1
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  ldmfd sp!,{r1}
  orr   r1,r1,r0,lsl #8
  subs  r0,spc_ya,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #16
  tst   r0,#0xff
  orrne spc_p,spc_p,#0x01000000
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5B:
  ldrb  r0,[spc_pc]
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  tst   r0,#0x01
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsr #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5C:
  and   r0,spc_ya,#0xff
  tst   r0,#0x01
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  mov   r0,r0,lsr #1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   spc_ya,spc_ya,lsr #8
  orr   spc_ya,r0,spc_ya,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5D:
  and   spc_x,spc_ya,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5E:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  mov   r1,spc_ya,lsr #8
  subs  r12,r1,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu5F:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu60:
  bic   spc_p,spc_p,#flag_c
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu61:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x12]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu62:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x08
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu63:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x08
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu64:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu65:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu66:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu67:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu68:
  ldrb  r0,[spc_pc],#1
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu69:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  orr   spc_x,spc_x,r0,lsl #24
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  mov   r1,spc_x,lsr #24
  subs  r12,r0,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  and   spc_x,spc_x,#0xff
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6A:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  bicne spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6B:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  tst   spc_p,#flag_c
  orrne r0,r0,#0x100
  movs  r0,r0,lsr #1
  orrcs spc_p,spc_p,#flag_c
  biccc spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6C:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  tst   spc_p,#flag_c
  orrne r0,r0,#0x100
  movs  r0,r0,lsr #1
  orrcs spc_p,spc_p,#flag_c
  biccc spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6D:
  mov   r0,spc_ya,lsr #8
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6E:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#1
  sub   r0,r0,#1
  tst   r0,r0
  addeq spc_pc,spc_pc,#1
  ldrnesb r2,[spc_pc],#1
  addne spc_pc,spc_pc,r2
  subne cycles,cycles,#42
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu6F:
  add   spc_s,spc_s,#2
  add   r1,spc_ram,spc_s
  ldrb  r0,[r1,#0xff]
  ldrb  r1,[r1,#0x100]
  orr   r0,r0,r1,lsl #8
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu70:
  tst   spc_p,#0x00000040
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu71:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x10]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu72:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x08
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu73:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x08
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu74:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu75:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu76:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu77:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r12,spc_ya,#0xff
  subs  r12,r12,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu78:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  ldrb  r1,[spc_pc],#2
  subs  r12,r0,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu79:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  orr   spc_x,spc_x,r0,lsl #24
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  mov   r1,spc_x,lsr #24
  subs  r12,r1,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  and   spc_x,spc_x,#0xff
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7A:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc],#1
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  ldmfd sp!,{r1}
  orr   r1,r1,r0,lsl #8
  add   r0,spc_ya,r1
  movs  r2,r0,lsr #16
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  bic   r2,r0,#0x00ff0000
  eor   r3,r1,r2
  eor   r12,spc_ya,r1
  mvn   r12,r12
  and   r12,r12,r3
  tst   r12,#0x8000
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #16
  tst   spc_ya,#0xff
  orrne spc_p,spc_p,#0x01000000
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7B:
  ldrb  r0,[spc_pc]
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  tst   spc_p,#flag_c
  orrne r0,r0,#0x100
  movs  r0,r0,lsr #1
  orrcs spc_p,spc_p,#flag_c
  biccc spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7C:
  and   r0,spc_ya,#0xff
  tst   spc_p,#flag_c
  orrne r0,r0,#0x100
  movs  r0,r0,lsr #1
  orrcs spc_p,spc_p,#flag_c
  biccc spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   spc_ya,spc_ya,lsr #8
  orr   spc_ya,r0,spc_ya,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7D:
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,spc_x
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7E:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  mov   r1,spc_ya,lsr #8
  subs  r12,r1,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu7F:
  add   spc_s,spc_s,#1
  add   spc_p,spc_ram,spc_s
  ldrb  spc_p,[spc_p,#0x100]
  and   r0,spc_p,#(flag_z|flag_n)
  eor   r0,r0,#flag_z
  orr   spc_p,spc_p,r0,lsl #24
  tst   spc_p,#flag_d
  addne r0,spc_ram,#0x100
  moveq r0,spc_ram
  str   r0,[context,#iapu_directpage]
  add   spc_s,spc_s,#2
  add   r1,spc_ram,spc_s
  ldrb  r0,[r1,#0xff]
  ldrb  r1,[r1,#0x100]
  orr   r0,r0,r1,lsl #8
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu80:
  orr   spc_p,spc_p,#flag_c
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu81:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0xe]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu82:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x10
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu83:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x10
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu84:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu85:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu86:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu87:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu88:
  ldrb  r0,[spc_pc],#1
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu89:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  eor   r3,r0,r1
  add   r0,r0,r1
  tst   spc_p,#flag_c
  addne r0,r0,#1
  movs  r12,r0,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,r0,r1
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r0
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8A:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  eorne spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8B:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  sub   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8C:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  sub   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8D:
  ldrb  r0,[spc_pc],#1
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8E:
  add   spc_s,spc_s,#1
  add   spc_p,spc_ram,spc_s
  ldrb  spc_p,[spc_p,#0x100]
  and   r0,spc_p,#(flag_z|flag_n)
  eor   r0,r0,#flag_z
  orr   spc_p,spc_p,r0,lsl #24
  tst   spc_p,#flag_d
  addne r0,spc_ram,#0x100
  moveq r0,spc_ram
  str   r0,[context,#iapu_directpage]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu8F:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu90:
  tst   spc_p,#0x00000001
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu91:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0xc]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu92:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x10
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu93:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x10
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu94:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu95:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu96:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu97:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  eor   r3,spc_ya,r0
  add   spc_ya,spc_ya,r0
  tst   spc_p,#flag_c
  addne spc_ya,spc_ya,#1
  movs  r12,spc_ya,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r0
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,spc_ya
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu98:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#2
  eor   r3,r0,r1
  add   r0,r0,r1
  tst   spc_p,#flag_c
  addne r0,r0,#1
  movs  r12,r0,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,r0,r1
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r0
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc,#-1]
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu99:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  eor   r3,r0,r1
  add   r0,r0,r1
  tst   spc_p,#flag_c
  addne r0,r0,#1
  movs  r12,r0,lsr #8
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  eor   r12,r0,r1
  bic   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r0
  tst   r12,#0x10
  orrne spc_p,spc_p,#flag_h
  biceq spc_p,spc_p,#flag_h
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9A:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc],#1
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  ldmfd sp!,{r1}
  orr   r1,r1,r0,lsl #8
  subs  r0,spc_ya,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  mov   r2,r0,lsl #16
  mov   r2,r2,lsr #16
  eor   r3,spc_ya,r2
  eor   r12,spc_ya,r1
  and   r12,r12,r3
  tst   r12,#0x8000
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r1
  tst   r12,#0x10
  bicne spc_p,spc_p,#flag_h
  orreq spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #16
  tst   spc_ya,#0xff
  orrne spc_p,spc_p,#0x01000000
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9B:
  ldrb  r0,[spc_pc]
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  sub   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9C:
  and   r0,spc_ya,#0xff
  sub   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  and   r0,r0,#0xff
  mov   spc_ya,spc_ya,lsr #8
  orr   spc_ya,r0,spc_ya,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9D:
  mov   spc_x,spc_s
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9E:
  tst   spc_x,spc_x @ div by 0?
  orreq spc_ya,spc_ya,#0xff00
  orreq spc_ya,spc_ya,#0x00ff
  orreq spc_p,spc_p,#flag_o
  beq   Apu9E_end
  bic   spc_p,spc_p,#flag_o
@ Divide spc_ya by spc_x
  mov r3,#0
  mov r1,spc_x

@ Shift up divisor till it's just less than numerator
divshift:
  cmp r1,spc_ya,lsr #1
  movls r1,r1,lsl #1
  bcc divshift

divloop:
  cmp spc_ya,r1
  adc r3,r3,r3 ;@ Double r3 and add 1 if carry set
  subcs spc_ya,spc_ya,r1
  teq r1,spc_x
  movne r1,r1,lsr #1
  bne divloop

  and   spc_ya,spc_ya,#0xff
  and   r3,r3,#0xff
  orr   spc_ya,r3,spc_ya,lsl #8
Apu9E_end:
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#252
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


Apu9F:
  and   r0,spc_ya,#0xff
  mov   r1,r0,lsl #28
  orr   r0,r1,r0,lsl #20
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0,lsr #24
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA0:
  orr   spc_p,spc_p,#flag_i
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0xa]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x20
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x20
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA4:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA5:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA6:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA7:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA8:
  ldrb  r0,[spc_pc],#1
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuA9:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  movs  r12,spc_p,lsr #1
  sbcs  r2,r0,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,r0,r2
  eor   r3,r0,r1
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   r0,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAA:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r0,r0,lsr r1
  tst   r0,#1
  orrne spc_p,spc_p,#flag_c
  biceq spc_p,spc_p,#flag_c
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAB:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  add   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAC:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  add   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAD:
  ldrb  r0,[spc_pc],#1
  mov   r1,spc_ya,lsr #8
  subs  r12,r1,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAE:
  add   spc_s,spc_s,#1
  add   r0,spc_ram,spc_s
  ldrb  r0,[r0,#0x100]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuAF:
  mov   r0,spc_ya
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  add   spc_x,spc_x,#1
  and   spc_x,spc_x,#0xff
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB0:
  tst   spc_p,#0x00000001
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x8]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x20
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x20
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB4:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB5:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB6:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB7:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff00
  and   spc_ya,spc_ya,#0xff
  movs  r12,spc_p,lsr #1
  sbcs  r2,spc_ya,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,spc_ya,r2
  eor   r3,spc_ya,r0
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   spc_ya,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r1
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB8:
  ldrb  r0,[spc_pc,#1]
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#2
  movs  r12,spc_p,lsr #1
  sbcs  r2,r0,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,r0,r2
  eor   r3,r0,r1
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   r0,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc,#-1]
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuB9:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  stmfd sp!,{r0}
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldmfd sp!,{r1}
  movs  r12,spc_p,lsr #1
  sbcs  r2,r0,r1
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  eor   r12,r0,r2
  eor   r3,r0,r1
  and   r12,r12,r3
  tst   r12,#0x80
  orrne spc_p,spc_p,#flag_o
  biceq spc_p,spc_p,#flag_o
  eor   r12,r3,r2
  tst   r12,#0x10
  orreq spc_p,spc_p,#flag_h
  bicne spc_p,spc_p,#flag_h
  mov   r0,r2
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBA:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  mov   spc_ya,r0
  ldrb  r0,[spc_pc],#1
  add   r0,r0,#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  orr   spc_ya,spc_ya,r0,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #16
  tst   spc_ya,#0xff
  orrne spc_p,spc_p,#0x01000000
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBB:
  ldrb  r0,[spc_pc]
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  add   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBC:
  and   r0,spc_ya,#0xff
  add   r0,r0,#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  and   r0,r0,#0xff
  mov   spc_ya,spc_ya,lsr #8
  orr   spc_ya,r0,spc_ya,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBD:
  mov   spc_s,spc_x
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBE:
  and   r0,spc_ya,#0xff
  and   r1,spc_ya,#0x0f
  cmp   r1,#9
  subhi r0,r0,#6
  tstls spc_p,#flag_h
  subeq r0,r0,#6
  cmp   r0,#0x9f
  bhi   ApuBE_tens
  tst   spc_p,#flag_c
  beq   ApuBE_tens
  orr   spc_p,spc_p,#flag_c
  b     ApuBE_end
ApuBE_tens:
  sub   r0,r0,#0x60
  bic   spc_p,spc_p,#flag_c
ApuBE_end:
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuBF:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  add   spc_x,spc_x,#1
  and   spc_x,spc_x,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC0:
  bic   spc_p,spc_p,#flag_i
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x6]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x40
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x40
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC4:
  ldrb  r1,[spc_pc],#1
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC5:
  ldrb  r1,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r1,r1,r12,lsl #8
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC6:
  mov   r0,spc_ya
  mov   r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC7:
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  and   r1,r1,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r1,[r12,r1]!
  ldrb  r12,[r12,#1]
  orr   r1,r1,r12,lsl #8
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#147
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC8:
  ldrb  r0,[spc_pc],#1
  subs  r12,spc_x,r0
  orrge spc_p,spc_p,#flag_c
  biclt spc_p,spc_p,#flag_c
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r12,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuC9:
  ldrb  r1,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r1,r1,r12,lsl #8
  mov   r0,spc_x
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCA:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r2,#1
  mov   r2,r2,lsl r1
  tst   spc_p,#flag_c
  orrne r0,r0,r2
  biceq r0,r0,r2
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCB:
  ldrb  r1,[spc_pc],#1
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCC:
  ldrb  r1,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r1,r1,r12,lsl #8
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCD:
  ldrb  spc_x,[spc_pc],#1
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCE:
  add   spc_s,spc_s,#1
  add   spc_x,spc_ram,spc_s
  ldrb  spc_x,[spc_x,#0x100]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuCF:
  mov   r0,spc_ya,lsr #8
  and   spc_ya,spc_ya,#0xff
  mul   spc_ya,r0,spc_ya
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #16
  tst   spc_ya,#0xff
  orrne spc_p,spc_p,#0x01000000
  subs   cycles,cycles,#189
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD0:
  tst   spc_p,#0xFF000000
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x4]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x40
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x40
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD4:
  mov   r0,spc_ya
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD5:
  ldrb  r1,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r1,r1,r12,lsl #8
  add   r1,r1,spc_x
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD6:
  ldrb  r1,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r1,r1,r12,lsl #8
  add   r1,r1,spc_ya,lsr #8
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD7:
  ldrb  r1,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r1,[r12,r1]!
  ldrb  r12,[r12,#1]
  orr   r1,r1,r12,lsl #8
  add   r1,r1,spc_ya,lsr #8
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#147
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD8:
  ldrb  r1,[spc_pc],#1
  mov   r0,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuD9:
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_ya,lsr #8
  mov   r0,spc_x
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDA:
  ldrb  r1,[spc_pc]
  mov   r0,spc_ya
  call_c_function(S9xAPUSetByteZ)
  ldrb  r1,[spc_pc],#1
  add   r1,r1,#1
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDB:
  ldrb  r1,[spc_pc],#1
  add   r1,r1,spc_x
  mov   r0,spc_ya,lsr #8
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDC:
  mov   r0,spc_ya,lsr #8
  sub   r0,r0,#1
  and   r0,r0,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDD:
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,spc_ya,lsr #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDE:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   r1,spc_ya,#0xff
  cmp   r0,r1
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuDF:
  and   r0,spc_ya,#0xff
  and   r1,spc_ya,#0x0f
  cmp   r1,#9
  addhi r0,r0,#6
  bls   ApuDF_testHc
  cmphi r0,#0xf0
  orrhi spc_p,spc_p,#flag_c
  b     ApuDF_test2
ApuDF_testHc:
  tst   spc_p,#flag_h
  addne r0,r0,#6
  beq   ApuDF_test2
  cmp   r0,#0xf0
  orrhi spc_p,spc_p,#flag_c
ApuDF_test2:
  tst   spc_p,#flag_c
  addne r0,r0,#0x60
  bne   ApuDF_end
  cmp   r0,#0x9f
  addhi r0,r0,#0x60
  orrhi spc_p,spc_p,#flag_c
  bicls spc_p,spc_p,#flag_c
ApuDF_end:
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE0:
  bic   spc_p,spc_p,#(flag_o|flag_h)
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x2]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  orr   r0,r0,#0x80
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x80
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE4:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE5:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE6:
  mov   r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE7:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  and   r0,r0,#0xff
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE8:
  ldrb  r0,[spc_pc],#1
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuE9:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  mov   spc_x,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuEA:
  ldrb  r0,[spc_pc],#1
  ldrb  r1,[spc_pc],#1
  add   r0,r0,r1,lsl #8
  mov   r1,r1,lsr #5
  mov   r0,r0,lsl #19
  mov   r0,r0,lsr #19
  orr   spc_x,spc_x,r1,lsl #29 @ store membit where it can survive memhandler call
  stmfd sp!,{r0}
  call_c_function(S9xAPUGetByte)
  mov   r1,spc_x,lsr #29
  and   spc_x,spc_x,#0xff
  mov   r2,#1
  mov   r2,r2,lsl r1
  eor   r0,r0,r2
  ldmfd sp!,{r1}
  call_c_function(S9xAPUSetByte)
  ldr   spc_ram,[context,#iapu_ram] @ restore what memhandler(s) messed up
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuEB:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuEC:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuED:
  eor   spc_p,spc_p,#flag_c
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuEE:
  add   spc_s,spc_s,#1
  add   r0,spc_ram,spc_s
  ldrb  r0,[r0,#0x100]
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End

20:
  .long _CPU
ApuEF:
  ldr   r0, 20b
  mov   r1,#0
  strb  r1,[r0,#122]
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF0:
  tst   spc_p,#0xFF000000
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF1:
  sub   r0,spc_pc,spc_ram
  add   r1,spc_ram,spc_s
  strb  r0,[r1,#0xff]
  mov   r0,r0,lsr #8
  strb  r0,[r1,#0x100]
  sub   spc_s,spc_s,#2
  ldr   r0,[context,#iapu_extraram]
  ldrh  r0,[r0,#0x0]
  add   spc_pc,spc_ram,r0
  subs   cycles,cycles,#168
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF2:
  ldrb  r0,[spc_pc]
  call_c_function(S9xAPUGetByteZ)
  bic   r0,r0,#0x80
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF3:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  tst   r0,#0x80
  addne spc_pc,spc_pc,#1
  ldreqsb r0,[spc_pc],#1
  addeq spc_pc,spc_pc,r0
  subeq cycles,cycles,#42
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF4:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF5:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF6:
  ldrb  r0,[spc_pc],#1
  ldrb  r12,[spc_pc],#1
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF7:
  ldrb  r0,[spc_pc],#1
  ldr   r12,[context,#iapu_directpage]
  ldrb  r0,[r12,r0]!
  ldrb  r12,[r12,#1]
  orr   r0,r0,r12,lsl #8
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByte)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff00
  orr   spc_ya,spc_ya,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#126
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF8:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  mov   spc_x,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuF9:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_ya,lsr #8
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  mov   spc_x,r0
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_x,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuFA:
  ldrb  r0,[spc_pc],#1
  call_c_function(S9xAPUGetByteZ)
  ldrb  r1,[spc_pc],#1
  call_c_function(S9xAPUSetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  subs   cycles,cycles,#105
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuFB:
  ldrb  r0,[spc_pc],#1
  add   r0,r0,spc_x
  call_c_function(S9xAPUGetByteZ)
  ldr   spc_ram,[context,#iapu_ram]
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuFC:
  mov   r0,spc_ya,lsr #8
  add   r0,r0,#1
  and   r0,r0,#0xff
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,r0,lsl #24
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,r0,lsl #8
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuFD:
  and   spc_ya,spc_ya,#0xff
  orr   spc_ya,spc_ya,spc_ya,lsl #8
  and   spc_p,spc_p,#0xff
  orr   spc_p,spc_p,spc_ya,lsl #24
  subs   cycles,cycles,#42
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


ApuFE:
  sub   spc_ya,spc_ya,#0x100
  mov   spc_ya,spc_ya,lsl #16
  mov   spc_ya,spc_ya,lsr #16
  movs  r0,spc_ya,lsr #8
  addeq spc_pc,spc_pc,#1
  ldrnesb r0,[spc_pc],#1
  addne spc_pc,spc_pc,r0
  subne cycles,cycles,#42
  subs   cycles,cycles,#84
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End

21:
  .long _CPU
ApuFF:
  ldr   r0, 21b
  mov   r1,#0
  strb  r1,[r0,#122]
  subs   cycles,cycles,#63
  ldrgeb opcode,[spc_pc],#1
  ldrge  pc,[opcodes,opcode,lsl #2]
  b      spc700End


@ -------------------------- Jump Table --------------------------
_Spc700JumpTab:
  .long Apu00, Apu01, Apu02, Apu03, Apu04, Apu05, Apu06, Apu07 @ 00
  .long Apu08, Apu09, Apu0A, Apu0B, Apu0C, Apu0D, Apu0E, Apu0F @ 08
  .long Apu10, Apu11, Apu12, Apu13, Apu14, Apu15, Apu16, Apu17 @ 10
  .long Apu18, Apu19, Apu1A, Apu1B, Apu1C, Apu1D, Apu1E, Apu1F @ 18
  .long Apu20, Apu21, Apu22, Apu23, Apu24, Apu25, Apu26, Apu27 @ 20
  .long Apu28, Apu29, Apu2A, Apu2B, Apu2C, Apu2D, Apu2E, Apu2F @ 28
  .long Apu30, Apu31, Apu32, Apu33, Apu34, Apu35, Apu36, Apu37 @ 30
  .long Apu38, Apu39, Apu3A, Apu3B, Apu3C, Apu3D, Apu3E, Apu3F @ 38
  .long Apu40, Apu41, Apu42, Apu43, Apu44, Apu45, Apu46, Apu47 @ 40
  .long Apu48, Apu49, Apu4A, Apu4B, Apu4C, Apu4D, Apu4E, Apu4F @ 48
  .long Apu50, Apu51, Apu52, Apu53, Apu54, Apu55, Apu56, Apu57 @ 50
  .long Apu58, Apu59, Apu5A, Apu5B, Apu5C, Apu5D, Apu5E, Apu5F @ 58
  .long Apu60, Apu61, Apu62, Apu63, Apu64, Apu65, Apu66, Apu67 @ 60
  .long Apu68, Apu69, Apu6A, Apu6B, Apu6C, Apu6D, Apu6E, Apu6F @ 68
  .long Apu70, Apu71, Apu72, Apu73, Apu74, Apu75, Apu76, Apu77 @ 70
  .long Apu78, Apu79, Apu7A, Apu7B, Apu7C, Apu7D, Apu7E, Apu7F @ 78
  .long Apu80, Apu81, Apu82, Apu83, Apu84, Apu85, Apu86, Apu87 @ 80
  .long Apu88, Apu89, Apu8A, Apu8B, Apu8C, Apu8D, Apu8E, Apu8F @ 88
  .long Apu90, Apu91, Apu92, Apu93, Apu94, Apu95, Apu96, Apu97 @ 90
  .long Apu98, Apu99, Apu9A, Apu9B, Apu9C, Apu9D, Apu9E, Apu9F @ 98
  .long ApuA0, ApuA1, ApuA2, ApuA3, ApuA4, ApuA5, ApuA6, ApuA7 @ a0
  .long ApuA8, ApuA9, ApuAA, ApuAB, ApuAC, ApuAD, ApuAE, ApuAF @ a8
  .long ApuB0, ApuB1, ApuB2, ApuB3, ApuB4, ApuB5, ApuB6, ApuB7 @ b0
  .long ApuB8, ApuB9, ApuBA, ApuBB, ApuBC, ApuBD, ApuBE, ApuBF @ b8
  .long ApuC0, ApuC1, ApuC2, ApuC3, ApuC4, ApuC5, ApuC6, ApuC7 @ c0
  .long ApuC8, ApuC9, ApuCA, ApuCB, ApuCC, ApuCD, ApuCE, ApuCF @ c8
  .long ApuD0, ApuD1, ApuD2, ApuD3, ApuD4, ApuD5, ApuD6, ApuD7 @ d0
  .long ApuD8, ApuD9, ApuDA, ApuDB, ApuDC, ApuDD, ApuDE, ApuDF @ d8
  .long ApuE0, ApuE1, ApuE2, ApuE3, ApuE4, ApuE5, ApuE6, ApuE7 @ e0
  .long ApuE8, ApuE9, ApuEA, ApuEB, ApuEC, ApuED, ApuEE, ApuEF @ e8
  .long ApuF0, ApuF1, ApuF2, ApuF3, ApuF4, ApuF5, ApuF6, ApuF7 @ f0
  .long ApuF8, ApuF9, ApuFA, ApuFB, ApuFC, ApuFD, ApuFE, ApuFF @ f8
