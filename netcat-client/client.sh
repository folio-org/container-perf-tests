#!/bin/sh
s="$1"
if test -z "$s"; then
	s=10
fi
b=`expr $s '*' 1048576`
/bin/echo -n -e "POST / HTTP/1.1\r\nContent-Length: $b\r\n\r\n" >input


dd if=/dev/zero of=input bs=1048576 count=$s oflag=append conv=notrunc
# sleep 3 && kill -s 14 $$ &
nc localhost 8080 < input
# kill %1

