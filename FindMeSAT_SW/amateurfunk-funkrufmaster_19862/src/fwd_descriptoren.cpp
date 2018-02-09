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

#include "fwd_descriptoren.h"

#include <fstream.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "logfile.h"

extern config_file configuration;


nachrichten_descriptor::nachrichten_descriptor()
{
  typ = n_keine;
  m = String("");
  n_t = 0;
  dateiname = "";
  t_create = zeit();
}

/* 
   nachrichten_descriptor::get_file versucht im mit String pfad angegebenen Verzeichnis
   einen freien Dateinamen fuer eine Nachricht des angegebenen Typs zu finden.
   Wenn ein freier Dateiname gefunden wurde, wird dieser in String &msgfile eingetragen und
   true zurueck gegeben. Andernfalls wird false zurueck gegeben.
*/

bool nachrichten_descriptor::get_file( n_types typ , String &msgfile , String pfad)
{
  String fname;
  int i = 0;
  bool found = false;
  // Maximal 100 mal versuchen, ein File zu oeffnen
  while (i < 100 && !found)
    {
      ostringstream fn;
      fn << pfad;
      switch (typ)
	{
	  case n_eigenschaften     : fn << 'E';
	                             break;
	  case n_funkrufe          : fn << 'F';
	                             break;
	  case n_aenderungen       : fn << 'C';
	                             break;
	  case n_update            : fn << 'U';
	                             break;
	  case n_updateanforderung : fn << 'R';
	                             break;
	  case n_keine             : fn << 'N';
	                             break;
	  case n_bestaetigung      : fn << 'A';
	                             break;
	  case n_skyper_board      : fn << 'S';
	                             break;
	  case n_zeit              : fn << 'Z';
	                             break;
	  case n_destination       : fn << 'D';
	}
      fn << "MSG.";
      fn << rand() % 32768 << ends;
      fname = "";
      fname.append(fn);
      if (!ifstream(strtochar(fname)))
	{
	  // Testen ob das File bereits existiert.
	  msgfile = fname;
	  found = true;
	}
      i++;
    }
  return found;
}

/*
  Dieser Konstruktor erzeugt aus der Nachricht und dem Pfadnamen des FWD-Queue-
  Verzeichnisses den Nachrichten Descriptor. Die Nachricht wird im Filesystem
  gespeichert.
  Wenn kein File geoeffnet werden konnte, wird eine Exception erzeugt.
*/
nachrichten_descriptor::nachrichten_descriptor( const nachricht& msg, String pfad)
{
  try
    {
      typ = msg.get_typ();
      m = msg.m_id;
      t_create = zeit();
      //bool flag = false;
      if (get_file(typ,dateiname,pfad))
	{
	  ofstream msgfile(strtochar(dateiname));
	  msgfile << msg;
	  t_last = zeit(0);
	  n_t = 0;
	}
      else
	throw Error_could_not_open_msg_file();
    }
  catch( Error_destin_checksum_error )
    {
      throw Error_could_not_write_msg_file();
    }
}

