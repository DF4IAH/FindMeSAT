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

#include "texte.h"
#include "logfile.h"

// Der Operator gibt den gesamten Text in den angegebenen
// Stream aus. Dazu wird der Text aus der gegebenen Datei
// Zeilenweise eingegeben und dann in den Stream ausgegeben

ostream& operator<<(ostream &strm, text &t )
{
  String line,line2;

  if (t.source)  
    {
      // Zuruecksetzen des Streams
      t.source.clear();
      t.source.seekg(0, ios::beg );
      while (!t.source.eof())
	{
	  line2.getline(t.source,255);
	  if (t.makro_flag)
	    line = t.macros.line(line2);
	  else
	    line = line2;
	  for (unsigned int i = 0; i < line.slen(); i++ )
	    if (line[i] == '\n')
	      line[i] = ' '; 
	  strm << line << t.cr;
	}
    }
  return strm;
}

const String & text::get( void )
{
  static String out;
  String line,line2;

  out = "";
  if (source)  
    {
      // Zuruecksetzen des Streams
      source.clear();
      source.seekg(0, ios::beg );
      while (!source.eof())
	{
	  line2.getline(source,255);
	  if (makro_flag)
	    line = macros.line(line2);
	  else
	    line = line2;
	  for (unsigned int i = 0; i < line.slen(); i++ )
	    if (line[i] == '\n')
	      line[i] = ' '; 
	  out.append(line);
	  out.append(cr);
	}
    }
  return out;
}

