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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *  		                                			    *
 ****************************************************************************/

#include "wx.h"

#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <sys/types.h>
#include <dirent.h>

#include "spoolfiles.h"
#include "board.h"
#include "logfile.h"
#include "callsign.h"
#include "export.h"

extern config_file configuration;
extern spoolfiles spool;
extern callsign G_mycall;




/*
  ----------------------------------------------------------------------------
  zunaechst kommen alle Methoden der Konfigurations-Datei-Klasse
  wx_config_files
  ----------------------------------------------------------------------------
*/
wx_config_file::wx_config_file( void )
{
  save_flag = false;
  slot = 0;
};

wx_config_file::wx_config_file( String & dname )
{
  read(dname);
}

void wx_config_file::read( String & dname )
{
  ifstream cfg_file(strtochar(dname));
  if (!cfg_file) throw Error_could_not_open_wx_configfile();

  else
    {
      dateiname = dname;
      save_flag = false;
      String line;
      while (cfg_file)
	{
	  line.getline(cfg_file,250);
	  int l = line.slen();
	  if (l > 0 &&  line[0] != '#')
	    {
	      try
		{
		  int p;
		  if ((p = line.pos(String('='))) != -1 )
		    {
		      String parameter = line.copy(0,p);
		      String wert = line.copy(p+1,l-p-1);
		      wert.kuerze();
		      if (parameter == "WX_STATION")
			station = callsign(wert);
		      else if (parameter == "LOCATOR" )
			loc = locator(wert);
		      else if (parameter == "WX_CONNECT_PFAD")
			pfad = connect_string(wert);
		      else if (parameter == "SLOT")
			slot = atoi(strtochar(wert));
		      else if (parameter == "PROMPT")
			prompt = wert;
		      else if (parameter == "COMMAND")
			command = wert;
		      else if (parameter == "TEMPERATUR")
			temperatur = wert;
		      else if (parameter == "LUFTDRUCK")
			luftdruck = wert;
		      else if (parameter == "LUFTFEUCHTE")
			luftfeuchte = wert;
		      else if (parameter == "WINDRICHTUNG")
			windrichtung = wert;
		      else if (parameter == "WINDGESCHWINDIGKEIT")
			windgeschwindigkeit = wert;
		      else if (parameter == "BOEN" )
			boen = wert;
		      else if (parameter == "NIEDERSCHLAG1" )
			niederschlag1 = wert;
		      else if (parameter == "NIEDERSCHLAG4")
			niederschlag4 = wert;
		      else if (parameter == "NIEDERSCHLAGM")
			niederschlag_m = wert;
		      else 
			throw Error_unknown_parameter_in_wx_configfile();
		    }
		  else 
		    throw Error_wrong_format_in_wx_config_file();
		}
	      catch( Error_syntax_fehler_in_connect_string )
		{
		  throw Error_wrong_format_in_wx_config_file();
		}
	      catch( Error_no_callsign )
		{
		  throw Error_wrong_format_in_wx_config_file();
		}
	      catch( Error_not_a_locator )
		{
		  throw Error_wrong_format_in_wx_config_file();
		}
	    }
	}
    }
}

void wx_config_file::PrintOn( ostream &strm )
{
  strm << "#" << endl;
  strm << "# WX-configuration-file!" << endl;
  strm << "# automaticaly generated, please do not edit!" << endl;
  strm << "#" << endl;
  strm << "WX_STATION=" << station << endl;
  strm << "LOCATOR=" << loc << endl;
  strm << "#" << endl;
  strm << "WX_CONNECT_PFAD=" << pfad << endl;
  strm << "SLOT=" << slot << endl;
  strm << "PROMPT=" << prompt << endl;
  strm << "COMMAND=" << command << endl;
  strm << "TEMPERATUR=" << temperatur << endl;
  strm << "LUFTDRUCK=" << luftdruck << endl;
  strm << "LUFTFEUCHTE=" << luftfeuchte << endl;
  strm << "WINDRICHTUNG=" << windrichtung << endl;
  strm << "WINDGESCHWINDIGKEIT=" << windgeschwindigkeit << endl;
  strm << "BOEN=" << boen << endl;
  strm << "NIEDERSCHLAG1=" << niederschlag1 << endl;
  strm << "NIEDERSCHLAG4=" << niederschlag4 << endl;
  strm << "NIEDERSCHLAGM=" << niederschlag_m << endl;
}

void wx_config_file::show(ostream &strm )
{
  station.set_format(true);
  station.set_nossid(false);

  strm << station << " : " << loc << "    " << setw(2) << slot << "  " << pfad;
}

