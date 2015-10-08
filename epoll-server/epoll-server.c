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

int main(int argc, char *argv[])
{
  int sfd, s;
  int efd;
  int so_rcvbuf = 0;
  struct epoll_event event;
  struct epoll_event *events;
  struct edata {
    int fd;
    int content_length;
    int content_offset;
    char *header_buf;
    int header_off;
    int header_max;
  };

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

  s = make_socket_non_blocking(sfd);
  if (s == -1)
    abort();

  s = listen(sfd, SOMAXCONN);
  if (s == -1)
    {
      perror("listen");
      abort();
    }

  efd = epoll_create1(0);
  if (efd == -1)
    {
      perror("epoll_create");
      abort();
    }

  struct edata *ed = malloc(sizeof(*ed));
  ed->fd = sfd;
  ed->header_buf = 0;

  event.data.ptr = ed;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
  if (s == -1)
    {
      perror("epoll_ctl");
      abort();
    }

  /* Buffer where events are returned */
  events = calloc(MAXEVENTS, sizeof event);

  /* The event loop */
  while (1)
    {
      int n, i;

      n = epoll_wait(efd, events, MAXEVENTS, -1);
      for (i = 0; i < n; i++)
	{
	  struct edata *ed = events[i].data.ptr;
	  int fd = ed->fd;

	  if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
	    {
	      fprintf(stderr, "epoll error\n");

	      s = epoll_ctl(efd, EPOLL_CTL_DEL, fd, 0);
	      if (s == -1)
		{
		  perror("epoll_ctl");
		  abort();
		}
	      close(fd);
	      free(ed);
	      continue;
	    }

	  else if (sfd == fd)
	    {
              while (1)
                {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  in_len = sizeof in_addr;
                  infd = accept(sfd, &in_addr, &in_len);
                  if (infd == -1)
                    {
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                        {
                          break;
                        }
                      else
                        {
                          perror("accept");
                          break;
                        }
                    }

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
                  s = make_socket_non_blocking(infd);
                  if (s == -1)
                    abort();
		  ed = malloc(sizeof(*ed));
		  ed->fd = infd;
		  ed->content_length = -1;
		  ed->header_max = 999;
		  ed->header_buf = malloc(ed->header_max + 1);
		  ed->header_off = 0;
		  event.data.ptr = ed;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                    {
                      perror("epoll_ctl");
                      abort();
                    }
                }
              continue;
            }
          else
            {
              int done = 0;
              while (1)
                {
                  ssize_t count;
                  char buf[32768];

		  if (ed->content_length == -1)
		    {
		      count = read(fd, ed->header_buf + ed->header_off,
				   ed->header_max - ed->header_off);

		    }
		  else
		    {
		      count = read(fd, buf, sizeof buf);
		    }
                  if (count == -1)
                    {
                      if (errno != EAGAIN)
                        {
                          perror("read");
                          done = 1;
                        }
                      break;
                    }
                  else if (count == 0)
                    {
                      done = 1;
                      break;
                    }
		  else
		    {
		      if (ed->content_length == -1)
			{
			  const char *cp2;
			  ed->header_off += count;
			  ed->header_buf[ed->header_off] = '\0';
			  cp2 = strstr(ed->header_buf, "\r\n\r\n");
			  if (cp2)
			    cp2 += 4;
			  else if (!cp2)
			    {
			      cp2 = strstr(ed->header_buf, "\n\n");
			      if (cp2)
				cp2 += 2;
			    }
			  if (cp2) /* complete header? */
			    {
			      const char *cp1 =
				strstr(ed->header_buf, "\nContent-Length:");
			      if (cp1 && cp1 < cp2)
				ed->content_length = atoi(cp1 + 16);
			      else
				ed->content_length = 0;
			      ed->content_offset = ed->header_off -
				(cp2 - ed->header_buf);
			      printf("complete header length=%d\n",
				     ed->content_length);
			    }
			  else
			    {
			      if (ed->header_off == ed->header_max)
				{
				  printf("Too big header\n");
				  done = 1;
				}
			    }
			}
		      else
			{
			  ed->content_offset += count;
			}
		      if (ed->content_length != -1 &&
			  ed->content_offset == ed->content_length)
			{
			  printf("Got all content %d\n",
				 ed->content_length);
			  done = 1;
			}
		    }
                }

              if (done)
                {
                  printf("Closing descriptor %d\n", fd);
		  char buf[1000];

		  strcpy(buf, "HTTP/1.0 200 OK\r\n"
			 "Content-Length: 0\r\n"
			 "Connection: Close\r\n"
			 "\r\n\r\n");
		  write(fd, buf, strlen(buf));

                  s = epoll_ctl(efd, EPOLL_CTL_DEL, fd, 0);
                  if (s == -1)
                    {
                      perror("epoll_ctl");
                      abort();
                    }
                  close(fd);
		  free(ed->header_buf);
		  free(ed);
                }
            }
        }
    }

  free(events);

  close(sfd);

  return EXIT_SUCCESS;
}
