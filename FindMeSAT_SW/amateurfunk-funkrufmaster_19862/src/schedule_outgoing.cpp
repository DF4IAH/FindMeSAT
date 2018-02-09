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

extern config_file configuration;
extern fwd_router router;
extern connection_control connector;
#ifdef COMPILE_SLAVES
extern slave_control slaves;
#endif
#ifdef COMPILE_CLUSTER
extern cluster_control cluster_cntl;
#endif

#ifdef COMPILE_AX25
#ifdef COMPILE_CLUSTER


/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_cluster_connect startet die Verbindung zum DX-Cluster              *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_cluster_connect( const callsign &call, connect_string cluster_pfad, dx_cluster_typ tp )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *cliptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = cluster_pfad.get_digi(0).call();
  String mycall = cluster_pfad.get_mycall().call();
  String port = cluster_pfad.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < cluster_pfad.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( cluster_pfad.get_l2_digi(i).call() );


  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      cliptr = new ax25_client(digi,mycall,port,l2_digis,connected);

      // instanz eines clusterinterfaces erzeugen und Pointer darauf 
      // speichern
      cluster_interface *ci = new cluster_interface(outp,true,tp,cluster_pfad);
      cluster_cntl.aufbau();
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*cliptr,*ci,thread::t_dxclu,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
      
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("Fehlerhafte Port Angabe",LOGMASK_PRGRMERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_connection_timed_out )
    {
      logf.eintrag("Time-Out beim Verbindungsaufbau zum DX-Cluster",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_connection_refused )
    {
      logf.eintrag("Verbindungsaufbau abgelehnt zum DX-Cluster",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_host_unreachable )
    {
      logf.eintrag("DX-Cluster nicht erreichbar.",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_net_unreachable )
    {
      logf.eintrag("Netzwerk nicht erreichbar",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_connect_failed )
    {
      logf.eintrag("Verbindungsaufbau zum DX-Cluster fehlgeschlagen",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte Socket nicht mit eigener Adresse verbinden",LOGMASK_VERBERR);
      cluster_cntl.gescheitert();
    }
}

#endif // COMPILE_CLUSTER

#ifdef COMPILE_WX

/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_wx_connect(wx_config_file& ) startet die Verbindung zu einer       *
 * Wetter-Station                                                           *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_wx_connect( wx_config_file &wx_cfg )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *cliptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = wx_cfg.pfad.get_digi(0).call();
  String mycall = wx_cfg.pfad.get_mycall().call();
  String port = wx_cfg.pfad.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < wx_cfg.pfad.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( wx_cfg.pfad.get_l2_digi(i).call() );

  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      cliptr = new ax25_client(digi,mycall,port,l2_digis,connected);
      // instanz eines clusterinterfaces erzeugen und Pointer darauf 
      // speichern
      wx_interface *wxi = new wx_interface(outp,true,wx_cfg);
      
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*cliptr,*wxi,thread::t_wx,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("Fehlerhafte Port Angabe",LOGMASK_PRGRMERR);
    }
  catch( Error_connection_timed_out )
    {
      logf.eintrag("Time-Out beim Verbindungsaufbau zur Wetterstation " + digi, LOGMASK_VERBERR);
    }
  catch( Error_connection_refused )
    {
      logf.eintrag("Verbindungsaufbau abgelehnt",LOGMASK_VERBERR);
    }
  catch( Error_host_unreachable )
    {
      logf.eintrag("Gegenstation nicht erreichbar.",LOGMASK_VERBERR);
    }
  catch( Error_net_unreachable )
    {
      logf.eintrag("Netzwerk nicht erreichbar",LOGMASK_VERBERR);
    }
  catch( Error_connect_failed )
    {
      logf.eintrag("Verbindungsaufbau fehlgeschlagen " +digi,LOGMASK_VERBERR);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte Socket nicht mit eigener Adresse verbinden",LOGMASK_VERBERR);
    }
}


#endif // COMPILE_WX


/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_rpc_connect startet die Verbindung zu einer Funkruf-Karte          *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

#ifdef COMPILE_SLAVES
void schedule::start_rpc_connect( const callsign &call, connect_string &cpf, const String &slots, const String &bake, const String &pfad, slave::slave_mode mode, const String &pw )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *ax25ptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = cpf.get_digi(0).call();
  String mycall = cpf.get_mycall().call();
  String port = cpf.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < cpf.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( cpf.get_l2_digi(i).call() );

  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      ax25ptr = new ax25_client(digi,mycall,port,l2_digis,connected);
      // instanz eines rpcinterfaces erzeugen und Pointer darauf 
      // speichern
      rpc_interface *rpci = new rpc_interface(outp,true,call,cpf,bake,slots,pfad,mode,pw);
      slaves.set_status(call,slave::sts_aufbau);
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*ax25ptr,*rpci,thread::t_rpc,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_ax25_config_get_addr )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Fehlerhafte Port Angabe",LOGMASK_PRGRMERR);
    }
  catch( Error_connection_timed_out )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Time-Out beim Verbindungsaufbau zum Funkrufsender " + digi, LOGMASK_VERBERR);
    }
  catch( Error_connection_refused )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Verbindungsaufbau abgelehnt",LOGMASK_VERBERR);
    }
  catch( Error_host_unreachable )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Gegenstation nicht erreichbar.",LOGMASK_VERBERR);
    }
  catch( Error_net_unreachable )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Netzwerk nicht erreichbar",LOGMASK_VERBERR);
    }
  catch( Error_connect_failed )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Verbindungsaufbau fehlgeschlagen " +digi,LOGMASK_VERBERR);
    }
  catch( Error_could_not_bind_socket )
    {
      slaves.set_status(call,slave::sts_gescheitert);
      logf.eintrag("Konnte Socket nicht mit eigener Adresse verbinden",LOGMASK_VERBERR);
    }
}
#endif

