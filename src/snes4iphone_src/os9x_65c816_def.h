//#define __PALMOS__

//#define __TESTING__

#define MAP_LAST	12

#define regA		R11	//format : 0xhhll0000 or 0xll000000
#define rstatus		R4      //format : 0xff800000
#define regDBank	R4	//format : 0x000000ll
#define regX		R5	//format : 0xhhll0000 or 0xll000000
#define regY		R6	//format : 0xhhll0000 or 0xll000000

#define rpc		R7	//32bits address
#define regD		R8	//format : 0xhhll0000
#define regPBank	R8	//format : 0x000000ll
#define regCycles	R9	//32bits counter
#define regS		R10	//format : 0x0000hhll

#define rscratch	R0	//format : 0xhhll0000 if data and calculation or return of S9XREADBYTE	or WORD
#define regopcode	R0	//format : 0x000000ll
#define rscratch2	R1	//format : 0xhhll for calculation and value
#define rscratch3	R2	//
#define rscratch4	R3	//??????


#define rscratch5	R5	//??????
#define rscratch6	R6	//??????
#define rscratch7	R8	//??????
#define rscratch8	R9	//??????
#define rscratch9	R10	//??????

#define regpcbase	R12	//32bits address

#define regCPUvar       R14



//not used
//R13	//Pointer 32 bit on a struct.

//R15 = pc (sic!)


/*#define Carry       1
#define Zero        2
#define IRQ         4
#define Decimal     8
#define IndexFlag  16
#define MemoryFlag 32
#define Overflow   64
#define Negative  128
#define Emulation 256*/

#define STATUS_SHIFTER		24
#define MASK_EMUL		(1<<(STATUS_SHIFTER-1))
#define MASK_SHIFTER_CARRY	(STATUS_SHIFTER+1)
#define	MASK_CARRY		(1<<(STATUS_SHIFTER))  //0
#define	MASK_ZERO		(2<<(STATUS_SHIFTER))  //1
#define MASK_IRQ		(4<<(STATUS_SHIFTER))  //2
#define MASK_DECIMAL		(8<<(STATUS_SHIFTER))  //3
#define	MASK_INDEX		(16<<(STATUS_SHIFTER)) //4  //1
#define	MASK_MEM		(32<<(STATUS_SHIFTER)) //5  //2
#define	MASK_OVERFLOW		(64<<(STATUS_SHIFTER)) //6  //4
#define	MASK_NEG		(128<<(STATUS_SHIFTER))//7  //8

#define ONE_CYCLE 6
#define SLOW_ONE_CYCLE 8

#define	NMI_FLAG	    (1 << 7)
#define IRQ_PENDING_FLAG    (1 << 11)
#define SCAN_KEYS_FLAG	    (1 << 4)


#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_SHIFT 12
#define MEMMAP_MASK (0xFFF)
