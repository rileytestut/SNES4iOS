
	.align 4
	.globl  DecodeBlockAsm
	.globl  DecodeBlockAsm2
	
/*
;Bit-Rate Expand Waveform
;
;Desc:
;   Decompresses a 9-byte bit-rate reduced block into 16 16-bit samples.
;   This procedure is designed to be recursively called to decompress a series of blocks.
;In:
;   R0=ESI-> Sample Block
;   R1=EDI   -> Output buffer
;   R2=EDX   =3D Last sample of previous block (32-bit)
;   R3=EBX   =3D Next to last sample (sign extended from 16-bits)
;Out:
;   R0=ESI -> Next Block
;   R1=EDI -> After last sample
;   R2=EDX =3D Last sample (32-bit)
;   R3=EBX =3D Next to last sample (16-bit)
;Destroys:
;   R4=EAX	
*/

.macro	SMP12CLIP16	
	CMP 	R4, R12
	BGT 	1111f	
	MOVNE 	R4, R12
	B 	1112f
1111:
	CMP 	R4, R11
	MOVGT 	R4, R11
1112:
	CMP 	R2, R12
	BGT 	1121f	
	MOVNE 	R2, R12
	B 	1122f
1121:
	CMP 	R2, R11
	MOVGT 	R2, R11
1122:
.endm

@ void DecodeBlockAsm2 (int8 *, int16 *, int32 *, int32 *);
@ DecodeBlockAsm2 (compressed, raw, &ch->previous [0], &ch->previous [1]);
	.align 4
DecodeBlockAsm:

	STMFD		SP!,{R4,LR}
			
	STMFD		SP!,{R2,R3}
	LDR		R2,[R2]
	LDR		R3,[R3]
	BL		BREWave
	LDMFD		SP!,{R0,R1}
	STR		R2,[R0]
	STR		R3,[R1]
		
	LDMFD		SP!,{R4,LR}
	MOV		PC,LR	

BREWave:	
	STMFD	SP!,{R5,R6,R7}
	@ Mov	AL,[ESI]                           ;Get header byte
	@ Inc	ESI
	
	LDRB	R4,[R0],#1
	@ Mov	CL,0CFh
	@ Sub	CL,AL	
	@ SetC	AH
	@ Dec	AH
	@ And	CL,AH
	@ ShR	CL,4                               ;Isolate range
	CMP	R4,#0xD0
	MOVHS	R5,#0
	MOVLO	R5,#0xCF
	SUBLO	R5,R5,R4
	MOVLO	R5,R5,LSR #4
						
	@ Mov	CH,8                               ;Decompress 8 bytes (16 nybbles)
	MOV	R6,#8	
	@ Test	AL,0Ch                             ;Does block use ADPCM compression?
	@ JZ 	@@Method0                    ;  No
	TST	R4,#0xC
	BEQ	Method0
	@ Test	AL,8                               ;Does block use method 1?
	@ JZ 	@@Method1                    ;  Yes
	TST	R4,#0x8
	BEQ	Method1
	@ Test	AL,4                               ;Does block use method 2?
	@ jnz 	@@Method3                          ;  Yes
	@ jmp	@@Method2
	TST	R4,#0x4
	BEQ	Method2
	B	Method3			@ ;Must use method 3
/*
ALIGN	16
	;[Smp] ----------------------------------*/
Method0:
	ADD	R5,R5,#16
