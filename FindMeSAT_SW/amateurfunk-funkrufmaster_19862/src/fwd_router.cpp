/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
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


#include "fwd_router.h"

#include <netdb.h>
#include <iomanip.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream.h>

#include "logfile.h"
#include "fwd_execution.h"
#include "fwd_autorouter.h"

extern config_file configuration;
extern callsign G_mycall;
extern autorouter a_router;

/* 
   Die Datei beginnt mit den Methoden zur Nachbartabelle. In dieser Tabelle 
   werden alle Informationen ueber Forward-Nachbarn gepseichert und verwaltet.
*/

neighbor_tab_entry::neighbor_tab_entry( istream &fwdln, istream &fwd )
{
  fwdlog fwd_log(configuration);
  syslog logf(configuration);
  c_status = st_getrennt;
  n_pers = 0;
  n_bul = 0;
  n_dest = 0;
  n_sonst = 0;
  int_n_pers = 0;
  int_n_bul = 0;
  int_n_dest = 0;
  int_n_sonst = 0;

  t_w = 0;
  n_max = 0;
  unack = 0;
  fehler_zaehler = 0;

  no_first_connect = true;

  dests.clear();
  init_messtab();
  scheiter_zaehler = 0;
  aktive_verbindungen.clear();
  options = protokoll_optionen("");
  char puffer[20];
  String pu;
  try
    {
      fwdln.get(puffer,19,' ');
      pu = String(puffer);
      nachbar = callsign(pu);

      fwd_log.eintrag(nachbar,"Rt-Tab : Nachbar angelegt.",FWDLOGMASK_INIT);

      fwdln >> typ;
      typ = toupper(typ);

      if ( typ == '-' )
	{
	  autorouting = false;
	  fwdln >> typ;
	  typ = toupper(typ);
	}
      else
	autorouting = true;

      fwd_log.eintrag(nachbar,"Rt-Tab : Verbindungstyp : "+String(typ),FWDLOGMASK_INIT);

      if ((typ == 'U') || (typ == 'T') || (typ == 'A'))
	{
	  char ch;
	  fwdln >> ch;
	  if (ch == ':')
	    {
	      fwdln >> adress;
	      
	      fwd_log.eintrag(nachbar,"Rt-Tab : Adresse : "+adress,FWDLOGMASK_INIT);
	      char puffer[1000];
	      fwd.getline(puffer,999);
	      while (*puffer == ' ')
		{
		  istringstream dln(puffer);
		  int i = 0;
		  while (!dln.eof() && i < 20) 
		    {
		      destin dest;
		      dln >> dest;

		      dests.push_back(dest);
		    }
		  if (!fwd.eof())
		    fwd.getline(puffer,999);
		  else
		    *puffer = '*';
		}
	      fwd_log.eintrag(nachbar,"Rt-Tab : Einlesen des Forward-Partners abgeschlossen.",FWDLOGMASK_INIT);
	    }
	  else
	    {
	      fwd_log.eintrag("Fehler in Forward-Eintrag "+nachbar.str(),FWDLOGMASK_FWDERR);
	      throw Error_wrong_fwd_file_format(pu);
	    }
	}
      else
	{
	  fwd_log.eintrag("Falscher Verbindungstyp in Forward-Eintrag "+nachbar.str(),FWDLOGMASK_FWDERR);
	  throw Error_wrong_fwd_connection_typ(typ);
	}
    }
  catch( Error_no_callsign )
    {
      fwd_log.eintrag("Falsches Rufzeichen in Forward-Eintrag "+pu,FWDLOGMASK_FWDERR);
      throw Error_wrong_fwd_call(pu);
    }
}


neighbor_tab_entry::neighbor_tab_entry( const callsign &nbar, char tp, const String &adr )
{
  nachbar = nbar;
  typ = tp;
  adress = adr;
  dests.clear();
  c_status = st_getrennt;
  letzte_status_aenderung = zeit();
  aktive_verbindungen.clear();
  n_pers = 0;
  n_bul = 0;
  n_dest = 0;
  n_sonst = 0;
  int_n_pers = 0;
  int_n_bul = 0;
  int_n_dest = 0;
  int_n_sonst = 0;

  t_w = 0;
  n_max = 0;
  unack = 0;
  fehler_zaehler = 0;

  scheiter_zaehler = 0;
  init_messtab();
  options = protokoll_optionen("");
  autorouting = false;
  no_first_connect = true;
}


neighbor_tab_entry::~neighbor_tab_entry( void )
{
  dests.clear();
}

void neighbor_tab_entry::enable_autorouting( void )
{
  autorouting = true;
}

void neighbor_tab_entry::disable_autorouting( void )
{
  autorouting = false;
}

bool neighbor_tab_entry::add_destin( const destin &d )
{
  if (!d.check())
    return false;

  try
    {
      for (vector<destin>::iterator it = dests.begin() ; it != dests.end(); ++it)
	if (*it == d)
	  return false;

      dests.push_back(d);
      return true;
    }
  catch( Error_destin_checksum_error )
    {
      dests.clear();
#ifdef _DEBUG_ELOGS_
      cerr << "Nachbartabelle enthaelt korrupiertes Zielgebiet." << endl;
#endif
      return false;
    }
}

bool neighbor_tab_entry::del_destin( const destin &d )
{
  if (!d.check())
    return false;

  try
    {
      for (vector<destin>::iterator it = dests.begin() ; it != dests.end(); ++it)
	if (*it == d)
	  {
	    dests.erase(it);
	    return true;
	  }
      return false;
    }
  catch( Error_destin_checksum_error )
    {
      dests.clear();
#ifdef _DEBUG_ELOGS_
      cerr << "Nachbartabelle enthaelt korrupiertes Zielgebiet." << endl;
#endif
      return false;
    }
}

