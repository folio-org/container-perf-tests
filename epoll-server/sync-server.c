#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAXEVENTS 64

static int create_and_bind(char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset(&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo(NULL, port, &hints, &result);
  if (s != 0)
    {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
          break;
      close(sfd);
    }

  if (rp == NULL)
    {
      fprintf(stderr, "Could not bind\n");
      return -1;
    }

  freeaddrinfo(result);

  return sfd;
}

int main(int argc, char *argv[])
{
  int sfd, s;
  int so_rcvbuf = 0;

  if (argc < 2 || argc > 3)
    {
      fprintf(stderr, "Usage: %s [port] [rcvbuf]\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  sfd = create_and_bind(argv[1]);
  if (sfd == -1)
    abort();

  if (argc == 3)
      so_rcvbuf = atoi(argv[2]);

  int one = 1;
  if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)
		 &one, sizeof(one)) < 0)
    abort();
  s = listen(sfd, SOMAXCONN);
  if (s == -1)
    {
      perror("listen");
      abort();
    }

  struct sockaddr in_addr;
  socklen_t in_len;
  int infd;
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  in_len = sizeof in_addr;
  infd = accept(sfd, &in_addr, &in_len);
  if (infd == -1)
    {
      perror("accept");
      abort();
    }
  close(sfd);
  s = getnameinfo(&in_addr, in_len,
		  hbuf, sizeof hbuf,
		  sbuf, sizeof sbuf,
		  NI_NUMERICHOST | NI_NUMERICSERV);
  if (s == 0)
    {
      printf("Accepted connection on descriptor %d "
	     "(host=%s, port=%s)\n", infd, hbuf, sbuf);
    }
  if (so_rcvbuf)
    if (setsockopt(infd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf,
		   sizeof(so_rcvbuf)))
      {
	perror("setsocopt");
	abort();
      }
  int total_count = 0;
  int done = 0;
  while (!done)
    {
      ssize_t count;
      char buf[32768];
      count = read(infd, buf, sizeof buf);
      if (count == -1)
	{
	  if (errno != EAGAIN)
	    {
	      perror("read");
	      done = 1;
	    }
	}
      else if (count == 0)
	{
	  done = 1;
	}
      else
	{
	  total_count += count;
	}
    }
  printf("Closing descriptor %d\n", infd);
  close(infd);
  return EXIT_SUCCESS;
}
