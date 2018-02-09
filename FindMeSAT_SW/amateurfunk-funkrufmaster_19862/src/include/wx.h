/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
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
 *  		                                			    *
 ****************************************************************************/

#ifndef __WX_H__
#define __WX_H__

#include <vector>

#include "String.h"
#include "callsign.h"
#include "locator.h"
#include "connect_string.h"
#include "config.h"
#include "destin.h"
#include "zeit.h"


using namespace std;

class Error_could_not_open_wx_configfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_wx_configfile()
    {
      cerr << "Error_could_not_open_wx_configfile" << endl;
    }
#endif
};

class Error_unknown_parameter_in_wx_configfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_unknown_parameter_in_wx_configfile()
    {
      cerr << "Error_unknown_parameter_in_wx_configfile" << endl;
    }
#endif
};

class Error_wrong_format_in_wx_config_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_format_in_wx_config_file()
    {
      cerr << "Error_wrong_format_in_wx_config_file" << endl;
    }
#endif
};

class Error_unknown_wx_station
{
#ifdef _DEBUG_EXEC_
 public:
  Error_unknown_wx_station()
    {
      cerr << "Error_unknown_wx_station" << endl;
    }
#endif
};


// Die Klasse config_file dient dazu, ein Konfigurationsfile einzulesen. Das File wird beim
// Aufruf des Konstruktors eingelesen. Veraenderungen nach dem Aufruf des KOnstruktors haben keinen
// Einfluss auf den Programmablauf mehr.
// Das Konfigurationsfile muss folgendes Format haben: Kommentarzeilen beginnen mit #, 
// alle anderen Zeilen haben das Format
// PARAMETER=WERT
//
// Die Methode find sucht anhand eines Strings den Parameter und gibt den Wert als
// String zurueck.
//

class wx_config_file
{
 public:
  String dateiname;
  bool save_flag;

  callsign station;
  locator loc;
  connect_string pfad;
  int slot;
  String prompt;
  String command;
  String temperatur;
  String luftdruck;
  String luftfeuchte;
  String windrichtung;
  String windgeschwindigkeit;
  String boen;
  String niederschlag1;
  String niederschlag4;
  String niederschlag_m;

  
 public:
  wx_config_file();
  wx_config_file(String &);
  ~wx_config_file( void );
  void PrintOn( ostream & );
  void show(ostream &);
  void full_show(ostream &, char);
  void read(String & );
};

class wx_config_files
{
 protected:
  vector<wx_config_file> files;
  vector<wx_config_file>::iterator it;

  String wx_dir_name;

 public:
  wx_config_files( void );
  wx_config_files(config_file& );
  bool get_first( wx_config_file& );
  bool get_next( wx_config_file& );
  void add( wx_config_file );
  bool del( const callsign & );
  bool set( const callsign & , const wx_config_file& );
  wx_config_file& find( const callsign & );
  void show( ostream &, char );
  void full_show(const callsign &, ostream &, char);
};

class wx_meldung
{
 public:
  callsign call;
  locator loc;
  zeit von;

  float temp;
  float druck;
  float feucht;

  int wind_speed;
  int wind_dir;
  int boen;

  float rain_24;
  float rain_4;
  float rain_mn8;

 public:
  wx_meldung( void );
  String spool_msg( void ) const;
  String asynop_msg( void ) const;
};

class wx_control
{
 protected:
  wx_config_files wxfiles;
  bool start_flag;
  destin ds;

  bool start_first;
  bool activ;
  zeit last_fetch;

 public:
  wx_control( void );
  void load( config_file & );
  bool add( const callsign & );
  bool del( const callsign & );
  void enable( config_file& );
  void disable( config_file& );
  bool set_loc( const callsign &, const locator & );
  bool set_pfad( const callsign &, const connect_string & );
  bool set_slot( const callsign &, int  );
  bool set_prompt( const callsign &, const String & );
  bool set_command( const callsign &, const String & );
  bool set_temperatur( const callsign &, const String & );
  bool set_luftdruck( const callsign &, const String & );
  bool set_luftfeuchte( const callsign &, const String & );
  bool set_windrichtung( const callsign &, const String & );
  bool set_windgeschwindigkeit( const callsign &, const String & );
  bool set_boen( const callsign &, const String & );
  bool set_niederschlag1( const callsign &, const String & );
  bool set_niederschlag4( const callsign &, const String & );
  bool set_niederschlag_m( const callsign &, const String & );

  void start( void );
  bool start_connection( wx_config_file & );
  void meldung( int, const wx_meldung & );
  void spool_msg( int , const String & );
  void show( ostream &, char );
  void full_show(const callsign &, ostream &, char );
};

#endif

