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

#ifndef __DESTIN_H__
#define __DESTIN_H__

#include <iostream>
#include <sstream>
#include <ctype.h>
#include <vector>

#include "String.h"

using namespace std;

class Error_no_destin
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_destin()
    {
      cerr << "Error_no_destin" << endl;
    }
#endif
};

class Error_could_not_generate_own_dests
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_generate_own_dests()
    {
      cerr << "Error_could_not_generate_own_dests" << endl;
    }
#endif
};

class Error_destin_checksum_error
{
#ifdef _DEBUG_EXEC_
 public:
  Error_destin_checksum_error()
    {
      cerr << "Error_destin_checksum_error" << endl;
    }
#endif
};
// die Klasse destin Representiert ein Zielgebiet eines
// Funkrufs
class destin
{
 private:

  vector<String> felder;
  unsigned short pruefsumme;


 protected:
  void calc_ps( void );
  bool check_ps( void ) const;

 public:
  inline destin( void )
    {
      felder.clear();
      felder.push_back(String('*'));
      calc_ps();
    }
  destin (const String& ); 
  friend ostream& operator<<( ostream&, const destin& );
  friend istream& operator>>( istream&, destin& );
  friend bool operator==( const destin& , const destin& );
  friend bool operator!=( const destin&, const destin& );
  bool in( const destin& ) const ;
  String get_string( void ) const;
  inline bool check( void ) const
    {
      return check_ps();
    }
};

destin get_default_destin( void  );

// own_destin beinhaltet die Zielgebiete, die von diesem Funkrufserver
// abgedeckt werden. Sie werden mit dem Konstruktor aus dem 
// Konfigurationsfile gelesen

class own_destins
{
 private:
  typedef vector<destin> t_dests;
  t_dests dests;

 public:
  own_destins();
  own_destins( const String& );
  bool check_destin( const destin & );
  void PrintOn( ostream& ) const;
  String get_string( void ) const;
  inline vector<destin> get_dests( void ) const
    {
      return dests;
    }
};

inline ostream& operator<<( ostream& strm, const own_destins & d )
{
  d.PrintOn(strm);
  return strm;
}

#endif
