
	.align 4
	.globl  funcSMULT1616
	.globl  funcUMULT1616
	.globl  funcSMULT32
	.globl  funcUMULT32
	.globl  funcUADDMULT1616
	.globl  funcSADDMULT1616
	.globl  funcSADDMULT32
	.globl  funcUADDMULT32
	
	
		
// Int32 funcSMULT1616(Int32 a,Int32 b,Int32 dummy1,Int32 dummy2)
funcSMULT1616:	
	mov   r2,r0
	mov   r3,r1
	smull r0,r1,r3,r2
	mov   r0,r0,lsr #16
	orr   r0,r0,r1,lsl #16
	mov	pc,lr


// Int32 funcUMULT1616(UInt32 a,UInt32 b,Int32 dummy1,Int32 dummy2)
funcUMULT1616:
	mov   r2,r0
	mov   r3,r1
	umull r0,r1,r3,r2
    mov   r0,r0,lsr #16
    orr   r0,r0,r1,lsl #16
		mov	pc,lr
			
// Int32 funcSMULT32(Int32 a,Int32 b,Int32 dummy1,Int32 dummy2)
funcSMULT32:
	mov	  r2,r0
	mov	  r3,r1
	smull r1,r0,r2,r3
		mov	pc,lr	
	

// UInt32 funcUMULT32(UInt32 a,UInt32 b,Int32 dummy1,Int32 dummy2)
funcUMULT32:
	mov   r2,r0
	mov	  r3,r1
	umull r1,r0,r3,r2
		mov	pc,lr
			
// UInt32 funcUADDMULT1616(UInt32 a,UInt32 b,UInt32 c,UInt32 d)
funcUADDMULT1616:
	stmfd r13!,{r4,r5}
	mov	  r4,r0
	mov	  r5,r1
	umull r0,r1,r5,r4
	umlal r0,r1,r2,r3
	mov   r0,r0,lsr #16
	orr   r0,r0,r1,lsl #16
	ldmfd r13!,{r4,r5}
		mov	pc,lr
			
// Int32 funcSADDMULT1616(Int32 a,Int32 b,Int32 c,Int32 d)
funcSADDMULT1616:
	stmfd r13!,{r4,r5}
	mov	  r4,r0
	mov	  r5,r1
	smull r0,r1,r5,r4
	smlal r0,r1,r2,r3
	mov   r0,r0,lsr #16
	orr   r0,r0,r1,lsl #16
	ldmfd r13!,{r4,r5}	
		mov	pc,lr

// Int32 funcSADDMULT32(Int32 a,Int32 b,Int32 c,Int32 d)
funcSADDMULT32:
	stmfd r13!,{r4,r5}
	mov	  r4,r0
	mov	  r5,r1
	smull r1,r0,r5,r4
	smlal r1,r0,r2,r3
	ldmfd r13!,{r4,r5}	
		mov	pc,lr
			
// UInt32 funcUADDMULT32(UInt32 a,UInt32 b,UInt32 c,UInt32 d)
funcUADDMULT32:
	stmfd r13!,{r4,r5}
	mov	  r4,r0
	mov	  r5,r1
	umull r1,r0,r5,r4
	umlal r1,r0,r2,r3
	ldmfd r13!,{r4,r5}	
		mov	pc,lr
		

.pool