Method0loop:
	@ XOr	EAX,EAX
	@ XOr	EDX,EDX
	@ Mov	AH,byte[ESI]                        ;Get byte
	@ Mov	DH,AH
	@ And	AH,0F0h                         ;AH = High nybble << 12
	@ ShL	DH,4                            ;DH = Low nybble << 12
	LDRB	R4,[R0],#1
	 @ R4 = 0x000000hl
	MOV	R2,R4,LSL #(28)
	 @ R2 = 0xl0000000
	MOV	R4,R4,LSL #(24)
	 @ R4 = 0xhl000000
	BIC	R4,R4,#0x0F000000
	 @ R4 = 0xh0000000
	 	
	@ SAR	AX,CL                           ;Reduce samples according to range
	@ SAR	DX,CL
	MOV	R4,R4,ASR R5	
	MOV	R2,R2,ASR R5
		
	@ Mov	word[EDI],AX
	@ Mov	word[2+EDI],DX
	STRH	R4,[R1],#2
	STRH	R2,[R1],#2
	
	@ Add	EDI,4
	@ Inc	ESI
	@ Dec	CH
	@ JNZ	Short @@Method0
	@ MovSX	EDX,DX
	@ MovSX	EBX,AX
	@ Ret	
	SUBS	R6,R6,#1
	BNE	Method0loop	
	MOV	R2,R2,LSL #16
	MOV	R2,R2,ASR #16
	MOV	R3,R4,LSL #16
	MOV	R3,R3,ASR #16		
	LDMFD	SP!,{R5,R6,R7}
	MOV	PC,LR

@ ALIGN	16
@ 	;[Delta]+[Smp-1](15/16) -----------------
Method1:
	ADD	R7,R5,#16
Method1loop:	
		@ MovSX	EBX,byte[ESI]              ;Sign extend upper nybble into EBX
		@ And	BL,0F0h
		@ ShL	EBX,8
		@ SAR	EBX,CL
		LDRSB   R3,[R0]
		BIC	R3,R3,#0xF
		MOV	R3,R3,LSL #8
		MOV	R3,R3,ASR R5

		@ MovSX	EAX,DX
		@ Add	EBX,EAX
		@ SAR	EAX,4
		@ Sub	EBX,EAX
		MOV	R4,R2,LSL #16		
		ADD	R3,R3,R4,ASR #16
		SUB	R3,R3,R4,ASR #20

		@ Mov	word[EDI],BX
		STRH	R3,[R1],#2

		@ Mov	DL,byte[ESI]
		@ ShL	EDX,12
		@ MovSX	EDX,DX
		@ SAR	EDX,CL
		LDRSB   R2,[R0],#1		
		MOV	R2,R2,LSL #(12+16)		
		MOV	R2,R2,ASR R7

		@ MovSX	EAX,BX
		@ Add	EDX,EAX
		@ SAR	EAX,4
		@ Sub	EDX,EAX
		MOV	R4,R3,LSL #16		
		ADD	R2,R2,R4,ASR #16
		SUB	R2,R2,R4,ASR #20

		@ Mov	word[2+EDI],DX
		STRH	R2,[R1],#2
		@ Add	EDI,4

		@ Inc	ESI

	@ Dec	CH
	@ JNZ	Short @@Method1
	@ MovSX	EBX,BX
	@ Ret
	SUBS	R6,R6,#1
	BNE	Method1loop
	
	MOV	R3,R3,LSL #16
	MOV	R3,R3,ASR #16
	
	LDMFD	SP!,{R5,R6,R7}
	MOV	PC,LR

@ ALIGN	16
	@ ;[Delta]+[Smp-1](61/32)-[Smp-2](30/32) --
Method2:
	ADD	R7,R5,#16