/* 
   Soll nach einem Programmabsturz oder einem aehnlichen unerwarteten Ereigniss die
   Sendequeue wieder restauriert werden, muessen aus den Nachrichten-Dateien wieder
   die Descriptoren erzeugt werden. Dazu dient dieser Konstruktor
*/
nachrichten_descriptor::nachrichten_descriptor( String dname)
{
  Mid mid = String("");
  eigenschaften_nachricht emsg(mid);
  funkruf_nachricht fmsg(mid);
  datenbankaenderung_nachricht cmsg(mid);
  datenbankupdate_nachricht umsg(mid);
  datenbankanforderung_nachricht rmsg(mid);
  skyper_rubrik_nachricht smsg(mid);
  zeit_nachricht zmsg(mid);
  zielgebiets_nachricht dmsg(mid);
		     	     	     	     		       
  ifstream msg_file(strtochar(dname));
  if (msg_file)
    {
      try
	{
	  char ch;
	  msg_file.get(ch);
	  msg_file.putback(ch);
	  switch(ch)
	    {
	      case 'E' : typ = n_eigenschaften;
	                 msg_file >> emsg;
	                 m = emsg.m_id;
		         break;
	      case 'F' : typ = n_funkrufe;
	                 msg_file >> fmsg;
		         m = fmsg.m_id;
		         break;
	      case 'C' : typ = n_aenderungen;
	                 msg_file >> cmsg;
		         m = cmsg.m_id;
		         break;
	      case 'U' : typ = n_update;
	                 msg_file >> umsg;
		         m = umsg.m_id;
		         break;
	      case 'R' : typ = n_updateanforderung;
	                 msg_file >> rmsg;
		         m = rmsg.m_id;
		         break;
	      case 'B' : typ = n_skyper_board;
	                 msg_file >> smsg;
		         m = smsg.m_id;
		         break;
	      case 'Z' : typ = n_zeit;
	                 msg_file >> zmsg;
		         m = zmsg.m_id;
		         break;
	      case 'D' : typ = n_destination;
	                 msg_file >> dmsg;
		         m = dmsg.m_id;
		         break;
	      default  : msg_file.close();
		         throw Error_could_not_read_msg_file();
	    }
	  n_t = 0;
	  t_last = zeit(0);
	  dateiname = dname;

	  // Erzeugungszeit ermitteln

	  struct stat stat_struct;

	  stat(strtochar(dateiname), &stat_struct);
	  t_create = stat_struct.st_mtime;

	}
      catch( Error_wrong_message_typ )
	{
	  msg_file.close();
	  throw Error_could_not_read_msg_file();
	}
      catch( Error_wrong_message_format )
	{
	  msg_file.close();
	  throw Error_could_not_read_msg_file();
	}
    }
  else
    throw Error_could_not_open_msg_file();
}


/*
   Diese Methode liest die Nachricht aus dem Dateisystem wieder ein und gibt
   eine Referenz auf eine entsprechende Nachrichtenklasse zurueck.
*/

nachricht& nachrichten_descriptor::get_nachricht( void )
{
  fwdlog fwd_log(configuration);
  Mid mid = String("");
  static eigenschaften_nachricht emsg(mid);
  static funkruf_nachricht fmsg(mid);
  static datenbankaenderung_nachricht cmsg(mid);
  static datenbankupdate_nachricht umsg(mid);
  static datenbankanforderung_nachricht rmsg(mid);
  static skyper_rubrik_nachricht smsg(mid);
  static zeit_nachricht zmsg(mid);
  static zielgebiets_nachricht dmsg(mid);
  // fwd_log << "Dateiname : " << dateiname << endl;                  
  ifstream msg_file(strtochar(dateiname));
  switch(typ)
    {
      case n_eigenschaften     : //fwd_log << 'E' << endl;
	                         msg_file >> emsg;	
			         return emsg;
			         break;
      case n_funkrufe          : //fwd_log << 'F' << endl;
	                         msg_file >> fmsg;	
			         return fmsg;
			         break;
      case n_aenderungen       : //fwd_log << 'C' << endl;
	                         msg_file >> cmsg;
			         return cmsg;
			         break;
      case n_update            : //fwd_log << 'U' << endl;
	                         msg_file >> umsg;
			         return umsg;
			         break;
      case n_updateanforderung : //fwd_log << 'R' << endl;
	                         msg_file >> rmsg;	
			         return rmsg;
			         break;
      case n_skyper_board      : //fwd_log << 'B' << endl;
                                 msg_file >> smsg;
				 return smsg;
				 break;
      case n_zeit              : //fwd_log << 'B' << endl;
                                 msg_file >> zmsg;
				 return zmsg;
				 break;
      case n_destination       : //fwd_log << 'B' << endl;
                                 msg_file >> dmsg;
				 return dmsg;
				 break;
      default                  : //fwd_log << "Unmoeglicher Fall 1" << endl;
	                         fwd_log.eintrag("N-Descr : Descriptor hat falschen Nachrichtentyp.",FWDLOGMASK_FWDERR);
	                         return emsg;
    }
}

// Mit dieser Methode wird die Nachricht aus dem Dateisystem geloescht.
void nachrichten_descriptor::loesche_nachricht( void )
{
  remove(strtochar(dateiname));
}

// Beim Aussenden der Nachricht muss die zeit der letzten Aussendung neu
// gesetzt und der Sendezaehler inkrementiert werden.
void nachrichten_descriptor::set_sendemarkierung( void )
{
  t_last = zeit();
  n_t++;
}

