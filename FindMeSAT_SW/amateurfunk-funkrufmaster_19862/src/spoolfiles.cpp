/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
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
 *                                                                          *
 ****************************************************************************/

#include "spoolfiles.h"
#include "logfile.h"
#include "fwd_autorouter.h"
#include "export.h"

extern callsign G_mycall;
extern spoolfiles spool;
extern config_file configuration;
extern autorouter a_router;


// Konstruktor der Klasse Spoolfiles
spoolfiles::spoolfiles( void )
{
  spooldirs.clear();
}


void spoolfiles::add_dir( const String &dirname, const own_destins &d )
{
  struct entry eintrag;

  eintrag.pfad_name = dirname;
  eintrag.destinations = d;

  spooldirs.push_back(eintrag);
  it = spooldirs.begin();
  a_router.slave_connect( d.get_dests() );
}

void spoolfiles::del_dir( const String &dirname )
{
  for (t_spooldirs::iterator lit = spooldirs.begin() ; lit != spooldirs.end() ; ++lit )
    {
      if (lit->pfad_name == dirname )
	{
	  a_router.slave_discon(lit->destinations.get_dests());
	  spooldirs.erase(lit);
	  break;
	}
    }
}

void spoolfiles::first( void )
{
  // Iterator auf Anfang der Liste setzen
  it = spooldirs.begin();
}
void spoolfiles::next( void )
{
  ++it;
  if (it == spooldirs.end())
    throw Error_no_more_directories();
}

bool spoolfiles::check_destin( const destin &d )
{
  try
    {
      if (it != spooldirs.end())
	return it->destinations.check_destin(d);
      else
	throw Error_no_more_directories();
    }
  catch( Error_destin_checksum_error )
    {
      syslog logf(configuration);
      logf.eintrag("Pruefsummenfehler beim Spoolen einer Nachricht",LOGMASK_PRGRMERR);
      return false;
    }
}

void spoolfiles::get_file( bool aktivierung, ofstream &spoolfile, unsigned int prio )
{
  if (it != spooldirs.end())
    {
      // Prioritaet auf zulaessigen Bereich einschraenken
      if (prio > 255)
	prio = 255;

      String fname;
      int i = 0;
      bool found = false;
      // Maximal 100 mal versuchen, ein File zu oeffnen
      while (i < 100 && !found)
	{
	  ostringstream fn;
	  fn << it->pfad_name;
	  // Filename besteht aus FUNKRUF. bzw. FUNKRUF.SK. und einer Zufallszahl
	  fn << "FUNKRUF.";
	  fn << setfill('0') << setw(3) << prio << '.';

	  if (aktivierung)
	    fn << "SK.";
	  
	  fn << rand() % 32768 << ends;
	  fname = "";
	  fname.append(fn);
	  if (!ifstream(strtochar(fname)))
	    {
	      // Testen ob das File bereits existiert.
	      spoolfile.open(strtochar(fname));
	      found = true;
	    }
	  i++;
	}
      if (!found) throw Error_could_not_open_file();
      // Nur dann zum naechsten Spoolverzeichnis uebergehen, wenn es
      // sich _nicht_ um eine Dateianforderung fuer eine Aktivierungs-
      //sequence handelte!
      if (!aktivierung) ++it; 
    }
  else
    throw Error_no_more_directories();
}


