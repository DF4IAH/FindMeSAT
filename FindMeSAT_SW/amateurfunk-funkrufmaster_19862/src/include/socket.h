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

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "iperrors.h"
#include "iputil.h"
#include "String.h"

enum Socket_type { SOCK_UNDEF, SOCK_TCP, SOCK_UDP, SOCK_AX25 };

class client
{
 public:
  enum Status  { UNCONNECTED, CONNECTED };

 protected:
  Status status;;
  int sockfd;
  Socket_type sockt;

 public:
  client( void );
  ~client( void );
  //  Status get_status();
  void get( char & );
  void get( String& , int );
  void getline( String& , int );
  void put( char );
  void put( const String & );
  int put( const String&, int );
  void close( void );
  //void shutdown( void );
  inline int select_descr( void ) const
    {
      return sockfd;
    } 
  inline Socket_type get_socket_type( void )
    {
      return sockt;
    }
};


class server
{
 protected:
  int listenfd;
  Socket_type sockt;

 public:
  server( void );
  virtual ~server();
  virtual void do_accept( client* &  );
  //virtual void close_client( client* );
  void close( void );
  inline int select_descr( void ) const
    {
      return listenfd;
    }
  inline Socket_type get_socket_type( void )
    {
      return sockt;
    }
};


#endif // __SOCKET_H__
