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
#include "connector.h"
#include "cluster.h"

#ifdef COMPILE_WX
#include "wx.h"
#endif
#ifdef COMPILE_DIGISTATUS
#include "digi.h"
#endif

extern fwd_router router;
extern connection_control connector;
#ifdef COMPILE_SLAVES
extern slave_control slaves;
#endif
extern config_file configuration;

#ifdef COMPILE_CLUSTER
extern cluster_control cluster_cntl;
#endif

#ifdef COMPILE_DIGISTATUS
extern digi_control digi;
#endif // COMPILE_DIGISTATUS

#ifdef COMPILE_WX
extern wx_control wx;
#endif
extern int kill_thread;
extern zeit shutdown_time;
extern bool shutdown_flag;

void schedule::process_init_polling( void )
{
  polling.clear();
#ifdef COMPILE_TCP
  if (server_tcp != NULL) 
    polling.rset(*server_tcp);
  if (server_tcp_fwd != NULL)
    polling.rset(*server_tcp_fwd);
#ifdef COMPILE_SMTP
  if (server_tcp_smtp != NULL)
    polling.rset(*server_tcp_smtp);
#endif
#ifdef COMPILE_HTTP
  if (server_tcp_http != NULL)
    polling.rset(*server_tcp_http);
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  if (server_ax25 != NULL) 
    polling.rset(*server_ax25);
#ifdef COMPILE_AXHTP
  if (server_axhtp != NULL)
    polling.rset(*server_axhtp);
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
  for (t_threads_it i = threads.begin(); i != threads.end(); ++i)
    i->set_str(polling);
}

void schedule::process_new_incomming_connections( void )
{
#ifdef COMPILE_TCP
  if (server_tcp != NULL && polling.rtest(*server_tcp))
    new_tcp_connect();
  if (server_tcp_fwd != NULL && polling.rtest(*server_tcp_fwd))
    new_tcp_fwd_connect();
#ifdef COMPILE_SMTP
  if (server_tcp_smtp != NULL && polling.rtest(*server_tcp_smtp))
    new_tcp_smtp_connect();
#endif
#ifdef COMPILE_HTTP
  if (server_tcp_http != NULL && polling.rtest(*server_tcp_http))
    new_tcp_http_connect();
#endif
#endif // COMPILE_TCP
#ifdef COMPILE_AX25
  if (server_ax25 != NULL && polling.rtest(*server_ax25))
    new_ax25_connect();
#ifdef COMPILE_AXHTP
  if ( server_axhtp != NULL && polling.rtest(*server_axhtp))
    new_axhtp_connect();
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
}

void schedule::process_outgoing_connections( void )
{
#ifdef COMPILE_AX25
#ifdef COMPILE_WX
  wx_config_file wxcfg;
  if (wx.start_connection(wxcfg))
    start_wx_connect(wxcfg);
#endif // COMPILE_WX

#ifdef COMPILE_DIGISTATUS
  digi_config_file digicfg;
  if (digi.start_connection(digicfg))
    start_digi_connect(digicfg);
#endif // COMPILE_DIGISTATUS

#ifdef COMPILE_SLAVES
  {
    callsign call;
    connect_string cpf;
    String bake;
    String slots;
    String pfad;
    slave::slave_mode mode;
    String pw;
    
    if (slaves.start_connection( call,pfad,cpf,slots,bake,mode,pw ) )
      start_rpc_connect(call,cpf,slots,bake,pfad,mode,pw);
  }
#endif // COMPILE_SLAVES
#endif // COMPILE_AX25
  {
    callsign call;
    char typ;
    String adresse;
    bool ar;
    
    if (router.start_connection(call,typ,adresse,ar))
      start_fwd_connect(call,typ,adresse,ar);
  }
  
  {
    char typ;
    String adresse;
    int id;
    if (connector.start_connection(typ,adresse,id))
      start_connection_connect(id,typ,adresse);
  }
#ifdef COMPILE_AX25
#ifdef COMPILE_CLUSTER

  {
    callsign call;
    connect_string cs;
    dx_cluster_typ tp;

    if (cluster_cntl.cluster_connect(call,cs,tp))
      start_cluster_connect(call,cs,tp);

  }
#endif // COMPILE_CLUSTER
#endif // COMPILE_AX25
}

void schedule::process_threads( void )
{

  if ( shutdown_flag && shutdown_time - zeit() < 12 )
    for (t_threads_it it = threads.begin() ; it != threads.end() ; ++it )
      it->do_close();

  else
    {
      t_threads_it it = threads.begin();
      bool end_flag  = it == threads.end();
      while (!end_flag)
	{
	  t_threads_it i2 = it;
	  ++it;
	  end_flag = it == threads.end();


	  if (do_thread(i2,polling))
	    {
	      // thread sofort beenden und aus der Liste nehmen.
	      i2->do_kill();
	      threads.erase(i2);
	    }
	  else
	    if ( do_interface(i2) )
	      // jetzt thread zunaechst schliessen
	      i2->do_close();
	}
    }
  // Ueberpruefen, ob ein Thread geloescht werden soll. Dies kann erreicht
  // werden, indem in die Thread-ID in die globale Variable kill_thread
  // eingetragen wird.
  if (kill_thread != -1)
    {
      for (t_threads_it it = threads.begin() ; it != threads.end();++it)
	if (it->get_id() == kill_thread)
	  {
	    // thread beenden und aus der Liste nehmen.
	    it->do_kill();
	    threads.erase(it);
	    break;
	  }
      kill_thread = -1;
    }
  
  t_threads_it it = threads.begin();
  bool end_flag  = it == threads.end();
  while (!end_flag)
    {
      t_threads_it i2 = it;
      ++it;
      end_flag = it == threads.end();
      if ( i2->do_discon() )
	threads.erase(i2);
    }

}

void schedule::do_process( void )
{
  try
    {
      process_init_polling();
      
      polling.poll(MAX_POLL_WAIT_TIME);
      
      process_new_incomming_connections();
      process_outgoing_connections();
      
      system_info sysinf;
      sysinf.clear_tlist();
      for (t_threads_it it = threads.begin() ; it != threads.end();++it)
	{
	  sysinf.set_tlist(it->get_info());
	}
      process_threads();
    }
  catch( Error_while_select ek )
    {
      int en = ek.get_enum();
      logf.eintrag(String("Fehler beim Socket-Polling, Fehlernummer: ")+itoS(en),LOGMASK_PRGRMERR);
      if ( en == 9 )
	{
	  system_info si;
	  si.Print_descriptoren( cerr, '\n' );
	}
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}
