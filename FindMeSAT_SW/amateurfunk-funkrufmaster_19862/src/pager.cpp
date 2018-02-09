/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <signal.h>

#include "globdef.h"
#include "database.h"
#include "config.h"
#include "cmdline.h"
#include "user.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "texte.h"
#include "String.h"
#include "logfile.h"
#include "destin.h"
#include "gruppe.h"
#include "user_interface.h"
#include "schedule.h"
#include "crontab.h"
#include "bake.h"
#include "zeit.h"
#include "board.h"
#ifdef COMPILE_SAT
#include "sat.h"
#endif
#include "import.h"
#include "fwd_router.h"
#include "fwd_autorouter.h"
#include "slaves.h"
#include "make_boards.h"
#ifdef COMPILE_DIGISTATUS
#include "digi.h"
#endif
#ifdef COMPILE_ASTRO
#include "astronom.h"
#endif
#ifdef COMPILE_TIDE
#include "gezeiten.h"
#endif
#include "default.h"
#include "talk.h"
#include "cluster_interface.h"
#include "trace.h"
#include "connector.h"
#include "cluster.h"
#include "system_info.h"

bool shutdown_flag = false;
zeit shutdown_time;
zeit start_time;
cpu_messung glob_cpu_messung;

// Globale Definition des Konfigurationsdatensatzes. Alle Programmteile 
// koennen damit global
// Auf die Systemparameter zugreifen.
config_file configuration;
// Nun die globale Definition der Rufzeichendatenbank. Auch diese ist damit
// Global im gesamten Quelltext erreichbar.
callsign_database calls;

#ifdef COMPILE_SLAVES
// Die Spoolverzeichnisse werden ebenso global deklariert
//
spoolfiles spool;
slave_control slaves;
#endif

callsign G_mycall;

fwd_router router;
autorouter a_router;

#ifdef COMPILE_SAT
sat satelliten;
#endif

#ifdef COMPILE_TIDE
gezeiten_control tiden;
#endif

#ifdef COMPILE_CLUSTER
cluster_control cluster_cntl;
#endif

#ifdef COMPILE_WX
wx_control wx;
#endif

#ifdef COMPILE_DIGISTATUS
digi_control digi;
#endif

talker talk;

trace tracer;

connection_control connector;

cluster_if dxc;

/*************************************************************************
 
Der Signal-Handler faengt eine Reihe externer Signale ab um einen 
kontrollierten Shutdown und eine entsprechende Meldung
im Logfile zu ermoeglichen.

*/

void signal_handler( int signal )
{
  syslog logf(configuration);
  switch(signal)
    {
      case SIGABRT : logf.eintrag("Programm extern abgebrochen",LOGMASK_SIGNALS);
                     shutdown_flag = true;
		     shutdown_time = zeit();
	  	     break;
      case SIGTERM : logf.eintrag("Programm erhielt TERM-Signal",LOGMASK_SIGNALS);
	             shutdown_time = zeit();
	             shutdown_flag = true;
		     break;
      case SIGKILL : logf.eintrag("Programm erhielt Kill-Signal",LOGMASK_SIGNALS);
	             shutdown_time = zeit();
	             shutdown_flag = true;
		     break;
      case SIGPIPE : logf.eintrag("SIGPIPE-Signal empfangen",LOGMASK_SIGNALS);
	             break;
      case SIGSEGV : logf.eintrag("Speicherschutzverletzung!",LOGMASK_SIGNALS);
	             exit(-1);
    }
}

/***********************************************************************

Mit install_signal_handler wird die programmeigene Signalbearbeitung beim 
Betriebssystem eingetragen.

*/

void install_signal_handler( void )
{
  signal(SIGABRT,&signal_handler);
  signal(SIGTERM,&signal_handler);
  signal(SIGKILL,&signal_handler);
  signal(SIGPIPE,&signal_handler);
  signal(SIGSEGV,&signal_handler);
}

