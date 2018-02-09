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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 * List of other authors:                                                   *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *                                                                          *
 ****************************************************************************/

#include "digi.h"
#include "spoolfiles.h"
#include "board.h"
#include "logfile.h"
#include "callsign.h"

String digi_config_file::digi_typen[5]={"UNKNOWN", "FLEX", "TNN", "XNET" };

/* Hier mal einige Gedanken dazu von DH6BB:

Linkstatus:
12345678901234567890
DB0WHV<>DB0LER 11/12
DB0WHV<>DB0BHV 1+/3+     (Laufzeit 100/300 bis 199/399)
DB0WHV<>DB0PDF 2+/1*     (Laufzeit 200/1000 bis 299/1999)
DB0WHV<>DB0TNC --/--     (Kein Link)
DB0WHV<>DB0SM  (-/-)     (Link weg, gibt aber Umweg)
DB0WHV<>DB0SM  (./.)     (Laufzeit 10/10 bis 99/99, gibt aber schnelleren Umweg)

Wir nehmen die Zahlen von Flexnet. TNN-Zeiten teilen wir durch 100.
Das entspricht etwa den Flexnet-Zeiten.

Digi-Status:
12345678901234567890
DB0WHV      Up: 123d
Destinations  :  689
Zeile noch frei
Zeile noch frei


Bitte Kommentare.


Hier der Vorschlag von DH4DAI:

Die gesamte Rubrik "Digipeater" besteht aus 10 Slots a 4 Zeilen, also aus
40 Zeilen. Jeder Digipeater, der automatisch abgefragt wird, erhaelt eine
Statuszeile. Darunter koennen fuer jeden Digipeater beliebig viele Einstiege 
und Links konfiguriert werden, deren Status ebenfalls in einer Zeile 
dargestellt wird.

Es gibt die folgenden Formate:

Statuszeile eines Digipeaters:
==============================

12345678901234567890
DB0WTS/JO31NK ##:##*
              ------  Uptime

Das Format der Zeitausgabe der Klasse delta_t muss fuer die Uptime leicht
modifiziert werden, da normalerweise ein Leerzeichen vor der Dimension
eingefuegt wird. Dies ist hier aus Platzgruenden nicht moeglich.


Bei den Statuszeilen von Einstiegen und Links wurden als Grundlage die bei
RMNC/Flexnet zur Verfuegung stehenden Informationen benutzt. Fuer andere 
Knotenrechnersysteme muss geschaut werden, ob diese Informationen zur 
Verfuegung stehen oder ob andere Informationen angezeigt werden koennen

Statuszeile eines Einstiegs
===========================

12345678901234567890
70cm ###  #.## * ###
                 ---  Quality
          ------      Datenrate
     ---              Anzahl der QSOs
----                  Kuerzel des Einstiegs

Die Quality wird von Flexnet der Prozentsatz  der als korrekt bestaetigten
Info-Frames auf der Anzeige angezeigt. Es handelt sich um einen Integer 
zwischen 0 und 100

Die Datenrate wird bei Flexnet in KByte / 10min angegeben. Um einen direkten
Vergleich zur Nenndatenrate zu haben, kann diese Datenrate in Bit/s 
(1 KByte / 10min = 13,653 Bit/s ) umgerechnet werden. Es werden beide 
Richtungen addiert.

Die Anzahl der QSOs auf diesem Port wird von Flexnet direkt ausgegeben

Als Kuerzel dient eine eindeutige Kennung dieses Einstiegs aus vier Zeichen,
z.B. '70cm' ' 2m ', ' 9k6' oder '76k8'

Statuszeile eines Links
=======================

12345678901234567890
0IUZ #.## * ## #####
               -----  Linkzeit
            --        Quality
     ------           Datenrate
----                  Kuerzel

Die Linkzeit wird in einer Notation angegeben, wie von Jens vorgeschlagen

Fuer die Quality stehen hier nur zwei Ziffern zur Verfuegung eine Quality von
100% wird mit '1+' angegeben.

Fuer die Datenrate gilt das gleiche, wie bei den Einstiegen

Das Kuerzel ist hier wiede eine eindeutige Kennung aus vier Buchstaben, 
ueblicherweise Ziffer und Suffix des Rufzeichens des Linkpartners

Zusatzfunktion Sysopalarm
=========================

Fuer den Fall, dass sich bestimmte Parameter derart verschlechtern, dass 
die Vermutung besteht, dass ein technisches Problem aufgetreten ist, kann
als Zusatzfunktion ein Funkruf zu einem technischen Verantwortlichen
ausgeloest werden. Folgende Alarmzustaende sind moeglich:

* Digipeateralarm
  Wird ausgeloest, wenn ein Digipeaterneustart erkannt wurde oder wenn ein
  Verbindungsaufbau zu diesem Digipeater gescheitert ist.
  Ein Digipeaterneustart ist daran zu erkennen, dass die Uptime kleiner ist,
  als bei der letzten Abfrage.

* Einstiegsalarm
  Wird ausgeloest, wenn die Uebertragungsqualitaet auf diesem Einstieg 
  signifikant abgesunken ist.

* Linkalarm
  Wird ausgeloest, wenn die Uebertragungsqualitaet auf diesem Link 
  signifikant abgesunken ist oder die Linkzeiten signifikant angestiegen sind.

Was als signifikante abnahme oder anstieg gewertet wird, muss noch genau
definiert werden.

Die Konfiguration erfolgt ueber eine Konfigurationsdatei, deren Format wie 
folgt aussehen koennte:


#
# Kommentare, 
#
#
# Hier beginnt jetzt die Konfiguration eines Digipeaters
#
DIGI <rufzeichen>
#
# Zur Digikonfiguration gehoeren Pfad, Digityp, etc
#
PFAD <connect_pfad>
#
TYP [FLEX|TNN|...]
#
ALARM <rufzeichen>,<rufzeichen>,...
#
# gibt an, wer bei einem Digipeateralarm zu alarmieren ist
#
# Nun beginnt die Konfiguration eines Einstiegs
#
EINSTIEG <kuerzel> <port-nr>
#
ALARM <rufzeichen>,<rufzeichen>,...
#
ENDE
#
# Ende der Einstiegsdeklaration
#
# Nun beginnt die Linkdeklaration
#
LINK <kuerzel> <port-nr>
#
ALARM <rufzeichen>,<rufzeichen>,...
#
ENDE
#
# Ende der Linkdeklaration
#
#
ENDE
#
# Ende der Digipeaterdeklaration
#
#


Fehlt noch was in der Konfiguration ?

Kommentare zu meinem Vorschlag ???
*/

