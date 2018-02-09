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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#ifndef __GRUPPE_H__
#define __GRUPPE_H__

#include <iostream>
#include <vector>
#include <algo.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>

#include "String.h"
#include "callsign.h"
#include "config.h"
#include "globdef.h"
#include "database.h"

using namespace std;

class Error_could_not_open_groupfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_groupfile()
    {
      cerr << "Error_could_not_open_groupfile" << endl;
    }
#endif
};

class Error_Callsign_already_in_group
{
#ifdef _DEBUG_EXEC_
 public:
  Error_Callsign_already_in_group()
    {
      cerr << "Error_Callsign_already_in_group" << endl;
    }
#endif
};

class Error_Callsign_not_in_group
{
#ifdef _DEBUG_EXEC_
 public:
  Error_Callsign_not_in_group()
    {
      cerr << "Error_Callsign_not_in_group" << endl;
    }
#endif
};

class Error_no_more_call_in_group
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_more_call_in_group()
    {
      cerr << "Error_no_more_call_in_group" << endl;
    }
#endif
};

class Error_group_does_not_exist
{
#ifdef _DEBUG_EXEC_
 public:
  Error_group_does_not_exist()
    {
      cerr << "Error_group_does_not_exist" << endl;
    }
#endif
};

class Error_group_already_exist
{
#ifdef _DEBUG_EXEC_
 public:
  Error_group_already_exist()
    {
      cerr << "Error_group_already_exist" << endl;
    }
#endif
};

class Error_group_not_empty
{
#ifdef _DEBUG_EXEC_
 public:
  Error_group_not_empty()
    {
      cerr << "Error_group_not_empty" << endl;
    }
#endif
};

class Error_no_group_name
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_group_name()
    {
      cerr << "Error_no_group_name" << endl;
    }
#endif
};

class Error_could_not_open_groups
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_groups()
    {
      cerr << "Error_could_not_open_groups" << endl;
    }
#endif
};

class gruppe
{
 private:
  typedef vector<callsign> t_rufzeichenliste;
  
  String beschreibung;
  t_rufzeichenliste rufzeichenliste;
  t_rufzeichenliste::iterator index;

  String name;
  String filename;
  bool changed;
  bool dataset;

 public:
  gruppe( void );
  gruppe( const String& , const String&, const String& = "" );
  ~gruppe( void );
  inline String get_name( void ) const
    {
      return name;
    }
  inline String get_info( void ) const
    {
      return beschreibung;
    }
  inline int get_anzahl( void ) const
    {
      return rufzeichenliste.size();
    }
  void add_call( callsign& );
  void del_call( callsign& );
  void find_call( callsign& );
  void change_beschr( const String& );
  callsign get_first();
  callsign get_next();
};

struct gruppe_info
{
  String name;
  int anz;
  String info;
};

class gruppen
{
 private:
  String gdb_pathname;
  char cr;

 public:
  gruppen( config_file & );
  void add_group( const String &, const String & );
  void del_group( const String & );
  gruppe find( const String & );
  vector<struct gruppe_info> get_infos( void );
};

#endif