bool spoolfiles::spool_msg(bool activierung, callsign sender, const callsign &master, zeit t_master, adress adr, String msg, bool from_fwd, const destin &d, unsigned int prio)
{
  // Als erstes wird die Nachricht ueber die Export-Funktion 
  // ausgegeben.

  try
    {
      export_sys exp;
      exp.write_user(sender,master,t_master,adr,msg,from_fwd,d,prio);
    }
  catch( Error_could_not_initialize_exportsystem )
    {
      // Hat nicht geklappt...
    }
  syslog logf(configuration);
  stat.spool(!from_fwd,true);
  first();
  bool flag1 = true,flag2 = false;

  spoollog logs(configuration);
  logs.eintrag(sender,master,t_master,d,msg,prio,SPOOLLOGMASK_PERS);

  // ueberfluessige Leerzeichen am Ende des Funkrufes entfernen
  msg.kuerze();
  // Laenge des Funkrufs jetzt auf 80 Zeichen begrenzen
  if (msg.slen() > 80)
    msg = msg.copy(0,80);


  while (flag1)
    {
      try
	{
	  ofstream sp;
	  // Im Falle einer Aktivierung erst die Aktivierungssequence und
	  // dann die Nachricht erzeugen
	  if (activierung)
	    spool_actseq(adr);
	  if (check_destin(d))
	    {
	      get_file(false,sp,prio);
	      sender.set_nossid(true);
	      sp << adr << endl;
	      sp << sender << ':' << msg << endl;
	      sp.close();
	    }
	  else
	    next();
	  flag2 = true;
	}
      catch( Error_no_more_directories )
	{
	  flag1 = false;
	}
      catch( Error_could_not_open_file )
	{
	  String dname = it->pfad_name;
	  del_dir(dname);
	  logf.eintrag("Kann keine Datei in Spoolverzeichnis oeffnen.", LOGMASK_PRGRMERR);
	  logf.eintrag("Verzeichnis disabled!",LOGMASK_PRGRMERR);
	  logf.eintrag("Pfad : "+dname,LOGMASK_PRGRMERR);
	  flag1 = false;
	}
    }
  return flag2;
}

bool spoolfiles::spool_msg(adress adr, String msg, const destin &d, unsigned int prio)
{
  syslog logf(configuration);
  stat.spool(true,true);
  first();
  bool flag1 = true,flag2 = false;

  // ueberfluessige Leerzeichen am Ende des Funkrufes entfernen
  msg.kuerze();
  // Laenge des Funkrufs jetzt auf 80 Zeichen begrenzen
  if (msg.slen() > 80)
    msg = msg.copy(0,80);

  spoollog logs(configuration);
  logs.eintrag(G_mycall,G_mycall,zeit(),d,msg,prio,SPOOLLOGMASK_PERS);

  while (flag1)
    {
      try
	{
	  ofstream sp;
	  // Im Falle einer Aktivierung erst die Aktivierungssequence und
	  // dann die Nachricht erzeugen
	  if (check_destin(d))
	    {
	      get_file(false,sp,prio);
	      sp << adr << endl;
	      sp << msg << endl;
	      sp.close();
	    }
	  else
	    next();
	  flag2 = true;
	}
      catch( Error_no_more_directories )
	{
	  flag1 = false;
	}
      catch( Error_could_not_open_file )
	{
	  String dname = it->pfad_name;
	  del_dir(dname);
	  logf.eintrag("Kann keine Datei in Spoolverzeichnis oeffnen.", LOGMASK_PRGRMERR);
	  logf.eintrag("Verzeichnis disabled!",LOGMASK_PRGRMERR);
	  logf.eintrag("Pfad : "+dname,LOGMASK_PRGRMERR);
	  flag1 = false;
	}
      catch( Error_destin_checksum_error )
	{
	  logf.eintrag("Pruefsummenfehler beim Spoolen einer Aktivierung",LOGMASK_PRGRMERR);
	}
    }
  return flag2;
}

