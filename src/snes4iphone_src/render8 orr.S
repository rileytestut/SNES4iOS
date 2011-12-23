	.global asmDrawTile8_noflip
	.global asmDrawTile8_hflip
	.global asmDrawTile8_hvflip
	.global asmDrawTile8_vflip
	.global asmDrawTile8T_noflip
	.global asmDrawTile8T_hflip
	.global asmDrawTile8T_hvflip
	.global asmDrawTile8T_vflip
	
	;@ ###############################
	.macro DRAWTILE_NOFLIP
	;@ ###############################
	ldr r3,[r0],#4
	
	ands r12,r3,#0x0000000F
	orrne r12,r1,r12
	strneb r12,[r2,#0]
	
	ands r12,r3,#0x000000F0
	orrne r12,r1,r12,lsr#4
	strneb r12,[r2,#1]
	
	ands r12,r3,#0x00000F00
	orrne r12,r1,r12,lsr#8
	strneb r12,[r2,#2]
	
	ands r12,r3,#0x0000F000
	orrne r12,r1,r12,lsr#12
	strneb r12,[r2,#3]
	
	ands r12,r3,#0x000F0000
	orrne r12,r1,r12,lsr#16
	strneb r12,[r2,#4]
	
	ands r12,r3,#0x00F00000
	orrne r12,r1,r12,lsr#20
	strneb r12,[r2,#5]
	
	ands r12,r3,#0x0F000000
	orrne r12,r1,r12,lsr#24
	strneb r12,[r2,#6]
	
	ands r12,r3,#0xF0000000
	orrne r12,r1,r12,lsr#28
	strneb r12,[r2,#7]

	add r2,r2,#320
	.endm
	
	;@ ###############################
	.macro DRAWTILE_HFLIP
	;@ ###############################
	ldr r3,[r0],#4
	
	ands r12,r3,#0x0000000F
	orrne r12,r1,r12
	strneb r12,[r2,#7]
	
	ands r12,r3,#0x000000F0
	orrne r12,r1,r12,lsr#4
	strneb r12,[r2,#6]
	
	ands r12,r3,#0x00000F00
	orrne r12,r1,r12,lsr#8
	strneb r12,[r2,#5]
	
	ands r12,r3,#0x0000F000
	orrne r12,r1,r12,lsr#12
	strneb r12,[r2,#4]
	
	ands r12,r3,#0x000F0000
	orrne r12,r1,r12,lsr#16
	strneb r12,[r2,#3]
	
	ands r12,r3,#0x00F00000
	orrne r12,r1,r12,lsr#20
	strneb r12,[r2,#2]
	
	ands r12,r3,#0x0F000000
	orrne r12,r1,r12,lsr#24
	strneb r12,[r2,#1]
	
	ands r12,r3,#0xF0000000
	orrne r12,r1,r12,lsr#28
	strneb r12,[r2,#0]

	add r2,r2,#320
	.endm
	
	;@ ###############################
	.macro DRAWTILE_HVFLIP
	;@ ###############################
	ldr r3,[r0],#4
	
	ands r12,r3,#0x0000000F
	orrne r12,r1,r12
	strneb r12,[r2,#7]
	
	ands r12,r3,#0x000000F0
	orrne r12,r1,r12,lsr#4
	strneb r12,[r2,#6]
	
	ands r12,r3,#0x00000F00
	orrne r12,r1,r12,lsr#8
	strneb r12,[r2,#5]
	
	ands r12,r3,#0x0000F000
	orrne r12,r1,r12,lsr#12
	strneb r12,[r2,#4]
	
	ands r12,r3,#0x000F0000
	orrne r12,r1,r12,lsr#16
	strneb r12,[r2,#3]
	
	ands r12,r3,#0x00F00000
	orrne r12,r1,r12,lsr#20
	strneb r12,[r2,#2]
	
	ands r12,r3,#0x0F000000
	orrne r12,r1,r12,lsr#24
	strneb r12,[r2,#1]
	
	ands r12,r3,#0xF0000000
	orrne r12,r1,r12,lsr#28
	strneb r12,[r2,#0]

	sub r2,r2,#320
	.endm
	
	;@ ###############################
	.macro DRAWTILE_VFLIP
	;@ ###############################
	ldr r3,[r0],#4
	
	ands r12,r3,#0x0000000F
	orrne r12,r1,r12
	strneb r12,[r2,#0]
	
	ands r12,r3,#0x000000F0
	orrne r12,r1,r12,lsr#4
	strneb r12,[r2,#1]
	
	ands r12,r3,#0x00000F00
	orrne r12,r1,r12,lsr#8
	strneb r12,[r2,#2]
	
	ands r12,r3,#0x0000F000
	orrne r12,r1,r12,lsr#12
	strneb r12,[r2,#3]
	
	ands r12,r3,#0x000F0000
	orrne r12,r1,r12,lsr#16
	strneb r12,[r2,#4]
	
	ands r12,r3,#0x00F00000
	orrne r12,r1,r12,lsr#20
	strneb r12,[r2,#5]
	
	ands r12,r3,#0x0F000000
	orrne r12,r1,r12,lsr#24
	strneb r12,[r2,#6]
	
	ands r12,r3,#0xF0000000
	orrne r12,r1,r12,lsr#28
	strneb r12,[r2,#7]

	sub r2,r2,#320
	.endm


asmDrawTile8_noflip:
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	DRAWTILE_NOFLIP
	mov pc,lr
	
asmDrawTile8_hflip:
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	DRAWTILE_HFLIP
	mov pc,lr
	
asmDrawTile8_hvflip:
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
	DRAWTILE_HVFLIP
    mov pc,lr
	
asmDrawTile8_vflip:
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	DRAWTILE_VFLIP
	mov pc,lr
	
	
	


