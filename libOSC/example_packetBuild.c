/* buildPacket.c -- builds a lame OSC packet. 
 * needs OSC-client.c and OSC-timetag.c to compile. Gillies092605
 */

#include "OSC-client.h"
#include <stdio.h>

#define BUF_SIZE 256

int main()
{
	/* addresses */
	char a1[] = "the/first/address";
	char a2[] = "the/second/address";
	/* type strings */
	char t1[] = ",ff";
	char t2[] = ",i";
	/* message data */
	float floats[] = {1.0f, 5.0f};
	int4byte i = 4;
	int errorcode;

	OSCbuf oscbuf;
	OSCbuf * b = &oscbuf;
	char packetBuf[BUF_SIZE];
	
	OSC_initBuffer(b, sizeof(char) * BUF_SIZE, packetBuf);
	OSC_openBundle(b, OSCTT_CurrentTime());
	
	/* here comes first message in bundle */
	if (OSC_writeAddressAndTypes(b, a1, t1)) {
	  printf("1: %s\n", OSC_errorMessage);
	}

	printf("Current typeStringPtr %p points to %d ('%c')\n",
	       b->typeStringPtr, *(b->typeStringPtr), *(b->typeStringPtr));

	if (OSC_writeFloatArg(b, floats[0])) {
	  printf("2a: %s\n", OSC_errorMessage);
	}

	printf("Current typeStringPtr %p points to %d ('%c')\n",
	       b->typeStringPtr, *(b->typeStringPtr), *(b->typeStringPtr));

	if (OSC_writeFloatArg(b, floats[1])) {
	  printf("2b: %s\n", OSC_errorMessage);
	}
	
	printf("Current typeStringPtr %p points to %d ('%c')\n",
	       b->typeStringPtr, *(b->typeStringPtr), *(b->typeStringPtr));

	printf("-----------so far, so good-----------\n");
	
	/* here comes second message in bundle */
	errorcode = OSC_writeAddressAndTypes(b, a2, t2);

	printf("hosed?\n");

	if(errorcode)	{	
	  printf("Error code %d\n", errorcode);
		printf("3%s\n", OSC_errorMessage);	 /* why error? */
		return 1;
	}
	
	OSC_writeIntArg(b, i);
	
	OSC_closeBundle(b);

	return 0;
}
