#ifndef __os9x_asm_cpu__
#define __os9x_asm_cpu__

extern "C" void test_opcode(SCPUState *cpuptr);

extern "C" void asmMainLoop(SCPUState *cpuptr);

extern "C" void asm_S9xMainLoop(void);

#endif