extern config_file configuration;
extern spoolfiles spool;
extern callsign G_mycall;
extern digi_control Digi;

digi_control::digi_control(void)
{
  start_flag = false;
  start_first = false;
  activ = false;
  last_fetch = zeit(-1);
}

digi_control::~digi_control(void)
{
}

/*
  ----------------------------------------------------------------------------
  zunaechst kommen alle Methoden der Konfigurations-Datei-Klasse
  digi_config_files
  ----------------------------------------------------------------------------
*/
digi_config_file::digi_config_file( void )
{
  slot = 0;
};

digi_config_file::digi_config_file( String & dname )
{
  read(dname);
}

void digi_config_file::read( String & dname )
{
  ifstream cfg_file(strtochar(dname));
  if (!cfg_file) 
    throw Error_could_not_open_digi_configfile();

  else
    {
      dateiname = dname;
      String line;
      link_anzahl=0;
      enabled=false;
      
      while (cfg_file)
	{
	  line.getline(cfg_file,250);
	  int l = line.slen();
	  if (l > 0 &&  line[0] != '#')
	    {
	      try
		{
		  int p;
		  digi.set_format(false);
		  if ((p = line.pos(String('='))) != -1 )
		    {
		      String parameter = line.copy(0,p);
		      String wert = line.copy(p+1,l-p-1);
		      wert.kuerze();

		      if (parameter == "DIGI")
			digi = callsign(wert);			
		      else if (parameter == "SLOT")
			slot = wert.Stoi();			
		      else if (parameter == "STATUS" )
		      {
		            if (wert=="ENABLED")
		        	enabled=true;
		      }
		      else if (parameter == "TYP" )
		      {
		            if (wert=="TNN")
		        	typ=tnn;
			    else if (wert=="FLEX")
		        	typ=flexnet;
			    else if (wert=="XNET")
			        typ=xnet;
			    else
			    	typ=unknown;
		      }
		      else if (parameter == "PFAD")
			pfad = connect_string(wert);
		      else if (parameter == "LINK")
		      {
			link_call[link_anzahl] = callsign(wert);
			link_anzahl++;
		      }
		      else 
			throw Error_unknown_parameter_in_digi_configfile();			
		    }
		  else 
		    throw Error_wrong_format_in_digi_config_file();
		}
	      catch( Error_syntax_fehler_in_connect_string )
		{
		  throw Error_wrong_format_in_digi_config_file();
		}
	      catch( Error_no_callsign )
		{
		  throw Error_wrong_format_in_digi_config_file();
		}
	      catch( Error_not_a_locator )
		{
		  throw Error_wrong_format_in_digi_config_file();
		}
	    }
	}
    }
#ifdef _DEBUG_DIGI_
cerr << "DIGI: " << digi << "  Typ: " << digi_typen[typ] << "   Path: " << pfad << endl;
cerr << "Links: " << link_anzahl << endl;
#endif
}

void digi_config_file::PrintOn( ostream &strm )
{
  callsign call;
  digi.set_format(false);
  strm << "#" << endl;
  strm << "# Digi-configuration-file!" << endl;
  strm << "# automaticaly generated, please do not edit!" << endl;
  strm << "#" << endl;
  if (enabled)
  strm << "STATUS=ENABLED" << endl;
  else
  strm << "STATUS=DISABLED" << endl;  
  strm << "DIGI=" << digi << endl;
  strm << "PFAD=" << pfad << endl;
  strm << "TYP=" << digi_typen[typ] << endl;
  strm << "SLOT=" << slot << endl;
  for (int i=0; i<link_anzahl; i++)
  {
    try
    {
	call=callsign(link_call[i]);
    	strm << "LINK=" << link_call[i] << endl;
    }
    catch (Error_no_callsign)
    {
    }
  }
  strm << "#" << endl;
}

void digi_config_file::show(ostream &strm, char cr )
{
  digi.set_format(true);
  digi.set_nossid(false);
  strm <<  digi  << setw(2) << slot << setw (8) << digi_typen[typ] << "    " << link_anzahl << "   " << pfad << cr;
  digi.set_format(false);
}

