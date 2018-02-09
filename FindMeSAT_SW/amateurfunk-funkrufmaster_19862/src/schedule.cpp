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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *  		                                			    *
 ****************************************************************************/

#include "schedule.h"

#include "fwd_router.h"
#include "logfile.h"
#include "connector.h"
#include "callsign.h"

#ifdef COMPILE_SMTP
#include "smtp_interface.h"
#endif


extern callsign G_mycall;
extern config_file configuration;
extern callsign_database calls;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
extern slave_control slaves;
#endif
extern fwd_router router;
extern connection_control connector;

int kill_thread;

#ifdef COMPILE_TCP 
int schedule::get_portnummer(t_tcp_ports port)
{
  char service_name[20];
  char prot_name[20];
  int cmdline_port_no = -1;
  int default_port_no = -1;
  int config_port_no  = -1;
  int port_no         = -1;

  switch(port)
    {
      case p_login : strcpy(service_name,"frm");
	             strcpy(prot_name,"tcp");
                     cmdline_port_no = cmdline_login_port;
		     try { config_port_no  = configuration.find("TELNET_PORT").Stoi(); }
		     catch( Error_parameter_not_defined ) { }
                     default_port_no = 4711;
                     break;
      case p_fwd   : strcpy(service_name,"frm_fwd");
                     strcpy(prot_name,"tcp");
                     cmdline_port_no = cmdline_fwd_port;
		     try { config_port_no  = configuration.find("FWD_PORT").Stoi(); }
		     catch( Error_parameter_not_defined ) { }
                     default_port_no = 4712;
                     break;
#ifdef COMPILE_HTTP
      case p_http  : strcpy(service_name,"frm_http");
                     strcpy(prot_name,"tcp");
		     try { config_port_no  = configuration.find("HTTP_PORT").Stoi(); }
		     catch( Error_parameter_not_defined ) { }
                     cmdline_port_no = cmdline_http_port;
                     default_port_no = 8080;
                     break;
#endif
#ifdef COMPILE_SMTP
      case p_smtp  : strcpy(service_name,"frm_smtp");
                     strcpy(prot_name,"tcp");
		     try { config_port_no  = configuration.find("SMTP_PORT").Stoi(); }
		     catch( Error_parameter_not_defined ) { }
                     cmdline_port_no = cmdline_smtp_port;
                     default_port_no = 8025;
                     break;
#endif
    }

  if (cmdline_port_no != -1)
    port_no = cmdline_port_no;
  else
    {
      struct servent *ent;
      ent = getservbyname(service_name,prot_name);
      if (ent != NULL)
	port_no = ntohs(ent->s_port);
      else 
      {
	if (config_port_no != -1)
	    port_no = config_port_no;
	else
	    port_no = default_port_no;
      }
    }
   if (port_no != config_port_no)
   {
      switch(port)
	{
    	    case p_login : configuration.set("TELNET_PORT",itoS(port_no)); break;
    	    case p_fwd   : configuration.set("FWD_PORT",itoS(port_no)); break;
#ifdef COMPILE_HTTP
    	    case p_http  : configuration.set("HTTP_PORT",itoS(port_no)); break;
#endif
#ifdef COMPILE_SMTP 
   	    case p_smtp  : configuration.set("SMTP_PORT",itoS(port_no)); break;
#endif
	 }
	 configuration.save();
    }
   return port_no;
}
#endif // COMPILE_TCP




bool schedule::do_interface( t_threads::iterator it )
{
  bool end_flag = false;
  String outp;

  try
    {
      end_flag = !it->do_interface(it->input_puffer,outp);
      it->input_puffer = "";
    }
  catch( Error_String_index_out_of_range )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "String - Indexverletzung im Interface-Prozess" << endl;
#endif
    }
  it->output_puffer.append(outp);

  return end_flag;

}