// Die Konstruktoren fuer die abgeleiteten Klassen ctext, atext, 
// itext und htext holen den entsprechenden Parameter aus dem
// Konfigurationsfile und oeffnen die dort angegebene DAtei.
//
// Falls der entsprechende Parameter im Konfigurationsfile nicht 
// definiert wurde, wird die dabei auftretende Exception abgefangen
// und eine entsprechende Fehlermedlung ins Systemlog geschrieben.
//
ctext::ctext( config_file &cfg, makros &m, const String &l , char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("CTEXT")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;
     
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter CTEXT nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

atext::atext( config_file &cfg, makros &m, const String &l ,char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("AKTUELL")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;

    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter AKTUELL nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

itext::itext( config_file &cfg, makros &m, const String &l ,char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("INFO")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;
    
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter INFO nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

htctext::htctext( config_file &cfg, makros &m, const String &l , char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("CTEXT_HTML")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;
     
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter CTEXT_HTML nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

htatext::htatext( config_file &cfg, makros &m, const String &l ,char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("AKTUELL_HTML")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;

    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter AKTUELL_HTML nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

htitext::htitext( config_file &cfg, makros &m, const String &l ,char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("INFO_HTML")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename);
      makro_flag = true;
    
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter INFO_HTML nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

rt_text::rt_text( const String& fname, makros &m ,char wr ) : text(m,"dl",wr)
{
  source.open(strtochar(fname));
  makro_flag = false;
} 

wt_text::wt_text( const String& fname, makros &m ,char wr ) : text(m,"dl",wr)
{
  filename = fname;
  ofstream text_file(strtochar(fname));
  source.open(strtochar(fname));
  makro_flag = false;
} 

bool wt_text::status( void )
{
  return (source);
}

void wt_text::line( const String & ln )
{
  ofstream text_file(strtochar(filename), ios::app );
  text_file << ln << endl;
}

htext::htext( config_file &cfg, makros &m, const String &l, char wr ) : text(m,l,wr)
{
  try
    {
      char filename[1000];
      strcpy(filename,strtochar(cfg.find("HELP")));
      strcat(filename,".");
      strcat(filename,strtochar(l));
      source.open(filename); 
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter HELP nicht definiert.",LOGMASK_PRGRMERR);
    }
} 

bool htext::check_cmd( const String &line, const String &cmd )
{
  String stripped_line;
  
  if (line[1] == '1' && line[2] == ':')
    stripped_line = line.copy(3,line.slen() - 3);
  else
    stripped_line = line.copy(1,line.slen() - 1);

  stripped_line.lowcase();
  if (cmd.slen() > stripped_line.slen())
    return false;
  else if (cmd.slen() == stripped_line.slen())
    return cmd == stripped_line;
  else
    {
      String line_frag = stripped_line.copy(0,cmd.slen());
      return cmd == line_frag;
    }
}

String htext::get_cmd( const String &line )
{
  String stripped_line;
  
  if (line[1] == '1' && line[2] == ':')
    stripped_line = line.copy(3,line.slen() - 3);
  else
    stripped_line = line.copy(1,line.slen() - 1);

  return stripped_line;
}

vector<struct h_eintrag>  htext::suche( String cmd, const user &usr )
{
  struct h_eintrag hent;
  vector<struct h_eintrag> ergebnis;
  bool flag = false;

  ergebnis.clear();

  int cnt = 0;

  if (source)   // Helpfile vorhanden?
    {
      // Zuruecksetzen des Streams
      source.clear();
      source.seekg(0, ios::beg );

      // Das gesamte Helpfile durchgehen
      while (!source.eof())
	{
	  // Zeile in einen dafuer angelegten String einlesen
	  String line;
	  line.getline(source,255);

	  // Zeile nur bearbeiten, wenn es keine Kommentarzeile ist
	  if (line.slen() > 0 && line[0] != '#')
	    {
	      // Erstes Zeichen ein ':', dann folgt eine Hilfe fuer einen neuen Befehl
	      if (line[0] != ':')
		{
		  // Wenn nicht, dann solange count > 0, alle Zeilen der Hilfedatei ausgeben
		  if (cnt-- > 0)
		    {
		      hent.htxt.push_back(line);
		      flag = true;
		    }
		}
	      else
		{
		  if (flag)
		    {
		      ergebnis.push_back(hent);
		      flag = false;
		    }
		  // Wurde Hilfe fuer einen bestimmten Befehl angefragt?
		  if (cmd.slen() > 0)
		    {
		      // Ja, erstmal angefragten Befehl in kleinbuchstaben wandeln und dann
		      // Ueberpruefen, ob nach dem Doppelpunkt dieser Befehl folgt
		      cmd.lowcase();
		      if (check_cmd(line,cmd))
			{
			  // Wenn ja, dann die folgenden Zeilen ausgeben
			  cnt = 3200000;
			  hent.cmd = get_cmd(line);
			  hent.htxt.clear();
			}
		      else
			{
			  // Sonst Ausgabe stoppen
			  cnt = 0;
			}
		    }
		  else
		    {
		      // Wenn keine spezielle Hilfe angefordert wurde, von jedem Befehl nur die 
		      // erste Zeile ausgeben.
		      cnt = 1;
		      hent.cmd = get_cmd(line);
		      hent.htxt.clear();
		    }

		  // Ueberpruefen, ob es sich um Sysopbefehl handelt und der Benutzer kein 
		  // Sysop ist.
		  if (line[1] == '1' && line[2] == ':' && !usr.is_sysop())
		    {
		      // In diesem Fall Ausgabe wieder unterbinden. Nur Sysops bekommen auch
		      // die Sysopbefehle angezeigt.
		      cnt = 0;
		    }
		}
	    }
	}
    }
  return ergebnis;
}

bool htext::print( ostream& strm, const String &cmd, const user &usr )
{
  struct h_eintrag hent;
  vector<struct h_eintrag> ergebnis;

  ergebnis = suche(cmd,usr);
  if (ergebnis.size() > 0)
    {
      for (vector<struct h_eintrag>::iterator it1 = ergebnis.begin(); it1 != ergebnis.end(); ++it1 )
	{
	  bool l1_flag = true;
	  for (vector<String>::iterator it2 = (it1->htxt).begin(); it2 != (it1->htxt).end(); ++it2)
	    {
	      if (l1_flag)
		{
		  String cmd = it1->cmd+String("            ");
		  strm << cmd.copy(0,12);
		  l1_flag = false;
		}
	      else
		strm << "            ";
	      strm << " ";
	      strm << *it2 << cr;
	    }
	}
      return true;
    }
  return false;
}

