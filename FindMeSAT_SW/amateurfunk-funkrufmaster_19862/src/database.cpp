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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "database.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fstream.h>
#include <iomanip.h>
#include <string.h>
#include <vector>

#include "globdef.h"

// A dirty trick !
#ifndef __CONFIG_H__
vector<String> komma_separeted( const String & );
#endif


String pager_typ::types[20] = { "NO DEFINED" , "UNKNOWN0k5", "UNKNOWN1k2", 
			        "UNKNOWN2k4", "SKYPER" , "QUIX" , "TELMI" ,
				"CITYRUF", "PRIMO", "SCALL" };

#define PAGER_TYPEN 10

pager_typ::pager_typ( String Typ )
{
  if(Typ.isnum())	// Pager-Typen bestehen IMMER aus Buchstaben (und Ziffern)
    Typ=types[Typ.Stoi()];
  Typ.upcase();

  typ = 0;
  for (int i = 0; i < PAGER_TYPEN;i++ )
  {
    if ( types[i] == Typ ) typ = i;
  }
}

String pager_typ::get_string( void ) const
{
  return types[typ];
}

bool pager_typ::operator==( const pager_typ &t )
{
  return typ == t.typ;
}

bool pager_typ::operator!=( const pager_typ &t )
{
  return typ != t.typ;
}


ostream& operator<< ( ostream& strm, const pager_typ& t )
{
  strm << t.types[t.typ];
  return strm;
}

istream& operator>> ( istream& strm, pager_typ &t )
{
  char ch;
  while ( strm.get(ch) && (ch == ' ') );
  strm.putback(ch);

  char puffer[20];
  strm.get(puffer,19,' ');

  int i = 0;
  while ( ( i < 20) && (puffer[i] != '\0') ) 
    {
      puffer[i] = toupper(puffer[i]);
      i++;
    }

  for (int i = 0; i < PAGER_TYPEN;i++ )
    if ( t.types[i] == String(puffer) ) t.typ = i;

  return strm;
}

ostream& operator< ( ostream& strm , const pager_typ &t )
{
  strm << t.typ;
  return strm;
}

istream& operator> ( istream &strm, pager_typ &t )
{
  strm >> t.typ;
  return strm;
}

database_entry::database_entry( const callsign &c, const adress &a, pager_typ g, const callsign &s )
{
  call = c;
  adresse = a;
  geraet = g;
  last_change = zeit();
  server = s;
  loc = locator();
  name = String("");
  language = String("dl");
  last_login = zeit(-1);
}

database_entry::database_entry( const String & str , float version)
{
  vector<String> parts = komma_separeted(str);

  vector<String>::iterator it = parts.begin();

  try
    {
      call = callsign(*it);
      it++;
      
      adresse = adress(*it);
      it++;

      geraet = pager_typ(*it);
      it++;

      try
	{
	  loc = locator(*it);
	}
      catch( Error_not_a_locator )
	{
	  loc = locator();
	}
      it++;

      name = *it;
      it++;
      
      last_change = zeit(it->Stoi());
      it++;
      
      server = callsign(*it);
      it++;
      
      if (version > 0.2999 )
	{
	  language = *it;
	  it++;
	  
	  last_login = zeit(it->Stoi());
	  it++;
	  
	  ax25_pw = *it;
	  it++;
	  
	  consol_pw = *it;

	}
    }
  catch( Error_no_callsign )
    {
      throw Error_illegal_database_entry();
    }
}
 
void database_entry::set_call( const callsign& c, const callsign &s, zeit z )
{
  call = c;
  last_change = z;
  server = s;
}

void database_entry::set_adr( const adress &a, const callsign &s, zeit z )
{
  adresse = a;
  last_change = z;
  server = s;
}

void database_entry::set_geraet( const pager_typ g, const callsign &s, zeit z )
{
  geraet = g;
  last_change = z;
  server = s;
}

void database_entry::set_loc( const locator &l, const callsign &s, zeit z )
{
  loc = l;
  last_change = z;
  server = s;
}

void database_entry::set_name( const String &n, const callsign &s, zeit z )
{
  name = n;
  last_change = z;
  server = s;
}

