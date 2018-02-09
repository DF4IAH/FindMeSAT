/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003-2004 by Holger Flemming                               *
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


#include "fwd_autorouter.h"

#include "fwd_nachrichten.h"
#include "fwd_router.h" 
#include "logfile.h"
#include "config.h"

extern callsign G_mycall;
extern fwd_router router;
extern config_file configuration;

routing_tab_nachbar_eintrag::routing_tab_nachbar_eintrag( const callsign &call )
{
  nachbar = call;
  delay_n = -1.;
  delay_n_1 = -1.;
  delay_m = -1.;

  t_n = zeit(0);
  t_n_1 = zeit(0);
  t_m = zeit(0);
}

void routing_tab_nachbar_eintrag::rx_delay( double d, double rtt)
{
  if (d == -1.)
    {
      delay_n = -1.;
      delay_n_1 = -1.;
    }
  else
    {
      delay_n = d;
      delay_n_1 = d + 0.5 * rtt;
      t_n = zeit();
      t_n_1 = zeit();
    }
}

void routing_tab_nachbar_eintrag::tx_delay( double d )
{
  delay_m = d;
  t_m = zeit();
}

void routing_tab_nachbar_eintrag::messung( double d )
{
#ifdef _DEBUG_ARLOGS_
  cerr << "routing_tab_nachbar_eintrag::messung : " << d  << endl;
  cerr << "Zielgebiet : " << ds;
  cerr << " Nachbar  : " << nachbar << endl;
  cerr << "Vorher : " << delay_n << "  " << delay_n_1 << "  " << t_n_1 << endl;
#endif
  if ( (d == -1.) || (delay_n == -1.) )
    {
      delay_n = -1.;
      delay_n_1 = -1.;
    }
  else
    {
      delay_n_1 = delay_n + 0.5 * d;
      t_n_1 = zeit();
    }
#ifdef _DEBUG_ARLOGS_
  cerr << "Nachher : " << delay_n << "  " << delay_n_1 << "  " << t_n_1 << endl;
#endif 
}

void routing_tab_nachbar_eintrag::init_nachbar( void )
{
#ifdef _DEBUG_ARLOGS_
  cerr << "routing_tab_nachbar_eintrag::init_nachbar : " << d  << endl;
  cerr << "Zielgebiet : " << ds;
  cerr << " Nachbar  : " << nachbar << endl;
  cerr << "Vorher : " << delay_n << "  " << delay_n_1 << "  " << t_n_1 << endl;
#endif
  delay_n = -1.;
  delay_n_1 = -1.;
  delay_m = -1.;
  t_n = zeit();
  t_n_1 = zeit();
  t_m = zeit();
#ifdef _DEBUG_ARLOGS_
  cerr << "Nachher : " << delay_n << "  " << delay_n_1 << "  " << t_n_1 << endl;
#endif 
}


double routing_tab_nachbar_eintrag::tx_delay( void )
{
  return delay_n_1;
}

double routing_tab_nachbar_eintrag::last_tx_delay( void )
{
  return delay_m;
}

zeit routing_tab_nachbar_eintrag::last_tx_meldung( void )
{
  return t_m;
}

zeit routing_tab_nachbar_eintrag::last_rx_meldung( void )
{
  return t_n;
}

routing_tab_entry::routing_tab_entry( const destin &ds, bool lo )
{
  zielgebiet = ds;
  local = lo;
  t_l = zeit();
  nachbarn.clear();
}

void routing_tab_entry::set_local( bool lo )
{
  local = lo;
  t_l = zeit();
}

void routing_tab_entry::add( const callsign & c )
{
#ifdef _DEBUG_ARLOGS_
  cerr << "routing_tab_entry::add( const callsign & c , all_nachbar_tabs &tabs)" << endl;
  try
    {
      cerr << zielgebiet << " : " << c << endl;
    }
  catch( Error_destin_checksum_error )
    {
      cerr << "Zielgebiet enthaelt Pruefsummenfehler !!! " << endl;
    }
#endif 
  routing_tab_nachbar_eintrag eintrag(c);

  nachbarn.push_back(eintrag);

}

void routing_tab_entry::del( const callsign &c )
{
  for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if ( samecall(it->nachbar,c) )
      {
	//tabs.del(c,it);
	nachbarn.erase(it);
	return;
      }
}

