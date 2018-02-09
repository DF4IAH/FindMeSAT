/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002 by Holger Flemming                                    *
 *                                                                          *
 * This Program is free software; yopu can redistribute ist and/or modify   *
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

#ifndef __CRONTAB_H__
#define __CRONTAB_H__

#include <iostream>
#include <vector>


#include "zeit.h"
#include "String.h"
#include "config.h"

using namespace std;

class Error_crontab_number_read_error
{
#ifdef _DEBUG_EXEC_
 public:
  Error_crontab_number_read_error()
    {
      cerr << "Error_crontab_number_read_error" << endl;
    }
#endif
};

class Error_no_utc_avalable
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_utc_avalable()
    {
      cerr << "Error_no_utc_avalable" << endl;
    }
#endif
};

class crontab_time
{
 private:
  vector<int> mins;
  vector<int> stds;
  vector<int> wts;
  vector<int> tgs;
  vector<int> mons;

 public:
  crontab_time(void );
  friend istream& operator>>( istream&, crontab_time& );
  friend ostream& operator<<( ostream&, crontab_time& );
  bool operator== (zeit& );
};

enum ct_cmd_typ { ct_cmd_no, ct_cmd_user_defined, ct_cmd_bake, ct_cmd_boards, ct_cmd_dbsave, ct_cmd_brdmsg, ct_cmd_wx, ct_cmd_sat, ct_cmd_statistics, ct_cmd_gezeiten, ct_cmd_astro, ct_cmd_purge, ct_cmd_digi };

class crontab_command
{
 private:
  ct_cmd_typ typ;
  String cmd_string;

 public:
  crontab_command( ct_cmd_typ = ct_cmd_no );
  friend istream& operator>> (istream& , crontab_command& );
  friend ostream& operator<< (ostream& , crontab_command& );
  inline ct_cmd_typ get_typ( void )
    {
      return typ;
    }
  inline String get_cmd_string( void )
    {
      return cmd_string;
    }
};

class crontab_entry
{
 private:
  crontab_time time;
  crontab_command Kommando;

 public:
  crontab_entry( void );
  bool get_entry( istream & );
  friend ostream& operator<<( ostream &, crontab_entry & );
  crontab_command test_entry( void );
};

class crontab
{
 private:
  vector<crontab_entry> entries;
  vector<crontab_command> commands;
  zeit last_check;

 public:
  crontab( config_file &cfg );
  const vector<crontab_command> & test_crontab( void );
  friend ostream& operator<<( ostream& , crontab & );
};

#endif
