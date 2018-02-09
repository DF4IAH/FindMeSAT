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

#ifndef __DIGI_H__
#define __DIGI_H__
#include <iostream>
#include "interfaces.h"
#include "String.h"

#define MASKE_UPTIME      	1
#define MASKE_DESTINATION	2
#define MASKE_NODES		4
#define MASKE_LINK		8

//#define MASKE_ALL           3


using namespace std;

class Error_could_not_open_digi_configfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_digi_configfile()
    {
      cerr << "Error_could_not_open_digi_configfile" << endl;
    }
#endif
};

class Error_unknown_parameter_in_digi_configfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_unknown_parameter_in_digi_configfile()
    {
      cerr << "Error_unknown_parameter_in_digi_configfile" << endl;
    }
#endif
};

class Error_wrong_format_in_digi_config_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_format_in_digi_config_file()
    {
      cerr << "Error_wrong_format_in_digi_config_file" << endl;
    }
#endif
};

class Error_unknown_digi
{
#ifdef _DEBUG_EXEC_
 public:
  Error_unknown_digi()
    {
      cerr << "Error_unknown_digi" << endl;
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

enum digi_typ { unknown, flexnet, tnn, xnet };


class digi_config_file
{
 public:
  String dateiname;
  unsigned int slot;
  bool enabled;

  callsign digi;
  connect_string pfad;
  digi_typ typ;
  int link_anzahl;   
  callsign link_call[10];

  digi_config_file();
  digi_config_file(String &);
  ~digi_config_file( void );
  void PrintOn( ostream & );
  void show(ostream &, char);
  void full_show(ostream &, char);
  void read(String & );
  void save(void);
  static String digi_typen[5];
};

class digi_meldung
{
 public:
  callsign call;
  zeit von;
  
  delta_t uptime;
  int destin;
  int nodes;
  int gefundene_links;
  String link_rtt[10];

  digi_meldung( void );
  String spool_msg_digi( void ) const;
  String spool_msg_link(digi_config_file &) const;
};


class digi_config_files
{
 protected:
  vector<digi_config_file> files;
  vector<digi_config_file>::iterator it;

  String digi_dir_name;

 public:
  digi_config_files( void );
  digi_config_files(config_file& );
  bool get_first( digi_config_file& );
  bool get_next( digi_config_file& );
  bool add( digi_config_file );
  bool del( const callsign & );
  bool set( const callsign & , const digi_config_file& );
  digi_config_file& find( const callsign & );
  void show( ostream &, char );
  void full_show(const callsign &, ostream &, char);
};


class digi_control
{
 protected:
  digi_config_files digifiles;
  bool start_flag;
  destin ds;

  bool start_first;
  bool activ;
  zeit last_fetch;

 public:
  digi_control();
  ~digi_control();
  
  void load( config_file & );
  bool add( const callsign & );
  bool del( const callsign & );
  bool add_link( const callsign &, const callsign & );
  bool del_link( const callsign &, const callsign & );
  void enable( config_file& );
  void disable( config_file& );
  bool set_loc( const callsign &, const locator & );
  bool set_pfad( const callsign &, const connect_string & );
  bool set_slot( const callsign &, int  );
  bool set_typ( const callsign &, const String & );
  bool set_command( const callsign &, const String & );
  bool set_status(const callsign &, bool );
  void start( void );
  bool start_connection( digi_config_file & );
  void meldung( int, const digi_meldung &, digi_config_file &);
  void spool_msg( int , const String &, String );
  void show( ostream &, char );
  void full_show(const callsign &, ostream &, char );
  
  digi_typ typ;
};

class digi_status : public interfaces
{
 protected:
 
  digi_config_file digi_cfg; // Hier werden die Daten der Konfiguration gespeichert
  
  unsigned int maske;
  destin ds;
  zeit last_activity;
  
 public:
  digi_status(String &, bool, digi_config_file& );
  ~digi_status();
};


class digi_interface : public interfaces
{
 protected:
  digi_config_file digi_cfg; // Hier werden die Daten der Konfiguration gespeichert
  digi_meldung mdg;
  unsigned int maske;
  destin ds;
  zeit last_activity;

 protected:
  bool check_tag( const String &, const String & );
  String get_part(const String &, const String & );
  void digi_line(String);
  void flex_digi_line(String);
  void tnn_digi_line(String);
  void xnet_digi_line(String);
  void set_maske( void );
  String rtt_calc(String, digi_typ);

 public:
  bool do_process( bool, String & );
  digi_interface(String &, bool, digi_config_file& );
  ~digi_interface();
};

#endif