Method2loop:
		@ MovSX	EAX,Byte[ESI]              ;EAX = Delta
		LDRSB	R4,[R0]
		@ And	AL,0F0h
		@ ShL	EAX,8
		@ SAR	EAX,CL
		BIC	R4,R4,#0xF
		MOV	R4,R4,LSL #8
		MOV	R4,R4,ASR R5

		@ ;Subtract 15/16 of second sample -----
		@ Sub	EAX,EBX
		@ SAR	EBX,4
		@ Add	EAX,EBX
		@ MovSX	EBX,DX
		SUB	R4,R4,R3
		ADD	R4,R4,R3,ASR #4
		MOV	R3,R2
		
		@ ;Add 61/32 of last sample ------------
		@ And	DL,~3
		@ Add	EAX,EDX
		@ Add	EAX,EDX
		@ SAR	EDX,4
		@ Sub	EAX,EDX
		@ SAR	EDX,1		
		BIC	R2,R2,#3
		ADD	R4,R4,R2,LSL #1		
		SUB	R4,R4,R2,ASR #4						
		@ MovSX	EDX,DX
		@ Sub	EAX,EDX		
		SUB	R4,R4,R2,ASR #5
		@ Mov	word[EDI],AX
		STRH	R4,[R1],#2

		@ Mov	DL,byte[ESI]
		LDRB	R2,[R0],#1
		@ ShL	EDX,12
		@ MovSX	EDX,DX
		@ SAR	EDX,CL
		MOV	R2,R2,LSL #(12+16)		
		MOV	R2,R2,ASR R7

		@ Sub	EDX,EBX
		@ SAR	EBX,4
		@ Add	EDX,EBX
		@ MovSX	EBX,AX
		SUB	R2,R2,R3
		ADD	R2,R2,R3,ASR #4
		MOV	R3,R4
		
		@ And	AL,~3
		@ Add	EDX,EAX
		@ Add	EDX,EAX
		@ SAR	EAX,4
		@ Sub	EDX,EAX
		@ SAR	EAX,1
		BIC	R4,R4,#3
		ADD	R2,R2,R4,LSL #1		
		SUB	R2,R2,R4,ASR #4						
		@ MovSX	EAX,AX
		@ Sub	EDX,EAX		
		SUB	R2,R2,R4,ASR #5		

		@ Mov	word[2+EDI],DX
		STRH	R2,[R1],#2
		@ Add	EDI,4

		@ Inc	ESI

	@ Dec	CH
	@ JNZ	@@Method2
	@ Ret
	SUBS	R6,R6,#1
	BNE	Method2loop
	LDMFD	SP!,{R5,R6,R7}
	MOV	PC,LR

@ ALIGN	16
	@ ;[Delta]+[Smp-1](115/64)-[Smp-2](52/64) -
Method3:
	ADD	R7,R5,#16
Method3loop:
	@ MovSX	EAX,Byte[ESI]
	LDRSB	R4,[R0]
		@ And	AL,0F0h
		@ ShL	EAX,8
		@ SAR	EAX,CL
	BIC	R4,R4,#0xF
	MOV	R4,R4,LSL #8
	MOV	R4,R4,ASR R5

		@ ;Subtract 13/16 of second sample -----
		@ Sub	EAX,EBX
		@ SAR	EBX,3
		@ Add	EAX,EBX
		@ SAR	EBX,1
		@ Add	EAX,EBX
		@ MovSX	EBX,DX
	SUB	R4,R4,R3
	ADD	R4,R4,R3,ASR #3
	ADD	R4,R4,R3,ASR #4
	MOV	R3,R2

		@ ;Add 115/64 of last sample -----------
		@ And	DL, ~3
		@ Add	EAX,EDX
		@ Add	EAX,EDX
		@ SAR	EDX,3
		@ Sub	EAX,EDX
		@ SAR	EDX,1
		@ Sub	EAX,EDX
		@ SAR	EDX,2
		@ Sub	EAX,EDX
	BIC	R2,R2,#3
	ADD	R4,R4,R2,LSL #1
	SUB	R4,R4,R2,ASR #3
	SUB	R4,R4,R2,ASR #4
	SUB	R4,R4,R2,ASR #6

		@ Mov	word[EDI],AX
	STRH	R4,[R1],#2

		@ Mov	DL,byte[ESI]
	LDRB	R2,[R0],#1

		@ ShL	EDX,12
		@ MovSX	EDX,DX
		@ SAR	EDX,CL
	MOV	R2,R2,LSL #(12+16)
	MOV	R2,R2,ASR R7

		@ Sub	EDX,EBX
		@ SAR	EBX,3
		@ Add	EDX,EBX
		@ SAR	EBX,1
		@ Add	EDX,EBX
		@ MovSX	EBX,AX
	SUB	R2,R2,R3
	ADD	R2,R2,R3,ASR #3
	ADD	R2,R2,R3,ASR #4
	MOV	R3,R4

		@ And	AL, ~3
		@ Add	EDX,EAX
		@ Add	EDX,EAX
		@ SAR	EAX,3
		@ Sub	EDX,EAX
		@ SAR	EAX,1
		@ Sub	EDX,EAX
		@ SAR	EAX,2
		@ Sub	EDX,EAX
	BIC	R4,R4,#3
	ADD	R2,R2,R4,LSL #1
	SUB	R2,R2,R4,ASR #3
	SUB	R2,R2,R4,ASR #4
	SUB	R2,R2,R4,ASR #6

		@ Mov	word[2+EDI],DX
	STRH	R4,[R1],#2
		@ Add	EDI,4

		@ Inc	ESI

	@ Dec	CH
	@ JNZ	@@Method3
	@ Ret
	SUBS	R6,R6,#1
	BNE	Method3loop
	
	LDMFD	SP!,{R5,R6,R7}
	MOV	PC,LR	


