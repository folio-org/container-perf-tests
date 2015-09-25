# Postverticle

Small program to demonstrate slowness of HTTP POST on some platforms
for Vert.x.

We don't know if the problem is in our program, Vert.x, netty or lower down.

We have seen problem on Debian jessie - x64 and Ubuntu 15.04. Posting of
a 10 MB file takes 1-30 seconds on these systems. It varies greatly
depending on how rapid the HTTP Client sends data to the server.

The problem does not seem to be present on Debian wheezy or CentOS 6.
On the slowest platform we've tried - a Celeron 420 server running Debian
Wheezy - posting a 10 MB file is performed in less than 0.5 second.
Most modern hardware will do it at least 10 times faster than that.

## Build and run:

    mvn install
    java -jar target/postverticle-fat.jar

The program will listen on 8080. Modify as necessary.

## Demonstrating the problem

prepare a file

    dd if=/dev/zero of=10m count=10 bs=1048576

Posting a file of size 10MB should take < 0.1 seconds on modern
hardware and any web server.

    wget --post-file=10m http://localhost:8080

or

    yaz-url -p10m http://localhost:8080

or

    netcat - see netcat-client/client.sh

vert.x-3.0.0 uses netty 4.0.28 by default. We've tried with netty
4.0.29, 4.0.30, 4.0.31 with similar results (apparent slowness on some
platforms).

## Analyzing the behavior with strace

With:

    strace -o s.log -ff -T -tt java -java target/postverticle-fat.jar

We'll get a per process/thread split of what's going on - including
completion time for sytem calls. We have included two versions -
a fast one (strace/jessie.strace.fast.log) - and a slow one
(strace/jessie.strace.slow.log). Both are quite slow - compared to
what we'd expect.

The fast log was produced with:

    curl --data-binary @10m http://localhost:8080

The fast one completes in 1-2 seconds (still too slow).

The slow one was produced with:

    wget --post-file=10m  http://localhost:8080

The slow one completes in 20-30 seconds.

Let's watch the net socket 74 and epoll calls:

    egrep '(read\(74,)|epoll' jessie.strace.slow.log |less

Log from point of setting up epoll FDs and leading POST.

    13:39:24.528979 epoll_wait(24, {{EPOLLIN, {u32=73, u64=9194210617664208969}}}, 8192, 1000) = 1 <0.774085>
    13:39:25.357812 epoll_ctl(24, EPOLL_CTL_ADD, 74, {EPOLLIN, {u32=74, u64=3853375938494538}}) = 0 <0.000023>
    13:39:25.357889 epoll_ctl(24, EPOLL_CTL_MOD, 74, {EPOLLIN, {u32=74, u64=3853375938494538}}) = 0 <0.000015>
    13:39:25.357949 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 171) = 1 <0.000017>
    13:39:25.378237 read(74, "POST / HTTP/1.1\r\nUser-Agent: Wge"..., 43690) = 43690 <0.000058>
    13:39:25.409216 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000022>
    13:39:25.409502 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000019>
    13:39:25.409761 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 190 <0.000007>
    13:39:25.410298 epoll_wait(24, {}, 8192, 119) = 0 <0.119188>
    13:39:25.529760 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 1000) = 1 <0.019910>
    13:39:25.549883 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000094>
    13:39:25.550936 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 9942 <0.000253>
    13:39:25.551674 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 978) = 1 <0.000051>
    13:39:25.551876 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 9024 <0.000048>
    13:39:25.552131 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 978) = 1 <0.205514>
    13:39:25.757859 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000094>
    13:39:25.758798 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 86 <0.000126>
    13:39:25.759145 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 771) = 1 <0.000019>
    13:39:25.759313 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 18880 <0.000076>
    13:39:25.759808 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 770) = 1 <0.201836>

Some epoll_wait calls are slow: 0.20 seconds. They appear all along the way - making the whole operation take around 20 seconds.

Here's the faster one with a total response time of about a second.
It has a call to epoll_wait that takes 0.35 seconds to complete.

    13:37:34.778689 epoll_wait(24, {{EPOLLIN, {u32=73, u64=8135600822441476169}}}, 8192, 1000) = 1 <0.352702>
    13:37:35.182710 epoll_ctl(24, EPOLL_CTL_ADD, 74, {EPOLLIN, {u32=74, u64=3853375938494538}}) = 0 <0.000024>
    13:37:35.182793 epoll_ctl(24, EPOLL_CTL_MOD, 74, {EPOLLIN, {u32=74, u64=3853375938494538}}) = 0 <0.000017>
    13:37:35.182856 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 596) = 1 <0.000018>
    13:37:35.215577 read(74, "POST / HTTP/1.1\r\nUser-Agent: cur"..., 43690) = 176 <0.000024>
    13:37:35.255981 epoll_wait(24, {}, 8192, 523) = 0 <0.523570>
    13:37:35.779700 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 1000) = 1 <0.352943>
    13:37:36.132878 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000065>
    13:37:36.138220 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000072>
    13:37:36.138798 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 22956 <0.000053>
    13:37:36.139181 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 641) = 1 <0.000022>
    13:37:36.139329 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 22400 <0.000046>
    13:37:36.139873 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 640) = 1 <0.000027>
    13:37:36.140063 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 22400 <0.000056>
    13:37:36.140519 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 639) = 1 <0.000019>
    13:37:36.140671 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 22400 <0.000041>
    13:37:36.140955 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 639) = 1 <0.000021>
    13:37:36.141120 read(74, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 22400 <0.000055>
    13:37:36.141583 epoll_wait(24, {{EPOLLIN, {u32=74, u64=3853375938494538}}}, 8192, 638) = 1 <0.000020>

On Debian wheezy or CentOS 6 response times are acceptable. Here's a snippet from
a centos6 server where everything works in order, it seems:

    10:54:28.822708 epoll_wait(13, {{EPOLLIN, {u32=38, u64=16040768918511419430}}}, 4096, 1000) = 1 <0.498378>
    10:54:29.549955 epoll_ctl(13, EPOLL_CTL_ADD, 41, {EPOLLIN, {u32=41, u64=547212093737140265}}) = 0 <0.000016>
    10:54:29.550002 epoll_ctl(13, EPOLL_CTL_MOD, 41, {EPOLLIN, {u32=41, u64=547212093737140265}}) = 0 <0.000012>
    10:54:29.550076 epoll_wait(13, {{EPOLLIN, {u32=41, u64=547212093737140265}}}, 4096, 1000) = 1 <0.000013>
    10:54:29.632812 read(41, "POST / HTTP/1.1\r\nContent-Length:"..., 43690) = 43690 <0.000057>
    10:54:29.787827 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000056>
    10:54:29.788635 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000039>
    10:54:29.789591 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000076>
    10:54:29.791345 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000054>
    10:54:29.792414 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000035>
    10:54:29.794223 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000092>
    10:54:29.796629 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000056>
    10:54:29.798486 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000056>
    10:54:29.799252 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000036>
    10:54:29.800440 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000068>
    10:54:29.803150 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000072>
    10:54:29.805051 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000054>
    10:54:29.806388 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000055>
    10:54:29.808854 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000083>
    10:54:29.810404 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000053>
    10:54:29.815803 epoll_wait(13, {{EPOLLIN, {u32=41, u64=547212093737140265}}}, 4096, 1000) = 1 <0.000014>
    10:54:29.815937 read(41, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 43690) = 43690 <0.000035>

The read calls here are not "cut-off".. calling epoll_wait is OK because it's in level triggered.