void routing_tab_entry::rx_delay( const callsign &c, double d, double rtt )
{
  for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if ( samecall(it->nachbar,c) )
      {
	it->rx_delay(d,rtt);
	return;
      }
}

double routing_tab_entry::get_min_delay( callsign &c )
{
  double d_min = -1.;

  if ( local )
    {
      d_min = 0.;
      c = G_mycall;
    }
  else
    for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
      {
	if ( it->tx_delay() != -1. )
	  if ( (d_min == -1.) || (d_min > it->tx_delay()) )
	    {
	      d_min = it->tx_delay();
	      c = it->nachbar;
	    }
      }
  return d_min;
}


struct destin_info routing_tab_entry::get_info( void ) 
{
  struct destin_info tmp;

  tmp.zielgebiet = zielgebiet;
  tmp.min_delay = get_min_delay(tmp.min_delay_call);
  for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    tmp.delay.push_back( it->tx_delay() );

   return tmp;
}


void routing_tab_entry::messung( const callsign &call, double rtt )
{
  for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if (samecall(call,it->nachbar))
      {
	it->messung(rtt);
	return;
      }
}

void routing_tab_entry::init_nachbar( const callsign &call )
{
  for (t_nachbarn_it it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if (samecall(call,it->nachbar))
      {
	it->init_nachbar();
	return;
      }
}


/*-----------------------------------------
  


-----------------------------------------
*/


routing_tab::routing_tab( void )
{
  eintraege.clear();
  //  nachbar_tabellen.clear();
  all_nachbarn.clear();
}

void routing_tab::clear( void )
{
  eintraege.clear();
  //nachbar_tabellen.clear();
  all_nachbarn.clear();
}

void routing_tab::add( const callsign &call )
{
  fwdlog flog(configuration);
  flog.eintrag(call,"Fuege Nachbarrufzeichen zu Routingtabelle hinzu.",FWDLOGMASK_AUTOROUTE);
  //nachbar_tabellen.add(call);
  struct nachbarn_t nb;
  nb.call = call;
  nb.last_rtt = -1.;
  all_nachbarn.push_back(nb);
  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
    it->add(call);
}

void routing_tab::del( const callsign &call )
{
  fwdlog flog(configuration);
  flog.eintrag(call,"Loesche Nachbar aus Routingtabelle",FWDLOGMASK_AUTOROUTE);
  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
    it->del(call);
  //nachbar_tabellen.del(call);
  for ( vector<struct nachbarn_t>::iterator it = all_nachbarn.begin() ; it != all_nachbarn.end() ; ++it )
    {
      if ( samecall(it->call,call) )
	{
	  all_nachbarn.erase(it);
	  break;
	}
    }
}

bool routing_tab::exist( const destin &ds )
{
  try
    {
      fwdlog flog(configuration);
      flog.eintrag("Pruefe ob Ziel "+ds.get_string()+" existiert",FWDLOGMASK_AUTOROUTE);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Zielgebiet.",FWDLOGMASK_FWDERR);
      return false;
    }
  try
    {
      for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	if (ds == it->get_destin() )
	  return true;
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Routingtabelle",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }
  return false;
}

void routing_tab::add( const destin &ds, bool l)
{
  try
    {
#ifdef _DEBUG_ARLOGS_
      cerr << "routing_tab::add( const destin &ds, bool l)" << endl;
      cerr << ds << "  " << l << endl;
#endif
      fwdlog flog(configuration);
      if (!exist(ds))
	{
	  flog.eintrag("Fuege Ziel "+ds.get_string()+" zur Routingtabelle zu",FWDLOGMASK_AUTOROUTE);
	  routing_tab_entry eintrag(ds,l);
	  //vector<callsign> nachbarn = nachbar_tabellen.get_all_nachbarn();
	  for (vector<struct nachbarn_t>::iterator it = all_nachbarn.begin(); it != all_nachbarn.end(); ++it )
	    eintrag.add(it->call);
	  eintraege.push_back(eintrag);
	}
      else
	flog.eintrag("Ziel "+ds.get_string()+" ist bereits in Routingtabelle",FWDLOGMASK_AUTOROUTE);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Zielgebiet.",FWDLOGMASK_FWDERR);
    }
}

void routing_tab::del( const destin &ds )
{
  try
    {
      fwdlog flog(configuration);
      if (exist(ds))
	{
	  flog.eintrag("Loesche Ziel "+ds.get_string()+" aus Routingtabelle",FWDLOGMASK_AUTOROUTE);
	  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	    if (ds == it->get_destin() )
	      {
		//vector<callsign> nachbarn = nachbar_tabellen.get_all_nachbarn();
		for (vector<struct nachbarn_t>::iterator it2 = all_nachbarn.begin(); it2 != all_nachbarn.end(); ++it2 )
		  it->del(it2->call);
		
		eintraege.erase(it);
		return;
	      }
	}
      else
	flog.eintrag("Ziel "+ds.get_string()+" ist nicht in Routingtabelle",FWDLOGMASK_AUTOROUTE);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Zielgebiet.",FWDLOGMASK_FWDERR);
    }
}

void routing_tab::set_local( const destin &ds )
{
  try
    {
      fwdlog flog(configuration);
      flog.eintrag("Setze Ziel "+ds.get_string()+" als lokal",FWDLOGMASK_AUTOROUTE);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Zielgebiet.",FWDLOGMASK_FWDERR);
      return;
    }
  try
    {
      for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	if (ds == it->get_destin() )
	  it->set_local(true);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Rotuingtabelle.",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }
}

void routing_tab::rx_delay( const destin &ds, const callsign &from, double d )
{
  try
    {
      fwdlog flog(configuration);
      flog.eintrag(from,"Fuer Ziel "+ds.get_string()+" Delay empfangen",FWDLOGMASK_AUTOROUTE);
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Zielgebiet.",FWDLOGMASK_FWDERR);
      return;
    }
  double rtt = -1.;
  for ( vector<struct nachbarn_t>::iterator it = all_nachbarn.begin(); it != all_nachbarn.end(); ++it )
    {
      if (samecall(it->call,from))
	{
	  rtt = it->last_rtt;
	  break;
	}
    }
  try
    {
      for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	if (ds == it->get_destin() )
	  {
	    it->rx_delay(from,d,rtt);
	    return;
	  }
    }
  catch( Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destiantion Pruefsummenfehler in Rotuingtabelle.",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }
}

bool routing_tab::messung( const callsign &from, double  rtt )
{
#ifdef _DEBUG_ARLOGS_
  cerr << "routing_tab::messung : " << from << " " << rtt << endl;
#endif
  for (t_routing_tab_it it = eintraege.begin(); it != eintraege.end() ; ++it )
    it->messung(from,rtt);
  //  nachbar_tabellen.messung(from,rtt);
  bool flag = false;
  for ( vector<struct nachbarn_t>::iterator it = all_nachbarn.begin(); it != all_nachbarn.end(); ++it )
    {
      if (samecall(it->call,from))
	{
	  flag = ( (it->last_rtt == -1.) && (rtt != -1.) );
	  it->last_rtt = rtt;
	  break;
	}
    }
  return flag;
}

void routing_tab::init_nachbar( const callsign &nb )
{
  for (t_routing_tab_it it = eintraege.begin(); it != eintraege.end() ; ++it )
    it->init_nachbar(nb);
}

vector<callsign> routing_tab::get_route( const destin &ds )
{
  vector<callsign> tmp;

  if (ds.check() )
    {
      try
	{
	  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	    if (ds.in(it->get_destin() ) )
	      {
		callsign call;
		it->get_min_delay(call);
		bool flag = false;
		for (vector<callsign>::iterator it2 = tmp.begin(); it2 != tmp.end() && !flag; ++it2)
		  if (samecall(*it2,call))
		    flag = true;
		
		if (!flag) tmp.push_back(call);
	      }
	}
      catch( Error_destin_checksum_error )
	{
	  fwdlog flog(configuration);
	  flog.eintrag("Destiantion Pruefsummenfehler in Rotuingtabelle.",FWDLOGMASK_FWDERR);
	  throw Error_routing_tab_corrupted();
	}
    }
  return tmp;
}

void routing_tab::add_d_info(vector<struct d_infos> &ents, t_nachbarn_it it, double d_min, const destin &ds)
{
  double rtt = -1.;
  for ( vector<struct nachbarn_t>::iterator it2 = all_nachbarn.begin(); it2 != all_nachbarn.end(); ++it2 )
    if (samecall(it2->call,it->nachbar))
      {
	rtt = it2->last_rtt;
	break;
      }

  if ( rtt != -1. )
    // Nachricht nur an Nachbarn erzeugen, zu denen es einen aktuell 
    // laufenden Link gibt.
    {
      fwdlog flog(configuration);
      flog.eintrag(it->nachbar,"Erzeuge D-Nachricht",FWDLOGMASK_AUTOROUTE);
      struct d_infos di;
      di.to = it->nachbar;
      di.zielgebiet = ds;
      di.delay = d_min;
      ents.push_back(di);
      it->tx_delay(d_min);
    }
}

bool routing_tab::check_d_info(double d_min, const t_routing_tab_it &it, const t_nachbarn_it & it2 , const callsign &call_min, bool local) 
{
  if ( ( d_min == -1. ) && ( it2->last_tx_delay() != -1. ))
    // Ziel ist nicht mehr erreichbar, bisher nicht gemeldet
    return true;

  else if ( local )
    {
      // Ziel ist lokal erreichbar
      
      if ( it2->last_tx_delay() == -1.)
	// Ziel wurde bisher dem Nachbarn nicht gemeldet
	return true;
      
      if ( zeit() - it2->last_tx_meldung() > AUTOROUTER_LOCAL_REFRESH )
	// Info ueber lokales Ziel wurde seit mehr als einer halben Stunde
	// nicht mehr weiter geleitet.
	return true;
    }

  else if ( ( it2->nachbar != call_min ) && ( d_min != -1. ) )
    {
      // D-Nachrichten werden nicht an Nachbarn geschickt, ueber die
      // das Ziel am besten zu erreichen ist.
      
      // Weitere D-Nachrichten nur fuer den Fall, dass das Ziel auch
      // erreichbar ist
      
      if ( it2->last_tx_delay() == -1.)
	// Ziel wurde bisher dem Nachbarn nicht gemeldet
	return true;	

      else if ( ( d_min > Q_PLUS * it2->last_tx_delay() ) &&
		( fabs(d_min - it2->last_tx_delay()) > Q_PLUS_ABS ) ) 
	// Laufzeit hat sich deutlich vergroessert
	return true;	

      else if ( ( d_min < Q_MINUS * it2->last_tx_delay() ) && 
		( fabs(d_min - it2->last_tx_delay()) > Q_MINUS_ABS ) )
	// Laufzeit hat sich deutlich verkuerzt
	return true;

      else if ( zeit() - it2->last_tx_meldung() > AUTOROUTER_REFRESH)
	// Letzte Meldung an den Nachbarn laenger als Refreshzeit her
	if (!check_age(it,AUTOROUTER_MAX_ENTRY_AGE1))
	  // Eintrag in der letzten halben Stunde bestaetigt?
	  return true;

    }
  else if ( (it2->nachbar == call_min ) && ( d_min != -1. ) && 
	    ( it2->last_tx_delay() != -1. ) )
    // Der Nachbar ist derjenige, ueber den das Ziel am besten zu 
    // erreichen ist. Es wurde ihm jedoch bereits eine zeit uebermittelt.
    // In diesem Fall muss diese Uebermittlung rueckgaengig gemacht werden,
    // indem diesem Nachbarn die Nichterreichbarkeit gemeldet wird.
    return true;
  return false;	
}

bool routing_tab::check_age(const t_routing_tab_it &it, int max_age )
{

  if (it->is_local())
    return false;

  zeit youngest_rx = zeit(0);

  for (t_nachbarn_it it2 = it->nachbarn.begin(); it2 != it->nachbarn.end(); ++it2)
    {
      if (it2->last_rx_meldung() > youngest_rx)
	youngest_rx = it2->last_rx_meldung();
    }
  return zeit() - youngest_rx > max_age;
}

vector<struct d_infos> routing_tab::get_d_infos( const destin &ds )
{
  vector<struct d_infos> tmp;
  if (!ds.check())
    {
      fwdlog flog(configuration);
      flog.eintrag("Pruefsummenfehler in Zielgebiet",FWDLOGMASK_FWDERR);
      return tmp;
    }

  fwdlog flog(configuration);
  flog.eintrag("Fuer Ziel "+ds.get_string()+" Delays ermitteln",FWDLOGMASK_AUTOROUTE);
  try
    {
      for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	if ( ds == it->get_destin() )
	  {
	    bool local = it->is_local();
	    callsign call_min;
	    double d_min = it->get_min_delay(call_min);
	    flog.eintrag(call_min,"kleinstes Delay :"+dtoS(d_min),FWDLOGMASK_AUTOROUTE);
	    for (t_nachbarn_it it2 = it->nachbarn.begin() ; it2 != it->nachbarn.end(); ++it2)
	      if (check_d_info(d_min,it,it2,call_min,local) )
		{
		  if (it2->nachbar == call_min)
		    // Nachbar ist derjenige, ueber den das Ziel am besten zu 
		    // erreichen ist. Diesem Nachbarn mit -1 melden, dass man 
		    //keinen besseren Weg kennt.
		    add_d_info(tmp,it2,-1.,ds);
		  else
		    add_d_info(tmp,it2,d_min,ds);
		}
	    return tmp;
	  }
    }
  catch( Error_destin_checksum_error)
    {
      flog.eintrag("Destination Pruefsummenfehler in Routingtabelle",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }
  return tmp;
}

vector<struct d_infos> routing_tab::get_d_infos( void )
{
  vector<struct d_infos> tmp;

  fwdlog flog(configuration);
  flog.eintrag("Fuer alle Ziele Delays ermitteln",FWDLOGMASK_AUTOROUTE);
  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
    {
      bool local = it->is_local();
      flog.eintrag("Ziel :"+it->get_destin().get_string(),FWDLOGMASK_AUTOROUTE);
      callsign call_min;
      double d_min = it->get_min_delay(call_min);
      flog.eintrag(call_min,"kleinstes Delay :"+dtoS(d_min),FWDLOGMASK_AUTOROUTE);
      for (t_nachbarn_it it2 = it->nachbarn.begin() ; it2 != it->nachbarn.end(); ++it2)
	if (check_d_info(d_min,it,it2,call_min,local) )
	    {
	      if (it2->nachbar == call_min)
		// Nachbar ist derjenige, ueber den das Ziel am besten zu 
		// erreichen ist. Diesem Nachbarn mit -1 melden, dass man 
		//keinen besseren Weg kennt.
		add_d_info(tmp,it2,-1.,it->get_destin());
	      else
		add_d_info(tmp,it2,d_min,it->get_destin());
	    }
    }
  return tmp;
}


void routing_tab::clean_up( const destin &ds )
{
  if (!ds.check())
    {
      fwdlog flog(configuration);
      flog.eintrag("Pruefsummenfehler in Zielgebiet",FWDLOGMASK_FWDERR);
      return;
    }

  fwdlog flog(configuration);
  try
    {
      for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); )
	{
	  t_routing_tab_it it2 = it;
	  ++it;
	  if ( ds == it2->get_destin() )
	    {
	      callsign call_min;
	      double d_min = it2->get_min_delay(call_min);
	      if ( d_min == -1. || check_age(it2,AUTOROUTER_MAX_ENTRY_AGE2) )
		{
		  flog.eintrag("Ziel "+it2->get_destin().get_string()+" nicht mehr erreichbar, aus Routingtabelle streichen.",FWDLOGMASK_AUTOROUTE);
		  del(ds);
		}
	      return;
	    }
	}
    }
  catch( Error_destin_checksum_error)
    {
      flog.eintrag("Destination Pruefsummenfehler in Routingtabelle",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }
}

void routing_tab::clean_up( void )
{
  fwdlog flog(configuration);
  for ( t_routing_tab_it it = eintraege.begin(); it != eintraege.end();  )
    {
      t_routing_tab_it it2 = it;
      ++it;
      callsign call_min;
      double d_min = it2->get_min_delay(call_min);
      if (d_min == -1. || check_age(it2,AUTOROUTER_MAX_ENTRY_AGE2) )
	{
	  flog.eintrag("Ziel "+it2->get_destin().get_string()+" nicht mehr erreichbar, aus Routingtabelle streichen.",FWDLOGMASK_AUTOROUTE);
	  del(it2->get_destin());
	}
    }
}

vector<struct d_infos> routing_tab::reinit_corrupted_tab( void )
{
  fwdlog flog(configuration );
  flog.eintrag("Korrupierte Routing-Tabelle erkannt. Reinitialisierung",FWDLOGMASK_FWDERR);
  eintraege.clear();

  vector<struct d_infos> tmp;
  tmp.clear();

  vector<struct nachbarn_t>::iterator it;
  for (it = all_nachbarn.begin(); it != all_nachbarn.end(); ++it )
    {
      struct d_infos info_tmp;
      info_tmp.to = it->call;
      info_tmp.zielgebiet = destin();
      info_tmp.delay = -1.;
      tmp.push_back(info_tmp);
    }

  return tmp;
}


vector<struct destin_info> routing_tab::get_infos( void ) 
{
  vector<struct destin_info> tmp;

  try
    {
      for (t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	tmp.push_back(it->get_info());
    }
  catch(  Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destination Pruefsummenfehler in Routingtabelle",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }

  return tmp;
}

vector<struct destin_info> routing_tab::get_infos( const destin &ds ) 
{
  vector<struct destin_info> tmp;

  if (!ds.check())
    {
      syslog logf(configuration);
      logf.eintrag("Checksum-Error in Destination, Show",LOGMASK_PRGRMERR);
      return tmp;
    }
  try
    {
      for (t_routing_tab_it it = eintraege.begin(); it != eintraege.end(); ++it )
	if (ds.in(it->get_destin() ) )
	  tmp.push_back(it->get_info());
    }
  catch(  Error_destin_checksum_error )
    {
      fwdlog flog(configuration);
      flog.eintrag("Destination Pruefsummenfehler in Routingtabelle",FWDLOGMASK_FWDERR);
      throw Error_routing_tab_corrupted();
    }

  return tmp;
}

vector<callsign> routing_tab::get_nachbarcalls( void ) 
{
  vector<callsign> cls;

  for ( vector<struct nachbarn_t>::iterator it = all_nachbarn.begin() ; it != all_nachbarn.end(); ++it )
    cls.push_back(it->call);

  return cls;
}

/*-----------------------------------------
  


-----------------------------------------
*/

void autorouter::create_init_message( const callsign &nb )
{
  Mid m = router.get_mid(true);
  zielgebiets_nachricht dmsg(m);
  dmsg.zielgebiet = destin();
  dmsg.delay = -1.;
  nachricht *msg_ptr = (nachricht*) &dmsg;
  router.route_message(*msg_ptr,false,G_mycall,nb);
}

void autorouter::create_d_message( const destin &ds )
{
  vector<struct d_infos> tmp;
  tmp = rt.get_d_infos(ds);

  for (vector<struct d_infos>::iterator it = tmp.begin();it != tmp.end(); ++it)
    {
      Mid m = router.get_mid(true);
      zielgebiets_nachricht dmsg(m);
      dmsg.zielgebiet = it->zielgebiet;
      dmsg.delay = it->delay;
      nachricht *msg_ptr = (nachricht*) &dmsg;
      router.route_message(*msg_ptr,false,G_mycall,it->to);
    }
}

void autorouter::create_d_message( void )
{
  vector<struct d_infos> tmp;
  tmp = rt.get_d_infos();

  for (vector<struct d_infos>::iterator it = tmp.begin();it != tmp.end(); ++it)
    {
      Mid m = router.get_mid(true);
      zielgebiets_nachricht dmsg(m);
      dmsg.zielgebiet = it->zielgebiet;
      dmsg.delay = it->delay;
      nachricht *msg_ptr = (nachricht*) &dmsg;
      router.route_message(*msg_ptr,false,G_mycall,it->to);
    }
}


void autorouter::clean_up( const destin &ds )
{
  rt.clean_up(ds);
}

void autorouter::clean_up( void )
{
  rt.clean_up();
}

void autorouter::reinit_corrupted_routing_tab( void )
{
  vector<struct d_infos> tmp;
  tmp = rt.reinit_corrupted_tab();

  for (vector<struct d_infos>::iterator it = tmp.begin();it != tmp.end(); ++it)
    {
      Mid m = router.get_mid(true);
      zielgebiets_nachricht dmsg(m);
      dmsg.zielgebiet = it->zielgebiet;
      dmsg.delay = it->delay;
      nachricht *msg_ptr = (nachricht*) &dmsg;
      router.route_message(*msg_ptr,false,G_mycall,it->to);
    }
}

autorouter::autorouter( void )
{
  init();
}

autorouter::~autorouter( void )
{}

void autorouter::enable( void )
{
  if (!activ)
    {
      activ = true;
      configuration.set("AUTOROUTER","JA");
      configuration.save();
    }
}

void autorouter::disable( void )
{
  if (activ)
    {
      activ = false;
      configuration.set("AUTOROUTER","NEIN");
      configuration.save();
    }
}

void autorouter::init( void )
{
  try
    {
      activ = ( configuration.find("AUTOROUTER") == "JA" );
    }
  catch( Error_parameter_not_defined )
    {
      activ = false;
    }
}

void autorouter::clear( void )
{
  try
    {
      rt.clear();
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
    }
}

void autorouter::add( const callsign &nachbar )
{
  try
    {
      rt.add(nachbar);
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
    }
}

void autorouter::del( const callsign &nachbar )
{
  try
    {
      rt.del(nachbar);
      clean_up();
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
    }
}

void autorouter::slave_connect( vector<destin> dests )
{
  try
    {
      for (vector<destin>::iterator it = dests.begin() ; it != dests.end(); ++it )
	{
	  if ( rt.exist(*it) )
	    rt.set_local(*it);
	  else
	    rt.add(*it,true);
	  create_d_message(*it);
	}
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
    }
}

void autorouter::slave_discon( vector<destin> dests )
{
  try
    {
      for (vector<destin>::iterator it = dests.begin() ; it != dests.end(); ++it )
	{
	  if (rt.exist(*it) )
	    {
	      rt.del(*it);
	      create_d_message(*it);
	      clean_up(*it);
	    }
	}
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
    }
}

void autorouter::rx_d_message( const zielgebiets_nachricht &dmsg, const callsign &from )
{

  if (activ)
    {
      try
	{
	  if (!dmsg.zielgebiet.check())
	    {
	      fwdlog flog(configuration);
	      flog.eintrag("Dest.-Pruefsummenfehler in Empfangener Zielgebietsnachricht",FWDLOGMASK_FWDERR);
	      return;
	    }
	  try
	    {
	      if ( (dmsg.zielgebiet == destin()) && (dmsg.delay == -1.))
		{
		  rt.init_nachbar(from);
		  create_d_message();
		  clean_up();
		}
	      else
		{
		  if ( rt.exist(dmsg.zielgebiet) )
		    rt.rx_delay(dmsg.zielgebiet,from,dmsg.delay);
		  else
		    if (dmsg.delay != -1.)
		      {
			rt.add(dmsg.zielgebiet,false);
			rt.rx_delay(dmsg.zielgebiet,from,dmsg.delay);
		      }
		  
		  create_d_message(dmsg.zielgebiet);
		  clean_up(dmsg.zielgebiet);
		}
	    }
	  catch( Error_destin_checksum_error )
	    {
	      fwdlog flog(configuration);
	      flog.eintrag("Destination-Pruefsummenfehler in Routing-Tabelle",FWDLOGMASK_FWDERR);
	    }
	}
      catch( Error_routing_tab_corrupted )
	{
	  reinit_corrupted_routing_tab();
	}
    }
}

void autorouter::rtt_messung( const callsign &from, double rtt )
{
#ifdef _DEBUG_ARLOGS_
  cerr << "autorouter::rtt_messung : " << from << " " << rtt << endl;
#endif
  if (activ)
    {
      try
	{ 
	  if (rt.messung(from,rtt))
	    create_init_message(from);
	  create_d_message();
	}
      catch( Error_routing_tab_corrupted )
	{
	  reinit_corrupted_routing_tab();
	}
    }
}

void autorouter::messung_gescheitert( const callsign &from )
{
  if (activ)
    {
      try
	{ 
	  rt.messung(from,-1.);
	  create_d_message();
	  clean_up();
	}
      catch( Error_routing_tab_corrupted )
	{
	  reinit_corrupted_routing_tab();
	}
    }
}

void autorouter::cyclic( void )
{
  if (activ)
    {

      try
	{
	  create_d_message();
	  clean_up();
	}
      catch( Error_routing_tab_corrupted )
	{
	  reinit_corrupted_routing_tab();
	}
    }
}

vector<callsign> autorouter::get_route(const destin &ds )
{
  try
    {
      return rt.get_route(ds);
    }
  catch( Error_routing_tab_corrupted )
    {
      reinit_corrupted_routing_tab();
      vector<callsign> tmp;
      tmp.clear();
      return tmp;
    }
}

