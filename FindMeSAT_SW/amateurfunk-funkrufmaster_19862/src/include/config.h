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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>
#include <map>
#include <vector>

using namespace std;

#include "String.h"


//#include "array_map.h"

class Error_parameter_not_defined
{
#ifdef _DEBUG_EXEC_
 public:
  Error_parameter_not_defined()
    {
      cerr << "Error_parameter_not_defined" << endl;
    }
#endif
};

class Error_could_not_open_configfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_configfile()
    {
      cerr << "Error_could_not_open_configfile" << endl;
    }
#endif
};

class Error_config_file_format_error
{
  String line;
 public:
  Error_config_file_format_error( const String& );
  String get_error_line( void );
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

class config_file
{
 private:

  struct ltstr
    {
      bool operator()(const String &s1, const String &s2) const
      {
	return s1 < s2;
      }
    };
  //typedef array_map<String,String> t_config;
  typedef map<String,String,ltstr> t_config;
  t_config Configuration;
  char filename[255];
  bool loaded;


 public:
  config_file();
  ~config_file();
  void save( void );
  void load( void );

  String find(const String & );  
  void set( const String& , const String& );
  void clear( const String& );
  void printOn( ostream&, char = '\n' );
};

inline ostream& operator<<( ostream& strm, config_file &cfg )
{
  cfg.printOn(strm);
  return strm;
}

vector<String> komma_separeted( const String & );

#endif