DecodeBlockAsm2:
	STMFD		SP!,{R4,R11,LR}
	
	@ R12 = -32768
	LDR		R12, = (-32768)
	@ R11 = 32767
	LDR		R11, = 32767
	
	
	@ R0 = source (compressed)
	@ R1 = destination (uncompressed)
	@ 
	
	/*mov esi,dword[esp+20]
	mov edi,dword[esp+24]
	mov edx,dword[esp+28]
	mov edx,dword[edx]
	mov ebx,dword[esp+32]
	mov ebx,dword[ebx]*/

	@ call BREWave2
	@ sauvgarde les valeurs de R2 et R3 (&ch->prev[0] et &...[1])
	STMFD		SP!,{R2,R3}	
	LDR		R2,[R2]
	LDR		R3,[R3]	
	BL		BREWave2
		@ renvoie dans R2 et R3 les valeurs a mettre dans &ch->prev[0] et &ch...[1]
	/*mov eax,dword[esp+28]
	mov dword[eax],edx
	mov eax,dword[esp+32]
	mov dword[eax],ebx*/
	LDMFD		SP!,{R0,R1}
	STR		R2,[R0]
	STR		R3,[R1]
		
	LDMFD		SP!,{R4,R11,LR}
	MOV		PC,LR	
	@ ret

@ ALIGN	16
BREWave2:
	@ Push	CX	
	STMFD	SP!,{R5,R6}	
	@ Mov	AL,byte [esi]                         ;Get header byte
	@ Inc	esi
	LDRB	R4,[R0],#1	
	@ Mov	CL,0CFh
	@ Sub	CL,AL                              ;Invert range
	@ ShR	CL,4                               ;Isolate range
	MOV	R5,#0xCF
	SUB	R5,R5,R4
	MOV	R5,R5,LSR #4	
	@ Mov	CH,8                               ;Decompress 8 bytes (16 nybbles)
	MOV	R6,#8	
	@ Test	AL,0Ch                             ;Does block use ADPCM compression?
	@ JZ 	@@Method02                    ;  No
	TST	R4,#0xC
	BEQ	Method02
	@ Test	AL,8                               ;Does block use method 1?
	@ JZ 	@@Method12                    ;  Yes
	TST	R4,#0x8
	BEQ	Method12
	@ Test	AL,4                               ;Does block use method 2?
	@ jnz 	@@Method332                          ;  Yes
	@ jmp	@@Method22
	TST	R4,#0x4
	BEQ	Method22
	B	Method32
@ @@Method332:
	@ Jmp	@@Method32                          ;Must use method 3	