void digi_config_file::full_show(ostream &strm, char cr )
{
  digi.set_format(true);
  digi.set_nossid(false);
  strm << "Digi                   : " << digi << cr;
  strm << "Digi-Typ               : " << digi_typen[typ] << cr;
  strm << "Slot                   : " << slot << cr;
  strm << "Connect-Pfad           : " << pfad << cr;
  for (int i=0; i<link_anzahl; i++)
    strm << "Link                   : " << link_call[i] << cr;
  digi.set_format(false);
  if (enabled)
  strm << "Status                 : Enabled" << cr;
  else
  strm << "Status                 : Disabled" << cr;  
}

void digi_config_file::save(void)
{
      ofstream ostr(strtochar(dateiname));
#ifdef _DEBUG_DIGI_
cerr << "SAVE Digistat" << endl;      
#endif
      if (ostr)
      {
	PrintOn(ostr);	
      }
}

digi_config_file::~digi_config_file( void )
{
}


void digi_control::start(void)
{
  if (activ)
    {
      start_flag = true;
      start_first = true;
    }
}


bool digi_control::set_status(const callsign &call, bool status)
{
  try
    {
      digi_config_file digicfg = digifiles.find(call);
      digicfg.enabled=status;
      if (digifiles.set(call,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}



bool digi_control::set_typ(const callsign &call, const String &typ)
{
  try
    {
      digi_config_file digicfg = digifiles.find(call);
      digicfg.typ=unknown;
      if (typ=="TNN")
        digicfg.typ=tnn;
      else if (typ=="FLEX")
        digicfg.typ=flexnet;
      else if (typ=="XNET")
        digicfg.typ=xnet;
      if (digifiles.set(call,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}

// Der Destruktor bleibt leer
digi_status::~digi_status( void )
{
}

digi_interface::digi_interface(String &outp, bool ax_flag, digi_config_file &digi_confg) : interfaces(outp,ax_flag)
{
  digi_cfg = digi_confg;  // Daten in klasseninterner Variablen speichern
  set_maske();
  mdg.gefundene_links=0;
  set_connect_path(digi_cfg.pfad);
  first_connect(outp);
  interface_id = 'I';
  last_activity = zeit();
  mdg.call = digi_cfg.digi;
}

// Der Destruktor bleibt leer
digi_interface::~digi_interface( void )
{
}


bool digi_interface::do_process( bool rx_flag, String &outp )
{
    digi_control Digi;
    outp = "";
    if (rx_flag)
    {
        String inp;
        while (get_line(inp))
	{
	  // Steht in der Zeile "reconnected" ? Dann wird der Prozess beendet 
	  if (inp.in("*** reconnected") || inp.in("Reconnected to"))
	    return false;
	  else
	  {
	      if (connect_prompt) 
	      {
		  if (maske != 0)
	    	    digi_line(inp);  
		  
		  if (maske == 0)
		    {
		      Digi.meldung(digi_cfg.slot,mdg,digi_cfg);
		      in_msg++;
#ifdef _DEBUG_DIGI_
			cerr << "Maske ist Null" << endl;
#endif
		      return false;
		    }
	      }
	      else 
	      {
#ifdef _DEBUG_DIGI_
cerr << "Connected" << endl;
#endif
		    String command;
		    switch (digi_cfg.typ)
		    {
			case flexnet:
			    command = "p *";
			    break;
			case xnet:
			    command = "s *\r\nl";
			    break;
			case tnn:
			    command = "s\r\nr v";
			    break;
			default:
			    return false;
		    }
		    outp.append(command + char(13));  // Befehl senden
		    connect_prompt=true;
	    	    last_activity = zeit();
	      }
	    }
	}
    }
    return true;
}


digi_config_files::digi_config_files( void )
{
  files.clear();
}

digi_config_files::digi_config_files( config_file& cfg )
{
  files.clear();
  syslog slog(cfg);
  
  try
    {
      digi_dir_name = cfg.find("DIGI");
      DIR *digi_dir;
      digi_dir = opendir(strtochar(digi_dir_name));
      if (digi_dir != NULL)
	{
	  struct dirent *entry;
	  while ((entry = readdir(digi_dir)) != NULL )
	    {
	      if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
		{
		  String fname(digi_dir_name + entry->d_name);
		  if (fname.in(String(".digi")))
		    try
		      {
			files.push_back(digi_config_file(fname));
		      }
		    catch(Error_could_not_open_digi_configfile)
		      {
			slog.eintrag("Digi-Configurationsfile "+fname+" nicht zu oeffnen",LOGMASK_PRGRMERR);
		      }
		    catch(Error_unknown_parameter_in_digi_configfile)
		      {
			slog.eintrag("Unbekannter Parameter in digi-Config-Datei "+fname,LOGMASK_PRGRMERR);
		      }
		    catch(Error_wrong_format_in_digi_config_file)
		      {
		     	slog.eintrag("Falsches Dateiformat in digi-Config-Datei "+fname,LOGMASK_PRGRMERR);
		      }
		}
	    }
	  closedir(digi_dir);
	}
    }
  catch( Error_parameter_not_defined )
    {
    }
}

bool digi_config_files::get_first( digi_config_file &f )
{
  it = files.begin();
  if (it != files.end())
    {    
      f = *it;
      return true;
    }
  else
    return false;
}

bool digi_config_files::get_next( digi_config_file &f )
{
  if (it == files.end())
    return false;
  else
    {
      it++;
      if (it != files.end())
	{    
	  f = *it;
	  return true;
	}
      else
	return false;
    }
}

bool digi_control::add_link( const callsign &digicall, const callsign &linkcall )
{
  try
    {
      digi_config_file digicfg = digifiles.find(digicall);
      for (int a=0; a<digicfg.link_anzahl; a++)
      {
	if(samecall(digicfg.link_call[a],linkcall))
	    return false;
      }      
      digicfg.link_call[digicfg.link_anzahl] = linkcall;
      digicfg.link_anzahl++;
      if (digifiles.set(digicall,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}


bool digi_control::del_link( const callsign &digicall, const callsign &linkcall )
{
  try
    {
      int a=0, b=0;
      digi_config_file digicfg = digifiles.find(digicall);
      for (a=0; a<digicfg.link_anzahl; a++)
      {
	if(samecall(digicfg.link_call[a],linkcall))
	{
	}
	else
	{
    	    digicfg.link_call[b] = digicfg.link_call[a];
	    b++;
	}
      }
      if (a==b)
        return false;
      digicfg.link_anzahl=b;
      if (digifiles.set(digicall,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;      
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}


bool digi_control::add( const callsign &call )
{
  digi_config_file digicfg;
  digicfg.digi = call;
  return digifiles.add(digicfg);
}


bool digi_control::del( const callsign &call )
{
  digi_config_file digicfg;
  digicfg.digi = call;
  return digifiles.del(call);
}


bool digi_config_files::add( digi_config_file digicfg )
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(digicfg.digi,i->digi))
	  return false;
    }

  String dname = digi_dir_name + digicfg.digi.str() + ".digi";
#ifdef _DEBUG_DIGI_
cerr << "Digi: " << dname << endl;  
#endif
  digicfg.dateiname = dname;
  digicfg.slot=1;
  digicfg.typ=unknown;
  digicfg.pfad=connect_string("no:n0cal>n0cal");
  digicfg.link_anzahl=0;
  digicfg.enabled=false;
  files.push_back(digicfg);
  digicfg.save();	
  return true;
}

bool digi_config_files::del( const callsign &call )
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->digi))
	{
	  remove(strtochar(i->dateiname));
	  files.erase(i);
	  it = files.begin();
	  return true;
	}
    }
  return false;
}


bool digi_config_files::set( const callsign &call, const digi_config_file &digicfg )
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->digi))
	{
	  *i = digicfg;
	  return true;
	}
    }
  return false;
}

digi_config_file& digi_config_files::find( const callsign &call )
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->digi))
	return *i;
    }
  throw Error_unknown_digi();
}