void database_entry::set_language( const String &l, const callsign &s, zeit z )
{
  language = l;
  last_change = z;
  server = s;
}

void database_entry::set_ax25_pw( const String &pw, const callsign &s, zeit z )
{
  ax25_pw = pw;
  last_change = z;
  server = s;
}

void database_entry::set_consol_pw( const String &pw, const callsign &s, zeit z )
{
  consol_pw = pw;
  last_change = z;
  server = s;
}

void database_entry::login( void )
{
  last_login = zeit();
}

void database_entry::update( const callsign &s )
{
  last_change = zeit();
  server = s;
}

ostream& operator<( ostream& strm, database_entry &e )
{
  e.call.set_format(false);
  e.call.set_nossid(false);
  strm << e.call << ',';
  strm << e.adresse << ',';
  strm < e.geraet;
  strm << ',' << e.loc << ',' << e.name << ',';
  e.server.set_format(false);
  e.server.set_nossid(false);
  (strm < e.last_change) << ',' << e.server;
  strm << ',' << e.language << ',';
  strm < e.last_login;
  strm << ',';
  strm << e.ax25_pw << ',' << e.consol_pw;
  return strm;
}

callsign_database::callsign_database( void )
{
  database_filename = "";
  changed = false;
  valid = false;
  Database.clear();
}

callsign_database::callsign_database( const String &filename )
{
  database_filename = filename;
  float version;

  // Oeffnen des Datenbank-Files als Input-Streams
  ifstream db_source(strtochar(database_filename));

  if (!db_source)
    {
      // Wenn das File nicht vorhanden ist, neu anlegen
      ofstream db_gen(strtochar(database_filename));
      if (!db_gen)
	throw Error_can_not_create_database();
      db_gen << "# Callsign-Database-File" << endl;
      db_gen << "# this file is generated automaticaly, please do not edit!" << endl;
      db_gen << "% Datenbank Version 0.31" << endl;
      db_gen.close();
      db_source.open(strtochar(database_filename));
    }
  
  String line;
  // Jetzt Datenbankfile einlesen
  while (!db_source.eof())
    {
      line.getline(db_source,255);
      // Zeile holen und Kommentarzeilen ignorieren
      if (line.slen() > 0)
	{
	  if (line[0] == '%') // Versionsnummer der Datenbank ueberpruefen
	    {
	      if ( sscanf(strtochar(line),"%% Datenbank Version %f",&version) == 1 )
		{
		  if ( version != 0.22 && ( version < 0.2999 || version > 0.3101))
		    throw Error_wrong_database_version();
		}
	    }
	  else
	    if (line[0] != '#')
	      {
		try
		  {
		    database_entry ad(line,version);
		    callsign call = ad.get_call();
		    String callstr = call.call();	  
		    Database[callstr] = ad; // Und in interne Datenbank abspeichern
		  }
		catch( Error_illegal_database_entry )
		  {
		    cerr << "Fehlerhafter Datanbankeintrag, ";
		    cerr << "der nicht rekonstruiert werden kann." << endl;
		  }
	      }
	}
    }
  changed = version < 0.31;
  valid = true;
}

callsign_database::~callsign_database( void )
{
  // Im Destruktor muss die rufzeichendatenbank wieder abgespeichert werden,
  // wenn sie veraendert wurde.
  // Und Datenbank loeschen
  save();
  Database.clear();
}

callsign_database::callsign_database(const callsign_database & d )
{
  Database = d.Database;
  database_filename = d.database_filename;
  changed = false;
  valid = d.valid;
}

callsign_database callsign_database::operator= ( const callsign_database & d )
{
  if (&d != this)
    {
      Database = d.Database;
      database_filename = d.database_filename;
      changed = false;
      valid = d.valid;
    }
  return *this;
}

void callsign_database::save( void )
{
  if (changed) // Veraenderung der Datenbank ?
    {
      // Oeffnen des Datenbank-files.
      ofstream db_source(strtochar(database_filename));
      
      db_source << "# Callsign-Database-File" << endl;
      db_source << "# this file is generated automaticaly, please do not edit!" << endl;
      db_source << "% Datenbank Version 0.31" << endl;
      
      // Mittels eines Iterators alle Datenbankeintraege durchgehen
      t_database::iterator i;
      
      for ( i = Database.begin(); i != Database.end();++i)
	{
	  // Datenbankeintrag abspeichern
	  database_entry en = i->second;
	  (db_source < en) << endl;
	}
      changed = false;
    }
}


