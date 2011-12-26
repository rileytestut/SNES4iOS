	.TEXT
	.ARM
	.ALIGN

#include "mixrate.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function called with:
@ r0 - Raw brr data (s8*)
@ r1 - Decoded sample data (s16*)
@ r2 - DspChannel *channel
@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function Data:
@ r4 - r4
@ r5 - r5
@ r6,r7 - tmp
@ r8 - shift amount
@ r9 - number of iterations left
@ r10 - 0xf
@ r11 - low clip
@ r12 - high clip


.GLOBAL brrHash
brrHash:
.word 0

.GLOBAL DecodeSampleBlockAsm
DecodeSampleBlockAsm:
    stmfd sp!, {r4-r12,r14}

    @ Save the channel pointer
    mov r14, r2
    
/*    @ Hash the block, and skip the decode if we can
    ldmia r0, {r4-r7}
    ldr r8, =0x050C5D1F @ (2166136261 * 16777619)
    ldr r9, =16777619
    eor r8, r8, r4
    mul r8, r9, r8
    eor r8, r8, r5
    mul r8, r9, r8
    eor r8, r8, r6

    @ r8 is the hashed value

    ldr r4, brrHash

    @ Compute the actual brr location minus the apu ram base
    ldr r6, =APU_MEM
    ldr r6, [r6]
    sub r6, r0, r6

    @ Compute the address of the cached samples in brrHash
    add r3, r4, #0x8000
    mov r6, r6, lsr #3
    add r3, r3, r6, lsl #5

    @ Load the previous hash value
    ldr r5, [r4, r6, lsl #2]
    str r8, [r4, r6, lsl #2]
    cmp r5, r8
    bne doDecode

    @ Load the cached samples
    ldmia r3, {r4-r11}
    stmia r1!, {r4-r11}

    ldrsh r2, [r1, #-2]
    ldrsh r3, [r1, #-4]

    b doneDecodeCached

doDecode:
    stmfd sp!, {r3}
*/
    @ Load r2 and r3
    ldrsh r2, [r14, #62]
    ldrsh r3, [r14, #64]
    
    ldrb r4, [r0], #1
    @ Compute the index into the brrTab to load the bytes from
    mov r9, r4, lsr #4
    ldr r8, =brrTab
    add r8, r8, r9, lsl #5 @ brrTabPtr = brrTab + (r4 * 32)

    mov r10, #0xf << 1
    ldr r11, =0xffff8000
    ldr r12, =0x7fff

    @ 16 samples to decode, but do two at a time
    mov r9, #8
    @ Figure out the type of decode filter
    mov r4, r4, lsr #2
    and r4, r4, #3
    ldr pc, [pc, r4, lsl #2]
    nop
.word case0
.word case1
.word case2
.word case3
case0:
    ldrb r4, [r0], #1
    and r5, r10, r4, lsl #1
    ldrsh r5, [r8, r5]
    and r4, r10, r4, lsr #3
    ldrsh r4, [r8, r4]

    mov r4, r4, lsl #1
    mov r5, r5, lsl #1
    strh r4, [r1], #2
    strh r5, [r1], #2

    subs r9, r9, #1
    bne case0

    @ Set up r2 and r3
    ldrsh r2, [r1, #-2]
    ldrsh r3, [r1, #-4]
    
    b doneDecode

case1:
    ldrb r4, [r0], #1
    and r5, r10, r4, lsl #1
    ldrsh r5, [r8, r5]
    and r4, r10, r4, lsr #3
    ldrsh r4, [r8, r4]

    @ r3 = r4 + (last1 >> 1) - (last1 >> 5)    
    add r3, r4, r2, asr #1
    sub r3, r3, r2, asr #5
    
    cmp r3, r12
    movgt r3, r12
    cmp r3, r11
    movlt r3, r11

    mov r3, r3, lsl #1
    strh r3, [r1], #2
    ldrsh r3, [r1, #-2]

    @ same for r2 now
    add r2, r5, r3, asr #1
    sub r2, r2, r3, asr #5

    cmp r2, r12
    movgt r2, r12
    cmp r2, r11
    movlt r2, r11

    mov r2, r2, lsl #1
    strh r2, [r1], #2
    ldrsh r2, [r1, #-2]

    subs r9, r9, #1
    bne case1

    b doneDecode

case2:
    ldrb r4, [r0], #1
    and r5, r10, r4, lsl #1
    ldrsh r5, [r8, r5]
    and r4, r10, r4, lsr #3
    ldrsh r4, [r8, r4]

    @ Sample 1
    mov r6, r3, asr #1
    rsb r6, r6, r3, asr #5
    mov r3, r2
    add r7, r2, r2, asr #1
    rsb r7, r7, #0
    add r6, r6, r7, asr #5
    add r7, r4, r2
    add r2, r6, r7

    cmp r2, r12
    movgt r2, r12
    cmp r2, r11
    movlt r2, r11
    mov r2, r2, lsl #1
    strh r2, [r1], #2
    ldrsh r2, [r1, #-2]

    @ Sample 2
    mov r6, r3, asr #1
    rsb r6, r6, r3, asr #5
    mov r3, r2
    add r7, r2, r2, asr #1
    rsb r7, r7, #0
    add r6, r6, r7, asr #5
    add r7, r5, r2
    add r2, r6, r7

    cmp r2, r12
    movgt r2, r12
    cmp r2, r11
    movlt r2, r11
    mov r2, r2, lsl #1
    strh r2, [r1], #2
    ldrsh r2, [r1, #-2]
    
    subs r9, r9, #1
    bne case2

    b doneDecode

case3:
    ldrb r4, [r0], #1
    and r5, r10, r4, lsl #1
    ldrsh r5, [r8, r5]
    and r4, r10, r4, lsr #3
    ldrsh r4, [r8, r4]

    @ Sample 1
    add r6, r3, r3, asr #1
    mov r6, r6, asr #4
    sub r6, r6, r3, asr #1
    mov r3, r2
    add r7, r2, r2, lsl #2
    add r7, r7, r2, lsl #3
    rsb r7, r7, #0
    add r6, r6, r7, asr #7
    add r6, r6, r2
    add r2, r4, r6

    cmp r2, r12
    movgt r2, r12
    cmp r2, r11
    movlt r2, r11
    mov r2, r2, lsl #1
    strh r2, [r1], #2
    ldrsh r2, [r1, #-2]

    @ Sample 2
    add r6, r3, r3, asr #1
    mov r6, r6, asr #4
    sub r6, r6, r3, asr #1
    mov r3, r2
    add r7, r2, r2, lsl #2
    add r7, r7, r2, lsl #3
    rsb r7, r7, #0
    add r6, r6, r7, asr #7
    add r6, r6, r2
    add r2, r5, r6

    cmp r2, r12
    movgt r2, r12
    cmp r2, r11
    movlt r2, r11
    mov r2, r2, lsl #1
    strh r2, [r1], #2
    ldrsh r2, [r1, #-2]

    subs r9, r9, #1
    bne case3

doneDecode:
/*    sub r1, r1, #32
    ldmia r1, {r4-r11}
    ldmfd sp!, {r1}
    stmia r1, {r4-r11}*/

doneDecodeCached:
    @ Store r2 and r3
    strh r2, [r14, #62]
    strh r3, [r14, #64]

    ldmfd sp!, {r4-r12,r14}
    bx lr

#define ENVSTATE_INCREASE	6
#define ENVSTATE_BENTLINE	7
#define ENVSTATE_DECREASE	8
#define ENVSTATE_DECEXP		9

@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Function called with:
@ r0 - int Number of samples to mix
@ r1 - u16* mix buffer (left first, right is always 4000 * 4 bytes ahead
@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#define PREVDECODE_OFFSET 16
#define BLOCKPOS_OFFSET 66
#define KEYWAIT_OFFSET 76

@ r0 - channel structure base
@ r1 - mix buffer
@ r2 - echo buffer ptr
@ r3 - numSamples
@ r4 - sampleSpeed
@ r5 - samplePos
@ r6 - envCount
@ r7 - envSpeed
@ r8 - sampleValue (value of the current sample)
@ r9 - tmp
@ r10 - leftCalcVol
@ r11 - rightCalcVol
@ r12 - tmp
@ r13 - tmp
@ r14 - tmp

.GLOBAL DspMixSamplesStereo
.FUNC DspMixSamplesStereo
DspMixSamplesStereo:
    stmfd sp!, {r4-r12, lr}

    mov r3, #0
    strb r3, channelNum
    str r0, numSamples

    @ Store the original mix buffer for use later
    stmfd sp!, {r1}

    @ Clear the left and right mix buffers, saving their initial positions
    ldr r1, =r1
    ldr r2, =echoBuffer
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
clearLoop:
    stmia r1!, {r3-r6}
    stmia r1!, {r3-r6}
    stmia r2!, {r3-r6}
    stmia r2!, {r3-r6}
    subs r0, r0, #4
    cmp r0, #0
    bgt clearLoop

    @ Load the initial mix buffer and echo position
    ldr r1, =r1
    ldr r2, =echoBuffer

    ldr r0, =channels
channelLoopback:
    @ Check if active == 0, then next
    ldrb r3, [r0, #77]
    cmps r3, #0
    beq nextChannelNothingDone

    @ Save the start position of the mix buffer & echo buffer
    stmfd sp!, {r1,r2}

    @ Get echo enabled, then replace the opcode there if it's enabled
    ldrb r14, [r0, #79]
    cmp r14, #1
    ldr r3, =0x01A00000 @ mov r0, r0
    streq r3, branchLocation

    ldrb r3, numSamples
    @ Load the important variables into registers
    ldmia r0, {r4-r7}
    ldrsh r10, [r0, #68]
    ldrsh r11, [r0, #70]

mixLoopback:

    @ Commence the mixing
    subs r6, r6, r7
    bpl noEnvelopeUpdate

    @ Update envelope
    mov r6, #0x7800

    ldrsh r9, [r0, #60]
    ldrb r12, [r0, #72]

    ldr pc, [pc, r12, lsl #2]
    nop
@ Jump table for envelope handling
.word noEnvelopeUpdate
.word envStateAttack
.word envStateDecay
.word envStateSustain
.word envStateRelease
.word noEnvelopeUpdate      @ Actually direct, but we don't need to do anything
.word envStateIncrease
.word envStateBentline
.word envStateDecrease
.word envStateSustain       @ Actually decrease exponential, but it's the same code

envStateAttack:
    add r9, r9, #4 << 8

    cmp r9, #0x7f00
    ble storeEnvx
    @ envx = 0x7f, state = decay, speed = decaySpeed
    mov r9, #0x7f00
    mov r12, #2
    strb r12, [r0, #72]
    ldrh r7, [r0, #56]
    b storeEnvx
    
envStateDecay:
    rsb r9, r9, r9, lsl #8
    mov r9, r9, asr #8

    ldrb r12, [r0, #73]
    cmp r9, r12, lsl #8
    bge storeEnvx
    @ state = sustain, speed = sustainSpeed
    mov r12, #3
    strb r12, [r0, #72]
    ldrh r7, [r0, #58]
    
    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx
    
    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3, r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3, r14}
    b nextChannel
    
envStateSustain:
    rsb r9, r9, r9, lsl #8
    mov r9, r9, asr #8

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx

    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

envStateRelease:
    sub r9, r9, #1 << 8

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx

    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

envStateIncrease:
    add r9, r9, #4 << 8

    cmp r9, #0x7f00
    ble storeEnvx
    @ envx = 0x7f, state = direct, speed = 0
    mov r9, #0x7f00
    mov r12, #5
    strb r12, [r0, #72]
    mov r7, #0
    b storeEnvx

envStateBentline:
    cmp r9, #0x5f << 8
    addgt r9, r9, #1 << 8
    addle r9, r9, #4 << 8

    cmp r9, #0x7f00
    blt storeEnvx
    @ envx = 0x7f, state = direct, speed = 0
    mov r9, #0x7f00
    mov r12, #5
    strb r12, [r0, #72]
    mov r7, #0
    b storeEnvx

envStateDecrease:
    sub r9, r9, #4 << 8

    @ Make sure envx > 0
    cmp r9, #0
    bge storeEnvx
    
    @ If not, end channel, then go to next channel
    stmfd sp!, {r0-r3,r14}
    ldrb r0, channelNum
    bl DspSetEndOfSample
    ldmfd sp!, {r0-r3,r14}
    b nextChannel

storeEnvx:
    strh r9, [r0, #60]

    @ Recalculate leftCalcVol and rightCalcVol
    ldrsb r10, [r0, #74]
    mul r10, r9, r10
    mov r10, r10, asr #7

    ldrsb r11, [r0, #75]
    mul r11, r9, r11
    mov r11, r11, asr #7
    
noEnvelopeUpdate:
    add r5, r5, r4
    cmp r5, #16 << 12
    blo noSampleUpdate
    
    @ Decode next 16 bytes...
    sub r5, r5, #16 << 12

    @ Decode the sample block, r0 = DspChannel*
    stmfd sp!, {r0-r3, r14}
    bl DecodeSampleBlock
    cmps r0, #1
    ldmfd sp!, {r0-r3, r14}
    beq nextChannel

noSampleUpdate:
    @ This is really a >> 12 then << 1, but since samplePos bit 0 will never be set, it's safe.
    @ Must ensure that sampleSpeed bit 0 is never set, and samplePos is never set to anything but 0
    @ TODO - The speed up hack doesn't work.  Find out why
    mov r12, r5, lsr #12
    add r12, r0, r12, lsl #1
    ldrsh r8, [r12, #24]

branchLocation:
    b mixEchoDisabled

mixEchoEnabled:
    @ Echo mixing
    ldr r9, [r2]
    mla r9, r8, r10, r9
    str r9, [r2], #4

    ldr r9, [r2]
    mla r9, r8, r11, r9
    str r9, [r2], #4

mixEchoDisabled:
    ldr r9, [r1]
    mla r9, r8, r10, r9
    str r9, [r1], #4

    ldr r9, [r1]
    mla r9, r8, r11, r9
    str r9, [r1], #4

    subs r3, r3, #1
    bne mixLoopback

nextChannel:

    @ Set ENVX and OUTX
    ldrb r3, channelNum
    ldr r12, =DSP_MEM
    add r12, r12, r3, lsl #4

    @ Set ENVX
    ldrsh r9, [r0, #60]
    mov r9, r9, asr #8
    strb r9, [r12, #0x8]

    @ Set OUTX
    mul r9, r8, r9
    mov r9, r9, asr #15
    strb r9, [r12, #0x9]
    
    strh r10, [r0, #68]
    strh r11, [r0, #70]

    @ Store changing values
    stmia r0, {r4-r7}

    @ Reload mix&echo buffer position
    ldmfd sp!, {r1,r2}

nextChannelNothingDone:
    @ Move to next channel
    add r0, r0, #80

    @ Increment channelNum
    ldrb r3, channelNum
    add r3, r3, #1
    strb r3, channelNum
    cmps r3, #8
    blt channelLoopback

@ This is the end of normal mixing

#ifdef NEVER
    @ Store the original mix & echo buffers, cause we trash these regs
    stmfd sp!, {r1, r2}

    @ r0 - 
    @ r1 - 
    @ r2 - 
    @ r3 - 
    @ r4 - 
    @ r5 - echo volume (right)
    @ r6 - numSamples
    @ r7 - echo in apu ram (r/w)
    @ r8 - echo mix buffer (r/w)
    @ r9 - end of echo in apu ram
    @ r10 - echo volume (left)
    @ r11 - echo feedback
    @ r12 - FIR coefficients in DSP ram
    @ r13 - FIR table base
    @ r14 - FIR offset

@ Process the echo filter stuff
echoMixSetup:
    mov r8, r2

    ldr r0, =DSP_MEM

    ldrsb r10, [r0, #0x2C] @ Get left echo volume
    mov r10, r10, lsl #7
    ldrsb r5, [r0, #0x3C] @ Get right echo volume
    mov r5, r5, lsl #7

    @ Get echo feedback
    ldrsb r11, [r0, #0x0D]

    @ Check if echo is enabled
    ldrb r1, [r0, #0x6C]
    strb r1, echoEnabled
    @ Get echo base (APU_MEM + DSP_ESA << 8)
    ldr r7, =echoBase
    ldr r7, [r7]
    str r7, echoBufferStart
    @ Set up end of echo delay area in r8
    ldr r0, =echoDelay
    ldrh r0, [r0]
    add r9, r7, r0

    @ Set up current echo cursor location
    ldr r0, =echoCursor
    ldrh r0, [r0]
    add r7, r7, r0

@    str r13, tmpSp

    ldr r14, =firOffset
    ldrb r14, [r14]

    @ Offset firTable to start at FIR #7
    ldr r12, =DSP_MEM
    add r12, r12, #0x7F

    ldr r6, numSamples

echoMixLoopback:
    @ Load the old echo value (l,r)
    ldrsh r0, [r7]
    ldrsh r1, [r7, #2]

/*    @ Increment and wrap firOffset
    add r14, r14, #2
    and r14, r14, #(8 * 2) - 1

    @ Get &firTable[firOffset + 8] into r13
    ldr r13, =firTable + ((8 * 2) * 4)
    add r13, r13, r14, lsl #2

    @ Store the computed samples in the FIR ring buffer
    str r0, [r13]
    str r1, [r13, #4]
    str r0, [r13, #-8 * 2 * 4]
    str r1, [r13, #(-8 * 2 * 4) + 4]

    @ Process FIR sample 0 (special)
    ldr r2, [r13], #4
    ldr r3, [r13], #-12
    ldrsb r4, [r12], #-0x10
    mul r0, r2, r4
    mul r1, r3, r4

.MACRO processFir
    ldr r2, [r13], #4
    ldr r3, [r13], #-12
    ldrsb r4, [r12], #-0x10
    mla r0, r2, r4, r0
    mla r1, r3, r4, r1
.ENDM
    processFir
    processFir
    processFir
    processFir
    processFir
    processFir

    @ Last FIR sample (special)
    ldr r2, [r13], #4
    ldr r3, [r13], #-12
    ldrsb r4, [r12], #0x70

    mla r0, r2, r4, r0
    mla r1, r3, r4, r1
    
    @ Get rid of volume multiplication stuff
    mov r0, r0, asr #7
    mov r1, r1, asr #7*/

    @ r0,r1 contains the filtered samples
    ldr r2, [r8]
    @ Left channel = (feedback * filtered) >> 7
    mla r2, r11, r0, r2
    mov r2, r2, asr #15

    ldr r3, [r8, #4]
    @ Right channel = (feedback * filtered) >> 7
    mla r3, r11, r1, r3
    mov r3, r3, asr #15

    @ Store (filtered * echoFB) + echobuffer into echobuffer
    ldrb r5, echoEnabled
    tst r5, #0x20
    streqh r2, [r7], #2
    streqh r3, [r7], #2
    cmp r7, r9
    ldrge r7, echoBufferStart

    @ Store (filtered * echoVol) into echomix
    mul r2, r10, r0
    str r2, [r8], #4
    mul r2, r5, r1
    str r2, [r8], #4

    subs r6, r6, #1
    bne echoMixLoopback
    
doneEchoMix:

/*    ldr r13, tmpSp
    
    @ Store changed values
    ldr r0, =firOffset
    strb r14, [r0]*/

    ldr r3, echoBufferStart
    sub r7, r7, r3
    ldr r0, =echoCursor
    strh r7, [r0]

    @ Reload mix buffer & echo positions
    ldmfd sp!, {r1, r2}
    
#endif
    
clipAndMix:
    @ Put the original output buffer into r3
    ldmfd sp!, {r3}
    
    @ Set up the preamp & overall volume
    ldr r8, =dspPreamp
    ldrh r8, [r8]

    ldr r9, =DSP_MEM
    ldrsb r4, [r9, #0x0C] @ Main left volume
    ldrsb r6, [r9, #0x1C] @ Main right volume
    
    mul r4, r8, r4
    mov r4, r4, asr #7
    mul r6, r8, r6
    mov r6, r6, asr #7

    @ r0 - numSamples
    @ r1 - mix buffer
    @ r2 - echo buffer
    @ r3 - output buffer
    @ r4 - left volume
    @ r5 - TMP (assigned to sample value)
    @ r6 - right volume
    @ r7 - TMP
    @ r8 - preamp
    @ r9 - 
    @ r10 - 
    @ r11 - 
    @ r12 - 
    @ r14 - 

    @ Do volume multiplication, mix in echo buffer and clipping here
    ldr r0, numSamples

mixClipLoop:
    @ Load and scale by volume (LEFT)
    ldr r5, [r1], #4
    mov r5, r5, asr #15
    mul r5, r4, r5
    ldr r7, [r2], #4
    add r5, r5, r7, asr #7
    mov r5, r5, asr #7

    @ Clip and store
    cmp r5, #0x7f00
    movgt r5, #0x7f00
    cmn r5, #0x7f00
    movlt r5, #0x8100
    strh r5, [r3]
    add r3, r3, #4000 * 4

    @ Load and scale by volume (RIGHT)
    ldr r5, [r1], #4
    mov r5, r5, asr #15
    mul r5, r6, r5
    ldr r7, [r2], #4
    add r5, r5, r7, asr #7
    mov r5, r5, asr #7

    @ Clip and store
    cmp r5, #0x7f00
    movgt r5, #0x7f00
    cmn r5, #0x7f00
    movlt r5, #0x8100
    strh r5, [r3], #2
    sub r3, r3, #4000 * 4

    subs r0, r0, #1
    bne mixClipLoop

doneMix:
    ldmfd sp!, {r4-r12, lr}
    bx lr
.ENDFUNC

.GLOBAL channelNum

tmpSp:
.word 0
echoBufferStart:
.word 0
numSamples:
.word 0
channelNum:
.byte 0
echoEnabled:
.byte 0

.align
.pool