@ ALIGN	16
@ 	;[Smp] ----------------------------------
@ @@Method02:
Method02:
	@ shr	al, 4
	MOV	R4,R4,LSR #4
	@ mov	cl, al
	MOV	R5,R4
	@ cmp	cl, 12
	CMP	R4,#12	
	@ jbe	@@Method022	
	SUBLS	R5,R5,#4
	@ sub cl, 4
Method022:
	@ XOr	AX,AX	
	@ XOr	DX,DX	
	@ Mov	al,byte [esi]                      ;Get byte
	LDRSB	R4,[R0],#1
	@ mov	dl, al	
	@ sar	al, 4
	@ sal	dl, 4
	@ sar	dl, 4
	@ movsx eax, al
	@ movsx edx, dl
	MOV	R2,R4,LSL #28
	MOV	R4,R4,ASR #4	@ top nib
	MOV	R2,R2,ASR #28  @ bottom nib			
	@ sal	eax,cl                           ;Reduce samples according to range
	@ sal	edx,cl
	MOV	R4,R4,ASL R5   @ <<shift
	MOV	R2,R2,ASL R5	@ <<shift

	@ call SMP12CLIP16
	SMP12CLIP16		@ clip

	@ Mov	[edi],AX
	STRH	R4,[R1],#2	@ store
	@ Mov	[2+edi],DX
	STRH	R2,[R1],#2
	@ Add	edi,4
	@ Inc	esi 	@ deja fait plus haut	
	@ Dec	CH
	SUBS	R6,R6,#1
	@ JNZ	@@Method022
	BNE	Method022
	@ Pop	CX
	LDMFD	SP!,{R5,R6}
	@ MovSX	EDX,DX
		@ deja bon
	@ MovSX	EBX,AX	
	MOV	R3,R4
	
	@ Ret
	MOV	PC,LR

@ ALIGN	16
@ 	;[Delta]+[Smp-1](15/16) -----------------
@ @@Method12:
Method12:
	ADD	R5,R5,#16
Method12loop:	
@ 	MovSX	EBX,byte [esi]            ;Sign extend upper nybble into EBX
@ 	And	BL,0F0h
	LDRB	R3,[R0]
	BIC	R3,R3,#0xF
	
	@ ShL	BX,8
	@ SAR	BX,CL	
	MOV	R3,R3, LSL #(8+16)
	MOV	R3,R3,ASR R5
	
	@ MovSX	EAX,DX		
	@ Add	EBX,EAX	
	@ SAR	EAX,4
	@ Sub	EBX,EAX	
	MOV	R4,R2,LSL #16
	ADD	R3,R3,R4,ASR #16		@ + 15/16*(smp-1)
	SUB	R3,R3,R4,ASR #20
	
	@ Mov	[edi],BX
	STRH	R3,[R1],#2

	@ Mov	DL,byte [esi]
	LDRB	R2,[R0],#1	
	@ ShL	DX,12	
	@ SAR	DX,CL
	@ MovSX	EDX,DX
	MOV	R2,R2,LSL #(12+16)
	MOV	R2,R2,ASR R5
	
	
	@ MovSX	EAX,BX	
	@ Add	EDX,EAX		
	@ SAR	EAX,4
	@ Sub	EDX,EAX	
	MOV	R4,R3,LSL #16
	ADD	R2,R2,R4,ASR #16
	SUB	R2,R2,R4,ASR #20   	@ + 15/16*(smp-1)
		
	@ Mov	[2+edi],DX
	STRH	R2,[R1],#2
	@ Add	edi,4  @ deja fait
	@ Inc	esi	@ deja fait
	@ Dec	CH	
	@ JNZ	@@Method12
	SUBS	R6,R6,#1
	BNE	Method12loop
	@ Pop	CX
	@ MovSX	EBX,BX
	@ Ret	
	MOV	R3,R3,LSL #16		
	LDMFD	SP!,{R5,R6}
	MOV	R3,R3,ASR #16
	MOV	PC,LR

