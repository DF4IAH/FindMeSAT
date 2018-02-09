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

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <netdb.h>

#include "autoconf.h"
#include "socket.h"
#ifdef COMPILE_AX25
#include "ax25.h"
#endif // COMPILE_AX25
#ifdef COMPILE_TCP
#include "tcp.h"
#endif

#include "recv_select.h"
#include "String.h"
#include "user.h"
#include "interfaces.h"
#include "user_interface.h"
#ifdef COMPILE_CLUSTER
#include "cluster_interface.h"
#endif
#include "makros.h"
#include "connect_string.h"
#include "bake.h"
#ifdef COMPILE_DIGISTATUS
#include "digi.h"
#endif
#ifdef COMPILE_WX
#include "wx_interface.h"
#endif
#include "thread.h"
#include "table.h"
#include "system_info.h"
#ifdef COMPILE_SLAVES
#include "slaves.h"
#endif
#include "cmdline.h"
#ifdef COMPILE_HTTP
#include "http_interface.h"
#endif
#include <iostream.h>
#include <string.h>
#include <fstream.h>

using namespace std;

class Error_could_not_initialize_scheduler
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_initialize_scheduler()
    {
      cerr << "Error_could_not_initialize_scheduler" << endl;
    }
#endif
};




class schedule
{
 private:
  t_baken *baken;
#ifdef COMPILE_DIGISTATUS
  digi_config_files digi_files;
#endif  
#ifdef COMPILE_WX
  wx_config_files wx_files;
#endif // COMPILE_WX
  syslog logf;
  
  rx_tx_select polling;
#ifdef COMPILE_TCP
  tcp_server *server_tcp;
  tcp_server *server_tcp_fwd;
#ifdef COMPILE_HTTP
  tcp_server *server_tcp_http;
#endif
#ifdef COMPILE_SMTP
  tcp_server *server_tcp_smtp;
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  ax25_server *server_ax25;
#ifdef COMPILE_AXHTP
  ax25_server *server_axhtp;
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
  t_threads threads;
  callsign mycall;

  int cmdline_login_port;
  int cmdline_fwd_port;
  int cmdline_http_port;
  int cmdline_smtp_port;

  void cut_line( String& );
#ifdef COMPILE_TCP
  int get_portnummer(t_tcp_ports);
  void new_tcp_connect(void );
  void new_tcp_fwd_connect(void );
#ifdef COMPILE_SMTP
  void new_tcp_smtp_connect( void );
#endif
#ifdef COMPILE_HTTP
  void new_tcp_http_connect( void );
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  void new_ax25_connect( void );
  void new_ax25_fwd_connect( ax25_client&, const callsign& , const callsign&, bool );
#ifdef COMPILE_SMTP
  void new_ax25_smtp_connect( ax25_client&, const callsign& );
#endif
#ifdef COMPILE_AXHTP
  void new_axhtp_connect( void );
#endif
  void start_cluster_connect( const callsign&, connect_string, dx_cluster_typ );
#ifdef COMPILE_DIGISTATUS
  void start_digi_connect( digi_config_file & );
#endif
#ifdef COMPILE_WX
  void start_wx_connect( wx_config_file & );
#endif
  void start_rpc_connect( const callsign &, connect_string &, const String &, const String &, const String &, slave::slave_mode, const String& );
  void start_ax25_fwd_connect( const callsign &, connect_string & , bool);
  void start_ax25_connection_connect( int , connect_string & );
#endif // COMPILE_AX25
#ifdef COMPILE_TCP
  void start_tcp_fwd_connect( const callsign&, const String&, const String&, bool );
  void start_tcp_connection_connect( int , const String &, const String &   );
#endif // COMPILE_TCP
  void start_fwd_connect(const callsign&, char, const String& , bool);
  void start_connection_connect( int, char , const String& );
  bool do_interface( t_threads::iterator  );
  bool do_thread( t_threads::iterator, rx_tx_select& );
 
#ifdef COMPILE_TCP
  void start_tcp_server( void );
  void start_tcp_fwd_server( void );
#ifdef COMPILE_SMTP
  void start_tcp_smtp_server( void );
#endif
#ifdef COMPILE_HTTP
  void start_tcp_http_server( void );
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  void start_ax25_server( void );
#ifdef COMPILE_AXHTP
  void start_axhtp_server( void );
#endif
#endif // COMPILE_AX25

  void process_init_polling( void );
  void process_new_incomming_connections( void );
  void process_outgoing_connections( void  );
  void process_threads( void );

 public:
  schedule(t_baken&, cmdline& );
  void do_process(void );
  void do_shutdown( void );
};
#endif
