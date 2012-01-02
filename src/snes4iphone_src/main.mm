
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#ifdef __GIZ__
#define TIMER_1_SECOND	1000
#include <sys/wcetypes.h>
#include <KGSDK.h>
#include <Framework.h>
#include <Framework2D.h>
#include <FrameworkAudio.h>
#include "giz_kgsdk.h"
#endif

#ifdef __GP2X__
#define TIMER_1_SECOND	1000000
#include "gp2x_sdk.h"
#include "squidgehack.h"
#endif

#ifdef __IPHONE__
#include <pthread.h>
#include <sys/time.h>
#define TIMER_1_SECOND	1000
#include "iphone_sdk.h"

extern int isMultiTouching;
extern int __transparency;
extern int __speedhack;
extern int __autosave;
extern unsigned long __fps_debug;
extern int iphone_soundon;
extern volatile int __emulation_paused;
extern "C" void app_DemuteSound(int buffersize);
extern "C" void app_MuteSound(void);

extern "C" void saveScreenshotToFile(char *filepath);
#endif

#include "menu.h"
#include "snes9x.h"
#include "memmap.h"
#include "apu.h"
#include "gfx.h"
#include "soundux.h"
#include "snapshot.h"
#include "fxinst.h"
#include "fxemu.h"

#define EMUVERSION "SquidgeSNES V0.37 01-Jun-06"

//---------------------------------------------------------------------------

extern struct FxInit_s SuperFX;
extern struct FxRegs_s GSU;

#ifdef __GP2X__
extern "C" char joy_Count();
extern "C" int InputClose();
extern "C" int joy_getButton(int joyNumber);
#endif

unsigned char gammatab[10][32]={
	{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F},
	{0x00,0x01,0x02,0x03,0x04,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
	0x11,0x12,0x13,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F},
	{0x00,0x01,0x03,0x04,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,
	0x12,0x13,0x14,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F},
	{0x00,0x02,0x04,0x06,0x07,0x08,0x09,0x0A,0x0C,0x0D,0x0E,0x0F,0x0F,0x10,0x11,0x12,
	0x13,0x14,0x15,0x16,0x16,0x17,0x18,0x19,0x19,0x1A,0x1B,0x1C,0x1C,0x1D,0x1E,0x1F},
	{0x00,0x03,0x05,0x07,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,
	0x14,0x15,0x16,0x17,0x17,0x18,0x19,0x19,0x1A,0x1B,0x1B,0x1C,0x1D,0x1D,0x1E,0x1F},
	{0x00,0x05,0x07,0x09,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x14,0x15,
	0x16,0x16,0x17,0x18,0x18,0x19,0x1A,0x1A,0x1B,0x1B,0x1C,0x1C,0x1D,0x1D,0x1E,0x1F},
	{0x00,0x07,0x0A,0x0C,0x0D,0x0E,0x10,0x11,0x12,0x12,0x13,0x14,0x15,0x15,0x16,0x17,
	0x17,0x18,0x18,0x19,0x1A,0x1A,0x1B,0x1B,0x1B,0x1C,0x1C,0x1D,0x1D,0x1E,0x1E,0x1F},
	{0x00,0x0B,0x0D,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,0x17,0x17,0x18,0x18,
	0x19,0x19,0x1A,0x1A,0x1B,0x1B,0x1B,0x1C,0x1C,0x1D,0x1D,0x1D,0x1E,0x1E,0x1E,0x1F},
	{0x00,0x0F,0x11,0x13,0x14,0x15,0x16,0x17,0x17,0x18,0x18,0x19,0x19,0x1A,0x1A,0x1A,
	0x1B,0x1B,0x1B,0x1C,0x1C,0x1C,0x1C,0x1D,0x1D,0x1D,0x1D,0x1E,0x1E,0x1E,0x1E,0x1F},
	{0x00,0x15,0x17,0x18,0x19,0x19,0x1A,0x1A,0x1B,0x1B,0x1B,0x1B,0x1C,0x1C,0x1C,0x1C,
	0x1D,0x1D,0x1D,0x1D,0x1D,0x1D,0x1D,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1F}
};

int32 gp32_fastmode = 1;
int gp32_8bitmode = 0;
int32 gp32_ShowSub = 0;
int gp32_fastsprite = 1;
int gp32_gammavalue = 0;
int squidgetranshack = 0;
uint8 *vrambuffer = NULL;
int globexit = 0;
int sndvolL, sndvolR;
char fps_display[256];
int samplecount=0;
int enterMenu = 0;
void *currentFrameBuffer;
int16 oldHeight = 0;

int compute_hex( char *x )
{
    int v = 0;
    while( *x )
    {
        v = v * 16;
        if( *x>='0' && *x<='9' )
            v = v + (*x - '0');
        if( *x>='A' && *x<='F' )
            v = v + (*x - 'A' + 10);
        if( *x>='a' && *x<='f' )
            v = v + (*x - 'a' + 10);
        x++;
    }
    return v;
}

int compute_hex8( char *x )
{
    char h[10];
    h[0] = x[0];
    h[1] = x[1];
    h[2] = 0;
    return compute_hex( h );
}




const uint32 crc32Table[256] = {
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

//CRC32 for char arrays
uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32) {	
	for (register uint32 i = 0; i < size; i++) {
		crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];
	}
	return ~crc32;
}