@ ALIGN	16
@ 	;[Delta]+[Smp-1](61/32)-[Smp-2](30/32) --
@ @@Method22:
Method22:	
	/*MOV	R2,#0
	MOV	R3,#0
	LDMFD	SP!,{R5,R6}
	MOV	PC,LR*/	
	ADD	R5,R5,#16
Method22loop:	

@ 	MovSX	EAX,byte [esi]            ;EAX =3D Delta
	LDRB	R4,[R0]
@ 	And	AL,0F0h
	BIC	R4,R4,#0xF
@ 	ShL	AX,8
@ 	SAR	AX,CL
	MOV	R4,R4,LSL #(8+16)
	MOV	R4,R4,ASR R5
	
@ 	;Subtract 15/16 of second sample -----
@ 	Sub	EAX,EBX
	SUB	R4,R4,R3
@ 	SAR	EBX,4	
@ 	Add	EAX,EBX
	ADD	R4,R4,R3,ASR #4
@ 	MovSX	EBX,DX                          ;Truncate lower 16-bits	
	MOV	R3,R2, LSL #16
	MOV	R3,R3, ASR #16
	
@ 	;Add 61/32 of last sample ------------
@ 	And	DL,~3                        ;(Lose lower 2-bits of precision)
	BIC	R2,R2,#3
@ 	Add	EAX,EDX
@ 	Add	EAX,EDX
	ADD	R4,R4,R2
	ADD	R4,R4,R2
@ 	SAR	EDX,5
	MOV	R2,R2,ASR #5
@ 	Sub	EAX,EDX	
	SUB	R4,R4,R2
@ 	ShL	EDX,1	
@ 	MovSX	EDX,DX	
	MOV	R2,R2,LSL #(1+16)
@ 	Sub	EAX,EDX
	SUB	R4,R4,R2,ASR #(16)
	
@ 	Mov	[edi],AX
	STRH	R4,[R1],#2

@ 	Mov	DL,byte [esi]
	LDRB	R2,[R0],#1
@ 	ShL	DX,12
@ 	SAR	DX,CL
@ 	MovSX	EDX,DX
	MOV	R2,R2,LSL #(12+16)
	MOV	R2,R2,ASR R5
			
@ 	Sub	EDX,EBX
	SUB	R2,R2,R3
@ 	SAR	EBX,4
@ 	Add	EDX,EBX
	ADD	R2,R2,R3,ASR #4
@ 	MovSX	EBX,AX
	MOV	R3,R4,LSL #16
	MOV	R3,R3,ASR #16
		
@ 	And	AL,~3
	BIC	R4,R4,#3
@ 	Add	EDX,EAX
@ 	Add	EDX,EAX
	ADD	R2,R2,R4
	ADD	R2,R2,R4
@ 	SAR	EAX,5
	MOV	R4,R4,ASR #5
@ 	Sub	EDX,EAX
	SUB	R2,R2,R4	
@ 	ShL	EAX,1	
@ 	MovSX	EAX,AX	
	MOV	R4,R4,LSL #(1+16)
@ 	Sub	EDX,EAX		
	SUB	R2,R2,R4,ASR #(16)
	

@ 	Mov	[2+edi],DX
	STRH	R2,[R1],#2
@ 	Add	edi,4

@ 	Inc	esi

@ 	Dec	CH
@ 	JNZ	@@Method22
	SUBS	R6,R6,#1
	BNE	Method22loop
@ 	Pop	CX
@ 	Ret	
	LDMFD	SP!,{R5,R6}
	MOV	PC,LR

@ ALIGN	16
@ 	;[Delta]+[Smp-1](115/64)-[Smp-2](52/64) -
@ @@Method32:
Method32:	
	/*MOV	R2,#0
	MOV	R3,#0
	LDMFD	SP!,{R5,R6}
	MOV	PC,LR*/	
	ADD	R5,R5,#16
Method32loop:
@ 	MovSX	EAX,byte [esi]
	LDRB	R4,[R0]
