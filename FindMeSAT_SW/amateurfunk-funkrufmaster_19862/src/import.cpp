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

#include "import.h"


/*
  Import-Dateien werden in dem im Konfigurationsverzeichnis 
  mit dem Parameter IMPORTDIR festgelegten Verzeichnis abgelegt und
  muessen einen DAteinamen der Form IMPORT.<suffix> haben. 
  <suffix> kann dabei eine beliebige Zeichenkombination
  sein, die den Unix-Dateinamenkonventionen entspricht.

  Die Importschnittstelle legt im gleichen Verzeichnis fuer diesen
  Import-Vorgang eine Datei EXPORT.<suffix> an, in die alle Ausgaben
  des Benutzerinterfaces geschrieben werden.

  Beginnt der Dateiname der Import-Datei mit IMPORT.NO_EXPORT., so
  wird keine Export-Datei angelegt. Die Ausgaben des Benutzerinterfaces
  werden in eine temporaere Datei geschrieben, die nach abarbeiten der
  Importdatei sofort wieder geloescht wird.

  In der ersten Zeile der Import-Datei muss das Rufzeichen stehen
  unter dem die Import-Datei ausgefuehrt wird. Ist hier kein 
  gueltiges Rufzeichen angegeben, wird der Import-Vorgang abgebrochen.

  Der Rest der Import-Datei wird zeilenweise abgearbeitet. Zeilen,
  die mit einer Raute ('#') beginnen werden als Kommentarzeile 
  betrachtet und nicht ausgewertet. Alle anderen Zeilen werden an
  den Kommando-interpreter des Benutzer-Interfaces zur Auswertung
  weiter gegeben. Dort werden sie wie normale Benutzereingaben
  behandelt.

  Der Parameter IMPORT_AS_SYSOP legt fest, ob die Befehlsfolgen im 
  Benutzer oder im Sysop-Modus abgearbeitet werden. Fuer die Auswertung
  von Sysop-Befehlen ist diesem Parameter der Wert JA zuzuweisen.

  Fehlt dieser Parameter oder hat er den Wert NEIN werden die Befehle
  im Benutzermodus ausgefuehrt.

*/

void import_system::delete_import_file( String &input_filename)
{
  remove(strtochar(import_dir_name+input_filename));
}

void import_system::read_import_file( String &input_filename, bool exp )
{

  String outp;

  String output_filename;
  if (exp)
    {
      unsigned int l = input_filename.slen();
      String suffix = input_filename.copy(7,l-7);
      output_filename = import_dir_name + String("EXPORT.") + suffix;
    }
  else
    output_filename = String("/dev/null");


  ifstream input(strtochar(import_dir_name+input_filename));
  ofstream output(strtochar(output_filename));

  String line;
  line.getline(input,99);

  try
    {
      outp = "";
      user usr(line,sysop_flag);      
      user_interface interface(outp,usr,false,baken);
      output << outp;
      char puffer[1000];
      // Jede Zeile einzeln holen
      while (input.getline(puffer,254))
	{
	  // Kommentarzeilen nicht weiter bearbeiten
	  if (*puffer != '#')
	    {
	      outp = "";
	      interface.process(String(puffer)+String('\n'),outp);
	      output << outp;
	    }
	}
      interface.set_io('I',0,0);
    }
  catch( Error_no_callsign )
    {
      output << "IMPORT-ERROR: Kein gueltiges Rufzeichen in erster Zeile." << endl;
    }
}

import_system::import_system( config_file &cfg,  t_baken &b ) : logf(cfg)
{
  try
    {
      import_dir_name = cfg.find("IMPORTDIR");
      activ = true;
      baken = b;
      try
	{
	  sysop_flag = cfg.find("IMPORT_AS_SYSOP") == "JA";
	}
      catch( Error_parameter_not_defined )
	{
	  sysop_flag = false;
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Kein Import-Verzeichnis definiert.",LOGMASK_PRGRMERR);
      activ = false;
    }
}

void import_system::import( void )
{
  if (activ)
    {
      DIR *import_dir ;
      
      import_dir = opendir(strtochar(import_dir_name));
      if (import_dir != NULL)
	{
	  struct dirent *entry;
	  while (( entry = readdir(import_dir)) != NULL )
	    {
	      if ( strncmp(entry->d_name,"IMPORT.",7) == 0 )
		{
		  bool no_export = ( strncmp(entry->d_name,"IMPORT.NO_EXPORT.",17) == 0 );
		  String fname = String(entry->d_name);
		  read_import_file(fname,!no_export);
		  delete_import_file(fname);
		}
	    }
	  closedir(import_dir);
	}
      else
	{
	  logf.eintrag("Kann Import-Verzeichnis nicht oeffnen. Import wird deaktiviert.",LOGMASK_PRGRMERR);
	  activ = false;
	}
    }
}
