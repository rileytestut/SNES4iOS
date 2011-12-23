/*****************************************************************
	FLAGS  
*****************************************************************/

.macro		UPDATE_C
		// CC : ARM Carry Clear
		BICCC	rstatus, rstatus, #MASK_CARRY  //	0 : AND	mask 11111011111 : set C to zero
		// CS : ARM Carry Set
		ORRCS	rstatus, rstatus, #MASK_CARRY      //	1 : OR	mask 00000100000 : set C to one
.endm
.macro		UPDATE_Z
		// NE : ARM Zero Clear
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		// EQ : ARM Zero Set
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one		
.endm
.macro		UPDATE_ZN
		// NE : ARM Zero Clear
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		// EQ : ARM Zero Set
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
		// PL : ARM Neg Clear
		BICPL	rstatus, rstatus, #MASK_NEG	// 0 : AND mask 11111011111 : set N to zero
		// MI : ARM Neg Set
		ORRMI	rstatus, rstatus, #MASK_NEG	// 1 : OR  mask 00000100000 : set N to one
.endm

/*****************************************************************
	OPCODES_MAC
*****************************************************************/




.macro ADC8
		TST rstatus, #MASK_DECIMAL
		BEQ 1111f				
		S9xGetByte		
		
	
	        STMFD 	R13!,{rscratch}		
		MOV 	rscratch4,#0x0F000000
		//rscratch2=xxW1xxxxxxxxxxxx
		AND 	rscratch2, rscratch, rscratch4
		//rscratch=xxW2xxxxxxxxxxxx
		AND 	rscratch, rscratch4, rscratch, LSR #4
		//rscratch3=xxA2xxxxxxxxxxxx
		AND 	rscratch3, rscratch4, regA, LSR #4
		//rscratch4=xxA1xxxxxxxxxxxx		
		AND 	rscratch4,regA,rscratch4		
		//R1=A1+W1+CARRY
		TST 	rstatus, #MASK_CARRY
		ADDNE 	rscratch2, rscratch2, #0x01000000
		ADD 	rscratch2,rscratch2,rscratch4
		// if R1 > 9
		CMP 	rscratch2, #0x09000000
		// then R1 -= 10
		SUBGT 	rscratch2, rscratch2, #0x0A000000
		// then A2++
		ADDGT 	rscratch3, rscratch3, #0x01000000
		// R2 = A2+W2
		ADD 	rscratch3, rscratch3, rscratch
		// if R2 > 9
		CMP 	rscratch3, #0x09000000
		// then R2 -= 10//
		SUBGT 	rscratch3, rscratch3, #0x0A000000
		// then SetCarry()
		ORRGT 	rstatus, rstatus, #MASK_CARRY // 1 : OR mask 00000100000 : set C to one
		// else ClearCarry()
		BICLE 	rstatus, rstatus, #MASK_CARRY // 0 : AND mask 11111011111 : set C to zero
		// gather rscratch3 and rscratch2 into ans8
		// rscratch3 : 0R2000000
		// rscratch2 : 0R1000000
		// -> 0xR2R1000000
		ORR 	rscratch2, rscratch2, rscratch3, LSL #4		
		LDMFD 	R13!,{rscratch}
		//only last bit
		AND 	rscratch,rscratch,#0x80000000
		// (register.AL ^ Work8)
		EORS 	rscratch3, regA, rscratch
		BICNE 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		BNE 	1112f
		// (Work8 ^ Ans8)
		EORS 	rscratch3, rscratch2, rscratch
		// & 0x80 
		TSTNE	rscratch3,#0x80000000
		BICEQ 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		ORRNE 	rstatus, rstatus, #MASK_OVERFLOW // 1 : OR mask 00000100000 : set V to one 
1112:
		MOVS regA, rscratch2
		UPDATE_ZN
		B 1113f
1111:
		S9xGetByteLow
		MOVS rscratch2, rstatus, LSR #MASK_SHIFTER_CARRY
		SUBCS rscratch, rscratch, #0x100 
		ADCS regA, regA, rscratch, ROR #8
		//OverFlow
		ORRVS rstatus, rstatus, #MASK_OVERFLOW
		BICVC rstatus, rstatus, #MASK_OVERFLOW
		//Carry
		UPDATE_C
		//clear lower part
		ANDS regA, regA, #0xFF000000
		//Update flag
		UPDATE_ZN
1113: 
.endm
/* TO TEST */
.macro ADC16 
		TST rstatus, #MASK_DECIMAL
		BEQ 1111f 
		S9xGetWord
		
		//rscratch = W3W2W1W0........
		LDR 	rscratch4, = 0x0F0F0000
		// rscratch2 = xxW2xxW0xxxxxx
		// rscratch3 = xxW3xxW1xxxxxx
		AND 	rscratch2, rscratch4, rscratch
		AND 	rscratch3, rscratch4, rscratch, LSR #4 
		// rscratch2 = xxW3xxW1xxW2xxW0
		ORR 	rscratch2, rscratch3, rscratch2, LSR #16 		
		// rscratch3 = xxA2xxA0xxxxxx
		// rscratch4 = xxA3xxA1xxxxxx
		// rscratch2 = xxA3xxA1xxA2xxA0
		AND 	rscratch3, rscratch4, regA
		AND 	rscratch4, rscratch4, regA, LSR #4
		ORR 	rscratch3, rscratch4, rscratch3, LSR #16		
		ADD 	rscratch2, rscratch3, rscratch2 		
		LDR 	rscratch4, = 0x0F0F0000		
		// rscratch2 = A + W
		TST 	rstatus, #MASK_CARRY
		ADDNE 	rscratch2, rscratch2, #0x1
		// rscratch2 = A + W + C
		//A0
		AND 	rscratch3, rscratch2, #0x0000001F
		CMP 	rscratch3, #0x00000009
		ADDHI 	rscratch2, rscratch2, #0x00010000
		SUBHI 	rscratch2, rscratch2, #0x0000000A
		//A1
		AND 	rscratch3, rscratch2, #0x001F0000
		CMP 	rscratch3, #0x00090000
		ADDHI 	rscratch2, rscratch2, #0x00000100
		SUBHI 	rscratch2, rscratch2, #0x000A0000
		//A2
		AND 	rscratch3, rscratch2, #0x00001F00
		CMP 	rscratch3, #0x00000900
		SUBHI 	rscratch2, rscratch2, #0x00000A00
		ADDHI 	rscratch2, rscratch2, #0x01000000
		//A3
		AND 	rscratch3, rscratch2, #0x1F000000
		CMP 	rscratch3, #0x09000000
		SUBHI 	rscratch2, rscratch2, #0x0A000000
		//SetCarry
		ORRHI 	rstatus, rstatus, #MASK_CARRY
		//ClearCarry
		BICLS 	rstatus, rstatus, #MASK_CARRY
		//rscratch2 = xxR3xxR1xxR2xxR0
		//Pack result 
		//rscratch3 = xxR3xxR1xxxxxxxx 
		AND 	rscratch3, rscratch4, rscratch2 
		//rscratch2 = xxR2xxR0xxxxxxxx
		AND 	rscratch2, rscratch4, rscratch2,LSL #16
		//rscratch2 = R3R2R1R0xxxxxxxx
		ORR 	rscratch2, rscratch2,rscratch3,LSL #4		
//only last bit
		AND 	rscratch,rscratch,#0x80000000
		// (register.AL ^ Work8)
		EORS 	rscratch3, regA, rscratch 
		BICNE 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		BNE 	1112f
		// (Work8 ^ Ans8)
		EORS 	rscratch3, rscratch2, rscratch 
		TSTNE	rscratch3,#0x80000000
		BICEQ 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		ORRNE 	rstatus, rstatus, #MASK_OVERFLOW // 1 : OR mask 00000100000 : set V to one 
1112:
		MOVS 	regA, rscratch2
		UPDATE_ZN
		B 	1113f
1111:
		S9xGetWordLow
		MOVS rscratch2, rstatus, LSR #MASK_SHIFTER_CARRY 
		SUBCS rscratch, rscratch, #0x10000 
		ADCS regA, regA,rscratch, ROR #16
		//OverFlow 
		ORRVS rstatus, rstatus, #MASK_OVERFLOW
		BICVC rstatus, rstatus, #MASK_OVERFLOW
		MOV regA, regA, LSR #16
		//Carry
		UPDATE_C
		//clear lower parts 
		MOVS regA, regA, LSL #16
		//Update flag
		UPDATE_ZN
1113: 
.endm