bool nachrichten_descriptor::expired( int max_age )
{
  if (max_age == -1)
    {
      if (typ == n_funkrufe)
	return zeit() - t_create > MAX_FWD_F_AGE;
      else if (typ == n_skyper_board)
	return zeit() - t_create > MAX_FWD_S_AGE;
      else 
	return false;
    }
  else
    return zeit() - t_create > max_age;
}

void routing_descriptor::init( void )
{
  typ = n_keine;
  dateiname = String("");
  pfadname = configuration.find("FWDDIR") + String("route/");
  recived_from = callsign();
  nachbarn.clear();
  t_create = zeit();
}

bool  routing_descriptor::get_file( n_types typ , String &msgfile )
{
  fwdlog fwd_log(configuration);

  String fname;
  int i = 0;
  bool found = false;
  // Maximal 100 mal versuchen, ein File zu oeffnen
  while (i < 100 && !found)
    {
      ostringstream fn;
      fn << pfadname;
      switch (typ)
	{
	  case n_eigenschaften     : fn << 'E';
	                             break;
	  case n_funkrufe          : fn << 'F';
	                             break;
	  case n_aenderungen       : fn << 'C';
	                             break;
	  case n_update            : fn << 'U';
	                             break;
	  case n_updateanforderung : fn << 'R';
	                             break;
	  case n_skyper_board      : fn << 'S';
	                             break;
	  case n_zeit              : fn << 'Z';
	                             break;
	  case n_destination       : fn << 'D';
	                             break;
	  default                  : fwd_log.eintrag("R-Descr : Descriptor hat falschen Nachrichtentyp.",FWDLOGMASK_FWDERR);
	                         
	}
      fn << "MSG.";
      fn << rand() % 32768 << ends;
      fname = "";
      fname.append(fn);
      if (!ifstream(strtochar(fname)))
	{
	  // Testen ob das File bereits existiert.
	  msgfile = String(fname);
	  found = true;
	}
      i++;
    }
  return found;
}

routing_descriptor::routing_descriptor( void )
{
  init();
}

routing_descriptor::routing_descriptor( nachricht &msg, const callsign &from )
{
  try
    {
      init();
      typ = msg.get_typ();
      ofstream msgfile;
      if (get_file(typ,dateiname))
	{
	  recived_from = from;
	  nachbarn.clear();
	  msgfile.open(strtochar(dateiname));
	  msgfile << msg << endl;
	  msgfile << "f" << from << endl;
	}
      else
	throw Error_could_not_open_msg_file();
    }
  catch( Error_destin_checksum_error )
    {
      throw Error_could_not_write_msg_file();
    }
}

routing_descriptor::routing_descriptor( const String &dname )
{
  init();
  dateiname = pfadname+dname;
  ifstream msg_file(strtochar(dateiname));
  if (msg_file)
    {
      char ch;
      msg_file.get(ch);
      msg_file.putback(ch);
      switch(ch)
	{
	  case 'E' : typ = n_eigenschaften;
	             break;
	  case 'F' : typ = n_funkrufe;
	             break;
	  case 'C' : typ = n_aenderungen;
	             break;
	  case 'U' : typ = n_update;
	             break;
	  case 'R' : typ = n_updateanforderung;
	             break;
	  case 'B' : typ = n_skyper_board;
	             break;
	  case 'Z' : typ = n_zeit;
	             break;
	  case 'D' : typ = n_destination;
	             break;
	  default  : msg_file.close();
	             throw Error_could_not_read_msg_file();
	}

      // Erzeugungszeit ermitteln
      
      struct stat stat_struct;
      
      stat(strtochar(dateiname), &stat_struct);
      t_create = stat_struct.st_mtime;
      
      String dummy;
      dummy.getline(msg_file,9999);
      while ( !msg_file.eof() )
	{
	  try
	    {
	      msg_file >> ch;
	      if ( msg_file.eof() )
		break;
	      if ( ch == 'f' )
		{
		  msg_file >> recived_from;
		  dummy.getline(msg_file,9999);
		}
	      else if ( ch == '+' )
		{
		  callsign call;
		  msg_file >> call;
		  nachbarn.push_back(call);
		  dummy.getline(msg_file,9999);
		}
	      else if ( ch == '-' )
		{
		  callsign call;
		  msg_file >> call;
		  for (list<callsign>::iterator it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
		    if (*it ==  call )
		      {
			nachbarn.erase(it);
			break;
		      }
		  dummy.getline(msg_file,9999);
		}
	      else
		throw Error_could_not_read_msg_file();
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_could_not_read_msg_file();
	    }
	}
    }
}

void  routing_descriptor::add_nachbar( const callsign &ziel )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag(ziel,"R-Descr : Add Nachbar.",FWDLOGMASK_DESCR);
  ofstream msg_file(strtochar(dateiname), ios::app );
  msg_file << '+' << ziel << endl;
  nachbarn.push_back(ziel);
}

