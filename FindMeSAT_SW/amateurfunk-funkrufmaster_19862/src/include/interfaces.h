/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
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

#ifndef __INTERFACES_H__
#define __INTERFACES_H__

#include <iostream>
#include <sstream>

#include "config.h"
#include "logfile.h"
#include "String.h"
#include "connect_string.h"

using namespace std;

class interfaces // Mutterklasse fuer alle Interface-Klassen
{
 protected:
  syslog logf;
  userlog ulogf;
  zeit login;
  int thread_id;
  char interface_id;
  char connection_id;
  char cr,ende;

  connect_string c_path;
  bool path_finished;
  bool wait_connect;
  bool connect_failed;
  bool connect_prompt;
  callsign wait_digi;
  callsign gegenstation;
  unsigned int next_digi;
  zeit last_activity;

  int in_b,out_b;
  unsigned int in_msg, out_msg;
  String input_puffer;

 protected:


  void cut_blanks( istream & strm );
  String cut_blanks( const String &cmd );
  void cut_blank( istream & strm ); // 1 Blank
  void cut_line(String & );

  bool get_line( String& );

  // set_connect_path initialisiert den Autoconnecter, indem der Connect-Pfad 
  // gesetzt wird.
  void set_connect_path( const connect_string& );
  
  // connect_path ueberprueft die Eingabe ob ein zwischenziel erreicht wurde 
  // und fuehrt dann ggf. den naechsten Schritt aus.
  void connect_path( String, String& );

  // Beim ersten Connect-Schritt muss vorher keine Eingabe ueberprueft werden.
  // Daher eine eigene Methode
  void first_connect(String& );



  // Jetzt folgt noch der Konstruktor, ein virtueller Destruktor und die 
  // virtuelle Methode process, die in den einzelnen abgeleiteten Klassen 
  // implementiert werden muss.
 public:
  interfaces(String&, bool );
  virtual ~interfaces( void );
  virtual bool do_process( bool, String& );
  bool process( const String&, String& );
  callsign get_connected_digi( void );
  void set_io( char , int, int );
  inline void set_thread_id( int tid )
    {
      thread_id = tid;
      //cerr << "thread-ID gesetzt : " << thread_id << endl;
    }
};

#endif