.macro		AND16
		S9xGetWord
		ANDS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		AND8
		S9xGetByte
		ANDS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		A_ASL8
		// 7	instr		
		MOVS	regA, regA, LSL #1
		UPDATE_C
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		A_ASL16
		// 7	instr		
		MOVS	regA, regA, LSL #1
		UPDATE_C
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		ASL16		
		S9xGetWordRegNS	rscratch2	      //	do not destroy Opadress	in rscratch
		MOVS		rscratch2, rscratch2, LSL #1
		UPDATE_C
		UPDATE_ZN		
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		ASL8				
		S9xGetByteRegNS	rscratch2	      //	do not destroy Opadress	in rscratch
		MOVS		rscratch2, rscratch2, LSL #1
		UPDATE_C
		UPDATE_ZN		
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
.macro		BIT8
		S9xGetByte
		MOVS	rscratch2, rscratch, LSL #1
		// Trick in ASM : shift one more bit	: ARM C	= Snes N
		//					  ARM N	= Snes V
		// If Carry Set, then Set Neg in SNES
		BICCC	rstatus, rstatus, #MASK_NEG	// 0 : AND mask 11111011111 : set C to zero
		ORRCS	rstatus, rstatus, #MASK_NEG	// 1 : OR  mask 00000100000 : set C to one
		// If Neg Set, then Set Overflow in SNES
		BICPL	rstatus, rstatus, #MASK_OVERFLOW  // 0 : AND mask 11111011111	: set N	to zero
		ORRMI	rstatus, rstatus, #MASK_OVERFLOW	     // 1 : OR  mask 00000100000	: set N	to one

		// Now do a real AND	with A register
		// Set Zero Flag, bit test
		ANDS	rscratch2, regA, rscratch
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
.endm

.macro		BIT16
		S9xGetWord
		MOVS	rscratch2, rscratch, LSL #1
		// Trick in ASM : shift one more bit	: ARM C	= Snes N
		//					  ARM N	= Snes V
		// If Carry Set, then Set Neg in SNES
		BICCC	rstatus, rstatus, #MASK_NEG	// 0 : AND mask 11111011111 : set N to zero
		ORRCS	rstatus, rstatus, #MASK_NEG	// 1 : OR  mask 00000100000 : set N to one
		// If Neg Set, then Set Overflow in SNES
		BICPL	rstatus, rstatus, #MASK_OVERFLOW  // 0 : AND mask 11111011111	: set V	to zero
		ORRMI	rstatus, rstatus, #MASK_OVERFLOW	     // 1 : OR  mask 00000100000	: set V	to one
		// Now do a real AND	with A register
		// Set Zero Flag, bit test
		ANDS	rscratch2, regA, rscratch
		// Bit set  ->Z=0->xxxNE Clear flag
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		// Bit clear->Z=1->xxxEQ Set flag
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
.endm
.macro		CMP8
		S9xGetByte			
		SUBS 	rscratch2,regA,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		
.endm
.macro		CMP16
		S9xGetWord
		SUBS 	rscratch2,regA,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		
.endm
.macro		CMX16
		S9xGetWord
		SUBS 	rscratch2,regX,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
.endm
.macro		CMX8
		S9xGetByte
		SUBS 	rscratch2,regX,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
.endm
.macro		CMY16
		S9xGetWord
		SUBS 	rscratch2,regY,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
.endm
.macro		CMY8
		S9xGetByte
		SUBS 	rscratch2,regY,rscratch		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
.endm
.macro		A_DEC8		
		MOV		rscratch,#0		
		SUBS		regA, regA, #0x01000000
		STR		rscratch,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		A_DEC16		
		MOV		rscratch,#0
		SUBS 		regA, regA, #0x00010000
		STR		rscratch,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		DEC16		
		S9xGetWordRegNS rscratch2	       // do not	destroy	Opadress in rscratch		
		MOV		rscratch3,#0
		SUBS		rscratch2, rscratch2, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN		
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		DEC8
		S9xGetByteRegNS rscratch2	       // do not	destroy	Opadress in rscratch
		MOV		rscratch3,#0
		SUBS		rscratch2, rscratch2, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN		
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
.macro		EOR16
		S9xGetWord
		EORS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		EOR8
		S9xGetByte
		EORS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		A_INC8		
		MOV		rscratch3,#0
		ADDS		regA, regA, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		A_INC16		
		MOV		rscratch3,#0	
		ADDS		regA, regA, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		INC16		
		S9xGetWordRegNS	rscratch2
		MOV		rscratch3,#0
		ADDS		rscratch2, rscratch2, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN		
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		INC8		
		S9xGetByteRegNS	rscratch2
		MOV		rscratch3,#0
		ADDS		rscratch2, rscratch2, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN		
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
.macro		LDA16
		S9xGetWordRegStatus regA
		UPDATE_ZN
.endm
.macro		LDA8
		S9xGetByteRegStatus regA
		UPDATE_ZN
.endm
.macro		LDX16
		S9xGetWordRegStatus regX
		UPDATE_ZN
.endm
.macro		LDX8
		S9xGetByteRegStatus regX
		UPDATE_ZN
.endm
.macro		LDY16
		S9xGetWordRegStatus regY
		UPDATE_ZN
.endm
.macro		LDY8
		S9xGetByteRegStatus regY
		UPDATE_ZN
.endm
.macro		A_LSR16				
		BIC	rstatus, rstatus, #MASK_NEG	 // 0 : AND mask	11111011111 : set N to zero
		MOVS	regA, regA, LSR #17		 // hhhhhhhh llllllll 00000000 00000000 -> 00000000 00000000 0hhhhhhh hlllllll
		// Update Zero
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		MOV	regA, regA, LSL #16			// -> 0lllllll 00000000 00000000	00000000
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
		// Note : the two MOV are included between instruction, to optimize
		// the pipeline.
		UPDATE_C
		ADD1CYCLE
.endm
.macro		A_LSR8		
		BIC	rstatus, rstatus, #MASK_NEG	 // 0 : AND mask	11111011111 : set N to zero
		MOVS	regA, regA, LSR #25		 // llllllll 00000000 00000000 00000000 -> 00000000 00000000 00000000 0lllllll
		// Update Zero
		BICNE	rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		MOV	regA, regA, LSL #24			// -> 00000000 00000000 00000000	0lllllll
		ORREQ	rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one		
		// Note : the two MOV are included between instruction, to optimize
		// the pipeline.
		UPDATE_C
		ADD1CYCLE
.endm
.macro		LSR16				
		S9xGetWordRegNS	rscratch2
		// N set to zero by >> 1 LSR
		BIC		rstatus, rstatus, #MASK_NEG	 // 0 : AND mask	11111011111 : set N to zero
		MOVS		rscratch2, rscratch2, LSR #17		   // llllllll 00000000 00000000	00000000 -> 00000000 00000000 00000000 0lllllll
		// Update Carry		
		BICCC		rstatus, rstatus, #MASK_CARRY  //	0 : AND	mask 11111011111 : set C to zero		
		ORRCS		rstatus, rstatus, #MASK_CARRY      //	1 : OR	mask 00000100000 : set C to one
		// Update Zero
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one	
		S9xSetWordLow 	rscratch2
		ADD1CYCLE
.endm
.macro		LSR8				
		S9xGetByteRegNS	rscratch2
		// N set to zero by >> 1 LSR
		BIC		rstatus, rstatus, #MASK_NEG	 // 0 : AND mask	11111011111 : set N to zero
		MOVS		rscratch2, rscratch2, LSR #25		   // llllllll 00000000 00000000	00000000 -> 00000000 00000000 00000000 0lllllll
		// Update Carry		
		BICCC		rstatus, rstatus, #MASK_CARRY  //	0 : AND	mask 11111011111 : set C to zero		
		ORRCS		rstatus, rstatus, #MASK_CARRY      //	1 : OR	mask 00000100000 : set C to one
		// Update Zero
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one		
		S9xSetByteLow 	rscratch2
		ADD1CYCLE
.endm
.macro		ORA8
		S9xGetByte
		ORRS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		ORA16
		S9xGetWord
		ORRS		regA, regA, rscratch
		UPDATE_ZN
.endm
.macro		A_ROL16		
		TST		rstatus, #MASK_CARRY
		ORRNE		regA, regA, #0x00008000
		MOVS		regA, regA, LSL #1
		UPDATE_ZN
		UPDATE_C
		ADD1CYCLE
.endm
.macro		A_ROL8		
		TST		rstatus, #MASK_CARRY
		ORRNE		regA, regA, #0x00800000
		MOVS		regA, regA, LSL #1
		UPDATE_ZN
		UPDATE_C
		ADD1CYCLE
.endm
.macro		ROL16		
		S9xGetWordRegNS	rscratch2
		TST		rstatus, #MASK_CARRY
		ORRNE		rscratch2, rscratch2, #0x00008000
		MOVS		rscratch2, rscratch2, LSL #1
		UPDATE_ZN
		UPDATE_C		
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		ROL8		
		S9xGetByteRegNS	rscratch2
		TST		rstatus, #MASK_CARRY
		ORRNE		rscratch2, rscratch2, #0x00800000
		MOVS		rscratch2, rscratch2, LSL #1
		UPDATE_ZN
		UPDATE_C		
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
.macro		A_ROR16		
		MOV			regA,regA, LSR #16
		TST			rstatus, #MASK_CARRY
		ORRNE			regA, regA, #0x00010000
		ORRNE			rstatus,rstatus,#MASK_NEG
		BICEQ			rstatus,rstatus,#MASK_NEG		
		MOVS			regA,regA,LSR #1
		UPDATE_C
		UPDATE_Z		
		MOV			regA,regA, LSL #16
		ADD1CYCLE
.endm
.macro		A_ROR8				
		MOV			regA,regA, LSR #24
		TST			rstatus, #MASK_CARRY
		ORRNE			regA, regA, #0x00000100
		ORRNE			rstatus,rstatus,#MASK_NEG
		BICEQ			rstatus,rstatus,#MASK_NEG		
		MOVS			regA,regA,LSR #1
		UPDATE_C
		UPDATE_Z		
		MOV			regA,regA, LSL #24
		ADD1CYCLE