#ifdef COMPILE_DIGISTATUS

/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_digi_connect(digi_config_file& ) startet die Verbindung zu einem   *
 * Digi zwecks Statusabfrage                                                *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_digi_connect( digi_config_file &digi_cfg )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *cliptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = digi_cfg.pfad.get_digi(0).call();
  String mycall = digi_cfg.pfad.get_mycall().call();
  String port = digi_cfg.pfad.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < digi_cfg.pfad.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( digi_cfg.pfad.get_l2_digi(i).call() );

  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      cliptr = new ax25_client(digi,mycall,port,l2_digis,connected);
      // instanz eines digiinterfaces erzeugen und Pointer darauf 
      // speichern
      digi_interface *digii = new digi_interface(outp,true,digi_cfg);
      
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*cliptr,*digii,thread::t_digi,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("Fehlerhafte Port Angabe",LOGMASK_PRGRMERR);
    }
  catch( Error_connection_timed_out )
    {
      logf.eintrag("Time-Out beim Verbindungsaufbau zum Digi " + digi, LOGMASK_VERBERR);
    }
  catch( Error_connection_refused )
    {
      logf.eintrag("Verbindungsaufbau abgelehnt",LOGMASK_VERBERR);
    }
  catch( Error_host_unreachable )
    {
      logf.eintrag("Gegenstation nicht erreichbar.",LOGMASK_VERBERR);
    }
  catch( Error_net_unreachable )
    {
      logf.eintrag("Netzwerk nicht erreichbar",LOGMASK_VERBERR);
    }
  catch( Error_connect_failed )
    {
      logf.eintrag("Verbindungsaufbau fehlgeschlagen " +digi,LOGMASK_VERBERR);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte Socket nicht mit eigener Adresse verbinden",LOGMASK_VERBERR);
    }
}


#endif // COMPILE_DIGISTATUS



/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_ax25_fwd_connect startet die Verbindung zu einem                   *
 * AX25-Forward-Partner                                                     *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_ax25_fwd_connect( const callsign &call, connect_string &cpf, bool ar )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *ax25ptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = cpf.get_digi(0).call();
  String mycall = cpf.get_mycall().call();
  String port = cpf.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < cpf.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( cpf.get_l2_digi(i).call() );

  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      ax25ptr = new ax25_client(digi,mycall,port,l2_digis,connected);
      // instanz eines clusterinterfaces erzeugen und Pointer darauf 
      // speichern
      fwd_interface *fi = new fwd_interface(outp,call,true,cpf,ar);
      router.connection_started(call);
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*ax25ptr,*fi,thread::t_fwd,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("Fehlerhafte Port Angabe",LOGMASK_PRGRMERR);
      router.connection_failed(call);
    }
  catch( Error_connection_timed_out )
    {
      logf.eintrag("Time-Out beim Verbindungsaufbau zum Forwardpartner " + digi, LOGMASK_VERBERR);
      router.connection_failed(call);
    }
  catch( Error_connection_refused )
    {
      logf.eintrag("Verbindungsaufbau abgelehnt",LOGMASK_VERBERR);
      router.connection_failed(call);
    }
  catch( Error_host_unreachable )
    {
      logf.eintrag("Gegenstation nicht erreichbar.",LOGMASK_VERBERR);
      router.connection_failed(call);
    }
  catch( Error_net_unreachable )
    {
      logf.eintrag("Netzwerk nicht erreichbar",LOGMASK_VERBERR);
      router.connection_failed(call);
    }
  catch( Error_connect_failed )
    {
      logf.eintrag("Verbindungsaufbau fehlgeschlagen " +digi,LOGMASK_VERBERR);
      router.connection_failed(call);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte Socket nicht mit eigener Adresse verbinden",LOGMASK_VERBERR);
      router.connection_failed(call);
    }
}