void digi_config_files::show( ostream &ostr , char cr )
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      i->show(ostr, cr);
    }
}

void digi_config_files::full_show(const callsign &call, ostream &ostr, char cr)
{
  for (vector<digi_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->digi))
      {
#ifdef _DEBUG_DIGI_
cerr << "Samecall: " << call << endl;
#endif
         i->full_show(ostr,cr);
      }
    }
}

void digi_control::load( config_file & cfg )
{
  digifiles = digi_config_files(cfg);
  ds = get_default_destin();
  try
    {
      String en = cfg.find("DIGI_STATUS");
      en.kuerze();
      if ( en == "JA" )
	activ = true;
      else
	activ = false;
    }
  catch ( Error_parameter_not_defined )
    {
      activ = false;
    }
}

void digi_control::show( ostream &ostr, char cr )
{
  digifiles.show(ostr,cr);
  ostr << cr << cr;
  ostr << "Status : ";

  if (activ)
    ostr << "  activ";
  else
    ostr << "inactiv";

  ostr << cr  << cr;
}

void digi_control::full_show(const callsign &call, ostream &ostr, char cr)
{
  digifiles.full_show(call, ostr, cr);
}

bool digi_control::start_connection( digi_config_file &digicfg )
{
  if (start_flag)
    {
      if (start_first)
	{
	  if ( !digifiles.get_first(digicfg) )
	    {
	      start_flag = false;
	      start_first = false;
	      return false;
	    }
	  else
	    {
	      start_first = false;
	      if (digicfg.enabled==false) 
	        return false;
	      last_fetch = zeit();
	      return true;
	    }
	}
      else
	if ( !digifiles.get_next(digicfg) )
	  {
	    start_flag = false;
	    return false;
	  }
	else
	  {
	    if (digicfg.enabled==false) 
		return false;
	    last_fetch = zeit();
	    return true;
	  }
    }
  else
    return false;
}

/*
  ----------------------------------------------------------------------------
  Nun folgen die Methoden der Klasse digi_meldung
  ----------------------------------------------------------------------------
*/

digi_meldung::digi_meldung( void )
{
    uptime = 0;
    destin = -1;
}

String digi_meldung::spool_msg_digi( void ) const
{
  // Erste Zeile enthaelt Rufzeichen
  int pos=1;
  zeit jetzt;
  callsign c = call;
  delta_t Uptime = uptime;

  c.set_format(true);
//  c.set_nossid(true);
  String tmp = c.call();
  jetzt.set_darstellung(zeit::f_zeit_s);
  pos=tmp.slen()+5;
  while(pos++<20) tmp.append(" ");
  tmp.append(jetzt.get_zeit_string());
  tmp.append("Uptime: ");
  tmp.append((Uptime.get_string()));
  pos=tmp.slen();
  while(pos++<40) tmp.append(" ");  
  tmp.append("Destin: ");
  tmp.append(itoS(destin,4));
  return tmp;
}

