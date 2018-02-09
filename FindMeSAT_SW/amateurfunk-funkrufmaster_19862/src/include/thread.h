/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2001-2004 by Holger Flemming                               *
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
 *  		                                			    *
 ****************************************************************************/

#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <list.h>

#include "autoconf.h"
#include "socket.h"

#include "recv_select.h"
#include "String.h"
#include "user.h"
#include "interfaces.h"
#include "user_interface.h"
#include "cluster_interface.h"
#include "makros.h"
#include "connect_string.h"
#include "bake.h"
#include "wx_interface.h"
#include "zeit.h"
#include "rpc_interface.h"
#include "fwd_interface.h"
#include "smtp_interface.h"
#include "http_interface.h"
#include "connect_interface.h"
#include "digi.h"

#include <string.h>
#include <fstream>

using namespace std;

class Error_try_to_do_NULL_Interface
{
#ifdef _DEBUG_EXEC_
 public:
  Error_try_to_do_NULL_Interface()
    {
      cerr << "Error_try_to_do_NULL_Interface" << endl;
    }
#endif
};

class Error_try_to_access_NULL_Stream
{
#ifdef _DEBUG_EXEC_
 public:
  Error_try_to_access_NULL_Stream()
    {
      cerr << "Error_try_to_access_NULL_Stream" << endl;
    }
#endif
};

struct thread_info
{
  int pid;
  callsign call;
  char direction;
  char state;
  String typ;
  delta_t login_zeit;
  delta_t user_zeit;
  delta_t sys_zeit;
  int in_bytes;
  int out_bytes;
};

class thread
{
 private:
  static int last_id;
  bool discon;
  
 public:
  enum types { t_nodef, t_ax25_con, t_tcpcon, t_dxclu, t_wx, t_rpc, t_fwd, 
	       t_smtp, t_http, t_axhtp, t_digi, t_outgoing };
  char end_of_line;
  String input_puffer;
  String output_puffer;
  bool wait_for_connect;

 private:
  int id;
  bool tcp_ax25_flag;
  bool in_out_coming_flag;
  client *cli_sock;
  interfaces *intf;
  types typ;
  zeit startzeit;
  cpu_time cpu_zeiten;
  unsigned int in_bytes;
  unsigned int out_bytes;

 public:
  thread( void );
  thread( client&, interfaces&, types, bool );
  inline types get_typ( void )
    {
      return typ;
    }
  inline bool get_tcp_ax25_flag( void )
    {
      return tcp_ax25_flag;
    }
  inline void set_str(rx_tx_select &p)
    {
      if (cli_sock != NULL)
	{
	  p.rset(*cli_sock);
	  if (output_puffer.slen() > 0 || wait_for_connect)
	    p.wset(*cli_sock);
	}
    }
  inline bool test_str_r(rx_tx_select &p)
    {
      if (cli_sock != NULL)
	return p.rtest(*cli_sock);
      else
	return false;
    }
  inline bool test_str_w(rx_tx_select &p)
    {
      if (cli_sock != NULL)
	return p.wtest(*cli_sock);
      else
	return false;
    }
  inline int get_id( void )
    {
      return id;
    }
  bool get_char( char & );
  int gets( String&, size_t  );
  int write( const String& );
  int write( const String&, int );
  bool do_interface(const String& , String& );
  void do_close();
  bool do_discon( void );
  void do_kill( void );
  int get_next_id(void);
  struct thread_info get_info( void );
};

  typedef list<thread> t_threads;
  typedef t_threads::iterator t_threads_it;
#endif
