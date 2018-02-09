/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2003 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; yopu can redistribute ist and/or modify   *
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

#ifndef __AX25_H__
#define __AX25_H__

#include "autoconf.h"

#include "String.h"
#include "callsign.h"
#include "iperrors.h"
#include "ax25_errors.h"
#include "socket.h"

#include <netinet/in.h>
#include <sys/socket.h>

#ifndef CONFIG_SUSE_8_0
#include <linux/ax25.h>
#include <linux/rose.h>
#include <linux/netrom.h>
#endif // CONFIG_SUSE_8_0

#include <netax25/axlib.h>



class ax25_client : public client
{
 protected:
  struct full_sockaddr_ax25 max;
  struct full_sockaddr_ax25 sax;

 public:
  ax25_client( void );
  ax25_client( int, struct full_sockaddr_ax25, struct full_sockaddr_ax25);
  ax25_client( callsign , callsign , const String &, vector<callsign>  );
  ax25_client( callsign , callsign , const String &, vector<callsign> , bool & );
  callsign get_mycall( void );
  callsign get_slavecall( void );
};

class ax25_server : public server
{
 protected:
  struct full_sockaddr_ax25 max;

 public:
  // ax25_server(..) : Moegliche Ausnahmen:
  // Error_ax25_config_get_addr
  // Error_ax25_master_call
  // Error_could_not_gen_socket
  // Error_address_in_use
  // Error_could_not_bind_socket
  // Error_could_not_set_to_listen
  ax25_server(callsign mycall, String &port, int backlog = 10);
  ~ax25_server();
  void do_accept( client* & );
};

// init_ax25() : Moelgliche Ausnahmen:
// Error_ax25_config_load_ports
void init_ax25( void );

#endif
