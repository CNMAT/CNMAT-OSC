## The "sendOSC" Program

Matt Wright, revised 3/3/3

The program "sendOSC" is a text-based [OpenSoundControl](index.html)
client. User can enter messages via command line arguments or standard
input; sendOSC formats these messages according to the
"OpenSoundControl" protocol, then sends the OpenSoundControl packet to
an OpenSoundControl server via UDP or Unix protocol.

### Availability

The "sendOSC" program is available as [source code](/OSC/src/sendOSC)
and as [compiled binaries for Mac OS X](/OSC/send%2BdumpOSC-OSX.tar.gz).
It has been tested under Linux, Mac OS X, and SGI IRIX.

### Selecting a host machine and port number

    usage: sendOSC [-h host] [-r] port_number [message...]

The "-h" flag indicates that the next argument is the name of the
target host. The "-r" flag instructs sendOSC to look at the
environment variable REMOTE_ADDR to get the name of the target host. The
host can be either an Internet host name (e.g., "les") or an Internet
address in standard dot notation (e.g., "128.32.122.13")

If a target host is given, sendOSC uses the UDP protocol. Otherwise, the
program uses the Unix protocol to connect to an OpenSoundControl server
on the same machine.

The next command argument must be the port number.

For example, this invocation of sendOSC causes it to try to connect to
host "rodet" on port 7009:

`sendOSC -h rodet 7009 `

This invocation asks sendOSC to connect to port 7005 using the UNIX
protocol:

`sendOSC 7005 `

This invocation asks sendOSC to connect to port 7022 of host
128.32.122.14:

`setenv REMOTE_ADDR 128.32.122.14 sendOSC -r 7022`

### Command-Line Mode

If there are any additional command-line arguments after the port
number, they're taken to be messages to be sent. In this mode, sendOSC
reads all of the command line arguments, translates them into a
OpenSoundControl packet, sends it in a single packet, and exits. sendOSC
does not print any error or warning messages in this mode, which makes
it a useful helper program for, e.g., CGI scripts.

Each command-line argument after the port number corresponds to an
entire OpenSoundControl message, according to this comma-delimited
format:

`message_name,arg1,arg2,arg3,arg4,... `

Each argument must be one of the following:

-   `integer: [-]?[0-9]+`
-   `float: [-]?([0-9]+.[0-9]*|.[0-9]+)`
-   `string: any other sequence of non-comma characters   `

The messages are combined into a single UDP packet and sent atomically.

Example: The following Unix command sends the messages
"/voices/0/tp/timbre_index 0", "/voices/0/tm/rate 1.0", and
"/voices/0/tm/goto 0.0" to a server on port 7003 of the current
machine:

`sendOSC 7003 /voices/0/tp/timbre_index,0 /voices/0/tm/rate,1.0 /voices/0/tm/goto,0.0`

Example: The following Unix command sends the messages
"/voices/1/tp/timbre_index 3" and "load_file
/usr/local/near/data/foo.format":

`sendOSC 7003 /voices/1/tp/timbre_index,3 load_file,/usr/local/near/data/foo.format`

Example: The following Unix command sends the message
"/voices/2/tp/tone_bank 200. -19. 300. -32. 400. -40.":

`sendOSC 7003 /voices/2/tp/tone_bank,200.,-19.,300.,-32.,400.,-40.`

Remember that the Unix shell will interpret any whitespace characters
(space, tab, etc.) as delimiting the arguments to sendOSC. So if you
want your string arguments to include any space characters, you must
quote them to the shell. For example, these equivalent commands send the
message "string_message" with the argument string "this string has
spaces":

`sendOSC 7003 "string_message,this string has spaces" sendOSC 7003 string_message,this\ string\ has\ spaces`

There are other characters which are special to Unix shells: "!",
"\$", ";", "\~", etc. Be careful.

### Interactive Mode

If the port number is the last command-line argument, sendOSC enters an
interactive mode. In this mode, lines of stdin should look like this:

`message_name arg1 arg2 arg3... `

Each line of stdin is converted into a one-message OpenSoundControl
packet and sent immediately.

Example:

`sendOSC 7003 message1 2.0 3.14159 message2 100 200 300 message3 string1 string_number_two /the/third/string message4 1 2.0 three`

In this interactive mode, any whitespace character is interpreted as
delimiting the arguments to a message. To include whitespace in a
string, surround the entire string with double-quote characters.
Example:

`sendOSC 7003 stringmessage "This string has spaces in it"`\
To exit interactive mode and return to the shell, type an EOF
(control-D) or interrupt (control-C). The sendOSC program will also quit
if it is unable to send a packet, i.e., if you don't have a server
listening on the machine and port that sendOSC is trying to send to.

#### Constructing Multiple-Message Bundles

To bundle multiple messages into the same packet while in interactive
mode, begin by entering an open bracket character ("[") by itself on
a line. Once you've done this, all of the lines you enter will be
accumulated into a large OpenSoundControl bundle until you enter a close
bracket character ("]") by itself on a line, at which point the
entire packet will be sent.

Example:

`sendOSC 7003 [ /voices/0/tp/timbre_index 7.0 /voices/0/tm/rate 1.0 /voices/0/tm/goto 0.0 ]`

These bracketed bundles can be nested.

#### Blank Lines

If you enter a blank line in the middle of constructing a message group
with [ and ], it is ignored. Otherwise, if you enter a blank line,
sendOSC re-sends the previous packet, which may be a single message or a
group of messages.

#### The "play" Hack

To save typing, if you enter "play" by itself on an empty line, it's
equivalent to the following:

    [
    /voices/0/tp/timbre_index 0
    /voices/0/tm/goto 0.0
    ]

#### Sending Time Tags

The OpenSound Control protocol includes space for a 8-byte time tag in
each bundle. By default, sendOSC uses the value 1, meaning "do it
immediately" as the time tag for every bundle. But you can put a time
tag into a bundle by typing some stuff after the open bracket character
that opens the bundle. There are two ways to do this.

If you have a hexadecimal number after the open bracket character, it
will be used as the time tag. Since OSC time tags are 8 bytes, you can
have up to 16 hex digits.

If you have a plus sign and then a (decimal) number after the open
bracket character, the number will be interpreted as a number of seconds
into the future, which will be added to the computer's notion of the
current time to produce the time tag. (This feature is currently
available only on the SGI version of sendOSC.)

### Exit Status Codes

sendOSC sets the Unix exit status to indicate whether or not it was
successful. In interactive mode, sendOSC also prints warning and error
messages to the screen, so the exit status is not so crucial. In command
line mode, however, the exit status is the only way to know if sendOSC
could do its job.

Here are the possible exit status codes:

0: Successful

2: Message(s) were dropped because of buffer overflow.

3: Socket error

4: Usage error: incorrect command-line arguments, or a "-r" without
REMOTE_ADDR defined.

5: Internal error: this should never occur.

<hr/>

The text above was translated from the HTML retrieved on 2021-06-11:

<http://cnmat.org/OpenSoundControl/clients/sendOSC.html>
