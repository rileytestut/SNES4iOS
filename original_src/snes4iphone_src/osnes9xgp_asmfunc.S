        .global asmClearScreenFast8        
        .global asmClearScreenFast16
        
asmClearScreenFast16:
;@R0 = scr
;@R1 = starty
;@R2 = endy
;@R3 = col
		stmfd	R13!,{R4-R11}
		
		sub		R12,R2,R1
		add		R12,R12,#1
		;@R12 = endy-starty+1
		
		mov r2,#320
		mul r1,r2,r1
		;@R1 = 320*starty
		
		add		R0,R0,R1,lsl #1
		;@R0 = screen + ((320*starty)<<1)
		
		mov		R2,R3
		mov 		R4,R3
		mov		R5,R3
		mov		R6,R3
		mov		R7,R3
		mov		R8,R3
		mov		R9,R3
2:
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		
		add r0,r0,#32*4
	
	    subs 		R12,R12,#1
		bne 		2b
		
		ldmfd 	R13!,{R4-R11}
        mov	  	PC,LR        
         
		
asmClearScreenFast8:
;R0 = scr
;R1 = starty
;R2 = endy
;R3 = col
		stmfd	R13!,{R4-R11}
		
		sub		R12,R2,R1
		add		R12,R12,#1
		;@R12 = endy-starty+1
		
		mov r2,#320
		mul r1,r2,r1
		;@R1 = 320*starty
		
		add		R0,R0,R1
		;@R0 = screen + ((320*starty))

		mov		R2,R3
		mov 		R4,R3
		mov		R5,R3
		mov		R6,R3
		mov		R7,R3
		mov		R8,R3
		mov		R9,R3
	
2:		
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}
		stmia  	R0!,{R2,R3,R4,R5,R6,R7,R8,R9}

		subs 		R12,R12,#1
		bne 2b
	
		ldmfd 	R13!,{R4-R11}
        mov	  	PC,LR        
		
		