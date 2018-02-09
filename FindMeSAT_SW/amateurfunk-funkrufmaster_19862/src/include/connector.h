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
 *                                                                          *
 ****************************************************************************/

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include <vector>
#include <list>

using namespace std;

#include "String.h"

class connection
{

 protected:
  class puffer_entry
    {
    public:
      String line;
      bool eol_flag;
    };


 public:
  int id;
  bool gestartet;
 protected:
  char typ;
  String adresse;
  bool connected;
  bool disconnect_angefordert;
  bool disconnected;

  vector<puffer_entry>  in_puffer;
  vector<puffer_entry> out_puffer;

 public:
  connection( void );
  connection( char , const String & );
  // Zunächst die Methoden, die von der Connection-Seite benutzt werden.
  char get_typ( void );
  String get_adresse( void );
  void established( void );
  void disconnect( void );
  void recv_line( const String &, bool );
  bool send_line( String &, bool&, bool & );
  inline bool disc_request( void )
    {
      return disconnect_angefordert;
    }

  // Nun die Methoden, die von der Master-Seite benutzt werden.
  void send( const String &, bool );
  bool receive( String &, bool&, bool & );
  void discon ( void );
};

class connection_control
{
 protected:
  list<connection> verbindungen;
  int next_id;

 public:
  connection_control( void );

  // Methoden, die der scheduler benutzt:

  bool start_connection( char &, String & , int& );

  // Methoden, die das connection_interface aufruft:
  void established( int );
  void disconnected( int );
  void received( int, const String &, bool );
  bool to_send( int, String &, bool &, bool& );
  bool disc_request( int );

  // Methoden, die das Benutzerinterface benutzt
  int connect( char, const String & );
  void discon( int );
  void send( int, const String &, bool );
  bool receive( int, String &, bool &, bool&  );
};

#endif // __CONNECTOR_H__