bool schedule::do_thread( t_threads::iterator it, rx_tx_select &p )
{
  String puffer;
  int bytes;

  try
    {
      if (it->test_str_r(p)) 
	{
	  if ((bytes = it->gets(puffer,16383)) > 0)
	    {
	      it->input_puffer.append(String(puffer));
	    }
	  else
	    {
	      return true;
	    }
	}
    }
  catch( Error_String_index_out_of_range )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "Indexverletzung beim Fuellen des Input-Puffers" << endl;
#endif
    }
  try
    {
      if (it->test_str_w(p))
	{
	  if (it->wait_for_connect)
	    it->wait_for_connect = false;

	  int l = (int) it->output_puffer.slen();
	  if (l > 0 )
	    {
	      String pu = it->output_puffer;
	      int l = pu.slen();
	      int l2;
	      if (l > 1020)
		l2 = 1020;
	      else
		l2 = l;
	      bytes = it->write(pu, l2 );
	      if (bytes > 0)
		if ( bytes < l )
		  {
		    it->output_puffer = it->output_puffer.copy(bytes,l-bytes);
		  }
		else
		  it->output_puffer = String("");
	      else
		return true;
	    }
	}
    }
  catch( Error_String_index_out_of_range )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "Index-Verletzung beim Schreiben des Ausgangspuffers" << endl;
#endif
      it->output_puffer = String ("");
      return false;
    }
  return false;
  //  return do_interface( it );
}


schedule::schedule(t_baken &bs, cmdline &cmd) : logf(configuration)
{
  char *par_ptr;
  //shutdown_discon = false;
  if ((par_ptr = cmd.option('t')) != NULL)
    {
      cmdline_login_port = atoi(par_ptr);
    }
  else 
    cmdline_login_port = -1;

  if ((par_ptr = cmd.option('f')) != NULL)
    {
    cmdline_fwd_port = atoi(par_ptr);
    }
  else 
    cmdline_fwd_port = -1;
#ifdef COMPILE_HTTP
  if ((par_ptr = cmd.option('h')) != NULL)
    {
    cmdline_http_port = atoi(par_ptr);
    }
  else 
    cmdline_http_port = -1;
#endif
#ifdef COMPILE_SMTP
  if ((par_ptr = cmd.option('s')) != NULL)
    {
    cmdline_smtp_port = atoi(par_ptr);
    }
  else 
    cmdline_smtp_port = -1;
#endif
  kill_thread = -1;
#ifdef COMPILE_TCP
  server_tcp = NULL;
  server_tcp_fwd = NULL;
#ifdef COMPILE_HTTP
  server_tcp_http = NULL;
#endif
#ifdef COMPILE_SMTP
  server_tcp_smtp = NULL;
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  server_ax25 = NULL;
#ifdef COMPILE_AXHTP
  server_axhtp = NULL;
#endif // COMPILE_AXHTP
#endif // _COMPILE_AX25
  baken = &bs;
#ifdef COMPILE_WX
  wx_files = wx_config_files(configuration);
#endif // COMPILE_WX
#ifdef COMPILE_DIGISTATUS
  digi_files = digi_config_files(configuration);
#endif // COMPILE_DIGISTATUS
  mycall = G_mycall;
  polling.clear();
#ifdef COMPILE_TCP

  start_tcp_server();
  start_tcp_fwd_server();
#ifdef COMPILE_SMTP
  start_tcp_smtp_server();
#endif
#ifdef COMPILE_HTTP
  start_tcp_http_server();
#endif

#endif // COMPILE_TCP

#ifdef COMPILE_AX25
  start_ax25_server();
#ifdef COMPILE_AXHTP
  start_axhtp_server();
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
}


void schedule::do_shutdown( void )
{
  t_threads_it it = threads.begin();

  // Alle threads beenden
  for (it = threads.begin() ; it != threads.end() ; it++ )
    it->do_close();

  // und thread-Liste loeschen
  threads.clear();


#ifdef COMPILE_TCP
  if (server_tcp != NULL)
    delete server_tcp;

  if (server_tcp_fwd != NULL)
    delete server_tcp_fwd;
#ifdef COMPILE_HTTP
  if (server_tcp_http != NULL)
    delete server_tcp_http;
#endif
#ifdef COMPILE_SMTP
  if (server_tcp_smtp != NULL)
    delete server_tcp_smtp;
#endif
#endif

#ifdef COMPILE_AX25
  if (server_ax25 != NULL)
    delete server_ax25;
#endif

// fehlt hier nicht noch der axhtp-Server?
}