.endm
.macro		ROR16		
		S9xGetWordLowRegNS	rscratch2
		TST			rstatus, #MASK_CARRY
		ORRNE			rscratch2, rscratch2, #0x00010000
		ORRNE			rstatus,rstatus,#MASK_NEG
		BICEQ			rstatus,rstatus,#MASK_NEG		
		MOVS			rscratch2,rscratch2,LSR #1
		UPDATE_C
		UPDATE_Z
		S9xSetWordLow 	rscratch2
		ADD1CYCLE

.endm
.macro		ROR8		
		S9xGetByteLowRegNS	rscratch2
		TST			rstatus, #MASK_CARRY
		ORRNE			rscratch2, rscratch2, #0x00000100
		ORRNE			rstatus,rstatus,#MASK_NEG
		BICEQ			rstatus,rstatus,#MASK_NEG		
		MOVS			rscratch2,rscratch2,LSR #1
		UPDATE_C
		UPDATE_Z
		S9xSetByteLow 	rscratch2
		ADD1CYCLE
.endm

.macro SBC16
		TST rstatus, #MASK_DECIMAL
		BEQ 1111f
		//TODO
		S9xGetWord
		
		STMFD 	R13!,{rscratch5,rscratch6,rscratch7,rscratch8,rscratch9}
		MOV 	rscratch9,#0x000F0000
		//rscratch2=xxxxxxW1xxxxxxxxxx + !Carry
		//rscratch3=xxxxxxW2xxxxxxxxxx		
		//rscratch4=xxxxxxW3xxxxxxxxxx
		//rscratch5=xxxxxxW4xxxxxxxxxx		
		AND 	rscratch2, rscratch9, rscratch
		TST 	rstatus, #MASK_CARRY
		ADDEQ 	rscratch2, rscratch2, #0x00010000  //W1=W1+!Carry
		AND 	rscratch3, rscratch9, rscratch, LSR #4
		AND 	rscratch4, rscratch9, rscratch, LSR #8
		AND 	rscratch5, rscratch9, rscratch, LSR #12
		
		//rscratch6=xxxxxxA1xxxxxxxxxx
		//rscratch7=xxxxxxA2xxxxxxxxxx		
		//rscratch8=xxxxxxA3xxxxxxxxxx
		//rscratch9=xxxxxxA4xxxxxxxxxx		
		AND 	rscratch6, rscratch9, regA
		AND 	rscratch7, rscratch9, regA, LSR #4
		AND 	rscratch8, rscratch9, regA, LSR #8
		AND 	rscratch9, rscratch9, regA, LSR #12				
								
		SUB 	rscratch2,rscratch6,rscratch2		//R1=A1-W1-!Carry
		CMP 	rscratch2, #0x00090000	// if R1 > 9		
		ADDHI 	rscratch2, rscratch2, #0x000A0000 // then R1 += 10		
		ADDHI 	rscratch3, rscratch3, #0x00010000  // then (W2++)
		SUB 	rscratch3,rscratch7,rscratch3		//R2=A2-W2
		CMP 	rscratch3, #0x00090000	// if R2 > 9		
		ADDHI 	rscratch3, rscratch3, #0x000A0000 // then R2 += 10		
		ADDHI 	rscratch4, rscratch4, #0x00010000  // then (W3++)
		SUB 	rscratch4,rscratch8,rscratch4		//R3=A3-W3
		CMP 	rscratch4, #0x00090000	// if R3 > 9		
		ADDHI 	rscratch4, rscratch4, #0x000A0000 // then R3 += 10		
		ADDHI 	rscratch5, rscratch5, #0x00010000  // then (W3++)
		SUB 	rscratch5,rscratch9,rscratch5		//R4=A4-W4
		CMP 	rscratch5, #0x00090000	// if R4 > 9		
		ADDHI 	rscratch5, rscratch5, #0x000A0000 // then R4 += 10
		BICHI 	rstatus, rstatus, #MASK_CARRY	// then ClearCarry
		ORRLS 	rstatus, rstatus, #MASK_CARRY	// else SetCarry
		
		MOV 	rscratch9,#0x000F0000
		AND	rscratch2,rscratch9,rscratch2
		AND	rscratch3,rscratch9,rscratch3
		AND	rscratch4,rscratch9,rscratch4
		AND	rscratch5,rscratch9,rscratch5
		ORR	rscratch2,rscratch2,rscratch3,LSL #4
		ORR	rscratch2,rscratch2,rscratch4,LSL #8
		ORR	rscratch2,rscratch2,rscratch5,LSL #12
		
		LDMFD 	R13!,{rscratch5,rscratch6,rscratch7,rscratch8,rscratch9}
		//only last bit
		AND 	regA,regA,#0x80000000
		// (register.A.W ^ Work8)			
		EORS 	rscratch3, regA, rscratch
		BICEQ 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		BEQ 	1112f
		// (register.A.W ^ Ans8)
		EORS 	rscratch3, regA, rscratch2
		// & 0x80 
		TSTNE	rscratch3,#0x80000000
		BICEQ   rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero		
		ORRNE 	rstatus, rstatus, #MASK_OVERFLOW // 1 : OR mask 00000100000 : set V to one 
1112:
		MOVS 	regA, rscratch2
		UPDATE_ZN 		
		B 1113f
1111:
		S9xGetWordLow 
		MOVS rscratch2,rstatus,LSR #MASK_SHIFTER_CARRY
		SBCS regA, regA, rscratch, LSL #16 
		//OverFlow 
		ORRVS rstatus, rstatus, #MASK_OVERFLOW
		BICVC rstatus, rstatus, #MASK_OVERFLOW
		MOV regA, regA, LSR #16
		//Carry
		UPDATE_C
		MOVS regA, regA, LSL #16
		//Update flag
		UPDATE_ZN
1113:
.endm 

.macro SBC8
		TST rstatus, #MASK_DECIMAL 
		BEQ 1111f		
		S9xGetByte					
		STMFD 	R13!,{rscratch}		
		MOV 	rscratch4,#0x0F000000
		//rscratch2=xxW1xxxxxxxxxxxx
		AND 	rscratch2, rscratch, rscratch4
		//rscratch=xxW2xxxxxxxxxxxx
		AND 	rscratch, rscratch4, rscratch, LSR #4				
		//rscratch3=xxA2xxxxxxxxxxxx
		AND 	rscratch3, rscratch4, regA, LSR #4
		//rscratch4=xxA1xxxxxxxxxxxx
		AND 	rscratch4,regA,rscratch4		
		//R1=A1-W1-!CARRY
		TST 	rstatus, #MASK_CARRY
		ADDEQ 	rscratch2, rscratch2, #0x01000000
		SUB 	rscratch2,rscratch4,rscratch2
		// if R1 > 9
		CMP 	rscratch2, #0x09000000
		// then R1 += 10
		ADDHI 	rscratch2, rscratch2, #0x0A000000
		// then A2-- (W2++)
		ADDHI 	rscratch, rscratch, #0x01000000
		// R2=A2-W2
		SUB 	rscratch3, rscratch3, rscratch
		// if R2 > 9
		CMP 	rscratch3, #0x09000000
		// then R2 -= 10//
		ADDHI 	rscratch3, rscratch3, #0x0A000000
		// then SetCarry()
		BICHI 	rstatus, rstatus, #MASK_CARRY // 1 : OR mask 00000100000 : set C to one
		// else ClearCarry()
		ORRLS 	rstatus, rstatus, #MASK_CARRY // 0 : AND mask 11111011111 : set C to zero
		// gather rscratch3 and rscratch2 into ans8
		AND	rscratch3,rscratch3,#0x0F000000
		AND	rscratch2,rscratch2,#0x0F000000		
		// rscratch3 : 0R2000000
		// rscratch2 : 0R1000000
		// -> 0xR2R1000000				
		ORR 	rscratch2, rscratch2, rscratch3, LSL #4		
		LDMFD 	R13!,{rscratch}
		//only last bit
		AND 	regA,regA,#0x80000000
		// (register.AL ^ Work8)			
		EORS 	rscratch3, regA, rscratch
		BICEQ 	rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		BEQ 	1112f
		// (register.AL ^ Ans8)
		EORS 	rscratch3, regA, rscratch2
		// & 0x80 
		TSTNE	rscratch3,#0x80000000
		BICEQ rstatus, rstatus, #MASK_OVERFLOW // 0 : AND mask 11111011111 : set V to zero
		ORRNE rstatus, rstatus, #MASK_OVERFLOW // 1 : OR mask 00000100000 : set V to one 
1112:
		MOVS regA, rscratch2
		UPDATE_ZN 
		B 1113f
1111:
		S9xGetByteLow
		MOVS rscratch2,rstatus,LSR #MASK_SHIFTER_CARRY
		SBCS regA, regA, rscratch, LSL #24 
		//OverFlow 
		ORRVS rstatus, rstatus, #MASK_OVERFLOW
		BICVC rstatus, rstatus, #MASK_OVERFLOW 
		//Carry
		UPDATE_C 
		//Update flag
		ANDS regA, regA, #0xFF000000
		UPDATE_ZN
1113:
.endm 