void patch(int crc, unsigned char* buffer)
{
    char datFileName[1024];
    sprintf( datFileName, "/Applications/SNES-HD.app/snesadvance.dat");
	
    char string[4096];
    char hex[100];
    FILE *fp = fopen( datFileName, "rb" );
    if( fp==NULL )
    {
        fprintf(stderr, "No patch superdat found. The game may run slowly or not run at all.\n" );
        return;
    }
	
    sprintf( hex, "%08X", crc );
	
    while( !feof( fp ) )
    {
        memset( string, 0, 4096 );
        fgets( string, 4095, fp );
        if( string[strlen(string)-1] == 13 || string[strlen(string)-1] == 10 )
            string[strlen(string)-1] = 0;
        if( string[strlen(string)-1] == 13 || string[strlen(string)-1] == 10 )
            string[strlen(string)-1] = 0;
		
        char *ps = 0;
        char *s = strtok( string, "|" );
        
		
        if( s )
        {
            // this should be the CRC
            //
            if( strcmp( s, hex )!=0 )
            {
                continue;
            }
        }
        else
            continue;
		
        // CRC matches, so grab the patch
        //
        int c = 0;
        while( s )
        {
            ps = s;
			
            if( c==1 )
                fprintf(stderr, "Game        : %s\n", s );
            s = strtok( NULL, "|" );
            c++;
        }
		
        /*printf( "patch = %s\n", ps );*/
		
        // ps contains the patch
        //
        fprintf(stderr, "Patch       :\n" );
        s = strtok( ps, "," );
        while( s )
        {
            ps = s + (strlen(s)+1);
            char *addr = strtok( s, "=" );
            char *val = strtok( NULL, "=" );
			
            int iaddr = compute_hex(addr);
            while( *val && *(val+1) )
            {
                int c = compute_hex8( val );
                buffer[iaddr] = c;
                fprintf(stderr, "%08x = %02x\n", iaddr, c );
                iaddr += 1;
                val += 2;
            }
			
            s = strtok( ps, "," );
        }
        return;
    }
    fprintf(stderr, "No patch found. The game may run slowly or not run at all.\n" );
	
}

int os9x_findhacks(int game_crc32){
	int i=0,j;
	int _crc32;	
	char c;
	char str[256];
	unsigned int size_snesadvance;
	unsigned char *snesadvance;
	FILE *f;
	
	if(!__speedhack)
	{
		return 0;
	}
#ifdef __IPHONE__
	sprintf(str,"/Applications/SNES-HD.app/snesadvance.dat");
#else
	sprintf(str,"%s/snesadvance.dat",currentWorkingDir);
#endif
	f=fopen(str,"rb");
	if (!f) return 0;
	fseek(f,0,SEEK_END);
	size_snesadvance=ftell(f);
	fseek(f,0,SEEK_SET);
	snesadvance=(unsigned char*)malloc(size_snesadvance);
	fread(snesadvance,1,size_snesadvance,f);
	fclose(f);
	
	for (;;) {
		//get crc32
		j=i;
		while ((i<size_snesadvance)&&(snesadvance[i]!='|')) i++;
		if (i==size_snesadvance) {free(snesadvance);return 0;}
		//we have (snesadvance[i]=='|')
		//convert crc32 to int
		_crc32=0;
		while (j<i) {
			c=snesadvance[j];
			if ((c>='0')&&(c<='9'))	_crc32=(_crc32<<4)|(c-'0');
			else if ((c>='A')&&(c<='F'))	_crc32=(_crc32<<4)|(c-'A'+10);
			else if ((c>='a')&&(c<='f'))	_crc32=(_crc32<<4)|(c-'a'+10);				
			j++;
		}
		if (game_crc32==_crc32) {
			//int p=0;
			for (;;) {
				int adr,val;
				
				i++;
				j=i;
				while ((i<size_snesadvance)&&(snesadvance[i]!=0x0D)&&(snesadvance[i]!=',')) {
					if (snesadvance[i]=='|') j=i+1;
					i++;
				}
				if (i==size_snesadvance) {free(snesadvance);return 0;}
				memcpy(str,&snesadvance[j],i-j);
				str[i-j]=0;								
				sscanf(str,"%X=%X",&adr,&val);
				//sprintf(str,"read : %X=%X",adr,val);
				//pgPrintAllBG(32,31-p++,0xFFFF,str);
				
				//if ((val==0x42)||((val&0xFF00)==0x4200)) {					
					if (val&0xFF00) {
						ROM[adr]=(val>>8)&0xFF;
						ROM[adr+1]=val&0xFF;
					} else ROM[adr]=val;
				//}
				
				if (snesadvance[i]==0x0D) {free(snesadvance);return 1;				}
			}
			
		}
		while ((i<size_snesadvance)&&(snesadvance[i]!=0x0A)) i++;
		if (i==size_snesadvance) {free(snesadvance);return 0;}
		i++; //new line
	}
}

void S9xExit ()
{
}
void S9xGenerateSound (void)
   {
      S9xMessage (0,0,"generate sound");
	   return;
   }
   