bool neighbor_tab_entry::check_destin( const destin &d )
{
  if (!d.check())
    return false;

  try
    {
      vector<destin>::iterator it;
      for (it = dests.begin() ; it != dests.end(); ++it)
	{
	  if ( d.in(*it) )
	    return true;
	}
      return false;
    }
  catch( Error_destin_checksum_error )
    {
      dests.clear();
#ifdef _DEBUG_ELOGS_
      cerr << "Nachbartabelle enthaelt korrupiertes Zielgebiet." << endl;
#endif
      return false;
    }
}


bool neighbor_tab_entry::update_pfad( const callsign &nbar, char tp, const String &adr )
{
  nachbar = nbar;
  typ = tp;
  adress = adr;
  return true;
//  letzte_status_aenderung = zeit();
}

bool neighbor_tab_entry::reset( const callsign &nbar )
{
  nachbar = nbar;
  scheiter_zaehler = 0;
  letzte_status_aenderung = zeit();
  init_messtab();
  return true;
}


/* Die folgenden Methoden dienen Dazu die Messtabelle fuer Laufzeit und
   Offsetmessungen zu verwalten.

*/

// init_messtab initialisiert die Messtabelle

void neighbor_tab_entry::init_messtab( void )
{
  for (int i = 0 ; i < 16; i++)
    {
      rtt[i] = -1.;
      offset[i] = -9999.;
    }
  last_messung = zeit(0);
  messungen_in_burst = 10;
  messung_in_progress = false;
}

// now_messung gibt einen Bool-Wert zurueck, ob wieder eine Messung
// gestartet werden muss.
bool neighbor_tab_entry::now_messung( void )
{
  if (messung_in_progress || !options.check_option(OPTION_ZEITMESSUNG))
    return false;
  if ( messungen_in_burst >= MESSUNGEN_IN_BURST )
    return (zeit() - last_messung > WAIT_AFTER_MESSBURST);
  else
    return (zeit() - last_messung > WAIT_AFTER_MESSUNG);
}

// messung_startet setzt das "messung_in_progress"-Flag und erhoeht den
// messungen_in_burst-Zaehler
void neighbor_tab_entry::messung_started( void )
{
  messung_in_progress = true;
  last_messung = zeit();
  if (messungen_in_burst >= MESSUNGEN_IN_BURST)
    messungen_in_burst = 0;
  else
    messungen_in_burst++;
}

// messung traegt ein neues Messergebnis in die Messtabelle ein.
void neighbor_tab_entry::messung( double r, double o )
{
  for (int i = 0;i < 15; i++)
    {
      rtt[i] = rtt[i+1];
      offset[i] = offset[i+1];
    }
  rtt[15] = r;
  offset[15] = o;
  messung_in_progress = false;
}


double neighbor_tab_entry::get_rtt( void )
{
  double wichtung = 0.;
  double sum = 0.;
  for ( int i = 0; i < 16 ; i++ )
    if (rtt[i] > -0.5)
      {
	sum += rtt[i];
	wichtung += 1.;
      }

  if (wichtung > 0)
    return sum / wichtung;
  else 
    return -1.;
}

double neighbor_tab_entry::get_offset( void )
{
  double wichtung = 0.;
  double sum = 0.;
  for ( int i = 0; i < 16 ; i++ )
    if (rtt[i] > -0.5)
      {
	sum += offset[i] * 1/rtt[i];
	wichtung += 1/rtt[i];
      }

  if (wichtung > 0)
    return sum / wichtung;
  else 
    return -9999.;
}

struct neighbor_info neighbor_tab_entry::get_info( void )
{
  struct neighbor_info tmp;

  tmp.call = nachbar;
  tmp.typ = typ;
  tmp.address = adress;
  tmp.stat = c_status;
  tmp.sch_cnt = scheiter_zaehler;
  tmp.last_change = letzte_status_aenderung;
  tmp.n_pers = n_pers + int_n_pers;
  tmp.n_bul = n_bul + int_n_bul;
  tmp.n_dest = n_dest + int_n_dest;
  tmp.n_sonst = n_sonst + int_n_sonst;
  tmp.t_w = t_w;
  tmp.n_max = n_max;
  tmp.unack = unack;
  tmp.fehler_zaehler = fehler_zaehler;
  tmp.options = options;
  tmp.akt_thrds = aktive_verbindungen;
  for (int i = 0; i < 16; i++)
    {
      tmp.rtt.push_back(rtt[i]);
      tmp.offset.push_back(offset[i]);
    }
  tmp.mean_rtt = get_rtt();
  tmp.mean_offset = get_offset();

  return tmp;
}

void neighbor_tab_entry::save( ostream &ostr )
{
  nachbar.set_format(false);
  nachbar.set_nossid(false);
  if (!autorouting)
    ostr << nachbar << " -" << typ << ':' << adress << endl;
  else
    ostr << nachbar << " " << typ << ':' << adress << endl;

  int cnt = 0;
  try
    {
      for (vector<destin>::iterator it = dests.begin() ; it != dests.end(); ++it)
	{
	  if (it->check())
	    {
	      ostr << ' ' << *it;
	      if (++cnt > 3)
		{
		  ostr << endl;
		  cnt = 0;
		}
	    }
	}
    }
  catch( Error_destin_checksum_error )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "Pruefsummenfehler in Zielgebiet in der Nachbartabelle." << endl;
#endif
      dests.clear();
    }
 if (cnt != 0) 
    ostr << endl;
}


void neighbor_tab_entry::status( fwd_conn_status s )
{
  c_status = s;
  if (s == st_gescheitert )
    {
      if (scheiter_zaehler==0) 
	letzte_status_aenderung = zeit();
      scheiter_zaehler++;
      init_messtab();
      a_router.messung_gescheitert(nachbar);
    }
  else if (s == st_getrennt )
    {
      messung_in_progress = false;
      letzte_status_aenderung = zeit();
    }
  else if (s == st_aktiv)
    {
      scheiter_zaehler = 0;
      letzte_status_aenderung = zeit();
    }
} 



