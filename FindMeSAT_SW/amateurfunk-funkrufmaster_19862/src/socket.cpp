/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Holger Flemming                                    *
 *                                                                          *
 * This Program is free software; you can redistribute ist and/or modify    *
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


#include "socket.h"
#include "iputil.h"
#include <netinet/in.h>
#include <unistd.h>

inline int s_close(int fd )
{
  return close(fd);
}

inline int s_shutdown( int fd, int how)
{
  return shutdown(fd,how);
}

client::client( void )
{
  sockfd = 0;
  sockt = SOCK_UNDEF;
  status = UNCONNECTED;
}

client::~client( void )
{
  close(  );
}

void client::get( char &ch)
{
  if ( readn( sockfd,  &ch, 1 ) < 0 )
    throw Error_while_reading_socket();
}

void client::get( String & str, int n_max )
{
  if (!str.socket_read(sockfd,n_max))
    throw  Error_while_reading_socket();
}

void client::getline ( String  &str, int n_max )
{
  if (!str.socket_readln( sockfd,  n_max ))
    throw  Error_while_reading_socket();
}

void client::put( char ch)
{
  if (writen( sockfd,  &ch, 1 ) < 0 )
    throw Error_while_writing_socket();
}

void client::put( const String &str )
{
  int l = str.slen();
  if ( writen( sockfd, strtochar(str), l ) < 0 )
    throw Error_while_writing_socket();
}

int client::put( const String &str, int max_l )
{
  int l = str.slen();
  if (l > max_l)
    l = max_l;
  int m = writen( sockfd, strtochar(str), l);
  if (l < 0)
    throw Error_while_writing_socket();
  return m;
} 

void client::close( void )
{
  s_close(sockfd);
  sockt = SOCK_UNDEF;
  status = UNCONNECTED;
}
 
server::server( void )
{
  listenfd = 0;
  sockt = SOCK_UNDEF;
}

server::~server( void )
{
}

void server::do_accept( client* &cl )
{}

//void server::close_client( client *cl )
//{}


void server::close( void )
{
  s_close(listenfd);
  sockt = SOCK_UNDEF;
}