String digi_meldung::spool_msg_link(digi_config_file &digi_cfg) const
{
  String von = call.str();
  String an;
  if (von.slen()<6) von.append(" ");
  String tmp;
  for(int i=0; i<digi_cfg.link_anzahl; i++)
  {
    an=digi_cfg.link_call[i].str();
    if (an.slen()<6) an.append(" ");
    tmp.append(von+"<>"+an+" "+link_rtt[i]);
  }
  if (tmp.slen()<2) tmp="---";      
  return tmp;
}

void digi_control::meldung( int slot, const digi_meldung &mldg, digi_config_file &digi_cfg )
{
  String msg;
  msg = mldg.spool_msg_digi();
  spool_msg(slot,msg,RUB_DIGI_STAT);
// ToDo: Slots + >4 Links
//  if (mdg.link_anzahl>0)
  {
    msg = mldg.spool_msg_link(digi_cfg);
    spool_msg(slot,msg,RUB_LINK_STAT);
  }
}

void digi_control::spool_msg( int sl, const String &msg, String rubrik )
{
  int slot;
  syslog logf(configuration);

  try
    {  
      board brd(rubrik,configuration);
      int board = brd.get_brd_id();
      
      if (sl > 10)		//Rotierende Slots
	slot = brd.get_slot();
      else
	slot = sl;	//fester Slot
      
      brd.set_msg(msg,slot,ds);
      
      // Nachricht ins Spoolverzeichnis schreiben
      spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,ds,128);
    }
  // Moegliche Fehler-Exceptions abfangen
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Digi Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Digi Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}


void digi_interface::set_maske( void )
{
  maske=0;
  if ( digi_cfg.typ == flexnet )
  {
#ifdef _DEBUG_DIGI_
    cerr << "Typ ist Flexnet" << endl;
#endif
    maske |= MASKE_UPTIME;
    maske |= MASKE_DESTINATION;
    maske |= MASKE_LINK;
  }
  if ( digi_cfg.typ == tnn )
  {
#ifdef _DEBUG_DIGI_
    cerr << "Typ ist TNN" << endl;
#endif
    maske |= MASKE_UPTIME;
    maske |= MASKE_DESTINATION;
    maske |= MASKE_LINK;
  }
  if ( digi_cfg.typ == xnet )
  {
#ifdef _DEBUG_DIGI_
    cerr << "Typ ist XNET" << endl;
#endif
    maske |= MASKE_UPTIME;
    maske |= MASKE_DESTINATION;
    maske |= MASKE_NODES;
    maske |= MASKE_LINK;
  }
#ifdef _DEBUG_DIGI_
    cerr << "Maske: " << maske << endl;  
#endif
}

void digi_interface::digi_line( String line )
{
  switch (digi_cfg.typ)
  {
      case flexnet:
        flex_digi_line(line);
        break;
      case tnn:
        tnn_digi_line(line);
        break;
      case xnet:
        xnet_digi_line(line);
        break;
      default:
        break;
  }
}

