
#include "menu.h"

#if defined(__GP2X__)
#include "usbjoy.h"
static struct usbjoy *joys[4];
static char joyCount = 0;
static int buttonMap[4][32];
#endif

struct INPUT Inp;
static int repeatCounter = 0;
int InputInit()
{
  memset(&Inp,0,sizeof(struct INPUT));
#if defined(__GP2X__)
int i;
	for (i=1; i<5; i++)
	{
		struct usbjoy *joy = joy_open(i);
		if(joy != NULL)
		{
			joys[joyCount] = joy;
			memset(buttonMap[joyCount],0,sizeof(buttonMap[joyCount]));
			buttonMap[joyCount][0] = (1<<INP_BUTTON_A);
			buttonMap[joyCount][1] = (1<<INP_BUTTON_B);
			buttonMap[joyCount][2] = (1<<INP_BUTTON_X);
			buttonMap[joyCount][3] = (1<<INP_BUTTON_Y);
			buttonMap[joyCount][4] = (1<<INP_BUTTON_L);
			buttonMap[joyCount][5] = (1<<INP_BUTTON_R);
			if (joy->numbuttons<10)
			{
				buttonMap[joyCount][6] = (1<<INP_BUTTON_SELECT);
				buttonMap[joyCount][7] = (1<<INP_BUTTON_START);
			}
			else
			{
				buttonMap[joyCount][6] = (1<<INP_BUTTON_L);
				buttonMap[joyCount][7] = (1<<INP_BUTTON_R);
				buttonMap[joyCount][8] = (1<<INP_BUTTON_SELECT);
				buttonMap[joyCount][9] = (1<<INP_BUTTON_START);
			}
			joyCount++;
		}
	}
#endif
  return 0;
}

#if defined(__GP2X__)
int InputClose()
{
int i;
	for (i=0; i<joyCount; i++)
	{
		joy_close(joys[i]);
	}
}

int joy_getButton(int joyNumber)
{
unsigned int key=0;
	if (joyNumber<joyCount)
	{
		int i;
		joy_update(joys[joyNumber]);
		if(joy_getaxe(JOYUP, joys[joyNumber])) key|=	(1<<INP_BUTTON_UP);
		if(joy_getaxe(JOYDOWN, joys[joyNumber])) key|=	 (1<<INP_BUTTON_DOWN);
		if(joy_getaxe(JOYLEFT, joys[joyNumber])) key|=	 (1<<INP_BUTTON_LEFT);
		if(joy_getaxe(JOYRIGHT, joys[joyNumber])) key|= (1<<INP_BUTTON_RIGHT);
		for (i = 0; i < joy_buttons(joys[joyNumber]); i++)
		{
			if (joy_getbutton(i, joys[joyNumber]))
				key|= buttonMap[joyNumber][i];
		}
	}
	return key;
}

char joy_Count()
{
	return joyCount;
}
#endif

int InputUpdate(int EnableDiagnals)
{
  int i=0;
  unsigned int key=0;
  // Get input
#if defined(__GIZ__)
  key=gp_getButton(EnableDiagnals);
  key&=	(1<<INP_BUTTON_UP)|
			(1<<INP_BUTTON_LEFT)|
			(1<<INP_BUTTON_DOWN)|
			(1<<INP_BUTTON_RIGHT)|
			(1<<INP_BUTTON_HOME)|
			(1<<INP_BUTTON_VOL)|
			(1<<INP_BUTTON_L)|
			(1<<INP_BUTTON_R)|
			(1<<INP_BUTTON_REWIND)|
			(1<<INP_BUTTON_FORWARD)|
			(1<<INP_BUTTON_PLAY)|
			(1<<INP_BUTTON_STOP)|
			(1<<INP_BUTTON_BRIGHT);
#endif

#if defined(__IPHONE__)
  key=gp_getButton(EnableDiagnals);
#endif

#if defined(__GP2X__)
  key=gp_getButton(EnableDiagnals);
  key&=	(1<<INP_BUTTON_UP)|
			(1<<INP_BUTTON_LEFT)|
			(1<<INP_BUTTON_DOWN)|
			(1<<INP_BUTTON_RIGHT)|
			(1<<INP_BUTTON_START)|
			(1<<INP_BUTTON_SELECT)|
			(1<<INP_BUTTON_L)|
			(1<<INP_BUTTON_R)|
			(1<<INP_BUTTON_A)|
			(1<<INP_BUTTON_B)|
			(1<<INP_BUTTON_X)|
			(1<<INP_BUTTON_Y)|
			(1<<INP_BUTTON_VOL_UP)|
			(1<<INP_BUTTON_VOL_DOWN)|
			(1<<INP_BUTTON_STICK_PUSH);
  key |= joy_getButton(0);
#endif
  // Find out how long key was pressed for
  for (i=0;i<32;i++)
  {
    int held=Inp.held[i];

    if (key&(1<<i)) held++; else held=0;

    //if (held>=0x80) held&=0xbf; // Keep looping around

    Inp.held[i]=held;
  }

  // Work out some key repeat values:
  for (i=0;i<32;i++)
  {
    char rep=0;
    int held=Inp.held[i];

    if (held==1) 
	{
		// Key has just been pressed again, so set repeat by default
		rep=1;
	}
	else
	{
		// Now make sure key has been held for a period of time
		// before auto toggling the repeat flag
		if (held>=0x20)
		{
			repeatCounter++;
			if(repeatCounter>15)
			{
				rep=1;
				repeatCounter=0;
			}
		}
	}

    Inp.repeat[i]=rep;
  }

  return 0;
}
