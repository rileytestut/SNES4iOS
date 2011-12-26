    
	.global BlitBufferToScreen

BlitBufferToScreen:  
    ;@ r0 - Buffer to copy from
	;@ r1 - Buffer to copy to
	stmfd sp!,{r4-r12,lr}
	mov r12,#240>>1
1:
    ;@ first line is perfectly aligned
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	ldmia r0!,{r2-r11}
	stmia r1!,{r2-r11}
	
	add r1,r1,#2

	ldmia r0!,{r2-r6}
	strh r2,[r1],#2
	mov lr,#15
2:
	mov r2,r2,lsr#16
	orr r2,r2,r3,lsl#16
	mov r3,r3,lsr#16
	orr r3,r3,r4,lsl#16
	mov r4,r4,lsr#16
	orr r4,r4,r5,lsl#16
	mov r5,r5,lsr#16
	orr r5,r5,r6,lsl#16
	mov r6,r6,lsr#16
	ldmia r0!,{r7-r11}
	orr r6,r6,r7,lsl#16
	stmia r1!,{r2-r6}
	mov r7,r7,lsr#16
	orr r7,r7,r8,lsl#16
	mov r8,r8,lsr#16
	orr r8,r8,r9,lsl#16
	mov r9,r9,lsr#16
	orr r9,r9,r10,lsl#16
	mov r10,r10,lsr#16
	orr r10,r10,r11,lsl#16
	mov r11,r11,lsr#16
	ldmia r0!,{r2-r6}
	orr r11,r11,r2,lsl#16
	stmia r1!,{r7-r11}
	subs lr,lr,#1
	bne 2b
	
	mov r2,r2,lsr#16
	orr r2,r2,r3,lsl#16
	mov r3,r3,lsr#16
	orr r3,r3,r4,lsl#16
	mov r4,r4,lsr#16
	orr r4,r4,r5,lsl#16
	mov r5,r5,lsr#16
	orr r5,r5,r6,lsl#16
	mov r6,r6,lsr#16
	ldmia r0!,{r7-r11}
	orr r6,r6,r7,lsl#16
	stmia r1!,{r2-r6}
	mov r7,r7,lsr#16
	orr r7,r7,r8,lsl#16
	mov r8,r8,lsr#16
	orr r8,r8,r9,lsl#16
	mov r9,r9,lsr#16
	orr r9,r9,r10,lsl#16
	mov r10,r10,lsr#16
	orr r10,r10,r11,lsl#16
	mov r11,r11,lsr#16
	stmia r1!,{r7-r11}
	
	subs r12,r12,#1
    bne 1b
	
	ldmfd sp!,{r4-r12,pc}



