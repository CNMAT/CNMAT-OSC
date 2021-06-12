#ifndef PRINTF
#define PRINTF printf
#endif

#ifndef COMPLAIN
extern void complain(char *s, ...);
#define COMPLAIN complain
#endif

#include <stdio.h>
#include <string.h> // strncmp
#include <ctype.h> // isprint

#include "printOSCpacket.h"

void PrintOSCPacketRecursive(char *buf, int n, int bundleDepth);
static void PrintOSCMessage(char *address, void *v, int n, char *indent);
static void PrintTypeTaggedArgs(void *v, int n);
static void PrintHeuristicallyTypeGuessedArgs(void *v, int n, int skipComma);
char *DataAfterAlignedString(char *string, char *boundary) ;
int IsNiceString(char *string, char *boundary) ;

char *error_string;

void PrintOSCPacket(char *buf, int n) {
  PrintOSCPacketRecursive(buf, n, 0);
}


#define MAX_DEPTH 10
#define INDENTATION_AMOUNT 2

void PrintOSCPacketRecursive(char *buf, int n, int bundleDepth) {
    int size, messageLen, i;
    char *messageName;
    char *args;
    char indentation[MAX_DEPTH*INDENTATION_AMOUNT +1];

    if ((n%4) != 0) {
	COMPLAIN("OSC packet size (%d) not a multiple of 4 bytes: dropping",
		 n);
	return;
    }

    if (bundleDepth > MAX_DEPTH) bundleDepth = MAX_DEPTH;

    for (i=0; i<bundleDepth * INDENTATION_AMOUNT; ++i) {
      indentation[i] = ' ';
    }
    indentation[i] = '\0';


    if ((n >= 8) && (strncmp(buf, "#bundle", 8) == 0)) {
	/* This is a bundle message. */

	if (n < 16) {
	    COMPLAIN("Bundle message too small (%d bytes) for time tag", n);
	    return;
	}

	/* Print the time tag */
	PRINTF("%s[ %lx%08lx\n", indentation,
	       ntohl(*((unsigned long *)(buf+8))),
	       ntohl(*((unsigned long *)(buf+12))));
	/* Note: if we wanted to actually use the time tag as a little-endian
	   64-bit int, we'd have to word-swap the two 32-bit halves of it */

	i = 16; /* Skip "#bundle\0" and time tag */
	while(i<n) {
	    size = ntohl(*((int *) (buf + i)));
	    if ((size % 4) != 0) {
		COMPLAIN("Bad size count %d in bundle (not a multiple of 4)", size);
		return;
	    }
	    if ((size + i + 4) > n) {
		COMPLAIN("Bad size count %d in bundle (only %d bytes left in entire bundle)",
			 size, n-i-4);
		return;	
	    }
	    
	    /* Recursively handle element of bundle */
	    PrintOSCPacketRecursive(buf+i+4, size, bundleDepth+1);
	    i += 4 + size;
	}
	if (i != n) {
	    COMPLAIN("This can't happen");
	}
	PRINTF("%s]\n", indentation);
    } else {
	/* This is not a bundle message */

	messageName = buf;
	args = DataAfterAlignedString(messageName, buf+n);
	if (args == 0) {
	    COMPLAIN("Bad message name string: %s\nDropping entire message.\n",
		     error_string);
	    return;
	}
	messageLen = args-messageName;	    
	PrintOSCMessage(messageName, (void *)args, n-messageLen, indentation);
    }
}


#define SMALLEST_POSITIVE_FLOAT 0.000001f

static void PrintOSCMessage(char *address, void *v, int n, char *indentation) {
    char *chars = v;

    PRINTF("%s%s ", indentation, address);

    if (n != 0) {
	if (chars[0] == ',') {
	    if (chars[1] != ',') {
		/* This message begins with a type-tag string */
		PrintTypeTaggedArgs(v, n);
	    } else {
		/* Double comma means an escaped real comma, not a type string */
		PrintHeuristicallyTypeGuessedArgs(v, n, 1);
	    }
	} else {
	    PrintHeuristicallyTypeGuessedArgs(v, n, 0);
	}
    }

    PRINTF("\n");
    fflush(stdout);	/* Added for Sami 5/21/98 */
}