void do_cron_jobs( crontab & ctab, t_baken &baken, boards &rubriken, syslog &logf )
{

  vector<crontab_command> cmds = ctab.test_crontab();
  for (vector<crontab_command>::iterator it = cmds.begin() ; it != cmds.end() ; it++)
    {
      if (it->get_typ() == ct_cmd_bake)
	{
	  logf.eintrag("Bakenaussendung",LOGMASK_CNTBPRCS);
	  for (t_baken::iterator it2 = baken.begin() ; it2 != baken.end() ; it2++)
	    (*it2)->send();
	}
      else if (it->get_typ() == ct_cmd_boards)
	{
	  logf.eintrag("Aussendung der Boardnamen",LOGMASK_CNTBPRCS);
	  rubriken.send_names();
	}
      else if (it->get_typ() == ct_cmd_purge )
	{
	  logf.eintrag("Purgen der Rubrikeninhalte und Funkrufe",LOGMASK_CNTBPRCS);
	  rubriken.purge();
#ifdef COMPILE_SLAVES
	  slaves.purge(); // Veraltete Funkrufe loeschen
#endif
	}
      else if (it->get_typ() == ct_cmd_brdmsg)
	{
	  logf.eintrag("Aussendung der Rubrikennachrichten",LOGMASK_CNTBPRCS);
	  int anz = rubriken.send_messages();
	  char puf[80];
	  sprintf(puf,"%d Nachrichten ausgesendet",anz);
	  logf.eintrag(String(puf),LOGMASK_CNTBPRCS);
	}
      else if (it->get_typ() == ct_cmd_dbsave)
	{
	  logf.eintrag("Speichern der Datenbank",LOGMASK_CNTBPRCS);
	  calls.save();
	}
#ifdef COMPILE_WX
      else if (it->get_typ() == ct_cmd_wx )
        {
	  logf.eintrag("Wetterstationsabfrage",LOGMASK_CNTBPRCS);
	  wx.start();
	}
#endif
#ifdef COMPILE_DIGISTATUS
      else if (it->get_typ() == ct_cmd_digi )
        {
	  logf.eintrag("Digistatus",LOGMASK_CNTBPRCS);
	  digi.start();
	}
#endif
#ifdef COMPILE_SAT
      else if (it->get_typ() == ct_cmd_sat )
        {
	  logf.eintrag("Sat-Modul",LOGMASK_CNTBPRCS);
	  satelliten.process();
	}

#endif
#ifdef COMPILE_TIDE
      else if (it->get_typ() == ct_cmd_gezeiten )
        {
	  gezeiten gezeit;
	  logf.eintrag("Gezeitenberechnung",LOGMASK_CNTBPRCS);
	  gezeit.process();
	}
#endif	
#ifdef COMPILE_ASTRO
      else if (it->get_typ() == ct_cmd_astro )
        {
	  logf.eintrag("Astro-Daten",LOGMASK_CNTBPRCS);
	  astro_daten astro;
	  astro.process();
	}
#endif
#ifdef COMPILE_SLAVES
      else if (it->get_typ() == ct_cmd_statistics )
	{
	  logf.eintrag("Funkruf-Statistik ausgesendet",LOGMASK_CNTBPRCS);
	  spoolstatistic stat = spool.get_stat();
	  String msg =stat.page_msg();
	  try
	    {
	      board brd(RUB_STATISTIK,configuration);
	      int bid = brd.get_brd_id();
	      int slot = brd.get_slot();
	      destin ds = get_default_destin();
	      brd.set_msg(msg,slot,ds);
	      spool.spool_bul(G_mycall,zeit(),bid,slot,msg,false,ds,128);
	    }
	  catch( Error_could_not_open_boardfile )
	    {}
	  catch( Error_boarddirectory_not_defined )
	    {}
	  catch( Error_could_not_create_boardfile )
	    {}
	}
#endif
    }
}

