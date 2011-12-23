#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>

extern int errno;

int memfd;

void *trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
    char *p;
    int aa;

    //printf ("mmap(%X, %X, %X, %X, %X, %X) ... ", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset);
    p = mmap (start, length, prot, flags, fd, offset);
    if (p == (char *)0xFFFFFFFF)
    {
        aa = errno;
        printf ("failed mmap(%X, %X, %X, %X, %X, %X) errno = %d\n", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset, aa);
    }
    else
    {
        //printf ("OK! (%X)\n", (unsigned int)p);
    }

    return p;
}

unsigned char initphys (void)
{
    memfd = open("/dev/mem", O_RDWR);
    if (memfd == -1)
    {
        printf ("Open failed\n");
        return 0;
    }

    printf ("/dev/mem opened successfully - fd = %d\n", memfd);

    return 1;
}

void closephys (void)
{
    close (memfd);
}

int myuname(char *buffer)
{
    asm volatile ("swi #0x90007a");
}

void DecodeCoarse (unsigned int indx, unsigned int sa)
{
    unsigned int cpt[256];
    unsigned int dom = (sa >> 5) & 15;
    unsigned int temp;
    unsigned int i = 0;
    unsigned int wb = 0;
    
    sa &= 0xfffffc00;
    indx *= 1048576;
    
    //printf ("  > %08X\n", sa);
    //printf ("%d\n",
    lseek (memfd, sa, SEEK_SET);
    memset (cpt, 0, 256*4);
    temp = read (memfd, cpt, 256*4);
    //printf ("%d\n", temp);
    if (temp != 256*4)
    {
        printf ("  # Bad read\n");
        return;
    }

    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
    
    for (i = 0; i < 256; i ++)
    {
        if (cpt[i])
        {
            switch (cpt[i] & 3)
            {
                case 0:
                    //printf ("  -- [%08X] Invalid --\n", cpt[i]);
                    break;
                case 1:
                    printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
                            cpt[i] & 0xFFFF0000, (cpt[i] & 0xFFFF0000) | 0xFFFF,
                            (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
                            (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
                    break;
                case 2:
                    printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
                            cpt[i] & 0xfffff000, (cpt[i] & 0xfffff000) | 0xFFF,
                            (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
                            (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
                    // This is where we look for any virtual addresses that map to physical address 0x03000000 and
                    // alter the cachable and bufferable bits...
                    /*if (((cpt[i] & 0xffff0000) == 0x03000000) && ((cpt[i] & 12)==0))
                    {
                        //printf("c and b bits not set, overwriting\n");
                        cpt[i] |= 0xFFC;
                        wb = 1;
                    }*/
					if (((cpt[i] & 0xff000000) == 0x02000000) )
					{
						//printf("SOUND c and b bits not set, overwriting\n");
						if((cpt[i] & 12)==0) {
							cpt[i] |= 0xFFC;
							wb++;
						}
					}
					//if ((a>=0x31 && a<=0x36) && ((cpt[i] & 12)==0))
					if (((cpt[i] & 0xff000000) == 0x03000000) )
					{
						//printf("SDL   c and b bits not set, overwriting\n");
						if((cpt[i] & 12)==0) {
							cpt[i] |= 0xFFC;
							wb++;
						}
					}
                    break;
                case 3:
                    //printf ("  -- [%08X/%d] Unsupported --\n", cpt[i],cpt[i] & 3);
                    break;
                default:
                    //printf ("  -- [%08X/%d] Unknown --\n", cpt[i], cpt[i] & 3);
                    break;
            }
        }
        indx += 4096;
    }
    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
    if (wb)
    {
        //printf("Hacking cpt\n");
        lseek (memfd, sa, SEEK_SET);
        temp = write (memfd, cpt, 256*4);
        printf("%d bytes written, %s\n", temp, temp == 1024 ? "yay!" : "oh fooble :(!");
    }
}

void dumppgtable (unsigned int ttb)
{
    unsigned int pgtable[4096];
    char *desctypes[] = {"Invalid", "Coarse", "Section", "Fine"};

    memset (pgtable, 0, 4096*4);
    lseek (memfd, ttb, SEEK_SET);
    read (memfd, pgtable, 4096*4);

    int i;
    for (i = 0; i < 4096; i ++)
    {
        int temp;
        
        if (pgtable[i])
        {
            printf ("Indx: %d VA: %08X Type: %d [%s] \n", i, i * 1048576, pgtable[i] & 3, desctypes[pgtable[i]&3]);
            switch (pgtable[i]&3)
            {
                case 0:
                    //printf (" -- Invalid --\n");
                    break;
                case 1:
                    DecodeCoarse(i, pgtable[i]);
                    break;
                case 2:
                    temp = pgtable[i] & 0xFFF00000;
                    //printf ("  PA: %08X - %08X A: %d D: %d C: %d B: %d\n", temp, temp | 0xFFFFF,
                    //        (pgtable[i] >> 10) & 3, (pgtable[i] >> 5) & 15, (pgtable[i] >> 3) & 1,
                    //        (pgtable[i] >> 2) & 1);
                            
                    break;
                case 3:
                    printf ("  -- Unsupported! --\n");
                    break;
            }
        }
    }
}

void benchmark (void *memptr)
{
    int starttime = time (NULL);
    int a,b,c,d;
    volatile unsigned int *pp = (unsigned int *) memptr;

    while (starttime == time (NULL));

    printf ("\n\nmemory benchmark of volatile VA: %08X\n\nread test\n", memptr);
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                b += pp[a];
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("write test\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] = 0x37014206;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf  ("combined test (read, write back)\n");
    for (d = 0; d < 3; d ++)
    {
        starttime = time (NULL);
        b = 0;
        c = 0;
        while (starttime == time (NULL))
        {
            for (a = 0; a < 20000; a ++)
            {
                pp[a] += 0x55017601;
            }
            c ++;
        }
        printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
    }

    printf ("test complete\n");
}

void hackpgtable (void)
{
    unsigned int oldc1, oldc2, oldc3, oldc4;
    unsigned int newc1 = 0xee120f10, newc2 = 0xe12fff1e;
    unsigned int ttb, ttx;
    char name[256];

    // We need to execute a "MRC p15, 0, r0, c2, c0, 0", to get the pointer to the translation table base, but we can't
    // do this in user mode, so we have to patch the kernel to get it to run it for us in supervisor mode. We do this
    // at the moment by overwriting the sys_newuname function and then calling it.

    lseek (memfd, 0x6ec00, SEEK_SET); // fixme: We should ask the kernel for this address rather than assuming it...
    read (memfd, &oldc1, 4);
    read (memfd, &oldc2, 4);
    read (memfd, &oldc3, 4);
    read (memfd, &oldc4, 4);

    printf ("0:%08X %08X\n", oldc1, oldc2);

    lseek (memfd, 0x6ec00, SEEK_SET);
    write (memfd, &newc1, 4);
    write (memfd, &newc2, 4);    
    
    ttb = myuname(name);
    
    lseek (memfd, 0x6ec00, SEEK_SET);
    write (memfd, &oldc1, 4);
    write (memfd, &oldc2, 4);    

    printf ("1:%08X\n", ttb);

    //printf ("Restored contents\n");
    
    // We now have the translation table base ! Walk the table looking for our allocation and hack it :)
    dumppgtable(ttb);    

    // Now drain the write buffer and flush the tlb caches. Something else we can't do in user mode...
    unsigned int tlbc1 = 0xe3a00000; // mov    r0, #0
    unsigned int tlbc2 = 0xee070f9a; // mcr    15, 0, r0, cr7, cr10, 4
    unsigned int tlbc3 = 0xee080f17; // mcr    15, 0, r0, cr8, cr7, 0
    unsigned int tlbc4 = 0xe1a0f00e; // mov    pc, lr

    lseek (memfd, 0x6ec00, SEEK_SET);
    write (memfd, &tlbc1, 4);
    write (memfd, &tlbc2, 4);    
    write (memfd, &tlbc3, 4);    
    write (memfd, &tlbc4, 4);    
    
    ttx = myuname(name);
    
    //printf ("Return from uname: %08X\n", ttx);
    
    lseek (memfd, 0x6ec00, SEEK_SET);
    write (memfd, &oldc1, 4);
    write (memfd, &oldc2, 4);    
    write (memfd, &oldc3, 4);    
    write (memfd, &oldc4, 4);    

    //printf ("Restored contents\n");

    //printf ("Pagetable after modification!\n");
    //printf ("-------------------------------\n");
    //dumppgtable(ttb);
}

/*int
main( int argc, char* argv[] )
{    
    if (!initphys())
        return 0;

    volatile unsigned int *myBuf = trymmap((void *)0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x03000000);
    volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

    //memset ((void *)myBuf, 0x55, 65536);
    //memset ((void *)secbuf, 0x55, 65536);

    printf("mmaped 0x03000000 buffer @ VA: %08X malloc'd buffer @ VA: %08X\n", myBuf, secbuf);

    hackpgtable();

    //benchmark ((void*)myBuf);
    //benchmark ((void*)secbuf);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    printf ("\n\nCleaning up...\n");    
    printf ("Closing files...\n");
    close (memfd);
    printf ("Exiting...\n");
    
    return 0;        
} */