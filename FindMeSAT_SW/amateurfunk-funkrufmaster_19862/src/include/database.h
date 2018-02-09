/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
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

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <iostream>

#include "String.h"
#include "callsign.h"
#include "my_hash_map.h"
#include "adress.h"
#include "locator.h"
#include "zeit.h"

using namespace std;

class Error_callsign_already_exists
{
#ifdef _DEBUG_EXEC_
 public:
  Error_callsign_already_exists()
    {
      cerr << "Error_callsign_already_exists" << endl;
    }
#endif
};

class Error_adress_does_not_exist
{
#ifdef _DEBUG_EXEC_
 public:
  Error_adress_does_not_exist()
    {
      cerr << "Error_adress_does_not_exist" << endl;
    }
#endif
};


class Error_callsign_does_not_exist
{
#ifdef _DEBUG_EXEC_
 public:
  Error_callsign_does_not_exist()
    {
      cerr << "Error_callsign_does_not_exist" << endl;
    }
#endif
};

class Error_wrong_database_version
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_database_version()
    {
      cerr << "Error_wong_database_version" << endl;
    }
#endif
};

class Error_can_not_create_database
{
#ifdef _DEBUG_EXEC_
 public:
  Error_can_not_create_database()
    {
      cerr << "Error_can_not_create_database" << endl;
    }
#endif
};

class Error_illegal_database_entry
{
#ifdef _DEBUG_EXEC_
 public:
  Error_illegal_database_entry()
    {
      cerr << "Error_illegal_database_entry" << endl;
    }
#endif
};

class pager_typ
{
 private:
  int typ;
  static String types[20];

 public:
  inline pager_typ()
    {
      typ = 0;
    }
  pager_typ( String  );
  String get_string( void ) const;
  bool operator==( const pager_typ & );
  bool operator!=( const pager_typ & );
  friend ostream& operator<< ( ostream&, const pager_typ& );
  friend istream& operator>> ( istream&, pager_typ& );
  friend ostream& operator< ( ostream&, const pager_typ& );
  friend istream& operator> ( istream&, pager_typ& );
};

class database_entry
{
 private:
  callsign call;
  adress adresse;
  zeit last_change;
  callsign server;
  pager_typ geraet;
  locator loc;
  String name;
  String language;
  zeit last_login;
  String ax25_pw;
  String consol_pw;

 public:
  inline database_entry( void )
    {
      language = String("dl");
      last_login = zeit(-1);
    }
  database_entry(const callsign&, const adress &, pager_typ, const callsign& );
  database_entry( const String & , float );
  inline callsign get_call( void ) const
    {
      return call;
    }
  inline adress get_adr( void ) const
    {
      return adresse;
    }
  inline zeit get_last_change( void ) const 
    {
      return last_change;
    }
  inline pager_typ get_geraet( void ) const
    {
      return geraet;
    }
  inline locator get_locator( void ) const
    {
      return loc;
    }
  inline String get_name( void ) const
    {
      return name;
    }
  inline callsign get_server( void ) const
    {
      return server;
    }
  inline String get_language( void ) const
    {
      return language;
    }
  inline zeit get_last_login( void ) const
    {
      return last_login;
    }
  inline String get_ax25_pw( void ) const
    {
      return ax25_pw;
    }
  inline String get_consol_pw( void ) const
    {
      return consol_pw;
    }
  void set_call( const callsign& , const callsign&, zeit = zeit() );
  void set_adr( const adress &, const callsign&, zeit = zeit() );
  void set_geraet( const pager_typ, const callsign&, zeit = zeit() );
  void set_loc( const locator&, const callsign&, zeit = zeit() );
  void set_name( const String &, const callsign&, zeit = zeit() );
  void set_language( const String &, const callsign&, zeit = zeit() );
  void set_ax25_pw( const String &, const callsign&, zeit = zeit() );
  void set_consol_pw( const String &, const callsign&, zeit = zeit() );
  void login( void );
  void update( const callsign & );

  //  friend ostream& operator<<( ostream& , database_entry & );
  //  friend istream& operator>>( istream&, database_entry & );
  friend ostream& operator<( ostream& , database_entry & );
};

// Die Klasse callsign_database realisiert eine rufzeichendatenbank, die beim Aufruf des
// Konstruktors aus einer Datei eingelesen wird und bei einer Veraenderung im Destruktor
// wieder abgespeichert wird.
// Unter Angabe eines Rufzeichens sucht die Methode find nach der entsprechenden 
// Pageradresse.

/*
struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
*/
struct eqstr
{
  bool operator()(const String &s1, const String &s2) const
  {
    return s1 == s2;
  }
};

class StringHash
{
 private:
  hash<const char*> hash_function;

 public:
  size_t operator() (const String& s)
    {
      return (hash_function(strtochar(s)));
    }
};

class callsign_database
{
 private:
  //typedef array_map<callsign,database_entry> t_database;
  //typedef my_hash_map<const char*,database_entry,hash<const char*>,eqstr> t_database;
  typedef my_hash_map<String, database_entry,StringHash,eqstr> t_database;
  //typedef hash_map<const char* ,database_entry,hash<const char*>,eqstr> t_database;
  t_database Database;
  t_database::iterator first_next_it;

  String database_filename;
  bool changed;
  bool valid;

 public:
  callsign_database();
  callsign_database( const String& );
  ~callsign_database();

  callsign_database( const callsign_database& );
  callsign_database operator=( const callsign_database& );

  void save( void );
  inline callsign_database new_db( void )
    {
      return callsign_database( database_filename );
    }

  void add( const database_entry& );
  void del( const callsign& );

  database_entry find(const callsign& );
  database_entry find_adress( const adress& );
  bool first( database_entry& );
  bool next( database_entry& );
  void change(const callsign&, const database_entry& );
  void change_call(const callsign&, const callsign& , const callsign& ); 
  vector<database_entry> get_user( const String &, bool );
  vector<database_entry> get_other_pager( const callsign & );
  //friend ostream& operator<<(ostream &, callsign_database &);
  //void print_part(ostream&, String );
  //void print_other_pager(ostream &, String);
  int get_size_pager( void );
  inline int get_size( void )
    {
      return Database.size();
    }
};

#endif
