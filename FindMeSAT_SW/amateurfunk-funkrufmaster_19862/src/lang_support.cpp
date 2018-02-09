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

#include "lang_support.h"


#include "config.h"

user_meldungen::user_meldungen(config_file &cfg ) : logf(cfg)
{
  try
    {
      filename = cfg.find("MESSAGES");
      Meldungen.clear();
      loaded = false;
    }
  catch( Error_parameter_not_defined )
    {
      throw Error_message_file_not_defined();
    }
}



void user_meldungen::load( const String &lang )
{
  ifstream lng_file;

  String fname = filename+String('.')+lang;
  // Konfigurationsfile oeffnen
  lng_file.open(strtochar(fname));

  /*
  if (!lng_file)
    {
      fname = filename+String(".dl");
      lng_file.open(strtochar(fname));
    } 
  */
  if (!lng_file) throw Error_could_not_open_messagefile();
  
  else
    {
      char puffer[255];
      // Jede Zeile einzeln holen
      while (lng_file.getline(puffer,254))
	// Kommentarzeilen nicht weiter bearbeiten
	if (*puffer != '#')
	  {
	    istringstream input(puffer);
	    
	    char parameter[60];
	    char value[200];
	    char ch;
	    
	    // Einen Input-String-Stream aus der Zeile erzeugen
	    // und parameter und Value daraus extrahieren
	    input.get(parameter,59,'=');
	    input >> ch;
	    if (ch == '=')
	      {
		input.get(value,199);
		String Par(parameter);
		String Val(value);
		unsigned int MNr = (unsigned int) Par.Stoi();
		// Parameterpar in die Konfigurationsdatenbank ablegen
		Meldungen[MNr] = Val;
	      }
	    else 
	      throw Error_message_file_format_error();
	  }
      loaded = true;
    }
}

String user_meldungen::find( unsigned int MNr)
{
  // Mit Parameter als Schluessel in Datenbank suchen.
  // Zurueckgegeben wird ein Iterator
  t_lang::iterator it = Meldungen.find(MNr);
  
  if (it == Meldungen.end())
    {
      logf.eintrag("Meldung Nr. "+itoS(MNr)+" nicht in Sprachdatei.",LOGMASK_PRGRMERR);
      throw Error_message_not_defined();
    }
  else
    return it->second; // Zurueckgabe des Parameterwertes
}