/*---------------------------------------------------------------
  Nun folgen die Methoden des Forward-Routers. Zunaechst die Methoden, die
  sich mit der Nachbar-Tabelle befassen.

  -----------------------------------------------------------------
*/

void fwd_router::read_neighbor_tab( void )
{
  ifstream fwd(strtochar(configuration.find("FWDFILE")));
  if (fwd)
    {
      char puffer[1000];
      
      while (!fwd.eof())
	{
	  fwd.getline(puffer,999);
	  if (*puffer != '#' && *puffer != '\0')
	    {
	      istringstream ln(puffer);
	      neighbor_tab_entry eintrag(ln,fwd);
	      neighbor_tab.push_back(eintrag);
	      a_router.add(eintrag.get_call());
	    }
	}
      pos = neighbor_tab.begin();
      geaendert = false;
    }
  else
    throw Error_could_not_open_fwd_file();
}

void fwd_router::save_neighbor_tab( void )
{
  if (geaendert)
    {
      ofstream fwd(strtochar(configuration.find("FWDFILE")));
      if (fwd)
	{
	  fwd << "#" << endl;
	  fwd << "# Forward-Konfiguration" << endl;
	  fwd << "# Diese Datei wurde automatisch erzeugt. bitte nicht editieren." << endl;
	  fwd << "# Alle notwendigen Konfigurationen koennen mit entsprechenden" << endl;
	  fwd << "# Sysop-Befehlen an der Benutzeroberflaeche durchgefuehrt werden." << endl;
	  fwd << "#" << endl;
	  fwd << "#" << endl;
	  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	    {
	      it->save(fwd);
	      fwd << "#" << endl;
	    }
	  geaendert = false;
	}
      else
	throw Error_could_not_open_fwd_file();
    }
}

int fwd_router::wait_time(list<neighbor_tab_entry>::iterator it  )
{
  int cnt = it->n_gescheitert();

  if (cnt == 0) 
    return 0;
  else if (cnt < 3)
    return WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 5)
    return 5 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 7)
    return 15 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 11)
    return 60 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 20)
    return 120 * WAIT_UNTIL_CONN_RETRY;
  else 
    return 1440 * WAIT_UNTIL_CONN_RETRY;
}

fwd_router::fwd_router( void )
{
  neighbor_tab.clear();
  descriptors.clear();
  activ = false;
  geaendert = false;
  fwddirname = String("");
}

fwd_router::~fwd_router( void )
{
  save_neighbor_tab();
  clear_neighbor_tab();
}

bool fwd_router::enable( void )
{
  if (!activ)
    {
      try
	{
	  load_fwd();
	  configuration.set("FORWARDING","JA");
	  configuration.save();
	  activ = true;
	  String midfilename;
	  try
	    {
	      midfilename = configuration.find("MIDLIST");
	    }
	  catch( Error_parameter_not_defined )
	    {
	      syslog logf(configuration);
	      logf.eintrag("Parameter MIDLIST nicht definiert.",LOGMASK_PRGRMERR);
	      midfilename = "";
	    }
	  mid_tab = mid(midfilename,MAX_MID_TAB_SIZE);
	  mid_tab_routinginf = mid("",8096);
	  return true;
	}
      catch( Error_could_not_init_router )
	{
	  return false;
	}
    }
  else
    return false;
}

bool fwd_router::disable( void )
{
  if (activ)
    {
      save_neighbor_tab();
      clear_neighbor_tab();
      mid_tab.save();
      mid_tab.clear();
      configuration.set("FORWARDING","NEIN");
      configuration.save();
      activ = false;
      return true;
    }
  else
    return false;
}

void fwd_router::load_fwd( void )
{
  syslog logf(configuration);
  try
    {
      read_neighbor_tab();
      activ = true;
      geaendert = false;
      base_fwddirname = configuration.find("FWDDIR");
      fwddirname = base_fwddirname + String("route/");
      String midfilename;
      try
	{
	  midfilename = configuration.find("MIDLIST");
	}
      catch( Error_parameter_not_defined )
	{
	  syslog logf(configuration);
	  logf.eintrag("Parameter MIDLIST nicht definiert.",LOGMASK_PRGRMERR);
	  midfilename = "";
	}
      mid_tab = mid(midfilename,MAX_MID_TAB_SIZE);
      mid_tab_routinginf = mid("",8096);
      
      mid_tab.load();
      read_spool();
    }
  catch( Error_wrong_fwd_file_format ec)
    {
      logf.eintrag("Falsches Forward-File-Format bei "+ec.get_error_line(),LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    }  
  catch( Error_wrong_fwd_connection_typ ec)
    {
      logf.eintrag("Falscher Forward Verbindungstyp "+ec.get_conn_typ(),LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    } 
  catch( Error_wrong_fwd_call ec)
    {
      logf.eintrag("Falsches Rufzeichen im Forward: "+ec.get_wrong_call(),LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    } 
  catch( Error_could_not_open_fwd_file )
    {
      logf.eintrag("Kann Forward-Datei nicht oeffnen",LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Forwardverzeichnis nicht definiert",LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    }
}

void fwd_router::save_fwd( void )
{
  syslog logf(configuration);
  try
    {
      save_neighbor_tab();
      geaendert = false;
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Forward-Datei nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_fwd_file )
    {
      logf.eintrag("Kann Forward-Datei nicht oeffnen",LOGMASK_PRGRMERR);
      throw Error_could_not_init_router();
    }
}

void fwd_router::clear_neighbor_tab( void )
{
  neighbor_tab.clear();
  a_router.clear();
}

bool fwd_router::set_pfad( const callsign &nb, char tp, const String &adr )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	{
          if (nb == it->get_call() || samecall(nb,it->get_call()))
	  {
	     it->update_pfad(nb,tp,adr);
//    	     neighbor_tab_entry eintrag(nb,tp,adr);
//	     neighbor_tab.push_back(eintrag);
	     geaendert = true;
	     return true;
	  }
	}
	return false;
    }
  else
    return false;
}

bool fwd_router::reset( const callsign &nb )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	{
          if (nb == it->get_call() || samecall(nb,it->get_call()))
	  {
	     it->reset(nb);
	     return true;
	  }
	}
	return false;
    }
  else
    return false;
}


bool fwd_router::add_partner(   const callsign &nb, char tp, const String &adr )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	{
	  if (nb.get_ssid() != 0)
	    {
	      if (nb == it->get_call())
		return false;
	    }
	  else
	    {
	      if (samecall(nb,it->get_call()))
		return false;
	    }
	}
      String pfad = base_fwddirname+nb.str()+String('/');
      DIR *test = opendir(strtochar(pfad));
      bool flag;
      if (test == NULL)
	{
	  mode_t mode = S_IRUSR+S_IWUSR+S_IXUSR+S_IRGRP+
	                S_IXGRP+S_IROTH+S_IXOTH;
	  flag = (mkdir(strtochar(pfad),mode) == 0);
	}
      else
	{
	  flag = true;
	  closedir(test);
	}
      if (flag)
	{
	  neighbor_tab_entry eintrag(nb,tp,adr);
	  neighbor_tab.push_back(eintrag);
	  geaendert = true;
	  a_router.add(nb);
	  return true;
	}
      else
	return false;
    }
  else
    return false;
}