static void PrintTypeTaggedArgs(void *v, int n) { 
    char *typeTags, *thisType;
    char *p;

    typeTags = v;

    if (!IsNiceString(typeTags, typeTags+n)) {
	/* No null-termination, so maybe it wasn't a type tag
	   string after all */
	PrintHeuristicallyTypeGuessedArgs(v, n, 0);
	return;
    }

    p = DataAfterAlignedString(typeTags, typeTags+n);


    for (thisType = typeTags + 1; *thisType != 0; ++thisType) {
	switch (*thisType) {
	    case 'i': case 'r': case 'm': case 'c':
	    PRINTF("%d ", ntohl(*((int *) p)));
	    p += 4;
	    break;

	    case 'f': {
		int i = ntohl(*((int *) p));
		float *floatp = ((float *) (&i));
		PRINTF("%f ", *floatp);
		p += 4;
	    }
	    break;

	    case 'h': case 't':
	    PRINTF("[A 64-bit int] ");
	    /* Syntaxes for printing this include %64l and %ll... */

	    p += 8;
	    break;

	    case 'd':
	    PRINTF("[A 64-bit float] ");
	    p += 8;
	    break;

	    case 's': case 'S':
	    if (!IsNiceString(p, typeTags+n)) {
		PRINTF("Type tag said this arg is a string but it's not!\n");
		return;
	    } else {
		PRINTF("\"%s\" ", p);
		p = DataAfterAlignedString(p, typeTags+n);
	    }
	    break;

	    case 'T': PRINTF("[True] "); break;
	    case 'F': PRINTF("[False] "); break;
	    case 'N': PRINTF("[Nil]"); break;
	    case 'I': PRINTF("[Infinitum]"); break;

	    default:
	    PRINTF("[Unrecognized type tag %c]", *thisType);
	    return;
	}
    }
}

static void PrintHeuristicallyTypeGuessedArgs(void *v, int n, int skipComma) {
    int i, thisi;
    float thisf;
    int *ints;
    char *chars;
    char *string, *nextString;
	

    /* Go through the arguments 32 bits at a time */
    ints = v;
    chars = v;

    for (i = 0; i<n/4; ) {
	string = &chars[i*4];
	thisi = ntohl(ints[i]);
	/* Reinterpret the (potentially byte-reversed) thisi as a float */
	thisf = *(((float *) (&thisi)));

	if  (thisi >= -1000 && thisi <= 1000000) {
	    PRINTF("%d ", thisi);
	    i++;
	} else if (thisf >= -1000.f && thisf <= 1000000.f &&
		   (thisf <=0.0f || thisf >= SMALLEST_POSITIVE_FLOAT)) {
	    PRINTF("%f ",  thisf);
	    i++;
	} else if (IsNiceString(string, chars+n)) {
	    nextString = DataAfterAlignedString(string, chars+n);
	    PRINTF("\"%s\" ", (i == 0 && skipComma) ? string +1 : string);
	    i += (nextString-string) / 4;
	} else {
	    PRINTF("0x%x ", ints[i]);
	    i++;
	}
    }
}


#define STRING_ALIGN_PAD 4

char *DataAfterAlignedString(char *string, char *boundary) 
{
    /* The argument is a block of data beginning with a string.  The
       string has (presumably) been padded with extra null characters
       so that the overall length is a multiple of STRING_ALIGN_PAD
       bytes.  Return a pointer to the next byte after the null
       byte(s).  The boundary argument points to the character after
       the last valid character in the buffer---if the string hasn't
       ended by there, something's wrong.

       If the data looks wrong, return 0, and set error_string */

    int i;

    if ((boundary - string) %4 != 0) {
	fprintf(stderr, "Internal error: DataAfterAlignedString: bad boundary\n");
	return 0;
    }

    for (i = 0; string[i] != '\0'; i++) {
	if (string + i >= boundary) {
	    error_string = "DataAfterAlignedString: Unreasonably long string";
	    return 0;
	}
    }

    /* Now string[i] is the first null character */
    i++;

    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	if (string + i >= boundary) {
	    error_string = "DataAfterAlignedString: Unreasonably long string";
	    return 0;
	}
	if (string[i] != '\0') {
	    error_string = "DataAfterAlignedString: Incorrectly padded string.";
	    return 0;
	}
    }

    return string+i;
}


int IsNiceString(char *string, char *boundary) 
{
    /* Arguments same as DataAfterAlignedString().  Is the given "string"
       really a string?  I.e., is it a sequence of isprint() characters
       terminated with 1-4 null characters to align on a 4-byte boundary? */

    int i;

    if ((boundary - string) %4 != 0) {
	fprintf(stderr, "Internal error: IsNiceString: bad boundary\n");
	return 0;
    }

    for (i = 0; string[i] != '\0'; i++) {
	if (!isprint(string[i])) return 0;
	if (string + i >= boundary) return 0;
    }

    /* If we made it this far, it's a null-terminated sequence of printing characters 
       in the given boundary.  Now we just make sure it's null padded... */

    /* Now string[i] is the first null character */
    i++;
    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	if (string[i] != '\0') return 0;
    }

    return 1;
}