@ 	And	AL,0F0h
	BIC	R4,R4,#0xF
@ 	ShL	AX,8
@ 	SAR	AX,CL
	MOV	R4,R4,LSL #(8+16)
	MOV	R4,R4,ASR R5

@ 	;Subtract 13/16 of second sample -----
@ 	Sub	EAX,EBX
	SUB	R4,R4,R3
@ 	SAR	EBX,3
@ 	Add	EAX,EBX
	ADD	R4,R4,R3,ASR #3
@ 	SAR	EBX,1
@ 	Add	EAX,EBX
	ADD	R4,R4,R3,ASR #4
@ 	MovSX	EBX,DX
	MOV	R3,R2,LSL #16
	MOV	R3,R3,ASR #16

@ 	;Add 115/64 of last sample -----------
@ ;	And	DL,~3
@ 	Add	EAX,EDX
@ 	Add	EAX,EDX
	ADD	R4,R4,R2
	ADD	R4,R4,R2
@ 	SAR	EDX,3
@ 	Sub	EAX,EDX
	SUB	R4,R4,R2,ASR #3
@ 	SAR	EDX,1
@ 	Sub	EAX,EDX
	SUB	R4,R4,R2,ASR #4
@ 	SAR	EDX,2
@ 	Sub	EAX,EDX
	SUB	R4,R4,R2,ASR #6

@ 	Mov	[edi],AX
	STRH	R4,[R1],#2

@ 	Mov	DL, [esi]
	LDRB	R2,[R0],#1
@ 	ShL	DX,12
@ 	SAR	DX,CL
@ 	MovSX	EDX,DX
	MOV	R2,R2,LSL #(12+16)
	MOV	R2,R2,ASR R5

@ 	Sub	EDX,EBX
	SUB	R2,R2,R3
@ 	SAR	EBX,3
@ 	Add	EDX,EBX
	ADD	R2,R2,R3,ASR #3
@ 	SAR	EBX,1
@ 	Add	EDX,EBX
	ADD	R2,R2,R3,ASR #4
@ 	MovSX	EBX,AX
	MOV	R3,R4,LSL #16
	MOV	R4,R4,ASR #16

@ ;	And	AL,~3
@ 	Add	EDX,EAX
@ 	Add	EDX,EAX
	ADD	R2,R2,R4
	ADD	R2,R2,R4
@ 	SAR	EAX,3
@ 	Sub	EDX,EAX
	SUB	R2,R2,R4,ASR #3
@ 	SAR	EAX,1
@ 	Sub	EDX,EAX
	SUB	R2,R2,R4,ASR #4
@ 	SAR	EAX,2
@ 	Sub	EDX,EAX
	SUB	R2,R2,R4,ASR #6

@ 	Mov	[2+edi],DX
	STRH	R2,[R1],#2
@ 	Add	edi,4

@ 	Inc	esi

@ 	Dec	CH
@ 	JNZ	@@Method32
	SUBS	R6,R6,#1
	BNE	Method32loop
@ 	Pop	CX
@ 	Ret		
	LDMFD	SP!,{R5,R6}
	MOV	PC,LR

/*SMP12CLIP16:
	cmp eax, -32768
	jg CLIP16SMP1SKIP0
	je CLIP16SMP1SKIP1
	mov eax, -32768
	jmp CLIP16SMP1SKIP1
CLIP16SMP1SKIP0:
	cmp eax, 32767
	jle CLIP16SMP1SKIP1
	mov eax, 32767
CLIP16SMP1SKIP1:
	cmp edx, -32768
	jg CLIP16SMP2SKIP0
	je CLIP16SMP2SKIP1
	mov edx, -32768
	jmp CLIP16SMP2SKIP1
CLIP16SMP2SKIP0:
	cmp edx, 32767
	jle CLIP16SMP2SKIP1
	mov edx, 32767
CLIP16SMP2SKIP1:*/


.pool