void wx_config_file::full_show(ostream &strm, char cr )
{
  station.set_format(true);
  station.set_nossid(false);

  strm << "WX-Station             : " << station << cr;
  strm << "Locator                : " << loc << cr;
  strm << "Connect-Pfad           : " << pfad << cr;
  strm << "Slot                   : " << slot << cr;
  strm << "Prompt                 : " << prompt << cr;
  strm << "Command                : " << command << cr;
  strm << "Temperatur             : " << temperatur << cr;
  strm << "Luftdruck              : " << luftdruck << cr;
  strm << "Luftfeuchte            : " << luftfeuchte << cr;
  strm << "Windrichtung           : " << windrichtung << cr;
  strm << "Windgeschwindigkeit    : " << windgeschwindigkeit << cr;
  strm << "Boen                   : " << boen << cr;
  strm << "Niederschlag letzte 24h: " << niederschlag1 << cr;
  strm << "Niederschlag letzte 4h : " << niederschlag4 << cr;
  strm << "Niederschlag seit 0 Uhr: " << niederschlag_m << cr;
}


wx_config_file::~wx_config_file( void )
{
  if (save_flag)
    {
      ofstream ostr(strtochar(dateiname));
      if (ostr)
	PrintOn(ostr);
    }
}


/*
  ----------------------------------------------------------------------------
  Eine weitere Klasse wx_config_files fasst alle Konfigurationsdateien
  zusammen. Die Methoden dieser Klasse folgen jetzt
  ----------------------------------------------------------------------------
*/

wx_config_files::wx_config_files( void )
{
  files.clear();
}

wx_config_files::wx_config_files( config_file& cfg )
{
  files.clear();
  syslog slog(cfg);
  try
    {
      wx_dir_name = cfg.find("WX");
      DIR *wx_dir;
      wx_dir = opendir(strtochar(wx_dir_name));
      if (wx_dir != NULL)
	{
	  struct dirent *entry;
	  while ((entry = readdir(wx_dir)) != NULL )
	    {
	      if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
		{
		  String fname(wx_dir_name + entry->d_name);
		  if (fname.in(String(".wx")))
		    try
		      {
			files.push_back(wx_config_file(fname));
		      }
		    catch(Error_could_not_open_wx_configfile)
		      {
			slog.eintrag("WX-Configurationsfile "+fname+" nicht zu oeffnen",LOGMASK_PRGRMERR);
		      }
		    catch(Error_unknown_parameter_in_wx_configfile)
		      {
			slog.eintrag("Unbekannter Parameter in WX-Config-Datei "+fname,LOGMASK_PRGRMERR);
		      }
		    catch(Error_wrong_format_in_wx_config_file)
		      {
		     	slog.eintrag("Falsches Dateiformat in WX-Config-Datei "+fname,LOGMASK_PRGRMERR);
		      }
		}
	    }
	  closedir(wx_dir);
	}
    }
  catch( Error_parameter_not_defined )
    {
    }
}

bool wx_config_files::get_first( wx_config_file &f )
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

bool wx_config_files::get_next( wx_config_file &f )
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

void wx_config_files::add( wx_config_file wxcfg )
{
  String dname = wx_dir_name + wxcfg.station.str() + ".wx";
  wxcfg.dateiname = dname;
  wxcfg.save_flag = false;

  files.push_back(wxcfg);
}

bool wx_config_files::del( const callsign &call )
{
  for (vector<wx_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->station))
	{
	  remove(strtochar(i->dateiname));
	  files.erase(i);
	  it = files.begin();
	  return true;
	}
    }
  return false;
}

bool wx_config_files::set( const callsign &call, const wx_config_file &wxcfg )
{
  for (vector<wx_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->station))
	{
	  *i = wxcfg;
	  i->save_flag = true;
	  return true;
	}
    }
  return false;
}

wx_config_file& wx_config_files::find( const callsign &call )
{
  for (vector<wx_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->station))
	return *i;
    }
  throw Error_unknown_wx_station();
}

void wx_config_files::show( ostream &ostr , char cr )
{
  for (vector<wx_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      i->show(ostr);
      ostr << cr;
    }
}

void wx_config_files::full_show(const callsign &call, ostream &ostr, char cr)
{
  for (vector<wx_config_file>::iterator i = files.begin() ; i != files.end() ; ++i)
    {
      if (samecall(call,i->station))
         i->full_show(ostr,cr);
    }
}


/*
  ----------------------------------------------------------------------------
  Nun folgen die Methoden der Klasse wx_meldung
  ----------------------------------------------------------------------------
*/