void digi_interface::flex_digi_line( String line )
{
    last_activity = zeit();
    double up = 0;
    
    if ( maske & MASKE_DESTINATION )
    {
      if (line.in(String("d:")) && 
	  line.in(String("v:")) && 
	  line.in(String("t:")))
	{
	    int pos1=0, pos2=0;
	    String dests;
	    if((pos1=line.pos("d:")) && (pos2=line.pos("v:")))
	    {
		dests=line.part(pos1+2,pos2-pos1-2);
#ifdef _DEBUG_DIGI_
		cerr << ">>" << dests << "<<" << endl;
#endif
		mdg.destin = dests.Stoi();
		maske &= ~MASKE_DESTINATION;
	    }		
	}
    }

    if ( maske & MASKE_UPTIME )
    {
      if (line.in(String("d:")) && 
	  line.in(String("v:")) && 
	  line.in(String("t:")))
	{
	    int pos1=0, pos2=0, pos3=0;
	    String Uptime;
	    if((pos1=line.pos("t:")) && (pos2=line.pos(")")))
	    {
		Uptime=line.part(pos1+2,pos2-pos1-2);
		if ((pos3=Uptime.pos(",")))	// 7h, 23m
		{
		    String high, low;
		    high=Uptime.part(0,pos3);
		    low=Uptime.part(pos3+1,Uptime.slen()-pos3-1);	    
#ifdef _DEBUG_DIGI_
		    cerr << ">>" << Uptime << "<<>>" << high << "<<>>" << low << "<<" << endl;
#endif
		    if (high.in("d"))
		    {
			high.part(0,high.slen()-2); up+=high.Stoi()*24*60*60;
		    }
		    if (high.in("h"))
		    {
			high.part(0,high.slen()-2); up+=high.Stoi()*60*60;
		    }
		    if (high.in("m"))
		    {
			high.part(0,high.slen()-2); up+=high.Stoi()*60;
		    }
		    if (high.in("s"))
		    {
			high.part(0,high.slen()-2); up=high.Stoi();
		    }
		    if (low.in("h"))
		    {
			low.part(0,low.slen()-2); up+=low.Stoi()*60*60;
		    }
		    if (low.in("m"))
		    {
			low.part(0,low.slen()-2); up+=low.Stoi()*60;
		    }
		    if (low.in("s"))
		    {
			low.part(0,low.slen()-2); up+=low.Stoi();
		    }
		}
		else
		{
		    if (Uptime.in("s"))
		    {
			Uptime.part(0,Uptime.slen()-2); up+=Uptime.Stoi();
		    }
		}
#ifdef _DEBUG_DIGI_
		cerr << "Uptime: " << delta_t(up) << endl;
#endif
		mdg.uptime=delta_t(up);
		maske &= ~MASKE_UPTIME;
	    }		
	}
    }
    if ( maske & MASKE_LINK )
    {
	String Linkcall;
	String hin, rueck;
	String laufzeit;
	String ssid;

	if (line.slen()>70) 
	{
	    Linkcall=line.part(54,7);
	    try
	    {
		for(int anzahl=0; anzahl<digi_cfg.link_anzahl; anzahl++)
		{
//cerr << "Call:" << digi_cfg.link_anzahl << digi_cfg.link_call[anzahl] << endl;
    		    if (samecall(callsign(Linkcall),digi_cfg.link_call[anzahl]))
		    {
			laufzeit=line.part(66,line.slen()-66);	    
			ssid=line.part(60,6);
#ifdef _DEBUG_DIGI_
cerr << "SSID     :" << ssid << endl;
#endif
			mdg.link_rtt[anzahl]=rtt_calc(laufzeit.part(0,laufzeit.slen()),flexnet);
#ifdef _DEBUG_DIGI_
cerr << "RTT:" << anzahl << ":" << mdg.link_rtt[anzahl] << endl;			    
#endif
			mdg.gefundene_links++;			
    			anzahl=digi_cfg.link_anzahl;  //gefunden. Raus aus der Schleife
		    }
		}
		if (mdg.gefundene_links==digi_cfg.link_anzahl)
		    maske &= ~MASKE_LINK;
	    }
	    catch (Error_no_callsign)
	    {
	    }
	}
    }
}

void digi_interface::tnn_digi_line( String line )
{
    last_activity = zeit();
    double up = 0;
    if ( maske & MASKE_DESTINATION )
    {
      if (line.in(String("     Active Nodes:")))
	{
	    String dests;
	    dests=line.part(20,line.slen()-27);
	    mdg.destin = dests.Stoi();
	    maske &= ~MASKE_DESTINATION;
#ifdef _DEBUG_DIGI_
	    cerr << ">>" << mdg.destin << "<<" << endl;
#endif
	}
    }

    if ( maske & MASKE_UPTIME )
    {
      if (line.in(String("            Uptime:")))
	{
	    String Uptime;
	    int pos;
	    Uptime=line.part(20,line.slen()-20);
    	    if ((pos=Uptime.pos("/")))	// 21/22:23
	    {
		String Up;
		Up=Uptime.part(0,pos);
		up+=Up.Stoi()*24*60*60;
		Up=Uptime.part(pos+1,2);	    
		up+=Up.Stoi()*60*60;
		Up=Uptime.part(pos+4,2);	    
		up+=Up.Stoi()*60;
#ifdef _DEBUG_DIGI_
		cerr << "Uptime: " << delta_t(up) << endl;
#endif
		mdg.uptime=delta_t(up);
		maske &= ~MASKE_UPTIME;
	    }		
	}
    }
    if ( maske & MASKE_LINK )
    {
	String Linkcall;
	String hin, rueck;
	String laufzeit;
	String ssid;

	if (line.slen()>50) 
	{
	    Linkcall=line.part(0,9);
	    try
	    {
		for(int anzahl=0; anzahl<digi_cfg.link_anzahl; anzahl++)
		{
//cerr << "Call:" << Linkcall <<":"<< digi_cfg.link_call[anzahl] << endl;
    		    if (samecall(callsign(Linkcall),digi_cfg.link_call[anzahl]))
		    {
			laufzeit=line.part(26,15);	    
//			ssid=line.part(60,6);
//cerr << "SSID     :" << ssid << endl;
			mdg.link_rtt[anzahl]=rtt_calc(laufzeit.part(0,laufzeit.slen()),tnn);
#ifdef _DEBUG_DIGI_
cerr << "RTT:" << anzahl << ":" << mdg.link_rtt[anzahl] << endl;			    
#endif
			mdg.gefundene_links++;
    			anzahl=digi_cfg.link_anzahl;  //gefunden. Raus aus der Schleife
		    }
		}
		if (mdg.gefundene_links==digi_cfg.link_anzahl)
		    maske &= ~MASKE_LINK;
	    }
	    catch (Error_no_callsign)
	    {
	    }
	}
    }
}

