.text
/****************************************************************	
****************************************************************/
	.align 4

    @ notaz
#define asm_spc700 		1		// 1 = use notaz's asm_spc700 core

/****************************************************************
	defines
****************************************************************/

#define map_last	12

#define rstatus 	r4  // format : 0xff800000
#define reg_d_bank	r4  // format : 0x000000ll
#define reg_a		r5  // format : 0xhhll0000 or 0xll000000
#define reg_d		r6  // format : 0xhhll0000
#define reg_p_bank	r6  // format : 0x000000ll
#define reg_x		r7  // format : 0xhhll0000 or 0xll000000
#define reg_s		r8  // format : 0x0000hhll
#define reg_y		r9  // format : 0xhhll0000 or 0xll000000

#define rpc	    	r10 // 32bits address
#define reg_cycles	r11 // 32bits counter
#define regpcbase	r12 // 32bits address

#define rscratch	r0  // format : 0xhhll0000 if data and calculation or return of s9xreadbyte	or word
#define regopcode	r0  // format : 0x000000ll
#define rscratch2	r1  // format : 0xhhll for calculation and value
#define rscratch3	r2  // 
#define rscratch4	r3  // ??????

// used for sbc opcode
#define rscratch9	r10 // ??????

#define reg_cpu_var r14



// not used
// r13	// pointer 32 bit on a struct.

// r15 = pc (sic!)


#define status_shifter		24
#define mask_emul		(1<<(status_shifter-1))
#define mask_shifter_carry	(status_shifter+1)
#define mask_carry		(1<<(status_shifter))  @ 0
#define mask_zero		(2<<(status_shifter))  @ 1
#define mask_irq		(4<<(status_shifter))  @ 2
#define mask_decimal		(8<<(status_shifter))  @ 3
#define mask_index		(16<<(status_shifter)) @ 4  @ 1
#define mask_mem		(32<<(status_shifter)) @ 5  @ 2
#define mask_overflow		(64<<(status_shifter)) @ 6  @ 4
#define mask_neg		(128<<(status_shifter))@ 7  @ 8

#define mask_emul_orr_mask_mem_orr_mask_index 813694976
#define mask_carry_orr_mask_mem_orr_mask_index 822083584
#define mask_mem_orr_mask_index 805306368

#define one_cycle 6
#define two_cycle 12
#define three_cycle 18
#define slow_one_cycle 8
#define slow_two_cycle 16

#define nmi_flag	    (1 << 7)
#define irq_pending_flag    (1 << 11)
#define irq_pending_flag_orr_nmi_flag 2176
#define scan_keys_flag	    (1 << 4)


#define memmap_block_size (0x1000)
#define memmap_shift 12
#define memmap_mask (0xfff)

/****************************************************************
	macros
****************************************************************/

@ #include "os9x_65c816_mac_gen.h"
/*****************************************************************/
/*     offset in scpustate structure				 */
/*****************************************************************/
#define flags_ofs		    0    
#define branchskip_ofs	4
#define nmiactive_ofs		5
#define irqactive_ofs		6
#define waitingforinterrupt_ofs	7

#define	rpb_ofs		8
#define	rdb_ofs		9
#define	rp_ofs		    10
#define	ra_ofs		    12
#define    rah_ofs	    13
#define	rd_ofs		    14
#define	rx_ofs		    16
#define	rs_ofs		    18
#define	ry_ofs		    20
@#define	rpc_ofs		22
   
#define pc_ofs			24
#define cycles_ofs		28
#define pcbase_ofs		32

#define pcatopcodestart_ofs	36
#define waitaddress_ofs		40
#define waitcounter_ofs		44
#define nextevent_ofs		    48
#define v_counter_ofs		    52
#define memspeed_ofs		    56
#define memspeedx2_ofs		60
#define fastromspeed_ofs	    64
#define autosavetimer_ofs	    68
#define nmitriggerpoint_ofs	72
#define nmicyclecount_ofs	    76
#define irqcyclecount_ofs	    80

#define indma_ofs	       84
#define whichevent		    85
#define srammodified_ofs	    86
#define brktriggered_ofs	    87
#define	asm_optable_ofs		88
#define triedinterleavedmode2_ofs	92

#define map_ofs		    96
#define writemap_ofs      100
#define memoryspeed_ofs   104
#define blockisram_ofs    108
#define sram 		        112
#define bwram             116
#define srammask          120

#define	apuexecuting_ofs   122

#define	iphone_r9_ofs 	    124
#define	iphone_r9s_ofs 	128

@ notaz
#define	apu_cycles 	    132

#define	jump1 	        136
#define	jump2 	        140
#define	jump3 	        144
#define	jump4 	        148


/*****************************************************************/