wx_meldung::wx_meldung( void )
{
  temp = -300.;
  druck = -1.;
  feucht = -1.;
  wind_speed = -1;
  wind_dir = -1;
  boen = -1;
  rain_24 = -1.;
  rain_4 = -1.;
  rain_mn8 = -1.;
}

String wx_meldung::spool_msg( void ) const
{
  // Erste Zeile enthaelt Rufzeichen und Locator:
  callsign c = call;

  c.set_format(true);
  c.set_nossid(true);
  String tmp = c.call() + "        ";
  tmp.append(loc.str());

  // Zweite Zeile enthaelt Informationen ueber Wind
  if (wind_dir >= 0)
    tmp.append("D="+itoS(wind_dir,3));
  else
    tmp.append("D=---");
  // Richtung : 5 Zeichen
  tmp.append("  ");
  if (wind_speed >= 0)
    tmp.append("S="+itoS(wind_speed,3));
  else
    tmp.append("S=---");
  // Geschwindigkeit : 5 Zeichen
  tmp.append("  ");
  if (boen >= 0)
    tmp.append("G="+itoS(boen,4));
  else
    tmp.append("G=----");
  // Wind Boen : 6 Zeichen


  // Dritte Zeile Enthaelt Luftdruck, Temperatur und Luftfeuchte
  if (druck > 0.)
    tmp.append("P="+dtoS((double) druck,4,0));
  else
    tmp.append("P=----");
  // Druck : 6 Zeichen
  tmp.append(" ");

  if (temp > -274.)
    tmp.append("T="+dtoS((double) temp,5,1));
  else
    tmp.append("T=-----");
  // Temperatur : 7 Zeichen
  tmp.append(" ");
  if (feucht > 0.)
    tmp.append("H="+dtoS((double) feucht,3,0));
  else
    tmp.append("H=---");
  // Luftfeuchte : 5 Zeichen


  // Die vierte Zeile enthaelt Informationen ueber den Niederschlag
  if (rain_24 >= 0.)
    tmp.append("1="+dtoS((double) rain_24,3));
  else
    tmp.append("1=---");
  // Niederschlag der letzten 24 Stunden : 5 Zeichen
  tmp.append("   ");

  if (rain_4 >= 0.)
    tmp.append("4="+dtoS((double) rain_4,3));
  else
    tmp.append("4=---");
  // Niederschlag der letzten 4 Stunden : 5 Zeichen
  tmp.append("  ");
  if (rain_mn8 > 0.)
    tmp.append("M="+dtoS((double) rain_mn8,3));
  else
    tmp.append("M=---");
  // Niederschlag seit Mitternacht : 5 Zeichen

  return tmp;
}

String wx_meldung::asynop_msg( void ) const
{
// Laesst sich nicht compilieren! Auskommentiert von DH6BB am 20.05.
// Holger, bitte pruefen!
/*
  char block[6];
  block[5] = '\0';
  String asynop;

  int d = von.get_tage();
  int h = von.get_stunden();

  block[0] = (char) ((d / 10) + 48);
  block[1] = (char) ((d % 10) + 48);
  block[2] = (char) ((h / 10) + 48);
  block[3] = (char) ((h % 10) + 48);
  block[4] = '1';
  asynop = String(block)+ String(' ');

  asynop.append(loc.asynop() + String(' '));
  asynop.append(String("46/00 "));

  block[0] = '/';
  block[1] = (char) ((wind_dir / 100) + 48);
  block[2] = (char) (((wind_dir / 10) % 10) + 48);
  block[3] = (char) ((wind_speed / 10) + 48);
  block[4] = (char) ((wind_speed % 10) + 48);
  asynop = String(block)+ String(' ');

  block[0] = '1';
  if ( temp > 0)
    block[1] = '0';
  else
    block[1] = '1';
  float atemp = fabs(temp);
  block[2] = (char) ((int) (atemp / 10.) + 48);
  block[3] = (char) ((((int) atemp) % 10 ) + 48);
  block[4] = (char) (((int) (atemp * 10.) % 10 ) + 48);
  asynop = String(block)+ String(' ');

  block[0] = '2';
  block[1] = '9';
  block[2] = (char) ((int) (feucht / 100.) + 48);
  block[3] = (char) ((int) (feucht / 10.) + 48);
  block[4] = (char) ((((int) feucht) % 10 ) + 48);
  asynop = String(block)+ String(' ');

  float p_red;
  if (druck >= 1000.)
    p_red = druck - 1000.;
  else
    p_red = druck;
  block[0] = '4';
  block[1] = (char) ((int) (p_red / 100.) + 48);
  block[2] = (char) ((int) (p_red / 10.) + 48);
  block[3] = (char) ((((int) p_red) % 10 ) + 48);
  block[4] = (char) (((int) (p_red * 10.) % 10 ) + 48);
  asynop = String(block);
  // Das muss erst noch implementiert werden...
  return asynop;
*/
    return ""; //DH6BB 06.05. wegen Compiler-Warning
}

