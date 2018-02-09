
#include "iputil.h"

#define bzero(ptr,n)      memset(ptr,0,n)
#define bcopy(src,dest,n) memcpy(dest,src,n)
#define bcmp(ptr1,ptr2,n) memcmp(ptr1,ptr2,n)


char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
  char	      portstr[7];
  static char str[128];		/* Unix domain is largest */

  switch (sa->sa_family) 
    {
      case AF_INET: {
	              struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		      if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		      if (ntohs(sin->sin_port) != 0) 
			{
			  snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
			  strcat(str, portstr);
			}
		      return(str);
                    }
      /* end sock_ntop */
      
#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		       if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			 return(NULL);
		       if (ntohs(sin6->sin6_port) != 0) 
			 {
			   snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin6->sin6_port));
			   strcat(str, portstr);
			 }
		       return(str);
                     }
#endif
        
      //#ifdef	AF_UNIX
      //case AF_UNIX: {
      //     	      struct sockaddr_un	*unp = (struct sockaddr_un *) sa;
      //	      
      //	      /* OK to have no pathname bound to the socket: happens on
      //		 every connect() unless client calls bind() first. */
      //	      if (unp->sun_path[0] == 0)
      //		strcpy(str, "(no pathname bound)");
      //	      else
      //		snprintf(str, sizeof(str), "%s", unp->sun_path);
      //	      return(str);
      //            }
      //#endif
      
      //#ifdef	HAVE_SOCKADDR_DL_STRUCT
      //case AF_LINK: {
      //	      struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;
      //	      
      //	      if (sdl->sdl_nlen > 0)
      //		snprintf(str, sizeof(str), "%*s",
      //			 sdl->sdl_nlen, &sdl->sdl_data[0]);
      //	      else
      //		snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
      //	      return(str);
      //            }
      //#endif

           default: snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
			     sa->sa_family, salen);
                    return(str);
    }
  return (NULL);
}


int sock_bind_wild(int sockfd, int family)
{
  socklen_t	len;
  
  switch (family) 
    {
      case AF_INET: {
	 	      struct sockaddr_in	sin;

		      bzero(&sin, sizeof(sin));
		      sin.sin_family = AF_INET;
		      sin.sin_addr.s_addr = htonl(INADDR_ANY);
		      sin.sin_port = htons(0);	/* bind ephemeral port */

		      if (bind(sockfd, (SA *) &sin, sizeof(sin)) < 0)
			return(-1);
		      len = sizeof(sin);
		      if (getsockname(sockfd, (SA *) &sin, &len) < 0)
			return(-1);
		      return(sin.sin_port);
	            }

#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	sin6;

		       bzero(&sin6, sizeof(sin6));
		       sin6.sin6_family = AF_INET6;
		       sin6.sin6_addr = in6addr_any;
		       sin6.sin6_port = htons(0);	/* bind ephemeral port */

		       if (bind(sockfd, (SA *) &sin6, sizeof(sin6)) < 0)
			 return(-1);
		       len = sizeof(sin6);
		       if (getsockname(sockfd, (SA *) &sin6, &len) < 0)
			 return(-1);
		       return(sin6.sin6_port);
	             }
#endif
    }
  return(-1);
}

int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2,
		  socklen_t salen)
{
  if (sa1->sa_family != sa2->sa_family)
    return(-1);
  
  switch (sa1->sa_family) 
    {
      case AF_INET: {
	 	      return(memcmp( &((struct sockaddr_in *) sa1)->sin_addr,
				     &((struct sockaddr_in *) sa2)->sin_addr,
				     sizeof(struct in_addr)));
	            }
      
#ifdef	IPV6
      case AF_INET6: {
		       return(memcmp( &((struct sockaddr_in6 *) sa1)->sin6_addr,
				      &((struct sockaddr_in6 *) sa2)->sin6_addr,
				      sizeof(struct in6_addr)));
                     }
#endif
      /*      
#ifdef	AF_UNIX
      case AF_UNIX: {
		      return(strcmp( ((struct sockaddr_un *) sa1)->sun_path,
				     ((struct sockaddr_un *) sa2)->sun_path));
                    }
#endif
     
      //#ifdef	HAVE_SOCKADDR_DL_STRUCT
      //case AF_LINK: {
      //		      return(-1);		// no idea what to compare here ?
      //             }
      //#endif
      */
    }
  return (-1);
}

int sock_cmp_port(const struct sockaddr *sa1, const struct sockaddr *sa2,
		  socklen_t salen)
{
  if (sa1->sa_family != sa2->sa_family)
    return(-1);
  
  switch (sa1->sa_family) 
    {
      case AF_INET: {
	              return( ((struct sockaddr_in *) sa1)->sin_port ==
			      ((struct sockaddr_in *) sa2)->sin_port);
	            }
      
#ifdef	IPV6
      case AF_INET6: {
	 	       return( ((struct sockaddr_in6 *) sa1)->sin6_port ==
			       ((struct sockaddr_in6 *) sa2)->sin6_port);
                     }
#endif
      
    }
  return (-1);
}

int sock_get_port(const struct sockaddr *sa, socklen_t salen)
{
  switch (sa->sa_family) 
    {
      case AF_INET: {
		      struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
		      
		      return(sin->sin_port);
                    }
      
#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		       return(sin6->sin6_port);
	             }
#endif
    }
  
  return(-1);		/* ??? */
}

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
  static char str[128];		/* Unix domain is largest */
  
  switch (sa->sa_family) 
    {
      case AF_INET: {
		      struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		      if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		      return(str);
                    }
      
#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
		       
		       if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			 return(NULL);
		       return(str);
                     }
