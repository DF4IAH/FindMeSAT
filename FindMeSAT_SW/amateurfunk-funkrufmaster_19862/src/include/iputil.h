#ifndef __IPUTIL_H__
#define __IPUTIL_H__

#include "globdef.h"
// Hier werden die Funktionsheader einer Reihe von Funktionen 
// deklariert, die fuer die IP-Klassen benoetigt werden.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <netdb.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#define	SA	struct sockaddr

#define MAXLINE 4096

char *sock_ntop(const struct sockaddr *sa, socklen_t salen);
int sock_bind_wild(int sockfd, int family);
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2,
		  socklen_t salen);
int sock_cmp_port(const struct sockaddr *sa1, const struct sockaddr *sa2,
		  socklen_t salen);
int sock_get_port(const struct sockaddr *sa, socklen_t salen);
char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen);
void sock_set_addr(struct sockaddr *sa, socklen_t salen, const void *addr);
void sock_set_port(struct sockaddr *sa, socklen_t salen, int port);
void sock_set_wild(struct sockaddr *sa, socklen_t salen);
ssize_t	readn(int fd, char *vptr, size_t n);
ssize_t	writen(int fd, const char *vptr, size_t n);
ssize_t readline(int fd, char *vptr, size_t maxlen);
int isfdtype(int fd, unsigned int fdtype);


#endif