extern "C"
{
	void S9xSetPalette ()
	{

	}

	void S9xExtraUsage ()
	{
	}
	
	void S9xParseArg (char **argv, int &index, int argc)
	{	
	}

	bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file)
	{
		if (read_only)
		{
			if (*file = OPEN_STREAM(fname,"rb")) 
				return(TRUE);
		}
		else
		{
			if (*file = OPEN_STREAM(fname,"w+b")) 
				return(TRUE);
		}

		return (FALSE);	
	}
	
	void S9xCloseSnapshotFile (STREAM file)
	{
		CLOSE_STREAM(file);
	}

   void S9xMessage (int /* type */, int /* number */, const char *message)
   {
		printf ("%s\n", message);
   }

   void erk (void)
   {
      S9xMessage (0,0, "Erk!");
   }

   char *osd_GetPackDir(void)
   {
      S9xMessage (0,0,"get pack dir");
      return ".";
   }

   const char *S9xGetSnapshotDirectory(void)
   {
      S9xMessage (0,0,"get snapshot dir");
      return "";
   }

   void S9xLoadSDD1Data (void)
   {
      S9xMessage (0,0,"load sdd1data");
      Memory.FreeSDD1Data();
   }

   

   bool8_32 S9xInitUpdate ()
   {
	  //GFX.Screen = (uint8 *) framebuffer16 + (640*8) + 64;

	  return (TRUE);
   }

   bool8_32 S9xDeinitUpdate (int Width, int Height, bool8_32)
   {
		unsigned int *pix;
		int i=0;
      if (snesMenuOptions.showFps) 
	  {
			gp_drawString(0,0,strlen(fps_display),fps_display,0xF800,(unsigned char*)vrambuffer);
	  }
	  
    // TODO clear Z buffer if not in fastsprite mode
		gp_setFramebuffer(currFB,0);
	   
	   return (TRUE);
   }

   const char *S9xGetFilename (const char *ex)
   {
      static char filename [PATH_MAX + 1];
      char drive [_MAX_DRIVE + 1];
      char dir [_MAX_DIR + 1];
      char fname [_MAX_FNAME + 1];
      char ext [_MAX_EXT + 1];

      _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
      strcpy (filename, S9xGetSnapshotDirectory ());
      strcat (filename, SLASH_STR);
      strcat (filename, fname);
      strcat (filename, ex);
      return (filename);
   }

#ifdef __GIZ__
   uint32 S9xReadJoypad (int which1)
   {
	   uint32 val=0x80000000;

	   if (which1 != 0) return val;
		unsigned long joy = gp_getButton(0);
		      
		if (snesMenuOptions.actionButtons)
		{
			if (joy & (1<<INP_BUTTON_REWIND)) val |= SNES_Y_MASK;
			if (joy & (1<<INP_BUTTON_FORWARD)) val |= SNES_A_MASK;
			if (joy & (1<<INP_BUTTON_PLAY)) val |= SNES_B_MASK;
			if (joy & (1<<INP_BUTTON_STOP)) val |= SNES_X_MASK;
		}
		else
		{
			if (joy & (1<<INP_BUTTON_REWIND)) val |= SNES_A_MASK;
			if (joy & (1<<INP_BUTTON_FORWARD)) val |= SNES_B_MASK;
			if (joy & (1<<INP_BUTTON_PLAY)) val |= SNES_X_MASK;
			if (joy & (1<<INP_BUTTON_STOP)) val |= SNES_Y_MASK;
		}
			
		if (joy & (1<<INP_BUTTON_UP)) val |= SNES_UP_MASK;
		if (joy & (1<<INP_BUTTON_DOWN)) val |= SNES_DOWN_MASK;
		if (joy & (1<<INP_BUTTON_LEFT)) val |= SNES_LEFT_MASK;
		if (joy & (1<<INP_BUTTON_RIGHT)) val |= SNES_RIGHT_MASK;
		if (joy & (1<<INP_BUTTON_HOME)) val |= SNES_START_MASK;
		if (joy & (1<<INP_BUTTON_L)) val |= SNES_TL_MASK;
		if (joy & (1<<INP_BUTTON_R)) val |= SNES_TR_MASK;
		
		if (joy & (1<<INP_BUTTON_BRIGHT))	enterMenu = 1;
      return val;
   }
#endif