/*
  ----------------------------------------------------------------------------
  Nun folgen die methoden der Klasse wx_control
  ----------------------------------------------------------------------------
*/


wx_control::wx_control( void )
{
  start_flag = false;
  start_first = false;
  activ = false;
  last_fetch = zeit(-1);
  //ds = get_default_destin();
}

void wx_control::load( config_file & cfg )
{
  wxfiles = wx_config_files(cfg);
  ds = get_default_destin();
  try
    {
      String en = cfg.find("WX_AUSLESE");
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

void wx_control::enable( config_file &cfg )
{
  cfg.set("WX_AUSLESE","JA");
  cfg.save();
  activ = true;
}

void wx_control::disable( config_file &cfg )
{
  cfg.set("WX_AUSLESE","NEIN");
  cfg.save();
  activ = false;
}

bool wx_control::add( const callsign &call )
{
  wx_config_file wxcfg;

  wxcfg.station = call;
  wxfiles.add(wxcfg);
  return true;
}

bool wx_control::del( const callsign &call )
{
  return wxfiles.del(call);
}

bool wx_control::set_loc( const callsign &call, const locator &l )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.loc = l;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_pfad( const callsign &call, const connect_string &pfd )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.pfad = pfd;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_slot( const callsign &call, int slt )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.slot = slt;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_prompt( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.prompt = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_command( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.command = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_temperatur( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.temperatur = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_luftdruck( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.luftdruck = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_luftfeuchte( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.luftfeuchte = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_windrichtung( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.windrichtung = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_windgeschwindigkeit( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.windgeschwindigkeit = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_boen( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.boen = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_niederschlag1( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.niederschlag1 = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_niederschlag4( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.niederschlag4 = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

bool wx_control::set_niederschlag_m( const callsign &call, const String &val )
{
  try
    {
      wx_config_file wxcfg = wxfiles.find(call);

      wxcfg.niederschlag_m = val;
      return wxfiles.set(call,wxcfg);
    }
  catch( Error_unknown_wx_station )
    {
      return false;
    }
}

void wx_control::start( void )
{
  if (activ)
    {
      start_flag = true;
      start_first = true;
    }
}

bool wx_control::start_connection( wx_config_file &wxcfg )
{
  if (start_flag)
    {
      if (start_first)
	{
	  if ( !wxfiles.get_first(wxcfg) )
	    {
	      start_flag = false;
	      start_first = false;
	      return false;
	    }
	  else
	    {
	      start_first = false;
	      last_fetch = zeit();
	      return true;
	    }
	}
      else
	if ( !wxfiles.get_next(wxcfg) )
	  {
	    start_flag = false;
	    return false;
	  }
	else
	  {
	    last_fetch = zeit();
	    return true;
	  }
    }
  else
    return false;
}


void wx_control::meldung( int slot, const wx_meldung &mldg )
{
  // Als erstes wird die Wettermeldung ueber die Export-Funktion 
  // ausgegeben.
  try
    {
      export_sys exp;
      exp.write_wx(mldg);
    }
  catch( Error_could_not_initialize_exportsystem )
    {
      // Hat nicht geklappt...
    }
  String msg = mldg.spool_msg();
  spool_msg(slot,msg);
}

void wx_control::spool_msg( int sl, const String &msg )
{
  String rub = RUB_WETTER;
  int slot;
  syslog logf(configuration);

  try
    {  
      board brd(rub,configuration);
      int board = brd.get_brd_id();
      
      if (sl > 10)		//"Rotierende" Slots
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
      logf.eintrag("Wetter Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Wetter Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}

void wx_control::show( ostream &ostr, char cr )
{
  wxfiles.show(ostr,cr);
  ostr << cr << cr;
  ostr << "Status : ";

  if (activ)
    ostr << "  activ";
  else
    ostr << "inactiv";

  if ( last_fetch != zeit(-1) )
    {
      ostr << "  Last fetch : ";
      
      delta_t dt( zeit() - last_fetch );

      ostr << dt;
    }
  ostr << cr  << cr;
}

void wx_control::full_show(const callsign &call, ostream &ostr, char cr)
{
  wxfiles.full_show(call, ostr, cr);
}