bool fwd_router::del_partner( const callsign &nb )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  {
	    neighbor_tab.erase(it);
	    geaendert = true;
	    a_router.del(nb);
	    return true;
	  }
      return false;
    }
  else
    return false;
}

bool fwd_router::add_destin( const callsign &nb, const destin &ds )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  if (it->add_destin(ds))
	    {
	      geaendert = true;
	      return true;
	    }
	  else
	    return false;
    }
  return false;
}

bool fwd_router::del_destin( const callsign &nb, const destin &ds )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  if (it->del_destin(ds))
	    {
	      geaendert = true;
	      return true;
	    }
	  else 
	    return false;
    }
  return false;
}

bool fwd_router::enable_autorouting( const callsign &nb )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  {
	    it->enable_autorouting();
	    geaendert = true;
	    return true;
	  }
    }
  return false;
}

bool fwd_router::disable_autorouting( const callsign &nb )
{
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  {
	    it->disable_autorouting();
	    geaendert = true;
	    return true;
	  }
    }
  return false;
}


void fwd_router::log_message(nachricht &msg)
{
  fwdlog fwd_log(configuration);
  funkruf_nachricht              *fmsg;
  skyper_rubrik_nachricht        *smsg;
  datenbankaenderung_nachricht   *cmsg;
  datenbankupdate_nachricht      *umsg;
  datenbankanforderung_nachricht *rmsg;
  zeit_nachricht                 *zmsg;
  zielgebiets_nachricht          *dmsg;

  n_types typ = msg.get_typ();
  String logline;
  logline = msg.m_id + ' ';

  switch(typ)
    {
      case n_funkrufe          : logline.append("F:");
	                         fmsg = (funkruf_nachricht*) &msg;
                                 logline.append(fmsg->text);
                                 break;
      case n_aenderungen       : logline.append("C:");
	                         cmsg = (datenbankaenderung_nachricht*) &msg;
                                 logline.append(cmsg->rufzeichen.call()+" ");
                                 logline.append(cmsg->typ);
                                 break;
      case n_update            : logline.append("U:");
	                         umsg = (datenbankupdate_nachricht*) &msg;
                                 logline.append(umsg->rufzeichen.call()+" ");
                                 logline.append(umsg->typ);
                                 break;
      case n_updateanforderung : logline.append("R:");
	                         rmsg = (datenbankanforderung_nachricht*) &msg;
                                 rmsg->tm.set_darstellung(zeit::f_zeitdatum_s);
                                 logline.append(rmsg->tm.get_zeit_string());
                                 break;
      case n_skyper_board      : logline.append("S:");
	                         smsg = (skyper_rubrik_nachricht*) &msg;
                                 logline.append(smsg->text);
                                 break;
      case n_zeit              : logline.append("Z:");
	                         zmsg = (zeit_nachricht*) &msg;
                                 logline.append(zmsg->typ);
                                 break;
      case n_destination       : logline.append("D:");
	                         try
				   {
				     dmsg = (zielgebiets_nachricht*) &msg;
				     logline.append(dmsg->zielgebiet.get_string());
				     logline.append(':');
				     logline.append(dtoS(dmsg->delay,6,2));
				   }
				 catch( Error_destin_checksum_error )
				   {
				     logline.append("Zielgebiet korrupiert.");
				   }
                                 break;
      default                  : logline.append("Unknown");
    }
  fwd_log.eintrag(logline,FWDLOGMASK_RXMSG);
}