#endif
      
      //#ifdef	AF_UNIX
      //case AF_UNIX: {
      //	      struct sockaddr_un	*unp = (struct sockaddr_un *) sa;
      //	      
      //	      /* OK to have no pathname bound to the socket: happens on
      //		 every connect() unless client calls bind() first. */
      //	      if (unp->sun_path[0] == 0)
      //		strcpy(str, "(no pathname bound)");
      //	      else
      //		snprintf(str, sizeof(str), "%s", unp->sun_path);
      //	      return(str);
      //            }
      //#endif
      //
      //#ifdef	HAVE_SOCKADDR_DL_STRUCT
      //case AF_LINK: {
      //	      struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;
      //	      
      //	      if (sdl->sdl_nlen > 0)
      //		snprintf(str, sizeof(str), "%*s",
      //			 sdl->sdl_nlen, &sdl->sdl_data[0]);
      //	      else
      //		snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
      //	      return(str);
      //            }
      //#endif

      default       : snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",
			       sa->sa_family, salen);
                      return(str);
    }
  return (NULL);
}


void sock_set_addr(struct sockaddr *sa, socklen_t salen, const void *addr)
{
  switch (sa->sa_family) 
    {
      case AF_INET: {
		      struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		      memcpy(&sin->sin_addr, addr, sizeof(struct in_addr));
		      break;
                    }

#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
		       
		       memcpy(&sin6->sin6_addr, addr, sizeof(struct in6_addr));
		       break;
                     }
#endif
    }
  
}

void sock_set_port(struct sockaddr *sa, socklen_t salen, int port)
{
  switch (sa->sa_family) 
    {
      case AF_INET: {
		      struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		      sin->sin_port = port;
		      break;
                    }
      
#ifdef	IPV6
      case AF_INET6: {
		       struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
		       
		       sin6->sin6_port = port;
		       break;
                     }
#endif
    }
}

void sock_set_wild(struct sockaddr *sa, socklen_t salen)
{
  const void *wildptr = NULL;
  
  switch (sa->sa_family) 
    {
      case AF_INET: {
		      static struct in_addr	in4addr_any;
		      
		      in4addr_any.s_addr = htonl(INADDR_ANY);
		      wildptr = &in4addr_any;
		      break;
	            }
      
#ifdef	IPV6
      case AF_INET6: {
		       wildptr = &in6addr_any;
		       break;
                     }
#endif
    }
  sock_set_addr(sa, salen, wildptr);
}

ssize_t	readn(int fd, char *vptr, size_t n)  /* Read "n" bytes from a descriptor. */
{
  size_t	nleft;
  ssize_t	nread;
  char	*ptr;
  
  ptr = vptr;
  nleft = n;
  while (nleft > 0) 
    {
      if ( (nread = read(fd, ptr, nleft)) < 0) 
	{
	  if (errno == EINTR)
	    nread = 0;		/* and call read() again */
	  else
	    return(-1);
	} 
      else 
	if (nread == 0)
	  break;				/* EOF */
      
      nleft -= nread;
      ptr   += nread;
    }
  return(n - nleft);		/* return >= 0 */
}
/* end readn */

ssize_t	writen(int fd, const char *vptr, size_t n) /* Write "n" bytes to a descriptor. */
{
  size_t		nleft;
  size_t 		q;
  ssize_t		nwritten;
  const char	*ptr;
  
  ptr = vptr;
  nleft = n;

  while (nleft > 0) 
    {
      if (nleft>MTU_SIZE) q=MTU_SIZE;	// Wenn n_left > MTU-Size
      else q=nleft;
      if ( (nwritten = write(fd, ptr, q)) <= 0) 
	{
	  if (errno == EINTR)
	    nwritten = 0;		/* and call write() again */
	  else if (errno == EPIPE)
	    return -2;
	  else
	    return(-1);			/* error */
	}  
      nleft -= nwritten;
      ptr   += nwritten;
    } 
  return(n);
}
/* end writen */

static ssize_t my_read(int fd, char *ptr)
{
  static int	read_cnt = 0;
  static char	*read_ptr;
  static char	read_buf[MAXLINE];
  
  if (read_cnt <= 0) 
    {
      again: if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) 
               {
		 if (errno == EINTR)
		   goto again;
		 return(-1);
		} 

      else if (read_cnt == 0)
	return(0);
      read_ptr = read_buf;
    }
  
  read_cnt--;
  *ptr = *read_ptr++;
  return(1);
}

ssize_t readline(int fd, char *vptr, size_t maxlen)
{
  size_t n;
  int	rc;
  char	c, *ptr;
  
  ptr = vptr;
  for (n = 1; n < maxlen; n++) 
    {
      if ( (rc = my_read(fd, &c)) == 1) 
	{
	  *ptr++ = c;
	  if (c == '\n' || c == '\r')
	    break;	/* newline is stored, like fgets() */
	} 
      else if (rc == 0) 
	{
	  if (n == 1)
	    return(0);	/* EOF, no data read */
	  else
	    break;		/* EOF, some data was read */
	} 
      else
	return(-1);		/* error, errno set by read() */
    }

  *ptr = 0;	/* null terminate like fgets() */
  return(n);
}

/* end readline */


#ifndef	S_IFSOCK
#error S_IFSOCK not defined
#endif

int isfdtype(int fd, unsigned int fdtype)
{
  struct stat	buf;
  
  if (fstat(fd, &buf) < 0)
    return(-1);
  
  if ((buf.st_mode & S_IFMT) == fdtype)
    return(1);
  else
    return(0);
}
