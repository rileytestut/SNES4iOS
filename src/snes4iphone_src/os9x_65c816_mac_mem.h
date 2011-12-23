/*****************************************************************

*****************************************************************/

//#define _C_GB_
//#define _C_GW_
//#define _C_SB_
//#define _C_SW_

.macro		S9xGetWord	
		// in  : rscratch (0x00hhmmll)
		// out : rscratch (0xhhll0000)
#ifdef  _C_GW_
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetWord
		RESTORE_C_CALL
		MOV	R0, R0, LSL #16
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetWord
		MOV	R0,R0
		MOV	R0, R0, LSL #16
#endif
.endm
.macro		S9xGetWordLow	
		// in  : rscratch (0x00hhmmll)
		// out : rscratch (0x0000hhll)		
#ifdef  _C_GW_
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetWord
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetWord
		MOV	R0,R0		
#endif		
.endm
.macro		S9xGetWordRegStatus	reg
		// in  : rscratch (0x00hhmmll) 
		// out : reg      (0xhhll0000)
		// flags have to be updated with read value
#ifdef  _C_GW_
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetWord		
		RESTORE_C_CALL
		MOVS	\reg, R0, LSL #16
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetWord
		MOV	R0,R0
		MOVS	\reg, R0, LSL #16
#endif		
.endm
.macro		S9xGetWordRegNS	reg
		// in  : rscratch (0x00hhmmll) 
		// out : reg (0xhhll0000)
		// DOES NOT DESTROY rscratch (R0)
#ifdef  _C_GW_
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL_R0
		BL	asm_S9xGetWord		
		MOV	\reg, R0, LSL #16
		RESTORE_C_CALL_R0
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{R0}
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetWord
		MOV	R0,R0
		MOV	\reg, R0, LSL #16
		LDMFD	R13!,{R0}
#endif		
.endm			
.macro		S9xGetWordLowRegNS	reg
		// in  : rscratch (0x00hhmmll) 
		// out : reg (0xhhll0000)
		// DOES NOT DESTROY rscratch (R0)
#ifdef  _C_GW_
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL_R0
		BL	asm_S9xGetWord		
		MOV	\reg, R0
		RESTORE_C_CALL_R0
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{R0}
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetWord
		MOV	R0,R0
		MOV	\reg, R0
		LDMFD	R13!,{R0}
#endif		
.endm			

.macro		S9xGetByte 	
		// in  : rscratch (0x00hhmmll)
		// out : rscratch (0xll000000)
#ifdef  _C_GB_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetByte						
		RESTORE_C_CALL
		MOV	R0, R0, LSL #24
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetByte
		MOV	R0,R0
		MOV	R0, R0, LSL #24
#endif		
.endm
.macro		S9xGetByteLow
		// in  : rscratch (0x00hhmmll) 
		// out : rscratch (0x000000ll)		
#ifdef  _C_GB_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetByte
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC}		
		B	asmS9xGetByte
		MOV	R0,R0
#endif		
.endm
.macro		S9xGetByteRegStatus	reg
		// in  : rscratch (0x00hhmmll)
		// out : reg      (0xll000000)
		// flags have to be updated with read value
#ifdef  _C_GB_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL
		BL	asm_S9xGetByte
		RESTORE_C_CALL
		MOVS	\reg, R0, LSL #24
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetByte
		MOV	R0,R0
		MOVS	\reg, R0, LSL #24
#endif		
.endm
.macro		S9xGetByteRegNS	reg
		// in  : rscratch (0x00hhmmll) 
		// out : reg      (0xll000000)
		// DOES NOT DESTROY rscratch (R0)
#ifdef  _C_GB_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		PREPARE_C_CALL_R0
		BL	asm_S9xGetByte		
		MOV	\reg, R0, LSL #24
		RESTORE_C_CALL_R0
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else
		STMFD	R13!,{R0}
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetByte
		MOV	R0,R0
		MOVS	\reg, R0, LSL #24
		LDMFD	R13!,{R0}
#endif		
.endm
.macro		S9xGetByteLowRegNS	reg
		// in  : rscratch (0x00hhmmll) 
		// out : reg      (0x000000ll)
		// DOES NOT DESTROY rscratch (R0)
#ifdef  _C_GB_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		PREPARE_C_CALL_R0
		BL	asm_S9xGetByte		
		MOV	\reg, R0, LSL #24
		RESTORE_C_CALL_R0
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else
		STMFD	R13!,{R0}
		STMFD	R13!,{PC} //Push return address
		B	asmS9xGetByte
		MOV	R0,R0
		MOVS	\reg, R0
		LDMFD	R13!,{R0}
#endif		
.endm

.macro		S9xSetWord	regValue		
		// in  : regValue  (0xhhll0000)
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SW_		
		STR	regCycles,[regCPUvar,#Cycles_ofs]
		MOV	R1,\regValue, LSR #16
		PREPARE_C_CALL
		BL	asm_S9xSetWord			
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,\regValue, LSR #16
		B	asmS9xSetWord
		MOV	R0,R0		
#endif		
.endm
.macro		S9xSetWordZero	
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SW_				
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		MOV	R1,#0
		PREPARE_C_CALL
		BL	asm_S9xSetWord			
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,#0
		B	asmS9xSetWord
		MOV	R0,R0		
#endif		
.endm
.macro		S9xSetWordLow	regValue		
		// in  : regValue  (0x0000hhll)
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SW_				
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		MOV	R1,\regValue
		PREPARE_C_CALL
		BL	asm_S9xSetWord			
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]		
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,\regValue
		B	asmS9xSetWord
		MOV	R0,R0		
#endif		
.endm
.macro		S9xSetByte	regValue
		// in  : regValue  (0xll000000)
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SB_
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		MOV	R1,\regValue, LSR #24
		PREPARE_C_CALL
		BL	asm_S9xSetByte
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,\regValue, LSR #24
		B	asmS9xSetByte
		MOV	R0,R0		
#endif		
.endm
.macro		S9xSetByteZero			
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SB_				
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		MOV	R1,#0
		PREPARE_C_CALL
		BL	asm_S9xSetByte
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,#0
		B	asmS9xSetByte
		MOV	R0,R0		
#endif		
.endm
.macro		S9xSetByteLow	regValue
		// in  : regValue  (0x000000ll)
		// in  : rscratch=address   (0x00hhmmll)