.macro		STA16
		S9xSetWord	regA
.endm
.macro		STA8
		S9xSetByte	regA
.endm
.macro		STX16
		S9xSetWord	regX
.endm
.macro		STX8
		S9xSetByte	regX
.endm
.macro		STY16
		S9xSetWord	regY
.endm
.macro		STY8
		S9xSetByte	regY
.endm
.macro		STZ16
		S9xSetWordZero
.endm
.macro		STZ8		
		S9xSetByteZero
.endm
.macro		TSB16			
		S9xGetWordRegNS	rscratch2
		TST		regA, rscratch2
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one		
		ORR		rscratch2, regA, rscratch2		
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		TSB8				
		S9xGetByteRegNS	rscratch2
		TST		regA, rscratch2
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
		ORR		rscratch2, regA, rscratch2				
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
.macro		TRB16		
		S9xGetWordRegNS	rscratch2
		TST		regA, rscratch2
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
		MVN		rscratch3, regA
		AND		rscratch2, rscratch3, rscratch2
		S9xSetWord 	rscratch2
		ADD1CYCLE
.endm
.macro		TRB8				
		S9xGetByteRegNS	rscratch2
		TST		regA, rscratch2
		BICNE		rstatus, rstatus, #MASK_ZERO	 // 0 : AND mask	11111011111 : set Z to zero
		ORREQ		rstatus, rstatus, #MASK_ZERO	 // 1 : OR  mask	00000100000  : set Z to	one
		MVN		rscratch3, regA
		AND		rscratch2, rscratch3, rscratch2		
		S9xSetByte 	rscratch2
		ADD1CYCLE
.endm
/**************************************************************************/


/**************************************************************************/

