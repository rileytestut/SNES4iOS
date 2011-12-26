
/*****************************************************************/
/*     Offset in SCPUState structure				 */
/*****************************************************************/
#define Flags_ofs		0    
#define BranchSkip_ofs		4
#define NMIActive_ofs		5
#define IRQActive_ofs		6
#define WaitingForInterrupt_ofs	7
#define InDMA_ofs		8
#define WhichEvent		9
#define SRAMModified_ofs	10
#define BRKTriggered_ofs	11
#define PC_ofs			12
#define PCBase_ofs		16

#define PCAtOpcodeStart_ofs	20

#define WaitAddress_ofs		24

#define WaitCounter_ofs		28
#define Cycles_ofs		32
#define NextEvent_ofs		36
#define V_Counter_ofs		40
#define MemSpeed_ofs		44
#define MemSpeedx2_ofs		48
#define FastROMSpeed_ofs	52
#define AutoSaveTimer_ofs	56
#define NMITriggerPoint_ofs	60
#define NMICycleCount_ofs	64
#define IRQCycleCount_ofs	68

#define	RPB_ofs		72
#define	RDB_ofs		73
#define	RP_ofs		74
#define	RA_ofs		76
#define RAH_ofs		RA_ofs+1
#define	RD_ofs		78
#define	RS_ofs		80
#define	RX_ofs		82
#define	RY_ofs		84
#define	RPC_ofs		86
   

#define	asm_OPTABLE_ofs		88
#define TriedInterleavedMode2_ofs	92



#define Map_ofs		96
#define WriteMap_ofs	100
#define MemorySpeed_ofs 104
#define BlockIsRAM_ofs  108
#define SRAM 		112
#define BWRAM 		116
#define SRAMMask 	120

#define	APUExecuting_ofs 122

#define	PALMOS_R9_ofs 	124
#define	PALMOS_R10_ofs 	128
/*****************************************************************/