void fwd_router::read_spool( void )
{
  DIR *fwd_dir;
  fwdlog fwd_log(configuration);
  int count_delete = 0; 
  
  fwd_dir = opendir(strtochar(fwddirname));
  struct dirent *eintrag;

  if (fwd_dir != NULL)
    {
      while ((eintrag = readdir(fwd_dir)) != NULL )
	{
	  String fname = String(eintrag->d_name);
	  if (fname != String(".") && fname != String("..") )
	    {
	      try
		{
		  routing_descriptor ddesc(fname);
		  if (ddesc.expired())
		    {
		      ddesc.delete_nachricht();
		      count_delete++;
		    }
		  else
		    {
		      for ( list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
			{
			  if ( ddesc.check_nachbar(it->get_call() ) )
			    {
			      if (fname.slen() > 0)
				{
				  if ( fname[0] == 'F' ) 
				    it->n_pers++;
				  else if ( fname[0] == 'S' || fname[0] == 'B' ) 
				    it->n_bul++;
				  else if ( fname[0] == 'D' )
				    it->n_dest++;
				  else
				    it->n_sonst++;
				}
			    }
			}
		      descriptors.push_back(ddesc);
		    }
		}
	      catch( Error_could_not_open_msg_file )
		{
		  fwd_log.eintrag("Fwd-Router: Konnte Nachrichtendatei beim Einlesen des Routing-Spools nicht oeffnen.",FWDLOGMASK_FWDERR);
		}
	      catch( Error_could_not_read_msg_file )
		{
		  fwd_log.eintrag("Fwd-Router: Konnte Nachrichtendatei beim Einlesen des Routing-Spools nicht lesen.",FWDLOGMASK_FWDERR);
		  // Wenn die Nachrichtendatei nicht mehr gelesen werden kann
		  // wird sie an dieser STelle geloescht.
		  remove(strtochar(fwddirname+fname));
		}
	    }
	}
      closedir(fwd_dir);
    }
  if (count_delete != 0)
    fwd_log.eintrag("Fwd-Router: "+itoS(count_delete)+" Nachrichten beim Einlesen des Routing_spools geloescht.",FWDLOGMASK_INIT); 
}


bool fwd_router::check_mid( nachricht &msg )
{
  nachricht *msg_ptr = &msg;

  if ( ( msg.get_typ() == n_zeit || msg.get_typ() == n_destination ) )
    return mid_tab_routinginf.check_mid(msg_ptr->m_id);
  else
    return mid_tab.check_mid(msg_ptr->m_id);
}
 
bool fwd_router::route_message(nachricht& msg, bool check_mid_flag, callsign from, callsign to)
{
  fwdlog fwd_log(configuration);
  nachricht *msg_ptr = &msg;
  zeit_nachricht zmsg("");
		  
  if (!samecall(from,G_mycall))
    {
      n_types typ = msg.get_typ();
      stat.spool(true,typ);
    }

  fwd_log.eintrag("Router : Route Nachricht von "+from.call()+" nach "+to.call(),FWDLOGMASK_ROUTER);
  if (activ)
    {
      if ( !check_mid_flag || !check_mid(msg) )
	{
	  log_message(msg);
	      
	  if (!samecall(from,G_mycall))
	    {
	      if ( msg_ptr->get_typ() == n_destination )
		{
		  zielgebiets_nachricht *dmsg_ptr = (zielgebiets_nachricht* ) msg_ptr;
		  a_router.rx_d_message(*dmsg_ptr,from);
		}
	      else if ( msg_ptr->get_typ() != n_zeit )
		{
		  //do_rcvd_msg(msg);
		  fwd_execution exec_modul;
		  exec_modul.exec_msg(msg,from);
		}
	      else
		{
		  fwd_log.eintrag("Zeitnachricht empfangen von "+from.str(),FWDLOGMASK_ROUTER);		  
		  zeit_nachricht *zmsg_ptr = (zeit_nachricht*) &msg;
		  if ( rx_rtt_messung(from,*zmsg_ptr,zmsg) )
		    {
		      fwd_log.eintrag("Anforderung erkannt, Antwort erzeugen",FWDLOGMASK_ROUTER);
		      msg_ptr = (nachricht*) &zmsg;
		      to = from;
		      from = G_mycall;
		    }
		  else
		    return activ;
		}
	    }
	      
	  fwd_log.eintrag("Router : MID ok.",FWDLOGMASK_ROUTER);
	  try
	    {
	      routing_descriptor desc(*msg_ptr,from);
	      fwd_log.eintrag("Router : Routing-Descriptor angelegt.",FWDLOGMASK_ROUTER);
	      fwd_log.eintrag("Router : Routing-Ziele eintragen.",FWDLOGMASK_ROUTER);
	      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
		{
		  fwd_log.eintrag("Router : Ziel : "+it->get_call().str(),FWDLOGMASK_ROUTER);
		  if (!samecall(it->get_call(),from))
		    {
		      n_types typ;
		      typ = msg_ptr->get_typ();
		      bool flag = false;
		      if (typ == n_funkrufe)
			{
			  fwd_log.eintrag("Router : Nachricht ist eine Funkruf-Nachricht.",FWDLOGMASK_ROUTER);
			  fwd_log.eintrag("Router : Ziel ueberpruefen.",FWDLOGMASK_ROUTER);
			  funkruf_nachricht fmsg = (*(funkruf_nachricht*) msg_ptr);
			  flag = it->check_destin(fmsg.dest);
			}
		      else if (typ == n_skyper_board)
			{
			  fwd_log.eintrag("Router : Nachricht ist eine Skyper-Rubrik-Nachricht.",FWDLOGMASK_ROUTER);
			  fwd_log.eintrag("Router : Ziel ueberpruefen.",FWDLOGMASK_ROUTER);
			  skyper_rubrik_nachricht smsg = (*(skyper_rubrik_nachricht*) msg_ptr);
			  flag = it->check_destin(smsg.dest);
			}
		      else if (typ == n_aenderungen)
			flag = true;
		      else if (typ == n_update || typ == n_updateanforderung )
			flag = samecall(it->get_call(),to);
		      else if (typ == n_zeit )
			{
			  if (it->options.check_option(OPTION_ZEITMESSUNG) )
			    flag = samecall(it->get_call(),to);
			  else
			    flag = false;
			}
		      else if (typ == n_destination )
			{
			  if (it->options.check_option(OPTION_ROUTINGAUSTAUSCH) )
			    flag = samecall(it->get_call(),to);
			  else
			    flag = false;
			}
		      if (flag)
			fwd_log.eintrag("Router : Eintragen.",FWDLOGMASK_ROUTER);
		      else
			fwd_log.eintrag("Router : Nicht eintragen.",FWDLOGMASK_ROUTER);
		      
		      if (flag)
			{
			  fwd_log.eintrag("Router : Ziel wird in Nachbarliste eingetragen.",FWDLOGMASK_ROUTER);
			  desc.add_nachbar(it->get_call());
			  if (typ == n_funkrufe)
			    it->n_pers++;
			  else if (typ == n_skyper_board)
			    it->n_bul++;
			  else if (typ == n_destination )
			    it->n_dest++;
			  else 
			    it->n_sonst++;
			  
			  stat.spool(false,typ);
			}
		    }
		}
	      if (!desc.is_empty())
		{
		  fwd_log.eintrag("Router : Routing-Descriptor ist nicht Leer und wird in Queue eingetragen.",FWDLOGMASK_ROUTER);
		  descriptors.push_back(desc); 
		}
	      else
		{
		  fwd_log.eintrag("Router : Routing-Descriptor ist Leer, Nachricht loeschen!",FWDLOGMASK_ROUTER);
		  desc.delete_nachricht();
		}
	    }
	  catch( Error_could_not_open_msg_file )
	    {
	      fwd_log.eintrag("Kann Datei im Routingspoolverzeichnis nicht anlegen.",FWDLOGMASK_FWDERR);
	    }
	  catch( Error_could_not_write_msg_file )
	    {
	      fwd_log.eintrag("Dann Datei im Routingspoolverzeicnis nicht schreiben.",FWDLOGMASK_FWDERR);
	      fwd_log.eintrag("Moeglicherweise liegt ein Zielgebiets-pruefsummenfehler vor.",FWDLOGMASK_FWDERR);
	    } 
	}
    }
  return activ;
  
}

bool fwd_router::message_avalable( const callsign &nachbar )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag("Router : Message Avalable ?",FWDLOGMASK_ROUTER);
  bool flag = false;
  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end() && !flag ; ++it ) 
        if ( samecall( it->get_call(), nachbar ) )
	  flag = (it->n_pers > 0) || (it->n_bul > 0) || (it->n_dest > 0 ) || 
	         (it->n_sonst > 0);

      if (flag)
	fwd_log.eintrag("Router : Ja!",FWDLOGMASK_ROUTER);
      else
	fwd_log.eintrag("Router : Nein!",FWDLOGMASK_ROUTER);
    }
  return flag;
}