void digi_interface::xnet_digi_line( String line )
{
    last_activity = zeit();
    double up = 0;
    if ( maske & MASKE_DESTINATION )
    {
      if (line.in(String("destinations        |")))
	{
	    String dests;
	    dests=line.part(21,9);
#ifdef _DEBUG_DIGI_
	    cerr << "Dest >>" << dests << "<<" << endl;
#endif
	    mdg.destin = dests.Stoi();
	    maske &= ~MASKE_DESTINATION;
	}		
    }

    if ( maske & MASKE_NODES )
    {
      if (line.in(String("nodes               |")))
      {
    	    String nodes;
	    nodes=line.part(21,9);
#ifdef _DEBUG_DIGI_
	    cerr << "Nodes >>" << nodes << "<<" << endl;
#endif
	    mdg.nodes = nodes.Stoi();
	    maske &= ~MASKE_NODES;
	}		
    }

    if ( maske & MASKE_UPTIME )
    {
      if (line.in(String("Uptime (")))
      {
	    String Uptime;
	    String high, low;

	    Uptime=line.part(8,8);		
    	    low=Uptime.part(4,4);
	    high=Uptime.part(0,4);	    
#ifdef _DEBUG_DIGI_
    	    cerr << ">>" << Uptime << "<<>>" << high << "<<>>" << low << "<<" << endl;
#endif
	    if (high.in("d"))
	    {
	        high.part(0,high.slen()-2); up+=high.Stoi()*24*60*60;
	    }
	    if (high.in("h"))
	    {
	        high.part(0,high.slen()-2); up+=high.Stoi()*60*60;
	    }
	    if (high.in("m"))
	    {
	        high.part(0,high.slen()-2); up+=high.Stoi()*60;
	    }
	    if (high.in("s"))
	    {
	        high.part(0,high.slen()-2); up=high.Stoi();
	    }
	    if (low.in("h"))
	    {
	        low.part(0,low.slen()-2); up+=low.Stoi()*60*60;
	    }
	    if (low.in("m"))
	    {
	        low.part(0,low.slen()-2); up+=low.Stoi()*60;
	    }
	    if (low.in("s"))
	    {
	        low.part(0,low.slen()-2); up+=low.Stoi();
	    }
#ifdef _DEBUG_DIGI_
    	    cerr << "Uptime: " << delta_t(up) << endl;
#endif
	    mdg.uptime=delta_t(up);
	    maske &= ~MASKE_UPTIME;
	}		
    }
    if ( maske & MASKE_LINK )
    {
	String Linkcall;
	String hin, rueck;
	String laufzeit;
	String ssid;

	if (line.slen()>75) 
	{
	    Linkcall=line.part(3,9);
	    try
	    {
		for(int anzahl=0; anzahl<digi_cfg.link_anzahl; anzahl++)
		{
//cerr << "Call:" << digi_cfg.link_anzahl << digi_cfg.link_call[anzahl] << endl;
    		    if (samecall(callsign(Linkcall),digi_cfg.link_call[anzahl]))
		    {
			laufzeit=line.part(23,9);	    
//			ssid=line.part(60,6);
//cerr << "SSID     :" << ssid << endl;
			mdg.link_rtt[anzahl]=rtt_calc(laufzeit.part(0,laufzeit.slen()),xnet);
#ifdef _DEBUG_DIGI_
cerr << "RTT:" << anzahl << ":" << mdg.link_rtt[anzahl] << endl;			    
#endif
			mdg.gefundene_links++;
    			anzahl=digi_cfg.link_anzahl;  //gefunden. Raus aus der Schleife
		    }
		}
		if (mdg.gefundene_links==digi_cfg.link_anzahl)
		    maske &= ~MASKE_LINK;
	    }
	    catch (Error_no_callsign)
	    {
	    }
	}
    }
    
}

