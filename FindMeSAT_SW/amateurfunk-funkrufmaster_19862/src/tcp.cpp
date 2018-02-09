/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
 *                                                                          *
 * Thist Program is free software; yopu can redistribute ist and/or modify  *
 * it under the terms of the GNU General Public License as published by the *
 * Free Software Foundation; either version 2 of the License, or            *
 * (at your option) any later versions.                                     *
 *                                                                          *
 * This program is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRENTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General *
 * Public License for more details.                                         *
 *                                                                          *
 * You should have receved a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,  *
 * 675 Mass Ave, Cambridge, MA 02139, USA.                                  *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 * Author:                                                                  *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *                                                                          *
 * List of other authors:                                                   *
 *                                                                          *
 ****************************************************************************/

#include "tcp.h"
#include "iputil.h"
#include <fcntl.h>


/*----------------------------------------------------------*/
// 
// Hier beginnt nun die Programmierung des beiden TCP-Klassen
//
//
/*----------------------------------------------------------*/

tcp_client::tcp_client( void )
{
}

tcp_client::tcp_client( int fd, struct sockaddr_in ai )
{
  sockt = SOCK_TCP;
  sockfd = fd;
  cl_adrinfo = ai;
  status = CONNECTED;
}

tcp_client::tcp_client( const String &hostname, const String &service )
{
  struct addrinfo hints,*res, *ressave;

  bzero(&hints,sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;


  int n = getaddrinfo(strtochar(hostname),strtochar(service),&hints,&res);
  if (n != 0)
    throw Error_hostname_look_up();

  ressave = res;

  do 
    {
      sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
      if (sockfd < 0 )
	continue;

      if ( connect(sockfd, res->ai_addr, res->ai_addrlen) == 0 )
	break;

      close();
    }
  while ((res = res->ai_next) != NULL);

  if (res != NULL)
    cl_adrinfo = *((struct sockaddr_in*) (res->ai_addr));

  freeaddrinfo(ressave);

  if (res == NULL)
    {
      if (errno == ETIMEDOUT)
	{
	  throw Error_connection_timed_out();
	}
      else if (errno == ECONNREFUSED)
	{
	  throw Error_connection_refused();
	}
      else if (errno == EHOSTUNREACH) 
	{
	  throw Error_host_unreachable();
	}
      else if (errno == ENETUNREACH) 
	{
	  throw Error_net_unreachable();
	}
      else 
	{
	  throw Error_connect_failed();
	}
    }
  else
    {
      //memcpy(cl_adrinfo,res->ai_addr,res->ai_addrlen);
      status = CONNECTED;
    }
}

tcp_client::tcp_client( const String& hostname, const String &service, bool &connected )
{
  struct addrinfo hints,*res, *ressave;
  int n, flags = 0;

  bzero(&hints,sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  n = getaddrinfo(strtochar(hostname),strtochar(service),&hints,&res);
  if (n != 0)
    {
      throw Error_hostname_look_up();
    }

  ressave = res;

  do 
    {
      sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
      if (sockfd < 0 )
	continue;

      flags = fcntl(sockfd,F_GETFL,0);
      fcntl(sockfd,F_SETFL, flags | O_NONBLOCK );


      if ( connect(sockfd, res->ai_addr, res->ai_addrlen) == 0 )
        {
       	  break;
        }


      if (errno == EINPROGRESS) 
	{
	  fcntl(sockfd,F_SETFL, flags );
	  //memcpy(cl_adrinfo,res->ai_addr,res->ai_addrlen);
	  connected = false;
          break;
	}

      close();
    }
  while ((res = res->ai_next) != NULL);

  if (res != NULL)
    cl_adrinfo = *((struct sockaddr_in*) (res->ai_addr));

  freeaddrinfo(ressave);

  if (res == NULL)
    {
      if (errno == ETIMEDOUT)
	{
	  throw Error_connection_timed_out();
	}
      else if (errno == ECONNREFUSED)
	{
	  throw Error_connection_refused();
	}
      else if (errno == EHOSTUNREACH) 
	{
	  throw Error_host_unreachable();
	}
      else if (errno == ENETUNREACH) 
	{
	  throw Error_net_unreachable();
	}
      else 
	{
	  throw Error_connect_failed();
	}
    }
  else
    {
      fcntl(sockfd,F_SETFL, flags );
      //memcpy(cl_adrinfo,res->ai_addr,res->ai_addrlen);
      status = CONNECTED;
      connected = true;
    }
}


//------------------------------------------------------------------------
//
//
//
//
//
//
//------------------------------------------------------------------------


tcp_server::tcp_server( int port , int backlog)
{
  sockt = SOCK_TCP;
  struct sockaddr_in servaddr;

  if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0 )) < 0 )
    throw Error_could_not_gen_socket();

  bzero(&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if ( bind( listenfd, (SA *) &servaddr, sizeof( servaddr) ) < 0 )
    if (errno == EADDRINUSE )
      throw Error_address_in_use();
    else
      {
	throw Error_could_not_bind_socket(errno);
      }

  if ( listen( listenfd, backlog ) < 0 )
    throw Error_could_not_set_to_listen();

}

tcp_server::~tcp_server( void )
{
  close();
}

void tcp_server::do_accept( client* &cl_pointer )
{
  bool term_flag = false;
  int connfd;

  struct sockaddr_in cliaddr;  
  socklen_t clilen;
  cl_pointer = NULL;

  while( !term_flag)
    {
      clilen = sizeof( cliaddr);
      if ( (connfd = accept( listenfd, (SA * ) &cliaddr, &clilen )) < 0 )
	{
	  if ((errno != EINTR) && (errno != ECONNABORTED )) 
	    throw Error_while_accepting_connection();
	}
      else
	{
	  cl_pointer = new tcp_client(connfd,cliaddr);
	  term_flag = true;	  
	}
    }
}