#endif // COMPILE_AX25

#ifdef COMPILE_TCP
/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_tcp_fwd_connect startet die Verbindung zu einem                    *
 * TCP-Forward-Partner                                                      *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_tcp_fwd_connect( const callsign &nachbar, const String &host, const String &port , bool ar )
{
  String outp;
  t_threads_it it1,it2;
  fwdlog fwd_log(configuration);
  //iplog ilog(configuration);

  // Zeiger auf einen ax25stream erzeugen
  tcp_client *tcpptr;

  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      tcpptr = new tcp_client(host,port,connected);
      // instanz eines fwd_interfaces erzeugen und Pointer darauf 
      // speichern
      fwd_interface *fwdi = new fwd_interface(outp,nachbar,nachbar,false,ar);
      router.connection_started(nachbar);
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*tcpptr,*fwdi,thread::t_fwd,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_connection_timed_out )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Time-Out beim Verbindungsaufbau zum Forwardpartner ", FWDLOGMASK_FWDERR);
    }
  catch( Error_connection_refused )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Verbindungsaufbau abgelehnt",FWDLOGMASK_FWDERR);
    }
  catch( Error_host_unreachable )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Gegenstation nicht erreichbar.",FWDLOGMASK_FWDERR);
    }
  catch( Error_net_unreachable )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Netzwerk nicht erreichbar",FWDLOGMASK_FWDERR);
    }
  catch( Error_connect_failed )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Verbindungsaufbau fehlgeschlagen ",FWDLOGMASK_FWDERR);
    }
  catch( Error_hostname_look_up )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Hostname unbekannt bei ",FWDLOGMASK_FWDERR);
    }
  catch( Error_could_not_bind_socket )
    {
      router.connection_failed(nachbar);
      fwd_log.eintrag(nachbar.call(),"Konnte Socket nicht mit eigener Adresse verbinden",FWDLOGMASK_FWDERR);
    }
}
#endif // COMPILE_TCP

/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_fwd_connect startet die Verbindung zu allen Forward-partnern       *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/


void schedule::start_fwd_connect( const callsign &nachbar, char typ, const String& adresse, bool ar)
{
#ifdef COMPILE_AX25
  if (typ == 'A')
    {
      try
	{
	  connect_string cpf(adresse);
	  start_ax25_fwd_connect(nachbar,cpf,ar);
	}
      catch( Error_syntax_fehler_in_connect_string )
	{
	  fwdlog fwd_log(configuration);
	  fwd_log.eintrag(nachbar.call(),"Fehler im Connectpfad zum Forwardpartner",FWDLOGMASK_FWDERR);
	}
    }
#endif // COMPILE_AX25
#ifdef COMPILE_TCP
  if (typ == 'T')
    {
      try
	{
	  String host;
	  String port;
	  
	  host = router.get_hostname(adresse);
	  port = router.get_portstring(adresse);
	  
	  start_tcp_fwd_connect(nachbar,host,port,ar);
	}
      catch( Error_wrong_ip_address )
	{
	  fwdlog fwd_log(configuration);
	  fwd_log.eintrag(nachbar,"Fehler in IP-Adresse zum Forwardpartner",FWDLOGMASK_FWDERR);
	}
    }
#endif // COMPILE_TCP
}