bool spoolfiles::spool_bul( const callsign &master, zeit t_master, int board, int slot, String msg, bool from_fwd, const destin &d, unsigned int prio, bool rep_flag)
{
  // Als erstes wird die Nachricht ueber die Export-Funktion 
  // ausgegeben.
  try
    {
      export_sys exp;
      exp.write_bul(master,t_master,board,slot,msg,from_fwd,d,prio,rep_flag);
    }
  catch( Error_could_not_initialize_exportsystem )
    {
      // Hat nicht geklappt...
    }
  syslog logf(configuration);
  stat.spool(!from_fwd,false);
  first();
  bool flag1 = true,flag2 = false;

  spoollog logs(configuration);
  String logmsg = "("+itoS(board)+","+itoS(slot)+")"+msg;

  if (rep_flag)
    logs.eintrag(master,master,t_master,d,logmsg,prio,SPOOLLOGMASK_REP);
  else
    logs.eintrag(master,master,t_master,d,logmsg,prio,SPOOLLOGMASK_BUL);

  // ueberfluessige Leerzeichen am Ende des Funkrufes entfernen
  msg.kuerze();

  // Laenge des Funkrufs jetzt auf 80 Zeichen begrenzen
  if (msg.slen() > 80)
    msg = msg.copy(0,80);

  while (flag1)
    {
      try
	{
	  ofstream sp;
	  if (check_destin(d))
	    {
	      get_file(false,sp,prio);
	      sp << "4520.3\r";
	      sp << (char) (board + 0x1f);
	      sp << (char) (slot + 0x20);
	      for (unsigned int i = 0;i < msg.slen(); i++ )
		sp << (char) ((int) msg[i]+1);
	    } 
	  else
	    next();
	  flag2 = true;
	}
      catch( Error_no_more_directories )
	{
	  flag1 = false;
	}
      catch( Error_could_not_open_file )
	{
	  String dname = it->pfad_name;
	  del_dir(dname);
	  logf.eintrag("Kann keine Datei in Spoolverzeichnis oeffnen.", LOGMASK_PRGRMERR);
	  logf.eintrag("Verzeichnis disabled!",LOGMASK_PRGRMERR);
	  logf.eintrag("Pfad : "+dname,LOGMASK_PRGRMERR);
	  flag1 = false;
	}
    }
  return flag2;
}

bool spoolfiles::spool_brd_name( String name, int brd)
{
  const int prio = 160;

  syslog logf(configuration);
  first();
  bool flag1 = true,flag2 = false;

  spoollog logs(configuration);
  logs.eintrag(G_mycall,G_mycall,zeit(),destin(),name+String(" : ")+itoS(brd),prio,SPOOLLOGMASK_NAMES);

  while (flag1)
    {
      try
	{
	  ofstream sp;
	  get_file(false,sp,prio);
	  sp << "4512.3\r";
	  sp << '1';
	  sp << (char) (brd + 0x1f);
	  sp << (char) (10 + 0x20);
	  int l = name.slen();
	  for (int i = 0;i < l;i++)
	    sp << (char) ((int) name[i] + 1);
	  sp.close();
	  flag2 = true;
	}
      catch( Error_no_more_directories )
	{
	  flag1 = false;
	}
      catch( Error_could_not_open_file )
	{
	  String dname = it->pfad_name;
	  del_dir(dname);
	  logf.eintrag("Kann keine Datei in Spoolverzeichnis oeffnen.", LOGMASK_PRGRMERR);
	  logf.eintrag("Verzeichnis disabled!",LOGMASK_PRGRMERR);
	  logf.eintrag("Pfad : "+dname,LOGMASK_PRGRMERR);
	  flag1 = false;
	}
       catch( Error_destin_checksum_error )
	{
	  logf.eintrag("Pruefsummenfehler beim Spoolen eines AktivierungBulletins",LOGMASK_PRGRMERR);
	}
   }
  return flag2;
}

void spoolfiles::show(ostream &strm, char cr )
{
  strm << "Spoolverzeichnisse:" << cr;
  strm << "------------------------------------------------------" << cr;

  try
    {
      for (t_spooldirs::iterator i = spooldirs.begin(); i != spooldirs.end(); ++i )
	{
	  strm << i->pfad_name << cr;
	  strm << "-->" << i->destinations << cr;
	  strm << "Anzahl: " << get_count(i->pfad_name) << cr << cr;
	}
    }
  catch( Error_destin_checksum_error )
    {
      strm << cr << "Error: Checksum-Error in Destination!" << cr;
      strm << "Please contact Sysop!" << cr;
    }
}

unsigned int spoolfiles::get_count(String &spool_dir)
{
        syslog logf(configuration);
	DIR *dir;
	struct dirent *de;
	int cnt=0;

	if ( !(dir = opendir(strtochar(spool_dir))) ) 
	{
    		logf.eintrag("Kann Spool-Dir nicht oeffnen: " + spool_dir,LOGMASK_PRGRMERR);
		return 0;
	}
	while ((de = readdir(dir)) != NULL) 
	{
		if (!strstr(de->d_name, "FUNKRUF"))
			continue;
		cnt++;
	}
	closedir(dir);
	return (cnt);
}