bool routing_descriptor::check_nachbar( const callsign &ziel )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag(ziel,"R-Descr : Teste Nachbar.",FWDLOGMASK_DESCR);
  bool flag = false;
  for (list<callsign>::iterator it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if (*it == ziel) 
      flag = true;

  if (flag)
    fwd_log.eintrag(ziel,"R-Descr : JA!",FWDLOGMASK_DESCR);
  else
    fwd_log.eintrag(ziel,"R-Descr : NEIN!",FWDLOGMASK_DESCR);

  return flag;
}

void routing_descriptor::delete_nachbar( const callsign &ziel )
{
  fwdlog fwd_log(configuration);

  fwd_log.eintrag(ziel,"R-Descr : Loesche Nachbar.",FWDLOGMASK_DESCR);
  for (list<callsign>::iterator it = nachbarn.begin() ; it != nachbarn.end() ; ++it )
    if (*it == ziel) 
      {
	nachbarn.erase(it);
	ofstream msg_file(strtochar(dateiname), ios::app );
	msg_file << '-' << ziel << endl;
	return;
      }
}

bool routing_descriptor::is_empty( void )
{
  return nachbarn.size() == 0;
}

void routing_descriptor::delete_nachricht( void )
{
  remove(strtochar(dateiname));
}

nachricht& routing_descriptor::get_nachricht( void )
{
  syslog logf(configuration);
  fwdlog fwd_log(configuration);

  fwd_log.eintrag("R-Descr : get_nachricht.",FWDLOGMASK_DESCR);
  Mid mid = String("");
  static eigenschaften_nachricht emsg(mid);
  static funkruf_nachricht fmsg(mid);
  static datenbankaenderung_nachricht cmsg(mid);
  static datenbankupdate_nachricht umsg(mid);
  static datenbankanforderung_nachricht rmsg(mid);
  static skyper_rubrik_nachricht smsg(mid);
  static zeit_nachricht zmsg(mid);
  static zielgebiets_nachricht dmsg(mid);
	                         
  fwd_log.eintrag("R-Descr : statische Nachrichten angelegt.",FWDLOGMASK_DESCR);
  ifstream msg_file(strtochar(dateiname));
  fwd_log.eintrag("R-Descr : Nachrichtendatei geoeffnet.",FWDLOGMASK_DESCR);
  switch(typ)
    {
      case n_eigenschaften     : msg_file >> emsg;
	                         return emsg;
		                 break;
      case n_funkrufe          : msg_file >> fmsg;
			         return fmsg;
			         break;
      case n_aenderungen       : msg_file >> cmsg;
			         return cmsg;
			         break;
      case n_update            : msg_file >> umsg;
			         return umsg;
			         break;
      case n_updateanforderung : msg_file >> rmsg;
			         return rmsg;
			         break;
      case n_skyper_board      : msg_file >> smsg;
                                 return smsg;
				 break;
      case n_zeit              : msg_file >> zmsg;
                                 return zmsg;
				 break;
      case n_destination       : msg_file >> dmsg;
                                 return dmsg;
				 break;
      default                  : fwd_log.eintrag("R-Descr : Descriptor hat falschen Nachrichtentyp.",FWDLOGMASK_FWDERR);
	                         return emsg;
    }
}

bool routing_descriptor::expired( int max_age )
{
  if (max_age == -1)
    {
      if (typ == n_funkrufe)
	return zeit() - t_create > MAX_FWD_F_AGE;
      else if (typ == n_skyper_board)
	return zeit() - t_create > MAX_FWD_S_AGE;
      else 
	return false;
    }
  else
    return zeit() - t_create > max_age;
}