void main_loop( syslog &logf, t_baken &baken, cmdline &cmd )
{
  static unsigned int zaehler;

  crontab ctab(configuration);
  logf.eintrag("Crontab eingelesen",LOGMASK_PRGRMMDG);
#ifdef _DEBUG_ELOGS_
  cerr << "Crontab: " << endl;
  cerr << ctab << endl;
#endif
  try
    {
      t_pt_bake ptr = new status_bake(configuration);
      baken.push_back(ptr);
    }
  catch( Error_could_not_create_beacon )
    {
      logf.eintrag("Fehler: Statusbakendefinition fehlerhaft. Bake wird nicht erzeugt",LOGMASK_PRGRMERR);
    }
  logf.eintrag("Bakenaussendung",LOGMASK_PRGRMMDG);
  for (t_baken::iterator it = baken.begin() ; it != baken.end() ; it++)
    (*it)->send();
#ifdef COMPILE_ASTRO
  // Hier werden zum Programmstart die Astro-Daten berechnet und ausgesendet.
  astro_daten astro;
  astro.process();
#endif
#ifdef COMPILE_TIDE
  {
    // Hier werden zum Programmstart die Tiden berechnet und ausgesendet.
    gezeiten gez;
    gez.process();
  }
#endif
  schedule sch(baken,cmd);
  try
    {
      boards rubriken;
      bool shutdown_msg1 = false;
      bool shutdown_msg2 = false;
      while (!shutdown_flag || shutdown_time - zeit() > 0)
	{

	  if (shutdown_flag && shutdown_time - zeit() < 60 && !shutdown_msg1)
	    {
	      talk.send_msg("Master Shutdown in less than 1 minute");
	      shutdown_msg1 = true;
	    }
	  if (shutdown_flag && shutdown_time - zeit() <  15 && !shutdown_msg2)
	    {	    
	      talk.send_msg("Master Shutdown, Disconnect in a few seconds");
	      shutdown_msg2 = true;
	    }

	  sch.do_process();
	  do_cron_jobs(ctab,baken,rubriken,logf);
	  router.process_rtt_messung();
	  a_router.cyclic();	  
	  if (zaehler++ >= 10)
	    {
	      zaehler = 0;
	      import_system imp(configuration,baken);
	      imp.import();
	    }
	  
	}
      sch.do_shutdown();
    }
  catch( Error_could_not_create_boardnames )
    {
      logf.eintrag("Rubriken konnten nicht erzeugt werden.",LOGMASK_PRGRMERR);
    }
}



/*******************************************************************

read_database liest die Rufzeichendatenbank ein und faengt alle dabei 
moeglicherweise entstehenden Exceptions ab. Entsprechende Fehlermeldungen
werden ausgegeben.

*/

bool read_database( void )
{
  try
    {
      // Instanz der Rufzeichendatenbank anlegen
      String db_filename;
      db_filename = configuration.find("DATABASE");
      calls = callsign_database(db_filename);
      cerr << zeit() << ": Benutzerdatenbank erfolgreich geladen." << endl;
      return true;
    }
  catch( Error_parameter_not_defined )
    {
      cerr << "Die Rufzeichendatenbank konnte nicht geoeffnet werden, da " << endl;
      cerr << "Die Position im Konfigurationsfile nicht festgelegt wurde. " << endl;
      return false;
    }
  catch( Error_wrong_data_time_string_format )
    {
      cerr << "Fehler beim Einlesen einer Zeit: Format der Zeitinformation" << endl;
      cerr << "falsch." << endl;
      return false;
    }
  catch( Error_wrong_database_version )
    {
      cerr << "Falsche Versionsnummer der Rufzeichendatenbank" << endl;
      cerr << "Bitte fuehren Sie zuerst eine Umwandlung der Datenbank durch." << endl;
      return false;
    }
  catch( Error_can_not_create_database )
    {
      cerr << "Kann Datenbank nicht einlesen oder erzeugen." << endl;
      cerr << "Moeglicherweise stimmt die Pfadangabe in der Konfiguration nicht." << endl;
      return false;
    }
}

/*******************************************************************

load_config liest die Programmkonfiguration ein und faengt die dabei 
moeglicherweise entstehenden Exceptions ab.

*/