nachricht & fwd_router::get_tx_message( const callsign &nachbar )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag("Router : get_tx_message !",FWDLOGMASK_ROUTER);

  fwd_log.eintrag("Router : Nachbar aus Routing-Tabelle holen",FWDLOGMASK_ROUTER);
  list<neighbor_tab_entry>::iterator nit = neighbor_tab.end();
  for (list<neighbor_tab_entry>::iterator i = neighbor_tab.begin(); i != neighbor_tab.end(); ++i )
    if (samecall(i->get_call(),nachbar))
      nit = i;
  if (nit == neighbor_tab.end())
    throw Error_request_for_non_neighbor();
  fwd_log.eintrag("Router : Routing-Tab-Eintrag liegt vor.",FWDLOGMASK_ROUTER);
  Mid mid = String("");
  static nachricht tmp_msg(mid);
  fwd_log.eintrag("Router : Statische Nachricht angelegt.",FWDLOGMASK_ROUTER);
  for (list<routing_descriptor>::iterator it = descriptors.begin() ; it != descriptors.end()  ; ++it )
    {
      fwd_log.eintrag("Router : Routing-descriptor testen.",FWDLOGMASK_ROUTER);
      if (it->check_nachbar(nachbar))
	{
	  fwd_log.eintrag("Router : Msg soll an Nachbar gehen.",FWDLOGMASK_ROUTER);
	  it->delete_nachbar(nachbar);
	  fwd_log.eintrag("Router : Nachbar entfernt.",FWDLOGMASK_ROUTER);
	  fwd_log.eintrag("Router : Als naechstes wird Nachricht ermittelt und zurueck gegeben.",FWDLOGMASK_ROUTER);
	  nachricht *nptr = &(it->get_nachricht());
	  n_types typ = nptr->get_typ();
	  if (typ == n_funkrufe)
	    nit->n_pers--;
	  else if (typ == n_skyper_board)
	    nit->n_bul--;
	  else if (typ == n_destination)
	    nit->n_dest--;
	  else // if (typ != n_zeit)
	    nit->n_sonst--;
	  fwd_log.eintrag("Router : Ueberpruefen, ob noch weitere Nachbarn im Descriptor stehen.",FWDLOGMASK_ROUTER);
	  if (it->is_empty())
	    {
	      fwd_log.eintrag("Router : Nein, Nachricht loeschen!",FWDLOGMASK_ROUTER);
	      it->delete_nachricht();
	      fwd_log.eintrag("Router : Und Descriptor aus Liste entfernen.",FWDLOGMASK_ROUTER);
	      descriptors.erase(it);
	    }
	  return *nptr;
	}
    }
  nit->n_pers = 0;
  nit->n_bul = 0;
  nit->n_dest = 0;
  nit->n_sonst = 0;
  throw Error_no_message_available();

  return tmp_msg;
}

