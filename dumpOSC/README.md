## The "dumpOSC" Program

Matt Wright, 2/3/98. Updated 3/3/03

The program "dumpOSC" listens on a UDP or Unix port for messages in
the [OpenSound Control](index.html) network format, and prints their
contents to the screen as they are received. This is useful primarily
for debugging OpenSound Control clients.

### Availability

The "dumpOSC" program is available as [source code](/OSC/src/dumpOSC/)
and as [compiled binaries for Mac OS X](/OSC/send%2BdumpOSC-OSX.tar.gz).
It has been tested under Linux, Mac OS X, and SGI IRIX.

### Invoking dumpOSC

The command-line argument to dumpOSC is the port number (UDP or Unix) to
which dumpOSC should listen. For example, the following invocation
causes dumpOSC to listen to port 7123:

`dumpOSC 7123`

You can also invoke dumpOSC with the -showbytes flag, which must come
after the port number. [See below](#showbytes) for what this does.

### Interpreting dumpOSC's output

The output of dumpOSC is the inverse of the input to
[sendOSC](clients/sendOSC.html). Here is some sample text that dumpOSC
might print:

    [ 1
    /voices/0/tm/rate 0.900000
    /voices/3/tpe/tpe_points -4.000000 15.000000 4.000000 8.120000 10.000000 5.000000
    /tplibrary/load "foo.fmt"
    ]
    [ 1
    /voices/0/tm/goto 0.560000
    ]

The square brackets delimit the messages that were received in the same
packet, grouped by the OpenSound Control "#bundle" mechanism. In the
above example, the two pairs of square brackets indicate that two
packets were received, the first containing three messages and the
second containing only one. After the open square bracket comes the time
tag associated with the bundle, printed in hexadecimal. Both of these
bundles have a time tag of 1, meaning "do it immediately".

dumpOSC prints each message on its own line, starting with the message
address and name and continuing with all of the arguments to the
message.

In the above example, the first message received was
"/voices/0/tm/rate", with a single floating point argument 0.9. The
second message received was "/voices/3/tpe/tpe_points", with six
floating point arguments. The third message, "/tplibrary/load", had a
single string argument. String arguments are always delimited by
double-quote characters.

Packets that are not bundles (i.e., packets that consist of a single
message) are printed on a single line, with no brackets. Nested bundles
are printed with nested brackets.

### Message Argument Type-guessing Heuristics

The OpenSound Control format now includes type tags that indicate how to
interpret each of the arguments as a floating point number, string,
integer, etc. The dumpOSC program reads these type tags and correctly
interprets argument types.

Early implementations of OpenSound Control allowed messages that did not
contain type tags. The idea was that the sender of a message had to know
which messages required which argument types, and the receiving
application just coerced the binary argument data to the expected types.
This caused lots of problems and is strongly discouraged.

Nevertheless, dumpOSC can receive non-type-tagged messages; when this
happens it uses the following heuristics to "guess" what the types of
the arguments were supposed to be:

1) If a given argument word represents an integer between -1000 and
1000000 (inclusive), it's interpreted (i.e., printed) as an integer.

2) Otherwise, if a given argument word represents a floating point
number between -1000.0f and 0.f or between 0.000001f and 1000000.0f,
it's interpreted as a float. (Note that all of the integers from 0 to
1000000000 represent floating point numbers between 0. and 0.005, so any
reasonably small nonnegative integer value will also appear to be a very
small floating point value. Also, the first four characters of strings
like "/usr" and "/home" have bit representations that look like very
small positive floating point numbers.)

3) Otherwise, if a given argument word is the beginning of a valid
OpenSound Control-formatted string, all of the bytes of that string are
interpreted as a string. A valid string is a sequence of characters that
satisfy the isprint() test (from ctype.h) ending with a null character
and enough extra null characters to align the string on a 4-byte
boundary.

4) As a last resort, if a given argument word satisfies none of these
criteria, it's printed in hex, followed by a question mark, e.g.,
"0xDE23C860(?)".

Obviously, these heuristics are just heuristics. Floats between 0. and
0.00001 will be interpreted as integers or strings. The string "@"
will be interpreted as the floating point number 2.000. But for most
reasonable data, these heuristics work fine.

### []{#showbytes}The -showbytes Option

The -showbytes option to dumpOSC (which can appear on the command line
after the port number) runs dumpOSC in a very verbose mode where it
prints every single byte of every message it receives. This is mainly
useful for low-level debugging of clients when you fear that they
aren't even generating legal OpenSound Control packets.

Here's how dumpOSC would print the first of the above two packets if
you invoked it with the -showbytes flag:

    128 byte message:
     23 (#)  62 (b)  75 (u)  6e (n)
     64 (d)  6c (l)  65 (e)  0 ()
     0 ()    0 ()    0 ()    0 ()
     0 ()    0 ()    0 ()    1 ()
     0 ()    0 ()    0 ()    18 ()
     2f (/)  76 (v)  6f (o)  69 (i)
     63 (c)  65 (e)  73 (s)  2f (/)
     30 (0)  2f (/)  74 (t)  6d (m)
     2f (/)  72 (r)  61 (a)  74 (t)
     65 (e)  0 ()    0 ()    0 ()
     3f (?)  66 (f)  66 (f)  66 (f)
     0 ()    0 ()    0 ()    34 (4)
     2f (/)  76 (v)  6f (o)  69 (i)
     63 (c)  65 (e)  73 (s)  2f (/)
     33 (3)  2f (/)  74 (t)  70 (p)
     65 (e)  2f (/)  74 (t)  70 (p)
     65 (e)  5f (_)  70 (p)  6f (o)
     69 (i)  6e (n)  74 (t)  73 (s)
     0 ()    0 ()    0 ()    0 ()
     c0 (¿)  80 ()   0 ()    0 ()
     41 (A)  70 (p)  0 ()    0 ()
     40 (@)  80 ()   0 ()    0 ()
     41 (A)  1 ()    eb (Î)  85 (
    )
     41 (A)  20 ( )  0 ()    0 ()
     40 (@)  a0 (Ý)  0 ()    0 ()
     0 ()    0 ()    0 ()    18 ()
     2f (/)  74 (t)  70 (p)  6c (l)
     69 (i)  62 (b)  72 (r)  61 (a)
     72 (r)  79 (y)  2f (/)  6c (l)
     6f (o)  61 (a)  64 (d)  0 ()
     66 (f)  6f (o)  6f (o)  2e (.)
     66 (f)  6d (m)  74 (t)  0 ()

    [ 1
    /voices/0/tm/rate 0.900000
    /voices/3/tpe/tpe_points -4.000000 15.000000 4.000000 8.120000 10.000000 5.00000
    0
    /tplibrary/load "foo.fmt"
    ]

As you can see, dumpOSC prints the byte-by-byte dump first, and then
prints the packet in the usual way. The byte-by-byte dump prints four
bytes on each line, giving each byte's value in hex, and then printing
the byte as anASCII character in parentheses.

<hr/>

The text above was translated from the HTML retrieved on 2021-06-11:

<http://cnmat.org/OpenSoundControl/dumpOSC.html>
