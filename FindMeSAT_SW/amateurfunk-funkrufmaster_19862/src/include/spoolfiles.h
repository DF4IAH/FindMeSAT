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

#ifndef __SPOOLFILES_H__
#define __SPOOLFILES_H__

#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>

#include "board.h"
#include "config.h"
#include "String.h"
#include "adress.h"
#include "callsign.h"
#include "zeit.h"
#include "destin.h"
#include "statistics.h"

using namespace std;


class Error_no_more_directories
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_more_directories()
    {
      cerr << "Error_no_more_directories" << endl;
    }
#endif
};

class Error_could_not_open_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_file()
    {
      cerr << "Error_could_not_open_file" << endl;
    }
#endif
};

class Error_could_not_open_directory
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_directory()
    {
      cerr << "Error_could_not_open_directory" << endl;
    }
#endif
};


// Die Klasse spoolfiles enthaellt alle notwendigen Methoden um
// das recht komplexe Spooldirectory zu organisieren.
// Zudem koennen nacheinander in allen Spool-Unterdirectories
// Dateien angelegt werden.


class spoolfiles
{
  struct entry
    {
      String pfad_name;
      own_destins destinations;
  };

  typedef vector<entry> t_spooldirs;

 private:
  t_spooldirs spooldirs;
  t_spooldirs::iterator it;
  spoolstatistic stat;

 public:
  spoolfiles( void );
  void add_dir( const String &, const own_destins& );
  void del_dir( const String & );
  void first();
  void next();
  bool check_destin( const destin& d);
  void get_file( bool, ofstream&, unsigned int );
  void spool_actseq( adress );
  bool spool_msg(bool, callsign, const callsign&, zeit, adress, String, bool, const destin&, unsigned int );
  bool spool_msg(adress, String, const destin&, unsigned int );
  bool spool_bul( const callsign&, zeit, int, int, String, bool, const destin&, unsigned int, bool = false);
  unsigned int get_count(String&);
  // slots muessen hier im Bereich von 1 bis 10 sein!

  bool spool_brd_name( String , int );
  inline spoolstatistic get_stat( void )
    {
      return stat;
    }
  void show(ostream& , char );
};


#endif
