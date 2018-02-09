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
 *                                                                          *
 ****************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <fstream.h>
#include <string.h>
#include "globdef.h"

Error_config_file_format_error::Error_config_file_format_error( const String&l )
{
  line = l;
#ifdef _DEBUG_EXEC_
  cerr << "Error_config_file_format_error in Zeile : " << line << endl;
#endif
}

String Error_config_file_format_error::get_error_line( void )
{
  return line;
}


config_file::config_file()
{
  char *path = getenv("FUNKRUF");  // Umgebungsvariable "FUNKRUF" abfragen
  if (path == NULL)                // Vorhanden ? 
    {
      // Nein ! Konfigurationsfile steht dann unter /etc"
      strcpy(filename,"/etc/frmaster.conf");
    }
  else
    {
      // ja ! Konfigurationsfile steht dann unter 
      // $FUNKRUF/etc
      strcpy(filename,path);
      strcat(filename,"etc/frmaster.conf");
    }
  Configuration.clear();
  loaded = false;
}



config_file::~config_file( void )
{
  // Konfigurationsdatenbank loeschen
  save();
  Configuration.clear();
}

void config_file::load( void )
{
  Configuration.clear();
  // Konfigurationsfile oeffnen
  ifstream cfg_file(filename);
  
  if (!cfg_file) throw Error_could_not_open_configfile();
  
  else
    {
      char puffer[255];
      // Jede Zeile einzeln holen
      while (cfg_file.getline(puffer,254))
	// Kommentarzeilen nicht weiter bearbeiten
	if (*puffer != '#')
	  {
	    istringstream input(puffer);
	    
	    char parameter[200];
	    char value[200];
	    char ch;
	    
	    // Einen Input-String-Stream aus der Zeile erzeugen
	    // und parameter und Value daraus extrahieren
	    input.get(parameter,199,'=');
	    input >> ch;
	    if (ch == '=')
	      {
		input.get(value,199);
		String Par(parameter);
		String Val(value);
		Par.upcase();  // Alle Parameter werden GROSS geschrieben
		// Parameterpar in die Konfigurationsdatenbank ablegen
#ifdef _DEBUG_ELOGS_ 
		cout << '.' << Par << '.' << endl;
#endif
		Configuration[Par] = Val;
	      }
	    else 
	      {
		cerr << "Fehlerhafte Zeile in der Systemkonfiguration:" << endl;
		cerr << puffer << endl;

		throw Error_config_file_format_error(parameter);
	      }
	  }
      loaded = true;
    }
}

void config_file::save( void )
{
  if (loaded)
    {
      ofstream cfile(filename);
      cfile << "#" << endl;
      cfile << "#" << endl;
      cfile << "# Pager-Konfigurationsdatei." << endl;
      cfile << "#" << endl;
      cfile << "#" << endl;
      cfile << "# Diese Datei wird automatisch erzeugt. Eine Aenderung ist nur ueber die" << endl;
      cfile << "# entsprechenden Kommandos im Funkruf-Benutzerinterface sinnvoll." << endl;
      cfile << "#" << endl;
      cfile << "#" << endl;
      cfile << "# Funkruf-Benutzerinterface, Version " << VERSION << endl;
      cfile << "#" << endl;
      cfile << "#" << endl;
      for (t_config::iterator it = Configuration.begin(); it != Configuration.end();++it)
	{
	  cfile << it->first << "=" << it->second << endl;
	  cfile << "#" << endl;
	}
    }
}

String config_file::find( const String &parameter)
{
  // Mit Parameter als Schluessel in Datenbank suchen.
  // Zurueckgegeben wird ein Iterator
  t_config::iterator it = Configuration.find(parameter);
  
  if (it == Configuration.end())
    {
#ifdef _DEBUG_ELOGS_
      cerr << "config.find()" << endl;
      cerr << "Parameter " << parameter << " nicht gefunden." << endl;
#endif
      throw Error_parameter_not_defined();
    }
  else
    return it->second; // Zurueckgabe des Parameterwertes
}

void config_file::set( const String &parameter, const String &wert )
{
  t_config::iterator it = Configuration.find(parameter);

  if (it == Configuration.end())
    {
      Configuration[parameter] = wert;
    }
  else
    {
      it->second = wert;
    }
}

void config_file::clear( const String &parameter )
{
  t_config::iterator it = Configuration.find(parameter);

  if (it == Configuration.end() )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "config.clear()" << endl;
      cerr << "Parameter " << parameter << " nicht gefunden." << endl;
#endif
      throw Error_parameter_not_defined();
    }
  else
    Configuration.erase(it);
}

void config_file::printOn( ostream &strm, char ende )
{
  for (t_config::iterator it = Configuration.begin(); it != Configuration.end();++it)
    strm << it->first << "=" << it->second << ende;
}


vector<String> komma_separeted( const String &input )
{
  vector<String> tmp;

  unsigned int length = input.slen();
  // String nur absuchen, wenn er auch Zeichen enthaelt
  if (length > 0)
    {
      unsigned int begin = 0;
      for ( unsigned int i = 0 ; i < length ; i++)
	{
	  if (input[i] == ',' )
	    {
	      String part = input.copy(begin,i-begin);
	      begin = i + 1;
	      tmp.push_back(part);
	    }
	}
      // Jetzt nochmal separat den letzten Teil abtrennen
      if (begin < length)
	{
	  String part = input.copy(begin,length-begin);
	  tmp.push_back(part);
	}
      else
	{
	  // Wenn letztes Zeichen ein Komma ist, dann wird begin == length
	  // In diesem Fall leeren String als letzte Komponente anhaengen
	  tmp.push_back(String());
	}
    }
  return tmp;
}