#ifdef COMPILE_AX25
/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_ax25_connection_connect startet eine angeforderte AX25-Verbindung  *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_ax25_connection_connect( int id, connect_string &cpf )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen ax25stream erzeugen
  ax25_client *ax25ptr;
  // ersten Digipeater, mycall und port aus dem Connect_pfad
  // extrahieren
  String digi = cpf.get_digi(0).call();
  String mycall = cpf.get_mycall().call();
  String port = cpf.get_port();
  vector<callsign> l2_digis;
  l2_digis.clear();
  for (unsigned int i = 0; i < cpf.get_l2_digi_anz() ; i++ )
    l2_digis.push_back( cpf.get_l2_digi(i).call() );

  // Mit diesen Parametern neuen ax25stream erzeugen. Der erste Digi wird
  // dabei automatisch connected
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      ax25ptr = new ax25_client(digi,mycall,port,l2_digis,connected);
      // instanz eines clusterinterfaces erzeugen und Pointer darauf 
      // speichern
      connect_interface *ci = new connect_interface(outp,true,id,cpf);
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*ax25ptr,*ci,thread::t_outgoing,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_ax25_config_get_addr )
    {
      connector.received(id,"Fehlerhafte Port Angabe",true);
      connector.disconnected(id);
    }
  catch( Error_connection_timed_out )
    {
      connector.received(id,"Time-Out beim Verbindungsaufbau",true);
      connector.disconnected(id);
    }
  catch( Error_connection_refused )
    {
      connector.received(id,"Verbindungsaufbau abgelehnt",true);
      connector.disconnected(id);
    }
  catch( Error_host_unreachable )
    {
      connector.received(id,"Gegenstation nicht erreichbar",true);
      connector.disconnected(id);
    }
  catch( Error_net_unreachable )
    {
      connector.received(id,"Netzwerk nicht erreichbar",true);
      connector.disconnected(id);
    }
  catch( Error_connect_failed )
    {
      connector.received(id,"Verbindungsaufbau fehlgeschlagen",true);
      connector.disconnected(id);
    }
  catch( Error_could_not_bind_socket )
    {
      connector.received(id,"Konnte Socket nicht mit eigener Adresse verbinden",true);
      connector.disconnected(id);
    }
}

#endif // COMPILE_AX25

#ifdef COMPILE_TCP
/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_tcp_connection_connect startet eine angeforderte TCP-Verbindung    *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_tcp_connection_connect( int id, const String &host, const String &port  )
{
  String outp;
  t_threads_it it1,it2;

  // Zeiger auf einen tcpstream erzeugen
  tcp_client *tcpptr;
  try
    {
      // in der Variablen connected wird gespeichert, ob der Connect
      // bereits erfolgt ist. 
      bool connected = true;
      // Connect nicht blockierend ausführen
      tcpptr = new tcp_client(host,port,connected);
      // instanz eines fwd_interfaces erzeugen und Pointer darauf 
      // speichern
      connect_interface *ci = new connect_interface(outp,false,id);
      // Neuen Thread anlegen und in Liste abspeichern
      thread th(*tcpptr,*ci,thread::t_outgoing,false);
      th.wait_for_connect = !connected;
      it1 = threads.begin();
      it2 = threads.insert(it1,th);
  
      it2->output_puffer.append(outp);
    }
  catch( Error_connection_timed_out )
    {
      connector.received(id,"Time-Out beim Verbindungsaufbau",true);
      connector.disconnected(id);
    }
  catch( Error_connection_refused )
    {
      connector.received(id,"Verbindungsaufbau abgelehnt",true);
      connector.disconnected(id);
    }
  catch( Error_host_unreachable )
    {
      connector.received(id,"Gegenstation nicht erreichbar.",true);
      connector.disconnected(id);
    }
  catch( Error_net_unreachable )
    {
      connector.received(id,"Netzwerk nicht erreichbar",true);
      connector.disconnected(id);
    }
  catch( Error_connect_failed )
    {
      connector.received(id,"Verbindungsaufbau fehlgeschlagen",true);
      connector.disconnected(id);
    }
  catch( Error_hostname_look_up )
    {
      connector.received(id,"Hostname unbekannt.",true);
      connector.disconnected(id);
    }
  catch( Error_could_not_bind_socket )
    {
      connector.received(id,"Konnte Socket nicht mit eigener Adresse verbinden",true);
      connector.disconnected(id);
    }
}
#endif // COMPILE_TCP

/****************************************************************************
 *                                                                          *
 *  		                                			    *
 * start_connection_connect startet eine angeforderte Verbindung            *
 *  		                                			    *
 *                                                                          *
 *  		                                			    *
 ****************************************************************************/

void schedule::start_connection_connect( int id, char typ, const String& adresse)
{
#ifdef COMPILE_AX25
  if (typ == 'A')
    {
      try
	{
	  connect_string cpf(adresse);
	  start_ax25_connection_connect(id,cpf);
	}
      catch( Error_syntax_fehler_in_connect_string )
	{
	  connector.received(id,"*** Formatfehler im Connect-Pfad",true);
	  connector.disconnected(id);
	}
    }
#endif // COMPILE_AX25
#ifdef COMPILE_TCP
  if (typ == 'T')
    {
      try
	{
	  String host;
	  String port;
	  
	  host = router.get_hostname(adresse);
	  port = router.get_portstring(adresse);
	  
	  start_tcp_connection_connect(id,host,port);
	}
      catch( Error_wrong_ip_address )
	{
	  connector.received(id,"*** Formatfehler in IP-Adresse",true);
	  connector.disconnected(id);
	}
    }
#endif // COMPILE_TCP
}
