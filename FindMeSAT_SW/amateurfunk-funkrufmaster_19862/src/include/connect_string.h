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
 ****************************************************************************/

#ifndef __CONNECT_STRING_H__
#define __CONNECT_STRING_H__

#include <vector>
#include "String.h"
#include "callsign.h"

using namespace std;

class Error_syntax_fehler_in_connect_string
{
#ifdef _DEBUG_EXEC_
 public:
  Error_syntax_fehler_in_connect_string()
    {
      cerr << "Error_syntax_fehler_in_connect_string" << endl;
    }
#endif
};

class connect_string
{
 protected:
  String port;
  callsign mycall;
  vector<callsign> l2_digis;
  vector<callsign> digis;
  unsigned int n_digis;
  unsigned int n_l2_digis;

 public:
  connect_string();
  connect_string( const String & );
  void PrintOn( ostream & );
  String get_string( void );
  inline String get_port( void )
    {
      return port;
    }
  inline callsign get_mycall( void )
    {
      return mycall;
    }
  inline callsign get_digi( int i_digi)
    {
      return digis[i_digi];
    }
  inline unsigned int get_digi_anz( void )
    {
      return n_digis;
    }
  inline unsigned int get_l2_digi_anz( void )
    {
      return n_l2_digis;
    }
  inline callsign get_l2_digi( int i_l2_digi )
    {
      return l2_digis[i_l2_digi];
    }
};

inline ostream& operator<<(ostream &strm,  connect_string cpfad )
{
  cpfad.PrintOn(strm);
  return strm;
}
#endif
