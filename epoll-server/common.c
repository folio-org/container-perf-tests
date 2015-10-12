#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#include "common.h"

#define MAXEVENTS 64

int make_socket_non_blocking(int sfd)
{
  int flags, s;

  flags = fcntl(sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl(sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror("fcntl");
      return -1;
    }

  return 0;
}

int create_and_bind(char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int pass, s, sfd;

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

  for (pass = 0; pass < 2; pass++)
    for (rp = result; rp; rp = rp->ai_next)
      if ((pass == 0 && rp->ai_family == AF_INET6)
	  || (pass == 1 && rp->ai_family != AF_INET6))
	{
	  sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	  if (sfd != -1)
	    {
	      pass = 2;
	      break;
	    }
	}

  if (rp == NULL)
    {
      sfd = -1;
      abort();
    }
  else
    {
      int one = 1;
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)
		     &one, sizeof(one)) < 0)
	{
	  perror("setsockopt");
	  close(sfd);
	  abort();
	}
      if (bind(sfd, rp->ai_addr, rp->ai_addrlen))
	{
	  perror("bind");
	  close(sfd);
	  sfd = -1;
	  abort();
	}
    }
  freeaddrinfo(result);
  return sfd;
}

