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

#ifndef __CALLSIGN_H__
#define __CALLSIGN_H__

#include <iostream>
#include <hashtable.h>
#include "String.h"

using namespace std;


class Error_no_callsign
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_callsign()
    {
      cerr << "Error_no_callsign" << endl;
    }
#endif
};

// Klasse zur representation eines Rufzeichens

class callsign
{
 private:
  String callsg;
  int ssid;
  bool formatiert,nossid;

  bool ispref( String &rz, unsigned int &index );
  bool issuf( String &rz, unsigned int &index );
  bool iscall( String &rz, unsigned int &index );

 public:
  callsign();
  callsign(String );

  inline void set_format( bool f )
    {
      formatiert = f;
    }
  inline void set_nossid( bool f )
    {
      nossid = f;
    }
  String call() const;
  friend ostream& operator<< ( ostream&, const callsign& );
  friend istream& operator>> ( istream&, callsign& );
  friend bool operator== ( const callsign&, const callsign& );
  friend bool operator!= ( const callsign&, const callsign& );
  friend bool samecall( const callsign&, const callsign& );
  inline String str( void ) const
    {
      return callsg;
    }
  inline int get_ssid( void ) const
    {
      return ssid;
    }
  callsign get_nossidcall( void ) const;
};
// Es wird jetzt noch ein Hasher-Klasse benoetigt, die mit Hilfe der
// STL-Hash<T>-Klasse aus Rufzeichen einen Hash-Wert bildet.
 
class callsign_hash
{
 private:
  hash<const char*> hash_function;

 public:
  size_t operator() (const callsign& c)
    {
      callsign ca = c;
      String cs = ca.str();
      return (hash_function(strtochar(cs)));
    }
};

// Und zum Schluss noch eine Struktur, die einen Operator enthaelt,
// der die gleichheit zweier Rufzeichen feststellen kann.

struct eqcall
{
  bool operator() ( const callsign& c1, const callsign &c2) const
  {
    return c1 == c2;
  }
};

#endif