/* prepare */
.macro		prepare_c_call
  str	r9,[reg_cpu_var,#iphone_r9_ofs]
  ldr	r9,[reg_cpu_var,#iphone_r9s_ofs]
	stmfd	r13!,{r12,r14}	
.endm
.macro		prepare_c_call_r0
  str	r9,[reg_cpu_var,#iphone_r9_ofs]
  ldr	r9,[reg_cpu_var,#iphone_r9s_ofs]
	stmfd	r13!,{r0,r12,r14}	
.endm
.macro		prepare_c_call_r0r1
  str	r9,[reg_cpu_var,#iphone_r9_ofs]
  ldr	r9,[reg_cpu_var,#iphone_r9s_ofs]
	stmfd	r13!,{r0,r1,r12,r14}		
.endm
.macro		prepare_c_call_light
  str	r9,[reg_cpu_var,#iphone_r9_ofs]
  ldr	r9,[reg_cpu_var,#iphone_r9s_ofs]
	stmfd	r13!,{r14}
.endm
.macro		prepare_c_call_lightr12
  str	r9,[reg_cpu_var,#iphone_r9_ofs]
  ldr	r9,[reg_cpu_var,#iphone_r9s_ofs]
	stmfd	r13!,{r12,r14}
.endm
/* restore */
.macro		restore_c_call
	ldmfd	r13!,{r12,r14}
	ldr	r9,[reg_cpu_var,#iphone_r9_ofs]
.endm
.macro		restore_c_call_r0
	ldmfd	r13!,{r0,r12,r14}
	ldr	r9,[reg_cpu_var,#iphone_r9_ofs]
.endm
.macro		restore_c_call_r1
	ldmfd	r13!,{r1,r12,r14}
	ldr	r9,[reg_cpu_var,#iphone_r9_ofs]
.endm
.macro		restore_c_call_light
	ldmfd	r13!,{r14}
	ldr	r9,[reg_cpu_var,#iphone_r9_ofs]
.endm
.macro		restore_c_call_lightr12
	ldmfd	r13!,{r12,r14}
	ldr	r9,[reg_cpu_var,#iphone_r9_ofs]
.endm


@ --------------
.macro		load_regs
    @ notaz
    add     r0,reg_cpu_var,#8
    ldmia   r0,{r1,reg_a,reg_x,reg_y,rpc,reg_cycles,regpcbase}
    @ rstatus (p) & reg_d_bank
    mov     reg_d_bank,r1,lsl #16
    mov     reg_d_bank,reg_d_bank,lsr #24
    mov     r0,r1,lsr #16
	orrs	rstatus, rstatus, r0,lsl #status_shifter @ 24
	@ if carry set, then emulation bit was set
	orrcs	rstatus,rstatus,#mask_emul	
    @ reg_d & reg_p_bank
    mov     reg_d,reg_a,lsr #16
    mov     reg_d,reg_d,lsl #8
    orr     reg_d,reg_d,r1,lsl #24
    mov     reg_d,reg_d,ror #24    @ 0xdddd00pb
    @ reg_x, reg_s
    mov     reg_s,reg_x,lsr #16
	@ shift x,y & a according to the current mode (index, memory bits)
	tst		rstatus,#mask_index
	movne	reg_x,reg_x,lsl #24
	movne	reg_y,reg_y,lsl #24
	moveq	reg_x,reg_x,lsl #16
	moveq	reg_y,reg_y,lsl #16
	tst		rstatus,#mask_mem
	movne	reg_a,reg_a,lsl #24
	moveq	reg_a,reg_a,lsl #16
.endm


.macro		save_regs
    @ notaz
    @ reg_p_bank, reg_d_bank and rstatus
    mov 	r1, rstatus, lsr #16
    orr     r1, r1, reg_p_bank, lsl #24
	movs	r1, r1, lsr #8
	orrcs	r1, r1, #0x100 @ emulation bit
    orr     r1, r1, reg_d_bank, lsl #24
    mov     r1, r1, ror #16
    @ reg_a, reg_d
	tst		rstatus,#mask_mem
	ldrneh	r0, [reg_cpu_var,#ra_ofs]
	bicne	r0, r0,#0xff
	orrne	reg_a, r0, reg_a,lsr #24	
	moveq	reg_a, reg_a, lsr #16
    mov     reg_d, reg_d, lsr #16
	orr  	reg_a, reg_a, reg_d, lsl #16
	@ shift x&y according to the current mode (index, memory bits)
	tst		rstatus,#mask_index
	movne	reg_x,reg_x,lsr #24
	movne	reg_y,reg_y,lsr #24
	moveq	reg_x,reg_x,lsr #16
	moveq	reg_y,reg_y,lsr #16
    @ reg_x, reg_s
	orr  	reg_x, reg_x, reg_s, lsl #16
    @ store
    add     r0,reg_cpu_var,#8
    stmia   r0,{r1,reg_a,reg_x,reg_y,rpc,reg_cycles,regpcbase}

/*
    @ reg_d & reg_p_bank is same register
	strb		reg_p_bank,[reg_cpu_var,#rpb_ofs]
	mov		rscratch,reg_d, lsr #16
	strh		rscratch,[reg_cpu_var,#rd_ofs]
	@ rstatus & reg_d_bank is same register
	strb		reg_d_bank,[reg_cpu_var,#rdb_ofs]
	movs		rscratch, rstatus, lsr #status_shifter  
	orrcs		rscratch,rscratch,#0x100 @ emulation bit
	strh		rscratch,[reg_cpu_var,#rp_ofs]
	@ 
	@ shift x,y & a according to the current mode (index, memory bits)
	tst		rstatus,#mask_index
	movne		rscratch,reg_x,lsr #24
	movne		rscratch2,reg_y,lsr #24
	moveq		rscratch,reg_x,lsr #16
	moveq		rscratch2,reg_y,lsr #16
	strh		rscratch,[reg_cpu_var,#rx_ofs]
	strh		rscratch2,[reg_cpu_var,#ry_ofs]
	tst		rstatus,#mask_mem
	ldrneh		rscratch,[reg_cpu_var,#ra_ofs]
	bicne		rscratch,rscratch,#0xff
	orrne		rscratch,rscratch,reg_a,lsr #24	
	moveq		rscratch,reg_a,lsr #16
	strh		rscratch,[reg_cpu_var,#ra_ofs]
	
	strh		reg_s,[reg_cpu_var,#rs_ofs]	
	str		regpcbase,[reg_cpu_var,#pcbase_ofs]
	str		rpc,[reg_cpu_var,#pc_ofs]
	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs]
*/
.endm

/*****************************************************************/
.macro		add1cycle		
		add	reg_cycles,reg_cycles, #one_cycle		
.endm
.macro		add1cyclene
		addne	reg_cycles,reg_cycles, #one_cycle		
.endm		
.macro		add1cycleeq
		addeq	reg_cycles,reg_cycles, #one_cycle		
.endm		

.macro		add2cycle
		add	reg_cycles,reg_cycles, #two_cycle
.endm
.macro		add2cyclene
		addne	reg_cycles,reg_cycles, #two_cycle
.endm
.macro		add2cycle2mem		
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]
		add	reg_cycles,reg_cycles, #two_cycle
		add	reg_cycles, reg_cycles, rscratch, lsl #1		
.endm
.macro		add2cycle1mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]
		add	reg_cycles,reg_cycles, #two_cycle
		add	reg_cycles, reg_cycles, rscratch
.endm

.macro		add3cycle
		add	reg_cycles,reg_cycles, #three_cycle
.endm

.macro		add1cycle1mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]
		add	reg_cycles,reg_cycles, #one_cycle
		add	reg_cycles, reg_cycles, rscratch
.endm

.macro		add1cycle2mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]
		add	reg_cycles,reg_cycles, #one_cycle
		add	reg_cycles, reg_cycles, rscratch, lsl #1
.endm

.macro		add1mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]		
		add	reg_cycles, reg_cycles, rscratch
.endm
			
.macro		add2mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]		
		add	reg_cycles, reg_cycles, rscratch, lsl #1
.endm
			
.macro		add3mem
		ldr	rscratch,[reg_cpu_var,#memspeed_ofs]		
		add	reg_cycles, rscratch, reg_cycles
		add	reg_cycles, reg_cycles, rscratch, lsl #1
.endm

/**************/
.macro		cleardecimal
		bic	rstatus,rstatus,#mask_decimal	
.endm			
.macro		setdecimal
		orr	rstatus,rstatus,#mask_decimal	
.endm
.macro		setirq
		orr	rstatus,rstatus,#mask_irq
.endm						
.macro		clearirq
		bic	rstatus,rstatus,#mask_irq
.endm

.macro		cpushutdown
@ if (settings.shutdown && cpu.pc == cpu.waitaddress)
		ldr		rscratch,[reg_cpu_var,#waitaddress_ofs]
		cmp		rpc,rscratch
		bne		5431f
@ if (cpu.waitcounter == 0 && !(cpu.flags & (irq_pending_flag | nmi_flag)))		
		ldr		rscratch,[reg_cpu_var,#flags_ofs]
		ldr		rscratch2,[reg_cpu_var,#waitcounter_ofs]
		tst		rscratch,#irq_pending_flag_orr_nmi_flag
		bne		5432f		
		movs		rscratch2,rscratch2
		bne		5432f
@ cpu.waitaddress = null;		
		mov		rscratch,#0
		str		rscratch,[reg_cpu_var,#waitaddress_ofs]
@ if (settings.sa1)
@ 		s9xsa1executeduringsleep ();		: todo
		
@ 	    cpu.cycles = cpu.nextevent;
		ldr		reg_cycles,[reg_cpu_var,#nextevent_ofs]
		ldrb		r0,[reg_cpu_var,#apuexecuting_ofs]
		movs		r0,r0
		beq		5431f
@ 	    if (iapu.apuexecuting)
/*	    {
		icpu.cpuexecuting = false;
		do
		{
		    apu_execute1();
		} while (apu.cycles < cpu.nextevent);
		icpu.cpuexecuting = true;
	    }
	*/					
		asmapu_execute2
		b		5431f
@@.pool		
5432:
/*	else
	if (cpu.waitcounter >= 2)
	    cpu.waitcounter = 1;
	else
	    cpu.waitcounter--;
*/
		cmp		rscratch2,#1
		movhi		rscratch2,#1
		@ subls		rscratch2,rscratch2,#1
		movls		rscratch2,#0
		str		rscratch2,[reg_cpu_var,#waitcounter_ofs]
5431:		

.endm						
.macro		branchcheck0	
		/*in rsctach : opaddress
		/*destroy rscratch2*/
		ldrb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		movs	rscratch2,rscratch2	
		beq	1110f
		mov	rscratch2,#0		
		strb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		sub	rscratch2,rpc,regpcbase
		@ if( cpu.pc - cpu.pcbase > opaddress) return;
		cmp	rscratch2,rscratch
		bhi	1111f
1110:		
.endm									
.macro		branchcheck1		
		/*in rsctach : opaddress
		/*destroy rscratch2*/
		ldrb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		movs	rscratch2,rscratch2	
		beq	1110f
		mov	rscratch2,#0		
		strb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		sub	rscratch2,rpc,regpcbase
		@ if( cpu.pc - cpu.pcbase > opaddress) return;
		cmp	rscratch2,rscratch
		bhi	1111f
1110:
.endm												
.macro		branchcheck2
		/*in rsctach : opaddress
		/*destroy rscratch2*/
		ldrb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		movs	rscratch2,rscratch2	
		beq	1110f
		mov	rscratch2,#0		
		strb	rscratch2,[reg_cpu_var,#branchskip_ofs]
		sub	rscratch2,rpc,regpcbase
		@ if( cpu.pc - cpu.pcbase > opaddress) return;
		cmp	rscratch2,rscratch
		bhi	1111f
1110:		
.endm
			
.macro		s9xsetpcbase
		@  in  : rscratch (0x00hhmmll)				
		prepare_c_call			
		bl	_asm_S9xSetPCBase		
		restore_c_call
		ldr	rpc,[reg_cpu_var,#pc_ofs]
		ldr	regpcbase,[reg_cpu_var,#pcbase_ofs]
.endm		

.macro		s9xfixcycles
		tst		rstatus, #mask_emul
		ldrne	rscratch, [reg_cpu_var,#jump1]
		bne		991111f
		@ emulation=0
		tst		rstatus,#mask_mem
		beq		991112f
		@ memory=1
		tst		rstatus,#mask_index
		@ index=1  @ mode 0 : m=1,x=1
		ldrne		rscratch, [reg_cpu_var,#jump1]		
		@ index=0  @ mode 1 : m=1,x=0
		ldreq		rscratch, [reg_cpu_var,#jump2]
		b		991111f
991112:		@ memory=0		
		tst		rstatus,#mask_index
		@ index=1   @ mode 3 : m=0,x=1
		ldrne		rscratch, [reg_cpu_var,#jump4]
		@ index=0   @ mode 2 : m=0,x=0
		ldreq		rscratch, [reg_cpu_var,#jump3]
991111:
		str		rscratch,[reg_cpu_var,#asm_optable_ofs]
.endm

.macro		s9xdohblankprocessing
		save_regs
		prepare_c_call_light
@		bl	asm_s9xdohblankprocessing
		bl	_S9xDoHBlankProcessing @ let's go straight to number one
		restore_c_call_light
		load_regs		
.endm

/********************************/
.macro		exec_op					
		ldr		r1,[reg_cpu_var,#asm_optable_ofs]
		str		rpc,[reg_cpu_var,#pcatopcodestart_ofs]
		add1mem
		ldrb		r0, [rpc], #1		
		
		ldr		pc, [r1,r0, lsl #2]
.endm
.macro		nextopcode
		ldr			rscratch,[reg_cpu_var,#nextevent_ofs]
		cmp			reg_cycles,rscratch
		blt			mainloop
  		s9xdohblankprocessing
		b			mainloop
.endm

.macro		asmapu_execute
		ldrb		r0,[reg_cpu_var,#apuexecuting_ofs]
		cmp 		r0,#1   @ spc700 enabled, hack mode off
		bne		    43210f
		ldr		    r0,[reg_cpu_var,#apu_cycles]
        subs	    r0,reg_cycles,r0
        bmi         43210f
.if asm_spc700
		prepare_c_call_lightr12
		bl		_spc700_execute
		restore_c_call_lightr12
        sub     r0,reg_cycles,r0 @ sub cycles left
		str		r0,[reg_cpu_var,#apu_cycles]
.else
        @ save_regs
		str		reg_cycles,[reg_cpu_var,#cycles_ofs]
		prepare_c_call_lightr12
		bl		_asm_APU_EXECUTE
		restore_c_call_lightr12
		ldr		reg_cycles,[reg_cpu_var,#cycles_ofs]
.endif
        @ load_regs
		@ s9xfixcycles
43210:
.endm

.macro		asmapu_execute2
.if asm_spc700
		ldrb		r0,[reg_cpu_var,#apuexecuting_ofs]
		cmp 		r0,#1   @ spc700 enabled, hack mode off
		bne		    43211f
		ldr		    r0,[reg_cpu_var,#apu_cycles]
        subs	    r0,reg_cycles,r0 @ reg_cycles == nextevent
        ble         43211f
		prepare_c_call_lightr12
		bl		_spc700_execute
		restore_c_call_lightr12
        sub     r0,reg_cycles,r0 @ sub cycles left
		str		r0,[reg_cpu_var,#apu_cycles]
43211:
.else
		@ save_regs		
		str		reg_cycles,[reg_cpu_var,#cycles_ofs]
		prepare_c_call_lightr12
		bl		_asm_APU_EXECUTE2
		restore_c_call_lightr12
		ldr		reg_cycles,[reg_cpu_var,#cycles_ofs]		
		@ load_regs
.endif
.endm

@ #include "os9x_65c816_mac_mem.h"
.macro		s9xgetword	
		@  in  : rscratch (0x00hhmmll)
		@  out : rscratch (0xhhll0000)
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetword
		mov	r0,r0
		mov	r0, r0, lsl #16
.endm
.macro		s9xgetwordlow	
		@  in  : rscratch (0x00hhmmll)
		@  out : rscratch (0x0000hhll)		
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetword
		mov	r0,r0		
.endm
.macro		s9xgetwordregstatus	reg
		@  in  : rscratch (0x00hhmmll) 
		@  out : reg      (0xhhll0000)
		@  flags have to be updated with read value
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetword
		mov	r0,r0
		movs	$0, r0, lsl #16
.endm
.macro		s9xgetwordregns	reg
		@  in  : rscratch (0x00hhmmll) 
		@  out : reg (0xhhll0000)
		@  does not destroy rscratch (r0)
		stmfd	r13!,{r0}
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetword
		mov	r0,r0
		mov	$0, r0, lsl #16
		ldmfd	r13!,{r0}
.endm			
.macro		s9xgetwordlowregns	reg
		@  in  : rscratch (0x00hhmmll) 
		@  out : reg (0xhhll0000)
		@  does not destroy rscratch (r0)
		stmfd	r13!,{r0}
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetword
		mov	r0,r0
		mov	$0, r0
		ldmfd	r13!,{r0}
.endm			

.macro		s9xgetbyte 	
		@  in  : rscratch (0x00hhmmll)
		@  out : rscratch (0xll000000)
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetbyte
		mov	r0,r0
		mov	r0, r0, lsl #24
.endm
.macro		s9xgetbytelow
		@  in  : rscratch (0x00hhmmll) 
		@  out : rscratch (0x000000ll)		
		stmfd	r13!,{pc}		
		b	_asms9xgetbyte
		mov	r0,r0
.endm
.macro		s9xgetbyteregstatus	reg
		@  in  : rscratch (0x00hhmmll)
		@  out : reg      (0xll000000)
		@  flags have to be updated with read value
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetbyte
		mov	r0,r0
		movs	$0, r0, lsl #24
.endm
.macro		s9xgetbyteregns	reg
		@  in  : rscratch (0x00hhmmll) 
		@  out : reg      (0xll000000)
		@  does not destroy rscratch (r0)
		stmfd	r13!,{r0}
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetbyte
		mov	r0,r0
		movs	$0, r0, lsl #24
		ldmfd	r13!,{r0}
.endm
.macro		s9xgetbytelowregns	reg
		@  in  : rscratch (0x00hhmmll) 
		@  out : reg      (0x000000ll)
		@  does not destroy rscratch (r0)
		stmfd	r13!,{r0}
		stmfd	r13!,{pc} @ push return address
		b	_asms9xgetbyte
		mov	r0,r0
		movs	$0, r0
		ldmfd	r13!,{r0}
.endm

.macro		s9xsetword	regvalue		
		@  in  : regvalue  (0xhhll0000)
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,$0, lsr #16
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetword
		mov	r0,r0		
.endm
.macro		s9xsetwordzero	
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,#0
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetword
		mov	r0,r0		
.endm
.macro		s9xsetwordlow	regvalue		
		@  in  : regvalue  (0x0000hhll)
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,$0
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetword
		mov	r0,r0		
.endm
.macro		s9xsetbyte	regvalue
		@  in  : regvalue  (0xll000000)
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,$0, lsr #24
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetbyte
		mov	r0,r0		
.endm
.macro		s9xsetbytezero			
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,#0
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetbyte
		mov	r0,r0		
.endm
.macro		s9xsetbytelow	regvalue
		@  in  : regvalue  (0x000000ll)
		@  in  : rscratch=address   (0x00hhmmll)
		mov	r1,$0
		stmfd	r13!,{pc} @ push return address
		b	_asms9xsetbyte
		mov	r0,r0
.endm


@  ===========================================
@  ===========================================
@  adressing mode
@  ===========================================
@  ===========================================


.macro		absolute		
		add2mem		
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc],#2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
.endm
.macro		absoluteindexedindirectx0
		add2mem		
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		add	rscratch    , reg_x, rscratch, lsl #16
		mov	rscratch , rscratch, lsr #16
		orr	rscratch    , rscratch,	reg_p_bank, lsl #16
		s9xgetwordlow
		
.endm
.macro		absoluteindexedindirectx1
		add2mem		
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		add	rscratch    , rscratch, reg_x, lsr #24
		bic	rscratch , rscratch, #0x00ff0000
		orr	rscratch    , rscratch,	reg_p_bank, lsl #16
		s9xgetwordlow
		
.endm
.macro		absoluteindirectlong		
		add2mem
		ldrb			rscratch2    , [rpc, #1]
		ldrb			rscratch   , [rpc], #2
		orr			rscratch    , rscratch,	rscratch2, lsl #8
		s9xgetwordlowregns 	rscratch2
		add			rscratch   , rscratch,	#2
		stmfd			r13!,{rscratch2}
		s9xgetbytelow
		ldmfd			r13!,{rscratch2}
		orr			rscratch   , rscratch2, rscratch, lsl #16
.endm
.macro		absoluteindirect
		add2mem
		ldrb	rscratch2    , [rpc,#1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		s9xgetwordlow
		orr	rscratch    , rscratch,	reg_p_bank, lsl #16
.endm
.macro		absoluteindexedx0		
		add2mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_x, lsr #16
.endm
.macro		absoluteindexedx1
		add2mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_x, lsr #24
.endm


.macro		absoluteindexedy0
		add2mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_y, lsr #16
.endm
.macro		absoluteindexedy1
		add2mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_y, lsr #24
.endm
.macro		absolutelong
		add3mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		ldrb	rscratch2   , [rpc], #1
		orr	rscratch    , rscratch,	rscratch2, lsl #16
.endm


.macro		absolutelongindexedx0
		add3mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		ldrb	rscratch2   , [rpc], #1
		orr	rscratch    , rscratch,	rscratch2, lsl #16
		add	rscratch    , rscratch,	reg_x, lsr #16
		bic	rscratch, rscratch, #0xff000000
.endm
.macro		absolutelongindexedx1
		add3mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		ldrb	rscratch2   , [rpc], #1
		orr	rscratch    , rscratch,	rscratch2, lsl #16
		add	rscratch    , rscratch,	reg_x, lsr #24
		bic	rscratch, rscratch, #0xff000000		
.endm
.macro		direct
		add1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , reg_d, rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
.endm
.macro		directindirect
		add1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , reg_d, rscratch,	 lsl #16		
		mov	rscratch, rscratch, lsr #16
		s9xgetwordlow
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
.endm
.macro		directindirectlong
		add1mem
		ldrb			rscratch    , [rpc], #1
		add			rscratch    , reg_d, rscratch,	 lsl #16
		mov			rscratch, rscratch, lsr #16		
		s9xgetwordlowregns	rscratch2
		add			rscratch    , rscratch,#2
		stmfd			r13!,{rscratch2}
		s9xgetbytelow
		ldmfd			r13!,{rscratch2}
		orr			rscratch   , rscratch2, rscratch, lsl #16
.endm
.macro		directindirectindexed0
		add1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , reg_d, rscratch,	 lsl #16
		mov	rscratch, rscratch, lsr #16
		s9xgetwordlow
		orr	rscratch, rscratch,reg_d_bank, lsl #16
		add	rscratch, rscratch,reg_y, lsr #16
.endm
.macro		directindirectindexed1
		add1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , reg_d, rscratch,	 lsl #16
		mov	rscratch, rscratch, lsr #16
		s9xgetwordlow
		orr	rscratch, rscratch,reg_d_bank, lsl #16
		add	rscratch, rscratch,reg_y, lsr #24
.endm
.macro		directindirectindexedlong0
		add1mem
		ldrb			rscratch    , [rpc], #1
		add			rscratch    , reg_d, rscratch,	 lsl #16
		mov			rscratch, rscratch, lsr #16		
		s9xgetwordlowregns	rscratch2
		add			rscratch    , rscratch,#2
		stmfd			r13!,{rscratch2}
		s9xgetbytelow
		ldmfd			r13!,{rscratch2}
		orr			rscratch   , rscratch2, rscratch, lsl #16				
		add			rscratch, rscratch,reg_y, lsr #16
.endm
.macro		directindirectindexedlong1
		add1mem
		ldrb			rscratch    , [rpc], #1
		add			rscratch    , reg_d, rscratch,	 lsl #16
		mov			rscratch, rscratch, lsr #16
		s9xgetwordlowregns	rscratch2
		add			rscratch    , rscratch,#2
		stmfd			r13!,{rscratch2}
		s9xgetbytelow
		ldmfd			r13!,{rscratch2}
		orr			rscratch   , rscratch2, rscratch, lsl #16
		add			rscratch, rscratch,reg_y, lsr #24
.endm
.macro		directindexedindirect0
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1				
		add	rscratch2   , reg_d , reg_x
		add	rscratch    , rscratch2 , rscratch, lsl #16		
		mov	rscratch, rscratch, lsr #16
		s9xgetwordlow
		orr	rscratch    , rscratch , reg_d_bank, lsl #16		
.endm
.macro		directindexedindirect1
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch2   , reg_d , reg_x, lsr #8
		add	rscratch    , rscratch2 , rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
		s9xgetwordlow
		orr	rscratch    , rscratch , reg_d_bank, lsl #16		
.endm
.macro		directindexedx0
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch2   , reg_d , reg_x
		add	rscratch    , rscratch2 , rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
.endm
.macro		directindexedx1
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch2   , reg_d , reg_x, lsr #8
		add	rscratch    , rscratch2 , rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
.endm
.macro		directindexedy0
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch2   , reg_d , reg_y
		add	rscratch    , rscratch2 , rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
.endm
.macro		directindexedy1
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch2   , reg_d , reg_y, lsr #8
		add	rscratch    , rscratch2 , rscratch, lsl #16
		mov	rscratch, rscratch, lsr #16
.endm
.macro		immediate8
		add	rscratch, rpc, reg_p_bank, lsl #16
		sub	rscratch, rscratch, regpcbase
		add	rpc, rpc, #1
.endm
.macro		immediate16
		add	rscratch, rpc, reg_p_bank, lsl #16
		sub	rscratch, rscratch, regpcbase
		add	rpc, rpc, #2
.endm
.macro		asmrelative
		add1mem
		ldrsb	rscratch    , [rpc],#1
		add	rscratch , rscratch , rpc
		sub	rscratch , rscratch, regpcbase		
		bic	rscratch,rscratch,#0x00ff0000
		bic	rscratch,rscratch,#0xff000000
.endm
.macro		asmrelativelong
		add1cycle2mem
		ldrb	rscratch2    , [rpc, #1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch    , rscratch,	rscratch2, lsl #8
		sub	rscratch2    , rpc, regpcbase
		add	rscratch    , rscratch2, rscratch		
		bic	rscratch,rscratch,#0x00ff0000
.endm


.macro		stackasmrelative
		add1cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , rscratch,	reg_s
		bic	rscratch,rscratch,#0x00ff0000
.endm
.macro		stackasmrelativeindirectindexed0
		add2cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , rscratch,	reg_s
		bic	rscratch,rscratch,#0x00ff0000
		s9xgetwordlow
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_y, lsr #16
		bic	rscratch, rscratch, #0xff000000
.endm
.macro		stackasmrelativeindirectindexed1
		add2cycle1mem
		ldrb	rscratch    , [rpc], #1
		add	rscratch    , rscratch,	reg_s
		bic	rscratch,rscratch,#0x00ff0000
		s9xgetwordlow
		orr	rscratch    , rscratch,	reg_d_bank, lsl #16
		add	rscratch    , rscratch,	reg_y, lsr #24
		bic	rscratch, rscratch, #0xff000000
.endm


/****************************************/
.macro		pushb		reg
		mov		rscratch,reg_s
		s9xsetbyte	$0
		sub		reg_s,reg_s,#1
.endm			
.macro		pushblow	reg
		mov		rscratch,reg_s
		s9xsetbytelow	$0
		sub		reg_s,reg_s,#1
.endm
.macro		pushwlow	reg 
		sub		rscratch,reg_s,#1
		s9xsetwordlow	$0
		sub		reg_s,reg_s,#2
.endm			
.macro		pushwrlow	
		mov		rscratch2,rscratch
		sub		rscratch,reg_s,#1
		s9xsetwordlow	rscratch2
		sub		reg_s,reg_s,#2
.endm			
.macro		pushw		reg
		sub		rscratch,reg_s,#1
		s9xsetword	$0
		sub		reg_s,reg_s,#2
.endm

/********/

.macro		pullb		reg
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		mov		$0,rscratch,lsl #24
.endm
.macro		pullbr		
		add		rscratch,reg_s,#1
		s9xgetbyte
		add		reg_s,reg_s,#1		
.endm
.macro		pullblow	reg
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		mov		$0,rscratch
.endm
.macro		pullbrlow
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1		
.endm
.macro		pullw		reg
		add		rscratch,reg_s,#1
		s9xgetwordlow
		add		reg_s,reg_s,#2
		mov		$0,rscratch,lsl #16
.endm

.macro		pullwlow	reg
		add		rscratch,reg_s,#1
		s9xgetwordlow	
		add		reg_s,reg_s,#2
		mov		$0,rscratch
.endm


/*****************/
.macro		pullbs		reg
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		movs		$0,rscratch,lsl #24
.endm
.macro		pullbrs	
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		movs		rscratch,rscratch,lsl #24
.endm
.macro		pullblows	reg
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		movs		$0,rscratch
.endm
.macro		pullbrlows	
		add		rscratch,reg_s,#1
		s9xgetbytelow
		add		reg_s,reg_s,#1
		movs		rscratch,rscratch
.endm
.macro		pullws		reg
		add		rscratch,reg_s,#1
		s9xgetwordlow
		add		reg_s,reg_s,#2
		movs		$0,rscratch, lsl #16
.endm
.macro		pullwrs		
		add		rscratch,reg_s,#1
		s9xgetwordlow
		add		reg_s,reg_s,#2
		movs		rscratch,rscratch, lsl #16
.endm
.macro		pullwlows	reg
		add		rscratch,reg_s,#1
		s9xgetwordlow
		add		reg_s,reg_s,#2
		movs		$0,rscratch
.endm
.macro		pullwrlows	
		add		rscratch,reg_s,#1
		s9xgetwordlow
		add		reg_s,reg_s,#2
		movs		rscratch,rscratch
.endm


.globl _asms9xgetbyte
.globl _asms9xgetword
.globl _asms9xsetbyte
.globl _asms9xsetword

@ uint8 aas9xgetbyte(uint32 address);
_asms9xgetbyte:
	@  in : r0  = 0x00hhmmll
	@  out : r0 = 0x000000ll
	@  destroyed : r1,r2,r3
	@  update : reg_cycles
	@ r1 <= block	
	mov		r1,r0,lsr #memmap_shift
	@ memmap_shift is 12, address is 0xffffffff at max, so
	@ r1 is maxed by 0x000fffff, memmap_mask is 0x1000-1=0xfff
	@ so and memmap_mask is bic 0xff000
	bic		r1,r1,#0xff000
	@ r2 <= map[block] (getaddress)
	ldr		r2,[reg_cpu_var,#map_ofs]
	ldr		r2,[r2,r1,lsl #2]
	cmp		r2,#map_last
	blo		gbspecial  @ special
	@  direct rom/ram acess
	@ r2 <= getaddress + address & 0xffff	
	@ r3 <= memoryspeed[block]			
	ldr		r3,[reg_cpu_var,#memoryspeed_ofs]
	mov		r0,r0,lsl #16		
	ldrb		r3,[r3,r1]
	add		r2,r2,r0,lsr #16
	@ update cpu.cycles
	add		reg_cycles,reg_cycles,r3	
	@ r3 = blockisram[block]
	ldr		r3,[reg_cpu_var,#blockisram_ofs]
	@ get value to return
	ldrb		r0,[r2]
	ldrb		r3,[r3,r1]
	movs		r3,r3
	@  if blockisram => update for cpushutdown
	ldrne		r1,[reg_cpu_var,#pcatopcodestart_ofs]
	strne		r1,[reg_cpu_var,#waitaddress_ofs]
	
	ldmfd		r13!,{pc} @ return
gbspecial:
	
	ldr		pc,[pc,r2,lsl #2]
	mov		r0,r0 		@ nop, for align
	.long gbppu
	.long gbcpu
	.long gbdsp
	.long gblsram
	.long gbhsram
	.long gbnone
	.long gbdebug
	.long gbc4
	.long gbbwram
	.long gbnone
	.long gbnone
	.long gbnone
	/*.long gb7rom
	.long gb7ram
	.long gb7srm*/
gbppu:
	@ indma ?
	ldrb		r1,[reg_cpu_var,#indma_ofs]
	movs		r1,r1	
	addeq		reg_cycles,reg_cycles,#one_cycle		@ no -> update cycles
	mov		r0,r0,lsl #16	@ s9xgetppu(address&0xffff);
	str		reg_cycles,[reg_cpu_var,#cycles_ofs]	@ save cycles
	mov		r0,r0,lsr #16	
		prepare_c_call
	bl		_S9xGetPPU
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gbcpu:	
	add		reg_cycles,reg_cycles,#one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetcpu(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call
	bl		_S9xGetCPU
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gbdsp:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetcpu(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call
	bl		_S9xGetDSP		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gblsram:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles		
	ldrh		r2,[reg_cpu_var,#srammask]
	ldr		r1,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask
	ldrb		r0,[r1,r0]		@ *memory.sram + address&srammask
	ldmfd		r13!,{pc}
gb7srm:	
gbhsram:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles		
	
	mov		r1,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r1,r1,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)
	add		r0,r2,r1
	ldrh		r2,[reg_cpu_var,#srammask]
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	ldr		r1,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask	
	ldrb		r0,[r1,r0]		@ *memory.sram + address&srammask
	ldmfd		r13!,{pc}		@ return
gb7rom:
gb7ram:	
gbnone:
	mov		r0,r0,lsr #8
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles
	and		r0,r0,#0xff
	ldmfd		r13!,{pc}
@ gbdebug:
	/*add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles
	mov		r0,#0
	ldmfd		r13!,{pc}*/
gbc4:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetc4(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call
	bl		_S9xGetC4
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles		
	ldmfd		r13!,{pc} @ return
gbdebug:	
gbbwram:
	mov		r0,r0,lsl #17  
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsr #17	@ address&0x7fff			
	ldr		r1,[reg_cpu_var,#bwram]	
	sub		r0,r0,#0x6000   @ ((address & 0x7fff) - 0x6000)	
	ldrb		r0,[r0,r1]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)	
	ldmfd		r13!,{pc}


@ uint16 aas9xgetword(uint32 address);
_asms9xgetword:
	@  in : r0  = 0x00hhmmll
	@  out : r0 = 0x000000ll
	@  destroyed : r1,r2,r3
	@  update : reg_cycles
	
	
	mov		r1,r0,lsl #19	
	adds		r1,r1,#0x80000
	@ if = 0x1fff => 0
	bne		gw_notboundary
	
	stmfd		r13!,{r0}
		stmfd		r13!,{pc}
	b		_asms9xgetbyte
		mov		r0,r0
	ldmfd		r13!,{r1}
	stmfd		r13!,{r0}
	add		r0,r1,#1
		stmfd		r13!,{pc}
	b		_asms9xgetbyte
		mov		r0,r0
	ldmfd		r13!,{r1}
	orr		r0,r1,r0,lsl #8
	ldmfd		r13!,{pc}
	
gw_notboundary:	
	
	@ r1 <= block	
	mov		r1,r0,lsr #memmap_shift
	@ memmap_shift is 12, address is 0xffffffff at max, so
	@ r1 is maxed by 0x000fffff, memmap_mask is 0x1000-1=0xfff
	@ so and memmap_mask is bic 0xff000
	bic		r1,r1,#0xff000
	@ r2 <= map[block] (getaddress)
	ldr		r2,[reg_cpu_var,#map_ofs]
	ldr		r2,[r2,r1,lsl #2]
	cmp		r2,#map_last
	blo		gwspecial  @ special
	@  direct rom/ram acess
	
	tst		r0,#1	
	bne		gw_not_aligned1
	@ r2 <= getaddress + address & 0xffff	
	@ r3 <= memoryspeed[block]			
	ldr		r3,[reg_cpu_var,#memoryspeed_ofs]
	mov		r0,r0,lsl #16
	ldrb		r3,[r3,r1]	
	mov		r0,r0,lsr #16
	@ update cpu.cycles
	add		reg_cycles,reg_cycles,r3, lsl #1
	@ r3 = blockisram[block]
	ldr		r3,[reg_cpu_var,#blockisram_ofs]
	@ get value to return
	ldrh		r0,[r2,r0]
	ldrb		r3,[r3,r1]
	movs		r3,r3
	@  if blockisram => update for cpushutdown
	ldrne		r1,[reg_cpu_var,#pcatopcodestart_ofs]
	strne		r1,[reg_cpu_var,#waitaddress_ofs]
	
	ldmfd		r13!,{pc} @ return
gw_not_aligned1:			

	mov		r0,r0,lsl #16		
	add		r3,r0,#0x10000
	ldrb		r3,[r2,r3,lsr #16]	@ getaddress+ (address+1)&0xffff
	ldrb		r0,[r2,r0,lsr #16]	@ getaddress+ address&0xffff	
	orr		r0,r0,r3,lsl #8	

	@  if blockisram => update for cpushutdown
	ldr		r3,[reg_cpu_var,#blockisram_ofs]	
	ldr		r2,[reg_cpu_var,#memoryspeed_ofs]
	ldrb		r3,[r3,r1]   @ r3 = blockisram[block]
	ldrb		r2,[r2,r1]   @ r2 <= memoryspeed[block]
	movs		r3,r3 	    @ isram ? cpushutdown stuff
	ldrne		r1,[reg_cpu_var,#pcatopcodestart_ofs]	
	strne		r1,[reg_cpu_var,#waitaddress_ofs]			
	add		reg_cycles,reg_cycles,r2, lsl #1 @ update cpu.cycles				
	ldmfd		r13!,{pc}  @ return
gwspecial:
	ldr		pc,[pc,r2,lsl #2]
	mov		r0,r0 		@ nop, for align
	.long gwppu
	.long gwcpu
	.long gwdsp
	.long gwlsram
	.long gwhsram
	.long gwnone
	.long gwdebug
	.long gwc4
	.long gwbwram
	.long gwnone
	.long gwnone
	.long gwnone
	/*.long gw7rom
	.long gw7ram
	.long gw7srm*/
/*	map_ppu, map_cpu, map_dsp, map_lorom_sram, map_hirom_sram,
	map_none, map_debug, map_c4, map_bwram, map_bwram_bitmap,
	map_bwram_bitmap2, map_sa1ram, map_last*/
	
gwppu:
	@ indma ?
	ldrb		r1,[reg_cpu_var,#indma_ofs]
	movs		r1,r1	
	addeq		reg_cycles,reg_cycles,#two_cycle		@ no -> update cycles
	mov		r0,r0,lsl #16	@ s9xgetppu(address&0xffff);
	str		reg_cycles,[reg_cpu_var,#cycles_ofs]	@ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call_r0
	bl		_S9xGetPPU
	ldmfd		r13!,{r1}
	stmfd		r13!,{r0}
	add		r0,r1,#1
	@ bic		r0,r0,#0x10000
	bl		_S9xGetPPU
		restore_c_call_r1
	orr		r0,r1,r0,lsl #8
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gwcpu:	
	add		reg_cycles,reg_cycles,#two_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetcpu(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call_r0
	bl		_S9xGetCPU
	ldmfd		r13!,{r1}
	stmfd		r13!,{r0}
	add		r0,r1,#1
	@ bic		r0,r0,#0x10000
	bl		_S9xGetCPU			
		restore_c_call_r1
	orr		r0,r1,r0,lsl #8
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gwdsp:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetcpu(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call_r0
	bl		_S9xGetDSP
	ldmfd		r13!,{r1}
	stmfd		r13!,{r0}
	add		r0,r1,#1
	@ bic		r0,r0,#0x10000
	bl		_S9xGetDSP	
		restore_c_call_r1
	orr		r0,r1,r0,lsl #8
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gwlsram:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles		
	
	tst		r0,#1
	bne		gw_not_aligned2
	ldrh		r2,[reg_cpu_var,#srammask]
	ldr		r1,[reg_cpu_var,#sram]
	and		r3,r2,r0		@ address&srammask
	ldrh		r0,[r3,r1]		@ *memory.sram + address&srammask		
	ldmfd		r13!,{pc}	@ return
gw_not_aligned2:	
	ldrh		r2,[reg_cpu_var,#srammask]
	ldr		r1,[reg_cpu_var,#sram]	
	and		r3,r2,r0		@ address&srammask
	add		r0,r0,#1
	and		r2,r0,r2		@ address&srammask
	ldrb		r3,[r1,r3]		@ *memory.sram + address&srammask
	ldrb		r2,[r1,r2]		@ *memory.sram + address&srammask
	orr		r0,r3,r2,lsl #8
	ldmfd		r13!,{pc}	@ return
gw7srm:	
gwhsram:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles		
	
	tst		r0,#1
	bne		gw_not_aligned3
	
	mov		r1,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r1,r1,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)
	add		r0,r2,r1
	ldrh		r2,[reg_cpu_var,#srammask]
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	ldr		r1,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask	
	ldrh		r0,[r1,r0]		@ *memory.sram + address&srammask
	ldmfd		r13!,{pc}		@ return
	
gw_not_aligned3:	
	mov		r3,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r3,r3,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)	
	add		r2,r2,r3						
	add		r0,r0,#1	
	sub		r2,r2,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	mov		r3,r0,lsl #17  
	and		r0,r0,#0xf0000
	mov		r3,r3,lsr #17	@ (address+1)&0x7fff	
	mov		r0,r0,lsr #3 @ ((address+1)&0xf0000 >> 3)	
	add		r0,r0,r3	
	ldrh		r3,[reg_cpu_var,#srammask]	@ reload mask	
	sub		r0,r0,#0x6000 @ (((address+1) & 0x7fff) - 0x6000 + (((address+1) & 0xf0000) >> 3))		
	and		r2,r3,r2		@ address...&srammask	
	and		r0,r3,r0		@ (address+1...)&srammask	

	ldr		r3,[reg_cpu_var,#sram]
	ldrb		r0,[r0,r3]		@ *memory.sram + (address...)&srammask	
	ldrb		r2,[r2,r3]		@ *memory.sram + (address+1...)&srammask
	orr		r0,r2,r0,lsl #8
			
	ldmfd		r13!,{pc}		@ return
gw7rom:
gw7ram:	
gwnone:		
	mov		r0,r0,lsl #16
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	mov		r0,r0,lsr #24
	orr		r0,r0,r0,lsl #8
	ldmfd		r13!,{pc}
gwdebug:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	mov		r0,#0
	ldmfd		r13!,{pc}
gwc4:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles	
	mov		r0,r0,lsl #16 @ s9xgetc4(address&0xffff);	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call_r0
	bl		_S9xGetC4
	ldmfd		r13!,{r1}
	stmfd		r13!,{r0}
	add		r0,r1,#1
	@ bic		r0,r0,#0x10000
	bl		_S9xGetC4
		restore_c_call_r1
	orr		r0,r1,r0,lsl #8
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
gwbwram:
	tst		r0,#1
	bne		gw_not_aligned4
	mov		r0,r0,lsl #17  
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	mov		r0,r0,lsr #17	@ address&0x7fff
	ldr		r1,[reg_cpu_var,#bwram]		
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000)		
	ldrh		r0,[r1,r0]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)	
	ldmfd		r13!,{pc}		@ return
gw_not_aligned4:
	mov		r0,r0,lsl #17  	
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	add		r3,r0,#0x20000
	mov		r0,r0,lsr #17	@ address&0x7fff
	mov		r3,r3,lsr #17	@ (address+1)&0x7fff
	ldr		r1,[reg_cpu_var,#bwram]		
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000)	
	sub		r3,r3,#0x6000 @ (((address+1) & 0x7fff) - 0x6000)	
	ldrb		r0,[r1,r0]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)		
	ldrb		r3,[r1,r3]		@ *memory.bwram + (((address+1) & 0x7fff) - 0x6000)	
	orr		r0,r0,r3,lsl #8
	ldmfd		r13!,{pc}		@ return




@ void aas9xsetbyte(uint32 address,uint8 val);
_asms9xsetbyte:
	@  in : r0=0x00hhmmll  r1=0x000000ll	
	@  destroyed : r0,r1,r2,r3
	@  update : reg_cycles	
	@ cpu shutdown
	mov		r2,#0
	str		r2,[reg_cpu_var,#waitaddress_ofs]
	@ 
	
	@ r3 <= block				
	mov		r3,r0,lsr #memmap_shift
	@ memmap_shift is 12, address is 0xffffffff at max, so
	@ r0 is maxed by 0x000fffff, memmap_mask is 0x1000-1=0xfff
	@ so and memmap_mask is bic 0xff000
	bic		r3,r3,#0xff000
	@ r2 <= map[block] (setaddress)
	ldr		r2,[reg_cpu_var,#writemap_ofs]
	ldr		r2,[r2,r3,lsl #2]
	cmp		r2,#map_last
	blo		sbspecial  @ special
	@  direct rom/ram acess
	
	@ r2 <= setaddress + address & 0xffff	
	mov		r0,r0,lsl #16	
	add		r2,r2,r0,lsr #16	
	ldr		r0,[reg_cpu_var,#memoryspeed_ofs]
	@ set byte
	strb		r1,[r2]		
	@ r0 <= memoryspeed[block]
	ldrb		r0,[r0,r3]	
	@ update cpu.cycles
	add		reg_cycles,reg_cycles,r0
	@ cpushutdown
	@ only sa1 here : todo	
	@ return
	ldmfd		r13!,{pc}
sbspecial:
	ldr		pc,[pc,r2,lsl #2]
	mov		r0,r0 		@ nop, for align
	.long sbppu
	.long sbcpu
	.long sbdsp
	.long sblsram
	.long sbhsram
	.long sbnone
	.long sbdebug
	.long sbc4
	.long sbbwram
	.long sbnone
	.long sbnone
	.long sbnone
	/*.long sb7rom
	.long sb7ram
	.long sb7srm*/
sbppu:
	@ indma ?
	ldrb		r2,[reg_cpu_var,#indma_ofs]
	movs		r2,r2	
	addeq		reg_cycles,reg_cycles,#one_cycle		@ no -> update cycles
	mov		r0,r0,lsl #16	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs]	@ save cycles
	mov		r0,r0,lsr #16
		prepare_c_call
	mov		r12,r0
	mov		r0,r1
	mov		r1,r12		
	bl		_S9xSetPPU		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
sbcpu:	
	add		reg_cycles,reg_cycles,#one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff
		prepare_c_call
	mov		r12,r0
	mov		r0,r1
	mov		r1,r12		
	bl		_S9xSetCPU		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
sbdsp:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff
		prepare_c_call
	mov		r12,r0
	mov		r0,r1
	mov		r1,r12		
	bl		_S9xSetDSP		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
sblsram:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles		
	ldrh		r2,[reg_cpu_var,#srammask]
	movs		r2,r2
	ldmeqfd		r13!,{pc} @ return if srammask=0
	ldr		r3,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask	
	strb		r1,[r0,r3]		@ *memory.sram + address&srammask	
	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}  @ return
sb7srm:	
sbhsram:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles		
	
	mov		r3,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r3,r3,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)	
	add		r0,r2,r3	
	
	ldrh		r2,[reg_cpu_var,#srammask]
	movs		r2,r2
	ldmeqfd		r13!,{pc} @ return if srammask=0
	
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	ldr		r3,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask	
	strb		r1,[r0,r3]		@ *memory.sram + address&srammask
	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}	@ return
sb7rom:
sb7ram:	
sbnone:	
sbdebug:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles
	ldmfd		r13!,{pc}
sbc4:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff	
		prepare_c_call
	mov		r12,r0
	mov		r0,r1
	mov		r1,r12		
	bl		_S9xSetC4		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
sbbwram:
	mov		r0,r0,lsl #17  
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles
	mov		r0,r0,lsr #17	@ address&0x7fff			
	ldr		r2,[reg_cpu_var,#bwram]	
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000)	
	strb		r1,[r0,r2]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)
	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	
	ldmfd		r13!,{pc}



@ void aas9xsetword(uint32 address,uint16 val);
_asms9xsetword:
	@  in : r0  = 0x00hhmmll r1=0x0000hhll
	@  destroyed : r0,r1,r2,r3
	@  update : reg_cycles
	@ r1 <= block	
	
	mov		r2,r0,lsl #19	
	adds		r2,r2,#0x80000
	@ if = 0x1fff => 0
	bne		sw_notboundary
	
	stmfd		r13!,{r0,r1}
		stmfd		r13!,{pc}
	b		_asms9xsetbyte
		mov		r0,r0
	ldmfd		r13!,{r0,r1}	
	add		r0,r0,#1
	mov		r1,r1,lsr #8
		stmfd		r13!,{pc}
	b		_asms9xsetbyte
		mov		r0,r0
	
	ldmfd		r13!,{pc}
	
sw_notboundary:	
	
	mov		r2,#0
	str		r2,[reg_cpu_var,#waitaddress_ofs]
	@ 	
	@ r3 <= block				
	mov		r3,r0,lsr #memmap_shift
	@ memmap_shift is 12, address is 0xffffffff at max, so
	@ r1 is maxed by 0x000fffff, memmap_mask is 0x1000-1=0xfff
	@ so and memmap_mask is bic 0xff000
	bic		r3,r3,#0xff000
	@ r2 <= map[block] (setaddress)
	ldr		r2,[reg_cpu_var,#writemap_ofs]
	ldr		r2,[r2,r3,lsl #2]
	cmp		r2,#map_last
	blo		swspecial  @ special
	@  direct rom/ram acess		
	
	
	@ check if address is 16bits aligned or not
	tst		r0,#1
	bne		sw_not_aligned1
	@ aligned
	mov		r0,r0,lsl #16
	add		r2,r2,r0,lsr #16	@ address & 0xffff + setaddress
	ldr		r0,[reg_cpu_var,#memoryspeed_ofs]
	@ set word
	strh		r1,[r2]		
	@ r1 <= memoryspeed[block]
	ldrb		r0,[r0,r3]
	@ update cpu.cycles
	add		reg_cycles,reg_cycles,r0, lsl #1
	@ cpushutdown
	@ only sa1 here : todo	
	@ return
	ldmfd		r13!,{pc}
	
sw_not_aligned1:	
	@ r1 = (address&0xffff)<<16
	mov		r0,r0,lsl #16		
	@ first write @address
	strb		r1,[r2,r0,lsr #16]
	add		r0,r0,#0x10000
	mov		r1,r1,lsr #8
	@ second write @address+1
	strb		r1,[r2,r0,lsr #16]	
	@ r1 <= memoryspeed[block]
	ldr		r0,[reg_cpu_var,#memoryspeed_ofs]
	ldrb		r0,[r0,r3]	
	@ update cpu.cycles
	add		reg_cycles,reg_cycles,r0,lsl #1
	@ cpushutdown
	@ only sa1 here : todo	
	@ return
	ldmfd		r13!,{pc}
swspecial:
	ldr		pc,[pc,r2,lsl #2]
	mov		r0,r0 		@ nop, for align
	.long swppu
	.long swcpu
	.long swdsp
	.long swlsram
	.long swhsram
	.long swnone
	.long swdebug
	.long swc4
	.long swbwram
	.long swnone
	.long swnone
	.long swnone
	/*.long sw7rom
	.long sw7ram
	.long sw7srm*/
swppu:
	@ indma ?
	ldrb		r2,[reg_cpu_var,#indma_ofs]
	movs		r2,r2	
	addeq		reg_cycles,reg_cycles,#two_cycle		@ no -> update cycles
	mov		r0,r0,lsl #16	
	str		reg_cycles,[reg_cpu_var,#cycles_ofs]	@ save cycles
	mov		r0,r0,lsr #16
	mov		r2,r1
	mov		r1,r0
	mov		r0,r2
		prepare_c_call_r0r1
	bl		_S9xSetPPU		
	ldmfd		r13!,{r0,r1}
	add		r1,r1,#1
	mov		r0,r0,lsr #8	
	bic		r1,r1,#0x10000		
	bl		_S9xSetPPU		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
swcpu:	
	add		reg_cycles,reg_cycles,#two_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff
	mov		r2,r1
	mov		r1,r0
	mov		r0,r2	
		prepare_c_call_r0r1
	bl		_S9xSetCPU		
	ldmfd		r13!,{r0,r1}
	add		r1,r1,#1
	mov		r0,r0,lsr #8	
	bic		r1,r1,#0x10000		
	bl		_S9xSetCPU		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
swdsp:
	add		reg_cycles,reg_cycles,#slow_one_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff
	mov		r2,r1
	mov		r1,r0
	mov		r0,r2
		prepare_c_call_r0r1
	bl		_S9xSetDSP	
	ldmfd		r13!,{r0,r1}
	add		r1,r1,#1
	mov		r0,r0,lsr #8	
	bic		r1,r1,#0x10000	
	bl		_S9xSetDSP		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
swlsram:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles		
	ldrh		r2,[reg_cpu_var,#srammask]
	movs		r2,r2
	ldmeqfd		r13!,{pc} @ return if srammask=0
			
	and		r3,r2,r0		@ address&srammask
	tst		r0,#1
	bne		sw_not_aligned2
	@ aligned	
	ldr		r0,[reg_cpu_var,#sram]	
	strh		r1,[r0,r3]		@ *memory.sram + address&srammask		
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}  @ return	
sw_not_aligned2:	

	add		r0,r0,#1
	and		r2,r2,r0		@ (address+1)&srammask		
	ldr		r0,[reg_cpu_var,#sram]	
	strb		r1,[r0,r3]		@ *memory.sram + address&srammask
	mov		r1,r1,lsr #8
	strb		r1,[r0,r2]		@ *memory.sram + (address+1)&srammask	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}  @ return
sw7srm:	
swhsram:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles		
	
	ldrh		r2,[reg_cpu_var,#srammask]
	movs		r2,r2
	ldmeqfd		r13!,{pc} @ return if srammask=0
	
	tst		r0,#1
	bne		sw_not_aligned3	
	@ aligned
	mov		r3,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r3,r3,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)	
	add		r0,r2,r3				
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	ldrh		r2,[reg_cpu_var,#srammask]
	ldr		r3,[reg_cpu_var,#sram]	
	and		r0,r2,r0		@ address&srammask	
	strh		r1,[r0,r3]		@ *memory.sram + address&srammask	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}	@ return		
sw_not_aligned3:	
	mov		r3,r0,lsl #17  
	and		r2,r0,#0xf0000
	mov		r3,r3,lsr #17	@ address&0x7fff	
	mov		r2,r2,lsr #3 @ (address&0xf0000 >> 3)	
	add		r2,r2,r3				
	sub		r2,r2,#0x6000 @ ((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3))
	
	add		r0,r0,#1	
	mov		r3,r0,lsl #17  
	and		r0,r0,#0xf0000
	mov		r3,r3,lsr #17	@ (address+1)&0x7fff	
	mov		r0,r0,lsr #3 @ ((address+1)&0xf0000 >> 3)	
	add		r0,r0,r3	
	ldrh		r3,[reg_cpu_var,#srammask]	@ reload mask	
	sub		r0,r0,#0x6000 @ (((address+1) & 0x7fff) - 0x6000 + (((address+1) & 0xf0000) >> 3))		
	and		r2,r3,r2		@ address...&srammask	
	and		r0,r3,r0		@ (address+1...)&srammask	
	
	ldr		r3,[reg_cpu_var,#sram]
	strb		r1,[r2,r3]		@ *memory.sram + (address...)&srammask
	mov		r1,r1,lsr #8
	strb		r1,[r0,r3]		@ *memory.sram + (address+1...)&srammask
	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]		
	ldmfd		r13!,{pc}	@ return	
sw7rom:
sw7ram:	
swnone:	
swdebug:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	ldmfd		r13!,{pc}	@ return
swc4:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles	
	mov		r0,r0,lsl #16 
	str		reg_cycles,[reg_cpu_var,#cycles_ofs] @ save cycles
	mov		r0,r0,lsr #16	@ address&0xffff	
	mov		r2,r1
	mov		r1,r0
	mov		r0,r2
		prepare_c_call_r0r1
	bl		_S9xSetC4		
	ldmfd		r13!,{r0,r1}	
	add		r1,r1,#1
	mov		r0,r0,lsr #8	
	bic		r1,r1,#0x10000		
	bl		_S9xSetC4		
		restore_c_call
	ldr		reg_cycles,[reg_cpu_var,#cycles_ofs] @ load cycles	
	ldmfd		r13!,{pc} @ return
swbwram:
	add		reg_cycles,reg_cycles,#slow_two_cycle	@ update cycles
	tst		r0,#1
	bne		sw_not_aligned4
	@ aligned
	mov		r0,r0,lsl #17		
	ldr		r2,[reg_cpu_var,#bwram]
	mov		r0,r0,lsr #17	@ address&0x7fff			
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000)	
	mov		r3,#1
	strh		r1,[r0,r2]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)			
	strb		r3,[reg_cpu_var,#srammodified_ofs]			
	ldmfd		r13!,{pc}	@ return
sw_not_aligned4:
	mov		r0,r0,lsl #17	
	add		r3,r0,#0x20000
	mov		r0,r0,lsr #17	@ address&0x7fff
	mov		r3,r3,lsr #17	@ (address+1)&0x7fff
	ldr		r2,[reg_cpu_var,#bwram]	
	sub		r0,r0,#0x6000 @ ((address & 0x7fff) - 0x6000)
	sub		r3,r3,#0x6000 @ (((address+1) & 0x7fff) - 0x6000)
	strb		r1,[r2,r0]		@ *memory.bwram + ((address & 0x7fff) - 0x6000)
	mov		r1,r1,lsr #8
	strb		r1,[r2,r3]		@ *memory.bwram + (((address+1) & 0x7fff) - 0x6000)	
	mov		r0,#1
	strb		r0,[reg_cpu_var,#srammodified_ofs]			
	ldmfd		r13!,{pc}		@ return
	




/*****************************************************************
	flags  
*****************************************************************/

.macro		update_c
		@  cc : arm carry clear
		biccc	rstatus, rstatus, #mask_carry  @ 	0 : and	mask 11111011111 : set c to zero
		@  cs : arm carry set
		orrcs	rstatus, rstatus, #mask_carry      @ 	1 : or	mask 00000100000 : set c to one
.endm
.macro		update_z
		@  ne : arm zero clear
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		@  eq : arm zero set
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one		
.endm
.macro		update_zn
		@  ne : arm zero clear
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		@  eq : arm zero set
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
		@  pl : arm neg clear
		bicpl	rstatus, rstatus, #mask_neg	@  0 : and mask 11111011111 : set n to zero
		@  mi : arm neg set
		orrmi	rstatus, rstatus, #mask_neg	@  1 : or  mask 00000100000 : set n to one
.endm

/*****************************************************************
	opcodes_mac
*****************************************************************/




.macro adc8
		tst rstatus, #mask_decimal
		beq 1111f				
		s9xgetbyte		
		
	
	        stmfd 	r13!,{rscratch}		
		mov 	rscratch4,#0x0f000000
		@ rscratch2=xxw1xxxxxxxxxxxx
		and 	rscratch2, rscratch, rscratch4
		@ rscratch=xxw2xxxxxxxxxxxx
		and 	rscratch, rscratch4, rscratch, lsr #4
		@ rscratch3=xxa2xxxxxxxxxxxx
		and 	rscratch3, rscratch4, reg_a, lsr #4
		@ rscratch4=xxa1xxxxxxxxxxxx		
		and 	rscratch4,reg_a,rscratch4		
		@ r1=a1+w1+carry
		tst 	rstatus, #mask_carry
		addne 	rscratch2, rscratch2, #0x01000000
		add 	rscratch2,rscratch2,rscratch4
		@  if r1 > 9
		cmp 	rscratch2, #0x09000000
		@  then r1 -= 10
		subgt 	rscratch2, rscratch2, #0x0a000000
		@  then a2++
		addgt 	rscratch3, rscratch3, #0x01000000
		@  r2 = a2+w2
		add 	rscratch3, rscratch3, rscratch
		@  if r2 > 9
		cmp 	rscratch3, #0x09000000
		@  then r2 -= 10@ 
		subgt 	rscratch3, rscratch3, #0x0a000000
		@  then setcarry()
		orrgt 	rstatus, rstatus, #mask_carry @  1 : or mask 00000100000 : set c to one
		@  else clearcarry()
		bicle 	rstatus, rstatus, #mask_carry @  0 : and mask 11111011111 : set c to zero
		@  gather rscratch3 and rscratch2 into ans8
		@  rscratch3 : 0r2000000
		@  rscratch2 : 0r1000000
		@  -> 0xr2r1000000
		orr 	rscratch2, rscratch2, rscratch3, lsl #4		
		ldmfd 	r13!,{rscratch}
		@ only last bit
		and 	rscratch,rscratch,#0x80000000
		@  (register.al ^ work8)
		eors 	rscratch3, reg_a, rscratch
		bicne 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		bne 	1112f
		@  (work8 ^ ans8)
		eors 	rscratch3, rscratch2, rscratch
		@  & 0x80 
		tstne	rscratch3,#0x80000000
		biceq 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		orrne 	rstatus, rstatus, #mask_overflow @  1 : or mask 00000100000 : set v to one 
1112:
		movs reg_a, rscratch2
		update_zn
		b 1113f
1111:
		s9xgetbytelow
		movs rscratch2, rstatus, lsr #mask_shifter_carry
		subcs rscratch, rscratch, #0x100 
		adcs reg_a, reg_a, rscratch, ror #8
		@ overflow
		orrvs rstatus, rstatus, #mask_overflow
		bicvc rstatus, rstatus, #mask_overflow
		@ carry
		update_c
		@ clear lower part
		ands reg_a, reg_a, #0xff000000
		@ update flag
		update_zn
1113: 
.endm
/* to test */
.macro adc16 
		tst rstatus, #mask_decimal
		beq 1111f 
		s9xgetword
		
		@ rscratch = w3w2w1w0........
		@ PATCH ldr 	rscratch4, = 0x0f0f0000
		mov   rscratch4, #0x0f0000
		orr   rscratch4, rscratch4, #0x0f000000
		@  rscratch2 = xxw2xxw0xxxxxx
		@  rscratch3 = xxw3xxw1xxxxxx
		and 	rscratch2, rscratch4, rscratch
		and 	rscratch3, rscratch4, rscratch, lsr #4 
		@  rscratch2 = xxw3xxw1xxw2xxw0
		orr 	rscratch2, rscratch3, rscratch2, lsr #16 		
		@  rscratch3 = xxa2xxa0xxxxxx
		@  rscratch4 = xxa3xxa1xxxxxx
		@  rscratch2 = xxa3xxa1xxa2xxa0
		and 	rscratch3, rscratch4, reg_a
		and 	rscratch4, rscratch4, reg_a, lsr #4
		orr 	rscratch3, rscratch4, rscratch3, lsr #16		
		add 	rscratch2, rscratch3, rscratch2 		
		@ PATCH ldr 	rscratch4, = 0x0f0f0000
		mov   rscratch4, #0x0f0000
		orr   rscratch4, rscratch4, #0x0f000000
		@  rscratch2 = a + w
		tst 	rstatus, #mask_carry
		addne 	rscratch2, rscratch2, #0x1
		@  rscratch2 = a + w + c
		@ a0
		and 	rscratch3, rscratch2, #0x0000001f
		cmp 	rscratch3, #0x00000009
		addhi 	rscratch2, rscratch2, #0x00010000
		subhi 	rscratch2, rscratch2, #0x0000000a
		@ a1
		and 	rscratch3, rscratch2, #0x001f0000
		cmp 	rscratch3, #0x00090000
		addhi 	rscratch2, rscratch2, #0x00000100
		subhi 	rscratch2, rscratch2, #0x000a0000
		@ a2
		and 	rscratch3, rscratch2, #0x00001f00
		cmp 	rscratch3, #0x00000900
		subhi 	rscratch2, rscratch2, #0x00000a00
		addhi 	rscratch2, rscratch2, #0x01000000
		@ a3
		and 	rscratch3, rscratch2, #0x1f000000
		cmp 	rscratch3, #0x09000000
		subhi 	rscratch2, rscratch2, #0x0a000000
		@ setcarry
		orrhi 	rstatus, rstatus, #mask_carry
		@ clearcarry
		bicls 	rstatus, rstatus, #mask_carry
		@ rscratch2 = xxr3xxr1xxr2xxr0
		@ pack result 
		@ rscratch3 = xxr3xxr1xxxxxxxx 
		and 	rscratch3, rscratch4, rscratch2 
		@ rscratch2 = xxr2xxr0xxxxxxxx
		and 	rscratch2, rscratch4, rscratch2,lsl #16
		@ rscratch2 = r3r2r1r0xxxxxxxx
		orr 	rscratch2, rscratch2,rscratch3,lsl #4		
@ only last bit
		and 	rscratch,rscratch,#0x80000000
		@  (register.al ^ work8)
		eors 	rscratch3, reg_a, rscratch 
		bicne 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		bne 	1112f
		@  (work8 ^ ans8)
		eors 	rscratch3, rscratch2, rscratch 
		tstne	rscratch3,#0x80000000
		biceq 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		orrne 	rstatus, rstatus, #mask_overflow @  1 : or mask 00000100000 : set v to one 
1112:
		movs 	reg_a, rscratch2
		update_zn
		b 	1113f
1111:
		s9xgetwordlow
		movs rscratch2, rstatus, lsr #mask_shifter_carry 
		subcs rscratch, rscratch, #0x10000 
		adcs reg_a, reg_a,rscratch, ror #16
		@ overflow 
		orrvs rstatus, rstatus, #mask_overflow
		bicvc rstatus, rstatus, #mask_overflow
		mov reg_a, reg_a, lsr #16
		@ carry
		update_c
		@ clear lower parts 
		movs reg_a, reg_a, lsl #16
		@ update flag
		update_zn
1113: 
.endm


.macro		and16
		s9xgetword
		ands		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		and8
		s9xgetbyte
		ands		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		a_asl8
		@  7	instr		
		movs	reg_a, reg_a, lsl #1
		update_c
		update_zn
		add1cycle
.endm
.macro		a_asl16
		@  7	instr		
		movs	reg_a, reg_a, lsl #1
		update_c
		update_zn
		add1cycle
.endm
.macro		asl16		
		s9xgetwordregns	rscratch2	      @ 	do not destroy opadress	in rscratch
		movs		rscratch2, rscratch2, lsl #1
		update_c
		update_zn		
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		asl8				
		s9xgetbyteregns	rscratch2	      @ 	do not destroy opadress	in rscratch
		movs		rscratch2, rscratch2, lsl #1
		update_c
		update_zn		
		s9xsetbyte 	rscratch2
		add1cycle
.endm
.macro		bit8
		s9xgetbyte
		movs	rscratch2, rscratch, lsl #1
		@  trick in asm : shift one more bit	: arm c	= snes n
		@ 					  arm n	= snes v
		@  if carry set, then set neg in snes
		biccc	rstatus, rstatus, #mask_neg	@  0 : and mask 11111011111 : set c to zero
		orrcs	rstatus, rstatus, #mask_neg	@  1 : or  mask 00000100000 : set c to one
		@  if neg set, then set overflow in snes
		bicpl	rstatus, rstatus, #mask_overflow  @  0 : and mask 11111011111	: set n	to zero
		orrmi	rstatus, rstatus, #mask_overflow	     @  1 : or  mask 00000100000	: set n	to one

		@  now do a real and	with a register
		@  set zero flag, bit test
		ands	rscratch2, reg_a, rscratch
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
.endm

.macro		bit16
		s9xgetword
		movs	rscratch2, rscratch, lsl #1
		@  trick in asm : shift one more bit	: arm c	= snes n
		@ 					  arm n	= snes v
		@  if carry set, then set neg in snes
		biccc	rstatus, rstatus, #mask_neg	@  0 : and mask 11111011111 : set n to zero
		orrcs	rstatus, rstatus, #mask_neg	@  1 : or  mask 00000100000 : set n to one
		@  if neg set, then set overflow in snes
		bicpl	rstatus, rstatus, #mask_overflow  @  0 : and mask 11111011111	: set v	to zero
		orrmi	rstatus, rstatus, #mask_overflow	     @  1 : or  mask 00000100000	: set v	to one
		@  now do a real and	with a register
		@  set zero flag, bit test
		ands	rscratch2, reg_a, rscratch
		@  bit set  ->z=0->xxxne clear flag
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		@  bit clear->z=1->xxxeq set flag
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
.endm
.macro		cmp8
		s9xgetbyte			
		subs 	rscratch2,reg_a,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		
.endm
.macro		cmp16
		s9xgetword
		subs 	rscratch2,reg_a,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		
.endm
.macro		cmx16
		s9xgetword
		subs 	rscratch2,reg_x,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
.endm
.macro		cmx8
		s9xgetbyte
		subs 	rscratch2,reg_x,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
.endm
.macro		cmy16
		s9xgetword
		subs 	rscratch2,reg_y,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
.endm
.macro		cmy8
		s9xgetbyte
		subs 	rscratch2,reg_y,rscratch		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
.endm
.macro		a_dec8		
		mov		rscratch,#0		
		subs		reg_a, reg_a, #0x01000000
		str		rscratch,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		a_dec16		
		mov		rscratch,#0
		subs 		reg_a, reg_a, #0x00010000
		str		rscratch,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		dec16		
		s9xgetwordregns rscratch2	       @  do not	destroy	opadress in rscratch		
		mov		rscratch3,#0
		subs		rscratch2, rscratch2, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn		
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		dec8
		s9xgetbyteregns rscratch2	       @  do not	destroy	opadress in rscratch
		mov		rscratch3,#0
		subs		rscratch2, rscratch2, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn		
		s9xsetbyte 	rscratch2
		add1cycle
.endm
.macro		eor16
		s9xgetword
		eors		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		eor8
		s9xgetbyte
		eors		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		a_inc8		
		mov		rscratch3,#0
		adds		reg_a, reg_a, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		a_inc16		
		mov		rscratch3,#0	
		adds		reg_a, reg_a, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		inc16		
		s9xgetwordregns	rscratch2
		mov		rscratch3,#0
		adds		rscratch2, rscratch2, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn		
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		inc8		
		s9xgetbyteregns	rscratch2
		mov		rscratch3,#0
		adds		rscratch2, rscratch2, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn		
		s9xsetbyte 	rscratch2
		add1cycle
.endm
.macro		lda16
		s9xgetwordregstatus reg_a
		update_zn
.endm
.macro		lda8
		s9xgetbyteregstatus reg_a
		update_zn
.endm
.macro		ldx16
		s9xgetwordregstatus reg_x
		update_zn
.endm
.macro		ldx8
		s9xgetbyteregstatus reg_x
		update_zn
.endm
.macro		ldy16
		s9xgetwordregstatus reg_y
		update_zn
.endm
.macro		ldy8
		s9xgetbyteregstatus reg_y
		update_zn
.endm
.macro		a_lsr16				
		bic	rstatus, rstatus, #mask_neg	 @  0 : and mask	11111011111 : set n to zero
		movs	reg_a, reg_a, lsr #17		 @  hhhhhhhh llllllll 00000000 00000000 -> 00000000 00000000 0hhhhhhh hlllllll
		@  update zero
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		mov	reg_a, reg_a, lsl #16			@  -> 0lllllll 00000000 00000000	00000000
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
		@  note : the two mov are included between instruction, to optimize
		@  the pipeline.
		update_c
		add1cycle
.endm
.macro		a_lsr8		
		bic	rstatus, rstatus, #mask_neg	 @  0 : and mask	11111011111 : set n to zero
		movs	reg_a, reg_a, lsr #25		 @  llllllll 00000000 00000000 00000000 -> 00000000 00000000 00000000 0lllllll
		@  update zero
		bicne	rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		mov	reg_a, reg_a, lsl #24			@  -> 00000000 00000000 00000000	0lllllll
		orreq	rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one		
		@  note : the two mov are included between instruction, to optimize
		@  the pipeline.
		update_c
		add1cycle
.endm
.macro		lsr16				
		s9xgetwordregns	rscratch2
		@  n set to zero by >> 1 lsr
		bic		rstatus, rstatus, #mask_neg	 @  0 : and mask	11111011111 : set n to zero
		movs		rscratch2, rscratch2, lsr #17		   @  llllllll 00000000 00000000	00000000 -> 00000000 00000000 00000000 0lllllll
		@  update carry		
		biccc		rstatus, rstatus, #mask_carry  @ 	0 : and	mask 11111011111 : set c to zero		
		orrcs		rstatus, rstatus, #mask_carry      @ 	1 : or	mask 00000100000 : set c to one
		@  update zero
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one	
		s9xsetwordlow 	rscratch2
		add1cycle
.endm
.macro		lsr8				
		s9xgetbyteregns	rscratch2
		@  n set to zero by >> 1 lsr
		bic		rstatus, rstatus, #mask_neg	 @  0 : and mask	11111011111 : set n to zero
		movs		rscratch2, rscratch2, lsr #25		   @  llllllll 00000000 00000000	00000000 -> 00000000 00000000 00000000 0lllllll
		@  update carry		
		biccc		rstatus, rstatus, #mask_carry  @ 	0 : and	mask 11111011111 : set c to zero		
		orrcs		rstatus, rstatus, #mask_carry      @ 	1 : or	mask 00000100000 : set c to one
		@  update zero
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one		
		s9xsetbytelow 	rscratch2
		add1cycle
.endm
.macro		ora8
		s9xgetbyte
		orrs		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		ora16
		s9xgetword
		orrs		reg_a, reg_a, rscratch
		update_zn
.endm
.macro		a_rol16		
		tst		rstatus, #mask_carry
		orrne		reg_a, reg_a, #0x00008000
		movs		reg_a, reg_a, lsl #1
		update_zn
		update_c
		add1cycle
.endm
.macro		a_rol8		
		tst		rstatus, #mask_carry
		orrne		reg_a, reg_a, #0x00800000
		movs		reg_a, reg_a, lsl #1
		update_zn
		update_c
		add1cycle
.endm
.macro		rol16		
		s9xgetwordregns	rscratch2
		tst		rstatus, #mask_carry
		orrne		rscratch2, rscratch2, #0x00008000
		movs		rscratch2, rscratch2, lsl #1
		update_zn
		update_c		
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		rol8		
		s9xgetbyteregns	rscratch2
		tst		rstatus, #mask_carry
		orrne		rscratch2, rscratch2, #0x00800000
		movs		rscratch2, rscratch2, lsl #1
		update_zn
		update_c		
		s9xsetbyte 	rscratch2
		add1cycle
.endm
.macro		a_ror16		
		mov			reg_a,reg_a, lsr #16
		tst			rstatus, #mask_carry
		orrne			reg_a, reg_a, #0x00010000
		orrne			rstatus,rstatus,#mask_neg
		biceq			rstatus,rstatus,#mask_neg		
		movs			reg_a,reg_a,lsr #1
		update_c
		update_z		
		mov			reg_a,reg_a, lsl #16
		add1cycle
.endm
.macro		a_ror8				
		mov			reg_a,reg_a, lsr #24
		tst			rstatus, #mask_carry
		orrne			reg_a, reg_a, #0x00000100
		orrne			rstatus,rstatus,#mask_neg
		biceq			rstatus,rstatus,#mask_neg		
		movs			reg_a,reg_a,lsr #1
		update_c
		update_z		
		mov			reg_a,reg_a, lsl #24
		add1cycle
.endm
.macro		ror16		
		s9xgetwordlowregns	rscratch2
		tst			rstatus, #mask_carry
		orrne			rscratch2, rscratch2, #0x00010000
		orrne			rstatus,rstatus,#mask_neg
		biceq			rstatus,rstatus,#mask_neg		
		movs			rscratch2,rscratch2,lsr #1
		update_c
		update_z
		s9xsetwordlow 	rscratch2
		add1cycle

.endm
.macro		ror8		
		s9xgetbytelowregns	rscratch2
		tst			rstatus, #mask_carry
		orrne			rscratch2, rscratch2, #0x00000100
		orrne			rstatus,rstatus,#mask_neg
		biceq			rstatus,rstatus,#mask_neg		
		movs			rscratch2,rscratch2,lsr #1
		update_c
		update_z
		s9xsetbytelow 	rscratch2
		add1cycle
.endm

.macro sbc16
        tst rstatus, #mask_decimal
		beq 1111f
		@ todo
		s9xgetword
		
		stmfd 	r13!,{rscratch9}
		mov 	rscratch9,#0x000f0000
        @ rscratch2 - result
        @ rscratch3 - scratch
        @ rscratch4 - scratch
        @ rscratch9 - pattern

		and 	rscratch2, rscratch, #0x000f0000
		tst 	rstatus, #mask_carry
		addeq 	rscratch2, rscratch2, #0x00010000  @ w1=w1+!carry
		and 	rscratch4, reg_a, #0x000f0000
        sub 	rscratch2, rscratch4,rscratch2		@ r1=a1-w1-!carry
		cmp 	rscratch2, #0x00090000	@  if r1 > 9		
		addhi 	rscratch2, rscratch2, #0x000a0000 @  then r1 += 10		
		and	    rscratch2, rscratch2, #0x000f0000

		and 	rscratch3, rscratch9, rscratch, lsr #4
        addhi 	rscratch3, rscratch3, #0x00010000  @  then (w2++)

		and 	rscratch4, rscratch9, reg_a, lsr #4
        sub 	rscratch3, rscratch4, rscratch3		@ r2=a2-w2
		cmp 	rscratch3, #0x00090000	@  if r2 > 9		
		addhi 	rscratch3, rscratch3, #0x000a0000 @  then r2 += 10		
		and	    rscratch3, rscratch3, #0x000f0000
		orr	    rscratch2, rscratch2, rscratch3,lsl #4

		and 	rscratch3, rscratch9, rscratch, lsr #8
        addhi 	rscratch3, rscratch3, #0x00010000  @  then (w3++)

		and 	rscratch4, rscratch9, reg_a, lsr #8
        sub 	rscratch3, rscratch4, rscratch3		@ r3=a3-w3
		cmp 	rscratch3, #0x00090000	@  if r3 > 9		
		addhi 	rscratch3, rscratch3, #0x000a0000 @  then r3 += 10		
		and	    rscratch3, rscratch3, #0x000f0000
		orr	    rscratch2, rscratch2, rscratch3,lsl #8

		and 	rscratch3, rscratch9, rscratch, lsr #12
        addhi 	rscratch3, rscratch3, #0x00010000  @  then (w3++)

		and 	rscratch4, rscratch9, reg_a, lsr #12				
        sub 	rscratch3, rscratch4, rscratch3		@ r4=a4-w4
		cmp 	rscratch3, #0x00090000	@  if r4 > 9		
		addhi 	rscratch3, rscratch3, #0x000a0000 @  then r4 += 10
		bichi 	rstatus, rstatus, #mask_carry	@  then clearcarry
		orrls 	rstatus, rstatus, #mask_carry	@  else setcarry
		
		and	    rscratch3,rscratch3,#0x000f0000
		orr	    rscratch2,rscratch2,rscratch3,lsl #12
		
		ldmfd 	r13!,{rscratch9}
		@ only last bit
		and 	reg_a,reg_a,#0x80000000
		@  (register.a.w ^ work8)			
		eors 	rscratch3, reg_a, rscratch
		biceq 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		beq 	1112f
		@  (register.a.w ^ ans8)
		eors 	rscratch3, reg_a, rscratch2
		@  & 0x80 
		tstne	rscratch3,#0x80000000
		biceq   rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero		
		orrne 	rstatus, rstatus, #mask_overflow @  1 : or mask 00000100000 : set v to one 
1112:
		movs 	reg_a, rscratch2
		update_zn 		
		b 1113f
1111:
		s9xgetwordlow 
		movs rscratch2,rstatus,lsr #mask_shifter_carry
		sbcs reg_a, reg_a, rscratch, lsl #16 
		@ overflow 
		orrvs rstatus, rstatus, #mask_overflow
		bicvc rstatus, rstatus, #mask_overflow
		mov reg_a, reg_a, lsr #16
		@ carry
		update_c
		movs reg_a, reg_a, lsl #16
		@ update flag
		update_zn
1113:
.endm 

.macro sbc8
		tst rstatus, #mask_decimal 
		beq 1111f		
		s9xgetbyte					
		stmfd 	r13!,{rscratch}		
		mov 	rscratch4,#0x0f000000
		@ rscratch2=xxw1xxxxxxxxxxxx
		and 	rscratch2, rscratch, rscratch4
		@ rscratch=xxw2xxxxxxxxxxxx
		and 	rscratch, rscratch4, rscratch, lsr #4				
		@ rscratch3=xxa2xxxxxxxxxxxx
		and 	rscratch3, rscratch4, reg_a, lsr #4
		@ rscratch4=xxa1xxxxxxxxxxxx
		and 	rscratch4,reg_a,rscratch4		
		@ r1=a1-w1-!carry
		tst 	rstatus, #mask_carry
		addeq 	rscratch2, rscratch2, #0x01000000
		sub 	rscratch2,rscratch4,rscratch2
		@  if r1 > 9
		cmp 	rscratch2, #0x09000000
		@  then r1 += 10
		addhi 	rscratch2, rscratch2, #0x0a000000
		@  then a2-- (w2++)
		addhi 	rscratch, rscratch, #0x01000000
		@  r2=a2-w2
		sub 	rscratch3, rscratch3, rscratch
		@  if r2 > 9
		cmp 	rscratch3, #0x09000000
		@  then r2 -= 10@ 
		addhi 	rscratch3, rscratch3, #0x0a000000
		@  then setcarry()
		bichi 	rstatus, rstatus, #mask_carry @  1 : or mask 00000100000 : set c to one
		@  else clearcarry()
		orrls 	rstatus, rstatus, #mask_carry @  0 : and mask 11111011111 : set c to zero
		@  gather rscratch3 and rscratch2 into ans8
		and	rscratch3,rscratch3,#0x0f000000
		and	rscratch2,rscratch2,#0x0f000000		
		@  rscratch3 : 0r2000000
		@  rscratch2 : 0r1000000
		@  -> 0xr2r1000000				
		orr 	rscratch2, rscratch2, rscratch3, lsl #4		
		ldmfd 	r13!,{rscratch}
		@ only last bit
		and 	reg_a,reg_a,#0x80000000
		@  (register.al ^ work8)			
		eors 	rscratch3, reg_a, rscratch
		biceq 	rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		beq 	1112f
		@  (register.al ^ ans8)
		eors 	rscratch3, reg_a, rscratch2
		@  & 0x80 
		tstne	rscratch3,#0x80000000
		biceq rstatus, rstatus, #mask_overflow @  0 : and mask 11111011111 : set v to zero
		orrne rstatus, rstatus, #mask_overflow @  1 : or mask 00000100000 : set v to one 
1112:
		movs reg_a, rscratch2
		update_zn 
		b 1113f
1111:
		s9xgetbytelow
		movs rscratch2,rstatus,lsr #mask_shifter_carry
		sbcs reg_a, reg_a, rscratch, lsl #24 
		@ overflow 
		orrvs rstatus, rstatus, #mask_overflow
		bicvc rstatus, rstatus, #mask_overflow 
		@ carry
		update_c 
		@ update flag
		ands reg_a, reg_a, #0xff000000
		update_zn
1113:
.endm 

.macro		sta16
		s9xsetword	reg_a
.endm
.macro		sta8
		s9xsetbyte	reg_a
.endm
.macro		stx16
		s9xsetword	reg_x
.endm
.macro		stx8
		s9xsetbyte	reg_x
.endm
.macro		sty16
		s9xsetword	reg_y
.endm
.macro		sty8
		s9xsetbyte	reg_y
.endm
.macro		stz16
		s9xsetwordzero
.endm
.macro		stz8		
		s9xsetbytezero
.endm
.macro		tsb16			
		s9xgetwordregns	rscratch2
		tst		reg_a, rscratch2
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one		
		orr		rscratch2, reg_a, rscratch2		
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		tsb8				
		s9xgetbyteregns	rscratch2
		tst		reg_a, rscratch2
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
		orr		rscratch2, reg_a, rscratch2				
		s9xsetbyte 	rscratch2
		add1cycle
.endm
.macro		trb16		
		s9xgetwordregns	rscratch2
		tst		reg_a, rscratch2
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
		mvn		rscratch3, reg_a
		and		rscratch2, rscratch3, rscratch2
		s9xsetword 	rscratch2
		add1cycle
.endm
.macro		trb8				
		s9xgetbyteregns	rscratch2
		tst		reg_a, rscratch2
		bicne		rstatus, rstatus, #mask_zero	 @  0 : and mask	11111011111 : set z to zero
		orreq		rstatus, rstatus, #mask_zero	 @  1 : or  mask	00000100000  : set z to	one
		mvn		rscratch3, reg_a
		and		rscratch2, rscratch3, rscratch2		
		s9xsetbyte 	rscratch2
		add1cycle
.endm
/**************************************************************************/


/**************************************************************************/

.macro		op09m0		/*ora*/
		ldrb		rscratch2, [rpc,#1]
		ldrb		rscratch, [rpc], #2
		orr		rscratch2,rscratch,rscratch2,lsl #8
		orrs		reg_a,reg_a,rscratch2,lsl #16
		update_zn
		add2mem
.endm
.macro		op09m1		/*ora*/
		ldrb		rscratch, [rpc], #1
		orrs		reg_a,reg_a,rscratch,lsl #24
		update_zn
		add1mem
.endm
/***********************************************************************/
.macro		op90 	/*bcc*/
		asmrelative		
		branchcheck0
		tst		rstatus, #mask_carry
		bne		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress +pcbase
                add1cycle
                cpushutdown
1111:
.endm
.macro		opb0	/*bcs*/
		asmrelative		
		branchcheck0
		tst		rstatus, #mask_carry
		beq		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress +pcbase
                add1cycle
                cpushutdown
1111:
.endm
.macro		opf0 	/*beq*/
		asmrelative		
		branchcheck2
		tst		rstatus, #mask_zero
		beq		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress +pcbase
                add1cycle
                cpushutdown
1111:
.endm
.macro		opd0	/*bne*/
		asmrelative		
		branchcheck1
		tst		rstatus, #mask_zero
		bne		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress +pcbase
                add1cycle
                cpushutdown
1111:
.endm
.macro		op30	/*bmi*/
		asmrelative		
		branchcheck0
		tst		rstatus, #mask_neg
		beq		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress +pcbase
                add1cycle
                cpushutdown
1111:
.endm
.macro		op10   /*bpl*/
		asmrelative
		branchcheck1
		tst 		rstatus, #mask_neg @  neg, z!=0, ne
		bne		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress + pcbase
                add1cycle
                cpushutdown
1111:                
.endm
.macro		op50   /*bvc*/
		asmrelative
		branchcheck0
		tst 		rstatus, #mask_overflow @  neg, z!=0, ne
		bne		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress + pcbase
                add1cycle
                cpushutdown
1111:                
.endm
.macro		op70   /*bvs*/
		asmrelative
		branchcheck0
		tst 		rstatus, #mask_overflow @  neg, z!=0, ne
		beq		1111f
                add 		rpc, rscratch, regpcbase @  rpc = opaddress + pcbase
                add1cycle
                cpushutdown
1111:                
.endm
.macro		op80   /*bra*/
		asmrelative				
                add 		rpc, rscratch, regpcbase @  rpc = opaddress + pcbase
                add1cycle
                cpushutdown
1111:                
.endm
/*******************************************************************************************/
/************************************************************/
/* setflag instructions ********************************************************************** */
.macro		op38 /*sec*/		
		orr		rstatus, rstatus, #mask_carry      @ 	1 : or	mask 00000100000 : set c to one
		add1cycle
.endm
.macro		opf8 /*sed*/		
		setdecimal
		add1cycle		
.endm
.macro		op78 /*sei*/
		setirq
		add1cycle
.endm


/****************************************************************************************/
/* clearflag instructions ******************************************************************** */		
.macro		op18  /*clc*/		
		bic 		rstatus, rstatus, #mask_carry
		add1cycle
.endm
.macro		opd8 /*cld*/		
		cleardecimal
		add1cycle
.endm
.macro		op58  /*cli*/		
		clearirq
		add1cycle		
		@ check_for_irq
.endm
.macro		opb8 /*clv*/		
		bic 		rstatus, rstatus, #mask_overflow
		add1cycle     
.endm

/******************************************************************************************/
/* dex/dey *********************************************************************************** */

.macro		opcax1  /*dex*/
		mov		rscratch3,#0
		subs 		reg_x, reg_x, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		opcax0  /*dex*/		
		mov		rscratch3,#0
		subs 		reg_x, reg_x, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		op88x1 /*dey*/
		mov		rscratch3,#0
		subs 		reg_y, reg_y, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		op88x0 /*dey*/
		mov		rscratch3,#0
		subs 		reg_y, reg_y, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm

/******************************************************************************************/
/* inx/iny *********************************************************************************** */		
.macro		ope8x1
		mov		rscratch3,#0
		adds 		reg_x, reg_x, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		ope8x0
		mov		rscratch3,#0
		adds 		reg_x, reg_x, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		opc8x1
		mov		rscratch3,#0
		adds 		reg_y, reg_y, #0x01000000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm
.macro		opc8x0		
		mov		rscratch3,#0
		adds 		reg_y, reg_y, #0x00010000
		str		rscratch3,[reg_cpu_var,#waitaddress_ofs]
		update_zn
		add1cycle
.endm

/**********************************************************************************************/

/* nop *************************************************************************************** */		
.macro		opea		
		add1cycle
.endm

/**************************************************************************/
/* push instructions **************************************************** */
.macro		opf4
		absolute		
		pushwrlow
.endm
.macro		opd4
		directindirect		
		pushwrlow
.endm
.macro		op62
		asmrelativelong
		pushwrlow
.endm
.macro		op48m0		
		pushw 		reg_a
		add1cycle
.endm
.macro		op48m1		
		pushb 		reg_a
		add1cycle
.endm
.macro		op8b
		and		rscratch2, reg_d_bank, #0xff
		pushblow 	rscratch2
		add1cycle
.endm
.macro		op0b
		pushw	 	reg_d
		add1cycle
.endm
.macro		op4b
		pushblow	reg_p_bank
		add1cycle
.endm
.macro		op08		
		pushb	 	rstatus
		add1cycle
.endm
.macro		opdax1
		pushb 		reg_x
		add1cycle
.endm
.macro		opdax0
		pushw 		reg_x
		add1cycle
.endm
.macro		op5ax1		
		pushb 		reg_y
		add1cycle
.endm
.macro		op5ax0
		pushw 		reg_y
		add1cycle
.endm
/**************************************************************************/
/* pull instructions **************************************************** */
.macro		op68m1
		pullbs		reg_a
		update_zn
		add2cycle
.endm
.macro		op68m0
		pullws		reg_a
		update_zn
		add2cycle
.endm
.macro		opab
		bic		reg_d_bank,reg_d_bank, #0xff
		pullbrs 	
		orr		reg_d_bank,reg_d_bank,rscratch, lsr #24
		update_zn
		add2cycle
.endm
.macro		op2b		
		bic		reg_d,reg_d, #0xff000000
		bic		reg_d,reg_d, #0x00ff0000
		pullwrs		
		orr		reg_d,rscratch,reg_d
		update_zn
		add2cycle
.endm
.macro		op28x1m1	/*plp*/
		@ index set, memory set
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		tst 		rstatus, #mask_index		
		@ index clear & was set : 8->16
		moveq		reg_x,reg_x,lsr #8
		moveq		reg_y,reg_y,lsr #8		
		tst 		rstatus, #mask_mem		
		@ memory cleared & was set : 8->16
		ldreqb		rscratch,[reg_cpu_var,#rah_ofs]		
		moveq		reg_a,reg_a,lsr #8
		orreq		reg_a,reg_a,rscratch, lsl #24
		s9xfixcycles
		add2cycle
.endm
.macro		op28x0m1	/*plp*/		
		@ index cleared, memory set
		bic		rstatus,rstatus,#0xff000000				
		pullbr		
		orr		rstatus,rscratch,rstatus
		tst 		rstatus, #mask_index
		@ index set & was cleared : 16->8
		movne		reg_x,reg_x,lsl #8
		movne		reg_y,reg_y,lsl #8
		tst 		rstatus, #mask_mem
		@ memory cleared & was set : 8->16
		ldreqb		rscratch,[reg_cpu_var,#rah_ofs]
		moveq		reg_a,reg_a,lsr #8
		orreq		reg_a,reg_a,rscratch, lsl #24
		s9xfixcycles
		add2cycle
.endm
.macro		op28x1m0	/*plp*/
		@ index set, memory set		
		bic		rstatus,rstatus,#0xff000000				
		pullbr		
		orr		rstatus,rscratch,rstatus
		tst 		rstatus, #mask_index
		@ index clear & was set : 8->16
		moveq		reg_x,reg_x,lsr #8
		moveq		reg_y,reg_y,lsr #8		
		tst 		rstatus, #mask_mem
		@ memory set & was cleared : 16->8				
		movne		rscratch,reg_a,lsr #24
		movne		reg_a,reg_a,lsl #8
		strneb		rscratch,[reg_cpu_var,#rah_ofs]
		s9xfixcycles
		add2cycle
.endm
.macro		op28x0m0	/*plp*/
		@ index set, memory set
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		tst 		rstatus, #mask_index
		@ index set & was cleared : 16->8
		movne		reg_x,reg_x,lsl #8
		movne		reg_y,reg_y,lsl #8
		tst 		rstatus, #mask_mem
		@ memory set & was cleared : 16->8				
		movne		rscratch,reg_a,lsr #24
		movne		reg_a,reg_a,lsl #8
		strneb		rscratch,[reg_cpu_var,#rah_ofs]
		s9xfixcycles
		add2cycle
.endm
.macro		opfax1
		pullbs 		reg_x
		update_zn
		add2cycle
.endm
.macro		opfax0	
		pullws 		reg_x
		update_zn
		add2cycle
.endm
.macro		op7ax1
		pullbs		reg_y
		update_zn
		add2cycle
.endm
.macro		op7ax0		
		pullws 		reg_y
		update_zn
		add2cycle
.endm		

/**********************************************************************************************/
/* transfer instructions ********************************************************************* */
.macro		opaax1m1 /*tax8*/		
		movs 		reg_x, reg_a
		update_zn
		add1cycle
.endm
.macro		opaax0m1 /*tax16*/		
		ldrb 		reg_x, [reg_cpu_var,#rah_ofs]
		mov		reg_x, reg_x,lsl #24
		orrs		reg_x, reg_x,reg_a, lsr #8		
		update_zn
		add1cycle
.endm
.macro		opaax1m0 /*tax8*/		
		movs 		reg_x, reg_a, lsl #8
		update_zn
		add1cycle
.endm
.macro		opaax0m0 /*tax16*/		
		movs 		reg_x, reg_a
		update_zn
		add1cycle
.endm
.macro		opa8x1m1 /*tay8*/		
		movs 		reg_y, reg_a
		update_zn
		add1cycle
.endm
.macro		opa8x0m1 /*tay16*/
		ldrb 		reg_y, [reg_cpu_var,#rah_ofs]
		mov		reg_y, reg_y,lsl #24
		orrs		reg_y, reg_y,reg_a, lsr #8		
		update_zn
		add1cycle
.endm
.macro		opa8x1m0 /*tay8*/		
		movs 		reg_y, reg_a, lsl #8
		update_zn
		add1cycle
.endm
.macro		opa8x0m0 /*tay16*/
		movs 		reg_y, reg_a
		update_zn
		add1cycle
.endm
.macro		op5bm1		
		ldrb		rscratch, [reg_cpu_var,#rah_ofs]
		mov		reg_d,reg_d,lsl #16
		mov		rscratch,rscratch,lsl #24
		orrs		rscratch,rscratch,reg_a, lsr #8		
		update_zn
		orr  		reg_d,rscratch,reg_d,lsr #16
		add1cycle
.endm
.macro		op5bm0		
		mov		reg_d,reg_d,lsl #16		
		movs		reg_a,reg_a
		update_zn
		orr  		reg_d,reg_a,reg_d,lsr #16
		add1cycle
.endm
.macro		op1bm1
		tst 		rstatus, #mask_emul
		movne		reg_s, reg_a, lsr #24
		orrne		reg_s, reg_s, #0x100		
		ldreqb		reg_s, [reg_cpu_var,#rah_ofs]
		orreq		reg_s, reg_s, reg_a
		moveq		reg_s, reg_s, ror #24
		add1cycle
.endm
.macro		op1bm0		
		mov 		reg_s, reg_a, lsr #16
		add1cycle
.endm
.macro		op7bm1		
		movs 		reg_a, reg_d, asr #16		
		update_zn
		mov		rscratch,reg_a,lsr #8		
		mov		reg_a,reg_a, lsl #24
		strb		rscratch, [reg_cpu_var,#rah_ofs]
		add1cycle
.endm
.macro		op7bm0
		movs 		reg_a, reg_d, asr #16		
		update_zn
		mov		reg_a,reg_a, lsl #16
		add1cycle
.endm
.macro		op3bm1
		mov		rscratch,reg_s, lsr #8
		movs		reg_a, reg_s, lsl #16
		strb		rscratch, [reg_cpu_var,#rah_ofs]
		update_zn
		mov		reg_a,reg_a, lsl #8
		add1cycle
.endm
.macro		op3bm0
		movs		reg_a, reg_s, lsl #16
		update_zn
		add1cycle
.endm
.macro		opbax1
		movs 		reg_x, reg_s, lsl #24
		update_zn
		add1cycle
.endm
.macro		opbax0
		movs 		reg_x, reg_s, lsl #16
		update_zn
		add1cycle
.endm		
.macro		op8am1x1
		movs 		reg_a, reg_x
		update_zn
		add1cycle
.endm
.macro		op8am1x0
		movs 		reg_a, reg_x, lsl #8
		update_zn
		add1cycle
.endm
.macro		op8am0x1
		movs 		reg_a, reg_x, lsr #8
		update_zn
		add1cycle
.endm
.macro		op8am0x0
		movs 		reg_a, reg_x
		update_zn
		add1cycle
.endm
.macro		op9ax1		
		mov 		reg_s, reg_x, lsr #24
		tst 		rstatus, #mask_emul		
		orrne		reg_s, reg_s, #0x100
		add1cycle
.endm
.macro		op9ax0		
		mov 		reg_s, reg_x, lsr #16
		add1cycle
.endm
.macro		op9bx1		
		movs 		reg_y, reg_x
		update_zn
		add1cycle
.endm
.macro		op9bx0		
		movs 		reg_y, reg_x
		update_zn
		add1cycle
.endm
.macro		op98m1x1	
		movs 		reg_a, reg_y
		update_zn
		add1cycle
.endm
.macro		op98m1x0
		movs 		reg_a, reg_y, lsl #8
		update_zn
		add1cycle
.endm
.macro		op98m0x1
		movs 		reg_a, reg_y, lsr #8
		update_zn
		add1cycle
.endm
.macro		op98m0x0
		movs 		reg_a, reg_y
		update_zn
		add1cycle
.endm
.macro		opbbx1		
		movs 		reg_x, reg_y
		update_zn
		add1cycle
.endm
.macro		opbbx0
		movs 		reg_x, reg_y
		update_zn
		add1cycle
.endm

/**********************************************************************************************/
/* xce *************************************************************************************** */

.macro		opfb
    tst		rstatus,#mask_carry
    beq		1111f
    @ carry is set
    tst		rstatus,#mask_emul    
    bne		1112f
    @ emul is cleared
    bic		rstatus,rstatus,#mask_carry
    tst		rstatus,#mask_index
    @ x & y were 16bits before
    moveq	reg_x,reg_x,lsl #8
    moveq	reg_y,reg_y,lsl #8
    tst		rstatus,#mask_mem
    @ a was 16bits before
    @ save ah
    moveq	rscratch,reg_a,lsr #24
    streqb	rscratch,[reg_cpu_var,#rah_ofs]
    moveq	reg_a,reg_a,lsl #8
    orr		rstatus,rstatus,#mask_emul_orr_mask_mem_orr_mask_index
    and		reg_s,reg_s,#0xff
    orr		reg_s,reg_s,#0x100    
    b		1113f    
1112:    
    @ emul is set
    tst		rstatus,#mask_index
    @ x & y were 16bits before
    moveq	reg_x,reg_x,lsl #8
    moveq	reg_y,reg_y,lsl #8
    tst		rstatus,#mask_mem
    @ a was 16bits before
    @ save ah
    moveq	rscratch,reg_a,lsr #24
    streqb	rscratch,[reg_cpu_var,#rah_ofs]
    moveq	reg_a,reg_a,lsl #8
    orr		rstatus,rstatus,#mask_carry_orr_mask_mem_orr_mask_index
    and		reg_s,reg_s,#0xff
    orr		reg_s,reg_s,#0x100    
    b		1113f
1111:    
    @ carry is cleared
    tst		rstatus,#mask_emul
    beq		1115f
    @ emul was set : x,y & a were 8bits
    @ now have to check memory & index for potential conversions to 16bits
    tst		rstatus,#mask_index
    @  x & y are now 16bits
    moveq	reg_x,reg_x,lsr #8	
    moveq	reg_y,reg_y,lsr #8	
    tst		rstatus,#mask_mem
    @  a is now 16bits
    moveq	reg_a,reg_a,lsr #8	
    @ restore ah
    ldreqb	rscratch,[reg_cpu_var,#rah_ofs]    
    orreq	reg_a,reg_a,rscratch,lsl #24
1115:    
    bic		rstatus,rstatus,#mask_emul
    orr		rstatus,rstatus,#mask_carry
1113:
    add1cycle
    s9xfixcycles
.endm

/*******************************************************************************/
/* brk *************************************************************************/
.macro		op00		/*brk*/
		mov		rscratch,#1
		strb		rscratch,[reg_cpu_var,#brktriggered_ofs]
		
		tst 		rstatus, #mask_emul
		@  eq is flag to zero (!checkemu)
		bne 		2001f@ elseop00
		pushblow	reg_p_bank
		sub 		rscratch, rpc, regpcbase
		add 		rscratch2, rscratch, #1
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank, reg_p_bank, #0xff
		mov 		rscratch, #0xe6
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add2cycle
		b 		2002f@ endop00
2001:@ elseop00
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic		reg_p_bank,reg_p_bank, #0xff
		mov 		rscratch, #0xfe
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add1cycle
2002:@ endop00
.endm


/**********************************************************************************************/
/* brl ************************************************************************************** */
.macro		op82	/*brl*/
		asmrelativelong
		orr		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase
.endm		
/**********************************************************************************************/
/* irq *************************************************************************************** */			
@ void s9xopcode_irq (void)		
.macro		s9xopcode_irq	@ irq
		tst 		rstatus, #mask_emul
		@  eq is flag to zero (!checkemu)
		bne 		2121f@ elseop02
		pushblow 	reg_p_bank
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank, reg_p_bank,#0xff
		mov 		rscratch, #0xee
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add2cycle
		b 2122f
2121:@ else
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank,reg_p_bank, #0xff
		mov 		rscratch, #0xfe
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add1cycle
2122:
.endm

/*
void asm_s9xopcode_irq(void)
{
    if (!checkemulation())
    {
        pushb (registers.pb);
        pushw (cpu.pc - cpu.pcbase);
        pushb (registers.pl);
        cleardecimal ();
        setirq ();

        registers.pb = 0;
		s9xsetpcbase (s9xgetword (0xffee));
        cpu.cycles += two_cycles;
    }
    else
    {
        pushw (cpu.pc - cpu.pcbase);
        pushb (registers.pl);
        cleardecimal ();
        setirq ();

        registers.pb = 0;
        s9xsetpcbase (s9xgetword (0xfffe));
        cpu.cycles += one_cycle;
    }
}
*/	
		
/**********************************************************************************************/
/* nmi *************************************************************************************** */		
@ void s9xopcode_nmi (void)
.macro		s9xopcode_nmi	@ nmi
		tst 		rstatus, #mask_emul
		@  eq is flag to zero (!checkemu)
		bne 		2123f@ elseop02
		pushblow 	reg_p_bank
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank, reg_p_bank,#0xff
		mov 		rscratch, #0xea
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add2cycle
		b 2124f
2123:@ else
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank,reg_p_bank, #0xff
		mov 		rscratch, #0xfa
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add1cycle
2124:
.endm
/*
void asm_s9xopcode_nmi(void)
{	
	if (!checkemulation())
    {
        pushb (registers.pb);
        pushw (cpu.pc - cpu.pcbase);
        pushb (registers.pl);
        cleardecimal ();
        setirq ();

        registers.pb = 0;
        s9xsetpcbase (s9xgetword (0xffea));
        cpu.cycles += two_cycles;
    }
    else
    {
        pushw (cpu.pc - cpu.pcbase);
        pushb (registers.pl);
        cleardecimal ();
        setirq ();

        registers.pb = 0;
        s9xsetpcbase (s9xgetword (0xfffa));
        cpu.cycles += one_cycle;
    }    
}
*/

/**********************************************************************************************/
/* cop *************************************************************************************** */
.macro		op02		/*cop*/
		tst 		rstatus, #mask_emul
		@  eq is flag to zero (!checkemu)
		bne 		2021f@ elseop02
		pushblow 	reg_p_bank
		sub 		rscratch, rpc, regpcbase
		add 		rscratch2, rscratch, #1
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank, reg_p_bank,#0xff
		mov 		rscratch, #0xe4
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add2cycle
		b 2022f@ endop02
2021:@ elseop02
		sub 		rscratch2, rpc, regpcbase
		pushwlow 	rscratch2
		@  packstatus
		pushb	 	rstatus
		cleardecimal
		setirq
		bic 		reg_p_bank,reg_p_bank, #0xff
		mov 		rscratch, #0xf4
		orr 		rscratch, rscratch, #0xff00
		s9xgetwordlow 		
		s9xsetpcbase 	
		add1cycle
2022:@ endop02
.endm

/**********************************************************************************************/
/* jml *************************************************************************************** */
.macro		opdc		
		absoluteindirectlong		
		bic		reg_p_bank,reg_p_bank,#0xff
		orr 		reg_p_bank,reg_p_bank, rscratch, lsr #16
		s9xsetpcbase 	
		add2cycle
.endm
.macro		op5c		
		absolutelong		
		bic		reg_p_bank,reg_p_bank,#0xff
		orr 		reg_p_bank,reg_p_bank, rscratch, lsr #16
		s9xsetpcbase 	
.endm

/**********************************************************************************************/
/* jmp *************************************************************************************** */
.macro		op4c
		absolute
		bic		rscratch, rscratch, #0xff0000
		orr		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase
		cpushutdown
.endm		
.macro		op6c
		absoluteindirect
		bic		rscratch, rscratch, #0xff0000
		orr		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase		
.endm		
.macro		op7c						
		add 		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase 	
		add1cycle
.endm

/**********************************************************************************************/
/* jsl/rtl *********************************************************************************** */
.macro		op22				
		pushblow	reg_p_bank
		sub 		rscratch, rpc, regpcbase
		@ sub 		rscratch2, rscratch2, #1
		add 		rscratch2, rscratch, #2
		pushwlow	rscratch2
		absolutelong		
		bic		reg_p_bank,reg_p_bank,#0xff
		orr 		reg_p_bank, reg_p_bank, rscratch, lsr #16
		s9xsetpcbase 	
.endm
.macro		op6b		
		pullwlow 	rpc		
		bic		reg_p_bank,reg_p_bank,#0xff
		pullbrlow 			
		orr 		reg_p_bank, reg_p_bank, rscratch
		add 		rscratch, rpc, #1
		bic		rscratch, rscratch,#0xff0000
		orr		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase
		add2cycle
.endm
/**********************************************************************************************/
/* jsr/rts *********************************************************************************** */
.macro		op20				
		sub 		rscratch, rpc, regpcbase
		@ sub 		rscratch2, rscratch2, #1
		add 		rscratch2, rscratch, #1		
		pushwlow	rscratch2				
		absolute		
		bic		rscratch, rscratch, #0xff0000		
		orr 		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase 
		add1cycle
.endm
.macro		opfcx0
		sub 		rscratch, rpc, regpcbase
		@ sub 		rscratch2, rscratch2, #1
		add		rscratch2, rscratch, #1
		pushwlow	rscratch2
		absoluteindexedindirectx0
		orr 		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase
		add1cycle
.endm
.macro		opfcx1
		sub 		rscratch, rpc, regpcbase
		@ sub 		rscratch2, rscratch2, #1
		add		rscratch2, rscratch, #1		
		pushwlow	rscratch2	
		absoluteindexedindirectx1
		orr 		rscratch, rscratch, reg_p_bank, lsl #16
		s9xsetpcbase 
		add1cycle
.endm
.macro		op60			
		pullwlow 	rpc
		add 		rscratch, rpc, #1		
		bic		rscratch, rscratch,#0x10000		
		orr		rscratch, rscratch, reg_p_bank, lsl #16		
		s9xsetpcbase 
		add3cycle
.endm

/**********************************************************************************************/
/* mvn/mvp *********************************************************************************** */		
.macro		op54x1m1
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #24		
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #24
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2	
		@ load 16bits a		
		ldrb		rscratch,[reg_cpu_var,#rah_ofs]
		mov		reg_a,reg_a,lsr #8
		orr		reg_a,reg_a,rscratch, lsl #24
		add		reg_x, reg_x, #0x01000000
		sub		reg_a, reg_a, #0x00010000
		add		reg_y, reg_y, #0x01000000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                @ update ah
                mov		rscratch, reg_a, lsr #24
                mov		reg_a,reg_a,lsl #8
                strb		rscratch,[reg_cpu_var,#rah_ofs]                
                add2cycle2mem
.endm
.macro		op54x1m0
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #24		
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #24
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2		
		add		reg_x, reg_x, #0x01000000
		sub		reg_a, reg_a, #0x00010000
		add		reg_y, reg_y, #0x01000000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                add2cycle2mem
.endm
.macro		op54x0m1
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #16
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #16
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2		
		@ load 16bits a		
		ldrb		rscratch,[reg_cpu_var,#rah_ofs]
		mov		reg_a,reg_a,lsr #8
		orr		reg_a,reg_a,rscratch, lsl #24
		add		reg_x, reg_x, #0x00010000
		sub		reg_a, reg_a, #0x00010000
		add		reg_y, reg_y, #0x00010000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3                
                @ update ah
                mov		rscratch, reg_a, lsr #24
                mov		reg_a,reg_a,lsl #8
                strb		rscratch,[reg_cpu_var,#rah_ofs]                
                add2cycle2mem
.endm
.macro		op54x0m0
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #16
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #16
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2		
		add		reg_x, reg_x, #0x00010000
		sub		reg_a, reg_a, #0x00010000
		add		reg_y, reg_y, #0x00010000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                add2cycle2mem
.endm

.macro		op44x1m1
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #24		
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #24
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2
		@ load 16bits a		
		ldrb		rscratch,[reg_cpu_var,#rah_ofs]
		mov		reg_a,reg_a,lsr #8
		orr		reg_a,reg_a,rscratch, lsl #24
		sub		reg_x, reg_x, #0x01000000
		sub		reg_a, reg_a, #0x00010000
		sub		reg_y, reg_y, #0x01000000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                @ update ah
                mov		rscratch, reg_a, lsr #24
                mov		reg_a,reg_a,lsl #8
                strb		rscratch,[reg_cpu_var,#rah_ofs]                
                add2cycle2mem
.endm
.macro		op44x1m0
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #24		
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #24
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2		
		sub		reg_x, reg_x, #0x01000000
		sub		reg_a, reg_a, #0x00010000
		sub		reg_y, reg_y, #0x01000000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                add2cycle2mem
.endm
.macro		op44x0m1
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #16
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #16
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2
		@ load 16bits a		
		ldrb		rscratch,[reg_cpu_var,#rah_ofs]
		mov		reg_a,reg_a,lsr #8
		orr		reg_a,reg_a,rscratch, lsl #24
		sub		reg_x, reg_x, #0x00010000
		sub		reg_a, reg_a, #0x00010000
		sub		reg_y, reg_y, #0x00010000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                @ update ah
                mov		rscratch, reg_a, lsr #24
                mov		reg_a,reg_a,lsl #8
                strb		rscratch,[reg_cpu_var,#rah_ofs]                
                add2cycle2mem
.endm
.macro		op44x0m0
		@ save regstatus = reg_d_bank >> 24
		mov		rscratch, reg_d_bank, lsr #16
                ldrb		reg_d_bank    , [rpc], #1
		ldrb		rscratch2    , [rpc], #1
		@ restore regstatus = reg_d_bank >> 24
		orr		reg_d_bank, reg_d_bank, rscratch, lsl #16
		mov		rscratch    , reg_x, lsr #16
                orr		rscratch    , rscratch, rscratch2, lsl #16                
		s9xgetbytelow 
		mov		rscratch2, rscratch
		mov		rscratch   , reg_y, lsr #16
		orr		rscratch   , rscratch, reg_d_bank, lsl #16		
		s9xsetbytelow 	rscratch2		
		sub		reg_x, reg_x, #0x00010000
		sub		reg_a, reg_a, #0x00010000
		sub		reg_y, reg_y, #0x00010000				
                cmp		reg_a, #0xffff0000
                subne		rpc, rpc, #3
                add2cycle2mem
.endm

/**********************************************************************************************/
/* rep/sep *********************************************************************************** */
.macro		opc2
		@  status&=~(*rpc++);
		@  so possible changes are :		
		@  index = 1 -> 0  : x,y 8bits -> 16bits
		@  mem = 1 -> 0 : a 8bits -> 16bits
		@ save old status for mask_index & mask_mem comparison
		mov		rscratch3, rstatus
		ldrb		rscratch, [rpc], #1
		mvn		rscratch, rscratch		
		and		rstatus,rstatus,rscratch, ror #8 @ 32 - status_shifter
		tst		rstatus,#mask_emul
		beq		1111f
		@ emulation mode on : no changes since it was on before opcode
		@ just be sure to reset mem & index accordingly
		orr		rstatus,rstatus,#mask_mem_orr_mask_index		
		b		1112f
1111:		
		@ not in emulation mode, check index & memory bits
		@ now check index
		tst		rscratch3,#mask_index
		beq		1113f		
		@  x & y were 8bit before
		tst		rstatus,#mask_index
		bne		1113f
		@  x & y are now 16bits
		mov		reg_x,reg_x,lsr #8
		mov		reg_y,reg_y,lsr #8
1113:		@ x & y still in 16bits
		@ now check memory
		tst		rscratch3,#mask_mem
		beq		1112f		
		@  a was 8bit before
		tst		rstatus,#mask_mem
		bne		1112f
		@  a is now 16bits
		mov		reg_a,reg_a,lsr #8		
		@ restore ah
    		ldreqb		rscratch,[reg_cpu_var,#rah_ofs]    		
    		orreq		reg_a,reg_a,rscratch,lsl #24
1112:
		s9xfixcycles
		add1cycle1mem
.endm
.macro		ope2
		@  status|=*rpc++;
		@  so possible changes are :
		@  index = 0 -> 1  : x,y 16bits -> 8bits
		@  mem = 0 -> 1 : a 16bits -> 8bits
		@ save old status for mask_index & mask_mem comparison
		mov		rscratch3, rstatus
		ldrb		rscratch, [rpc], #1		
		orr		rstatus,rstatus,rscratch, lsl #status_shifter
		tst		rstatus,#mask_emul
		beq		10111f
		@ emulation mode on : no changes sinc eit was on before opcode
		@ just be sure to have mem & index set accordingly
		orr		rstatus,rstatus,#mask_mem_orr_mask_index		
		b		10112f
10111:		
		@ not in emulation mode, check index & memory bits
		@ now check index
		tst		rscratch3,#mask_index
		bne		10113f		
		@  x & y were 16bit before
		tst		rstatus,#mask_index
		beq		10113f
		@  x & y are now 8bits
		mov		reg_x,reg_x,lsl #8
		mov		reg_y,reg_y,lsl #8
10113:		@ x & y still in 16bits
		@ now check memory
		tst		rscratch3,#mask_mem
		bne		10112f		
		@  a was 16bit before
		tst		rstatus,#mask_mem
		beq		10112f
		@  a is now 8bits
		@  save ah
		mov		rscratch,reg_a,lsr #24
		mov		reg_a,reg_a,lsl #8	
		strb		rscratch,[reg_cpu_var,#rah_ofs]	
10112:
		s9xfixcycles
		add1cycle1mem
.endm

/**********************************************************************************************/
/* xba *************************************************************************************** */
.macro		opebm1		
		@ a is 8bits
		add		rscratch,reg_cpu_var,#rah_ofs
		mov		reg_a,reg_a, lsr #24
		swpb		reg_a,reg_a,[rscratch]
		movs		reg_a,reg_a, lsl #24
		update_zn
		add2cycle
.endm
.macro		opebm0		
		@ a is 16bits
		mov 		rscratch, reg_a, ror #24 @  ll0000hh
		orr 		rscratch, rscratch, reg_a, lsr #8@  ll0000hh + 00hhll00 -> llhhllhh
		mov 		reg_a, rscratch, lsl #16@  llhhllhh -> llhh0000		
		movs		rscratch,rscratch,lsl #24 @ to set z & n flags with al		
		update_zn
		add2cycle
.endm


/**********************************************************************************************/
/* rti *************************************************************************************** */
.macro		op40x1m1
		@ index set, memory set		
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		pullwlow	rpc
		tst 		rstatus, #mask_emul
		orrne		rstatus, rstatus, #mask_mem_orr_mask_index
                bne		2401f
		pullbrlow
		bic		reg_p_bank,reg_p_bank,#0xff
		orr		reg_p_bank,reg_p_bank,rscratch
2401:		
		add 		rscratch, rpc, reg_p_bank, lsl #16
		s9xsetpcbase
		tst 		rstatus, #mask_index		
		@ index cleared & was set : 8->16
		moveq		reg_x,reg_x,lsr #8
		moveq		reg_y,reg_y,lsr #8
		tst 		rstatus, #mask_mem		
		@ memory cleared & was set : 8->16
		ldreqb		rscratch,[reg_cpu_var,#rah_ofs]		
		moveq		reg_a,reg_a,lsr #8		
		orreq		reg_a,reg_a,rscratch, lsl #24		
		add2cycle
		s9xfixcycles
.endm
.macro		op40x0m1
		@ index cleared, memory set		
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		pullwlow	rpc
		tst 		rstatus, #mask_emul
		orrne		rstatus, rstatus, #mask_mem_orr_mask_index
                bne		2401f
		pullbrlow
		bic		reg_p_bank,reg_p_bank,#0xff
		orr		reg_p_bank,reg_p_bank,rscratch
2401:		
		add 		rscratch, rpc, reg_p_bank, lsl #16
		s9xsetpcbase		
		tst 		rstatus, #mask_index		
		@ index set & was cleared : 16->8
		movne		reg_x,reg_x,lsl #8
		movne		reg_y,reg_y,lsl #8		
		tst 		rstatus, #mask_mem		
		@ memory cleared & was set : 8->16
		ldreqb		rscratch,[reg_cpu_var,#rah_ofs]		
		moveq		reg_a,reg_a,lsr #8		
		orreq		reg_a,reg_a,rscratch, lsl #24
		add2cycle
		s9xfixcycles
.endm
.macro		op40x1m0
		@ index set, memory cleared
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		pullwlow	rpc
		tst 		rstatus, #mask_emul
		orrne		rstatus, rstatus, #mask_mem_orr_mask_index
                bne		2401f
		pullbrlow
		bic		reg_p_bank,reg_p_bank,#0xff
		orr		reg_p_bank,reg_p_bank,rscratch
2401:		
		add 		rscratch, rpc, reg_p_bank, lsl #16
		s9xsetpcbase
		tst 		rstatus, #mask_index		
		@ index cleared & was set : 8->16
		moveq		reg_x,reg_x,lsr #8
		moveq		reg_y,reg_y,lsr #8		
		tst 		rstatus, #mask_mem		
		@ memory set & was cleared : 16->8
		movne		rscratch,reg_a,lsr #24
		movne		reg_a,reg_a,lsl #8
		strneb		rscratch,[reg_cpu_var,#rah_ofs]
		add2cycle
		s9xfixcycles
.endm
.macro		op40x0m0
		@ index cleared, memory cleared
		bic		rstatus,rstatus,#0xff000000
		pullbr
		orr		rstatus,rscratch,rstatus
		pullwlow	rpc
		tst 		rstatus, #mask_emul
		orrne		rstatus, rstatus, #mask_mem_orr_mask_index
                bne		2401f
		pullbrlow
		bic		reg_p_bank,reg_p_bank,#0xff
		orr		reg_p_bank,reg_p_bank,rscratch
2401:		
		add 		rscratch, rpc, reg_p_bank, lsl #16
		s9xsetpcbase
		tst 		rstatus, #mask_index
		@ index set & was cleared : 16->8
		movne		reg_x,reg_x,lsl #8
		movne		reg_y,reg_y,lsl #8		
		tst 		rstatus, #mask_mem		
		@ memory set & was cleared : 16->8
		@ memory set & was cleared : 16->8
		movne		rscratch,reg_a,lsr #24
		movne		reg_a,reg_a,lsl #8
		strneb		rscratch,[reg_cpu_var,#rah_ofs]
		add2cycle
		s9xfixcycles
.endm
	

/**********************************************************************************************/
/* stp/wai/db ******************************************************************************** */
@  wai
.macro		opcb	/*wai*/
	ldrb		rscratch,[reg_cpu_var,#irqactive_ofs]
	movs		rscratch,rscratch
	@ (cpu.irqactive)
	add2cyclene
	bne		1234f
/*
	cpu.waitingforinterrupt = true;
	cpu.pc--;*/	
	mov		rscratch,#1
	sub		rpc,rpc,#1
/*		
	    cpu.cycles = cpu.nextevent;	    
*/		
	strb		rscratch,[reg_cpu_var,#waitingforinterrupt_ofs]
	ldr		reg_cycles,[reg_cpu_var,#nextevent_ofs]
/*
	if (iapu.apuexecuting)
	    {
		icpu.cpuexecuting = false;
		do
		{
		    apu_execute1 ();
		} while (apu.cycles < cpu.nextevent);
		icpu.cpuexecuting = true;
	    }	
*/	
	ldrb		rscratch,[reg_cpu_var,#apuexecuting_ofs]
	movs		rscratch,rscratch
	beq		1234f
	asmapu_execute2	

1234:	
.endm
.macro		opdb	/*stp*/    
    		sub	rpc,rpc,#1
    		@ cpu.flags |= debug_mode_flag;
.endm
.macro		op42   /*reserved snes9x*/
.endm	
		
/**********************************************************************************************/
/* and ******************************************************************************** */
.macro		op29m1
		ldrb	rscratch    , [rpc], #1		
		ands	reg_a    , reg_a,	rscratch, lsl #24
		update_zn
		add1mem
.endm		
.macro		op29m0		
		ldrb	rscratch2  , [rpc,#1]
		ldrb	rscratch   , [rpc], #2
		orr	rscratch, rscratch, rscratch2, lsl #8		
		ands	reg_a    , reg_a,	rscratch, lsl #16
		update_zn
		add2mem
.endm

		


		

		

		

		

		

		
/**********************************************************************************************/
/* eor ******************************************************************************** */
.macro		op49m0		
                ldrb	rscratch2 , [rpc, #1]
                ldrb	rscratch , [rpc], #2
		orr	rscratch, rscratch, rscratch2,lsl #8                
		eors    reg_a, reg_a, rscratch,lsl #16
		update_zn
		add2mem
.endm

		
.macro		op49m1		
                ldrb	rscratch , [rpc], #1                
		eors    reg_a, reg_a, rscratch,lsl #24
		update_zn
		add1mem
.endm


/**********************************************************************************************/
/* sta *************************************************************************************** */		
.macro		op81m1				
		sta8
		@ tst 		rstatus, #mask_index
		@ add1cyclene
.endm
.macro		op81m0				
		sta16
		@ tst rstatus, #mask_index
		@ add1cyclene
.endm


/**********************************************************************************************/
/* bit *************************************************************************************** */
.macro		op89m1		
                ldrb	rscratch , [rpc], #1                
		tst     reg_a, rscratch, lsl #24
		update_z
		add1mem
.endm
.macro		op89m0		
                ldrb	rscratch2 , [rpc, #1]
                ldrb	rscratch , [rpc], #2
		orr	rscratch, rscratch, rscratch2, lsl #8                
		tst     reg_a, rscratch, lsl #16
		update_z
		add2mem
.endm

		

		
		

/**********************************************************************************************/
/* ldy *************************************************************************************** */
.macro		opa0x1
                ldrb	rscratch , [rpc], #1                
                movs    reg_y, rscratch, lsl #24
		update_zn
		add1mem
.endm
.macro		opa0x0		
                ldrb	rscratch2 , [rpc, #1]
                ldrb	rscratch , [rpc], #2
		orr	rscratch, rscratch, rscratch2, lsl #8                
                movs    reg_y, rscratch, lsl #16
		update_zn
		add2mem
.endm

/**********************************************************************************************/
/* ldx *************************************************************************************** */		
.macro		opa2x1		
                ldrb	rscratch , [rpc], #1                
                movs    reg_x, rscratch, lsl #24
		update_zn
		add1mem
.endm
.macro		opa2x0		
                ldrb	rscratch2 , [rpc, #1]
                ldrb	rscratch , [rpc], #2
		orr	rscratch, rscratch, rscratch2, lsl #8                
                movs    reg_x, rscratch, lsl #16
		update_zn
		add2mem
.endm
		
/**********************************************************************************************/
/* lda *************************************************************************************** */		
.macro		opa9m1		
                ldrb	rscratch , [rpc], #1
                movs    reg_a, rscratch, lsl #24
		update_zn
		add1mem
.endm
.macro		opa9m0		
                ldrb	rscratch2 , [rpc, #1]
                ldrb	rscratch , [rpc], #2
		orr	rscratch, rscratch, rscratch2, lsl #8                
                movs    reg_a, rscratch, lsl #16                
		update_zn
		add2mem
.endm
												
/**********************************************************************************************/
/* cmy *************************************************************************************** */
.macro		opc0x1
		ldrb	rscratch    , [rpc], #1		
		subs	rscratch2   , reg_y , rscratch, lsl #24
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn		
		add1mem
.endm
.macro		opc0x0
		ldrb	rscratch2   , [rpc, #1]
		ldrb	rscratch   , [rpc], #2		
		orr	rscratch, rscratch, rscratch2, lsl #8
		subs	rscratch2   , reg_y, rscratch, lsl #16
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		add2mem
.endm

		

		

/**********************************************************************************************/
/* cmp *************************************************************************************** */		
.macro		opc9m1		
		ldrb	rscratch    , [rpc], #1		
		subs	rscratch2   , reg_a , rscratch, lsl #24		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		add1mem
.endm
.macro		opc9m0		
		ldrb	rscratch2   , [rpc,#1]
		ldrb	rscratch   , [rpc], #2		
		orr	rscratch, rscratch, rscratch2, lsl #8
		subs	rscratch2   , reg_a, rscratch, lsl #16		
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		add2mem
.endm

/**********************************************************************************************/
/* cmx *************************************************************************************** */		
.macro		ope0x1		
		ldrb	rscratch    , [rpc], #1		
		subs	rscratch2   , reg_x , rscratch, lsl #24
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn		
		add1mem
.endm
.macro		ope0x0		
		ldrb	rscratch2   , [rpc,#1]
		ldrb	rscratch   , [rpc], #2		
		orr	rscratch, rscratch, rscratch2, lsl #8
		subs	rscratch2   , reg_x, rscratch, lsl #16
		biccc	rstatus, rstatus, #mask_carry
		orrcs	rstatus, rstatus, #mask_carry
		update_zn
		add2mem
.endm

/*


cli_ope_rec_nos_layer0 
  	nos.nos_ope_treasury_date = convert(datetime, @treasurydate, 103)
    	nos.nos_ope_accounting_date = convert(datetime, @accountingdate, 103)

cli_ope_nos_ope_layer0
	n.nos_ope_treasury_date = convert(datetime, @lard, 103)
	n.nos_ope_accounting_date = convert(datetime, @lard, 103)
    	
cli_ope_nos_layer0    	
	nos.nos_ope_treasury_date = convert(datetime, @lard, 103)
	nos.nos_ope_accounting_date = convert(datetime, @lard, 103)    	
	
ecrans:
------


[gnv] : utilisation de la lard (laccdate) pour afficher les openings.
   +nécessité d'avoir des valeurs dans l'opening pour date tréso=date compta=laccdate
	
[accounting rec] : si laccdate pas bonne (pas = bd-1) -> message warning et pas de donnée
sinon : 
  +données nécessaires : opening date tréso=date compta=laccdate=bd-1
  +données nécessaires : opening date tréso=date compta=laccdate-1
  +données nécessaires : opening date tréso=laccdate-1 et date compta=laccdate
   */


	
/****************************************************************
	global
****************************************************************/
	.globl   _test_opcode
	.globl	 _asmMainLoop

1:
  .long jumptable1
2:
  .long jumptable2
3:
  .long jumptable3
4:
  .long jumptable4

@ void asmmainloop(asm_cpu_var_t *asmcpuptr);
_asmMainLoop:
	@ save registers
	stmfd		r13!,{r4-r11,lr}
	@ init pointer to cpuvar structure
	mov		reg_cpu_var,r0
	ldr r1, 1b
	str r1, [reg_cpu_var,#jump1]
	ldr r1, 2b
	str r1, [reg_cpu_var,#jump2]
	ldr r1, 3b
	str r1, [reg_cpu_var,#jump3]
	ldr r1, 4b
	str r1, [reg_cpu_var,#jump4]
	str	r9,[reg_cpu_var,#iphone_r9s_ofs]
	@ init registers
	load_regs
	@ get cpu mode from flag and init jump table
	s9xfixcycles

mainloop:
	@ apu execute
	asmapu_execute

	@ test flags
	ldr		rscratch,[reg_cpu_var,#flags_ofs]
	movs		rscratch,rscratch
	bne		cpuflags_set	@ if flags => check for irq/nmi/scan_keys...	
	
	exec_op						@ execute next opcode
	
cpuflags_set:	@ check flags (!=0)
		tst	rscratch,#nmi_flag		@ check nmi
		beq	cpuflagsnmi_flag_cleared	
		ldr	rscratch2,[reg_cpu_var,#nmicyclecount_ofs]
		subs	rscratch2,rscratch2,#1
		str	rscratch2,[reg_cpu_var,#nmicyclecount_ofs]		
		bne	cpuflagsnmi_flag_cleared	
		bic	rscratch,rscratch,#nmi_flag
		str	rscratch,[reg_cpu_var,#flags_ofs]		
		ldrb	rscratch2,[reg_cpu_var,#waitingforinterrupt_ofs]
		movs	rscratch2,rscratch2
		beq	notcpuaitingforinterruptnmi
		mov	rscratch2,#0
		add	rpc,rpc,#1
		strb	rscratch2,[reg_cpu_var,#waitingforinterrupt_ofs]		
notcpuaitingforinterruptnmi:
		s9xopcode_nmi
		ldr	rscratch,[reg_cpu_var,#flags_ofs]	
cpuflagsnmi_flag_cleared:
		tst	rscratch,#irq_pending_flag   @ check irq_pending_flag
		beq	cpuflagsirq_pending_flag_cleared		
		ldr	rscratch2,[reg_cpu_var,#irqcyclecount_ofs]
		movs	rscratch2,rscratch2
		bne	cpuirqcyclecount_notzero		
	 	ldrb	rscratch2,[reg_cpu_var,#waitingforinterrupt_ofs]
		movs	rscratch2,rscratch2
		beq	notcpuaitingforinterruptirq
	        mov	rscratch2,#0
		add	rpc,rpc,#1
		strb	rscratch2,[reg_cpu_var,#waitingforinterrupt_ofs]
notcpuaitingforinterruptirq:
		ldrb	rscratch2,[reg_cpu_var,#irqactive_ofs]
		movs	rscratch2,rscratch2
		beq	cpuirqactive_cleared
		tst	rstatus,#mask_irq
		bne	cpuflagsirq_pending_flag_cleared
		s9xopcode_irq
		ldr	rscratch,[reg_cpu_var,#flags_ofs]	
		b	cpuflagsirq_pending_flag_cleared
cpuirqactive_cleared:		
		bic	rscratch,rscratch,#irq_pending_flag
		str	rscratch,[reg_cpu_var,#flags_ofs]	
		b	cpuflagsirq_pending_flag_cleared
cpuirqcyclecount_notzero:
		sub	rscratch2,rscratch2,#1
		str	rscratch2,[reg_cpu_var,#irqcyclecount_ofs]
cpuflagsirq_pending_flag_cleared:

		tst	rscratch,#scan_keys_flag   @ check scan_keys_flag
		bne	endmainloop		

	exec_op	@ execute next opcode

endmainloop:

    /*registers.pc = cpu.pc - cpu.pcbase;
    s9xpackstatus ();
    apuregisters.pc = iapu.pc - iapu.ram;
    s9xapupackstatus ();
    
    if (cpu.flags & scan_keys_flag)
    {
	    s9xsyncspeed ();
	cpu.flags &= ~scan_keys_flag;
    }	*/
/********end*/
	save_regs
	ldmfd		r13!,{r4-r11,lr}
	mov		pc,lr
@@.pool

@ void test_opcode(struct asm_cpu_var *asm_var);
_test_opcode:
	@ save registers
	stmfd		r13!,{r4-r11,lr}
	@ init pointer to cpuvar structure
	mov		reg_cpu_var,r0
	@ init registers
	load_regs
	@ get cpu mode from flag and init jump table
	s9xfixcycles
	
	exec_op
@@.pool

/*****************************************************************
       asm code
*****************************************************************/

	
jumptable1:	.long	op00mod1
			.long	op01m1mod1
			.long	op02mod1
			.long	op03m1mod1
			.long	op04m1mod1
			.long	op05m1mod1
			.long	op06m1mod1
			.long	op07m1mod1
			.long	op08mod1
			.long	op09m1mod1
			.long	op0am1mod1
			.long	op0bmod1
			.long	op0cm1mod1
			.long	op0dm1mod1
			.long	op0em1mod1
			.long	op0fm1mod1
			.long	op10mod1
			.long	op11m1mod1
			.long	op12m1mod1
			.long	op13m1mod1
			.long	op14m1mod1
			.long	op15m1mod1
			.long	op16m1mod1
			.long	op17m1mod1
			.long	op18mod1
			.long	op19m1mod1
			.long	op1am1mod1
			.long	op1bmod1
			.long	op1cm1mod1
			.long	op1dm1mod1
			.long	op1em1mod1
			.long	op1fm1mod1
			.long	op20mod1
			.long	op21m1mod1
			.long	op22mod1
			.long	op23m1mod1
			.long	op24m1mod1
			.long	op25m1mod1
			.long	op26m1mod1
			.long	op27m1mod1
			.long	op28mod1
			.long	op29m1mod1
			.long	op2am1mod1
			.long	op2bmod1
			.long	op2cm1mod1
			.long	op2dm1mod1
			.long	op2em1mod1
			.long	op2fm1mod1
			.long	op30mod1
			.long	op31m1mod1
			.long	op32m1mod1
			.long	op33m1mod1
			.long	op34m1mod1
			.long	op35m1mod1
			.long	op36m1mod1
			.long	op37m1mod1
			.long	op38mod1
			.long	op39m1mod1
			.long	op3am1mod1
			.long	op3bmod1
			.long	op3cm1mod1
			.long	op3dm1mod1
			.long	op3em1mod1
			.long	op3fm1mod1
			.long	op40mod1
			.long	op41m1mod1
			.long	op42mod1
			.long	op43m1mod1
			.long	op44x1mod1
			.long	op45m1mod1
			.long	op46m1mod1
			.long	op47m1mod1
			.long	op48m1mod1
			.long	op49m1mod1
			.long	op4am1mod1
			.long	op4bmod1
			.long	op4cmod1
			.long	op4dm1mod1
			.long	op4em1mod1
			.long	op4fm1mod1
			.long	op50mod1
			.long	op51m1mod1
			.long	op52m1mod1
			.long	op53m1mod1
			.long	op54x1mod1
			.long	op55m1mod1
			.long	op56m1mod1
			.long	op57m1mod1
			.long	op58mod1
			.long	op59m1mod1
			.long	op5ax1mod1
			.long	op5bmod1
			.long	op5cmod1
			.long	op5dm1mod1
			.long	op5em1mod1
			.long	op5fm1mod1
			.long	op60mod1
			.long	op61m1mod1
			.long	op62mod1
			.long	op63m1mod1
			.long	op64m1mod1
			.long	op65m1mod1
			.long	op66m1mod1
			.long	op67m1mod1
			.long	op68m1mod1
			.long	op69m1mod1
			.long	op6am1mod1
			.long	op6bmod1
			.long	op6cmod1
			.long	op6dm1mod1
			.long	op6em1mod1
			.long	op6fm1mod1
			.long	op70mod1
			.long	op71m1mod1
			.long	op72m1mod1
			.long	op73m1mod1
			.long	op74m1mod1
			.long	op75m1mod1
			.long	op76m1mod1
			.long	op77m1mod1
			.long	op78mod1
			.long	op79m1mod1
			.long	op7ax1mod1
			.long	op7bmod1
			.long	op7cmod1
			.long	op7dm1mod1
			.long	op7em1mod1
			.long	op7fm1mod1
			.long	op80mod1
			.long	op81m1mod1
			.long	op82mod1
			.long	op83m1mod1
			.long	op84x1mod1
			.long	op85m1mod1
			.long	op86x1mod1
			.long	op87m1mod1
			.long	op88x1mod1
			.long	op89m1mod1
			.long	op8am1mod1
			.long	op8bmod1
			.long	op8cx1mod1
			.long	op8dm1mod1
			.long	op8ex1mod1
			.long	op8fm1mod1
			.long	op90mod1
			.long	op91m1mod1
			.long	op92m1mod1
			.long	op93m1mod1
			.long	op94x1mod1
			.long	op95m1mod1
			.long	op96x1mod1
			.long	op97m1mod1
			.long	op98m1mod1
			.long	op99m1mod1
			.long	op9amod1
			.long	op9bx1mod1
			.long	op9cm1mod1
			.long	op9dm1mod1
			.long	op9em1mod1
			.long	op9fm1mod1
			.long	opa0x1mod1
			.long	opa1m1mod1
			.long	opa2x1mod1
			.long	opa3m1mod1
			.long	opa4x1mod1
			.long	opa5m1mod1
			.long	opa6x1mod1
			.long	opa7m1mod1
			.long	opa8x1mod1
			.long	opa9m1mod1
			.long	opaax1mod1
			.long	opabmod1
			.long	opacx1mod1
			.long	opadm1mod1
			.long	opaex1mod1
			.long	opafm1mod1
			.long	opb0mod1
			.long	opb1m1mod1
			.long	opb2m1mod1
			.long	opb3m1mod1
			.long	opb4x1mod1
			.long	opb5m1mod1
			.long	opb6x1mod1
			.long	opb7m1mod1
			.long	opb8mod1
			.long	opb9m1mod1
			.long	opbax1mod1
			.long	opbbx1mod1
			.long	opbcx1mod1
			.long	opbdm1mod1
			.long	opbex1mod1
			.long	opbfm1mod1
			.long	opc0x1mod1
			.long	opc1m1mod1
			.long	opc2mod1
			.long	opc3m1mod1
			.long	opc4x1mod1
			.long	opc5m1mod1
			.long	opc6m1mod1
			.long	opc7m1mod1
			.long	opc8x1mod1
			.long	opc9m1mod1
			.long	opcax1mod1
			.long	opcbmod1
			.long	opccx1mod1
			.long	opcdm1mod1
			.long	opcem1mod1
			.long	opcfm1mod1
			.long	opd0mod1
			.long	opd1m1mod1
			.long	opd2m1mod1
			.long	opd3m1mod1
			.long	opd4mod1
			.long	opd5m1mod1
			.long	opd6m1mod1
			.long	opd7m1mod1
			.long	opd8mod1
			.long	opd9m1mod1
			.long	opdax1mod1
			.long	opdbmod1
			.long	opdcmod1
			.long	opddm1mod1
			.long	opdem1mod1
			.long	opdfm1mod1
			.long	ope0x1mod1
			.long	ope1m1mod1
			.long	ope2mod1
			.long	ope3m1mod1
			.long	ope4x1mod1
			.long	ope5m1mod1
			.long	ope6m1mod1
			.long	ope7m1mod1
			.long	ope8x1mod1
			.long	ope9m1mod1
			.long	opeamod1
			.long	opebmod1
			.long	opecx1mod1
			.long	opedm1mod1
			.long	opeem1mod1
			.long	opefm1mod1
			.long	opf0mod1
			.long	opf1m1mod1
			.long	opf2m1mod1
			.long	opf3m1mod1
			.long	opf4mod1
			.long	opf5m1mod1
			.long	opf6m1mod1
			.long	opf7m1mod1
			.long	opf8mod1
			.long	opf9m1mod1
			.long	opfax1mod1
			.long	opfbmod1
			.long	opfcmod1
			.long	opfdm1mod1
			.long	opfem1mod1
			.long	opffm1mod1
			
op00mod1:
lbl00mod1:	op00
			nextopcode
op01m1mod1:
lbl01mod1a:	directindexedindirect1
lbl01mod1b:	ora8
			nextopcode
op02mod1:
lbl02mod1:	op02
			nextopcode
op03m1mod1:
lbl03mod1a:	stackasmrelative
lbl03mod1b:	ora8
			nextopcode
op04m1mod1:
lbl04mod1a:	direct
lbl04mod1b:	tsb8
			nextopcode
op05m1mod1:
lbl05mod1a:	direct
lbl05mod1b:	ora8
			nextopcode
op06m1mod1:
lbl06mod1a:	direct
lbl06mod1b:	asl8
			nextopcode
op07m1mod1:
lbl07mod1a:	directindirectlong
lbl07mod1b:	ora8
			nextopcode
op08mod1:
lbl08mod1:	op08
			nextopcode
op09m1mod1:
lbl09mod1:	op09m1
			nextopcode
op0am1mod1:
lbl0amod1a:	a_asl8
			nextopcode
op0bmod1:
lbl0bmod1:	op0b
			nextopcode
op0cm1mod1:
lbl0cmod1a:	absolute
lbl0cmod1b:	tsb8
			nextopcode
op0dm1mod1:
lbl0dmod1a:	absolute
lbl0dmod1b:	ora8
			nextopcode
op0em1mod1:
lbl0emod1a:	absolute
lbl0emod1b:	asl8
			nextopcode
op0fm1mod1:
lbl0fmod1a:	absolutelong
lbl0fmod1b:	ora8
			nextopcode
op10mod1:
lbl10mod1:	op10
			nextopcode
op11m1mod1:
lbl11mod1a:	directindirectindexed1
lbl11mod1b:	ora8
			nextopcode
op12m1mod1:
lbl12mod1a:	directindirect
lbl12mod1b:	ora8
			nextopcode
op13m1mod1:
lbl13mod1a:	stackasmrelativeindirectindexed1
lbl13mod1b:	ora8
			nextopcode
op14m1mod1:
lbl14mod1a:	direct
lbl14mod1b:	trb8
			nextopcode
op15m1mod1:
lbl15mod1a:	directindexedx1
lbl15mod1b:	ora8
			nextopcode
op16m1mod1:
lbl16mod1a:	directindexedx1
lbl16mod1b:	asl8
			nextopcode
op17m1mod1:
lbl17mod1a:	directindirectindexedlong1
lbl17mod1b:	ora8
			nextopcode
op18mod1:
lbl18mod1:	op18
			nextopcode
op19m1mod1:
lbl19mod1a:	absoluteindexedy1
lbl19mod1b:	ora8
			nextopcode
op1am1mod1:
lbl1amod1a:	a_inc8
			nextopcode
op1bmod1:
lbl1bmod1:	op1bm1
			nextopcode
op1cm1mod1:
lbl1cmod1a:	absolute
lbl1cmod1b:	trb8
			nextopcode
op1dm1mod1:
lbl1dmod1a:	absoluteindexedx1
lbl1dmod1b:	ora8
			nextopcode
op1em1mod1:
lbl1emod1a:	absoluteindexedx1
lbl1emod1b:	asl8
			nextopcode
op1fm1mod1:
lbl1fmod1a:	absolutelongindexedx1
lbl1fmod1b:	ora8
			nextopcode
op20mod1:
lbl20mod1:	op20
			nextopcode
op21m1mod1:
lbl21mod1a:	directindexedindirect1
lbl21mod1b:	and8
			nextopcode
op22mod1:
lbl22mod1:	op22
			nextopcode
op23m1mod1:
lbl23mod1a:	stackasmrelative
lbl23mod1b:	and8
			nextopcode
op24m1mod1:
lbl24mod1a:	direct
lbl24mod1b:	bit8
			nextopcode
op25m1mod1:
lbl25mod1a:	direct
lbl25mod1b:	and8
			nextopcode
op26m1mod1:
lbl26mod1a:	direct
lbl26mod1b:	rol8
			nextopcode
op27m1mod1:
lbl27mod1a:	directindirectlong
lbl27mod1b:	and8
			nextopcode
op28mod1:
lbl28mod1:	op28x1m1
			nextopcode
@@.pool			
op29m1mod1:
lbl29mod1:	op29m1
			nextopcode
op2am1mod1:
lbl2amod1a:	a_rol8
			nextopcode
op2bmod1:
lbl2bmod1:	op2b
			nextopcode
op2cm1mod1:
lbl2cmod1a:	absolute
lbl2cmod1b:	bit8
			nextopcode
op2dm1mod1:
lbl2dmod1a:	absolute
lbl2dmod1b:	and8
			nextopcode
op2em1mod1:
lbl2emod1a:	absolute
lbl2emod1b:	rol8
			nextopcode
op2fm1mod1:
lbl2fmod1a:	absolutelong
lbl2fmod1b:	and8
			nextopcode
op30mod1:
lbl30mod1:	op30
			nextopcode
op31m1mod1:
lbl31mod1a:	directindirectindexed1
lbl31mod1b:	and8
			nextopcode
op32m1mod1:
lbl32mod1a:	directindirect
lbl32mod1b:	and8
			nextopcode
op33m1mod1:
lbl33mod1a:	stackasmrelativeindirectindexed1
lbl33mod1b:	and8
			nextopcode
op34m1mod1:
lbl34mod1a:	directindexedx1
lbl34mod1b:	bit8
			nextopcode
op35m1mod1:
lbl35mod1a:	directindexedx1
lbl35mod1b:	and8
			nextopcode
op36m1mod1:
lbl36mod1a:	directindexedx1
lbl36mod1b:	rol8
			nextopcode
op37m1mod1:
lbl37mod1a:	directindirectindexedlong1
lbl37mod1b:	and8
			nextopcode
op38mod1:
lbl38mod1:	op38
			nextopcode
op39m1mod1:
lbl39mod1a:	absoluteindexedy1
lbl39mod1b:	and8
			nextopcode
op3am1mod1:
lbl3amod1a:	a_dec8
			nextopcode
op3bmod1:
lbl3bmod1:	op3bm1
			nextopcode
op3cm1mod1:
lbl3cmod1a:	absoluteindexedx1
lbl3cmod1b:	bit8
			nextopcode
op3dm1mod1:
lbl3dmod1a:	absoluteindexedx1
lbl3dmod1b:	and8
			nextopcode
op3em1mod1:
lbl3emod1a:	absoluteindexedx1
lbl3emod1b:	rol8
			nextopcode
op3fm1mod1:
lbl3fmod1a:	absolutelongindexedx1
lbl3fmod1b:	and8
			nextopcode
op40mod1:
lbl40mod1:	op40x1m1
			nextopcode
@@.pool						
op41m1mod1:
lbl41mod1a:	directindexedindirect1
lbl41mod1b:	eor8
			nextopcode
op42mod1:
lbl42mod1:	op42
			nextopcode
op43m1mod1:
lbl43mod1a:	stackasmrelative
lbl43mod1b:	eor8
			nextopcode
op44x1mod1:
lbl44mod1:	op44x1m1
			nextopcode
op45m1mod1:
lbl45mod1a:	direct
lbl45mod1b:	eor8
			nextopcode
op46m1mod1:
lbl46mod1a:	direct
lbl46mod1b:	lsr8
			nextopcode
op47m1mod1:
lbl47mod1a:	directindirectlong
lbl47mod1b:	eor8
			nextopcode
op48m1mod1:
lbl48mod1:	op48m1
			nextopcode
op49m1mod1:
lbl49mod1:	op49m1
			nextopcode
op4am1mod1:
lbl4amod1a:	a_lsr8
			nextopcode
op4bmod1:
lbl4bmod1:	op4b
			nextopcode
op4cmod1:
lbl4cmod1:	op4c
			nextopcode
op4dm1mod1:
lbl4dmod1a:	absolute
lbl4dmod1b:	eor8
			nextopcode
op4em1mod1:
lbl4emod1a:	absolute
lbl4emod1b:	lsr8
			nextopcode
op4fm1mod1:
lbl4fmod1a:	absolutelong
lbl4fmod1b:	eor8
			nextopcode
op50mod1:
lbl50mod1:	op50
			nextopcode
op51m1mod1:
lbl51mod1a:	directindirectindexed1
lbl51mod1b:	eor8
			nextopcode
op52m1mod1:
lbl52mod1a:	directindirect
lbl52mod1b:	eor8
			nextopcode
op53m1mod1:
lbl53mod1a:	stackasmrelativeindirectindexed1
lbl53mod1b:	eor8
			nextopcode
op54x1mod1:
lbl54mod1:	op54x1m1
			nextopcode
op55m1mod1:
lbl55mod1a:	directindexedx1
lbl55mod1b:	eor8
			nextopcode
op56m1mod1:
lbl56mod1a:	directindexedx1
lbl56mod1b:	lsr8
			nextopcode
op57m1mod1:
lbl57mod1a:	directindirectindexedlong1
lbl57mod1b:	eor8
			nextopcode
op58mod1:
lbl58mod1:	op58
			nextopcode
op59m1mod1:
lbl59mod1a:	absoluteindexedy1
lbl59mod1b:	eor8
			nextopcode
op5ax1mod1:
lbl5amod1:	op5ax1
			nextopcode
op5bmod1:
lbl5bmod1:	op5bm1
			nextopcode
op5cmod1:
lbl5cmod1:	op5c
			nextopcode
op5dm1mod1:
lbl5dmod1a:	absoluteindexedx1
lbl5dmod1b:	eor8
			nextopcode
op5em1mod1:
lbl5emod1a:	absoluteindexedx1
lbl5emod1b:	lsr8
			nextopcode
op5fm1mod1:
lbl5fmod1a:	absolutelongindexedx1
lbl5fmod1b:	eor8
			nextopcode
op60mod1:
lbl60mod1:	op60
			nextopcode
op61m1mod1:
lbl61mod1a:	directindexedindirect1
lbl61mod1b:	adc8
			nextopcode
op62mod1:
lbl62mod1:	op62
			nextopcode
op63m1mod1:
lbl63mod1a:	stackasmrelative
lbl63mod1b:	adc8
			nextopcode
op64m1mod1:
lbl64mod1a:	direct
lbl64mod1b:	stz8
			nextopcode
op65m1mod1:
lbl65mod1a:	direct
lbl65mod1b:	adc8
			nextopcode
op66m1mod1:
lbl66mod1a:	direct
lbl66mod1b:	ror8
			nextopcode
op67m1mod1:
lbl67mod1a:	directindirectlong
lbl67mod1b:	adc8
			nextopcode
op68m1mod1:
lbl68mod1:	op68m1
			nextopcode
op69m1mod1:
lbl69mod1a:	immediate8
lbl69mod1b:	adc8
			nextopcode
op6am1mod1:
lbl6amod1a:	a_ror8
			nextopcode
op6bmod1:
lbl6bmod1:	op6b
			nextopcode
op6cmod1:
lbl6cmod1:	op6c
			nextopcode
op6dm1mod1:
lbl6dmod1a:	absolute
lbl6dmod1b:	adc8
			nextopcode
op6em1mod1:
lbl6emod1a:	absolute
lbl6emod1b:	ror8
			nextopcode
op6fm1mod1:
lbl6fmod1a:	absolutelong
lbl6fmod1b:	adc8
			nextopcode
op70mod1:
lbl70mod1:	op70
			nextopcode
op71m1mod1:
lbl71mod1a:	directindirectindexed1
lbl71mod1b:	adc8
			nextopcode
op72m1mod1:
lbl72mod1a:	directindirect
lbl72mod1b:	adc8
			nextopcode
op73m1mod1:
lbl73mod1a:	stackasmrelativeindirectindexed1
lbl73mod1b:	adc8
			nextopcode

op74m1mod1:
lbl74mod1a:	directindexedx1
lbl74mod1b:	stz8
			nextopcode
op75m1mod1:
lbl75mod1a:	directindexedx1
lbl75mod1b:	adc8
			nextopcode
op76m1mod1:
lbl76mod1a:	directindexedx1
lbl76mod1b:	ror8
			nextopcode
op77m1mod1:
lbl77mod1a:	directindirectindexedlong1
lbl77mod1b:	adc8
			nextopcode
op78mod1:
lbl78mod1:	op78
			nextopcode
op79m1mod1:
lbl79mod1a:	absoluteindexedy1
lbl79mod1b:	adc8
			nextopcode
op7ax1mod1:
lbl7amod1:	op7ax1
			nextopcode
op7bmod1:
lbl7bmod1:	op7bm1
			nextopcode
op7cmod1:
lbl7cmod1:	absoluteindexedindirectx1
		op7c
			nextopcode
op7dm1mod1:
lbl7dmod1a:	absoluteindexedx1
lbl7dmod1b:	adc8
			nextopcode
op7em1mod1:
lbl7emod1a:	absoluteindexedx1
lbl7emod1b:	ror8
			nextopcode
op7fm1mod1:
lbl7fmod1a:	absolutelongindexedx1
lbl7fmod1b:	adc8
			nextopcode


op80mod1:
lbl80mod1:	op80
			nextopcode
op81m1mod1:
lbl81mod1a:	directindexedindirect1
lbl81mod1b:	op81m1
			nextopcode
op82mod1:
lbl82mod1:	op82
			nextopcode
op83m1mod1:
lbl83mod1a:	stackasmrelative
lbl83mod1b:	sta8
			nextopcode
op84x1mod1:
lbl84mod1a:	direct
lbl84mod1b:	sty8
			nextopcode
op85m1mod1:
lbl85mod1a:	direct
lbl85mod1b:	sta8
			nextopcode
op86x1mod1:
lbl86mod1a:	direct
lbl86mod1b:	stx8
			nextopcode
op87m1mod1:
lbl87mod1a:	directindirectlong
lbl87mod1b:	sta8
			nextopcode
op88x1mod1:
lbl88mod1:	op88x1
			nextopcode
op89m1mod1:
lbl89mod1:	op89m1
			nextopcode
op8am1mod1:
lbl8amod1:	op8am1x1
			nextopcode
op8bmod1:
lbl8bmod1:	op8b
			nextopcode
op8cx1mod1:
lbl8cmod1a:	absolute
lbl8cmod1b:	sty8
			nextopcode
op8dm1mod1:
lbl8dmod1a:	absolute
lbl8dmod1b:	sta8
			nextopcode
op8ex1mod1:
lbl8emod1a:	absolute
lbl8emod1b:	stx8
			nextopcode
op8fm1mod1:
lbl8fmod1a:	absolutelong
lbl8fmod1b:	sta8
			nextopcode
op90mod1:
lbl90mod1:	op90
			nextopcode
op91m1mod1:
lbl91mod1a:	directindirectindexed1
lbl91mod1b:	sta8
			nextopcode
op92m1mod1:
lbl92mod1a:	directindirect
lbl92mod1b:	sta8
			nextopcode
op93m1mod1:
lbl93mod1a:	stackasmrelativeindirectindexed1
lbl93mod1b:	sta8
			nextopcode
op94x1mod1:
lbl94mod1a:	directindexedx1
lbl94mod1b:	sty8
			nextopcode
op95m1mod1:
lbl95mod1a:	directindexedx1
lbl95mod1b:	sta8
			nextopcode
op96x1mod1:
lbl96mod1a:	directindexedy1
lbl96mod1b:	stx8
			nextopcode
op97m1mod1:
lbl97mod1a:	directindirectindexedlong1
lbl97mod1b:	sta8
			nextopcode
op98m1mod1:
lbl98mod1:	op98m1x1
			nextopcode
op99m1mod1:
lbl99mod1a:	absoluteindexedy1
lbl99mod1b:	sta8
			nextopcode
op9amod1:
lbl9amod1:	op9ax1
			nextopcode
op9bx1mod1:
lbl9bmod1:	op9bx1
			nextopcode
op9cm1mod1:
lbl9cmod1a:	absolute
lbl9cmod1b:	stz8
			nextopcode
op9dm1mod1:
lbl9dmod1a:	absoluteindexedx1
lbl9dmod1b:	sta8
			nextopcode
op9em1mod1:	
lbl9emod1:	absoluteindexedx1		
		stz8
			nextopcode
op9fm1mod1:
lbl9fmod1a:	absolutelongindexedx1
lbl9fmod1b:	sta8
			nextopcode
opa0x1mod1:
lbla0mod1:	opa0x1
			nextopcode
opa1m1mod1:
lbla1mod1a:	directindexedindirect1
lbla1mod1b:	lda8
			nextopcode
opa2x1mod1:
lbla2mod1:	opa2x1
			nextopcode
opa3m1mod1:
lbla3mod1a:	stackasmrelative
lbla3mod1b:	lda8
			nextopcode
opa4x1mod1:
lbla4mod1a:	direct
lbla4mod1b:	ldy8
			nextopcode
opa5m1mod1:
lbla5mod1a:	direct
lbla5mod1b:	lda8
			nextopcode
opa6x1mod1:
lbla6mod1a:	direct
lbla6mod1b:	ldx8
			nextopcode
opa7m1mod1:
lbla7mod1a:	directindirectlong
lbla7mod1b:	lda8
			nextopcode
opa8x1mod1:
lbla8mod1:	opa8x1m1
			nextopcode
opa9m1mod1:
lbla9mod1:	opa9m1
			nextopcode
opaax1mod1:
lblaamod1:	opaax1m1
			nextopcode
opabmod1:
lblabmod1:	opab
			nextopcode
opacx1mod1:
lblacmod1a:	absolute
lblacmod1b:	ldy8
			nextopcode
opadm1mod1:
lbladmod1a:	absolute
lbladmod1b:	lda8
			nextopcode
opaex1mod1:
lblaemod1a:	absolute
lblaemod1b:	ldx8
			nextopcode
opafm1mod1:
lblafmod1a:	absolutelong
lblafmod1b:	lda8
			nextopcode
opb0mod1:
lblb0mod1:	opb0
			nextopcode
opb1m1mod1:
lblb1mod1a:	directindirectindexed1
lblb1mod1b:	lda8
			nextopcode
opb2m1mod1:
lblb2mod1a:	directindirect
lblb2mod1b:	lda8
			nextopcode
opb3m1mod1:
lblb3mod1a:	stackasmrelativeindirectindexed1
lblb3mod1b:	lda8
			nextopcode
opb4x1mod1:
lblb4mod1a:	directindexedx1
lblb4mod1b:	ldy8
			nextopcode
opb5m1mod1:
lblb5mod1a:	directindexedx1
lblb5mod1b:	lda8
			nextopcode
opb6x1mod1:
lblb6mod1a:	directindexedy1
lblb6mod1b:	ldx8
			nextopcode
opb7m1mod1:
lblb7mod1a:	directindirectindexedlong1
lblb7mod1b:	lda8
			nextopcode
opb8mod1:
lblb8mod1:	opb8
			nextopcode
opb9m1mod1:
lblb9mod1a:	absoluteindexedy1
lblb9mod1b:	lda8
			nextopcode
opbax1mod1:
lblbamod1:	opbax1
			nextopcode
opbbx1mod1:
lblbbmod1:	opbbx1
			nextopcode
opbcx1mod1:
lblbcmod1a:	absoluteindexedx1
lblbcmod1b:	ldy8
			nextopcode
opbdm1mod1:
lblbdmod1a:	absoluteindexedx1
lblbdmod1b:	lda8
			nextopcode
opbex1mod1:
lblbemod1a:	absoluteindexedy1
lblbemod1b:	ldx8
			nextopcode
opbfm1mod1:
lblbfmod1a:	absolutelongindexedx1
lblbfmod1b:	lda8
			nextopcode
opc0x1mod1:
lblc0mod1:	opc0x1
			nextopcode
opc1m1mod1:
lblc1mod1a:	directindexedindirect1
lblc1mod1b:	cmp8
			nextopcode
opc2mod1:
lblc2mod1:	opc2
			nextopcode
@@.pool
opc3m1mod1:
lblc3mod1a:	stackasmrelative
lblc3mod1b:	cmp8
			nextopcode
opc4x1mod1:
lblc4mod1a:	direct
lblc4mod1b:	cmy8
			nextopcode
opc5m1mod1:
lblc5mod1a:	direct
lblc5mod1b:	cmp8
			nextopcode
opc6m1mod1:
lblc6mod1a:	direct
lblc6mod1b:	dec8
			nextopcode
opc7m1mod1:
lblc7mod1a:	directindirectlong
lblc7mod1b:	cmp8
			nextopcode
opc8x1mod1:
lblc8mod1:	opc8x1
			nextopcode
opc9m1mod1:
lblc9mod1:	opc9m1
			nextopcode
opcax1mod1:
lblcamod1:	opcax1
			nextopcode
opcbmod1:
lblcbmod1:	opcb
			nextopcode
opccx1mod1:
lblccmod1a:	absolute
lblccmod1b:	cmy8
			nextopcode
opcdm1mod1:
lblcdmod1a:	absolute
lblcdmod1b:	cmp8
			nextopcode
opcem1mod1:
lblcemod1a:	absolute
lblcemod1b:	dec8
			nextopcode
opcfm1mod1:
lblcfmod1a:	absolutelong
lblcfmod1b:	cmp8
			nextopcode
opd0mod1:
lbld0mod1:	opd0
			nextopcode
opd1m1mod1:
lbld1mod1a:	directindirectindexed1
lbld1mod1b:	cmp8
			nextopcode
opd2m1mod1:
lbld2mod1a:	directindirect
lbld2mod1b:	cmp8
			nextopcode
opd3m1mod1:
lbld3mod1a:	stackasmrelativeindirectindexed1
lbld3mod1b:	cmp8
			nextopcode
opd4mod1:
lbld4mod1:	opd4
			nextopcode
opd5m1mod1:
lbld5mod1a:	directindexedx1
lbld5mod1b:	cmp8
			nextopcode
opd6m1mod1:
lbld6mod1a:	directindexedx1
lbld6mod1b:	dec8
			nextopcode
opd7m1mod1:
lbld7mod1a:	directindirectindexedlong1
lbld7mod1b:	cmp8
			nextopcode
opd8mod1:
lbld8mod1:	opd8
			nextopcode
opd9m1mod1:
lbld9mod1a:	absoluteindexedy1
lbld9mod1b:	cmp8
			nextopcode
opdax1mod1:
lbldamod1:	opdax1
			nextopcode
opdbmod1:
lbldbmod1:	opdb
			nextopcode
opdcmod1:
lbldcmod1:	opdc
			nextopcode
opddm1mod1:
lblddmod1a:	absoluteindexedx1
lblddmod1b:	cmp8
			nextopcode
opdem1mod1:
lbldemod1a:	absoluteindexedx1
lbldemod1b:	dec8
			nextopcode
opdfm1mod1:
lbldfmod1a:	absolutelongindexedx1
lbldfmod1b:	cmp8
			nextopcode
ope0x1mod1:
lble0mod1:	ope0x1
			nextopcode
ope1m1mod1:
lble1mod1a:	directindexedindirect1
lble1mod1b:	sbc8
			nextopcode
ope2mod1:
lble2mod1:	ope2
			nextopcode
@@.pool
ope3m1mod1:
lble3mod1a:	stackasmrelative
lble3mod1b:	sbc8
			nextopcode
ope4x1mod1:
lble4mod1a:	direct
lble4mod1b:	cmx8
			nextopcode
ope5m1mod1:
lble5mod1a:	direct
lble5mod1b:	sbc8
			nextopcode
ope6m1mod1:
lble6mod1a:	direct
lble6mod1b:	inc8
			nextopcode
ope7m1mod1:
lble7mod1a:	directindirectlong
lble7mod1b:	sbc8
			nextopcode
ope8x1mod1:
lble8mod1:	ope8x1
			nextopcode
ope9m1mod1:
lble9mod1a:	immediate8
lble9mod1b:	sbc8
			nextopcode
opeamod1:
lbleamod1:	opea
			nextopcode
opebmod1:
lblebmod1:	opebm1
			nextopcode
opecx1mod1:
lblecmod1a:	absolute
lblecmod1b:	cmx8
			nextopcode
opedm1mod1:
lbledmod1a:	absolute
lbledmod1b:	sbc8
			nextopcode
opeem1mod1:
lbleemod1a:	absolute
lbleemod1b:	inc8
			nextopcode
opefm1mod1:
lblefmod1a:	absolutelong
lblefmod1b:	sbc8
			nextopcode
opf0mod1:
lblf0mod1:	opf0
			nextopcode
opf1m1mod1:
lblf1mod1a:	directindirectindexed1
lblf1mod1b:	sbc8
			nextopcode
opf2m1mod1:
lblf2mod1a:	directindirect
lblf2mod1b:	sbc8
			nextopcode
opf3m1mod1:
lblf3mod1a:	stackasmrelativeindirectindexed1
lblf3mod1b:	sbc8
			nextopcode
opf4mod1:
lblf4mod1:	opf4
			nextopcode
opf5m1mod1:
lblf5mod1a:	directindexedx1
lblf5mod1b:	sbc8
			nextopcode
opf6m1mod1:
lblf6mod1a:	directindexedx1
lblf6mod1b:	inc8
			nextopcode
opf7m1mod1:
lblf7mod1a:	directindirectindexedlong1
lblf7mod1b:	sbc8
			nextopcode
opf8mod1:
lblf8mod1:	opf8
			nextopcode
opf9m1mod1:
lblf9mod1a:	absoluteindexedy1
lblf9mod1b:	sbc8
			nextopcode
opfax1mod1:
lblfamod1:	opfax1
			nextopcode
opfbmod1:
lblfbmod1:	opfb
			nextopcode
opfcmod1:
lblfcmod1:	opfcx1
			nextopcode
opfdm1mod1:
lblfdmod1a:	absoluteindexedx1
lblfdmod1b:	sbc8
			nextopcode
opfem1mod1:
lblfemod1a:	absoluteindexedx1
lblfemod1b:	inc8
			nextopcode
opffm1mod1:
lblffmod1a:	absolutelongindexedx1
lblffmod1b:	sbc8
			nextopcode
@@.pool

			
jumptable2:		.long	op00mod2
			.long	op01m1mod2
			.long	op02mod2
			.long	op03m1mod2
			.long	op04m1mod2
			.long	op05m1mod2
			.long	op06m1mod2
			.long	op07m1mod2
			.long	op08mod2
			.long	op09m1mod2
			.long	op0am1mod2
			.long	op0bmod2
			.long	op0cm1mod2
			.long	op0dm1mod2
			.long	op0em1mod2
			.long	op0fm1mod2
			.long	op10mod2
			.long	op11m1mod2
			.long	op12m1mod2
			.long	op13m1mod2
			.long	op14m1mod2
			.long	op15m1mod2
			.long	op16m1mod2
			.long	op17m1mod2
			.long	op18mod2
			.long	op19m1mod2
			.long	op1am1mod2
			.long	op1bmod2
			.long	op1cm1mod2
			.long	op1dm1mod2
			.long	op1em1mod2
			.long	op1fm1mod2
			.long	op20mod2
			.long	op21m1mod2
			.long	op22mod2
			.long	op23m1mod2
			.long	op24m1mod2
			.long	op25m1mod2
			.long	op26m1mod2
			.long	op27m1mod2
			.long	op28mod2
			.long	op29m1mod2
			.long	op2am1mod2
			.long	op2bmod2
			.long	op2cm1mod2
			.long	op2dm1mod2
			.long	op2em1mod2
			.long	op2fm1mod2
			.long	op30mod2
			.long	op31m1mod2
			.long	op32m1mod2
			.long	op33m1mod2
			.long	op34m1mod2
			.long	op35m1mod2
			.long	op36m1mod2
			.long	op37m1mod2
			.long	op38mod2
			.long	op39m1mod2
			.long	op3am1mod2
			.long	op3bmod2
			.long	op3cm1mod2
			.long	op3dm1mod2
			.long	op3em1mod2
			.long	op3fm1mod2
			.long	op40mod2
			.long	op41m1mod2
			.long	op42mod2
			.long	op43m1mod2
			.long	op44x0mod2
			.long	op45m1mod2
			.long	op46m1mod2
			.long	op47m1mod2
			.long	op48m1mod2
			.long	op49m1mod2
			.long	op4am1mod2
			.long	op4bmod2
			.long	op4cmod2
			.long	op4dm1mod2
			.long	op4em1mod2
			.long	op4fm1mod2
			.long	op50mod2
			.long	op51m1mod2
			.long	op52m1mod2
			.long	op53m1mod2
			.long	op54x0mod2
			.long	op55m1mod2
			.long	op56m1mod2
			.long	op57m1mod2
			.long	op58mod2
			.long	op59m1mod2
			.long	op5ax0mod2
			.long	op5bmod2
			.long	op5cmod2
			.long	op5dm1mod2
			.long	op5em1mod2
			.long	op5fm1mod2
			.long	op60mod2
			.long	op61m1mod2
			.long	op62mod2
			.long	op63m1mod2
			.long	op64m1mod2
			.long	op65m1mod2
			.long	op66m1mod2
			.long	op67m1mod2
			.long	op68m1mod2
			.long	op69m1mod2
			.long	op6am1mod2
			.long	op6bmod2
			.long	op6cmod2
			.long	op6dm1mod2
			.long	op6em1mod2
			.long	op6fm1mod2
			.long	op70mod2
			.long	op71m1mod2
			.long	op72m1mod2
			.long	op73m1mod2
			.long	op74m1mod2
			.long	op75m1mod2
			.long	op76m1mod2
			.long	op77m1mod2
			.long	op78mod2
			.long	op79m1mod2
			.long	op7ax0mod2
			.long	op7bmod2
			.long	op7cmod2
			.long	op7dm1mod2
			.long	op7em1mod2
			.long	op7fm1mod2
			.long	op80mod2
			.long	op81m1mod2
			.long	op82mod2
			.long	op83m1mod2
			.long	op84x0mod2
			.long	op85m1mod2
			.long	op86x0mod2
			.long	op87m1mod2
			.long	op88x0mod2
			.long	op89m1mod2
			.long	op8am1mod2
			.long	op8bmod2
			.long	op8cx0mod2
			.long	op8dm1mod2
			.long	op8ex0mod2
			.long	op8fm1mod2
			.long	op90mod2
			.long	op91m1mod2
			.long	op92m1mod2
			.long	op93m1mod2
			.long	op94x0mod2
			.long	op95m1mod2
			.long	op96x0mod2
			.long	op97m1mod2
			.long	op98m1mod2
			.long	op99m1mod2
			.long	op9amod2
			.long	op9bx0mod2
			.long	op9cm1mod2
			.long	op9dm1mod2
			.long	op9em1mod2
			.long	op9fm1mod2
			.long	opa0x0mod2
			.long	opa1m1mod2
			.long	opa2x0mod2
			.long	opa3m1mod2
			.long	opa4x0mod2
			.long	opa5m1mod2
			.long	opa6x0mod2
			.long	opa7m1mod2
			.long	opa8x0mod2
			.long	opa9m1mod2
			.long	opaax0mod2
			.long	opabmod2
			.long	opacx0mod2
			.long	opadm1mod2
			.long	opaex0mod2
			.long	opafm1mod2
			.long	opb0mod2
			.long	opb1m1mod2
			.long	opb2m1mod2
			.long	opb3m1mod2
			.long	opb4x0mod2
			.long	opb5m1mod2
			.long	opb6x0mod2
			.long	opb7m1mod2
			.long	opb8mod2
			.long	opb9m1mod2
			.long	opbax0mod2
			.long	opbbx0mod2
			.long	opbcx0mod2
			.long	opbdm1mod2
			.long	opbex0mod2
			.long	opbfm1mod2
			.long	opc0x0mod2
			.long	opc1m1mod2
			.long	opc2mod2
			.long	opc3m1mod2
			.long	opc4x0mod2
			.long	opc5m1mod2
			.long	opc6m1mod2
			.long	opc7m1mod2
			.long	opc8x0mod2
			.long	opc9m1mod2
			.long	opcax0mod2
			.long	opcbmod2
			.long	opccx0mod2
			.long	opcdm1mod2
			.long	opcem1mod2
			.long	opcfm1mod2
			.long	opd0mod2
			.long	opd1m1mod2
			.long	opd2m1mod2
			.long	opd3m1mod2
			.long	opd4mod2
			.long	opd5m1mod2
			.long	opd6m1mod2
			.long	opd7m1mod2
			.long	opd8mod2
			.long	opd9m1mod2
			.long	opdax0mod2
			.long	opdbmod2
			.long	opdcmod2
			.long	opddm1mod2
			.long	opdem1mod2
			.long	opdfm1mod2
			.long	ope0x0mod2
			.long	ope1m1mod2
			.long	ope2mod2
			.long	ope3m1mod2
			.long	ope4x0mod2
			.long	ope5m1mod2
			.long	ope6m1mod2
			.long	ope7m1mod2
			.long	ope8x0mod2
			.long	ope9m1mod2
			.long	opeamod2
			.long	opebmod2
			.long	opecx0mod2
			.long	opedm1mod2
			.long	opeem1mod2
			.long	opefm1mod2
			.long	opf0mod2
			.long	opf1m1mod2
			.long	opf2m1mod2
			.long	opf3m1mod2
			.long	opf4mod2
			.long	opf5m1mod2
			.long	opf6m1mod2
			.long	opf7m1mod2
			.long	opf8mod2
			.long	opf9m1mod2
			.long	opfax0mod2
			.long	opfbmod2
			.long	opfcmod2
			.long	opfdm1mod2
			.long	opfem1mod2
			.long	opffm1mod2
op00mod2:
lbl00mod2:	op00
			nextopcode
op01m1mod2:
lbl01mod2a:	directindexedindirect0
lbl01mod2b:	ora8
			nextopcode
op02mod2:
lbl02mod2:	op02
			nextopcode
op03m1mod2:
lbl03mod2a:	stackasmrelative
lbl03mod2b:	ora8
			nextopcode
op04m1mod2:
lbl04mod2a:	direct
lbl04mod2b:	tsb8
			nextopcode
op05m1mod2:
lbl05mod2a:	direct
lbl05mod2b:	ora8
			nextopcode
op06m1mod2:
lbl06mod2a:	direct
lbl06mod2b:	asl8
			nextopcode
op07m1mod2:
lbl07mod2a:	directindirectlong
lbl07mod2b:	ora8
			nextopcode
op08mod2:
lbl08mod2:	op08
			nextopcode
op09m1mod2:
lbl09mod2:	op09m1
			nextopcode
op0am1mod2:
lbl0amod2a:	a_asl8
			nextopcode
op0bmod2:
lbl0bmod2:	op0b
			nextopcode
op0cm1mod2:
lbl0cmod2a:	absolute
lbl0cmod2b:	tsb8
			nextopcode
op0dm1mod2:
lbl0dmod2a:	absolute
lbl0dmod2b:	ora8
			nextopcode
op0em1mod2:
lbl0emod2a:	absolute
lbl0emod2b:	asl8
			nextopcode
op0fm1mod2:
lbl0fmod2a:	absolutelong
lbl0fmod2b:	ora8
			nextopcode
op10mod2:
lbl10mod2:	op10
			nextopcode
op11m1mod2:
lbl11mod2a:	directindirectindexed0
lbl11mod2b:	ora8
			nextopcode
op12m1mod2:
lbl12mod2a:	directindirect
lbl12mod2b:	ora8
			nextopcode
op13m1mod2:
lbl13mod2a:	stackasmrelativeindirectindexed0
lbl13mod2b:	ora8
			nextopcode
op14m1mod2:
lbl14mod2a:	direct
lbl14mod2b:	trb8
			nextopcode
op15m1mod2:
lbl15mod2a:	directindexedx0
lbl15mod2b:	ora8
			nextopcode
op16m1mod2:
lbl16mod2a:	directindexedx0
lbl16mod2b:	asl8
			nextopcode
op17m1mod2:
lbl17mod2a:	directindirectindexedlong0
lbl17mod2b:	ora8
			nextopcode
op18mod2:
lbl18mod2:	op18
			nextopcode
op19m1mod2:
lbl19mod2a:	absoluteindexedy0
lbl19mod2b:	ora8
			nextopcode
op1am1mod2:
lbl1amod2a:	a_inc8
			nextopcode
op1bmod2:
lbl1bmod2:	op1bm1
			nextopcode
op1cm1mod2:
lbl1cmod2a:	absolute
lbl1cmod2b:	trb8
			nextopcode
op1dm1mod2:
lbl1dmod2a:	absoluteindexedx0
lbl1dmod2b:	ora8
			nextopcode
op1em1mod2:
lbl1emod2a:	absoluteindexedx0
lbl1emod2b:	asl8
			nextopcode
op1fm1mod2:
lbl1fmod2a:	absolutelongindexedx0
lbl1fmod2b:	ora8
			nextopcode
op20mod2:
lbl20mod2:	op20
			nextopcode
op21m1mod2:
lbl21mod2a:	directindexedindirect0
lbl21mod2b:	and8
			nextopcode
op22mod2:
lbl22mod2:	op22
			nextopcode
op23m1mod2:
lbl23mod2a:	stackasmrelative
lbl23mod2b:	and8
			nextopcode
op24m1mod2:
lbl24mod2a:	direct
lbl24mod2b:	bit8
			nextopcode
op25m1mod2:
lbl25mod2a:	direct
lbl25mod2b:	and8
			nextopcode
op26m1mod2:
lbl26mod2a:	direct
lbl26mod2b:	rol8
			nextopcode
op27m1mod2:
lbl27mod2a:	directindirectlong
lbl27mod2b:	and8
			nextopcode
op28mod2:
lbl28mod2:	op28x0m1
			nextopcode
@@.pool
op29m1mod2:
lbl29mod2:	op29m1
			nextopcode
op2am1mod2:
lbl2amod2a:	a_rol8
			nextopcode
op2bmod2:
lbl2bmod2:	op2b
			nextopcode
op2cm1mod2:
lbl2cmod2a:	absolute
lbl2cmod2b:	bit8
			nextopcode
op2dm1mod2:
lbl2dmod2a:	absolute
lbl2dmod2b:	and8
			nextopcode
op2em1mod2:
lbl2emod2a:	absolute
lbl2emod2b:	rol8
			nextopcode
op2fm1mod2:
lbl2fmod2a:	absolutelong
lbl2fmod2b:	and8
			nextopcode
op30mod2:
lbl30mod2:	op30
			nextopcode
op31m1mod2:
lbl31mod2a:	directindirectindexed0
lbl31mod2b:	and8
			nextopcode
op32m1mod2:
lbl32mod2a:	directindirect
lbl32mod2b:	and8
			nextopcode
op33m1mod2:
lbl33mod2a:	stackasmrelativeindirectindexed0
lbl33mod2b:	and8
			nextopcode
op34m1mod2:
lbl34mod2a:	directindexedx0
lbl34mod2b:	bit8
			nextopcode
op35m1mod2:
lbl35mod2a:	directindexedx0
lbl35mod2b:	and8
			nextopcode
op36m1mod2:
lbl36mod2a:	directindexedx0
lbl36mod2b:	rol8
			nextopcode
op37m1mod2:
lbl37mod2a:	directindirectindexedlong0
lbl37mod2b:	and8
			nextopcode
op38mod2:
lbl38mod2:	op38
			nextopcode
op39m1mod2:
lbl39mod2a:	absoluteindexedy0
lbl39mod2b:	and8
			nextopcode
op3am1mod2:
lbl3amod2a:	a_dec8
			nextopcode
op3bmod2:
lbl3bmod2:	op3bm1
			nextopcode
op3cm1mod2:
lbl3cmod2a:	absoluteindexedx0
lbl3cmod2b:	bit8
			nextopcode
op3dm1mod2:
lbl3dmod2a:	absoluteindexedx0
lbl3dmod2b:	and8
			nextopcode
op3em1mod2:
lbl3emod2a:	absoluteindexedx0
lbl3emod2b:	rol8
			nextopcode
op3fm1mod2:
lbl3fmod2a:	absolutelongindexedx0
lbl3fmod2b:	and8
			nextopcode
op40mod2:
lbl40mod2:	op40x0m1
			nextopcode
@@.pool						
op41m1mod2:
lbl41mod2a:	directindexedindirect0
lbl41mod2b:	eor8
			nextopcode
op42mod2:
lbl42mod2:	op42
			nextopcode
op43m1mod2:
lbl43mod2a:	stackasmrelative
lbl43mod2b:	eor8
			nextopcode
op44x0mod2:
lbl44mod2:	op44x0m1
			nextopcode
op45m1mod2:
lbl45mod2a:	direct
lbl45mod2b:	eor8
			nextopcode
op46m1mod2:
lbl46mod2a:	direct
lbl46mod2b:	lsr8
			nextopcode
op47m1mod2:
lbl47mod2a:	directindirectlong
lbl47mod2b:	eor8
			nextopcode
op48m1mod2:
lbl48mod2:	op48m1
			nextopcode
op49m1mod2:
lbl49mod2:	op49m1
			nextopcode
op4am1mod2:
lbl4amod2a:	a_lsr8
			nextopcode
op4bmod2:
lbl4bmod2:	op4b
			nextopcode
op4cmod2:
lbl4cmod2:	op4c
			nextopcode
op4dm1mod2:
lbl4dmod2a:	absolute
lbl4dmod2b:	eor8
			nextopcode
op4em1mod2:
lbl4emod2a:	absolute
lbl4emod2b:	lsr8
			nextopcode
op4fm1mod2:
lbl4fmod2a:	absolutelong
lbl4fmod2b:	eor8
			nextopcode
op50mod2:
lbl50mod2:	op50
			nextopcode
op51m1mod2:
lbl51mod2a:	directindirectindexed0
lbl51mod2b:	eor8
			nextopcode
op52m1mod2:
lbl52mod2a:	directindirect
lbl52mod2b:	eor8
			nextopcode
op53m1mod2:
lbl53mod2a:	stackasmrelativeindirectindexed0
lbl53mod2b:	eor8
			nextopcode
op54x0mod2:
lbl54mod2:	op54x0m1
			nextopcode
op55m1mod2:
lbl55mod2a:	directindexedx0
lbl55mod2b:	eor8
			nextopcode
op56m1mod2:
lbl56mod2a:	directindexedx0
lbl56mod2b:	lsr8
			nextopcode
op57m1mod2:
lbl57mod2a:	directindirectindexedlong0
lbl57mod2b:	eor8
			nextopcode
op58mod2:
lbl58mod2:	op58
			nextopcode
op59m1mod2:
lbl59mod2a:	absoluteindexedy0
lbl59mod2b:	eor8
			nextopcode
op5ax0mod2:
lbl5amod2:	op5ax0
			nextopcode
op5bmod2:
lbl5bmod2:	op5bm1
			nextopcode
op5cmod2:
lbl5cmod2:	op5c
			nextopcode
op5dm1mod2:
lbl5dmod2a:	absoluteindexedx0
lbl5dmod2b:	eor8
			nextopcode
op5em1mod2:
lbl5emod2a:	absoluteindexedx0
lbl5emod2b:	lsr8
			nextopcode
op5fm1mod2:
lbl5fmod2a:	absolutelongindexedx0
lbl5fmod2b:	eor8
			nextopcode
op60mod2:
lbl60mod2:	op60
			nextopcode
op61m1mod2:
lbl61mod2a:	directindexedindirect0
lbl61mod2b:	adc8
			nextopcode
op62mod2:
lbl62mod2:	op62
			nextopcode
op63m1mod2:
lbl63mod2a:	stackasmrelative
lbl63mod2b:	adc8
			nextopcode
op64m1mod2:
lbl64mod2a:	direct
lbl64mod2b:	stz8
			nextopcode
op65m1mod2:
lbl65mod2a:	direct
lbl65mod2b:	adc8
			nextopcode
op66m1mod2:
lbl66mod2a:	direct
lbl66mod2b:	ror8
			nextopcode
op67m1mod2:
lbl67mod2a:	directindirectlong
lbl67mod2b:	adc8
			nextopcode
op68m1mod2:
lbl68mod2:	op68m1
			nextopcode
op69m1mod2:
lbl69mod2a:	immediate8
lbl69mod2b:	adc8
			nextopcode
op6am1mod2:
lbl6amod2a:	a_ror8
			nextopcode
op6bmod2:
lbl6bmod2:	op6b
			nextopcode
op6cmod2:
lbl6cmod2:	op6c
			nextopcode
op6dm1mod2:
lbl6dmod2a:	absolute
lbl6dmod2b:	adc8
			nextopcode
op6em1mod2:
lbl6emod2a:	absolute
lbl6emod2b:	ror8
			nextopcode
op6fm1mod2:
lbl6fmod2a:	absolutelong
lbl6fmod2b:	adc8
			nextopcode
op70mod2:
lbl70mod2:	op70
			nextopcode
op71m1mod2:
lbl71mod2a:	directindirectindexed0
lbl71mod2b:	adc8
			nextopcode
op72m1mod2:
lbl72mod2a:	directindirect
lbl72mod2b:	adc8
			nextopcode
op73m1mod2:
lbl73mod2a:	stackasmrelativeindirectindexed0
lbl73mod2b:	adc8
			nextopcode
op74m1mod2:
lbl74mod2a:	directindexedx0
lbl74mod2b:	stz8
			nextopcode
op75m1mod2:
lbl75mod2a:	directindexedx0
lbl75mod2b:	adc8
			nextopcode
op76m1mod2:
lbl76mod2a:	directindexedx0
lbl76mod2b:	ror8
			nextopcode
op77m1mod2:
lbl77mod2a:	directindirectindexedlong0
lbl77mod2b:	adc8
			nextopcode
op78mod2:
lbl78mod2:	op78
			nextopcode
op79m1mod2:
lbl79mod2a:	absoluteindexedy0
lbl79mod2b:	adc8
			nextopcode
op7ax0mod2:
lbl7amod2:	op7ax0
			nextopcode
op7bmod2:
lbl7bmod2:	op7bm1
			nextopcode
op7cmod2:
lbl7cmod2:	absoluteindexedindirectx0
		op7c
			nextopcode
op7dm1mod2:
lbl7dmod2a:	absoluteindexedx0
lbl7dmod2b:	adc8
			nextopcode
op7em1mod2:
lbl7emod2a:	absoluteindexedx0
lbl7emod2b:	ror8
			nextopcode
op7fm1mod2:
lbl7fmod2a:	absolutelongindexedx0
lbl7fmod2b:	adc8
			nextopcode


op80mod2:
lbl80mod2:	op80
			nextopcode
op81m1mod2:
lbl81mod2a:	directindexedindirect0
lbl81mod2b:	op81m1
			nextopcode
op82mod2:
lbl82mod2:	op82
			nextopcode
op83m1mod2:
lbl83mod2a:	stackasmrelative
lbl83mod2b:	sta8
			nextopcode
op84x0mod2:
lbl84mod2a:	direct
lbl84mod2b:	sty16
			nextopcode
op85m1mod2:
lbl85mod2a:	direct
lbl85mod2b:	sta8
			nextopcode
op86x0mod2:
lbl86mod2a:	direct
lbl86mod2b:	stx16
			nextopcode
op87m1mod2:
lbl87mod2a:	directindirectlong
lbl87mod2b:	sta8
			nextopcode
op88x0mod2:
lbl88mod2:	op88x0
			nextopcode
op89m1mod2:
lbl89mod2:	op89m1
			nextopcode
op8am1mod2:
lbl8amod2:	op8am1x0
			nextopcode
op8bmod2:
lbl8bmod2:	op8b
			nextopcode
op8cx0mod2:
lbl8cmod2a:	absolute
lbl8cmod2b:	sty16
			nextopcode
op8dm1mod2:
lbl8dmod2a:	absolute
lbl8dmod2b:	sta8
			nextopcode
op8ex0mod2:
lbl8emod2a:	absolute
lbl8emod2b:	stx16
			nextopcode
op8fm1mod2:
lbl8fmod2a:	absolutelong
lbl8fmod2b:	sta8
			nextopcode
op90mod2:
lbl90mod2:	op90
			nextopcode
op91m1mod2:
lbl91mod2a:	directindirectindexed0
lbl91mod2b:	sta8
			nextopcode
op92m1mod2:
lbl92mod2a:	directindirect
lbl92mod2b:	sta8
			nextopcode
op93m1mod2:
lbl93mod2a:	stackasmrelativeindirectindexed0
lbl93mod2b:	sta8
			nextopcode
op94x0mod2:
lbl94mod2a:	directindexedx0
lbl94mod2b:	sty16
			nextopcode
op95m1mod2:
lbl95mod2a:	directindexedx0
lbl95mod2b:	sta8
			nextopcode
op96x0mod2:
lbl96mod2a:	directindexedy0
lbl96mod2b:	stx16
			nextopcode
op97m1mod2:
lbl97mod2a:	directindirectindexedlong0
lbl97mod2b:	sta8
			nextopcode
op98m1mod2:
lbl98mod2:	op98m1x0
			nextopcode
op99m1mod2:
lbl99mod2a:	absoluteindexedy0
lbl99mod2b:	sta8
			nextopcode
op9amod2:
lbl9amod2:	op9ax0
			nextopcode
op9bx0mod2:
lbl9bmod2:	op9bx0
			nextopcode
op9cm1mod2:
lbl9cmod2a:	absolute
lbl9cmod2b:	stz8
			nextopcode
op9dm1mod2:
lbl9dmod2a:	absoluteindexedx0
lbl9dmod2b:	sta8
			nextopcode
op9em1mod2:	
lbl9emod2:	absoluteindexedx0		
		stz8
			nextopcode
op9fm1mod2:
lbl9fmod2a:	absolutelongindexedx0
lbl9fmod2b:	sta8
			nextopcode
opa0x0mod2:
lbla0mod2:	opa0x0
			nextopcode
opa1m1mod2:
lbla1mod2a:	directindexedindirect0
lbla1mod2b:	lda8
			nextopcode
opa2x0mod2:
lbla2mod2:	opa2x0
			nextopcode
opa3m1mod2:
lbla3mod2a:	stackasmrelative
lbla3mod2b:	lda8
			nextopcode
opa4x0mod2:
lbla4mod2a:	direct
lbla4mod2b:	ldy16
			nextopcode
opa5m1mod2:
lbla5mod2a:	direct
lbla5mod2b:	lda8
			nextopcode
opa6x0mod2:
lbla6mod2a:	direct
lbla6mod2b:	ldx16
			nextopcode
opa7m1mod2:
lbla7mod2a:	directindirectlong
lbla7mod2b:	lda8
			nextopcode
opa8x0mod2:
lbla8mod2:	opa8x0m1
			nextopcode
opa9m1mod2:
lbla9mod2:	opa9m1
			nextopcode
opaax0mod2:
lblaamod2:	opaax0m1
			nextopcode
opabmod2:
lblabmod2:	opab
			nextopcode
opacx0mod2:
lblacmod2a:	absolute
lblacmod2b:	ldy16
			nextopcode
opadm1mod2:
lbladmod2a:	absolute
lbladmod2b:	lda8
			nextopcode
opaex0mod2:
lblaemod2a:	absolute
lblaemod2b:	ldx16
			nextopcode
opafm1mod2:
lblafmod2a:	absolutelong
lblafmod2b:	lda8
			nextopcode
opb0mod2:
lblb0mod2:	opb0
			nextopcode
opb1m1mod2:
lblb1mod2a:	directindirectindexed0
lblb1mod2b:	lda8
			nextopcode
opb2m1mod2:
lblb2mod2a:	directindirect
lblb2mod2b:	lda8
			nextopcode
opb3m1mod2:
lblb3mod2a:	stackasmrelativeindirectindexed0
lblb3mod2b:	lda8
			nextopcode
opb4x0mod2:
lblb4mod2a:	directindexedx0
lblb4mod2b:	ldy16
			nextopcode
opb5m1mod2:
lblb5mod2a:	directindexedx0
lblb5mod2b:	lda8
			nextopcode
opb6x0mod2:
lblb6mod2a:	directindexedy0
lblb6mod2b:	ldx16
			nextopcode
opb7m1mod2:
lblb7mod2a:	directindirectindexedlong0
lblb7mod2b:	lda8
			nextopcode
opb8mod2:
lblb8mod2:	opb8
			nextopcode
opb9m1mod2:
lblb9mod2a:	absoluteindexedy0
lblb9mod2b:	lda8
			nextopcode
opbax0mod2:
lblbamod2:	opbax0
			nextopcode
opbbx0mod2:
lblbbmod2:	opbbx0
			nextopcode
opbcx0mod2:
lblbcmod2a:	absoluteindexedx0
lblbcmod2b:	ldy16
			nextopcode
opbdm1mod2:
lblbdmod2a:	absoluteindexedx0
lblbdmod2b:	lda8
			nextopcode
opbex0mod2:
lblbemod2a:	absoluteindexedy0
lblbemod2b:	ldx16
			nextopcode
opbfm1mod2:
lblbfmod2a:	absolutelongindexedx0
lblbfmod2b:	lda8
			nextopcode
opc0x0mod2:
lblc0mod2:	opc0x0
			nextopcode
opc1m1mod2:
lblc1mod2a:	directindexedindirect0
lblc1mod2b:	cmp8
			nextopcode
opc2mod2:
lblc2mod2:	opc2
			nextopcode
@@.pool
opc3m1mod2:
lblc3mod2a:	stackasmrelative
lblc3mod2b:	cmp8
			nextopcode
opc4x0mod2:
lblc4mod2a:	direct
lblc4mod2b:	cmy16
			nextopcode
opc5m1mod2:
lblc5mod2a:	direct
lblc5mod2b:	cmp8
			nextopcode
opc6m1mod2:
lblc6mod2a:	direct
lblc6mod2b:	dec8
			nextopcode
opc7m1mod2:
lblc7mod2a:	directindirectlong
lblc7mod2b:	cmp8
			nextopcode
opc8x0mod2:
lblc8mod2:	opc8x0
			nextopcode
opc9m1mod2:
lblc9mod2:	opc9m1
			nextopcode
opcax0mod2:
lblcamod2:	opcax0
			nextopcode
opcbmod2:
lblcbmod2:	opcb
			nextopcode
opccx0mod2:
lblccmod2a:	absolute
lblccmod2b:	cmy16
			nextopcode
opcdm1mod2:
lblcdmod2a:	absolute
lblcdmod2b:	cmp8
			nextopcode
opcem1mod2:
lblcemod2a:	absolute
lblcemod2b:	dec8
			nextopcode
opcfm1mod2:
lblcfmod2a:	absolutelong
lblcfmod2b:	cmp8
			nextopcode
opd0mod2:
lbld0mod2:	opd0
			nextopcode
opd1m1mod2:
lbld1mod2a:	directindirectindexed0
lbld1mod2b:	cmp8
			nextopcode
opd2m1mod2:
lbld2mod2a:	directindirect
lbld2mod2b:	cmp8
			nextopcode
opd3m1mod2:
lbld3mod2a:	stackasmrelativeindirectindexed0
lbld3mod2b:	cmp8
			nextopcode
opd4mod2:
lbld4mod2:	opd4
			nextopcode
opd5m1mod2:
lbld5mod2a:	directindexedx0
lbld5mod2b:	cmp8
			nextopcode
opd6m1mod2:
lbld6mod2a:	directindexedx0
lbld6mod2b:	dec8
			nextopcode
opd7m1mod2:
lbld7mod2a:	directindirectindexedlong0
lbld7mod2b:	cmp8
			nextopcode
opd8mod2:
lbld8mod2:	opd8
			nextopcode
opd9m1mod2:
lbld9mod2a:	absoluteindexedy0
lbld9mod2b:	cmp8
			nextopcode
opdax0mod2:
lbldamod2:	opdax0
			nextopcode
opdbmod2:
lbldbmod2:	opdb
			nextopcode
opdcmod2:
lbldcmod2:	opdc
			nextopcode
opddm1mod2:
lblddmod2a:	absoluteindexedx0
lblddmod2b:	cmp8
			nextopcode
opdem1mod2:
lbldemod2a:	absoluteindexedx0
lbldemod2b:	dec8
			nextopcode
opdfm1mod2:
lbldfmod2a:	absolutelongindexedx0
lbldfmod2b:	cmp8
			nextopcode
ope0x0mod2:
lble0mod2:	ope0x0
			nextopcode
ope1m1mod2:
lble1mod2a:	directindexedindirect0
lble1mod2b:	sbc8
			nextopcode
ope2mod2:
lble2mod2:	ope2
			nextopcode
@@.pool
ope3m1mod2:
lble3mod2a:	stackasmrelative
lble3mod2b:	sbc8
			nextopcode
ope4x0mod2:
lble4mod2a:	direct
lble4mod2b:	cmx16
			nextopcode
ope5m1mod2:
lble5mod2a:	direct
lble5mod2b:	sbc8
			nextopcode
ope6m1mod2:
lble6mod2a:	direct
lble6mod2b:	inc8
			nextopcode
ope7m1mod2:
lble7mod2a:	directindirectlong
lble7mod2b:	sbc8
			nextopcode
ope8x0mod2:
lble8mod2:	ope8x0
			nextopcode
ope9m1mod2:
lble9mod2a:	immediate8
lble9mod2b:	sbc8
			nextopcode
opeamod2:
lbleamod2:	opea
			nextopcode
opebmod2:
lblebmod2:	opebm1
			nextopcode
opecx0mod2:
lblecmod2a:	absolute
lblecmod2b:	cmx16
			nextopcode
opedm1mod2:
lbledmod2a:	absolute
lbledmod2b:	sbc8
			nextopcode
opeem1mod2:
lbleemod2a:	absolute
lbleemod2b:	inc8
			nextopcode
opefm1mod2:
lblefmod2a:	absolutelong
lblefmod2b:	sbc8
			nextopcode
opf0mod2:
lblf0mod2:	opf0
			nextopcode
opf1m1mod2:
lblf1mod2a:	directindirectindexed0
lblf1mod2b:	sbc8
			nextopcode
opf2m1mod2:
lblf2mod2a:	directindirect
lblf2mod2b:	sbc8
			nextopcode
opf3m1mod2:
lblf3mod2a:	stackasmrelativeindirectindexed0
lblf3mod2b:	sbc8
			nextopcode
opf4mod2:
lblf4mod2:	opf4
			nextopcode
opf5m1mod2:
lblf5mod2a:	directindexedx0
lblf5mod2b:	sbc8
			nextopcode
opf6m1mod2:
lblf6mod2a:	directindexedx0
lblf6mod2b:	inc8
			nextopcode
opf7m1mod2:
lblf7mod2a:	directindirectindexedlong0
lblf7mod2b:	sbc8
			nextopcode
opf8mod2:
lblf8mod2:	opf8
			nextopcode
opf9m1mod2:
lblf9mod2a:	absoluteindexedy0
lblf9mod2b:	sbc8
			nextopcode
opfax0mod2:
lblfamod2:	opfax0
			nextopcode
opfbmod2:
lblfbmod2:	opfb
			nextopcode
opfcmod2:
lblfcmod2:	opfcx0
			nextopcode
opfdm1mod2:
lblfdmod2a:	absoluteindexedx0
lblfdmod2b:	sbc8
			nextopcode
opfem1mod2:
lblfemod2a:	absoluteindexedx0
lblfemod2b:	inc8
			nextopcode
opffm1mod2:
lblffmod2a:	absolutelongindexedx0
lblffmod2b:	sbc8
			nextopcode

@@.pool


jumptable3:		.long	op00mod3
			.long	op01m0mod3
			.long	op02mod3
			.long	op03m0mod3
			.long	op04m0mod3
			.long	op05m0mod3
			.long	op06m0mod3
			.long	op07m0mod3
			.long	op08mod3
			.long	op09m0mod3
			.long	op0am0mod3
			.long	op0bmod3
			.long	op0cm0mod3
			.long	op0dm0mod3
			.long	op0em0mod3
			.long	op0fm0mod3
			.long	op10mod3
			.long	op11m0mod3
			.long	op12m0mod3
			.long	op13m0mod3
			.long	op14m0mod3
			.long	op15m0mod3
			.long	op16m0mod3
			.long	op17m0mod3
			.long	op18mod3
			.long	op19m0mod3
			.long	op1am0mod3
			.long	op1bmod3
			.long	op1cm0mod3
			.long	op1dm0mod3
			.long	op1em0mod3
			.long	op1fm0mod3
			.long	op20mod3
			.long	op21m0mod3
			.long	op22mod3
			.long	op23m0mod3
			.long	op24m0mod3
			.long	op25m0mod3
			.long	op26m0mod3
			.long	op27m0mod3
			.long	op28mod3
			.long	op29m0mod3
			.long	op2am0mod3
			.long	op2bmod3
			.long	op2cm0mod3
			.long	op2dm0mod3
			.long	op2em0mod3
			.long	op2fm0mod3
			.long	op30mod3
			.long	op31m0mod3
			.long	op32m0mod3
			.long	op33m0mod3
			.long	op34m0mod3
			.long	op35m0mod3
			.long	op36m0mod3
			.long	op37m0mod3
			.long	op38mod3
			.long	op39m0mod3
			.long	op3am0mod3
			.long	op3bmod3
			.long	op3cm0mod3
			.long	op3dm0mod3
			.long	op3em0mod3
			.long	op3fm0mod3
			.long	op40mod3
			.long	op41m0mod3
			.long	op42mod3
			.long	op43m0mod3
			.long	op44x0mod3
			.long	op45m0mod3
			.long	op46m0mod3
			.long	op47m0mod3
			.long	op48m0mod3
			.long	op49m0mod3
			.long	op4am0mod3
			.long	op4bmod3
			.long	op4cmod3
			.long	op4dm0mod3
			.long	op4em0mod3
			.long	op4fm0mod3
			.long	op50mod3
			.long	op51m0mod3
			.long	op52m0mod3
			.long	op53m0mod3
			.long	op54x0mod3
			.long	op55m0mod3
			.long	op56m0mod3
			.long	op57m0mod3
			.long	op58mod3
			.long	op59m0mod3
			.long	op5ax0mod3
			.long	op5bmod3
			.long	op5cmod3
			.long	op5dm0mod3
			.long	op5em0mod3
			.long	op5fm0mod3
			.long	op60mod3
			.long	op61m0mod3
			.long	op62mod3
			.long	op63m0mod3
			.long	op64m0mod3
			.long	op65m0mod3
			.long	op66m0mod3
			.long	op67m0mod3
			.long	op68m0mod3
			.long	op69m0mod3
			.long	op6am0mod3
			.long	op6bmod3
			.long	op6cmod3
			.long	op6dm0mod3
			.long	op6em0mod3
			.long	op6fm0mod3
			.long	op70mod3
			.long	op71m0mod3
			.long	op72m0mod3
			.long	op73m0mod3
			.long	op74m0mod3
			.long	op75m0mod3
			.long	op76m0mod3
			.long	op77m0mod3
			.long	op78mod3
			.long	op79m0mod3
			.long	op7ax0mod3
			.long	op7bmod3
			.long	op7cmod3
			.long	op7dm0mod3
			.long	op7em0mod3
			.long	op7fm0mod3
			.long	op80mod3
			.long	op81m0mod3
			.long	op82mod3
			.long	op83m0mod3
			.long	op84x0mod3
			.long	op85m0mod3
			.long	op86x0mod3
			.long	op87m0mod3
			.long	op88x0mod3
			.long	op89m0mod3
			.long	op8am0mod3
			.long	op8bmod3
			.long	op8cx0mod3
			.long	op8dm0mod3
			.long	op8ex0mod3
			.long	op8fm0mod3
			.long	op90mod3
			.long	op91m0mod3
			.long	op92m0mod3
			.long	op93m0mod3
			.long	op94x0mod3
			.long	op95m0mod3
			.long	op96x0mod3
			.long	op97m0mod3
			.long	op98m0mod3
			.long	op99m0mod3
			.long	op9amod3
			.long	op9bx0mod3
			.long	op9cm0mod3
			.long	op9dm0mod3
			.long	op9em0mod3
			.long	op9fm0mod3
			.long	opa0x0mod3
			.long	opa1m0mod3
			.long	opa2x0mod3
			.long	opa3m0mod3
			.long	opa4x0mod3
			.long	opa5m0mod3
			.long	opa6x0mod3
			.long	opa7m0mod3
			.long	opa8x0mod3
			.long	opa9m0mod3
			.long	opaax0mod3
			.long	opabmod3
			.long	opacx0mod3
			.long	opadm0mod3
			.long	opaex0mod3
			.long	opafm0mod3
			.long	opb0mod3
			.long	opb1m0mod3
			.long	opb2m0mod3
			.long	opb3m0mod3
			.long	opb4x0mod3
			.long	opb5m0mod3
			.long	opb6x0mod3
			.long	opb7m0mod3
			.long	opb8mod3
			.long	opb9m0mod3
			.long	opbax0mod3
			.long	opbbx0mod3
			.long	opbcx0mod3
			.long	opbdm0mod3
			.long	opbex0mod3
			.long	opbfm0mod3
			.long	opc0x0mod3
			.long	opc1m0mod3
			.long	opc2mod3
			.long	opc3m0mod3
			.long	opc4x0mod3
			.long	opc5m0mod3
			.long	opc6m0mod3
			.long	opc7m0mod3
			.long	opc8x0mod3
			.long	opc9m0mod3
			.long	opcax0mod3
			.long	opcbmod3
			.long	opccx0mod3
			.long	opcdm0mod3
			.long	opcem0mod3
			.long	opcfm0mod3
			.long	opd0mod3
			.long	opd1m0mod3
			.long	opd2m0mod3
			.long	opd3m0mod3
			.long	opd4mod3
			.long	opd5m0mod3
			.long	opd6m0mod3
			.long	opd7m0mod3
			.long	opd8mod3
			.long	opd9m0mod3
			.long	opdax0mod3
			.long	opdbmod3
			.long	opdcmod3
			.long	opddm0mod3
			.long	opdem0mod3
			.long	opdfm0mod3
			.long	ope0x0mod3
			.long	ope1m0mod3
			.long	ope2mod3
			.long	ope3m0mod3
			.long	ope4x0mod3
			.long	ope5m0mod3
			.long	ope6m0mod3
			.long	ope7m0mod3
			.long	ope8x0mod3
			.long	ope9m0mod3
			.long	opeamod3
			.long	opebmod3
			.long	opecx0mod3
			.long	opedm0mod3
			.long	opeem0mod3
			.long	opefm0mod3
			.long	opf0mod3
			.long	opf1m0mod3
			.long	opf2m0mod3
			.long	opf3m0mod3
			.long	opf4mod3
			.long	opf5m0mod3
			.long	opf6m0mod3
			.long	opf7m0mod3
			.long	opf8mod3
			.long	opf9m0mod3
			.long	opfax0mod3
			.long	opfbmod3
			.long	opfcmod3
			.long	opfdm0mod3
			.long	opfem0mod3
			.long	opffm0mod3
op00mod3:
lbl00mod3:	op00
			nextopcode
op01m0mod3:
lbl01mod3a:	directindexedindirect0
lbl01mod3b:	ora16
			nextopcode
op02mod3:
lbl02mod3:	op02
			nextopcode
op03m0mod3:
lbl03mod3a:	stackasmrelative
lbl03mod3b:	ora16
			nextopcode
op04m0mod3:
lbl04mod3a:	direct
lbl04mod3b:	tsb16
			nextopcode
op05m0mod3:
lbl05mod3a:	direct
lbl05mod3b:	ora16
			nextopcode
op06m0mod3:
lbl06mod3a:	direct
lbl06mod3b:	asl16
			nextopcode
op07m0mod3:
lbl07mod3a:	directindirectlong
lbl07mod3b:	ora16
			nextopcode
op08mod3:
lbl08mod3:	op08
			nextopcode
op09m0mod3:
lbl09mod3:	op09m0
			nextopcode
op0am0mod3:
lbl0amod3a:	a_asl16
			nextopcode
op0bmod3:
lbl0bmod3:	op0b
			nextopcode
op0cm0mod3:
lbl0cmod3a:	absolute
lbl0cmod3b:	tsb16
			nextopcode
op0dm0mod3:
lbl0dmod3a:	absolute
lbl0dmod3b:	ora16
			nextopcode
op0em0mod3:
lbl0emod3a:	absolute
lbl0emod3b:	asl16
			nextopcode
op0fm0mod3:
lbl0fmod3a:	absolutelong
lbl0fmod3b:	ora16
			nextopcode
op10mod3:
lbl10mod3:	op10
			nextopcode
op11m0mod3:
lbl11mod3a:	directindirectindexed0
lbl11mod3b:	ora16
			nextopcode
op12m0mod3:
lbl12mod3a:	directindirect
lbl12mod3b:	ora16
			nextopcode
op13m0mod3:
lbl13mod3a:	stackasmrelativeindirectindexed0
lbl13mod3b:	ora16
			nextopcode
op14m0mod3:
lbl14mod3a:	direct
lbl14mod3b:	trb16
			nextopcode
op15m0mod3:
lbl15mod3a:	directindexedx0
lbl15mod3b:	ora16
			nextopcode
op16m0mod3:
lbl16mod3a:	directindexedx0
lbl16mod3b:	asl16
			nextopcode
op17m0mod3:
lbl17mod3a:	directindirectindexedlong0
lbl17mod3b:	ora16
			nextopcode
op18mod3:
lbl18mod3:	op18
			nextopcode
op19m0mod3:
lbl19mod3a:	absoluteindexedy0
lbl19mod3b:	ora16
			nextopcode
op1am0mod3:
lbl1amod3a:	a_inc16
			nextopcode
op1bmod3:
lbl1bmod3:	op1bm0
			nextopcode
op1cm0mod3:
lbl1cmod3a:	absolute
lbl1cmod3b:	trb16
			nextopcode
op1dm0mod3:
lbl1dmod3a:	absoluteindexedx0
lbl1dmod3b:	ora16
			nextopcode
op1em0mod3:
lbl1emod3a:	absoluteindexedx0
lbl1emod3b:	asl16
			nextopcode
op1fm0mod3:
lbl1fmod3a:	absolutelongindexedx0
lbl1fmod3b:	ora16
			nextopcode
op20mod3:
lbl20mod3:	op20
			nextopcode
op21m0mod3:
lbl21mod3a:	directindexedindirect0
lbl21mod3b:	and16
			nextopcode
op22mod3:
lbl22mod3:	op22
			nextopcode
op23m0mod3:
lbl23mod3a:	stackasmrelative
lbl23mod3b:	and16
			nextopcode
op24m0mod3:
lbl24mod3a:	direct
lbl24mod3b:	bit16
			nextopcode
op25m0mod3:
lbl25mod3a:	direct
lbl25mod3b:	and16
			nextopcode
op26m0mod3:
lbl26mod3a:	direct
lbl26mod3b:	rol16
			nextopcode
op27m0mod3:
lbl27mod3a:	directindirectlong
lbl27mod3b:	and16
			nextopcode
op28mod3:
lbl28mod3:	op28x0m0
			nextopcode
@@.pool
op29m0mod3:
lbl29mod3:	op29m0
			nextopcode
op2am0mod3:
lbl2amod3a:	a_rol16
			nextopcode
op2bmod3:
lbl2bmod3:	op2b
			nextopcode
op2cm0mod3:
lbl2cmod3a:	absolute
lbl2cmod3b:	bit16
			nextopcode
op2dm0mod3:
lbl2dmod3a:	absolute
lbl2dmod3b:	and16
			nextopcode
op2em0mod3:
lbl2emod3a:	absolute
lbl2emod3b:	rol16
			nextopcode
op2fm0mod3:
lbl2fmod3a:	absolutelong
lbl2fmod3b:	and16
			nextopcode
op30mod3:
lbl30mod3:	op30
			nextopcode
op31m0mod3:
lbl31mod3a:	directindirectindexed0
lbl31mod3b:	and16
			nextopcode
op32m0mod3:
lbl32mod3a:	directindirect
lbl32mod3b:	and16
			nextopcode
op33m0mod3:
lbl33mod3a:	stackasmrelativeindirectindexed0
lbl33mod3b:	and16
			nextopcode
op34m0mod3:
lbl34mod3a:	directindexedx0
lbl34mod3b:	bit16
			nextopcode
op35m0mod3:
lbl35mod3a:	directindexedx0
lbl35mod3b:	and16
			nextopcode
op36m0mod3:
lbl36mod3a:	directindexedx0
lbl36mod3b:	rol16
			nextopcode
op37m0mod3:
lbl37mod3a:	directindirectindexedlong0
lbl37mod3b:	and16
			nextopcode
op38mod3:
lbl38mod3:	op38
			nextopcode
op39m0mod3:
lbl39mod3a:	absoluteindexedy0
lbl39mod3b:	and16
			nextopcode
op3am0mod3:
lbl3amod3a:	a_dec16
			nextopcode
op3bmod3:
lbl3bmod3:	op3bm0
			nextopcode
op3cm0mod3:
lbl3cmod3a:	absoluteindexedx0
lbl3cmod3b:	bit16
			nextopcode
op3dm0mod3:
lbl3dmod3a:	absoluteindexedx0
lbl3dmod3b:	and16
			nextopcode
op3em0mod3:
lbl3emod3a:	absoluteindexedx0
lbl3emod3b:	rol16
			nextopcode
op3fm0mod3:
lbl3fmod3a:	absolutelongindexedx0
lbl3fmod3b:	and16
			nextopcode
op40mod3:
lbl40mod3:	op40x0m0
			nextopcode
@@.pool						
op41m0mod3:
lbl41mod3a:	directindexedindirect0
lbl41mod3b:	eor16
			nextopcode
op42mod3:
lbl42mod3:	op42
			nextopcode
op43m0mod3:
lbl43mod3a:	stackasmrelative
lbl43mod3b:	eor16
			nextopcode
op44x0mod3:
lbl44mod3:	op44x0m0
			nextopcode
op45m0mod3:
lbl45mod3a:	direct
lbl45mod3b:	eor16
			nextopcode
op46m0mod3:
lbl46mod3a:	direct
lbl46mod3b:	lsr16
			nextopcode
op47m0mod3:
lbl47mod3a:	directindirectlong
lbl47mod3b:	eor16
			nextopcode
op48m0mod3:
lbl48mod3:	op48m0
			nextopcode
op49m0mod3:
lbl49mod3:	op49m0
			nextopcode
op4am0mod3:
lbl4amod3a:	a_lsr16
			nextopcode
op4bmod3:
lbl4bmod3:	op4b
			nextopcode
op4cmod3:
lbl4cmod3:	op4c
			nextopcode
op4dm0mod3:
lbl4dmod3a:	absolute
lbl4dmod3b:	eor16
			nextopcode
op4em0mod3:
lbl4emod3a:	absolute
lbl4emod3b:	lsr16
			nextopcode
op4fm0mod3:
lbl4fmod3a:	absolutelong
lbl4fmod3b:	eor16
			nextopcode
op50mod3:
lbl50mod3:	op50
			nextopcode
op51m0mod3:
lbl51mod3a:	directindirectindexed0
lbl51mod3b:	eor16
			nextopcode
op52m0mod3:
lbl52mod3a:	directindirect
lbl52mod3b:	eor16
			nextopcode
op53m0mod3:
lbl53mod3a:	stackasmrelativeindirectindexed0
lbl53mod3b:	eor16
			nextopcode
op54x0mod3:
lbl54mod3:	op54x0m0
			nextopcode
op55m0mod3:
lbl55mod3a:	directindexedx0
lbl55mod3b:	eor16
			nextopcode
op56m0mod3:
lbl56mod3a:	directindexedx0
lbl56mod3b:	lsr16
			nextopcode
op57m0mod3:
lbl57mod3a:	directindirectindexedlong0
lbl57mod3b:	eor16
			nextopcode
op58mod3:
lbl58mod3:	op58
			nextopcode
op59m0mod3:
lbl59mod3a:	absoluteindexedy0
lbl59mod3b:	eor16
			nextopcode
op5ax0mod3:
lbl5amod3:	op5ax0
			nextopcode
op5bmod3:
lbl5bmod3:	op5bm0
			nextopcode
op5cmod3:
lbl5cmod3:	op5c
			nextopcode
op5dm0mod3:
lbl5dmod3a:	absoluteindexedx0
lbl5dmod3b:	eor16
			nextopcode
op5em0mod3:
lbl5emod3a:	absoluteindexedx0
lbl5emod3b:	lsr16
			nextopcode
op5fm0mod3:
lbl5fmod3a:	absolutelongindexedx0
lbl5fmod3b:	eor16
			nextopcode
op60mod3:
lbl60mod3:	op60
			nextopcode
op61m0mod3:
lbl61mod3a:	directindexedindirect0
lbl61mod3b:	adc16
			nextopcode
op62mod3:
lbl62mod3:	op62
			nextopcode
op63m0mod3:
lbl63mod3a:	stackasmrelative
lbl63mod3b:	adc16
			nextopcode
@@.pool			
op64m0mod3:
lbl64mod3a:	direct
lbl64mod3b:	stz16
			nextopcode
op65m0mod3:
lbl65mod3a:	direct
lbl65mod3b:	adc16
			nextopcode
@@.pool			
op66m0mod3:
lbl66mod3a:	direct
lbl66mod3b:	ror16
			nextopcode
op67m0mod3:
lbl67mod3a:	directindirectlong
lbl67mod3b:	adc16
			nextopcode
@@.pool			
op68m0mod3:
lbl68mod3:	op68m0
			nextopcode
op69m0mod3:
lbl69mod3a:	immediate16
lbl69mod3b:	adc16
			nextopcode
@.pool			
op6am0mod3:
lbl6amod3a:	a_ror16
			nextopcode
op6bmod3:
lbl6bmod3:	op6b
			nextopcode
op6cmod3:
lbl6cmod3:	op6c
			nextopcode
op6dm0mod3:
lbl6dmod3a:	absolute
lbl6dmod3b:	adc16
			nextopcode
op6em0mod3:
lbl6emod3a:	absolute
lbl6emod3b:	ror16
			nextopcode
op6fm0mod3:
lbl6fmod3a:	absolutelong
lbl6fmod3b:	adc16
			nextopcode
op70mod3:
lbl70mod3:	op70
			nextopcode
op71m0mod3:
lbl71mod3a:	directindirectindexed0
lbl71mod3b:	adc16
			nextopcode
op72m0mod3:
lbl72mod3a:	directindirect
lbl72mod3b:	adc16
			nextopcode
op73m0mod3:
lbl73mod3a:	stackasmrelativeindirectindexed0
lbl73mod3b:	adc16
			nextopcode
@.pool
op74m0mod3:
lbl74mod3a:	directindexedx0
lbl74mod3b:	stz16
			nextopcode
op75m0mod3:
lbl75mod3a:	directindexedx0
lbl75mod3b:	adc16
			nextopcode
@.pool
op76m0mod3:
lbl76mod3a:	directindexedx0
lbl76mod3b:	ror16
			nextopcode
op77m0mod3:
lbl77mod3a:	directindirectindexedlong0
lbl77mod3b:	adc16
			nextopcode
op78mod3:
lbl78mod3:	op78
			nextopcode
op79m0mod3:
lbl79mod3a:	absoluteindexedy0
lbl79mod3b:	adc16
			nextopcode
op7ax0mod3:
lbl7amod3:	op7ax0
			nextopcode
op7bmod3:
lbl7bmod3:	op7bm0
			nextopcode
op7cmod3:
lbl7cmod3:	absoluteindexedindirectx0
		op7c
			nextopcode
op7dm0mod3:
lbl7dmod3a:	absoluteindexedx0
lbl7dmod3b:	adc16
			nextopcode
op7em0mod3:
lbl7emod3a:	absoluteindexedx0
lbl7emod3b:	ror16
			nextopcode
op7fm0mod3:
lbl7fmod3a:	absolutelongindexedx0
lbl7fmod3b:	adc16
			nextopcode
@.pool			
op80mod3:
lbl80mod3:	op80
			nextopcode
op81m0mod3:
lbl81mod3a:	directindexedindirect0
lbl81mod3b:	op81m0
			nextopcode
op82mod3:
lbl82mod3:	op82
			nextopcode
op83m0mod3:
lbl83mod3a:	stackasmrelative
lbl83mod3b:	sta16
			nextopcode
op84x0mod3:
lbl84mod3a:	direct
lbl84mod3b:	sty16
			nextopcode
op85m0mod3:
lbl85mod3a:	direct
lbl85mod3b:	sta16
			nextopcode
op86x0mod3:
lbl86mod3a:	direct
lbl86mod3b:	stx16
			nextopcode
op87m0mod3:
lbl87mod3a:	directindirectlong
lbl87mod3b:	sta16
			nextopcode
op88x0mod3:
lbl88mod3:	op88x0
			nextopcode
op89m0mod3:
lbl89mod3:	op89m0
			nextopcode
op8am0mod3:
lbl8amod3:	op8am0x0
			nextopcode
op8bmod3:
lbl8bmod3:	op8b
			nextopcode
op8cx0mod3:
lbl8cmod3a:	absolute
lbl8cmod3b:	sty16
			nextopcode
op8dm0mod3:
lbl8dmod3a:	absolute
lbl8dmod3b:	sta16
			nextopcode
op8ex0mod3:
lbl8emod3a:	absolute
lbl8emod3b:	stx16
			nextopcode
op8fm0mod3:
lbl8fmod3a:	absolutelong
lbl8fmod3b:	sta16
			nextopcode
op90mod3:
lbl90mod3:	op90
			nextopcode
op91m0mod3:
lbl91mod3a:	directindirectindexed0
lbl91mod3b:	sta16
			nextopcode
op92m0mod3:
lbl92mod3a:	directindirect
lbl92mod3b:	sta16
			nextopcode
op93m0mod3:
lbl93mod3a:	stackasmrelativeindirectindexed0
lbl93mod3b:	sta16
			nextopcode
op94x0mod3:
lbl94mod3a:	directindexedx0
lbl94mod3b:	sty16
			nextopcode
op95m0mod3:
lbl95mod3a:	directindexedx0
lbl95mod3b:	sta16
			nextopcode
op96x0mod3:
lbl96mod3a:	directindexedy0
lbl96mod3b:	stx16
			nextopcode
op97m0mod3:
lbl97mod3a:	directindirectindexedlong0
lbl97mod3b:	sta16
			nextopcode
op98m0mod3:
lbl98mod3:	op98m0x0
			nextopcode
op99m0mod3:
lbl99mod3a:	absoluteindexedy0
lbl99mod3b:	sta16
			nextopcode
op9amod3:
lbl9amod3:	op9ax0
			nextopcode
op9bx0mod3:
lbl9bmod3:	op9bx0
			nextopcode
op9cm0mod3:
lbl9cmod3a:	absolute
lbl9cmod3b:	stz16
			nextopcode
op9dm0mod3:
lbl9dmod3a:	absoluteindexedx0
lbl9dmod3b:	sta16
			nextopcode
op9em0mod3:	
lbl9emod3:	absoluteindexedx0		
		stz16
			nextopcode
op9fm0mod3:
lbl9fmod3a:	absolutelongindexedx0
lbl9fmod3b:	sta16
			nextopcode
opa0x0mod3:
lbla0mod3:	opa0x0
			nextopcode
opa1m0mod3:
lbla1mod3a:	directindexedindirect0
lbla1mod3b:	lda16
			nextopcode
opa2x0mod3:
lbla2mod3:	opa2x0
			nextopcode
opa3m0mod3:
lbla3mod3a:	stackasmrelative
lbla3mod3b:	lda16
			nextopcode
opa4x0mod3:
lbla4mod3a:	direct
lbla4mod3b:	ldy16
			nextopcode
opa5m0mod3:
lbla5mod3a:	direct
lbla5mod3b:	lda16
			nextopcode
opa6x0mod3:
lbla6mod3a:	direct
lbla6mod3b:	ldx16
			nextopcode
opa7m0mod3:
lbla7mod3a:	directindirectlong
lbla7mod3b:	lda16
			nextopcode
opa8x0mod3:
lbla8mod3:	opa8x0m0
			nextopcode
opa9m0mod3:
lbla9mod3:	opa9m0
			nextopcode
opaax0mod3:
lblaamod3:	opaax0m0
			nextopcode
opabmod3:
lblabmod3:	opab
			nextopcode
opacx0mod3:
lblacmod3a:	absolute
lblacmod3b:	ldy16
			nextopcode
opadm0mod3:
lbladmod3a:	absolute
lbladmod3b:	lda16
			nextopcode
opaex0mod3:
lblaemod3a:	absolute
lblaemod3b:	ldx16
			nextopcode
opafm0mod3:
lblafmod3a:	absolutelong
lblafmod3b:	lda16
			nextopcode
opb0mod3:
lblb0mod3:	opb0
			nextopcode
opb1m0mod3:
lblb1mod3a:	directindirectindexed0
lblb1mod3b:	lda16
			nextopcode
opb2m0mod3:
lblb2mod3a:	directindirect
lblb2mod3b:	lda16
			nextopcode
opb3m0mod3:
lblb3mod3a:	stackasmrelativeindirectindexed0
lblb3mod3b:	lda16
			nextopcode
opb4x0mod3:
lblb4mod3a:	directindexedx0
lblb4mod3b:	ldy16
			nextopcode
opb5m0mod3:
lblb5mod3a:	directindexedx0
lblb5mod3b:	lda16
			nextopcode
opb6x0mod3:
lblb6mod3a:	directindexedy0
lblb6mod3b:	ldx16
			nextopcode
opb7m0mod3:
lblb7mod3a:	directindirectindexedlong0
lblb7mod3b:	lda16
			nextopcode
opb8mod3:
lblb8mod3:	opb8
			nextopcode
opb9m0mod3:
lblb9mod3a:	absoluteindexedy0
lblb9mod3b:	lda16
			nextopcode
opbax0mod3:
lblbamod3:	opbax0
			nextopcode
opbbx0mod3:
lblbbmod3:	opbbx0
			nextopcode
opbcx0mod3:
lblbcmod3a:	absoluteindexedx0
lblbcmod3b:	ldy16
			nextopcode
opbdm0mod3:
lblbdmod3a:	absoluteindexedx0
lblbdmod3b:	lda16
			nextopcode
opbex0mod3:
lblbemod3a:	absoluteindexedy0
lblbemod3b:	ldx16
			nextopcode
opbfm0mod3:
lblbfmod3a:	absolutelongindexedx0
lblbfmod3b:	lda16
			nextopcode
opc0x0mod3:
lblc0mod3:	opc0x0
			nextopcode
opc1m0mod3:
lblc1mod3a:	directindexedindirect0
lblc1mod3b:	cmp16
			nextopcode
opc2mod3:
lblc2mod3:	opc2
			nextopcode
@.pool
opc3m0mod3:
lblc3mod3a:	stackasmrelative
lblc3mod3b:	cmp16
			nextopcode
opc4x0mod3:
lblc4mod3a:	direct
lblc4mod3b:	cmy16
			nextopcode
opc5m0mod3:
lblc5mod3a:	direct
lblc5mod3b:	cmp16
			nextopcode
opc6m0mod3:
lblc6mod3a:	direct
lblc6mod3b:	dec16
			nextopcode
opc7m0mod3:
lblc7mod3a:	directindirectlong
lblc7mod3b:	cmp16
			nextopcode
opc8x0mod3:
lblc8mod3:	opc8x0
			nextopcode
opc9m0mod3:
lblc9mod3:	opc9m0
			nextopcode
opcax0mod3:
lblcamod3:	opcax0
			nextopcode
opcbmod3:
lblcbmod3:	opcb
			nextopcode
opccx0mod3:
lblccmod3a:	absolute
lblccmod3b:	cmy16
			nextopcode
opcdm0mod3:
lblcdmod3a:	absolute
lblcdmod3b:	cmp16
			nextopcode
opcem0mod3:
lblcemod3a:	absolute
lblcemod3b:	dec16
			nextopcode
opcfm0mod3:
lblcfmod3a:	absolutelong
lblcfmod3b:	cmp16
			nextopcode
opd0mod3:
lbld0mod3:	opd0
			nextopcode
opd1m0mod3:
lbld1mod3a:	directindirectindexed0
lbld1mod3b:	cmp16
			nextopcode
opd2m0mod3:
lbld2mod3a:	directindirect
lbld2mod3b:	cmp16
			nextopcode
opd3m0mod3:
lbld3mod3a:	stackasmrelativeindirectindexed0
lbld3mod3b:	cmp16
			nextopcode
opd4mod3:
lbld4mod3:	opd4
			nextopcode
opd5m0mod3:
lbld5mod3a:	directindexedx0
lbld5mod3b:	cmp16
			nextopcode
opd6m0mod3:
lbld6mod3a:	directindexedx0
lbld6mod3b:	dec16
			nextopcode
opd7m0mod3:
lbld7mod3a:	directindirectindexedlong0
lbld7mod3b:	cmp16
			nextopcode
opd8mod3:
lbld8mod3:	opd8
			nextopcode
opd9m0mod3:
lbld9mod3a:	absoluteindexedy0
lbld9mod3b:	cmp16
			nextopcode
opdax0mod3:
lbldamod3:	opdax0
			nextopcode
opdbmod3:
lbldbmod3:	opdb
			nextopcode
opdcmod3:
lbldcmod3:	opdc
			nextopcode
opddm0mod3:
lblddmod3a:	absoluteindexedx0
lblddmod3b:	cmp16
			nextopcode
opdem0mod3:
lbldemod3a:	absoluteindexedx0
lbldemod3b:	dec16
			nextopcode
opdfm0mod3:
lbldfmod3a:	absolutelongindexedx0
lbldfmod3b:	cmp16
			nextopcode
ope0x0mod3:
lble0mod3:	ope0x0
			nextopcode
ope1m0mod3:
lble1mod3a:	directindexedindirect0
lble1mod3b:	sbc16
			nextopcode
ope2mod3:
lble2mod3:	ope2
			nextopcode
@.pool
ope3m0mod3:
lble3mod3a:	stackasmrelative
lble3mod3b:	sbc16
			nextopcode
ope4x0mod3:
lble4mod3a:	direct
lble4mod3b:	cmx16
			nextopcode
ope5m0mod3:
lble5mod3a:	direct
lble5mod3b:	sbc16
			nextopcode
ope6m0mod3:
lble6mod3a:	direct
lble6mod3b:	inc16
			nextopcode
ope7m0mod3:
lble7mod3a:	directindirectlong
lble7mod3b:	sbc16
			nextopcode
ope8x0mod3:
lble8mod3:	ope8x0
			nextopcode
ope9m0mod3:
lble9mod3a:	immediate16
lble9mod3b:	sbc16
			nextopcode
opeamod3:
lbleamod3:	opea
			nextopcode
opebmod3:
lblebmod3:	opebm0
			nextopcode
opecx0mod3:
lblecmod3a:	absolute
lblecmod3b:	cmx16
			nextopcode
opedm0mod3:
lbledmod3a:	absolute
lbledmod3b:	sbc16
			nextopcode
opeem0mod3:
lbleemod3a:	absolute
lbleemod3b:	inc16
			nextopcode
opefm0mod3:
lblefmod3a:	absolutelong
lblefmod3b:	sbc16
			nextopcode
opf0mod3:
lblf0mod3:	opf0
			nextopcode
opf1m0mod3:
lblf1mod3a:	directindirectindexed0
lblf1mod3b:	sbc16
			nextopcode
opf2m0mod3:
lblf2mod3a:	directindirect
lblf2mod3b:	sbc16
			nextopcode
opf3m0mod3:
lblf3mod3a:	stackasmrelativeindirectindexed0
lblf3mod3b:	sbc16
			nextopcode
opf4mod3:
lblf4mod3:	opf4
			nextopcode
opf5m0mod3:
lblf5mod3a:	directindexedx0
lblf5mod3b:	sbc16
			nextopcode
opf6m0mod3:
lblf6mod3a:	directindexedx0
lblf6mod3b:	inc16
			nextopcode
opf7m0mod3:
lblf7mod3a:	directindirectindexedlong0
lblf7mod3b:	sbc16
			nextopcode
opf8mod3:
lblf8mod3:	opf8
			nextopcode
opf9m0mod3:
lblf9mod3a:	absoluteindexedy0
lblf9mod3b:	sbc16
			nextopcode
opfax0mod3:
lblfamod3:	opfax0
			nextopcode
opfbmod3:
lblfbmod3:	opfb
			nextopcode
opfcmod3:
lblfcmod3:	opfcx0
			nextopcode
opfdm0mod3:
lblfdmod3a:	absoluteindexedx0
lblfdmod3b:	sbc16
			nextopcode
opfem0mod3:
lblfemod3a:	absoluteindexedx0
lblfemod3b:	inc16
			nextopcode
opffm0mod3:
lblffmod3a:	absolutelongindexedx0
lblffmod3b:	sbc16
			nextopcode
@.pool

jumptable4:		.long	op00mod4
			.long	op01m0mod4
			.long	op02mod4
			.long	op03m0mod4
			.long	op04m0mod4
			.long	op05m0mod4
			.long	op06m0mod4
			.long	op07m0mod4
			.long	op08mod4
			.long	op09m0mod4
			.long	op0am0mod4
			.long	op0bmod4
			.long	op0cm0mod4
			.long	op0dm0mod4
			.long	op0em0mod4
			.long	op0fm0mod4
			.long	op10mod4
			.long	op11m0mod4
			.long	op12m0mod4
			.long	op13m0mod4
			.long	op14m0mod4
			.long	op15m0mod4
			.long	op16m0mod4
			.long	op17m0mod4
			.long	op18mod4
			.long	op19m0mod4
			.long	op1am0mod4
			.long	op1bmod4
			.long	op1cm0mod4
			.long	op1dm0mod4
			.long	op1em0mod4
			.long	op1fm0mod4
			.long	op20mod4
			.long	op21m0mod4
			.long	op22mod4
			.long	op23m0mod4
			.long	op24m0mod4
			.long	op25m0mod4
			.long	op26m0mod4
			.long	op27m0mod4
			.long	op28mod4
			.long	op29m0mod4
			.long	op2am0mod4
			.long	op2bmod4
			.long	op2cm0mod4
			.long	op2dm0mod4
			.long	op2em0mod4
			.long	op2fm0mod4
			.long	op30mod4
			.long	op31m0mod4
			.long	op32m0mod4
			.long	op33m0mod4
			.long	op34m0mod4
			.long	op35m0mod4
			.long	op36m0mod4
			.long	op37m0mod4
			.long	op38mod4
			.long	op39m0mod4
			.long	op3am0mod4
			.long	op3bmod4
			.long	op3cm0mod4
			.long	op3dm0mod4
			.long	op3em0mod4
			.long	op3fm0mod4
			.long	op40mod4
			.long	op41m0mod4
			.long	op42mod4
			.long	op43m0mod4
			.long	op44x1mod4
			.long	op45m0mod4
			.long	op46m0mod4
			.long	op47m0mod4
			.long	op48m0mod4
			.long	op49m0mod4
			.long	op4am0mod4
			.long	op4bmod4
			.long	op4cmod4
			.long	op4dm0mod4
			.long	op4em0mod4
			.long	op4fm0mod4
			.long	op50mod4
			.long	op51m0mod4
			.long	op52m0mod4
			.long	op53m0mod4
			.long	op54x1mod4
			.long	op55m0mod4
			.long	op56m0mod4
			.long	op57m0mod4
			.long	op58mod4
			.long	op59m0mod4
			.long	op5ax1mod4
			.long	op5bmod4
			.long	op5cmod4
			.long	op5dm0mod4
			.long	op5em0mod4
			.long	op5fm0mod4
			.long	op60mod4
			.long	op61m0mod4
			.long	op62mod4
			.long	op63m0mod4
			.long	op64m0mod4
			.long	op65m0mod4
			.long	op66m0mod4
			.long	op67m0mod4
			.long	op68m0mod4
			.long	op69m0mod4
			.long	op6am0mod4
			.long	op6bmod4
			.long	op6cmod4
			.long	op6dm0mod4
			.long	op6em0mod4
			.long	op6fm0mod4
			.long	op70mod4
			.long	op71m0mod4
			.long	op72m0mod4
			.long	op73m0mod4
			.long	op74m0mod4
			.long	op75m0mod4
			.long	op76m0mod4
			.long	op77m0mod4
			.long	op78mod4
			.long	op79m0mod4
			.long	op7ax1mod4
			.long	op7bmod4
			.long	op7cmod4
			.long	op7dm0mod4
			.long	op7em0mod4
			.long	op7fm0mod4
			.long	op80mod4
			.long	op81m0mod4
			.long	op82mod4
			.long	op83m0mod4
			.long	op84x1mod4
			.long	op85m0mod4
			.long	op86x1mod4
			.long	op87m0mod4
			.long	op88x1mod4
			.long	op89m0mod4
			.long	op8am0mod4
			.long	op8bmod4
			.long	op8cx1mod4
			.long	op8dm0mod4
			.long	op8ex1mod4
			.long	op8fm0mod4
			.long	op90mod4
			.long	op91m0mod4
			.long	op92m0mod4
			.long	op93m0mod4
			.long	op94x1mod4
			.long	op95m0mod4
			.long	op96x1mod4
			.long	op97m0mod4
			.long	op98m0mod4
			.long	op99m0mod4
			.long	op9amod4
			.long	op9bx1mod4
			.long	op9cm0mod4
			.long	op9dm0mod4
			.long	op9em0mod4
			.long	op9fm0mod4
			.long	opa0x1mod4
			.long	opa1m0mod4
			.long	opa2x1mod4
			.long	opa3m0mod4
			.long	opa4x1mod4
			.long	opa5m0mod4
			.long	opa6x1mod4
			.long	opa7m0mod4
			.long	opa8x1mod4
			.long	opa9m0mod4
			.long	opaax1mod4
			.long	opabmod4
			.long	opacx1mod4
			.long	opadm0mod4
			.long	opaex1mod4
			.long	opafm0mod4
			.long	opb0mod4
			.long	opb1m0mod4
			.long	opb2m0mod4
			.long	opb3m0mod4
			.long	opb4x1mod4
			.long	opb5m0mod4
			.long	opb6x1mod4
			.long	opb7m0mod4
			.long	opb8mod4
			.long	opb9m0mod4
			.long	opbax1mod4
			.long	opbbx1mod4
			.long	opbcx1mod4
			.long	opbdm0mod4
			.long	opbex1mod4
			.long	opbfm0mod4
			.long	opc0x1mod4
			.long	opc1m0mod4
			.long	opc2mod4
			.long	opc3m0mod4
			.long	opc4x1mod4
			.long	opc5m0mod4
			.long	opc6m0mod4
			.long	opc7m0mod4
			.long	opc8x1mod4
			.long	opc9m0mod4
			.long	opcax1mod4
			.long	opcbmod4
			.long	opccx1mod4
			.long	opcdm0mod4
			.long	opcem0mod4
			.long	opcfm0mod4
			.long	opd0mod4
			.long	opd1m0mod4
			.long	opd2m0mod4
			.long	opd3m0mod4
			.long	opd4mod4
			.long	opd5m0mod4
			.long	opd6m0mod4
			.long	opd7m0mod4
			.long	opd8mod4
			.long	opd9m0mod4
			.long	opdax1mod4
			.long	opdbmod4
			.long	opdcmod4
			.long	opddm0mod4
			.long	opdem0mod4
			.long	opdfm0mod4
			.long	ope0x1mod4
			.long	ope1m0mod4
			.long	ope2mod4
			.long	ope3m0mod4
			.long	ope4x1mod4
			.long	ope5m0mod4
			.long	ope6m0mod4
			.long	ope7m0mod4
			.long	ope8x1mod4
			.long	ope9m0mod4
			.long	opeamod4
			.long	opebmod4
			.long	opecx1mod4
			.long	opedm0mod4
			.long	opeem0mod4
			.long	opefm0mod4
			.long	opf0mod4
			.long	opf1m0mod4
			.long	opf2m0mod4
			.long	opf3m0mod4
			.long	opf4mod4
			.long	opf5m0mod4
			.long	opf6m0mod4
			.long	opf7m0mod4
			.long	opf8mod4
			.long	opf9m0mod4
			.long	opfax1mod4
			.long	opfbmod4
			.long	opfcmod4
			.long	opfdm0mod4
			.long	opfem0mod4
			.long	opffm0mod4
op00mod4:
lbl00mod4:	op00
			nextopcode
op01m0mod4:
lbl01mod4a:	directindexedindirect1
lbl01mod4b:	ora16
			nextopcode
op02mod4:
lbl02mod4:	op02
			nextopcode
op03m0mod4:
lbl03mod4a:	stackasmrelative
lbl03mod4b:	ora16
			nextopcode
op04m0mod4:
lbl04mod4a:	direct
lbl04mod4b:	tsb16
			nextopcode
op05m0mod4:
lbl05mod4a:	direct
lbl05mod4b:	ora16
			nextopcode
op06m0mod4:
lbl06mod4a:	direct
lbl06mod4b:	asl16
			nextopcode
op07m0mod4:
lbl07mod4a:	directindirectlong
lbl07mod4b:	ora16
			nextopcode
op08mod4:
lbl08mod4:	op08
			nextopcode
op09m0mod4:
lbl09mod4:	op09m0
			nextopcode
op0am0mod4:
lbl0amod4a:	a_asl16
			nextopcode
op0bmod4:
lbl0bmod4:	op0b
			nextopcode
op0cm0mod4:
lbl0cmod4a:	absolute
lbl0cmod4b:	tsb16
			nextopcode
op0dm0mod4:
lbl0dmod4a:	absolute
lbl0dmod4b:	ora16
			nextopcode
op0em0mod4:
lbl0emod4a:	absolute
lbl0emod4b:	asl16
			nextopcode
op0fm0mod4:
lbl0fmod4a:	absolutelong
lbl0fmod4b:	ora16
			nextopcode
op10mod4:
lbl10mod4:	op10
			nextopcode
op11m0mod4:
lbl11mod4a:	directindirectindexed1
lbl11mod4b:	ora16
			nextopcode
op12m0mod4:
lbl12mod4a:	directindirect
lbl12mod4b:	ora16
			nextopcode
op13m0mod4:
lbl13mod4a:	stackasmrelativeindirectindexed1
lbl13mod4b:	ora16
			nextopcode
op14m0mod4:
lbl14mod4a:	direct
lbl14mod4b:	trb16
			nextopcode
op15m0mod4:
lbl15mod4a:	directindexedx1
lbl15mod4b:	ora16
			nextopcode
op16m0mod4:
lbl16mod4a:	directindexedx1
lbl16mod4b:	asl16
			nextopcode
op17m0mod4:
lbl17mod4a:	directindirectindexedlong1
lbl17mod4b:	ora16
			nextopcode
op18mod4:
lbl18mod4:	op18
			nextopcode
op19m0mod4:
lbl19mod4a:	absoluteindexedy1
lbl19mod4b:	ora16
			nextopcode
op1am0mod4:
lbl1amod4a:	a_inc16
			nextopcode
op1bmod4:
lbl1bmod4:	op1bm0
			nextopcode
op1cm0mod4:
lbl1cmod4a:	absolute
lbl1cmod4b:	trb16
			nextopcode
op1dm0mod4:
lbl1dmod4a:	absoluteindexedx1
lbl1dmod4b:	ora16
			nextopcode
op1em0mod4:
lbl1emod4a:	absoluteindexedx1
lbl1emod4b:	asl16
			nextopcode
op1fm0mod4:
lbl1fmod4a:	absolutelongindexedx1
lbl1fmod4b:	ora16
			nextopcode
op20mod4:
lbl20mod4:	op20
			nextopcode
op21m0mod4:
lbl21mod4a:	directindexedindirect1
lbl21mod4b:	and16
			nextopcode
op22mod4:
lbl22mod4:	op22
			nextopcode
op23m0mod4:
lbl23mod4a:	stackasmrelative
lbl23mod4b:	and16
			nextopcode
op24m0mod4:
lbl24mod4a:	direct
lbl24mod4b:	bit16
			nextopcode
op25m0mod4:
lbl25mod4a:	direct
lbl25mod4b:	and16
			nextopcode
op26m0mod4:
lbl26mod4a:	direct
lbl26mod4b:	rol16
			nextopcode
op27m0mod4:
lbl27mod4a:	directindirectlong
lbl27mod4b:	and16
			nextopcode
op28mod4:
lbl28mod4:	op28x1m0
			nextopcode
@.pool
op29m0mod4:
lbl29mod4:	op29m0
			nextopcode
op2am0mod4:
lbl2amod4a:	a_rol16
			nextopcode
op2bmod4:
lbl2bmod4:	op2b
			nextopcode
op2cm0mod4:
lbl2cmod4a:	absolute
lbl2cmod4b:	bit16
			nextopcode
op2dm0mod4:
lbl2dmod4a:	absolute
lbl2dmod4b:	and16
			nextopcode
op2em0mod4:
lbl2emod4a:	absolute
lbl2emod4b:	rol16
			nextopcode
op2fm0mod4:
lbl2fmod4a:	absolutelong
lbl2fmod4b:	and16
			nextopcode
op30mod4:
lbl30mod4:	op30
			nextopcode
op31m0mod4:
lbl31mod4a:	directindirectindexed1
lbl31mod4b:	and16
			nextopcode
op32m0mod4:
lbl32mod4a:	directindirect
lbl32mod4b:	and16
			nextopcode
op33m0mod4:
lbl33mod4a:	stackasmrelativeindirectindexed1
lbl33mod4b:	and16
			nextopcode
op34m0mod4:
lbl34mod4a:	directindexedx1
lbl34mod4b:	bit16
			nextopcode
op35m0mod4:
lbl35mod4a:	directindexedx1
lbl35mod4b:	and16
			nextopcode
op36m0mod4:
lbl36mod4a:	directindexedx1
lbl36mod4b:	rol16
			nextopcode
op37m0mod4:
lbl37mod4a:	directindirectindexedlong1
lbl37mod4b:	and16
			nextopcode
op38mod4:
lbl38mod4:	op38
			nextopcode
op39m0mod4:
lbl39mod4a:	absoluteindexedy1
lbl39mod4b:	and16
			nextopcode
op3am0mod4:
lbl3amod4a:	a_dec16
			nextopcode
op3bmod4:
lbl3bmod4:	op3bm0
			nextopcode
op3cm0mod4:
lbl3cmod4a:	absoluteindexedx1
lbl3cmod4b:	bit16
			nextopcode
op3dm0mod4:
lbl3dmod4a:	absoluteindexedx1
lbl3dmod4b:	and16
			nextopcode
op3em0mod4:
lbl3emod4a:	absoluteindexedx1
lbl3emod4b:	rol16
			nextopcode
op3fm0mod4:
lbl3fmod4a:	absolutelongindexedx1
lbl3fmod4b:	and16
			nextopcode
op40mod4:
lbl40mod4:	op40x1m0
			nextopcode
@.pool						
op41m0mod4:
lbl41mod4a:	directindexedindirect1
lbl41mod4b:	eor16
			nextopcode
op42mod4:
lbl42mod4:	op42
			nextopcode
op43m0mod4:
lbl43mod4a:	stackasmrelative
lbl43mod4b:	eor16
			nextopcode
op44x1mod4:
lbl44mod4:	op44x1m0
			nextopcode
op45m0mod4:
lbl45mod4a:	direct
lbl45mod4b:	eor16
			nextopcode
op46m0mod4:
lbl46mod4a:	direct
lbl46mod4b:	lsr16
			nextopcode
op47m0mod4:
lbl47mod4a:	directindirectlong
lbl47mod4b:	eor16
			nextopcode
op48m0mod4:
lbl48mod4:	op48m0
			nextopcode
op49m0mod4:
lbl49mod4:	op49m0
			nextopcode
op4am0mod4:
lbl4amod4a:	a_lsr16
			nextopcode
op4bmod4:
lbl4bmod4:	op4b
			nextopcode
op4cmod4:
lbl4cmod4:	op4c
			nextopcode
op4dm0mod4:
lbl4dmod4a:	absolute
lbl4dmod4b:	eor16
			nextopcode
op4em0mod4:
lbl4emod4a:	absolute
lbl4emod4b:	lsr16
			nextopcode
op4fm0mod4:
lbl4fmod4a:	absolutelong
lbl4fmod4b:	eor16
			nextopcode
op50mod4:
lbl50mod4:	op50
			nextopcode
op51m0mod4:
lbl51mod4a:	directindirectindexed1
lbl51mod4b:	eor16
			nextopcode
op52m0mod4:
lbl52mod4a:	directindirect
lbl52mod4b:	eor16
			nextopcode
op53m0mod4:
lbl53mod4a:	stackasmrelativeindirectindexed1
lbl53mod4b:	eor16
			nextopcode
op54x1mod4:
lbl54mod4:	op54x1m0
			nextopcode
op55m0mod4:
lbl55mod4a:	directindexedx1
lbl55mod4b:	eor16
			nextopcode
op56m0mod4:
lbl56mod4a:	directindexedx1
lbl56mod4b:	lsr16
			nextopcode
op57m0mod4:
lbl57mod4a:	directindirectindexedlong1
lbl57mod4b:	eor16
			nextopcode
op58mod4:
lbl58mod4:	op58
			nextopcode
op59m0mod4:
lbl59mod4a:	absoluteindexedy1
lbl59mod4b:	eor16
			nextopcode
op5ax1mod4:
lbl5amod4:	op5ax1
			nextopcode
op5bmod4:
lbl5bmod4:	op5bm0
			nextopcode
op5cmod4:
lbl5cmod4:	op5c
			nextopcode
op5dm0mod4:
lbl5dmod4a:	absoluteindexedx1
lbl5dmod4b:	eor16
			nextopcode
op5em0mod4:
lbl5emod4a:	absoluteindexedx1
lbl5emod4b:	lsr16
			nextopcode
op5fm0mod4:
lbl5fmod4a:	absolutelongindexedx1
lbl5fmod4b:	eor16
			nextopcode
op60mod4:
lbl60mod4:	op60
			nextopcode
op61m0mod4:
lbl61mod4a:	directindexedindirect1
lbl61mod4b:	adc16
			nextopcode
op62mod4:
lbl62mod4:	op62
			nextopcode
op63m0mod4:
lbl63mod4a:	stackasmrelative
lbl63mod4b:	adc16
			nextopcode
@.pool			
op64m0mod4:
lbl64mod4a:	direct
lbl64mod4b:	stz16
			nextopcode
op65m0mod4:
lbl65mod4a:	direct
lbl65mod4b:	adc16
			nextopcode
@.pool			
op66m0mod4:
lbl66mod4a:	direct
lbl66mod4b:	ror16
			nextopcode
op67m0mod4:
lbl67mod4a:	directindirectlong
lbl67mod4b:	adc16
			nextopcode
@.pool			
op68m0mod4:
lbl68mod4:	op68m0
			nextopcode
op69m0mod4:
lbl69mod4a:	immediate16
lbl69mod4b:	adc16
			nextopcode
@.pool			
op6am0mod4:
lbl6amod4a:	a_ror16
			nextopcode
op6bmod4:
lbl6bmod4:	op6b
			nextopcode
op6cmod4:
lbl6cmod4:	op6c
			nextopcode
op6dm0mod4:
lbl6dmod4a:	absolute
lbl6dmod4b:	adc16
			nextopcode
op6em0mod4:
lbl6emod4a:	absolute
lbl6emod4b:	ror16
			nextopcode
op6fm0mod4:
lbl6fmod4a:	absolutelong
lbl6fmod4b:	adc16
			nextopcode
op70mod4:
lbl70mod4:	op70
			nextopcode
op71m0mod4:
lbl71mod4a:	directindirectindexed1
lbl71mod4b:	adc16
			nextopcode
op72m0mod4:
lbl72mod4a:	directindirect
lbl72mod4b:	adc16
			nextopcode
op73m0mod4:
lbl73mod4a:	stackasmrelativeindirectindexed1
lbl73mod4b:	adc16
			nextopcode
@.pool
op74m0mod4:
lbl74mod4a:	directindexedx1
lbl74mod4b:	stz16
			nextopcode
op75m0mod4:
lbl75mod4a:	directindexedx1
lbl75mod4b:	adc16
			nextopcode
@.pool
op76m0mod4:
lbl76mod4a:	directindexedx1
lbl76mod4b:	ror16
			nextopcode
op77m0mod4:
lbl77mod4a:	directindirectindexedlong1
lbl77mod4b:	adc16
			nextopcode
op78mod4:
lbl78mod4:	op78
			nextopcode
op79m0mod4:
lbl79mod4a:	absoluteindexedy1
lbl79mod4b:	adc16
			nextopcode
op7ax1mod4:
lbl7amod4:	op7ax1
			nextopcode
op7bmod4:
lbl7bmod4:	op7bm0
			nextopcode
op7cmod4:
lbl7cmod4:	absoluteindexedindirectx1
		op7c
			nextopcode
op7dm0mod4:
lbl7dmod4a:	absoluteindexedx1
lbl7dmod4b:	adc16
			nextopcode
op7em0mod4:
lbl7emod4a:	absoluteindexedx1
lbl7emod4b:	ror16
			nextopcode
op7fm0mod4:
lbl7fmod4a:	absolutelongindexedx1
lbl7fmod4b:	adc16
			nextopcode
@.pool			
op80mod4:
lbl80mod4:	op80
			nextopcode
op81m0mod4:
lbl81mod4a:	directindexedindirect1
lbl81mod4b:	op81m0
			nextopcode
op82mod4:
lbl82mod4:	op82
			nextopcode
op83m0mod4:
lbl83mod4a:	stackasmrelative
lbl83mod4b:	sta16
			nextopcode
op84x1mod4:
lbl84mod4a:	direct
lbl84mod4b:	sty8
			nextopcode
op85m0mod4:
lbl85mod4a:	direct
lbl85mod4b:	sta16
			nextopcode
op86x1mod4:
lbl86mod4a:	direct
lbl86mod4b:	stx8
			nextopcode
op87m0mod4:
lbl87mod4a:	directindirectlong
lbl87mod4b:	sta16
			nextopcode
op88x1mod4:
lbl88mod4:	op88x1
			nextopcode
op89m0mod4:
lbl89mod4:	op89m0
			nextopcode
op8am0mod4:
lbl8amod4:	op8am0x1
			nextopcode
op8bmod4:
lbl8bmod4:	op8b
			nextopcode
op8cx1mod4:
lbl8cmod4a:	absolute
lbl8cmod4b:	sty8
			nextopcode
op8dm0mod4:
lbl8dmod4a:	absolute
lbl8dmod4b:	sta16
			nextopcode
op8ex1mod4:
lbl8emod4a:	absolute
lbl8emod4b:	stx8
			nextopcode
op8fm0mod4:
lbl8fmod4a:	absolutelong
lbl8fmod4b:	sta16
			nextopcode
op90mod4:
lbl90mod4:	op90
			nextopcode
op91m0mod4:
lbl91mod4a:	directindirectindexed1
lbl91mod4b:	sta16
			nextopcode
op92m0mod4:
lbl92mod4a:	directindirect
lbl92mod4b:	sta16
			nextopcode
op93m0mod4:
lbl93mod4a:	stackasmrelativeindirectindexed1
lbl93mod4b:	sta16
			nextopcode
op94x1mod4:
lbl94mod4a:	directindexedx1
lbl94mod4b:	sty8
			nextopcode
op95m0mod4:
lbl95mod4a:	directindexedx1
lbl95mod4b:	sta16
			nextopcode
op96x1mod4:
lbl96mod4a:	directindexedy1
lbl96mod4b:	stx8
			nextopcode
op97m0mod4:
lbl97mod4a:	directindirectindexedlong1
lbl97mod4b:	sta16
			nextopcode
op98m0mod4:
lbl98mod4:	op98m0x1
			nextopcode
op99m0mod4:
lbl99mod4a:	absoluteindexedy1
lbl99mod4b:	sta16
			nextopcode
op9amod4:
lbl9amod4:	op9ax1
			nextopcode
op9bx1mod4:
lbl9bmod4:	op9bx1
			nextopcode
op9cm0mod4:
lbl9cmod4a:	absolute
lbl9cmod4b:	stz16
			nextopcode
op9dm0mod4:
lbl9dmod4a:	absoluteindexedx1
lbl9dmod4b:	sta16
			nextopcode
op9em0mod4:	
lbl9emod4:	absoluteindexedx1		
		stz16
			nextopcode
op9fm0mod4:
lbl9fmod4a:	absolutelongindexedx1
lbl9fmod4b:	sta16
			nextopcode
opa0x1mod4:
lbla0mod4:	opa0x1
			nextopcode
opa1m0mod4:
lbla1mod4a:	directindexedindirect1
lbla1mod4b:	lda16
			nextopcode
opa2x1mod4:
lbla2mod4:	opa2x1
			nextopcode
opa3m0mod4:
lbla3mod4a:	stackasmrelative
lbla3mod4b:	lda16
			nextopcode
opa4x1mod4:
lbla4mod4a:	direct
lbla4mod4b:	ldy8
			nextopcode
opa5m0mod4:
lbla5mod4a:	direct
lbla5mod4b:	lda16
			nextopcode
opa6x1mod4:
lbla6mod4a:	direct
lbla6mod4b:	ldx8
			nextopcode
opa7m0mod4:
lbla7mod4a:	directindirectlong
lbla7mod4b:	lda16
			nextopcode
opa8x1mod4:
lbla8mod4:	opa8x1m0
			nextopcode
opa9m0mod4:
lbla9mod4:	opa9m0
			nextopcode
opaax1mod4:
lblaamod4:	opaax1m0
			nextopcode
opabmod4:
lblabmod4:	opab
			nextopcode
opacx1mod4:
lblacmod4a:	absolute
lblacmod4b:	ldy8
			nextopcode
opadm0mod4:
lbladmod4a:	absolute
lbladmod4b:	lda16
			nextopcode
opaex1mod4:
lblaemod4a:	absolute
lblaemod4b:	ldx8
			nextopcode
opafm0mod4:
lblafmod4a:	absolutelong
lblafmod4b:	lda16
			nextopcode
opb0mod4:
lblb0mod4:	opb0
			nextopcode
opb1m0mod4:
lblb1mod4a:	directindirectindexed1
lblb1mod4b:	lda16
			nextopcode
opb2m0mod4:
lblb2mod4a:	directindirect
lblb2mod4b:	lda16
			nextopcode
opb3m0mod4:
lblb3mod4a:	stackasmrelativeindirectindexed1
lblb3mod4b:	lda16
			nextopcode
opb4x1mod4:
lblb4mod4a:	directindexedx1
lblb4mod4b:	ldy8
			nextopcode
opb5m0mod4:
lblb5mod4a:	directindexedx1
lblb5mod4b:	lda16
			nextopcode
opb6x1mod4:
lblb6mod4a:	directindexedy1
lblb6mod4b:	ldx8
			nextopcode
opb7m0mod4:
lblb7mod4a:	directindirectindexedlong1
lblb7mod4b:	lda16
			nextopcode
opb8mod4:
lblb8mod4:	opb8
			nextopcode
opb9m0mod4:
lblb9mod4a:	absoluteindexedy1
lblb9mod4b:	lda16
			nextopcode
opbax1mod4:
lblbamod4:	opbax1
			nextopcode
opbbx1mod4:
lblbbmod4:	opbbx1
			nextopcode
opbcx1mod4:
lblbcmod4a:	absoluteindexedx1
lblbcmod4b:	ldy8
			nextopcode
opbdm0mod4:
lblbdmod4a:	absoluteindexedx1
lblbdmod4b:	lda16
			nextopcode
opbex1mod4:
lblbemod4a:	absoluteindexedy1
lblbemod4b:	ldx8
			nextopcode
opbfm0mod4:
lblbfmod4a:	absolutelongindexedx1
lblbfmod4b:	lda16
			nextopcode
opc0x1mod4:
lblc0mod4:	opc0x1
			nextopcode
opc1m0mod4:
lblc1mod4a:	directindexedindirect1
lblc1mod4b:	cmp16
			nextopcode
opc2mod4:
lblc2mod4:	opc2
			nextopcode
@.pool
opc3m0mod4:
lblc3mod4a:	stackasmrelative
lblc3mod4b:	cmp16
			nextopcode
opc4x1mod4:
lblc4mod4a:	direct
lblc4mod4b:	cmy8
			nextopcode
opc5m0mod4:
lblc5mod4a:	direct
lblc5mod4b:	cmp16
			nextopcode
opc6m0mod4:
lblc6mod4a:	direct
lblc6mod4b:	dec16
			nextopcode
opc7m0mod4:
lblc7mod4a:	directindirectlong
lblc7mod4b:	cmp16
			nextopcode
opc8x1mod4:
lblc8mod4:	opc8x1
			nextopcode
opc9m0mod4:
lblc9mod4:	opc9m0
			nextopcode
opcax1mod4:
lblcamod4:	opcax1
			nextopcode
opcbmod4:
lblcbmod4:	opcb
			nextopcode
opccx1mod4:
lblccmod4a:	absolute
lblccmod4b:	cmy8
			nextopcode
opcdm0mod4:
lblcdmod4a:	absolute
lblcdmod4b:	cmp16
			nextopcode
opcem0mod4:
lblcemod4a:	absolute
lblcemod4b:	dec16
			nextopcode
opcfm0mod4:
lblcfmod4a:	absolutelong
lblcfmod4b:	cmp16
			nextopcode
opd0mod4:
lbld0mod4:	opd0
			nextopcode
opd1m0mod4:
lbld1mod4a:	directindirectindexed1
lbld1mod4b:	cmp16
			nextopcode
opd2m0mod4:
lbld2mod4a:	directindirect
lbld2mod4b:	cmp16
			nextopcode
opd3m0mod4:
lbld3mod4a:	stackasmrelativeindirectindexed1
lbld3mod4b:	cmp16
			nextopcode
opd4mod4:
lbld4mod4:	opd4
			nextopcode
opd5m0mod4:
lbld5mod4a:	directindexedx1
lbld5mod4b:	cmp16
			nextopcode
opd6m0mod4:
lbld6mod4a:	directindexedx1
lbld6mod4b:	dec16
			nextopcode
opd7m0mod4:
lbld7mod4a:	directindirectindexedlong1
lbld7mod4b:	cmp16
			nextopcode
opd8mod4:
lbld8mod4:	opd8
			nextopcode
opd9m0mod4:
lbld9mod4a:	absoluteindexedy1
lbld9mod4b:	cmp16
			nextopcode
opdax1mod4:
lbldamod4:	opdax1
			nextopcode
opdbmod4:
lbldbmod4:	opdb
			nextopcode
opdcmod4:
lbldcmod4:	opdc
			nextopcode
opddm0mod4:
lblddmod4a:	absoluteindexedx1
lblddmod4b:	cmp16
			nextopcode
opdem0mod4:
lbldemod4a:	absoluteindexedx1
lbldemod4b:	dec16
			nextopcode
opdfm0mod4:
lbldfmod4a:	absolutelongindexedx1
lbldfmod4b:	cmp16
			nextopcode
ope0x1mod4:
lble0mod4:	ope0x1
			nextopcode
ope1m0mod4:
lble1mod4a:	directindexedindirect1
lble1mod4b:	sbc16
			nextopcode
ope2mod4:
lble2mod4:	ope2
			nextopcode
@.pool
ope3m0mod4:
lble3mod4a:	stackasmrelative
lble3mod4b:	sbc16
			nextopcode
ope4x1mod4:
lble4mod4a:	direct
lble4mod4b:	cmx8
			nextopcode
ope5m0mod4:
lble5mod4a:	direct
lble5mod4b:	sbc16
			nextopcode
ope6m0mod4:
lble6mod4a:	direct
lble6mod4b:	inc16
			nextopcode
ope7m0mod4:
lble7mod4a:	directindirectlong
lble7mod4b:	sbc16
			nextopcode
ope8x1mod4:
lble8mod4:	ope8x1
			nextopcode
ope9m0mod4:
lble9mod4a:	immediate16
lble9mod4b:	sbc16
			nextopcode
opeamod4:
lbleamod4:	opea
			nextopcode
opebmod4:
lblebmod4:	opebm0
			nextopcode
opecx1mod4:
lblecmod4a:	absolute
lblecmod4b:	cmx8
			nextopcode
opedm0mod4:
lbledmod4a:	absolute
lbledmod4b:	sbc16
			nextopcode
opeem0mod4:
lbleemod4a:	absolute
lbleemod4b:	inc16
			nextopcode
opefm0mod4:
lblefmod4a:	absolutelong
lblefmod4b:	sbc16
			nextopcode
opf0mod4:
lblf0mod4:	opf0
			nextopcode
opf1m0mod4:
lblf1mod4a:	directindirectindexed1
lblf1mod4b:	sbc16
			nextopcode
opf2m0mod4:
lblf2mod4a:	directindirect
lblf2mod4b:	sbc16
			nextopcode
opf3m0mod4:
lblf3mod4a:	stackasmrelativeindirectindexed1
lblf3mod4b:	sbc16
			nextopcode
opf4mod4:
lblf4mod4:	opf4
			nextopcode
opf5m0mod4:
lblf5mod4a:	directindexedx1
lblf5mod4b:	sbc16
			nextopcode
opf6m0mod4:
lblf6mod4a:	directindexedx1
lblf6mod4b:	inc16
			nextopcode
opf7m0mod4:
lblf7mod4a:	directindirectindexedlong1
lblf7mod4b:	sbc16
			nextopcode
opf8mod4:
lblf8mod4:	opf8
			nextopcode
opf9m0mod4:
lblf9mod4a:	absoluteindexedy1
lblf9mod4b:	sbc16
			nextopcode
opfax1mod4:
lblfamod4:	opfax1
			nextopcode
opfbmod4:
lblfbmod4:	opfb
			nextopcode
opfcmod4:
lblfcmod4:	opfcx1
			nextopcode
opfdm0mod4:
lblfdmod4a:	absoluteindexedx1
lblfdmod4b:	sbc16
			nextopcode
opfem0mod4:
lblfemod4a:	absoluteindexedx1
lblfemod4b:	inc16
			nextopcode
opffm0mod4:
lblffmod4a:	absolutelongindexedx1
lblffmod4b:	sbc16
			nextopcode

			
			@.pool

