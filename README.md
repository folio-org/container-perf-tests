# Postverticle

Small program to demonstrate slowness of HTTP POST on some platforms
for Vert.x.

We don't know if the problem is in our program, Vert.x, netty or lower down.

We have seen problem on Debian jessie - x64 and Ubuntu 15.04.
The problem happens for clients that sends data fast to server.
For some reason "fast" clients will make the server take 10 times
longer to just receive the posted content.

The problem has not been demonstrated on Debian wheezy.

We've seen the problem with wget, netcat and yaz-url (our own).
However, curl does not seem to suffer from this. We believe it's due
to slightly slower "wait" time between first and 2nd net write (order
of 1 ms).

## Build and run:

mvn install
java -jar target/postverticle-fat.jar

The program will listen on 8080. Modify as necessary.

## Demonstrating the problem

prepare a file
dd if=/dev/zero of=10m count=10 size=1048576

Posting a file of size 10MB should take < 2 seconds on any web server.
If things fail it will take at least 10 times longer.

wget --postfile=10m http://localhost:8080
or
yaz-url -p10m http://localhost:8080
or
netcat - see netcat-client/client.sh

vert.x-3.0.0 uses netty 4.0.28 by default. We've tried with netty
4.0.29, 4.0.30, 4.0.31 with similar results (apparent slowness on some
platforms).