bool fwd_router::start_connection(callsign &nachbar, char &typ, String &adresse , bool &autorouting)
{
  fwdlog fwd_log(configuration);

  if (activ)
    {
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin() ; it != neighbor_tab.end() ; ++it )
	{
	  if (it->status() == st_getrennt ||
	      it->status() == st_gescheitert &&
	      it->get_last_change() + wait_time(it) <= zeit()  )
	    {
	      if (it->n_pers  + it->int_n_pers  > START_FWD_PERS || 
		  it->n_bul   + it->int_n_bul   > START_FWD_BUL  ||
		  it->n_dest  + it->int_n_dest  > START_FWD_DEST ||
		  it->n_sonst + it->int_n_sonst > START_FWD_SONST ||
		  it->now_messung() ||
		  it->no_first_connect )
		{
		  nachbar = it->get_call();
		  typ = it->get_typ();
		  adresse = it->get_adress();
		  autorouting = ( it->get_autorouting() && a_router.is_activ() );
		  it->no_first_connect = false;
		  fwd_log.eintrag("Router: Verbindung starten "+String(typ)+":"+adresse,FWDLOGMASK_ROUTER);
		  return true;
		}
	    }
	}
      return false;
    }
  else 
    return false;
}

void fwd_router::connection_started( const callsign &nachbar )
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    {
      if ( (it->get_call() == nachbar) ||
	   ( (it->get_call().get_ssid() == 0) && samecall(it->get_call(),nachbar) ) )

	{
	  // Wenn noch eine Verbindung aktiv ist, 
	  // wird das Aufbauen ignoriert.
	  if ( it->aktive_verbindungen.size() == 0 )
	    it->status(st_aufbau);
	}
    }
}

void fwd_router::connection_established( const callsign &nachbar, const protokoll_optionen &opt , int tid)
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    {
      if (it->get_call().get_ssid() == 0)
	{
	  if (samecall(it->get_call(),nachbar))
	    {
	      it->status(st_aktiv);
	      it->options = opt;
	      it->aktive_verbindungen.push_back(tid);
	    }
	}
      else
	{
	  if (it->get_call() == nachbar)
	    {
	      it->status(st_aktiv);
	      it->aktive_verbindungen.push_back(tid);
	    }
	}
    }
}

void fwd_router::connection_closed( const callsign &nachbar , int tid)
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    {
      if ( (it->get_call() == nachbar) ||
	   ( (it->get_call().get_ssid() == 0) && samecall(it->get_call(),nachbar) ) )
	{
	  for ( vector<int>::iterator it2 = it->aktive_verbindungen.begin();
		it2 != it->aktive_verbindungen.end() ; ++it2 )
	    // Zunaechst einmal Thread-Id aus Thread-Liste streichen
	    if ( *it2 ==  tid )
	      {
		it->aktive_verbindungen.erase(it2);
		break;
	      }
	  // Wenn noch eine Verbindung aktiv ist, 
	  // wird das disconneten ignoriert.
	  if ( it->aktive_verbindungen.size() == 0 )
	    {
	      // Es gibt keine weiteren aktiven Verbindungen.
	      
	      // Wenn die Verbindung bereits waerend des Verbindungsaufbaus
	      // getrennt wird, kann man von einem Scheitern ausgehen
	      if (it->status() == st_aufbau )
		it->status(st_gescheitert);
	      else
		it->status(st_getrennt);
	    }
	}
    }
}

void fwd_router::connection_failed( const callsign &nachbar , int tid)
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    {
      if ( (it->get_call() == nachbar) ||
	   ( (it->get_call().get_ssid() == 0) && samecall(it->get_call(),nachbar) ) )
	{
	  if ( tid != -1 )
	    {
	      for ( vector<int>::iterator it2 = it->aktive_verbindungen.begin();
		    it2 != it->aktive_verbindungen.end() ; ++it2 )
		// Zunaechst einmal Thread-Id aus Thread-Liste streichen
		if ( *it2 ==  tid )
		  {
		    it->aktive_verbindungen.erase(it2);
		    break;
		  }
	    }
	  // Wenn noch eine Verbindung aktiv ist, wird das scheitern ignoriert.
	  if ( it->aktive_verbindungen.size() == 0 )
	    // Es gibt keine weiteren aktiven Verbindungen.
	    it->status(st_gescheitert);
	}
    }
}


vector<struct neighbor_info> fwd_router::get_infos( bool &akt )
{
  akt = activ;
  vector<struct neighbor_info> tmp;

  if (activ)
    for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
      tmp.push_back(it->get_info());

  return tmp;
}

vector<struct neighbor_info> fwd_router::get_infos( bool &akt, const callsign &call )
{
  akt = activ;
  vector<struct neighbor_info> tmp;

  if (activ)
    for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
      if (it->get_call() == call)
	tmp.push_back(it->get_info());

  return tmp;
}


bool fwd_router::is_nachbar( char typ, callsign& nachbar, bool &autorouting )
{
  if (activ)
    for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
      {
	if (typ == it->get_typ())
	  {
	    callsign eintrags_call = it->get_call();
	    if (eintrags_call.get_ssid() == 0)
	      {
		if (samecall(nachbar,eintrags_call))
		  {
		    nachbar = nachbar.get_nossidcall();
		    autorouting = ( it->get_autorouting() && a_router.is_activ() );
		    return true;
		  }
	      }
	    else
	      {
		if ( nachbar == eintrags_call )
		  {
		    autorouting = ( it->get_autorouting() && a_router.is_activ() );
		    return true;
		  }
	      }
	  }
      }
  return false;
}


String fwd_router::get_hostname( const String &adr )
{
  unsigned int l = adr.slen();
  for (unsigned int i = 0; i < l ; i++ )
    {
      if (adr[i] == ':')
	return adr.copy(0,i);
    }
  throw Error_wrong_ip_address();
}

String  fwd_router::get_portstring( const String &adr )
{
  unsigned int l = adr.slen();
  for (unsigned int i = 0; i < l ; i++ )
    {
      if (adr[i] == ':')
	if (l-i>1)
	  return adr.copy(i+1,l-i-1);
	else
	  throw Error_wrong_ip_address();
    }
  throw Error_wrong_ip_address();
}