#ifdef __PALMOS__
/* prepare */
.macro		PREPARE_C_CALL
	STMFD	R13!,{R9,R10,R12,R14}
	LDR	R9,[regCPUvar,#PALMOS_R9_ofs]
	LDR	R10,[regCPUvar,#PALMOS_R10_ofs]
.endm
.macro		PREPARE_C_CALL_R0
	STMFD	R13!,{R0,R9,R10,R12,R14}
	LDR	R9,[regCPUvar,#PALMOS_R9_ofs]
	LDR	R10,[regCPUvar,#PALMOS_R10_ofs]
.endm
.macro		PREPARE_C_CALL_R0R1
	STMFD	R13!,{R0,R1,R9,R10,R12,R14}
	LDR	R9,[regCPUvar,#PALMOS_R9_ofs]
	LDR	R10,[regCPUvar,#PALMOS_R10_ofs]
.endm
.macro		PREPARE_C_CALL_LIGHT
	STMFD	R13!,{R14}
	LDR	R9,[regCPUvar,#PALMOS_R9_ofs]
	LDR	R10,[regCPUvar,#PALMOS_R10_ofs]
.endm
.macro		PREPARE_C_CALL_LIGHTR12
	STMFD	R13!,{R9,R10,R12,R14}
	LDR	R9,[regCPUvar,#PALMOS_R9_ofs]
	LDR	R10,[regCPUvar,#PALMOS_R10_ofs]
.endm
/* restore */
.macro		RESTORE_C_CALL
	LDMFD	R13!,{R9,R10,R12,R14}	

.endm
.macro		RESTORE_C_CALL_R0
	LDMFD	R13!,{R0,R9,R10,R12,R14}	
.endm
.macro		RESTORE_C_CALL_R1
	LDMFD	R13!,{R1,R9,R10,R12,R14}	
.endm
.macro		RESTORE_C_CALL_LIGHT
	LDMFD	R13!,{R14}
.endm
.macro		RESTORE_C_CALL_LIGHTR12
	LDMFD	R13!,{R9,R10,R12,R14}
.endm
#else
/* prepare */
.macro		PREPARE_C_CALL
	STMFD	R13!,{R12,R14}	
.endm
.macro		PREPARE_C_CALL_R0
	STMFD	R13!,{R0,R12,R14}	
.endm
.macro		PREPARE_C_CALL_R0R1
	STMFD	R13!,{R0,R1,R12,R14}		
.endm
.macro		PREPARE_C_CALL_LIGHT
	STMFD	R13!,{R14}
.endm
.macro		PREPARE_C_CALL_LIGHTR12
	STMFD	R13!,{R12,R14}
.endm
/* restore */
.macro		RESTORE_C_CALL
	LDMFD	R13!,{R12,R14}
.endm
.macro		RESTORE_C_CALL_R0
	LDMFD	R13!,{R0,R12,R14}
.endm
.macro		RESTORE_C_CALL_R1
	LDMFD	R13!,{R1,R12,R14}
.endm
.macro		RESTORE_C_CALL_LIGHT
	LDMFD	R13!,{R14}
.endm
.macro		RESTORE_C_CALL_LIGHTR12
	LDMFD	R13!,{R12,R14}
.endm
#endif

//--------------
.macro		LOAD_REGS	
	//regD & regPBank share the same register
	LDRB		regPBank,[regCPUvar,#RPB_ofs]
	LDRH		rscratch,[regCPUvar,#RD_ofs]
	ORR		regD,regD,rscratch, LSL #16	
	//rstatus & regDBank share the same register
	LDRB		regDBank,[regCPUvar,#RDB_ofs]
	LDRH		rscratch,[regCPUvar,#RP_ofs]	
	ORRS		rstatus, rstatus, rscratch,LSL #STATUS_SHIFTER  	
	//if Carry set, then EMULATION bit was set
	ORRCS		rstatus,rstatus,#MASK_EMUL	
	//
	LDRH		regA,[regCPUvar,#RA_ofs]		
	LDRH		regX,[regCPUvar,#RX_ofs]
	LDRH		regY,[regCPUvar,#RY_ofs]
	LDRH		regS,[regCPUvar,#RS_ofs]
	//Shift X,Y & A according to the current mode (INDEX, MEMORY bits)
	TST		rstatus,#MASK_INDEX
	MOVNE		regX,regX,LSL #24
	MOVNE		regY,regY,LSL #24
	MOVEQ		regX,regX,LSL #16
	MOVEQ		regY,regY,LSL #16
	TST		rstatus,#MASK_MEM
	MOVNE		regA,regA,LSL #24
	MOVEQ		regA,regA,LSL #16
	
	LDR		regpcbase,[regCPUvar,#PCBase_ofs]
	LDR		rpc,[regCPUvar,#PC_ofs]	
	LDR		regCycles,[regCPUvar,#Cycles_ofs]
.endm


.macro		SAVE_REGS
	//regD & regPBank is same register
	STRB		regPBank,[regCPUvar,#RPB_ofs]
	MOV		rscratch,regD, LSR #16
	STRH		rscratch,[regCPUvar,#RD_ofs]
	//rstatus & regDBank is same register
	STRB		regDBank,[regCPUvar,#RDB_ofs]
	MOVS		rscratch, rstatus, LSR #STATUS_SHIFTER  
	ORRCS		rscratch,rscratch,#0x100 //EMULATION bit
	STRH		rscratch,[regCPUvar,#RP_ofs]
	//
	//Shift X,Y & A according to the current mode (INDEX, MEMORY bits)
	TST		rstatus,#MASK_INDEX
	MOVNE		rscratch,regX,LSR #24
	MOVNE		rscratch2,regY,LSR #24
	MOVEQ		rscratch,regX,LSR #16
	MOVEQ		rscratch2,regY,LSR #16
	STRH		rscratch,[regCPUvar,#RX_ofs]
	STRH		rscratch2,[regCPUvar,#RY_ofs]
	TST		rstatus,#MASK_MEM
	LDRNEH		rscratch,[regCPUvar,#RA_ofs]
	BICNE		rscratch,rscratch,#0xFF
	ORRNE		rscratch,rscratch,regA,LSR #24	
	MOVEQ		rscratch,regA,LSR #16
	STRH		rscratch,[regCPUvar,#RA_ofs]
	
	STRH		regS,[regCPUvar,#RS_ofs]	
	STR		regpcbase,[regCPUvar,#PCBase_ofs]
	STR		rpc,[regCPUvar,#PC_ofs]
	
	STR		regCycles,[regCPUvar,#Cycles_ofs]
.endm

/*****************************************************************/
.macro		ADD1CYCLE		
		add	regCycles,regCycles, #ONE_CYCLE		
.endm
.macro		ADD1CYCLENE
		addne	regCycles,regCycles, #ONE_CYCLE		
.endm		
.macro		ADD1CYCLEEQ
		addeq	regCycles,regCycles, #ONE_CYCLE		
.endm		

.macro		ADD2CYCLE
		add	regCycles,regCycles, #(ONE_CYCLE*2)
.endm
.macro		ADD2CYCLENE
		addne	regCycles,regCycles, #(ONE_CYCLE*2)
.endm
.macro		ADD2CYCLE2MEM		
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]
		add	regCycles,regCycles, #(ONE_CYCLE*2)
		add	regCycles, regCycles, rscratch, LSL #1		
.endm
.macro		ADD2CYCLE1MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]
		add	regCycles,regCycles, #(ONE_CYCLE*2)
		add	regCycles, regCycles, rscratch
.endm

.macro		ADD3CYCLE
		add	regCycles,regCycles, #(ONE_CYCLE*3)
.endm

.macro		ADD1CYCLE1MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]
		add	regCycles,regCycles, #ONE_CYCLE
		add	regCycles, regCycles, rscratch
.endm

.macro		ADD1CYCLE2MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]
		add	regCycles,regCycles, #ONE_CYCLE
		add	regCycles, regCycles, rscratch, lsl #1
.endm

.macro		ADD1MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]		
		add	regCycles, regCycles, rscratch
.endm
			
.macro		ADD2MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]		
		add	regCycles, regCycles, rscratch, lsl #1
.endm
			
.macro		ADD3MEM
		ldr	rscratch,[regCPUvar,#MemSpeed_ofs]		
		add	regCycles, rscratch, regCycles
		add	regCycles, regCycles, rscratch, lsl #1
.endm

/**************/
.macro		ClearDecimal
		BIC	rstatus,rstatus,#MASK_DECIMAL	
.endm			
.macro		SetDecimal
		ORR	rstatus,rstatus,#MASK_DECIMAL	
.endm
.macro		SetIRQ
		ORR	rstatus,rstatus,#MASK_IRQ
.endm						
.macro		ClearIRQ
		BIC	rstatus,rstatus,#MASK_IRQ
.endm

.macro		CPUShutdown
//if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
		LDR		rscratch,[regCPUvar,#WaitAddress_ofs]
		CMP		rpc,rscratch
		BNE		5431f
//if (CPU.WaitCounter == 0 && !(CPU.Flags & (IRQ_PENDING_FLAG | NMI_FLAG)))		
		LDR		rscratch,[regCPUvar,#Flags_ofs]
		LDR		rscratch2,[regCPUvar,#WaitCounter_ofs]
		TST		rscratch,#(IRQ_PENDING_FLAG|NMI_FLAG)
		BNE		5432f		
		MOVS		rscratch2,rscratch2
		BNE		5432f
//CPU.WaitAddress = NULL;		
		MOV		rscratch,#0
		STR		rscratch,[regCPUvar,#WaitAddress_ofs]
//if (Settings.SA1)
//		S9xSA1ExecuteDuringSleep ();		: TODO
		
//	    CPU.Cycles = CPU.NextEvent;
		LDR		regCycles,[regCPUvar,#NextEvent_ofs]
		LDRB		r0,[regCPUvar,#APUExecuting_ofs]
		MOVS		r0,r0
		BEQ		5431f
//	    if (IAPU.APUExecuting)
/*	    {
		ICPU.CPUExecuting = FALSE;
		do
		{
		    APU_EXECUTE1();
		} while (APU.Cycles < CPU.NextEvent);
		ICPU.CPUExecuting = TRUE;
	    }
	*/					
		asmAPU_EXECUTE2
		B		5431f
.pool		
5432:
/*	else
	if (CPU.WaitCounter >= 2)
	    CPU.WaitCounter = 1;
	else
	    CPU.WaitCounter--;
*/
		CMP		rscratch2,#1
		MOVHI		rscratch2,#1
		//SUBLS		rscratch2,rscratch2,#1
		MOVLS		rscratch2,#0
		STR		rscratch2,[regCPUvar,#WaitCounter_ofs]
5431:		

.endm						
.macro		BranchCheck0	
		/*in rsctach : OpAddress
		/*destroy rscratch2*/
		LDRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		MOVS	rscratch2,rscratch2	
		BEQ	1110f
		MOV	rscratch2,#0		
		STRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		SUB	rscratch2,rpc,regpcbase
		//if( CPU.PC - CPU.PCBase > OpAddress) return;
		CMP	rscratch2,rscratch
		BHI	1111f
1110:		
.endm									
.macro		BranchCheck1		
		/*in rsctach : OpAddress
		/*destroy rscratch2*/
		LDRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		MOVS	rscratch2,rscratch2	
		BEQ	1110f
		MOV	rscratch2,#0		
		STRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		SUB	rscratch2,rpc,regpcbase
		//if( CPU.PC - CPU.PCBase > OpAddress) return;
		CMP	rscratch2,rscratch
		BHI	1111f
1110:
.endm												
.macro		BranchCheck2
		/*in rsctach : OpAddress
		/*destroy rscratch2*/
		LDRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		MOVS	rscratch2,rscratch2	
		BEQ	1110f
		MOV	rscratch2,#0		
		STRB	rscratch2,[regCPUvar,#BranchSkip_ofs]
		SUB	rscratch2,rpc,regpcbase
		//if( CPU.PC - CPU.PCBase > OpAddress) return;
		CMP	rscratch2,rscratch
		BHI	1111f
1110:		
.endm
			
.macro		S9xSetPCBase
		// in  : rscratch (0x00hhmmll)				
		PREPARE_C_CALL			
		BL	asm_S9xSetPCBase		
		RESTORE_C_CALL
		LDR	rpc,[regCPUvar,#PC_ofs]
		LDR	regpcbase,[regCPUvar,#PCBase_ofs]
.endm		

.macro		S9xFixCycles
#ifdef	__PALMOS__
		LDR		rscratch2,[regCPUvar,#PALMOS_R10_ofs]
#endif		
		TST		rstatus,#MASK_EMUL
		LDRNE		rscratch, = jumptable1	   //Mode 0 : M=1,X=1
		BNE		991111f
		//EMULATION=0
		TST		rstatus,#MASK_MEM
		BEQ		991112f
		//MEMORY=1
		TST		rstatus,#MASK_INDEX
		//INDEX=1  //Mode 0 : M=1,X=1
		LDRNE		rscratch, = jumptable1		
		//INDEX=0  //Mode 1 : M=1,X=0
		LDREQ		rscratch, = jumptable2
		B		991111f
991112:		//MEMORY=0		
		TST		rstatus,#MASK_INDEX
		//INDEX=1   //Mode 3 : M=0,X=1
		LDRNE		rscratch, = jumptable4
		//INDEX=0   //Mode 2 : M=0,X=0
		LDREQ		rscratch, = jumptable3		
991111:
#ifdef	__PALMOS__
		ADD		rscratch,rscratch,rscratch2
#endif		
		STR		rscratch,[regCPUvar,#asm_OPTABLE_ofs]
.endm		
.macro		S9xOpcode_NMI
		SAVE_REGS
		PREPARE_C_CALL_LIGHT
		BL	asm_S9xOpcode_NMI
		RESTORE_C_CALL_LIGHT
		LOAD_REGS		
.endm
.macro		S9xOpcode_IRQ
		SAVE_REGS
		PREPARE_C_CALL_LIGHT
		BL	asm_S9xOpcode_IRQ
		RESTORE_C_CALL_LIGHT
		LOAD_REGS		
.endm
.macro		S9xDoHBlankProcessing
		SAVE_REGS
		PREPARE_C_CALL_LIGHT
		BL	asm_S9xDoHBlankProcessing
		RESTORE_C_CALL_LIGHT
		LOAD_REGS		
.endm

/********************************/
.macro		EXEC_OP					
		LDR		R1,[regCPUvar,#asm_OPTABLE_ofs]
		STR		rpc,[regCPUvar,#PCAtOpcodeStart_ofs]
		ADD1MEM
		LDRB		R0, [rpc], #1		
		
#ifdef	__PALMOS__		
		LDR		R2,[regCPUvar,#PALMOS_R10_ofs]
		LDR		R3,[R1,R0,LSL #2]
		ADD		PC,R2,R3	
#else
		LDR		PC, [R1,R0, LSL #2]
#endif		
.endm
.macro		NEXTOPCODE
#ifdef	__TESTING__
		B	endmainLoop
#endif		
		LDR			rscratch,[regCPUvar,#NextEvent_ofs]
		CMP			regCycles,rscratch
		BLT			mainLoop
  		S9xDoHBlankProcessing
		B			mainLoop
.endm

.macro		asmAPU_EXECUTE
		LDRB		R0,[regCPUvar,#APUExecuting_ofs]
		MOVS		R0,R0
		BEQ		43210f
		//SAVE_REGS
		STR		regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL_LIGHTR12
		BL		asm_APU_EXECUTE
		RESTORE_C_CALL_LIGHTR12
		LDR		regCycles,[regCPUvar,#Cycles_ofs]
		//LOAD_REGS
		//S9xFixCycles
43210:
.endm

.macro		asmAPU_EXECUTE2
		//SAVE_REGS		
		STR		regCycles,[regCPUvar,#Cycles_ofs]
		PREPARE_C_CALL_LIGHTR12
		BL		asm_APU_EXECUTE2
		RESTORE_C_CALL_LIGHTR12
		LDR		regCycles,[regCPUvar,#Cycles_ofs]		
		//LOAD_REGS
.endm


//void asmMainLoop(asm_cpu_var_t *asmcpuPtr);
asmMainLoop:
	//save registers
	STMFD		R13!,{R4-R11,LR}
	//init pointer to CPUvar structure
	MOV		regCPUvar,R0
	//init registers
	LOAD_REGS
	//get cpu mode from flag and init jump table
	S9xFixCycles
mainLoop:
	//APU Execute
	asmAPU_EXECUTE
	
	//Test Flags
	LDR		rscratch,[regCPUvar,#Flags_ofs]
	MOVS		rscratch,rscratch
	BNE		CPUFlags_set	//If flags => check for irq/nmi/scan_keys...	
	
	EXEC_OP						//Execute next opcode
	
CPUFlags_set:	//Check flags (!=0)
		TST	rscratch,#NMI_FLAG		//Check NMI
		BEQ	CPUFlagsNMI_FLAG_cleared	
		LDR	rscratch2,[regCPUvar,#NMICycleCount_ofs]
		SUBS	rscratch2,rscratch2,#1
		STR	rscratch2,[regCPUvar,#NMICycleCount_ofs]		
		BNE	CPUFlagsNMI_FLAG_cleared	
		BIC	rscratch,rscratch,#NMI_FLAG
		STR	rscratch,[regCPUvar,#Flags_ofs]		
		LDRB	rscratch2,[regCPUvar,#WaitingForInterrupt_ofs]
		MOVS	rscratch2,rscratch2
		BEQ	NotCPUaitingForInterruptNMI
		MOV	rscratch2,#0
		ADD	rpc,rpc,#1
		STRB	rscratch2,[regCPUvar,#WaitingForInterrupt_ofs]		
NotCPUaitingForInterruptNMI:
		S9xOpcode_NMI
		LDR	rscratch,[regCPUvar,#Flags_ofs]	
CPUFlagsNMI_FLAG_cleared:
		TST	rscratch,#IRQ_PENDING_FLAG   //Check IRQ_PENDING_FLAG
		BEQ	CPUFlagsIRQ_PENDING_FLAG_cleared		
		LDR	rscratch2,[regCPUvar,#IRQCycleCount_ofs]
		MOVS	rscratch2,rscratch2
		BNE	CPUIRQCycleCount_NotZero		
	 	LDRB	rscratch2,[regCPUvar,#WaitingForInterrupt_ofs]
		MOVS	rscratch2,rscratch2
		BEQ	NotCPUaitingForInterruptIRQ
	        MOV	rscratch2,#0
		ADD	rpc,rpc,#1
		STRB	rscratch2,[regCPUvar,#WaitingForInterrupt_ofs]
NotCPUaitingForInterruptIRQ:
		LDRB	rscratch2,[regCPUvar,#IRQActive_ofs]
		MOVS	rscratch2,rscratch2
		BEQ	CPUIRQActive_cleared
		TST	rstatus,#MASK_IRQ
		BNE	CPUFlagsIRQ_PENDING_FLAG_cleared
		S9xOpcode_IRQ
		LDR	rscratch,[regCPUvar,#Flags_ofs]	
		B	CPUFlagsIRQ_PENDING_FLAG_cleared
CPUIRQActive_cleared:		
		BIC	rscratch,rscratch,#IRQ_PENDING_FLAG
		STR	rscratch,[regCPUvar,#Flags_ofs]	
		B	CPUFlagsIRQ_PENDING_FLAG_cleared
CPUIRQCycleCount_NotZero:
		SUB	rscratch2,rscratch2,#1
		STR	rscratch2,[regCPUvar,#IRQCycleCount_ofs]
CPUFlagsIRQ_PENDING_FLAG_cleared:

		TST	rscratch,#SCAN_KEYS_FLAG   //Check SCAN_KEYS_FLAG
		BNE	endmainLoop		

	EXEC_OP	//Execute next opcode

endmainLoop:
    /*Registers.PC = CPU.PC - CPU.PCBase;
    S9xPackStatus ();
    APURegisters.PC = IAPU.PC - IAPU.RAM;
    S9xAPUPackStatus ();
    
    if (CPU.Flags & SCAN_KEYS_FLAG)
    {
	    S9xSyncSpeed ();
	CPU.Flags &= ~SCAN_KEYS_FLAG;
    }	*/
/********end*/
	SAVE_REGS
	LDMFD		R13!,{R4-R11,LR}
	MOV		PC,LR


.pool

//void test_opcode(struct asm_cpu_var *asm_var);
test_opcode:
	//save registers
	STMFD		R13!,{R4-R11,LR}
	//init pointer to CPUvar structure
	MOV		regCPUvar,R0
	//init registers
	LOAD_REGS
	//get cpu mode from flag and init jump table
	S9xFixCycles
	
	EXEC_OP
.pool