#ifdef __IPHONE__
   uint32 S9xReadJoypad (int which1)
   {
	   uint32 val=0x80000000;

	   if (which1 >= 4) return val;
       unsigned long joy = gp_getButton(which1);
		      
		if (joy & (1<<INP_BUTTON_HARDLEFT)) val |= SNES_Y_MASK;
		if (joy & (1<<INP_BUTTON_HARDRIGHT)) val |= SNES_A_MASK;
		if (joy & (1<<INP_BUTTON_HARDDOWN)) val |= SNES_B_MASK;
		if (joy & (1<<INP_BUTTON_HARDUP)) val |= SNES_X_MASK;
			
		if (joy & (1<<INP_BUTTON_UP)) val |= SNES_UP_MASK;
		if (joy & (1<<INP_BUTTON_DOWN)) val |= SNES_DOWN_MASK;
		if (joy & (1<<INP_BUTTON_LEFT)) val |= SNES_LEFT_MASK;
		if (joy & (1<<INP_BUTTON_RIGHT)) val |= SNES_RIGHT_MASK;
		if (joy & (1<<INP_BUTTON_START)) val |= SNES_START_MASK;
		if (joy & (1<<INP_BUTTON_L)) val |= SNES_TL_MASK;
		if (joy & (1<<INP_BUTTON_R)) val |= SNES_TR_MASK;
		
		if (joy & (1<<INP_BUTTON_SELECT)) val |= SNES_SELECT_MASK;
		
		//if (joy & (1<<INP_BUTTON_STICK_PUSH))	enterMenu = 1;
		if (joy & (1<<INP_BUTTON_R2)) 
		{
			snesMenuOptions.volume+=1;
			if(snesMenuOptions.volume>100) snesMenuOptions.volume=100;
			gp2x_sound_volume(snesMenuOptions.volume,snesMenuOptions.volume);
		}
		else if (joy & (1<<INP_BUTTON_L2))	
		{
			snesMenuOptions.volume-=1;
			if(snesMenuOptions.volume>100) snesMenuOptions.volume=0;
			gp2x_sound_volume(snesMenuOptions.volume,snesMenuOptions.volume);
		}
		
      return val;
   }
#endif


   bool8 S9xReadMousePosition (int /* which1 */, int &/* x */, int & /* y */,
			    uint32 & /* buttons */)
   {
      S9xMessage (0,0,"read mouse");
      return (FALSE);
   }

   bool8 S9xReadSuperScopePosition (int & /* x */, int & /* y */,
				 uint32 & /* buttons */)
   {
      S9xMessage (0,0,"read scope");
      return (FALSE);
   }

   const char *S9xGetFilenameInc (const char *e)
   {
      S9xMessage (0,0,"get filename inc");
      return e;
   }

   void S9xSyncSpeed(void)
   {
      //S9xMessage (0,0,"sync speed");
   }

   const char *S9xBasename (const char *f)
   {
      const char *p;

      S9xMessage (0,0,"s9x base name");

      if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
         return (p + 1);

      return (f);
   }

};

bool8_32 S9xOpenSoundDevice (int mode, bool8_32 stereo, int buffer_size)
	{
		so.sound_switch = 255;
		so.playback_rate = mode;
		so.stereo = FALSE;//stereo;
		return TRUE;
	}

	
void S9xAutoSaveSRAM (void)
{
	//since I can't sync the data, there is no point in even writing the data
	//out at this point.  Instead I'm now saving the data as the users enter the menu.
  S9xSaveSRAM();
	//sync();  can't sync when emulator is running as it causes delays
}

void S9xLoadSRAM (void)
{
	char path[MAX_PATH];
	
	sprintf(path,"%s%s%s",snesSramDir,DIR_SEPERATOR,S9xGetFilename (".srm"));
	Memory.LoadSRAM (path);
}

void S9xSaveSRAM (void)
{
	char path[MAX_PATH];
#if 0	
	if (CPU.SRAMModified)
#endif
	{
		sprintf(path,"%s%s%s",snesSramDir,DIR_SEPERATOR,S9xGetFilename (".srm"));
		Memory.SaveSRAM (path);
		sync();
	}
}

bool JustifierOffscreen(void)
{
   return false;
}

void JustifierButtons(uint32& justifiers)
{
}

