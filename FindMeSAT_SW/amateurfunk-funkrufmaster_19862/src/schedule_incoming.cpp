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
#include "logfile.h"
#include "fwd_router.h"

#ifdef COMPILE_SMTP
#include "smtp_interface.h"
#endif

#ifdef COMPILE_AXHTP
#include "axhtp_interface.h"
#endif

extern config_file configuration;
extern fwd_router router;

extern bool shutdown_flag;
extern zeit shutdown_time;

#ifdef COMPILE_TCP 

void schedule::new_tcp_connect( void )
{
  try
    {
      iplog ilog(configuration);
      String outp;
      t_threads_it it1,it2;
      
      //cerr << "Neuer TCP-Connect" << endl;
      client *cliptr;
      tcp_client *tcpc;
      server_tcp->do_accept(cliptr);
      tcpc = (tcp_client*) cliptr;
      ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_login);
      user_interface * ui = new user_interface(outp,tcpc->get_ip(),tcpc->get_port(),false,*baken);
      thread th(*cliptr,*ui,thread::t_tcpcon,true);
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
      it2->output_puffer.append(outp);
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer TCP-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}

void schedule::new_tcp_fwd_connect( void )
{
  try
    {
      iplog ilog(configuration);
      String outp;
      t_threads_it it1,it2;
      
      client *cliptr;
      tcp_client *tcpc;
      server_tcp_fwd->do_accept(cliptr);
      tcpc = (tcp_client*) cliptr;
      // An dieser stelle muss noch das Rufzeichen ermittelt werden
      callsign c;
      bool autorouting;
      if (router.is_nachbar(tcpc->get_ip(),c,autorouting))
	{
	  try
	    {
	      ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_fwd);
	      fwd_interface *fi = new fwd_interface(outp,c,c,false,autorouting);
	      thread th(*cliptr,*fi,thread::t_fwd,true);
	      it1 = threads.begin();
	      it2 = threads.insert(it1,th);
	      it2->output_puffer.append(outp);
	    }
	  catch( Error_could_not_init_fwd_interface )
	    {
	      ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_fwd,"Interface nicht initialisierbar");
	      cliptr->close();
	      delete cliptr;
	    }
	}
      else
	{
	  ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_fwd,"Zugriff verweigert, kein bekannter Partner");
	  cliptr->close();
	  delete cliptr;
	}
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer TCP-FWD-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}

#ifdef COMPILE_SMTP
void schedule::new_tcp_smtp_connect( void )
{
  try
    {
      iplog ilog(configuration);
      String outp;
      t_threads_it it1,it2;
      
      client *cliptr;
      tcp_client *tcpc;
      server_tcp_smtp->do_accept(cliptr);
      tcpc = (tcp_client*) cliptr;
      
      ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_smtp);
      try
	{
	  smtp_interface *si = new smtp_interface(outp);
	  thread th(*cliptr,*si,thread::t_smtp,true);
	  it1 = threads.begin();
	  it2 = threads.insert(it1,th);
	  it2->output_puffer.append(outp);
	}
      catch( Error_could_not_initialize_smtp_interface )
	{
	  cliptr->close();
	  delete cliptr;
	}
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer SMTP-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}
#endif

#ifdef COMPILE_HTTP
void schedule::new_tcp_http_connect( void )
{
  try
    {
      iplog ilog(configuration);
      String outp;
      t_threads_it it1,it2;
      
      client *cliptr;
      tcp_client *tcpc;
      server_tcp_http->do_accept(cliptr);
      tcpc = (tcp_client*) cliptr;
      ilog.eintrag(tcpc->get_ip(),tcpc->get_port(),p_http);
      http_interface *hi = new http_interface(outp,*baken,tcpc->get_ip());
      thread th(*cliptr,*hi,thread::t_http,true);
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
      it2->output_puffer.append(outp);
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer HTTP-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}
#endif // COMPILE_HTTP
#endif // COMPILE_TCP


#ifdef COMPILE_AX25

void schedule::new_ax25_connect( void )
{
  try
    {
      bool ar;
      
      client *cliptr;
      ax25_client *ax25c;  
      server_ax25->do_accept(cliptr);
      ax25c = (ax25_client*) cliptr;
      callsign call = ax25c->get_slavecall();
      callsign gegen = call;
      bool fwd_flag = router.is_nachbar('A',call,ar);
      
#ifdef COMPILE_SMTP
      bool smtp_flag = is_ax25_smtp_sender(call);
#endif
      if (fwd_flag)
	{
	  new_ax25_fwd_connect(*ax25c,call,gegen,ar);
	}
#ifdef COMPILE_SMTP
      else if (smtp_flag)
	{
	  new_ax25_smtp_connect(*ax25c,call);
	}
#endif
      else
	{
	  String outp;
	  t_threads_it it1,it2;
	  
	  user u(ax25c->get_slavecall());
	  user_interface *ui = new user_interface(outp,u,true,*baken);
	  thread th(*cliptr,*ui,thread::t_ax25_con,true);
	  it1 = threads.begin();
	  it2 = threads.insert(it1,th);
	  it2->output_puffer.append(outp);
	}
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer AX25-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}

void schedule::new_ax25_fwd_connect( ax25_client &cli , const callsign &call, const callsign &gegen , bool ar)
{
  String outp;
  t_threads_it it1,it2;

  fwd_interface *fi = new fwd_interface(outp,call,gegen,true,ar);
  thread th(cli,*fi,thread::t_fwd,true);
  it1 = threads.begin();
  it2 = threads.insert(it1,th);
  it2->output_puffer.append(outp);
}

#ifdef COMPILE_SMTP
void schedule::new_ax25_smtp_connect( ax25_client &cli, const callsign &call )
{
  String outp;
  t_threads_it it1,it2;

  smtp_interface *si = new smtp_interface(outp,true);
  thread th(cli,*si,thread::t_smtp,true);
  it1 = threads.begin();
  it2 = threads.insert(it1,th);
  it2->output_puffer.append(outp);
}
#endif

#ifdef COMPILE_AXHTP
void schedule::new_axhtp_connect( void )
{
  try
    {
      client *cliptr;
      ax25_client *ax25c;  
      server_axhtp->do_accept(cliptr);
      ax25c = (ax25_client*) cliptr;
      callsign call = ax25c->get_slavecall();
      callsign gegen = call;
      
      String outp;
      t_threads_it it1,it2;
      
      user u(call);
      axhtp_interface *ahi = new axhtp_interface(outp,*baken,u);
      thread th(*cliptr,*ahi,thread::t_axhtp,true);
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
      it2->output_puffer.append(outp);
    }
  catch( Error_while_accepting_connection )
    {
      syslog logf(configuration);
      logf.eintrag("Fehler beim Akzeptieren einer AXHTP-Verbindung",LOGMASK_PRGRMERR);
      // Programm sofort beenden!
      shutdown_flag = true;
      shutdown_time = zeit();
    }
}
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
