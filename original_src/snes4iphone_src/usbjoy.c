/* Title: USB Joystick library
   Version 0.2
   Written by Puck2099 (puck2099@gmail.com), (c) 2006.
   <http://www.gp32wip.com>
   
   If you use this library or a part of it, please, let it know.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdlib.h>
#include <stdio.h>		/* For the definition of NULL */
#include <sys/types.h>	        // For Device open
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>		// For Device read

#include <string.h>
#include <limits.h>		/* For the definition of PATH_MAX */
#include <linux/joystick.h>

#include "usbjoy.h"


/*
  Function: joy_open

  Opens a USB joystick and fills its information.

  Parameters:

  joynumber - Joystick's identifier (0 reserved for GP2X's builtin Joystick).

  Returns:

  Filled usbjoy structure.

*/
struct usbjoy * joy_open (int joynumber) {
  int fd, i;
  char path [128];
  struct usbjoy * joy = NULL;

  system ("insmod joydev"); // Loads joydev module

  if (joynumber == 0) {
  }
  else if (joynumber > 0) {
    sprintf (path, "/dev/input/js%d", joynumber-1);
    fd = open(path, O_RDONLY, 0);
    if (fd > 0) {
      joy = (struct usbjoy *) malloc(sizeof(struct usbjoy));

      // Joystick's file descriptor
      joy->fd = fd;

      // Set the joystick to non-blocking read mode
      fcntl(joy->fd, F_SETFL, O_NONBLOCK);

      // Joystick's name
      ioctl(joy->fd, JSIOCGNAME(128*sizeof(char)), joy->name);

      // Joystick's device
      sprintf (joy->device, path);

      // Joystick's buttons
      ioctl(joy->fd, JSIOCGBUTTONS, &joy->numbuttons);

      // Joystick's axes
      ioctl(joy->fd, JSIOCGAXES, &joy->numaxes);

      // Clean buttons and axes
      for (i=0; i<32; i++) joy->statebuttons[i] = 0;
      for (i=0; i<4; i++) joy->stateaxes[i] = 0;
    }
    else {
      printf ("ERROR: No Joystick found\n");
    }
  }
  return joy;
}

/*
  Function: joy_name

  Returns Joystick's name.

  Parameters:

  joy - Selected joystick.

  Returns:

  Joystick's name or NULL if <usbjoy> struct is empty.
*/
char * joy_name (struct usbjoy * joy) {
  if (joy != NULL)  return joy->name;
  else return NULL;
}


/*
  Function: joy_device

  Returns Joystick's device.

  Parameters:

  joy - Selected joystick.

  Returns:

  Joystick's device or NULL if <usbjoy> struct is empty.
*/
char * joy_device (struct usbjoy * joy) {
  if (joy != NULL)  return joy->device;
  else return NULL;
}


/*
  Function: joy_buttons

  Returns Joystick's buttons number.

  Parameters:

  joy - Selected joystick.

  Returns:

  Joystick's buttons or 0 if <usbjoy> struct is empty.
*/
int joy_buttons (struct usbjoy * joy) {
  if (joy != NULL) return joy->numbuttons;
  else return 0;
}


/*
  Function: joy_axes

  Returns Joystick's axes number.

  Parameters:

  joy - Selected joystick.

  Returns:

  Joystick's axes or 0 if <usbjoy> struct is empty.
*/
int joy_axes (struct usbjoy * joy) {
  if (joy != NULL) return joy->numaxes;
  else return 0;
}


/*
  Function: joy_update

  Updates Joystick's internal information (<statebuttons> and <stateaxes> fields).

  Parameters:

  joy - Selected joystick.

  Returns:

  0 - No events registered (no need to update).
  1 - Events registered (a button or axe has been pushed).
  -1 - Error: <usbjoy> struct is empty.
*/
int joy_update (struct usbjoy * joy) {
  struct js_event events[0xff];
  int i, len;
  int event = 0;
  if (joy != NULL) {    
    if ((len=read(joy->fd, events, (sizeof events))) >0) {
      len /= sizeof(events[0]);
      for ( i=0; i<len; ++i ) {
	switch (events[i].type & ~JS_EVENT_INIT) {
	case JS_EVENT_AXIS:
	  if (events[i].number == 0) {
	    joy->stateaxes[JOYLEFT] = joy->stateaxes[JOYRIGHT] = 0;
	    if (events[i].value < 0) joy->stateaxes[JOYLEFT] = 1;
	    else if (events[i].value > 0) joy->stateaxes[JOYRIGHT] = 1;
	  }
	  else if (events[i].number == 1) {
	    joy->stateaxes[JOYUP] = joy->stateaxes[JOYDOWN] = 0;
	    if (events[i].value < 0) joy->stateaxes[JOYUP] = 1;
	    else if (events[i].value > 0) joy->stateaxes[JOYDOWN] = 1;
	  }
	  event = 1;
	  break;
	case JS_EVENT_BUTTON:
	  joy->statebuttons[events[i].number] = events[i].value;
	  event = 1;
	  break;
	default:
	  break;
	}
      }
    }
  }
  else {
    event = -1;
  }   
  return event;
}


/*
  Function: joy_getbutton

  Returns Joystick's button information.

  Parameters:

  button - Button which value you want to know (from 0 to 31).
  joy - Selected joystick.

  Returns:

  0 - Button NOT pushed.
  1 - Button pushed.
  -1 - Error: <usbjoy> struct is empty.
*/
int joy_getbutton (int button, struct usbjoy * joy) {
  if (joy != NULL) {
    if (button < joy_buttons(joy)) return joy->statebuttons[button];
    else return 0;
  }
  else return -1;
}


/*
  Function: joy_getaxe

  Returns Joystick's axes information.

  Parameters:

  axe - Axe which value you want to know (see <Axes values>).
  joy - Selected joystick.

  Returns:

  0 - Direction NOT pushed.
  1 - Direction pushed.
  -1 - Error: <usbjoy> struct is empty.
*/
int joy_getaxe (int axe, struct usbjoy * joy) {
  if (joy != NULL) {
    if (axe < 4) return joy->stateaxes[axe];
    else return 0;
  }
  else return -1;
}


/*
  Function: joy_close

  Closes selected joystick's file descriptor and detroys it's fields.

  Parameters:

  joy - Selected joystick.

  Returns:

  0 - Joystick successfully closed.
  -1 - Error: <usbjoy> struct is empty.
*/
int joy_close (struct usbjoy * joy) {
  if (joy != NULL) {
    close (joy->fd);
    free (joy);
    return 0;
  }
  else return -1;
}