static int SnesRomLoad()
{
	char filename[MAX_PATH+1];
	int check;
	char text[256];
	FILE *stream=NULL;
  
    gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	sprintf(text,"Loading Rom...");
	gp_drawString(0,0,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	MenuFlip();
	S9xReset();
	//Save current rom shortname for save state etc
	strcpy(currentRomFilename,romList[currentRomIndex].filename);
	
	// get full filename
	sprintf(filename,"%s%s%s",romDir,DIR_SEPERATOR,currentRomFilename);
	
	if (!Memory.LoadROM (filename))
	{
		sprintf(text,"Loading Rom...Failed");
		gp_drawString(0,0,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
		MenuFlip();
		MenuPause();
		return 0;
	}
	
	sprintf(text,"Loading Rom...OK!");
	gp_drawString(0,0,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	sprintf(text,"Loading Sram");
	gp_drawString(0,8,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	MenuFlip();
	
	//Memory.LoadSRAM (S9xGetFilename (".srm")); 
	S9xLoadSRAM();

	//auto load default config for this rom if one exists
	if (LoadMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1))
	{
		//failed to load options for game, so load the default global ones instead
		if (LoadMenuOptions(snesOptionsDir, MENU_OPTIONS_FILENAME, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1))
		{
			//failed to load global options, so use default values
			SnesDefaultMenuOptions();
		}
	}
	
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	
	return(1);
}

static int SegAim()
{
#ifdef __GIZ__
  int aim=FrameworkAudio_GetCurrentBank(); 
#endif
#ifdef __GP2X__
  int aim=CurrentSoundBank; 
#endif
#ifdef __IPHONE__
  // TODO
  int aim=0; 
#endif  

  aim--; if (aim<0) aim+=8;

  return aim;
}
	


void _makepath (char *path, const char *, const char *dir,
	const char *fname, const char *ext)
{
	if (dir && *dir)
	{
		strcpy (path, dir);
		strcat (path, "/");
	}
	else
	*path = 0;
	strcat (path, fname);
	if (ext && *ext)
	{
		strcat (path, ".");
		strcat (path, ext);
	}
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
	char *ext)
{
	*drive = 0;

	char *slash = strrchr (path, '/');
	if (!slash)
		slash = strrchr (path, '\\');

	char *dot = strrchr (path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash)
	{
		strcpy (dir, "");
		strcpy (fname, path);
		if (dot)
		{
			*(fname + (dot - path)) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
	else
	{
		strcpy (dir, path);
		*(dir + (slash - path)) = 0;
		strcpy (fname, slash + 1);
		if (dot)
		{
			*(fname + (dot - slash) - 1) = 0;
			strcpy (ext, dot + 1);
		}
		else
			strcpy (ext, "");
	}
} 

// save state file I/O
int  (*statef_open)(const char *fname, const char *mode);
int  (*statef_read)(void *p, int l);
int  (*statef_write)(void *p, int l);
void (*statef_close)();
static FILE  *state_file = 0;

int state_unc_open(const char *fname, const char *mode)
{
	state_file = fopen(fname, mode);
	return (int) state_file;
}

int state_unc_read(void *p, int l)
{
	return fread(p, 1, l, state_file);
}

int state_unc_write(void *p, int l)
{
	return fwrite(p, 1, l, state_file);
}

void state_unc_close()
{
	fclose(state_file);
}

void delay_us(unsigned long long us_count)
{
	usleep(us_count);
}

void get_ticks_us(unsigned long long *ticks_return)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	
	*ticks_return =
	(unsigned long long)current_time.tv_sec * 1000000LL + current_time.tv_usec;
}

char **g_argv;
#ifdef __IPHONE__
extern "C" int iphone_main(char* filename)
#else
int main(int argc, char *argv[])
#endif
{
	unsigned long frameskip_counter = 0;
	unsigned long current_frameskip_value = 0;
	unsigned long fps = 60;
	unsigned long long last_screen_timestamp = 0;
	unsigned long long last_frame_interval_timestamp = 0;
	unsigned long long last_frame_value_timestamp = 0;
	unsigned long interval_skipped_frames = 0;
	unsigned long framecount = 0;
	unsigned long frames_counted;
	unsigned long skipped_frames = 0;
	int __saved = 0;
	char save_filename[1024];
 	unsigned int i = 0;
	unsigned int romrunning = 0;
	int aim=0, done=0, skip=0, Frames=0, tick=0, efps=0, SaveFrames=0;
	unsigned long Timer=0;
	int action=0;
	int romloaded=0;
	char text[256];
	
#ifndef __IPHONE__
	g_argv = argv;
#endif

	// saves
	statef_open  = state_unc_open;
	statef_read  = state_unc_read;
	statef_write = state_unc_write;
	statef_close = state_unc_close;
	
	
#if defined (__GP2X__)
	//getwd(currentWorkingDir); naughty do not use!
	getcwd(currentWorkingDir, MAX_PATH);
#else
	sprintf(currentWorkingDir, "%s", SYSTEM_DIR);
#endif
	sprintf(snesOptionsDir,"%s%s%s",currentWorkingDir,DIR_SEPERATOR,SNES_OPTIONS_DIR);
	sprintf(snesSramDir,"%s%s%s",currentWorkingDir,DIR_SEPERATOR,SNES_SRAM_DIR);
	sprintf(snesSaveStateDir,"%s%s%s",currentWorkingDir,DIR_SEPERATOR,SNES_SAVESTATE_DIR);
    sprintf(snesRomDir, "%s", currentWorkingDir);
    
    printf("rom directory: %s\n", snesRomDir);
    printf("sram directory: %s\n", snesSramDir);
    printf("save directory: %s\n", snesSaveStateDir);
	
	InputInit();  // clear input context

	//ensure dirs exist
	//should really check if these worked but hey whatever
    mode_t dir_mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	mkdir(snesOptionsDir,dir_mode);
	mkdir(snesSramDir,dir_mode);
	mkdir(snesSaveStateDir,dir_mode);
#if 0
	printf("Loading global menu options\r\n"); fflush(stdout);
	if (LoadMenuOptions(snesOptionsDir,MENU_OPTIONS_FILENAME,MENU_OPTIONS_EXT,(char*)&snesMenuOptions, sizeof(snesMenuOptions),0))
	{
		// Failed to load menu options so default options
		printf("Failed to load global options, so using defaults\r\n"); fflush(stdout);
		SnesDefaultMenuOptions();
	}
	
	printf("Loading default rom directory\r\n"); fflush(stdout);
	if (LoadMenuOptions(snesOptionsDir,DEFAULT_ROM_DIR_FILENAME,DEFAULT_ROM_DIR_EXT,(char*)snesRomDir, MAX_PATH,0))
	{
		// Failed to load options to default rom directory to current working directory
		printf("Failed to default rom dir, so using current dir\r\n"); fflush(stdout);
		strcpy(snesRomDir,currentWorkingDir);
	}
#endif
	// Init graphics (must be done before MMUHACK)
	gp_initGraphics(16,0,snesMenuOptions.mmuHack);
	printf("Init'd graphics.\r\n"); fflush(stdout);
#if defined(__GP2X__)
	if (snesMenuOptions.ramSettings)
	{
		printf("Craigs RAM settings are enabled.  Now applying settings..."); fflush(stdout);
		// craigix: --trc 6 --tras 4 --twr 1 --tmrd 1 --trfc 1 --trp 2 --trcd 2
		set_RAM_Timings(6, 4, 1, 1, 1, 2, 2);
		printf("Done\r\n"); fflush(stdout);
	}
	else
	{
		printf("Using normal Ram settings.\r\n"); fflush(stdout);
	}

	set_gamma(snesMenuOptions.gamma+100);
#endif

	UpdateMenuGraphicsGamma();
	
    printf("initializing...\n"); fflush(stdout);
	// Initialise Snes stuff
	////////
	///////
	memset( &IAPU, 0, sizeof( SIAPU ) );
	memset( &CPU, 0, sizeof( SCPUState ) );
  memset( &GFX, 0, sizeof( SGFX ) );
	memset( &SNESGameFixes, 0, sizeof( SSNESGameFixes ) );
	memset( &SuperFX, 0, sizeof( FxInit_s ) );
  memset( &ICPU, 0, sizeof( SICPU ) );
  memset( &DSP1, 0, sizeof( SDSP1 ) );
  memset( &GSU, 0, sizeof( FxRegs_s ) );
  memset( &BG, 0, sizeof( SBG ) );
  memset( &PPU, 0, sizeof( SPPU ) );
  memset( &DMA[0], 0, sizeof( SDMA ) * 8);
  memset( &IPPU, 0, sizeof( InternalPPU ) );
  memset( &SA1Registers, 0, sizeof( SSA1Registers ) );
  memset( &SA1, 0, sizeof( SSA1 ) );  
	memset( &Settings, 0, sizeof( SSettings ) );

	// ROM Options
	Settings.SDD1Pack = true;
	Settings.ForceLoROM = false;
	Settings.ForceInterleaved = false;
	Settings.ForceNotInterleaved = false;
	Settings.ForceInterleaved = false;
	Settings.ForceInterleaved2 = false;
	Settings.ForcePAL = false;
	Settings.ForceNTSC = false;
	Settings.ForceHeader = false;
	Settings.ForceNoHeader = false;   
	// Sound options            
	Settings.SoundSync = 0;
	Settings.InterpolatedSound = true;
	Settings.SoundEnvelopeHeightReading = true;
	Settings.DisableSoundEcho = false;
	Settings.DisableMasterVolume = false;
	Settings.Mute = FALSE;
	Settings.SoundSkipMethod = 0;
	Settings.SoundPlaybackRate = 22050;
	Settings.SixteenBitSound = true;
	Settings.Stereo = false;
	Settings.AltSampleDecode = 0;//os9x_sampledecoder;
	Settings.ReverseStereo = FALSE;
	Settings.SoundBufferSize = 0;//4;
	Settings.SoundMixInterval = 0;//20;
	Settings.DisableSampleCaching=TRUE;	
	Settings.FixFrequency = true;
	// Tracing options
	Settings.TraceDMA = false;
	Settings.TraceHDMA = false;
	Settings.TraceVRAM = false;
	Settings.TraceUnknownRegisters = false;
	Settings.TraceDSP = false;
	// Joystick options
	Settings.SwapJoypads = false;
	Settings.JoystickEnabled = false;
	// ROM timing options (see also H_Max above)
	Settings.PAL = false;
	Settings.FrameTimePAL = 20;
	Settings.FrameTimeNTSC = 17;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	// CPU options 
	Settings.CyclesPercentage = 100;
	Settings.Shutdown = true;
	Settings.ShutdownMaster = true;
	Settings.APUEnabled = (iphone_soundon!=0);
	Settings.DisableIRQ = 0; //os9x_DisableIRQ;
	Settings.Paused = false;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;    
	Settings.SkipFrames=AUTO_FRAMERATE;
	// ROM image and peripheral options
	Settings.ForceSuperFX = false;
	Settings.ForceNoSuperFX = false;
	Settings.MultiPlayer5 = true;
	Settings.Mouse = true;
	Settings.SuperScope = true;
	Settings.MultiPlayer5Master = true;
	Settings.SuperScopeMaster = true;
	Settings.MouseMaster = true;
	Settings.SuperFX = false; 
	// SNES graphics options
	Settings.BGLayering = false;
	Settings.DisableGraphicWindows = false;
	Settings.ForceTransparency = false;
	Settings.ForceNoTransparency = false;
	Settings.DisableHDMA = 0; //os9x_DisableHDMA;
	Settings.Mode7Interpolate = false;
	Settings.DisplayFrameRate = false;
    
	Settings.SixteenBit = 1;
	Settings.Transparency = 1;
	Settings.SupportHiRes = false;
	
	Settings.AutoSaveDelay = 5;
	Settings.ApplyCheats = true;
	
	Settings.TurboSkipFrames = 20;
	Settings.AutoMaxSkipFrames = 10;
    
	Settings.ForcedPause = 0;
	Settings.StopEmulation = TRUE;
	Settings.Paused = FALSE;       
	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
	
	///////////////////
	/////////////////// 
	
	//GFX.RealPitch = GFX.Pitch = 318 * 2;
	
    printf("creating vrambuffer\n"); fflush(stdout);
	
	GFX.Pitch = SNES_WIDTH * 2;
	GFX.RealPitch = SNES_WIDTH * 2;
	vrambuffer = (uint8 *) malloc (GFX.RealPitch * 480 * 2);
	memset (vrambuffer, 0, GFX.RealPitch*480*2);
	GFX.Screen = vrambuffer; // + (640*8) + 64;
	
	GFX.SubScreen = (uint8 *)malloc(GFX.RealPitch * 480 * 2); 
	GFX.ZBuffer =  (uint8 *)malloc(GFX.RealPitch * 480 * 2); 
	GFX.SubZBuffer = (uint8 *)malloc(GFX.RealPitch * 480 * 2);
	GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
	GFX.PPL = GFX.Pitch >> 1;
	GFX.PPLx2 = GFX.Pitch;
	GFX.ZPitch = GFX.Pitch >> 1;
	
	if (Settings.ForceNoTransparency)
         Settings.Transparency = FALSE;

	if (Settings.Transparency)
         Settings.SixteenBit = TRUE;

	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

    printf("init memory and APU\n"); fflush(stdout);
	if (!Memory.Init () || !S9xInitAPU())
         erk();

	//S9xSetRenderPixelFormat (RGB565);

     printf("initing graphics\n"); fflush(stdout);
	if (!S9xGraphicsInit ())
         erk();

	snesMenuOptions.menuVer=SNES_OPTIONS_VER;
	snesMenuOptions.frameSkip=5; //preferences.frameSkip;
	snesMenuOptions.soundOn=iphone_soundon; 
	snesMenuOptions.volume=100; 
	memset(snesMenuOptions.padConfig,0xFF,sizeof(snesMenuOptions.padConfig));
	snesMenuOptions.showFps= (__fps_debug ? 1 : 0);
	snesMenuOptions.gamma=0;
	snesMenuOptions.soundRate=2;
	snesMenuOptions.cpuSpeed=19;
	snesMenuOptions.autoSram=0;
	snesMenuOptions.transparency=(__transparency ? 1 : 0); //preferences.transparency;

	
	S9xReset();
		
		#ifdef __IPHONE__
        	if( (!strcasecmp(filename + (strlen(filename)-3), ".sv")) )
        	{
        		unsigned long pos;
                char *temp = (char *)malloc(MAX_PATH + 1);
        		sprintf(temp, "%s", filename);
        		pos = strlen(filename)-17;
        		filename[pos] = '\0';
        		__saved = 1;
        		sprintf(save_filename,"%s%s%s",snesSaveStateDir,DIR_SEPERATOR,temp);
                free(temp);
        	} else {
                save_filename[0] = '\0';
            }
        #endif

	//Save current rom shortname for save state etc
	strcpy(currentRomFilename,filename);
	
	// get full filename
	sprintf(filename,"%s%s%s",snesRomDir,DIR_SEPERATOR,currentRomFilename);


	
	printf("Loading Rom: %s\n", filename) ; fflush(stdout);
	
	if (!Memory.LoadROM (filename))
	{
        printf("Load ROM Failed!\n"); fflush(stdout);
		sprintf(text,"Loading Rom...Failed");
		gp_drawString(0,0,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
		MenuFlip();
		MenuPause();
		return 0;
	}
    printf("Load ROM OK!\n"); fflush(stdout);
	
  sprintf(Memory.ROMFilename, "%s", filename);
	
	sprintf(text,"Loading Rom...OK!");
	gp_drawString(0,0,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	sprintf(text,"Loading Sram");
	gp_drawString(0,8,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	
	//Memory.LoadSRAM (S9xGetFilename (".srm")); 
	printf("Loading sram...\n"); fflush(stdout);
	S9xLoadSRAM();
    printf("loaded sram OK\n"); fflush(stdout);

	if(os9x_findhacks(Memory.ROMCRC32))
	{
		sprintf(text,"Speedhack found! YAY!");
		gp_drawString(0,16,strlen(text),text,0xFFFF,(unsigned char*)vrambuffer);
	}
	MenuFlip();
	
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);

	// any change in configuration?
	gp_setCpuspeed(cpuSpeedLookup[snesMenuOptions.cpuSpeed]);
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
	
	if (snesMenuOptions.transparency)
	{
		Settings.Transparency = TRUE;
		Settings.SixteenBit = TRUE;
	}
	else
	{
		Settings.Transparency = FALSE;
		Settings.SixteenBit = TRUE;
	}
	
	// TEMP EDIT 
	//if (!S9xGraphicsInit ())
	//	erk();
 
	if (snesMenuOptions.renderMode == RENDER_MODE_SCALED)
	{
		gp2x_video_RGB_setscaling(256,224);
	}

    if (strlen(save_filename) > 0) 
    {
        printf("Loading save state: %s\n", save_filename); fflush(stdout);
      LoadStateFile(save_filename);
      printf("Load finished\n"); fflush(stdout);
      __saved = 2;
    }

	S9xSetSoundMute (TRUE);
	
	CPU.APU_APUExecuting = Settings.APUEnabled = snesMenuOptions.soundOn;
	
	if (snesMenuOptions.soundOn)
	{
		//Settings.SoundPlaybackRate=(unsigned int)soundRates[snesMenuOptions.soundRate];
		samplecount=Settings.SoundPlaybackRate/(Settings.PAL ? 50 : 60);
		Settings.SixteenBitSound=true;
		Settings.Stereo=true;
		Settings.SoundBufferSize=samplecount<<(1+(Settings.Stereo?1:0));
		so.stereo = Settings.Stereo;
		so.playback_rate = Settings.SoundPlaybackRate;
		S9xInitSound(); // (Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize);
		S9xSetPlaybackRate(so.playback_rate);
		S9xSetSoundMute (FALSE);
		app_DemuteSound(Settings.SoundBufferSize);
	}

	
	const long frame_speed = (Settings.PAL ? Settings.FrameTimePAL : Settings.FrameTimeNTSC);
	fps = (Settings.PAL ? 50 : 60);
	
	int skipcount = 0;
	int skipper = 0;
	unsigned long tickframe = 0;
	unsigned long frame_ticks_total = 0;
	
    printf("entering main loop\n"); fflush(stdout);
	while (1)
	{
		unsigned long frame_ticks;
		
		Timer=gp2x_timer_read();
		frame_ticks = Timer - tickframe;
		frame_ticks_total += frame_ticks;
		tickframe = Timer;
		Frames++;
		
		/*if(isMultiTouching != 0)
		{
			if(frame_ticks > frame_speed-8) // 10+
			{
				skipper++;
				if(skipper < 4)
				{
					skipcount++;
					IPPU.RenderThisFrame=FALSE;
				}
				else
				{
					skipper = 0;
					IPPU.RenderThisFrame=TRUE;
				}
			}
			else
			{
				skipper = 0;
				IPPU.RenderThisFrame=TRUE;
			}		
		}
		else*/
		{
			/*if((int)frame_ticks > frame_speed) // 14+
			{
				skipper++;
				if(skipper < 4)
				{
					skipcount++;
					IPPU.RenderThisFrame=FALSE;
				}
				else
				{
					skipper = 0;
					IPPU.RenderThisFrame=TRUE;
				}
			}
			else*/
			{
		    if(((frame_speed*Frames) > frame_ticks_total))
		    {
			    usleep(((frame_speed*Frames) - frame_ticks_total) * 1000);
          //frame_ticks_total = (frame_speed*Frames);
          skipper = 0;
          IPPU.RenderThisFrame = TRUE;
			  }
			  else
  			{
  				skipper++;
  				if(skipper < 10)
  				{
  					skipcount++;
  					IPPU.RenderThisFrame=FALSE;
  				}
  				else
  				{
  					skipper = 0;
  					IPPU.RenderThisFrame=TRUE;
  				}
  			}
  		}
		}
		
		if(Timer-tick>=(TIMER_1_SECOND))
		{
			fps=Frames;
			Frames=0;
			tick=Timer;
			sprintf(fps_display,"%d %d",fps, skipcount);
			skipcount = 0;
			frame_ticks_total = 0;
		}
		
		S9xMainLoop ();

		if(__autosave && ++SaveFrames >= 18000 )
		{
			char svfilename[1024];
			sprintf(svfilename, "%s-last-autosave.sv", filename);
            sprintf(svfilename, "%s%s%s", snesSaveStateDir, DIR_SEPERATOR, svfilename);
			gp_drawString(0,0,strlen("autosaving!"),"autosaving",0xFFFF,(unsigned char*)vrambuffer);
			MenuFlip();
			SaveStateFile(svfilename);
			SaveFrames = 0;
		}

		do
		{
  		if( !__emulation_run || __emulation_saving) 
  		{
  			char buffer[260];
  			char svfilename[1024];
  			time_t curtime;
  			struct tm *loctime;
			
  			S9xSaveSRAM();
			
  			if(__emulation_saving)
  			{
                printf("saving state ");
  				if(__saved != 0 && __emulation_saving == 2 && strlen(save_filename) > 0)
  				{
                    printf("to current file: %s\n", save_filename);fflush(stdout);
  					sprintf(svfilename, "%s", save_filename);
  				}
  				else
  				{
                    
  					curtime = time (NULL);
  					loctime = localtime (&curtime);
  					strftime (buffer, 260, "%y%m%d-%I%M%S", loctime);
  					sprintf(svfilename, "%s%s%s-%s.sv", snesSaveStateDir, DIR_SEPERATOR, currentRomFilename, buffer);
  					printf("to new file: %s\n", svfilename);fflush(stdout);
  				}
  				SaveStateFile(svfilename);
                char *imagefile = (char *)malloc(MAX_PATH + 1);
                sprintf(imagefile, "%s.png", svfilename);
                saveScreenshotToFile(imagefile);
                free(imagefile);
  				__emulation_saving = 0;
  			}
  			if(!__emulation_run)
  			{
    			set_gamma(100);
    			gp_Reset();
    			app_MuteSound();
    			gp_deinitGraphics();
    			/*if(vrambuffer != NULL) free(vrambuffer);
    			vrambuffer = NULL;
    			S9xGraphicsDeinit();
    			S9xDeinitAPU();
    			Memory.Deinit();
    			pthread_exit(NULL);*/
    			break;
    		}
  		}
			if(__emulation_paused)
			{
        usleep(16666);
        //sched_yield();
			}
    } while(__emulation_paused);
	}
	
	return 0;
}