void callsign_database::add( const database_entry& ad )
{
  callsign call = ad.get_call();
  String callstr = call.call();
  t_database::iterator it = Database.find(callstr);
  if (it != Database.end())
    throw Error_callsign_already_exists();
  else
    {
    Database[callstr] = ad;
    }
  changed = true;
}

void callsign_database::del( const callsign& call)
{
  // Rufzeichen in Datenbank suchen
  String callstr = call.call();
  t_database::iterator it = Database.find(callstr);

  if (it == Database.end())
    throw Error_callsign_does_not_exist();
  else
    Database.erase(it); // Wenn vorhanden, dann loeschen
  changed = true;
}

database_entry callsign_database::find( const callsign& call )
{
  // Rufzeichen in Datenbank suchen
  String callstr = call.call();
  t_database::iterator it = Database.find(callstr);

  if (it == Database.end())
    throw Error_callsign_does_not_exist();
  else
    return it->second; // Und Adresse ausgeben
}

database_entry callsign_database::find_adress( const adress& adresse )
{
  t_database::iterator it;
  callsign_database calls;

  for (it = Database.begin(); it != Database.end(); ++it)
    {
      callsign call = it->second.get_call();
      String c = call.call();
      database_entry en = it->second;
      if(en.get_adr()==adresse)
      {
	return it->second; // Und Adresse ausgeben      
      }
     }
    throw Error_adress_does_not_exist();
}

bool callsign_database::first( database_entry &ent )
{
  first_next_it = Database.begin();
  if (first_next_it != Database.end())
    {
      ent = first_next_it->second;
      ++first_next_it;
      return true;
    }
  else 
    return false;
}

bool callsign_database::next( database_entry &ent )
{
  if (first_next_it != Database.end())
    {
      ent = first_next_it->second;
      ++first_next_it;
      return true;
    }
  else 
    return false;
}

void callsign_database::change( const callsign& call, const database_entry& ad )
{
  // Rufzeichen in Datenbank suchen
  String callstr = call.call();  
  t_database::iterator it = Database.find(callstr);

  if (it == Database.end())
    throw Error_callsign_does_not_exist();
  else
    it->second = ad;
  changed = true;
}

void callsign_database::change_call( const callsign& call, const callsign& new_call, const callsign& server )
{
  // Rufzeichen in Datenbank suchen
  String callstr1 = call.call();
  String callstr2 = new_call.call();
  t_database::iterator it = Database.find(callstr1);

  if (it == Database.end())
    throw Error_callsign_does_not_exist();
  else
    {
      database_entry en = it->second;
      Database.erase(it);
      en.set_call(new_call,server);
      Database[callstr2] = en;
      changed = true;
    }
}

vector<database_entry> callsign_database::get_user( const String &part, bool all )
{
  bool p_flag = part.slen() > 0;
  vector<database_entry> tmp;
  t_database::iterator it;

  for (it = Database.begin(); it != Database.end(); ++it)
    {
      database_entry en = it->second;
      if (all || en.get_adr().gueltig())
	{
	  if (p_flag)
	    {
	      callsign call = it->second.get_call();
	      String c = call.call();
	      if (c.in(part) )
		tmp.push_back(en);
	    }
	  else
	    tmp.push_back(en);
	}
    }
  return tmp;
}

vector<database_entry> callsign_database::get_other_pager( const callsign &call )
{
  vector<database_entry> tmp;
  t_database::iterator it;
  
  for (it = Database.begin(); it != Database.end(); ++it)
    {
      database_entry en = it->second;
      callsign call2= it->second.get_call();
      if (samecall(call,call2) && call != call2)
	tmp.push_back(en);
    }

  return tmp;
}

int callsign_database::get_size_pager( void )
{
  t_database::iterator it;
  int size=0;

  for (it = Database.begin(); it != Database.end(); ++it)
  {
      database_entry en = it->second;
      if (en.get_adr().gueltig())
        size++;
  }
  return size;
}
