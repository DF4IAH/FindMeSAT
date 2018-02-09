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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#ifndef __TEXTE_H__
#define __TEXTE_H__

#include <iostream>
#include <fstream>
#include <string.h>

#include <vector>

#include "globdef.h"
#include "config.h"
#include "makros.h"
#include "String.h"

using namespace std;

// Die Klasse text dient als Basisklasse, die einen Text representiert
// und diesen dann in einen Stream ausgeben kann.
class text
{
 protected:
  makros macros;
  bool makro_flag;
  ifstream source;
  char cr;
  String lang;

 public:
  text(makros &m, const String &l, char wr = '\n')
    {
      macros = m;
      cr = wr;
      lang = l;
    }
  const String& get( void );
  friend ostream& operator<< ( ostream& , text & );
};

// Von der Basisklasse text werden vier weiter Klassen abgeleitet,
// die spezielle Texte representieren.

class ctext : public text
{
 public:
  ctext( config_file &c, makros & , const String & = "dl", char = '\n' );
};

class atext : public text
{
 public:
  atext( config_file &c, makros & , const String & = "dl", char = '\n' );
};

class itext : public text
{
 public:
  itext( config_file &c, makros &, const String & = "dl" , char = '\n' );
};

class htctext : public text
{
 public:
  htctext( config_file &c, makros & , const String & = "dl", char = '\n' );
};

class htatext : public text
{
 public:
  htatext( config_file &c, makros & , const String & = "dl", char = '\n' );
};

class htitext : public text
{
 public:
  htitext( config_file &c, makros &, const String & = "dl" , char = '\n' );
};
class rt_text : public text
{
 public:
  rt_text( const String&, makros&, char = '\n' );
};

class wt_text : public text
{
 protected: 
  String filename;

 public:
  wt_text( const String&, makros&, char = '\n' );
  bool status( void );
  void line( const String& );
};

struct h_eintrag
{
  String cmd;
  vector<String> htxt;
};

class htext : public text
{

 protected:
  bool check_cmd( const String&, const String& );
  String get_cmd( const String& );
 public:
  htext( config_file &, makros&, const String & = "dl", char = '\n' );
  vector<struct h_eintrag>  suche( String , const user & );
  bool print( ostream&, const String&, const user& );
};

#endif