#ifdef  _C_SB_
		STR	regCycles,[regCPUvar,#Cycles_ofs]		
		MOV	R1,\regValue
		PREPARE_C_CALL
		BL	asm_S9xSetByte
		RESTORE_C_CALL
		LDR	regCycles,[regCPUvar,#Cycles_ofs]
#else		
		STMFD	R13!,{PC} //Push return address
		MOV	R1,\regValue
		B	asmS9xSetByte
		MOV	R0,R0
#endif		
.endm


// ===========================================
// ===========================================
// Adressing mode
// ===========================================
// ===========================================


.macro		Absolute		
		ADD2MEM		
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc],#2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ORR	rscratch    , rscratch,	regDBank, LSL #16
.endm
.macro		AbsoluteIndexedIndirectX0
		ADD2MEM		
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ADD	rscratch    , regX, rscratch, LSL #16
		MOV	rscratch , rscratch, LSR #16
		ORR	rscratch    , rscratch,	regPBank, LSL #16
		S9xGetWordLow
		
.endm
.macro		AbsoluteIndexedIndirectX1
		ADD2MEM		
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ADD	rscratch    , rscratch, regX, LSR #24
		BIC	rscratch , rscratch, #0x00FF0000
		ORR	rscratch    , rscratch,	regPBank, LSL #16
		S9xGetWordLow
		
.endm
.macro		AbsoluteIndirectLong		
		ADD2MEM
		LDRB			rscratch2    , [rpc, #1]
		LDRB			rscratch   , [rpc], #2
		ORR			rscratch    , rscratch,	rscratch2, LSL #8
		S9xGetWordLowRegNS 	rscratch2
		ADD			rscratch   , rscratch,	#2
		STMFD			r13!,{rscratch2}
		S9xGetByteLow
		LDMFD			r13!,{rscratch2}
		ORR			rscratch   , rscratch2, rscratch, LSL #16
.endm
.macro		AbsoluteIndirect
		ADD2MEM
		LDRB	rscratch2    , [rpc,#1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		S9xGetWordLow
		ORR	rscratch    , rscratch,	regPBank, LSL #16
.endm
.macro		AbsoluteIndexedX0		
		ADD2MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regX, LSR #16
.endm
.macro		AbsoluteIndexedX1
		ADD2MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regX, LSR #24
.endm


.macro		AbsoluteIndexedY0
		ADD2MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regY, LSR #16
.endm
.macro		AbsoluteIndexedY1
		ADD2MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regY, LSR #24
.endm
.macro		AbsoluteLong
		ADD3MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		LDRB	rscratch2   , [rpc], #1
		ORR	rscratch    , rscratch,	rscratch2, LSL #16
.endm


.macro		AbsoluteLongIndexedX0
		ADD3MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		LDRB	rscratch2   , [rpc], #1
		ORR	rscratch    , rscratch,	rscratch2, LSL #16
		ADD	rscratch    , rscratch,	regX, LSR #16
		BIC	rscratch, rscratch, #0xFF000000
.endm
.macro		AbsoluteLongIndexedX1
		ADD3MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		LDRB	rscratch2   , [rpc], #1
		ORR	rscratch    , rscratch,	rscratch2, LSL #16
		ADD	rscratch    , rscratch,	regX, LSR #24
		BIC	rscratch, rscratch, #0xFF000000		
.endm
.macro		Direct
		ADD1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , regD, rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
.endm
.macro		DirectIndirect
		ADD1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , regD, rscratch,	 LSL #16		
		MOV	rscratch, rscratch, LSR #16
		S9xGetWordLow
		ORR	rscratch    , rscratch,	regDBank, LSL #16
.endm
.macro		DirectIndirectLong
		ADD1MEM
		LDRB			rscratch    , [rpc], #1
		ADD			rscratch    , regD, rscratch,	 LSL #16
		MOV			rscratch, rscratch, LSR #16		
		S9xGetWordLowRegNS	rscratch2
		ADD			rscratch    , rscratch,#2
		STMFD			r13!,{rscratch2}
		S9xGetByteLow
		LDMFD			r13!,{rscratch2}
		ORR			rscratch   , rscratch2, rscratch, LSL #16
.endm
.macro		DirectIndirectIndexed0
		ADD1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , regD, rscratch,	 LSL #16
		MOV	rscratch, rscratch, LSR #16
		S9xGetWordLow
		ORR	rscratch, rscratch,regDBank, LSL #16
		ADD	rscratch, rscratch,regY, LSR #16
.endm
.macro		DirectIndirectIndexed1
		ADD1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , regD, rscratch,	 LSL #16
		MOV	rscratch, rscratch, LSR #16
		S9xGetWordLow
		ORR	rscratch, rscratch,regDBank, LSL #16
		ADD	rscratch, rscratch,regY, LSR #24
.endm
.macro		DirectIndirectIndexedLong0
		ADD1MEM
		LDRB			rscratch    , [rpc], #1
		ADD			rscratch    , regD, rscratch,	 LSL #16
		MOV			rscratch, rscratch, LSR #16		
		S9xGetWordLowRegNS	rscratch2
		ADD			rscratch    , rscratch,#2
		STMFD			r13!,{rscratch2}
		S9xGetByteLow
		LDMFD			r13!,{rscratch2}
		ORR			rscratch   , rscratch2, rscratch, LSL #16				
		ADD			rscratch, rscratch,regY, LSR #16
.endm
.macro		DirectIndirectIndexedLong1
		ADD1MEM
		LDRB			rscratch    , [rpc], #1
		ADD			rscratch    , regD, rscratch,	 LSL #16
		MOV			rscratch, rscratch, LSR #16
		S9xGetWordLowRegNS	rscratch2
		ADD			rscratch    , rscratch,#2
		STMFD			r13!,{rscratch2}
		S9xGetByteLow
		LDMFD			r13!,{rscratch2}
		ORR			rscratch   , rscratch2, rscratch, LSL #16
		ADD			rscratch, rscratch,regY, LSR #24
.endm
.macro		DirectIndexedIndirect0
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1				
		ADD	rscratch2   , regD , regX
		ADD	rscratch    , rscratch2 , rscratch, LSL #16		
		MOV	rscratch, rscratch, LSR #16
		S9xGetWordLow
		ORR	rscratch    , rscratch , regDBank, LSL #16		
.endm
.macro		DirectIndexedIndirect1
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch2   , regD , regX, LSR #8
		ADD	rscratch    , rscratch2 , rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
		S9xGetWordLow
		ORR	rscratch    , rscratch , regDBank, LSL #16		
.endm
.macro		DirectIndexedX0
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch2   , regD , regX
		ADD	rscratch    , rscratch2 , rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
.endm
.macro		DirectIndexedX1
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch2   , regD , regX, LSR #8
		ADD	rscratch    , rscratch2 , rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
.endm
.macro		DirectIndexedY0
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch2   , regD , regY
		ADD	rscratch    , rscratch2 , rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
.endm
.macro		DirectIndexedY1
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch2   , regD , regY, LSR #8
		ADD	rscratch    , rscratch2 , rscratch, LSL #16
		MOV	rscratch, rscratch, LSR #16
.endm
.macro		Immediate8
		ADD	rscratch, rpc, regPBank, LSL #16
		SUB	rscratch, rscratch, regpcbase
		ADD	rpc, rpc, #1
.endm
.macro		Immediate16
		ADD	rscratch, rpc, regPBank, LSL #16
		SUB	rscratch, rscratch, regpcbase
		ADD	rpc, rpc, #2
.endm
.macro		asmRelative
		ADD1MEM
		LDRSB	rscratch    , [rpc],#1
		ADD	rscratch , rscratch , rpc
		SUB	rscratch , rscratch, regpcbase		
		BIC	rscratch,rscratch,#0x00FF0000
		BIC	rscratch,rscratch,#0xFF000000
.endm
.macro		asmRelativeLong
		ADD1CYCLE2MEM
		LDRB	rscratch2    , [rpc, #1]
		LDRB	rscratch   , [rpc], #2
		ORR	rscratch    , rscratch,	rscratch2, LSL #8
		SUB	rscratch2    , rpc, regpcbase
		ADD	rscratch    , rscratch2, rscratch		
		BIC	rscratch,rscratch,#0x00FF0000
.endm


.macro		StackasmRelative
		ADD1CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , rscratch,	regS
		BIC	rscratch,rscratch,#0x00FF0000
.endm
.macro		StackasmRelativeIndirectIndexed0
		ADD2CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , rscratch,	regS
		BIC	rscratch,rscratch,#0x00FF0000
		S9xGetWordLow
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regY, LSR #16
		BIC	rscratch, rscratch, #0xFF000000
.endm
.macro		StackasmRelativeIndirectIndexed1
		ADD2CYCLE1MEM
		LDRB	rscratch    , [rpc], #1
		ADD	rscratch    , rscratch,	regS
		BIC	rscratch,rscratch,#0x00FF0000
		S9xGetWordLow
		ORR	rscratch    , rscratch,	regDBank, LSL #16
		ADD	rscratch    , rscratch,	regY, LSR #24
		BIC	rscratch, rscratch, #0xFF000000
.endm


/****************************************/
.macro		PushB		reg
		MOV		rscratch,regS
		S9xSetByte	\reg
		SUB		regS,regS,#1
.endm			
.macro		PushBLow	reg
		MOV		rscratch,regS
		S9xSetByteLow	\reg
		SUB		regS,regS,#1
.endm
.macro		PushWLow	reg 
		SUB		rscratch,regS,#1
		S9xSetWordLow	\reg
		SUB		regS,regS,#2
.endm			
.macro		PushWrLow	
		MOV		rscratch2,rscratch
		SUB		rscratch,regS,#1
		S9xSetWordLow	rscratch2
		SUB		regS,regS,#2
.endm			
.macro		PushW		reg
		SUB		rscratch,regS,#1
		S9xSetWord	\reg
		SUB		regS,regS,#2
.endm

/********/

.macro		PullB		reg
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOV		\reg,rscratch,LSL #24
.endm
.macro		PullBr		
		ADD		rscratch,regS,#1
		S9xGetByte
		ADD		regS,regS,#1		
.endm
.macro		PullBLow	reg
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOV		\reg,rscratch
.endm
.macro		PullBrLow
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1		
.endm
.macro		PullW		reg
		ADD		rscratch,regS,#1
		S9xGetWordLow
		ADD		regS,regS,#2
		MOV		\reg,rscratch,LSL #16
.endm

.macro		PullWLow	reg
		ADD		rscratch,regS,#1
		S9xGetWordLow	
		ADD		regS,regS,#2
		MOV		\reg,rscratch
.endm


/*****************/
.macro		PullBS		reg
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOVS		\reg,rscratch,LSL #24
.endm
.macro		PullBrS	
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOVS		rscratch,rscratch,LSL #24
.endm
.macro		PullBLowS	reg
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOVS		\reg,rscratch
.endm
.macro		PullBrLowS	
		ADD		rscratch,regS,#1
		S9xGetByteLow
		ADD		regS,regS,#1
		MOVS		rscratch,rscratch
.endm
.macro		PullWS		reg
		ADD		rscratch,regS,#1
		S9xGetWordLow
		ADD		regS,regS,#2
		MOVS		\reg,rscratch, LSL #16
.endm
.macro		PullWrS		
		ADD		rscratch,regS,#1
		S9xGetWordLow
		ADD		regS,regS,#2
		MOVS		rscratch,rscratch, LSL #16
.endm
.macro		PullWLowS	reg
		ADD		rscratch,regS,#1
		S9xGetWordLow
		ADD		regS,regS,#2
		MOVS		\reg,rscratch
.endm
.macro		PullWrLowS	
		ADD		rscratch,regS,#1
		S9xGetWordLow
		ADD		regS,regS,#2
		MOVS		rscratch,rscratch
.endm


.globl asmS9xGetByte
.globl asmS9xGetWord
.globl asmS9xSetByte
.globl asmS9xSetWord

//uint8 aaS9xGetByte(uint32 address);
asmS9xGetByte:
	// in : R0  = 0x00hhmmll
	// out : R0 = 0x000000ll
	// DESTROYED : R1,R2,R3
	// UPDATE : regCycles
	//R1 <= block	
	MOV		R1,R0,LSR #MEMMAP_SHIFT
	//MEMMAP_SHIFT is 12, Address is 0xFFFFFFFF at max, so
	//R1 is maxed by 0x000FFFFF, MEMMAP_MASK is 0x1000-1=0xFFF
	//so AND MEMMAP_MASK is BIC 0xFF000
	BIC		R1,R1,#0xFF000
	//R2 <= Map[block] (GetAddress)
	LDR		R2,[regCPUvar,#Map_ofs]
	LDR		R2,[R2,R1,LSL #2]
	CMP		R2,#MAP_LAST
	BLO		GBSpecial  //special
	// Direct ROM/RAM acess
	//R2 <= GetAddress + Address & 0xFFFF	
	//R3 <= MemorySpeed[block]			
	LDR		R3,[regCPUvar,#MemorySpeed_ofs]
	MOV		R0,R0,LSL #16		
	LDRB		R3,[R3,R1]
	ADD		R2,R2,R0,LSR #16
	//Update CPU.Cycles
	ADD		regCycles,regCycles,R3	
	//R3 = BlockIsRAM[block]
	LDR		R3,[regCPUvar,#BlockIsRAM_ofs]
	//Get value to return
	LDRB		R0,[R2]
	LDRB		R3,[R3,R1]
	MOVS		R3,R3
	// if BlockIsRAM => update for CPUShutdown
	LDRNE		R1,[regCPUvar,#PCAtOpcodeStart_ofs]
	STRNE		R1,[regCPUvar,#WaitAddress_ofs]
	
	LDMFD		R13!,{PC} //Return
GBSpecial:
	
#ifdef __PALMOS__	
	LDR		R3,[regCPUvar,#PALMOS_R10_ofs]
	LDR		R2,[PC,R2,LSL #2]
	ADD		PC,R2,R3	
#else	
	LDR		PC,[PC,R2,LSL #2]
	MOV		R0,R0 		//nop, for align
#endif	
	.long GBPPU
	.long GBCPU
	.long GBDSP
	.long GBLSRAM
	.long GBHSRAM
	.long GBNONE
	.long GBDEBUG
	.long GBC4
	.long GBBWRAM
	.long GBNONE
	.long GBNONE
	.long GBNONE
	/*.long GB7ROM
	.long GB7RAM
	.long GB7SRM*/
GBPPU:
	//InDMA ?
	LDRB		R1,[regCPUvar,#InDMA_ofs]
	MOVS		R1,R1	
	ADDEQ		regCycles,regCycles,#ONE_CYCLE		//No -> update Cycles
	MOV		R0,R0,LSL #16	//S9xGetPPU(Address&0xFFFF);
	STR		regCycles,[regCPUvar,#Cycles_ofs]	//Save Cycles
	MOV		R0,R0,LSR #16	
		PREPARE_C_CALL
	BL		S9xGetPPU
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GBCPU:	
	ADD		regCycles,regCycles,#ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetCPU(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL
	BL		S9xGetCPU
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GBDSP:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetCPU(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL
	BL		S9xGetDSP		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GBLSRAM:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles		
	LDRH		R2,[regCPUvar,#SRAMMask]
	LDR		R1,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask
	LDRB		R0,[R1,R0]		//*Memory.SRAM + Address&SRAMMask
	LDMFD		R13!,{PC}
GB7SRM:	
GBHSRAM:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles		
	
	MOV		R1,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R1,R1,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)
	ADD		R0,R2,R1
	LDRH		R2,[regCPUvar,#SRAMMask]
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	LDR		R1,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask	
	LDRB		R0,[R1,R0]		//*Memory.SRAM + Address&SRAMMask
	LDMFD		R13!,{PC}		//return
GB7ROM:
GB7RAM:	
GBNONE:
	MOV		R0,R0,LSR #8
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles
	AND		R0,R0,#0xFF
	LDMFD		R13!,{PC}
//GBDEBUG:
	/*ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles
	MOV		R0,#0
	LDMFD		R13!,{PC}*/
GBC4:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetC4(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL
	BL		S9xGetC4
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles		
	LDMFD		R13!,{PC} //Return
GBDEBUG:	
GBBWRAM:
	MOV		R0,R0,LSL #17  
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSR #17	//Address&0x7FFF			
	LDR		R1,[regCPUvar,#BWRAM]	
	SUB		R0,R0,#0x6000   //((Address & 0x7fff) - 0x6000)	
	LDRB		R0,[R0,R1]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)	
	LDMFD		R13!,{PC}


//uint16 aaS9xGetWord(uint32 address);
asmS9xGetWord:
	// in : R0  = 0x00hhmmll
	// out : R0 = 0x000000ll
	// DESTROYED : R1,R2,R3
	// UPDATE : regCycles
	
	
	MOV		R1,R0,LSL #19	
	ADDS		R1,R1,#0x80000
	//if = 0x1FFF => 0
	BNE		GW_NotBoundary
	
	STMFD		R13!,{R0}
		STMFD		R13!,{PC}
	B		asmS9xGetByte
		MOV		R0,R0
	LDMFD		R13!,{R1}
	STMFD		R13!,{R0}
	ADD		R0,R1,#1
		STMFD		R13!,{PC}
	B		asmS9xGetByte
		MOV		R0,R0
	LDMFD		R13!,{R1}
	ORR		R0,R1,R0,LSL #8
	LDMFD		R13!,{PC}
	
GW_NotBoundary:	
	
	//R1 <= block	
	MOV		R1,R0,LSR #MEMMAP_SHIFT
	//MEMMAP_SHIFT is 12, Address is 0xFFFFFFFF at max, so
	//R1 is maxed by 0x000FFFFF, MEMMAP_MASK is 0x1000-1=0xFFF
	//so AND MEMMAP_MASK is BIC 0xFF000
	BIC		R1,R1,#0xFF000
	//R2 <= Map[block] (GetAddress)
	LDR		R2,[regCPUvar,#Map_ofs]
	LDR		R2,[R2,R1,LSL #2]
	CMP		R2,#MAP_LAST
	BLO		GWSpecial  //special
	// Direct ROM/RAM acess
	
	TST		R0,#1	
	BNE		GW_Not_Aligned1
	//R2 <= GetAddress + Address & 0xFFFF	
	//R3 <= MemorySpeed[block]			
	LDR		R3,[regCPUvar,#MemorySpeed_ofs]
	MOV		R0,R0,LSL #16
	LDRB		R3,[R3,R1]	
	MOV		R0,R0,LSR #16
	//Update CPU.Cycles
	ADD		regCycles,regCycles,R3, LSL #1
	//R3 = BlockIsRAM[block]
	LDR		R3,[regCPUvar,#BlockIsRAM_ofs]
	//Get value to return
	LDRH		R0,[R2,R0]
	LDRB		R3,[R3,R1]
	MOVS		R3,R3
	// if BlockIsRAM => update for CPUShutdown
	LDRNE		R1,[regCPUvar,#PCAtOpcodeStart_ofs]
	STRNE		R1,[regCPUvar,#WaitAddress_ofs]
	
	LDMFD		R13!,{PC} //Return
GW_Not_Aligned1:			

	MOV		R0,R0,LSL #16		
	ADD		R3,R0,#0x10000
	LDRB		R3,[R2,R3,LSR #16]	//GetAddress+ (Address+1)&0xFFFF
	LDRB		R0,[R2,R0,LSR #16]	//GetAddress+ Address&0xFFFF	
	ORR		R0,R0,R3,LSL #8	

	// if BlockIsRAM => update for CPUShutdown
	LDR		R3,[regCPUvar,#BlockIsRAM_ofs]	
	LDR		R2,[regCPUvar,#MemorySpeed_ofs]
	LDRB		R3,[R3,R1]   //R3 = BlockIsRAM[block]
	LDRB		R2,[R2,R1]   //R2 <= MemorySpeed[block]
	MOVS		R3,R3 	    //IsRAM ? CPUShutdown stuff
	LDRNE		R1,[regCPUvar,#PCAtOpcodeStart_ofs]	
	STRNE		R1,[regCPUvar,#WaitAddress_ofs]			
	ADD		regCycles,regCycles,R2, LSL #1 //Update CPU.Cycles				
	LDMFD		R13!,{PC}  //Return
GWSpecial:
#ifdef __PALMOS__	
	LDR		R3,[regCPUvar,#PALMOS_R10_ofs]
	LDR		R2,[PC,R2,LSL #2]
	ADD		PC,R2,R3	
#else	
	LDR		PC,[PC,R2,LSL #2]
	MOV		R0,R0 		//nop, for align
#endif	
	.long GWPPU
	.long GWCPU
	.long GWDSP
	.long GWLSRAM
	.long GWHSRAM
	.long GWNONE
	.long GWDEBUG
	.long GWC4
	.long GWBWRAM
	.long GWNONE
	.long GWNONE
	.long GWNONE
	/*.long GW7ROM
	.long GW7RAM
	.long GW7SRM*/
/*	MAP_PPU, MAP_CPU, MAP_DSP, MAP_LOROM_SRAM, MAP_HIROM_SRAM,
	MAP_NONE, MAP_DEBUG, MAP_C4, MAP_BWRAM, MAP_BWRAM_BITMAP,
	MAP_BWRAM_BITMAP2, MAP_SA1RAM, MAP_LAST*/
	
GWPPU:
	//InDMA ?
	LDRB		R1,[regCPUvar,#InDMA_ofs]
	MOVS		R1,R1	
	ADDEQ		regCycles,regCycles,#(ONE_CYCLE*2)		//No -> update Cycles
	MOV		R0,R0,LSL #16	//S9xGetPPU(Address&0xFFFF);
	STR		regCycles,[regCPUvar,#Cycles_ofs]	//Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL_R0
	BL		S9xGetPPU
	LDMFD		R13!,{R1}
	STMFD		R13!,{R0}
	ADD		R0,R1,#1
	//BIC		R0,R0,#0x10000
	BL		S9xGetPPU
		RESTORE_C_CALL_R1
	ORR		R0,R1,R0,LSL #8
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GWCPU:	
	ADD		regCycles,regCycles,#(ONE_CYCLE*2)	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetCPU(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL_R0
	BL		S9xGetCPU
	LDMFD		R13!,{R1}
	STMFD		R13!,{R0}
	ADD		R0,R1,#1
	//BIC		R0,R0,#0x10000
	BL		S9xGetCPU			
		RESTORE_C_CALL_R1
	ORR		R0,R1,R0,LSL #8
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GWDSP:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetCPU(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL_R0
	BL		S9xGetDSP
	LDMFD		R13!,{R1}
	STMFD		R13!,{R0}
	ADD		R0,R1,#1
	//BIC		R0,R0,#0x10000
	BL		S9xGetDSP	
		RESTORE_C_CALL_R1
	ORR		R0,R1,R0,LSL #8
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GWLSRAM:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles		
	
	TST		R0,#1
	BNE		GW_Not_Aligned2
	LDRH		R2,[regCPUvar,#SRAMMask]
	LDR		R1,[regCPUvar,#SRAM]
	AND		R3,R2,R0		//Address&SRAMMask
	LDRH		R0,[R3,R1]		//*Memory.SRAM + Address&SRAMMask		
	LDMFD		R13!,{PC}	//return
GW_Not_Aligned2:	
	LDRH		R2,[regCPUvar,#SRAMMask]
	LDR		R1,[regCPUvar,#SRAM]	
	AND		R3,R2,R0		//Address&SRAMMask
	ADD		R0,R0,#1
	AND		R2,R0,R2		//Address&SRAMMask
	LDRB		R3,[R1,R3]		//*Memory.SRAM + Address&SRAMMask
	LDRB		R2,[R1,R2]		//*Memory.SRAM + Address&SRAMMask
	ORR		R0,R3,R2,LSL #8
	LDMFD		R13!,{PC}	//return
GW7SRM:	
GWHSRAM:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles		
	
	TST		R0,#1
	BNE		GW_Not_Aligned3
	
	MOV		R1,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R1,R1,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)
	ADD		R0,R2,R1
	LDRH		R2,[regCPUvar,#SRAMMask]
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	LDR		R1,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask	
	LDRH		R0,[R1,R0]		//*Memory.SRAM + Address&SRAMMask
	LDMFD		R13!,{PC}		//return
	
GW_Not_Aligned3:	
	MOV		R3,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R3,R3,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)	
	ADD		R2,R2,R3						
	ADD		R0,R0,#1	
	SUB		R2,R2,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	MOV		R3,R0,LSL #17  
	AND		R0,R0,#0xF0000
	MOV		R3,R3,LSR #17	//(Address+1)&0x7FFF	
	MOV		R0,R0,LSR #3 //((Address+1)&0xF0000 >> 3)	
	ADD		R0,R0,R3	
	LDRH		R3,[regCPUvar,#SRAMMask]	//reload mask	
	SUB		R0,R0,#0x6000 //(((Address+1) & 0x7fff) - 0x6000 + (((Address+1) & 0xf0000) >> 3))		
	AND		R2,R3,R2		//Address...&SRAMMask	
	AND		R0,R3,R0		//(Address+1...)&SRAMMask	

	LDR		R3,[regCPUvar,#SRAM]
	LDRB		R0,[R0,R3]		//*Memory.SRAM + (Address...)&SRAMMask	
	LDRB		R2,[R2,R3]		//*Memory.SRAM + (Address+1...)&SRAMMask
	ORR		R0,R2,R0,LSL #8
			
	LDMFD		R13!,{PC}		//return
GW7ROM:
GW7RAM:	
GWNONE:		
	MOV		R0,R0,LSL #16
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	MOV		R0,R0,LSR #24
	ORR		R0,R0,R0,LSL #8
	LDMFD		R13!,{PC}
GWDEBUG:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	MOV		R0,#0
	LDMFD		R13!,{PC}
GWC4:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles	
	MOV		R0,R0,LSL #16 //S9xGetC4(Address&0xFFFF);	
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL_R0
	BL		S9xGetC4
	LDMFD		R13!,{R1}
	STMFD		R13!,{R0}
	ADD		R0,R1,#1
	//BIC		R0,R0,#0x10000
	BL		S9xGetC4
		RESTORE_C_CALL_R1
	ORR		R0,R1,R0,LSL #8
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
GWBWRAM:
	TST		R0,#1
	BNE		GW_Not_Aligned4
	MOV		R0,R0,LSL #17  
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	MOV		R0,R0,LSR #17	//Address&0x7FFF
	LDR		R1,[regCPUvar,#BWRAM]		
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000)		
	LDRH		R0,[R1,R0]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)	
	LDMFD		R13!,{PC}		//return
GW_Not_Aligned4:
	MOV		R0,R0,LSL #17  	
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	ADD		R3,R0,#0x20000
	MOV		R0,R0,LSR #17	//Address&0x7FFF
	MOV		R3,R3,LSR #17	//(Address+1)&0x7FFF
	LDR		R1,[regCPUvar,#BWRAM]		
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000)	
	SUB		R3,R3,#0x6000 //(((Address+1) & 0x7fff) - 0x6000)	
	LDRB		R0,[R1,R0]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)		
	LDRB		R3,[R1,R3]		//*Memory.BWRAM + (((Address+1) & 0x7fff) - 0x6000)	
	ORR		R0,R0,R3,LSL #8
	LDMFD		R13!,{PC}		//return

.pool


//void aaS9xSetByte(uint32 address,uint8 val);
asmS9xSetByte:
	// in : R0=0x00hhmmll  R1=0x000000ll	
	// DESTROYED : R0,R1,R2,R3
	// UPDATE : regCycles	
	//cpu shutdown
	MOV		R2,#0
	STR		R2,[regCPUvar,#WaitAddress_ofs]
	//
	
	//R3 <= block				
	MOV		R3,R0,LSR #MEMMAP_SHIFT
	//MEMMAP_SHIFT is 12, Address is 0xFFFFFFFF at max, so
	//R0 is maxed by 0x000FFFFF, MEMMAP_MASK is 0x1000-1=0xFFF
	//so AND MEMMAP_MASK is BIC 0xFF000
	BIC		R3,R3,#0xFF000
	//R2 <= Map[block] (SetAddress)
	LDR		R2,[regCPUvar,#WriteMap_ofs]
	LDR		R2,[R2,R3,LSL #2]
	CMP		R2,#MAP_LAST
	BLO		SBSpecial  //special
	// Direct ROM/RAM acess
	
	//R2 <= SetAddress + Address & 0xFFFF	
	MOV		R0,R0,LSL #16	
	ADD		R2,R2,R0,LSR #16	
	LDR		R0,[regCPUvar,#MemorySpeed_ofs]
	//Set byte
	STRB		R1,[R2]		
	//R0 <= MemorySpeed[block]
	LDRB		R0,[R0,R3]	
	//Update CPU.Cycles
	ADD		regCycles,regCycles,R0
	//CPUShutdown
	//only SA1 here : TODO	
	//Return
	LDMFD		R13!,{PC}
SBSpecial:
#ifdef __PALMOS__	
	LDR		R3,[regCPUvar,#PALMOS_R10_ofs]
	LDR		R2,[PC,R2,LSL #2]
	ADD		PC,R2,R3	
#else	
	LDR		PC,[PC,R2,LSL #2]
	MOV		R0,R0 		//nop, for align
#endif	
	.long SBPPU
	.long SBCPU
	.long SBDSP
	.long SBLSRAM
	.long SBHSRAM
	.long SBNONE
	.long SBDEBUG
	.long SBC4
	.long SBBWRAM
	.long SBNONE
	.long SBNONE
	.long SBNONE
	/*.long SB7ROM
	.long SB7RAM
	.long SB7SRM*/
SBPPU:
	//InDMA ?
	LDRB		R2,[regCPUvar,#InDMA_ofs]
	MOVS		R2,R2	
	ADDEQ		regCycles,regCycles,#ONE_CYCLE		//No -> update Cycles
	MOV		R0,R0,LSL #16	
	STR		regCycles,[regCPUvar,#Cycles_ofs]	//Save Cycles
	MOV		R0,R0,LSR #16
		PREPARE_C_CALL
	MOV		R12,R0
	MOV		R0,R1
	MOV		R1,R12		
	BL		S9xSetPPU		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SBCPU:	
	ADD		regCycles,regCycles,#ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF
		PREPARE_C_CALL
	MOV		R12,R0
	MOV		R0,R1
	MOV		R1,R12		
	BL		S9xSetCPU		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SBDSP:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF
		PREPARE_C_CALL
	MOV		R12,R0
	MOV		R0,R1
	MOV		R1,R12		
	BL		S9xSetDSP		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SBLSRAM:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles		
	LDRH		R2,[regCPUvar,#SRAMMask]
	MOVS		R2,R2
	LDMEQFD		R13!,{PC} //return if SRAMMask=0
	LDR		R3,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask	
	STRB		R1,[R0,R3]		//*Memory.SRAM + Address&SRAMMask	
	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}  //return
SB7SRM:	
SBHSRAM:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles		
	
	MOV		R3,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R3,R3,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)	
	ADD		R0,R2,R3	
	
	LDRH		R2,[regCPUvar,#SRAMMask]
	MOVS		R2,R2
	LDMEQFD		R13!,{PC} //return if SRAMMask=0
	
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	LDR		R3,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask	
	STRB		R1,[R0,R3]		//*Memory.SRAM + Address&SRAMMask
	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}	//return
SB7ROM:
SB7RAM:	
SBNONE:	
SBDEBUG:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles
	LDMFD		R13!,{PC}
SBC4:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF	
		PREPARE_C_CALL
	MOV		R12,R0
	MOV		R0,R1
	MOV		R1,R12		
	BL		S9xSetC4		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SBBWRAM:
	MOV		R0,R0,LSL #17  
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles
	MOV		R0,R0,LSR #17	//Address&0x7FFF			
	LDR		R2,[regCPUvar,#BWRAM]	
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000)	
	STRB		R1,[R0,R2]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)
	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	
	LDMFD		R13!,{PC}



//void aaS9xSetWord(uint32 address,uint16 val);
asmS9xSetWord:
	// in : R0  = 0x00hhmmll R1=0x0000hhll
	// DESTROYED : R0,R1,R2,R3
	// UPDATE : regCycles
	//R1 <= block	
	
	MOV		R2,R0,LSL #19	
	ADDS		R2,R2,#0x80000
	//if = 0x1FFF => 0
	BNE		SW_NotBoundary
	
	STMFD		R13!,{R0,R1}
		STMFD		R13!,{PC}
	B		asmS9xSetByte
		MOV		R0,R0
	LDMFD		R13!,{R0,R1}	
	ADD		R0,R0,#1
	MOV		R1,R1,LSR #8
		STMFD		R13!,{PC}
	B		asmS9xSetByte
		MOV		R0,R0
	
	LDMFD		R13!,{PC}
	
SW_NotBoundary:	
	
	MOV		R2,#0
	STR		R2,[regCPUvar,#WaitAddress_ofs]
	//	
	//R3 <= block				
	MOV		R3,R0,LSR #MEMMAP_SHIFT
	//MEMMAP_SHIFT is 12, Address is 0xFFFFFFFF at max, so
	//R1 is maxed by 0x000FFFFF, MEMMAP_MASK is 0x1000-1=0xFFF
	//so AND MEMMAP_MASK is BIC 0xFF000
	BIC		R3,R3,#0xFF000
	//R2 <= Map[block] (SetAddress)
	LDR		R2,[regCPUvar,#WriteMap_ofs]
	LDR		R2,[R2,R3,LSL #2]
	CMP		R2,#MAP_LAST
	BLO		SWSpecial  //special
	// Direct ROM/RAM acess		
	
	
	//check if address is 16bits aligned or not
	TST		R0,#1
	BNE		SW_not_aligned1
	//aligned
	MOV		R0,R0,LSL #16
	ADD		R2,R2,R0,LSR #16	//address & 0xFFFF + SetAddress
	LDR		R0,[regCPUvar,#MemorySpeed_ofs]
	//Set word
	STRH		R1,[R2]		
	//R1 <= MemorySpeed[block]
	LDRB		R0,[R0,R3]
	//Update CPU.Cycles
	ADD		regCycles,regCycles,R0, LSL #1
	//CPUShutdown
	//only SA1 here : TODO	
	//Return
	LDMFD		R13!,{PC}
	
SW_not_aligned1:	
	//R1 = (Address&0xFFFF)<<16
	MOV		R0,R0,LSL #16		
	//First write @address
	STRB		R1,[R2,R0,LSR #16]
	ADD		R0,R0,#0x10000
	MOV		R1,R1,LSR #8
	//Second write @address+1
	STRB		R1,[R2,R0,LSR #16]	
	//R1 <= MemorySpeed[block]
	LDR		R0,[regCPUvar,#MemorySpeed_ofs]
	LDRB		R0,[R0,R3]	
	//Update CPU.Cycles
	ADD		regCycles,regCycles,R0,LSL #1
	//CPUShutdown
	//only SA1 here : TODO	
	//Return
	LDMFD		R13!,{PC}
SWSpecial:
#ifdef __PALMOS__	
	LDR		R3,[regCPUvar,#PALMOS_R10_ofs]
	LDR		R2,[PC,R2,LSL #2]
	ADD		PC,R2,R3	
#else	
	LDR		PC,[PC,R2,LSL #2]
	MOV		R0,R0 		//nop, for align
#endif	
	.long SWPPU
	.long SWCPU
	.long SWDSP
	.long SWLSRAM
	.long SWHSRAM
	.long SWNONE
	.long SWDEBUG
	.long SWC4
	.long SWBWRAM
	.long SWNONE
	.long SWNONE
	.long SWNONE
	/*.long SW7ROM
	.long SW7RAM
	.long SW7SRM*/
SWPPU:
	//InDMA ?
	LDRB		R2,[regCPUvar,#InDMA_ofs]
	MOVS		R2,R2	
	ADDEQ		regCycles,regCycles,#(ONE_CYCLE*2)		//No -> update Cycles
	MOV		R0,R0,LSL #16	
	STR		regCycles,[regCPUvar,#Cycles_ofs]	//Save Cycles
	MOV		R0,R0,LSR #16
	MOV		R2,R1
	MOV		R1,R0
	MOV		R0,R2
		PREPARE_C_CALL_R0R1
	BL		S9xSetPPU		
	LDMFD		R13!,{R0,R1}
	ADD		R1,R1,#1
	MOV		R0,R0,LSR #8	
	BIC		R1,R1,#0x10000		
	BL		S9xSetPPU		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SWCPU:	
	ADD		regCycles,regCycles,#(ONE_CYCLE*2)	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF
	MOV		R2,R1
	MOV		R1,R0
	MOV		R0,R2	
		PREPARE_C_CALL_R0R1
	BL		S9xSetCPU		
	LDMFD		R13!,{R0,R1}
	ADD		R1,R1,#1
	MOV		R0,R0,LSR #8	
	BIC		R1,R1,#0x10000		
	BL		S9xSetCPU		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SWDSP:
	ADD		regCycles,regCycles,#SLOW_ONE_CYCLE	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF
	MOV		R2,R1
	MOV		R1,R0
	MOV		R0,R2
		PREPARE_C_CALL_R0R1
	BL		S9xSetDSP	
	LDMFD		R13!,{R0,R1}
	ADD		R1,R1,#1
	MOV		R0,R0,LSR #8	
	BIC		R1,R1,#0x10000	
	BL		S9xSetDSP		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SWLSRAM:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles		
	LDRH		R2,[regCPUvar,#SRAMMask]
	MOVS		R2,R2
	LDMEQFD		R13!,{PC} //return if SRAMMask=0
			
	AND		R3,R2,R0		//Address&SRAMMask
	TST		R0,#1
	BNE		SW_not_aligned2
	//aligned	
	LDR		R0,[regCPUvar,#SRAM]	
	STRH		R1,[R0,R3]		//*Memory.SRAM + Address&SRAMMask		
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}  //return	
SW_not_aligned2:	

	ADD		R0,R0,#1
	AND		R2,R2,R0		//(Address+1)&SRAMMask		
	LDR		R0,[regCPUvar,#SRAM]	
	STRB		R1,[R0,R3]		//*Memory.SRAM + Address&SRAMMask
	MOV		R1,R1,LSR #8
	STRB		R1,[R0,R2]		//*Memory.SRAM + (Address+1)&SRAMMask	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}  //return
SW7SRM:	
SWHSRAM:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles		
	
	LDRH		R2,[regCPUvar,#SRAMMask]
	MOVS		R2,R2
	LDMEQFD		R13!,{PC} //return if SRAMMask=0
	
	TST		R0,#1
	BNE		SW_not_aligned3	
	//aligned
	MOV		R3,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R3,R3,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)	
	ADD		R0,R2,R3				
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	LDRH		R2,[regCPUvar,#SRAMMask]
	LDR		R3,[regCPUvar,#SRAM]	
	AND		R0,R2,R0		//Address&SRAMMask	
	STRH		R1,[R0,R3]		//*Memory.SRAM + Address&SRAMMask	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}	//return		
SW_not_aligned3:	
	MOV		R3,R0,LSL #17  
	AND		R2,R0,#0xF0000
	MOV		R3,R3,LSR #17	//Address&0x7FFF	
	MOV		R2,R2,LSR #3 //(Address&0xF0000 >> 3)	
	ADD		R2,R2,R3				
	SUB		R2,R2,#0x6000 //((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3))
	
	ADD		R0,R0,#1	
	MOV		R3,R0,LSL #17  
	AND		R0,R0,#0xF0000
	MOV		R3,R3,LSR #17	//(Address+1)&0x7FFF	
	MOV		R0,R0,LSR #3 //((Address+1)&0xF0000 >> 3)	
	ADD		R0,R0,R3	
	LDRH		R3,[regCPUvar,#SRAMMask]	//reload mask	
	SUB		R0,R0,#0x6000 //(((Address+1) & 0x7fff) - 0x6000 + (((Address+1) & 0xf0000) >> 3))		
	AND		R2,R3,R2		//Address...&SRAMMask	
	AND		R0,R3,R0		//(Address+1...)&SRAMMask	
	
	LDR		R3,[regCPUvar,#SRAM]
	STRB		R1,[R2,R3]		//*Memory.SRAM + (Address...)&SRAMMask
	MOV		R1,R1,LSR #8
	STRB		R1,[R0,R3]		//*Memory.SRAM + (Address+1...)&SRAMMask
	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]		
	LDMFD		R13!,{PC}	//return	
SW7ROM:
SW7RAM:	
SWNONE:	
SWDEBUG:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	LDMFD		R13!,{PC}	//return
SWC4:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles	
	MOV		R0,R0,LSL #16 
	STR		regCycles,[regCPUvar,#Cycles_ofs] //Save Cycles
	MOV		R0,R0,LSR #16	//Address&0xFFFF	
	MOV		R2,R1
	MOV		R1,R0
	MOV		R0,R2
		PREPARE_C_CALL_R0R1
	BL		S9xSetC4		
	LDMFD		R13!,{R0,R1}	
	ADD		R1,R1,#1
	MOV		R0,R0,LSR #8	
	BIC		R1,R1,#0x10000		
	BL		S9xSetC4		
		RESTORE_C_CALL
	LDR		regCycles,[regCPUvar,#Cycles_ofs] //Load Cycles	
	LDMFD		R13!,{PC} //Return
SWBWRAM:
	ADD		regCycles,regCycles,#(SLOW_ONE_CYCLE*2)	//update Cycles
	TST		R0,#1
	BNE		SW_not_aligned4
	//aligned
	MOV		R0,R0,LSL #17		
	LDR		R2,[regCPUvar,#BWRAM]
	MOV		R0,R0,LSR #17	//Address&0x7FFF			
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000)	
	MOV		R3,#1
	STRH		R1,[R0,R2]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)			
	STRB		R3,[regCPUvar,#SRAMModified_ofs]			
	LDMFD		R13!,{PC}	//return
SW_not_aligned4:
	MOV		R0,R0,LSL #17	
	ADD		R3,R0,#0x20000
	MOV		R0,R0,LSR #17	//Address&0x7FFF
	MOV		R3,R3,LSR #17	//(Address+1)&0x7FFF
	LDR		R2,[regCPUvar,#BWRAM]	
	SUB		R0,R0,#0x6000 //((Address & 0x7fff) - 0x6000)
	SUB		R3,R3,#0x6000 //(((Address+1) & 0x7fff) - 0x6000)
	STRB		R1,[R2,R0]		//*Memory.BWRAM + ((Address & 0x7fff) - 0x6000)
	MOV		R1,R1,LSR #8
	STRB		R1,[R2,R3]		//*Memory.BWRAM + (((Address+1) & 0x7fff) - 0x6000)	
	MOV		R0,#1
	STRB		R0,[regCPUvar,#SRAMModified_ofs]			
	LDMFD		R13!,{PC}		//return
	

.pool
