/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000,2003 by Holger Flemming                               *
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

#ifndef __TCP_H__
#define __TCP_H__


#include <netinet/in.h>


#include "String.h"
#include "socket.h"


#include "iperrors.h"

class tcp_client : public client
{
 protected:
  struct sockaddr_in cl_adrinfo;

 public:
  tcp_client( void );
  tcp_client( int, struct sockaddr_in );
  tcp_client( const String&, const String& );
  tcp_client( const String&, const String&, bool& );
  ~tcp_client( void );

  inline uint32_t get_ip(void)
    {
      return cl_adrinfo.sin_addr.s_addr;
    }
  inline uint16_t get_port( void )
    {
      return cl_adrinfo.sin_port;
    }
};


class tcp_server : public server
{
 protected:
  //  socklen_t clilen;

 public:
  tcp_server(int port, int backlog = 10);
  ~tcp_server();
  void do_accept( client* & );
};


#endif
