#ifndef PRINTF
#define PRINTF printf
#endif

#ifndef COMPLAIN
extern void complain(char *s, ...);
#define COMPLAIN complain
#endif

#include <stdio.h>
#include "printOSCpacket.h"

typedef int Boolean;
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif



static void Smessage(char *address, void *v, int n);
static void PrintTypeTaggedArgs(void *v, int n);
static void PrintHeuristicallyTypeGuessedArgs(void *v, int n, int skipComma);
char *DataAfterAlignedString(char *string, char *boundary) ;
Boolean IsNiceString(char *string, char *boundary) ;


char *error_string;

void PrintOSCPacket(char *buf, int n) {
    int size, messageLen, i;
    char *messageName;
    char *args;

    if ((n%4) != 0) {
	COMPLAIN("OSC packet size (%d) not a multiple of 4 bytes: dropping",
		 n);
	return;
    }

    if ((n >= 8) && (strncmp(buf, "#bundle", 8) == 0)) {
	/* This is a bundle message. */

	if (n < 16) {
	    COMPLAIN("Bundle message too small (%d bytes) for time tag", n);
	    return;
	}

	/* Print the time tag */
	printf("[ %lx%08lx\n", ntohl(*((unsigned long *)(buf+8))),
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
	    PrintOSCPacket(buf+i+4, size);
	    i += 4 + size;
	}
	if (i != n) {
	    COMPLAIN("This can't happen");
	}
	printf("]\n");
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
	Smessage(messageName, (void *)args, n-messageLen);
    }
}


#define SMALLEST_POSITIVE_FLOAT 0.000001f

static void Smessage(char *address, void *v, int n) {
    char *chars = v;

    printf("%s ", address);

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

    printf("\n");
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
	    printf("%d ", ntohl(*((int *) p)));
	    p += 4;
	    break;

	    case 'f': {
		int i = ntohl(*((int *) p));
		float *floatp = ((float *) (&i));
		printf("%f ", *floatp);
		p += 4;
	    }
	    break;

	    case 'h': case 't':
	    printf("[A 64-bit int] ");
	    /* Syntaxes for printing this include %64l and %ll... */

	    p += 8;
	    break;

	    case 'd':
	    printf("[A 64-bit float] ");
	    p += 8;
	    break;

	    case 's': case 'S':
	    if (!IsNiceString(p, typeTags+n)) {
		printf("Type tag said this arg is a string but it's not!\n");
		return;
	    } else {
		printf("\"%s\" ", p);
		p = DataAfterAlignedString(p, typeTags+n);
	    }
	    break;

	    case 'T': printf("[True] "); break;
	    case 'F': printf("[False] "); break;
	    case 'N': printf("[Nil]"); break;
	    case 'I': printf("[Infinitum]"); break;

	    default:
	    printf("[Unrecognized type tag %c]", *thisType);
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
	    printf("%d ", thisi);
	    i++;
	} else if (thisf >= -1000.f && thisf <= 1000000.f &&
		   (thisf <=0.0f || thisf >= SMALLEST_POSITIVE_FLOAT)) {
	    printf("%f ",  thisf);
	    i++;
	} else if (IsNiceString(string, chars+n)) {
	    nextString = DataAfterAlignedString(string, chars+n);
	    printf("\"%s\" ", (i == 0 && skipComma) ? string +1 : string);
	    i += (nextString-string) / 4;
	} else {
	    printf("0x%x ", ints[i]);
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

Boolean IsNiceString(char *string, char *boundary) 
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
	if (!isprint(string[i])) return FALSE;
	if (string + i >= boundary) return FALSE;
    }

    /* If we made it this far, it's a null-terminated sequence of printing characters 
       in the given boundary.  Now we just make sure it's null padded... */

    /* Now string[i] is the first null character */
    i++;
    for (; (i % STRING_ALIGN_PAD) != 0; i++) {
	if (string[i] != '\0') return FALSE;
    }

    return TRUE;
}