bool load_config( void )
{
  try
    {
      configuration.load();
      cerr << zeit() << ": Konfigurationsdatei geladen." << endl;
      return true;
    }
  // Alle anderen moeglichen Exceptions abfangen
  catch( Error_could_not_open_configfile )
    {
      cerr << "Das Konfigurationsfile konnte nicht geoeffnet werden." << endl;
      return false;
    }
  catch (Error_config_file_format_error ec)
    {
      cerr << "Das Konfigurationsfile hat ein falsches Datenformat" << endl;
      cerr << "bei Parameter " << ec.get_error_line() << endl;
      return false;
    }
}

#ifdef COMPILE_SLAVES
bool init_slave_control( t_baken &baken )
{
  try
    {
      if (slaves.load(baken))
	{
	  cerr << zeit() << ": Slavekonfiguration erfolgreich geladen." << endl;
	  return true;
	}
      else
	{
	  cerr << zeit() << ": Slavekonfiguration nicht geladen." << endl;
	  return false;
	}
    }
  catch( Error_wrong_parameter_name_in_slave_conf ec)
    {
      cerr << "Fehler: Falscher Parametername ";
      cerr << ec.get_parameter_name() << " in der Slave-Konfiguration.";
      cerr << endl;
      return false;
    }
  catch( Error_wrong_file_format_in_slave_conf ec)
    {
      cerr << "Fehler: Falsches Dateiformat in der Slave-Konfiguration in " << endl;
      cerr << ec.get_error_line() << endl;
      return false;
    }
  catch( Error_spooldir_undefined )
    {
      cerr << "Fehler: Spool-Verzeichnis in der Konfigurationsdatei nicht definiert." << endl;
      return false;
    }
  catch( Error_wrong_slave_callsign )
    {
      cerr << "Fehler: Fehlerhaftes Slave-Rufzeichen in der Konfiguration." << endl;
      return false;
    }
  catch( Error_slavedef_starts_with_wrong_parameter )
    {
      cerr << "Fehler: Slavekonfiguration beginnt mit falschem Schluesselwort." << endl;
      return false;
    }
}
#endif

bool init_forwarding( void )
{
  try
    {
      if (configuration.find("FORWARDING") == "JA")
	{
	  router.load_fwd();
	  cerr << zeit() << ": Forwarding initialisiert." << endl;
	}
      else
	cerr << zeit() << ": Forwarding nicht aktiviert." << endl;
      return true;
    }
  catch( Error_could_not_init_router )
    {
      cerr << "Fehler, kann Forwarding nicht initialisieren." << endl;
      cerr << "Um Einzelheiten zu erfahren, konsultieren Sie bitte das" << endl;
      cerr << "Systemlogfile." << endl;
      return false;
    }
  catch( Error_parameter_not_defined )
    {
      cerr << "Parameter 'FORWARDING' nicht definiert. Forwarding wurde nicht" << endl;
      cerr << "initialisiert." << endl;
      return true;
    }
}
 