String digi_interface::rtt_calc(String line, digi_typ typ)
{
    String rtt=" --- ";
    String hin, rueck;
    int Hin=-2;
    int Rueck=-2;
    bool umleitung=false;
    int pos1;
    
    if (typ==flexnet)
    {
#ifdef _DEBUG_DIGI_
cerr << "RTT-String:" << line << endl;
#endif
	if (!line.in("/"))
	{
	    if (line.in("-"))
		return rtt;
	    else
	    {
		Hin=line.Stoi();
		if (Hin<10)
		    return ("   "+itoS(Hin)+" ");
		if (Hin<100)
		    return ("  "+itoS(Hin)+" ");
		if (Hin<1000)
		    return (" "+itoS(Hin)+" ");
    		return (itoS(Hin)+" ");
	    }
	}
	if((pos1=line.pos("/")))
	{
            hin=line.part(0,pos1);
	    rueck=line.part(pos1+1,line.slen()-pos1-1);
	    if (hin[0]==String("("))
	    {
		umleitung=true;
		hin=hin.part(1,hin.slen()-1);
		rueck[line.pos("(")]=' ';
	    }
	    if (hin[0]==String("-"))
		Hin=-1;
	    if (rueck[0]==String("-"))
		Rueck=-1;
	    if (Hin!=-1 && Rueck!=-1)
	    {
    		Hin=hin.Stoi();
    		Rueck=rueck.Stoi();
	    }
	    if (Hin==-1 && Rueck==-1)
	    {
		if (umleitung)
		    rtt="(-/-)";
		else
		    rtt="--/--";
		return rtt;
	    }
	    if (umleitung)
	    {
    		if(Hin>=999)
		    rtt="(*/";
		else if (Hin>99)
		    rtt="(+/";
		else if (Hin>9)
		    rtt="(./";
		else
		    rtt="("+itoS(Hin)+"/";
	    }
	    else
	    {
    		if(Hin>=999)
		{
		    Hin=Hin/1000;
		    rtt=itoS(Hin)+"*/";
		}
		else if (Hin>99)
		{
		    Hin=Hin/100;
		    rtt=itoS(Hin)+"+/";
		}
		else if (Hin>9)
		    rtt=itoS(Hin)+"/";
		else
		    rtt=" "+itoS(Hin)+"/";
	    }
	    if (umleitung)
	    {
    		if(Rueck>=999)
		    rtt.append("*)");
		else if (Rueck>99)
		    rtt.append("+)");
		else if (Rueck>9)
		    rtt.append(".)");
		else
		    rtt.append(itoS(Rueck)+")");
	    }
	    else
	    {
    		if(Rueck>=999)
		{
		    Rueck=Rueck/1000;
		    rtt.append(itoS(Rueck)+"*");
		}
		else if (Rueck>99)
		{
		    Rueck=Rueck/100;
		    rtt.append(itoS(Rueck)+"+");
		}
		else if (Rueck>9)
		    rtt.append(itoS(Rueck));
		else
		    rtt.append(itoS(Rueck)+" ");
	    }    
	}
    }
    else if (typ==tnn)
    {
#ifdef _DEBUG_DIGI_
cerr << "RTT-String:" << line << endl;
#endif
	if (!line.in("/"))
	{
	    return rtt;
	}
	if((pos1=line.pos("/")))
	{
            hin=line.part(0,pos1);
	    rueck=line.part(pos1+1,line.slen()-pos1-1);

	    if (hin[hin.slen()-1]==String("-"))
		Hin=-1;
	    if (rueck[0]==String("-"))
		Rueck=-1;
	    if (Hin!=-1 && Rueck!=-1)
	    {
    		Hin=hin.Stoi();
    		Rueck=rueck.Stoi();
		Hin/=100;
		Rueck/=100;
	    }
	    if (Hin==-1 && Rueck==-1)
	    {
		return ("--/--");
	    }
    	    if(Hin>=999)
	    {
		Hin=Hin/1000;
		rtt=itoS(Hin)+"*/";
	    }
	    else if (Hin>99)
	    {
	        Hin=Hin/100;
	        rtt=itoS(Hin)+"+/";
	    }
	    else if (Hin>9)
	        rtt=itoS(Hin)+"/";
	    else
	        rtt=" "+itoS(Hin)+"/";
	    
    	    if(Rueck>=999)
	    {
	        Rueck=Rueck/1000;
	        rtt.append(itoS(Rueck)+"*");
	    }
	    else if (Rueck>99)
	    {
	        Rueck=Rueck/100;
	        rtt.append(itoS(Rueck)+"+");
	    }
	    else if (Rueck>9)
	        rtt.append(itoS(Rueck));
	    else
	        rtt.append(itoS(Rueck)+" ");	        
	}
    }
    else if (typ==xnet)
    {    
#ifdef _DEBUG_DIGI_
cerr << "RTT-String:" << line << endl;
#endif
	if (!line.in("/"))
	{
	    if (line.in("-"))
		return rtt;
	    else
	    {
		Hin=line.Stoi();
		if (Hin<10)
		    return ("   "+itoS(Hin)+" ");
		if (Hin<100)
		    return ("  "+itoS(Hin)+" ");
		if (Hin<1000)
		    return (" "+itoS(Hin)+" ");
    		return (itoS(Hin)+" ");
	    }
	}
	if((pos1=line.pos("/")))
	{
            hin=line.part(0,pos1);
	    rueck=line.part(pos1+1,line.slen()-pos1-1);
	    if (hin[hin.slen()-1]==String("-"))
		Hin=-1;
	    if (rueck[0]==String("-"))
		Rueck=-1;
	    if (Hin!=-1 && Rueck!=-1)
	    {
    		Hin=hin.Stoi();
    		Rueck=rueck.Stoi();
	    }
	    if (Hin==-1 && Rueck==-1)
	    {
		return ("--/--");
	    }
	    if (Hin>99)
	    {
	        Hin=Hin/100;
	        rtt=itoS(Hin)+"+/";
	    }
	    else if (Hin>9)
	        rtt=itoS(Hin)+"/";
	    else
	        rtt=" "+itoS(Hin)+"/";
	    
	    if (Rueck>99)
	    {
	        Rueck=Rueck/100;
	        rtt.append(itoS(Rueck)+"+");
	    }
	    else if (Rueck>9)
	        rtt.append(itoS(Rueck));
	    else
	        rtt.append(itoS(Rueck)+" ");
	}	        
    }
    return rtt;
}


bool digi_control::set_slot( const callsign &call, int slt )
{
  try
    {
      digi_config_file digicfg = digifiles.find(call);

      digicfg.slot = slt;
      if (digifiles.set(call,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}

bool digi_control::set_pfad( const callsign &call, const connect_string &pfd )
{
  try
    {
      digi_config_file digicfg = digifiles.find(call);

      digicfg.pfad = pfd;
      if (digifiles.set(call,digicfg))
      {
         digicfg.save();
	 return true;
      }
      return false;
    }
  catch( Error_unknown_digi )
    {
      return false;
    }
}

void digi_control::enable( config_file &cfg )
{
  cfg.set("DIGI_STATUS","JA");
  cfg.save();
  activ = true;
}

void digi_control::disable( config_file &cfg )
{
  cfg.set("DIGI_STATUS","NEIN");
  cfg.save();
  activ = false;
}