.macro		Op09M0		/*ORA*/
		LDRB		rscratch2, [rpc,#1]
		LDRB		rscratch, [rpc], #2
		ORR		rscratch2,rscratch,rscratch2,LSL #8
		ORRS		regA,regA,rscratch2,LSL #16
		UPDATE_ZN
		ADD2MEM
.endm
.macro		Op09M1		/*ORA*/
		LDRB		rscratch, [rpc], #1
		ORRS		regA,regA,rscratch,LSL #24
		UPDATE_ZN
		ADD1MEM
.endm
/***********************************************************************/
.macro		Op90 	/*BCC*/
		asmRelative		
		BranchCheck0
		TST		rstatus, #MASK_CARRY
		BNE		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress +PCBase
                ADD1CYCLE
                CPUShutdown
1111:
.endm
.macro		OpB0	/*BCS*/
		asmRelative		
		BranchCheck0
		TST		rstatus, #MASK_CARRY
		BEQ		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress +PCBase
                ADD1CYCLE
                CPUShutdown
1111:
.endm
.macro		OpF0 	/*BEQ*/
		asmRelative		
		BranchCheck2
		TST		rstatus, #MASK_ZERO
		BEQ		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress +PCBase
                ADD1CYCLE
                CPUShutdown
1111:
.endm
.macro		OpD0	/*BNE*/
		asmRelative		
		BranchCheck1
		TST		rstatus, #MASK_ZERO
		BNE		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress +PCBase
                ADD1CYCLE
                CPUShutdown
1111:
.endm
.macro		Op30	/*BMI*/
		asmRelative		
		BranchCheck0
		TST		rstatus, #MASK_NEG
		BEQ		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress +PCBase
                ADD1CYCLE
                CPUShutdown
1111:
.endm
.macro		Op10   /*BPL*/
		asmRelative
		BranchCheck1
		TST 		rstatus, #MASK_NEG // neg, z!=0, NE
		BNE		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress + PCBase
                ADD1CYCLE
                CPUShutdown
1111:                
.endm
.macro		Op50   /*BVC*/
		asmRelative
		BranchCheck0
		TST 		rstatus, #MASK_OVERFLOW // neg, z!=0, NE
		BNE		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress + PCBase
                ADD1CYCLE
                CPUShutdown
1111:                
.endm
.macro		Op70   /*BVS*/
		asmRelative
		BranchCheck0
		TST 		rstatus, #MASK_OVERFLOW // neg, z!=0, NE
		BEQ		1111f
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress + PCBase
                ADD1CYCLE
                CPUShutdown
1111:                
.endm
.macro		Op80   /*BRA*/
		asmRelative				
                ADD 		rpc, rscratch, regpcbase // rpc = OpAddress + PCBase
                ADD1CYCLE
                CPUShutdown
1111:                
.endm
/*******************************************************************************************/
/************************************************************/
/* SetFlag Instructions ********************************************************************** */
.macro		Op38 /*SEC*/		
		ORR		rstatus, rstatus, #MASK_CARRY      //	1 : OR	mask 00000100000 : set C to one
		ADD1CYCLE
.endm
.macro		OpF8 /*SED*/		
		SetDecimal
		ADD1CYCLE		
.endm
.macro		Op78 /*SEI*/
		SetIRQ
		ADD1CYCLE
.endm


/****************************************************************************************/
/* ClearFlag Instructions ******************************************************************** */		
.macro		Op18  /*CLC*/		
		BIC 		rstatus, rstatus, #MASK_CARRY
		ADD1CYCLE
.endm
.macro		OpD8 /*CLD*/		
		ClearDecimal
		ADD1CYCLE
.endm
.macro		Op58  /*CLI*/		
		ClearIRQ
		ADD1CYCLE		
		//CHECK_FOR_IRQ
.endm
.macro		OpB8 /*CLV*/		
		BIC 		rstatus, rstatus, #MASK_OVERFLOW
		ADD1CYCLE     
.endm

/******************************************************************************************/
/* DEX/DEY *********************************************************************************** */

.macro		OpCAX1  /*DEX*/
		MOV		rscratch3,#0
		SUBS 		regX, regX, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpCAX0  /*DEX*/		
		MOV		rscratch3,#0
		SUBS 		regX, regX, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op88X1 /*DEY*/
		MOV		rscratch3,#0
		SUBS 		regY, regY, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op88X0 /*DEY*/
		MOV		rscratch3,#0
		SUBS 		regY, regY, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm

/******************************************************************************************/
/* INX/INY *********************************************************************************** */		
.macro		OpE8X1
		MOV		rscratch3,#0
		ADDS 		regX, regX, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpE8X0
		MOV		rscratch3,#0
		ADDS 		regX, regX, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpC8X1
		MOV		rscratch3,#0
		ADDS 		regY, regY, #0x01000000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpC8X0		
		MOV		rscratch3,#0
		ADDS 		regY, regY, #0x00010000
		STR		rscratch3,[regCPUvar,#WaitAddress_ofs]
		UPDATE_ZN
		ADD1CYCLE
.endm

/**********************************************************************************************/

/* NOP *************************************************************************************** */		
.macro		OpEA		
		ADD1CYCLE
.endm

/**************************************************************************/
/* PUSH Instructions **************************************************** */
.macro		OpF4
		Absolute		
		PushWrLow
.endm
.macro		OpD4
		DirectIndirect		
		PushWrLow
.endm
.macro		Op62
		asmRelativeLong
		PushWrLow
.endm
.macro		Op48M0		
		PushW 		regA
		ADD1CYCLE
.endm
.macro		Op48M1		
		PushB 		regA
		ADD1CYCLE
.endm
.macro		Op8B
		AND		rscratch2, regDBank, #0xFF
		PushBLow 	rscratch2
		ADD1CYCLE
.endm
.macro		Op0B
		PushW	 	regD
		ADD1CYCLE
.endm
.macro		Op4B
		PushBlow	regPBank
		ADD1CYCLE
.endm
.macro		Op08		
		PushB	 	rstatus
		ADD1CYCLE
.endm
.macro		OpDAX1
		PushB 		regX
		ADD1CYCLE
.endm
.macro		OpDAX0
		PushW 		regX
		ADD1CYCLE
.endm
.macro		Op5AX1		
		PushB 		regY
		ADD1CYCLE
.endm
.macro		Op5AX0
		PushW 		regY
		ADD1CYCLE
.endm
/**************************************************************************/
/* PULL Instructions **************************************************** */
.macro		Op68M1
		PullBS		regA
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		Op68M0
		PullWS		regA
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		OpAB
		BIC		regDBank,regDBank, #0xFF
		PullBrS 	
		ORR		regDBank,regDBank,rscratch, LSR #24
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		Op2B		
		BIC		regD,regD, #0xFF000000
		BIC		regD,regD, #0x00FF0000
		PullWrS		
		ORR		regD,rscratch,regD
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		Op28X1M1	/*PLP*/
		//INDEX set, MEMORY set
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		TST 		rstatus, #MASK_INDEX		
		//INDEX clear & was set : 8->16
		MOVEQ		regX,regX,LSR #8
		MOVEQ		regY,regY,LSR #8		
		TST 		rstatus, #MASK_MEM		
		//MEMORY cleared & was set : 8->16
		LDREQB		rscratch,[regCPUvar,#RAH_ofs]		
		MOVEQ		regA,regA,LSR #8
		ORREQ		regA,regA,rscratch, LSL #24
		S9xFixCycles
		ADD2CYCLE
.endm
.macro		Op28X0M1	/*PLP*/		
		//INDEX cleared, MEMORY set
		BIC		rstatus,rstatus,#0xFF000000				
		PullBr		
		ORR		rstatus,rscratch,rstatus
		TST 		rstatus, #MASK_INDEX
		//INDEX set & was cleared : 16->8
		MOVNE		regX,regX,LSL #8
		MOVNE		regY,regY,LSL #8
		TST 		rstatus, #MASK_MEM
		//MEMORY cleared & was set : 8->16
		LDREQB		rscratch,[regCPUvar,#RAH_ofs]
		MOVEQ		regA,regA,LSR #8
		ORREQ		regA,regA,rscratch, LSL #24
		S9xFixCycles
		ADD2CYCLE
.endm
.macro		Op28X1M0	/*PLP*/
		//INDEX set, MEMORY set		
		BIC		rstatus,rstatus,#0xFF000000				
		PullBr		
		ORR		rstatus,rscratch,rstatus
		TST 		rstatus, #MASK_INDEX
		//INDEX clear & was set : 8->16
		MOVEQ		regX,regX,LSR #8
		MOVEQ		regY,regY,LSR #8		
		TST 		rstatus, #MASK_MEM
		//MEMORY set & was cleared : 16->8				
		MOVNE		rscratch,regA,LSR #24
		MOVNE		regA,regA,LSL #8
		STRNEB		rscratch,[regCPUvar,#RAH_ofs]
		S9xFixCycles
		ADD2CYCLE
.endm
.macro		Op28X0M0	/*PLP*/
		//INDEX set, MEMORY set
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		TST 		rstatus, #MASK_INDEX
		//INDEX set & was cleared : 16->8
		MOVNE		regX,regX,LSL #8
		MOVNE		regY,regY,LSL #8
		TST 		rstatus, #MASK_MEM
		//MEMORY set & was cleared : 16->8				
		MOVNE		rscratch,regA,LSR #24
		MOVNE		regA,regA,LSL #8
		STRNEB		rscratch,[regCPUvar,#RAH_ofs]
		S9xFixCycles
		ADD2CYCLE
.endm
.macro		OpFAX1
		PullBS 		regX
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		OpFAX0	
		PullWS 		regX
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		Op7AX1
		PullBS		regY
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		Op7AX0		
		PullWS 		regY
		UPDATE_ZN
		ADD2CYCLE
.endm		

/**********************************************************************************************/
/* Transfer Instructions ********************************************************************* */
.macro		OpAAX1M1 /*TAX8*/		
		MOVS 		regX, regA
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpAAX0M1 /*TAX16*/		
		LDRB 		regX, [regCPUvar,#RAH_ofs]
		MOV		regX, regX,LSL #24
		ORRS		regX, regX,regA, LSR #8		
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpAAX1M0 /*TAX8*/		
		MOVS 		regX, regA, LSL #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpAAX0M0 /*TAX16*/		
		MOVS 		regX, regA
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpA8X1M1 /*TAY8*/		
		MOVS 		regY, regA
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpA8X0M1 /*TAY16*/
		LDRB 		regY, [regCPUvar,#RAH_ofs]
		MOV		regY, regY,LSL #24
		ORRS		regY, regY,regA, LSR #8		
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpA8X1M0 /*TAY8*/		
		MOVS 		regY, regA, LSL #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpA8X0M0 /*TAY16*/
		MOVS 		regY, regA
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op5BM1		
		LDRB		rscratch, [regCPUvar,#RAH_ofs]
		MOV		regD,regD,LSL #16
		MOV		rscratch,rscratch,LSL #24
		ORRS		rscratch,rscratch,regA, LSR #8		
		UPDATE_ZN
		ORR  		regD,rscratch,regD,LSR #16
		ADD1CYCLE
.endm
.macro		Op5BM0		
		MOV		regD,regD,LSL #16		
		MOVS		regA,regA
		UPDATE_ZN
		ORR  		regD,regA,regD,LSR #16
		ADD1CYCLE
.endm
.macro		Op1BM1
		TST 		rstatus, #MASK_EMUL
		MOVNE		regS, regA, LSR #24
		ORRNE		regS, regS, #0x100		
		LDREQB		regS, [regCPUvar,#RAH_ofs]
		ORREQ		regS, regS, regA
		MOVEQ		regS, regS, ROR #24
		ADD1CYCLE
.endm
.macro		Op1BM0		
		MOV 		regS, regA, LSR #16
		ADD1CYCLE
.endm
.macro		Op7BM1		
		MOVS 		regA, regD, ASR #16		
		UPDATE_ZN
		MOV		rscratch,regA,LSR #8		
		MOV		regA,regA, LSL #24
		STRB		rscratch, [regCPUvar,#RAH_ofs]
		ADD1CYCLE
.endm
.macro		Op7BM0
		MOVS 		regA, regD, ASR #16		
		UPDATE_ZN
		MOV		regA,regA, LSL #16
		ADD1CYCLE
.endm
.macro		Op3BM1
		MOV		rscratch,regS, LSR #8
		MOVS		regA, regS, LSL #16
		STRB		rscratch, [regCPUvar,#RAH_ofs]
		UPDATE_ZN
		MOV		regA,regA, LSL #8
		ADD1CYCLE
.endm
.macro		Op3BM0
		MOVS		regA, regS, LSL #16
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpBAX1
		MOVS 		regX, regS, LSL #24
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpBAX0
		MOVS 		regX, regS, LSL #16
		UPDATE_ZN
		ADD1CYCLE
.endm		
.macro		Op8AM1X1
		MOVS 		regA, regX
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op8AM1X0
		MOVS 		regA, regX, LSL #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op8AM0X1
		MOVS 		regA, regX, LSR #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op8AM0X0
		MOVS 		regA, regX
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op9AX1		
		MOV 		regS, regX, LSR #24
		TST 		rstatus, #MASK_EMUL		
		ORRNE		regS, regS, #0x100
		ADD1CYCLE
.endm
.macro		Op9AX0		
		MOV 		regS, regX, LSR #16
		ADD1CYCLE
.endm
.macro		Op9BX1		
		MOVS 		regY, regX
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op9BX0		
		MOVS 		regY, regX
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op98M1X1	
		MOVS 		regA, regY
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op98M1X0
		MOVS 		regA, regY, LSL #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op98M0X1
		MOVS 		regA, regY, LSR #8
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		Op98M0X0
		MOVS 		regA, regY
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpBBX1		
		MOVS 		regX, regY
		UPDATE_ZN
		ADD1CYCLE
.endm
.macro		OpBBX0
		MOVS 		regX, regY
		UPDATE_ZN
		ADD1CYCLE
.endm

/**********************************************************************************************/
/* XCE *************************************************************************************** */

.macro		OpFB
    TST		rstatus,#MASK_CARRY
    BEQ		1111f
    //CARRY is set
    TST		rstatus,#MASK_EMUL    
    BNE		1112f
    //EMUL is cleared
    BIC		rstatus,rstatus,#(MASK_CARRY)
    TST		rstatus,#MASK_INDEX
    //X & Y were 16bits before
    MOVEQ	regX,regX,LSL #8
    MOVEQ	regY,regY,LSL #8
    TST		rstatus,#MASK_MEM
    //A was 16bits before
    //save AH
    MOVEQ	rscratch,regA,LSR #24
    STREQB	rscratch,[regCPUvar,#RAH_ofs]
    MOVEQ	regA,regA,LSL #8
    ORR		rstatus,rstatus,#(MASK_EMUL|MASK_MEM|MASK_INDEX)
    AND		regS,regS,#0xFF
    ORR		regS,regS,#0x100    
    B		1113f    
1112:    
    //EMUL is set
    TST		rstatus,#MASK_INDEX
    //X & Y were 16bits before
    MOVEQ	regX,regX,LSL #8
    MOVEQ	regY,regY,LSL #8
    TST		rstatus,#MASK_MEM
    //A was 16bits before
    //save AH
    MOVEQ	rscratch,regA,LSR #24
    STREQB	rscratch,[regCPUvar,#RAH_ofs]
    MOVEQ	regA,regA,LSL #8
    ORR		rstatus,rstatus,#(MASK_CARRY|MASK_MEM|MASK_INDEX)
    AND		regS,regS,#0xFF
    ORR		regS,regS,#0x100    
    B		1113f
1111:    
    //CARRY is cleared
    TST		rstatus,#MASK_EMUL
    BEQ		1115f
    //EMUL was set : X,Y & A were 8bits
    //Now have to check MEMORY & INDEX for potential conversions to 16bits
    TST		rstatus,#MASK_INDEX
    // X & Y are now 16bits
    MOVEQ	regX,regX,LSR #8	
    MOVEQ	regY,regY,LSR #8	
    TST		rstatus,#MASK_MEM
    // A is now 16bits
    MOVEQ	regA,regA,LSR #8	
    //restore AH
    LDREQB	rscratch,[regCPUvar,#RAH_ofs]    
    ORREQ	regA,regA,rscratch,LSL #24
1115:    
    BIC		rstatus,rstatus,#(MASK_EMUL)
    ORR		rstatus,rstatus,#(MASK_CARRY)
1113:
    ADD1CYCLE
    S9xFixCycles
.endm

/*******************************************************************************/
/* BRK *************************************************************************/
.macro		Op00		/*BRK*/
		MOV		rscratch,#1
		STRB		rscratch,[regCPUvar,#BRKTriggered_ofs]
		
		TST 		rstatus, #MASK_EMUL
		// EQ is flag to zero (!CheckEmu)
		BNE 		2001f//elseOp00
		PushBLow	regPBank
		SUB 		rscratch, rpc, regpcbase
		ADD 		rscratch2, rscratch, #1
		PushWLow 	rscratch2
		// PackStatus
		PushB	 	rstatus
		ClearDecimal
		SetIRQ
		BIC 		regPBank, regPBank, #0xFF
		MOV 		rscratch, #0xE6
		ORR 		rscratch, rscratch, #0xFF00
		S9xGetWordLow 		
		S9xSetPCBase 	
		ADD2CYCLE
		B 		2002f//endOp00
2001://elseOp00
		SUB 		rscratch2, rpc, regpcbase
		PushWLow 	rscratch2
		// PackStatus
		PushB	 	rstatus
		ClearDecimal
		SetIRQ
		BIC		regPBank,regPBank, #0xFF
		MOV 		rscratch, #0xFE
		ORR 		rscratch, rscratch, #0xFF00
		S9xGetWordLow 		
		S9xSetPCBase 	
		ADD1CYCLE
2002://endOp00
.endm


/**********************************************************************************************/
/* BRL ************************************************************************************** */
.macro		Op82	/*BRL*/
		asmRelativeLong
		ORR		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase
.endm		
/**********************************************************************************************/
/* IRQ *************************************************************************************** */			
//void S9xOpcode_IRQ (void)		
/*
    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEE));	
	CPU.Cycles += TWO_CYCLES;
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFE));
	CPU.Cycles += ONE_CYCLE;
    }
 }
*/	
		
/**********************************************************************************************/
/* NMI *************************************************************************************** */		
//void S9xOpcode_NMI (void)
/*{
    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEA));
	CPU.Cycles += TWO_CYCLES;
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFA));
	CPU.Cycles += ONE_CYCLE;
    }
}
*/

/**********************************************************************************************/
/* COP *************************************************************************************** */
.macro		Op02		/*COP*/
		TST 		rstatus, #MASK_EMUL
		// EQ is flag to zero (!CheckEmu)
		BNE 		2021f//elseOp02
		PushBLow 	regPBank
		SUB 		rscratch, rpc, regpcbase
		ADD 		rscratch2, rscratch, #1
		PushWLow 	rscratch2
		// PackStatus
		PushB	 	rstatus
		ClearDecimal
		SetIRQ
		BIC 		regPBank, regPBank,#0xFF
		MOV 		rscratch, #0xE4
		ORR 		rscratch, rscratch, #0xFF00
		S9xGetWordLow 		
		S9xSetPCBase 	
		ADD2CYCLE
		B 2022f//endOp02
2021://elseOp02
		SUB 		rscratch2, rpc, regpcbase
		PushWLow 	rscratch2
		// PackStatus
		PushB	 	rstatus
		ClearDecimal
		SetIRQ
		BIC 		regPBank,regPBank, #0xFF
		MOV 		rscratch, #0xF4
		ORR 		rscratch, rscratch, #0xFF00
		S9xGetWordLow 		
		S9xSetPCBase 	
		ADD1CYCLE
2022://endOp02
.endm

/**********************************************************************************************/
/* JML *************************************************************************************** */
.macro		OpDC		
		AbsoluteIndirectLong		
		BIC		regPBank,regPBank,#0xFF
		ORR 		regPBank,regPBank, rscratch, LSR #16
		S9xSetPCBase 	
		ADD2CYCLE
.endm
.macro		Op5C		
		AbsoluteLong		
		BIC		regPBank,regPBank,#0xFF
		ORR 		regPBank,regPBank, rscratch, LSR #16
		S9xSetPCBase 	
.endm

/**********************************************************************************************/
/* JMP *************************************************************************************** */
.macro		Op4C
		Absolute
		BIC		rscratch, rscratch, #0xFF0000
		ORR		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase
		CPUShutdown
.endm		
.macro		Op6C
		AbsoluteIndirect
		BIC		rscratch, rscratch, #0xFF0000
		ORR		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase		
.endm		
.macro		Op7C						
		ADD 		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase 	
		ADD1CYCLE
.endm

/**********************************************************************************************/
/* JSL/RTL *********************************************************************************** */
.macro		Op22				
		PushBlow	regPBank
		SUB 		rscratch, rpc, regpcbase
		//SUB 		rscratch2, rscratch2, #1
		ADD 		rscratch2, rscratch, #2
		PushWlow	rscratch2
		AbsoluteLong		
		BIC		regPBank,regPBank,#0xFF
		ORR 		regPBank, regPBank, rscratch, LSR #16
		S9xSetPCBase 	
.endm
.macro		Op6B		
		PullWLow 	rpc		
		BIC		regPBank,regPBank,#0xFF
		PullBrLow 			
		ORR 		regPBank, regPBank, rscratch
		ADD 		rscratch, rpc, #1
		BIC		rscratch, rscratch,#0xFF0000
		ORR		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase
		ADD2CYCLE
.endm
/**********************************************************************************************/
/* JSR/RTS *********************************************************************************** */
.macro		Op20				
		SUB 		rscratch, rpc, regpcbase
		//SUB 		rscratch2, rscratch2, #1
		ADD 		rscratch2, rscratch, #1		
		PushWlow	rscratch2				
		Absolute		
		BIC		rscratch, rscratch, #0xFF0000		
		ORR 		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase 
		ADD1CYCLE
.endm
.macro		OpFCX0
		SUB 		rscratch, rpc, regpcbase
		//SUB 		rscratch2, rscratch2, #1
		ADD		rscratch2, rscratch, #1
		PushWlow	rscratch2
		AbsoluteIndexedIndirectX0
		ORR 		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase
		ADD1CYCLE
.endm
.macro		OpFCX1
		SUB 		rscratch, rpc, regpcbase
		//SUB 		rscratch2, rscratch2, #1
		ADD		rscratch2, rscratch, #1		
		PushWlow	rscratch2	
		AbsoluteIndexedIndirectX1
		ORR 		rscratch, rscratch, regPBank, LSL #16
		S9xSetPCBase 
		ADD1CYCLE
.endm
.macro		Op60			
		PullWLow 	rpc
		ADD 		rscratch, rpc, #1		
		BIC		rscratch, rscratch,#0x10000		
		ORR		rscratch, rscratch, regPBank, LSL #16		
		S9xSetPCBase 
		ADD3CYCLE
.endm

/**********************************************************************************************/
/* MVN/MVP *********************************************************************************** */		
.macro		Op54X1M1
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #24		
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #24
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2	
		//load 16bits A		
		LDRB		rscratch,[regCPUvar,#RAH_ofs]
		MOV		regA,regA,LSR #8
		ORR		regA,regA,rscratch, LSL #24
		ADD		regX, regX, #0x01000000
		SUB		regA, regA, #0x00010000
		ADD		regY, regY, #0x01000000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                //update AH
                MOV		rscratch, regA, LSR #24
                MOV		regA,regA,LSL #8
                STRB		rscratch,[regCPUvar,#RAH_ofs]                
                ADD2CYCLE2MEM
.endm
.macro		Op54X1M0
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #24		
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #24
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2		
		ADD		regX, regX, #0x01000000
		SUB		regA, regA, #0x00010000
		ADD		regY, regY, #0x01000000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                ADD2CYCLE2MEM
.endm
.macro		Op54X0M1
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #16
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #16
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2		
		//load 16bits A		
		LDRB		rscratch,[regCPUvar,#RAH_ofs]
		MOV		regA,regA,LSR #8
		ORR		regA,regA,rscratch, LSL #24
		ADD		regX, regX, #0x00010000
		SUB		regA, regA, #0x00010000
		ADD		regY, regY, #0x00010000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3                
                //update AH
                MOV		rscratch, regA, LSR #24
                MOV		regA,regA,LSL #8
                STRB		rscratch,[regCPUvar,#RAH_ofs]                
                ADD2CYCLE2MEM
.endm
.macro		Op54X0M0
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #16
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #16
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2		
		ADD		regX, regX, #0x00010000
		SUB		regA, regA, #0x00010000
		ADD		regY, regY, #0x00010000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                ADD2CYCLE2MEM
.endm

.macro		Op44X1M1
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #24		
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #24
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2
		//load 16bits A		
		LDRB		rscratch,[regCPUvar,#RAH_ofs]
		MOV		regA,regA,LSR #8
		ORR		regA,regA,rscratch, LSL #24
		SUB		regX, regX, #0x01000000
		SUB		regA, regA, #0x00010000
		SUB		regY, regY, #0x01000000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                //update AH
                MOV		rscratch, regA, LSR #24
                MOV		regA,regA,LSL #8
                STRB		rscratch,[regCPUvar,#RAH_ofs]                
                ADD2CYCLE2MEM
.endm
.macro		Op44X1M0
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #24		
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #24
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2		
		SUB		regX, regX, #0x01000000
		SUB		regA, regA, #0x00010000
		SUB		regY, regY, #0x01000000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                ADD2CYCLE2MEM
.endm
.macro		Op44X0M1
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #16
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #16
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2
		//load 16bits A		
		LDRB		rscratch,[regCPUvar,#RAH_ofs]
		MOV		regA,regA,LSR #8
		ORR		regA,regA,rscratch, LSL #24
		SUB		regX, regX, #0x00010000
		SUB		regA, regA, #0x00010000
		SUB		regY, regY, #0x00010000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                //update AH
                MOV		rscratch, regA, LSR #24
                MOV		regA,regA,LSL #8
                STRB		rscratch,[regCPUvar,#RAH_ofs]                
                ADD2CYCLE2MEM
.endm
.macro		Op44X0M0
		//Save RegStatus = regDBank >> 24
		MOV		rscratch, regDBank, LSR #16
                LDRB		regDBank    , [rpc], #1
		LDRB		rscratch2    , [rpc], #1
		//Restore RegStatus = regDBank >> 24
		ORR		regDBank, regDBank, rscratch, LSL #16
		MOV		rscratch    , regX, LSR #16
                ORR		rscratch    , rscratch, rscratch2, LSL #16                
		S9xGetByteLow 
		MOV		rscratch2, rscratch
		MOV		rscratch   , regY, LSR #16
		ORR		rscratch   , rscratch, regDBank, LSL #16		
		S9xSetByteLow 	rscratch2		
		SUB		regX, regX, #0x00010000
		SUB		regA, regA, #0x00010000
		SUB		regY, regY, #0x00010000				
                CMP		regA, #0xFFFF0000
                SUBNE		rpc, rpc, #3
                ADD2CYCLE2MEM
.endm

/**********************************************************************************************/
/* REP/SEP *********************************************************************************** */
.macro		OpC2
		// status&=~(*rpc++);
		// so possible changes are :		
		// INDEX = 1 -> 0  : X,Y 8bits -> 16bits
		// MEM = 1 -> 0 : A 8bits -> 16bits
		//SAVE OLD status for MASK_INDEX & MASK_MEM comparison
		MOV		rscratch3, rstatus
		LDRB		rscratch, [rpc], #1
		MVN		rscratch, rscratch		
		AND		rstatus,rstatus,rscratch, ROR #(32-STATUS_SHIFTER)
		TST		rstatus,#MASK_EMUL
		BEQ		1111f
		//emulation mode on : no changes since it was on before opcode
		//just be sure to reset MEM & INDEX accordingly
		ORR		rstatus,rstatus,#(MASK_MEM|MASK_INDEX)		
		B		1112f
1111:		
		//NOT in Emulation mode, check INDEX & MEMORY bits
		//Now check INDEX
		TST		rscratch3,#MASK_INDEX
		BEQ		1113f		
		// X & Y were 8bit before
		TST		rstatus,#MASK_INDEX
		BNE		1113f
		// X & Y are now 16bits
		MOV		regX,regX,LSR #8
		MOV		regY,regY,LSR #8
1113:		//X & Y still in 16bits
		//Now check MEMORY
		TST		rscratch3,#MASK_MEM
		BEQ		1112f		
		// A was 8bit before
		TST		rstatus,#MASK_MEM
		BNE		1112f
		// A is now 16bits
		MOV		regA,regA,LSR #8		
		//restore AH
    		LDREQB		rscratch,[regCPUvar,#RAH_ofs]    		
    		ORREQ		regA,regA,rscratch,LSL #24
1112:
		S9xFixCycles
		ADD1CYCLE1MEM
.endm
.macro		OpE2
		// status|=*rpc++;
		// so possible changes are :
		// INDEX = 0 -> 1  : X,Y 16bits -> 8bits
		// MEM = 0 -> 1 : A 16bits -> 8bits
		//SAVE OLD status for MASK_INDEX & MASK_MEM comparison
		MOV		rscratch3, rstatus
		LDRB		rscratch, [rpc], #1		
		ORR		rstatus,rstatus,rscratch, LSL #STATUS_SHIFTER
		TST		rstatus,#MASK_EMUL
		BEQ		10111f
		//emulation mode on : no changes sinc eit was on before opcode
		//just be sure to have mem & index set accordingly
		ORR		rstatus,rstatus,#(MASK_MEM|MASK_INDEX)		
		B		10112f
10111:		
		//NOT in Emulation mode, check INDEX & MEMORY bits
		//Now check INDEX
		TST		rscratch3,#MASK_INDEX
		BNE		10113f		
		// X & Y were 16bit before
		TST		rstatus,#MASK_INDEX
		BEQ		10113f
		// X & Y are now 8bits
		MOV		regX,regX,LSL #8
		MOV		regY,regY,LSL #8
10113:		//X & Y still in 16bits
		//Now check MEMORY
		TST		rscratch3,#MASK_MEM
		BNE		10112f		
		// A was 16bit before
		TST		rstatus,#MASK_MEM
		BEQ		10112f
		// A is now 8bits
		// save AH
		MOV		rscratch,regA,LSR #24
		MOV		regA,regA,LSL #8	
		STRB		rscratch,[regCPUvar,#RAH_ofs]	
10112:
		S9xFixCycles
		ADD1CYCLE1MEM
.endm

/**********************************************************************************************/
/* XBA *************************************************************************************** */
.macro		OpEBM1		
		//A is 8bits
		ADD		rscratch,regCPUvar,#RAH_ofs
		MOV		regA,regA, LSR #24
		SWPB		regA,regA,[rscratch]
		MOVS		regA,regA, LSL #24
		UPDATE_ZN
		ADD2CYCLE
.endm
.macro		OpEBM0		
		//A is 16bits
		MOV 		rscratch, regA, ROR #24 // ll0000hh
		ORR 		rscratch, rscratch, regA, LSR #8// ll0000hh + 00hhll00 -> llhhllhh
		MOV 		regA, rscratch, LSL #16// llhhllhh -> llhh0000		
		MOVS		rscratch,rscratch,LSL #24 //to set Z & N flags with AL		
		UPDATE_ZN
		ADD2CYCLE
.endm


/**********************************************************************************************/
/* RTI *************************************************************************************** */
.macro		Op40X1M1
		//INDEX set, MEMORY set		
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		PullWlow	rpc
		TST 		rstatus, #MASK_EMUL
		ORRNE		rstatus, rstatus, #(MASK_MEM|MASK_INDEX)
                BNE		2401f
		PullBrLow
		BIC		regPBank,regPBank,#0xFF
		ORR		regPBank,regPBank,rscratch
2401:		
		ADD 		rscratch, rpc, regPBank, LSL #16
		S9xSetPCBase
		TST 		rstatus, #MASK_INDEX		
		//INDEX cleared & was set : 8->16
		MOVEQ		regX,regX,LSR #8
		MOVEQ		regY,regY,LSR #8
		TST 		rstatus, #MASK_MEM		
		//MEMORY cleared & was set : 8->16
		LDREQB		rscratch,[regCPUvar,#RAH_ofs]		
		MOVEQ		regA,regA,LSR #8		
		ORREQ		regA,regA,rscratch, LSL #24		
		ADD2CYCLE
		S9xFixCycles
.endm
.macro		Op40X0M1
		//INDEX cleared, MEMORY set		
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		PullWlow	rpc
		TST 		rstatus, #MASK_EMUL
		ORRNE		rstatus, rstatus, #(MASK_MEM|MASK_INDEX)
                BNE		2401f
		PullBrLow
		BIC		regPBank,regPBank,#0xFF
		ORR		regPBank,regPBank,rscratch
2401:		
		ADD 		rscratch, rpc, regPBank, LSL #16
		S9xSetPCBase		
		TST 		rstatus, #MASK_INDEX		
		//INDEX set & was cleared : 16->8
		MOVNE		regX,regX,LSL #8
		MOVNE		regY,regY,LSL #8		
		TST 		rstatus, #MASK_MEM		
		//MEMORY cleared & was set : 8->16
		LDREQB		rscratch,[regCPUvar,#RAH_ofs]		
		MOVEQ		regA,regA,LSR #8		
		ORREQ		regA,regA,rscratch, LSL #24
		ADD2CYCLE
		S9xFixCycles
.endm
.macro		Op40X1M0
		//INDEX set, MEMORY cleared
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		PullWlow	rpc
		TST 		rstatus, #MASK_EMUL
		ORRNE		rstatus, rstatus, #(MASK_MEM|MASK_INDEX)
                BNE		2401f
		PullBrLow
		BIC		regPBank,regPBank,#0xFF
		ORR		regPBank,regPBank,rscratch
2401:		
		ADD 		rscratch, rpc, regPBank, LSL #16
		S9xSetPCBase
		TST 		rstatus, #MASK_INDEX		
		//INDEX cleared & was set : 8->16
		MOVEQ		regX,regX,LSR #8
		MOVEQ		regY,regY,LSR #8		
		TST 		rstatus, #MASK_MEM		
		//MEMORY set & was cleared : 16->8
		MOVNE		rscratch,regA,LSR #24
		MOVNE		regA,regA,LSL #8
		STRNEB		rscratch,[regCPUvar,#RAH_ofs]
		ADD2CYCLE
		S9xFixCycles
.endm
.macro		Op40X0M0
		//INDEX cleared, MEMORY cleared
		BIC		rstatus,rstatus,#0xFF000000
		PullBr
		ORR		rstatus,rscratch,rstatus
		PullWlow	rpc
		TST 		rstatus, #MASK_EMUL
		ORRNE		rstatus, rstatus, #(MASK_MEM|MASK_INDEX)
                BNE		2401f
		PullBrLow
		BIC		regPBank,regPBank,#0xFF
		ORR		regPBank,regPBank,rscratch
2401:		
		ADD 		rscratch, rpc, regPBank, LSL #16
		S9xSetPCBase
		TST 		rstatus, #MASK_INDEX
		//INDEX set & was cleared : 16->8
		MOVNE		regX,regX,LSL #8
		MOVNE		regY,regY,LSL #8		
		TST 		rstatus, #MASK_MEM		
		//MEMORY set & was cleared : 16->8
		//MEMORY set & was cleared : 16->8
		MOVNE		rscratch,regA,LSR #24
		MOVNE		regA,regA,LSL #8
		STRNEB		rscratch,[regCPUvar,#RAH_ofs]
		ADD2CYCLE
		S9xFixCycles
.endm
	

/**********************************************************************************************/
/* STP/WAI/DB ******************************************************************************** */
// WAI
.macro		OpCB	/*WAI*/
	LDRB		rscratch,[regCPUvar,#IRQActive_ofs]
	MOVS		rscratch,rscratch
	//(CPU.IRQActive)
	ADD2CYCLENE
	BNE		1234f
/*
	CPU.WaitingForInterrupt = TRUE;
	CPU.PC--;*/	
	MOV		rscratch,#1
	SUB		rpc,rpc,#1
/*		
	    CPU.Cycles = CPU.NextEvent;	    
*/		
	STRB		rscratch,[regCPUvar,#WaitingForInterrupt_ofs]
	LDR		regCycles,[regCPUvar,#NextEvent_ofs]
/*
	if (IAPU.APUExecuting)
	    {
		ICPU.CPUExecuting = FALSE;
		do
		{
		    APU_EXECUTE1 ();
		} while (APU.Cycles < CPU.NextEvent);
		ICPU.CPUExecuting = TRUE;
	    }	
*/	
	LDRB		rscratch,[regCPUvar,#APUExecuting_ofs]
	MOVS		rscratch,rscratch
	BEQ		1234f
	asmAPU_EXECUTE2	

1234:	
.endm
.macro		OpDB	/*STP*/    
    		SUB	rpc,rpc,#1
    		//CPU.Flags |= DEBUG_MODE_FLAG;
.endm
.macro		Op42   /*Reserved Snes9X*/
.endm	
		
/**********************************************************************************************/
/* AND ******************************************************************************** */
.macro		Op29M1
		LDRB	rscratch    , [rpc], #1		
		ANDS	regA    , regA,	rscratch, LSL #24
		UPDATE_ZN
		ADD1MEM
.endm		
.macro		Op29M0		
		LDRB	rscratch2  , [rpc,#1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch, rscratch, rscratch2, LSL #8		
		ANDS	regA    , regA,	rscratch, LSL #16
		UPDATE_ZN
		ADD2MEM
.endm

		


		

		

		

		

		

		
/**********************************************************************************************/
/* EOR ******************************************************************************** */
.macro		Op49M0		
                LDRB	rscratch2 , [rpc, #1]
                LDRB	rscratch , [rpc], #2
		ORR	rscratch, rscratch, rscratch2,LSL #8                
		EORS    regA, regA, rscratch,LSL #16
		UPDATE_ZN
		ADD2MEM
.endm

		
.macro		Op49M1		
                LDRB	rscratch , [rpc], #1                
		EORS    regA, regA, rscratch,LSL #24
		UPDATE_ZN
		ADD1MEM
.endm


/**********************************************************************************************/
/* STA *************************************************************************************** */		
.macro		Op81M1				
		STA8
		//TST 		rstatus, #MASK_INDEX
		//ADD1CYCLENE
.endm
.macro		Op81M0				
		STA16
		//TST rstatus, #MASK_INDEX
		//ADD1CYCLENE
.endm


/**********************************************************************************************/
/* BIT *************************************************************************************** */
.macro		Op89M1		
                LDRB	rscratch , [rpc], #1                
		TST     regA, rscratch, LSL #24
		UPDATE_Z
		ADD1MEM
.endm
.macro		Op89M0		
                LDRB	rscratch2 , [rpc, #1]
                LDRB	rscratch , [rpc], #2
		ORR	rscratch, rscratch, rscratch2, LSL #8                
		TST     regA, rscratch, LSL #16
		UPDATE_Z
		ADD2MEM
.endm

		

		
		

/**********************************************************************************************/
/* LDY *************************************************************************************** */
.macro		OpA0X1
                LDRB	rscratch , [rpc], #1                
                MOVS    regY, rscratch, LSL #24
		UPDATE_ZN
		ADD1MEM
.endm
.macro		OpA0X0		
                LDRB	rscratch2 , [rpc, #1]
                LDRB	rscratch , [rpc], #2
		ORR	rscratch, rscratch, rscratch2, LSL #8                
                MOVS    regY, rscratch, LSL #16
		UPDATE_ZN
		ADD2MEM
.endm

/**********************************************************************************************/
/* LDX *************************************************************************************** */		
.macro		OpA2X1		
                LDRB	rscratch , [rpc], #1                
                MOVS    regX, rscratch, LSL #24
		UPDATE_ZN
		ADD1MEM
.endm
.macro		OpA2X0		
                LDRB	rscratch2 , [rpc, #1]
                LDRB	rscratch , [rpc], #2
		ORR	rscratch, rscratch, rscratch2, LSL #8                
                MOVS    regX, rscratch, LSL #16
		UPDATE_ZN
		ADD2MEM
.endm
		
/**********************************************************************************************/
/* LDA *************************************************************************************** */		
.macro		OpA9M1		
                LDRB	rscratch , [rpc], #1
                MOVS    regA, rscratch, LSL #24
		UPDATE_ZN
		ADD1MEM
.endm
.macro		OpA9M0		
                LDRB	rscratch2 , [rpc, #1]
                LDRB	rscratch , [rpc], #2
		ORR	rscratch, rscratch, rscratch2, LSL #8                
                MOVS    regA, rscratch, LSL #16                
		UPDATE_ZN
		ADD2MEM
.endm
												
/**********************************************************************************************/
/* CMY *************************************************************************************** */
.macro		OpC0X1
		LDRB	rscratch    , [rpc], #1		
		SUBS	rscratch2   , regY , rscratch, LSL #24
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN		
		ADD1MEM
.endm
.macro		OpC0X0
		LDRB	rscratch2   , [rpc, #1]
		LDRB	rscratch   , [rpc], #2		
		ORR	rscratch, rscratch, rscratch2, LSL #8
		SUBS	rscratch2   , regY, rscratch, LSL #16
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		ADD2MEM
.endm

		

		

/**********************************************************************************************/
/* CMP *************************************************************************************** */		
.macro		OpC9M1		
		LDRB	rscratch    , [rpc], #1		
		SUBS	rscratch2   , regA , rscratch, LSL #24		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		ADD1MEM
.endm
.macro		OpC9M0		
		LDRB	rscratch2   , [rpc,#1]
		LDRB	rscratch   , [rpc], #2		
		ORR	rscratch, rscratch, rscratch2, LSL #8
		SUBS	rscratch2   , regA, rscratch, LSL #16		
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		ADD2MEM
.endm

/**********************************************************************************************/
/* CMX *************************************************************************************** */		
.macro		OpE0X1		
		LDRB	rscratch    , [rpc], #1		
		SUBS	rscratch2   , regX , rscratch, LSL #24
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN		
		ADD1MEM
.endm
.macro		OpE0X0		
		LDRB	rscratch2   , [rpc,#1]
		LDRB	rscratch   , [rpc], #2		
		ORR	rscratch, rscratch, rscratch2, LSL #8
		SUBS	rscratch2   , regX, rscratch, LSL #16
		BICCC	rstatus, rstatus, #MASK_CARRY
		ORRCS	rstatus, rstatus, #MASK_CARRY
		UPDATE_ZN
		ADD2MEM
.endm

/*


CLI_OPE_REC_Nos_Layer0 
  	nos.nos_ope_treasury_date = convert(DATETIME, @treasuryDate, 103)
    	nos.nos_ope_accounting_date = convert(DATETIME, @accountingDate, 103)

CLI_OPE_Nos_Ope_Layer0
	n.nos_ope_treasury_date = convert(DATETIME, @LARD, 103)
	n.nos_ope_accounting_date = convert(DATETIME, @LARD, 103)
    	
CLI_OPE_Nos_Layer0    	
	nos.nos_ope_treasury_date = convert(DATETIME, @LARD, 103)
	nos.nos_ope_accounting_date = convert(DATETIME, @LARD, 103)    	
	
Ecrans:
------


[GNV] : utilisation de la lard (laccdate) pour afficher les openings.
   +nécessité d'avoir des valeurs dans l'opening pour date tréso=date compta=laccdate
	
[Accounting rec] : si laccdate pas bonne (pas = BD-1) -> message warning et pas de donnée
sinon : 
  +données nécessaires : opening date tréso=date compta=laccdate=BD-1
  +données nécessaires : opening date tréso=date compta=laccdate-1
  +données nécessaires : opening date tréso=laccdate-1 et date compta=laccdate
   */


/*





*/