void boot_master( cmdline &cmd )
{
  t_baken baken;

  if (load_config() && read_database() 
#ifdef COMPILE_SLAVES
      && init_slave_control(baken)
#endif 
      && init_forwarding() )
    {
      // Autorouter initialisieren
      a_router.init();
#ifdef COMPILE_CLUSTER
      cluster_cntl.init();
#endif
      syslog logf(configuration);
      if (make_boards())
	{
	  install_signal_handler();
	  logf.eintrag(String("Funkruf-Benutzerinterface gestartet, Version ")+String(VERSION),LOGMASK_STARTSTOP);
	  try
	    {
	      G_mycall = callsign(configuration.find("MYCALL"));
#ifdef COMPILE_SAT
	      satelliten.load();
#endif
#ifdef COMPILE_TIDE
	      tiden.load();
#endif	
#ifdef COMPILE_WX
	      wx.load(configuration);
#endif
#ifdef COMPILE_DIGISTATUS
	      digi.load(configuration);
#endif

	      defaults defs;
	      defs.process();
	      main_loop(logf,baken,cmd);
	      logf.eintrag("Funkrufbenutzerinterface shutdown",LOGMASK_STARTSTOP);
	    }
	  
	  // Ab hier werden jetzt nur noch Exceptions abgefangen
	  //
	  catch( Error_parameter_not_defined )
	    {
	      logf.eintrag("Parameter Mycall nicht definiert",LOGMASK_PRGRMERR);
	    }
	  catch( Error_no_callsign )
	    {
#ifdef _DEBUG_ELOGS_
	      cerr << "Der Parameter Mycall enthaelt kein gueltiges Rufzeichen." << endl;
	      cerr << "Eintrag ist >" << configuration.find("MYCALL") << "<" << endl;
#endif
	      logf.eintrag("Parameter MYCALL enthaelt kein gueltiges Rufzeichen.",LOGMASK_PRGRMERR);
	    }
	  catch( Error_could_not_create_boardnames )
	    {
	      logf.eintrag("Boardnamenaussendung nicht moeglich.",LOGMASK_PRGRMERR);
	    }
	  catch( Error_could_not_open_groups )
	    {
	      logf.eintrag("Parameter GRUPPEN nicht definiert.",LOGMASK_PRGRMERR);
	    }
	  catch( Error_could_not_generate_own_dests )
	    {
	      logf.eintrag("Parameter DESTINATION nicht definiert.",LOGMASK_PRGRMERR);
	    }
	  catch( Error_no_mem_for_String )
	    {
#ifdef _DEBUG_ELOGS_
	      cerr << "Nicht genuegend Speicher zum Anlegen eines Strings vorhanden." << endl;
#endif
	      logf.eintrag("Nicht genuegend Speicher beim Anlegen eines String",LOGMASK_PRGRMERR);
	    }
	}
      else
	{
	  logf.eintrag("Kann Boarddateien nicht anlegen",LOGMASK_PRGRMERR);
	}
    }
  else
    {
      cerr << "Programm konnte nicht richtig initialisiert werden." << endl;
      cerr << "Programm wird beendet." << endl;
    }
}


int main( int argc, char *argv[] )
{
  try
    {
      start_time = zeit();
      glob_cpu_messung = cpu_messung();
      glob_cpu_messung.init();
#ifdef COMPILE_AX25
      init_ax25();
#endif // COMPILE_AX25
      cmdline cmd(argc,argv);
      cout << "FunkrufMaster, (c) 2002-2004 by DH4DAI, DH6BB" << endl;
      time_t t = time(NULL);
      srand(t);
      cout << "Version " << VERSION << endl;
      cout << endl;
      cout << "Fuer den FunkrufMaster besteht KEINERLEI GARANTIE; FunkrufMaster ist" << endl;
      cout << "freie Software, die Sie unter bestimmten Bedingungen weitergeben duerfen;" << endl;
      cout << "Details erfahren Sie aus der GNU General Public License, die dem Quellpaket" << endl;
      cout << "dieser Software beiligt oder die Sie unter http://www.gnu.org/licenses" << endl;
      cout << "erhalten." << endl;

        boot_master(cmd);
      cout << endl << endl << endl;
      cout << "Funkruf-Server herunter gefahren!" << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return 0;
    }
  catch( Error_no_mem_for_String )
    {
      cerr << "Nicht genuegend Speicher zum Anlegen eines Strings vorhanden." << endl;
      cerr << "Logfile noch nicht definiert." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
  catch( bad_alloc )
    {
      cerr << "Fehler beim Allozieren von Systemspeicher." << endl;
      cerr << "Logfile noch nicht definiert." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
  /*
  catch( bad_cast )
    {
      cerr << "Fehler bei der dynamischen Typenueberpruefung" << endl;
      cerr << "Logfile noch nicht definiert." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
  catch( bad_typeid )
    {
      cerr << "Fehlerhafte Typenid." << endl;
      cerr << "Logfile noch nicht definiert." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
  */
  catch( bad_exception )
    {
      cerr << "Fehlerhafte Ausnahmespezifikation." << endl;
      cerr << "Logfile noch nicht definiert." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
#ifdef COMPILE_AX25
  catch( Error_ax25_config_load_ports )
    {
      cerr << "Kann ax25-Port Konfiguration nicht lesen." << endl;
      cout << endl << endl << "Und tschuess ..." << endl;
      return -1;
    }
#endif // COMPILE_AX25
}
