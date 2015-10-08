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

      int one = 1;
      if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)
		     &one, sizeof(one)) < 0)
	{
	  perror("setsockopt");
	  abort();
	}

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