bool fwd_router::is_nachbar( uint32_t adr, callsign &call, bool &autorouting )
{
  if (activ)
    {
      syslog logf(configuration);
      fwdlog fwd_log(configuration);

      fwd_log.eintrag("Router : IP-Nachbar ? ",FWDLOGMASK_RTNCHBR);
      fwd_log.eintrag("Router : Eingangsadresse : "+itoS(adr,8,true),FWDLOGMASK_RTNCHBR);
      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	{
	  if (it->get_typ() == 'T' )
	    {
	      String hname = get_hostname(it->get_adress());
	      fwd_log.eintrag("Router : erkannter Hostname : "+hname,FWDLOGMASK_RTNCHBR); 
	      struct hostent *ent = gethostbyname(strtochar(hname));
	      if (ent != NULL)
		{
		  if (ent->h_addrtype == AF_INET && ent->h_length == 4)
		    {
		      char **ptr = ent->h_addr_list;
		      char *cptr;
		      while ((cptr = *ptr) != NULL)
			{
			  uint32_t ad = 0;
			  for (int i = 0; i < 4 ; i++ )
			    {
			      ad = (ad >> 8 | ((unsigned int) *cptr << 24));
			      cptr++;
			    }
			  fwd_log.eintrag("Router : Adresse : "+itoS(ad,8,true),FWDLOGMASK_RTNCHBR);
			  if (ad == adr)
			    {
			      call = it->get_call();
			      autorouting = ( it->get_autorouting() && a_router.is_activ() );
			      return true;
			    }
			  ptr++;
			}
		    }
		}
	      else
		{
		  fwd_log.eintrag("Hostname Lookup-Fehler : "+adr,FWDLOGMASK_FWDERR);
		}
	    }
	}
      return false;
    }
  return false;
}


Mid fwd_router::get_mid( bool autorouter_flag )
{
  if (autorouter_flag)
    return mid_tab_routinginf.get_newmid();
  else
    return mid_tab.get_newmid();
}

void fwd_router::process_rtt_messung( void )
{
  fwdlog fwd_log(configuration);

  for ( list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    if ( it->options.check_option(OPTION_ZEITMESSUNG) )
      if (it->status() == st_aktiv )
	if ( it->now_messung() )
	  {
	    fwd_log.eintrag(it->get_call(),"zeitmessung durchfuehren",FWDLOGMASK_TIMEMES);
	    route_message(do_rtt_messung(),false,G_mycall,it->get_call());
	    fwd_log.eintrag(it->get_call(),"Zeitmessnachricht an Router gegeben.",FWDLOGMASK_TIMEMES);
	    it->messung_started();
	  }
}

nachricht & fwd_router::do_rtt_messung( void )
{
  nachricht *msg_ptr;
  static zeit_nachricht zmsg("MID");

  Mid m = mid_tab_routinginf.get_newmid();
  zmsg = zeit_nachricht(m);
  zmsg.version=TIME_PROT_VERSION;
  zmsg.li = 0;
  zmsg.typ = 'C';
  zmsg.stratum = 15;
  zmsg.praez = 0;
  zmsg.ident = "LOCL";
  zmsg.t_ref = pzeit(0.);
  zmsg.t_rx1 = pzeit(0.);
  zmsg.t_tx = pzeit(0.);
  zmsg.t_orig = pzeit();

  msg_ptr = (nachricht*) &zmsg;
  return *msg_ptr;
}


bool fwd_router::rx_rtt_messung( const callsign &nb, const zeit_nachricht &z1, zeit_nachricht &z2 )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag(nb,"Zeitmessnachricht empfangen",FWDLOGMASK_TIMEMES);
  if (z1.typ == 'C')
    {
      fwd_log.eintrag(nb,"Zeitmessanforderung erhalten, Antwort wird erzeugt",FWDLOGMASK_TIMEMES);
      z2.t_rx1 = pzeit();
      z2.m_id = mid_tab_routinginf.get_newmid();
      z2.version=TIME_PROT_VERSION;
      z2.li = 0;
      z2.typ = 'S';
      z2.stratum = 15;
      z2.praez = 0;
      z2.ident = "LOCL";
      z2.t_ref = pzeit(0.);
      z2.t_orig = z1.t_orig;
      z2.t_tx = pzeit(0.);
      return true;
    }
  else
    {
      pzeit t_rx2 = pzeit();
      fwd_log.eintrag(nb,"Zeitmessantwort erhalten, Zeiten werden errechnet und eingetragen",FWDLOGMASK_TIMEMES);
      double rtt = (t_rx2 - z1.t_orig) - (z1.t_tx - z1.t_rx1);
      double offset = ((z1.t_rx1 - z1.t_orig) + (z1.t_tx - t_rx2))/2.;

      for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
	if (nb == it->get_call())
	  {
	    it->messung(rtt,offset);
	    a_router.rtt_messung(nb,it->get_rtt());
	  }
      return false;
    }
}

void fwd_router::set_spooled_messages( const callsign& call, int p, int b, 
				       int d, int s )
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    if (call == it->get_call())
      {
	it->int_n_pers  = p;
	it->int_n_bul   = b;
	it->int_n_dest  = d;
	it->int_n_sonst = s;
	return ;
      }
}

void fwd_router::set_interface_parameter( const callsign &call, int t_w,
					  int n_max, int unack, 
					  int fehler_zaehler )
{
  for (list<neighbor_tab_entry>::iterator it = neighbor_tab.begin(); it != neighbor_tab.end(); ++it )
    if (call == it->get_call())
      {
	it->t_w            = t_w;
	it->n_max          = n_max;
	it->unack          = unack;
	it->fehler_zaehler = fehler_zaehler;
      }
}

fwdstatistic fwd_router::get_stat( void )
{
  return stat;
}
