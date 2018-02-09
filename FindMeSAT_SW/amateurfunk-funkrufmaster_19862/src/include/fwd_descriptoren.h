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

#ifndef __FWD_DESCRIPTOREN_H__
#define __FWD_DESCRIPTOREN_H__

#include <iostream>
#include <list>

#include "fwd_nachrichten.h"
#include "zeit.h"
#include "String.h"
#include "callsign.h"

using namespace std;


class Error_could_not_open_msg_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_msg_file()
    {
      cerr << "Error_could_not_open_msg_file" << endl;
    }
#endif
};

class Error_could_not_write_msg_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_write_msg_file()
    {
      cerr << "Error_could_not_write_msg_file" << endl;
    }
#endif
};

class Error_could_not_read_msg_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_read_msg_file()
    {
      cerr << "Error_could_not_read_msg_file" << endl;
    }
#endif
};



/* 
   Zur internen Verwaltung wird fuer jede Nachricht ein Nachrichtendescriptor 
   benutzt. Er dient zur Verwaltung der Nachricht in Queues etc. Neben der
   Nachricht selber wird fuer den Fall, dass sich diese Nachricht in der 
   Sendequeue befindet, die Zeit der letzten Aussendung und die Anzahl der 
   Sendeversuche gespeichert.
   Die Nachricht selber wird nicht im Speicher, sondern im Filesystem gehalten,
   um im Falle eines Programmabsturzes die Information nicht zu verlieren.
*/
class nachrichten_descriptor
{
 protected:
  n_types typ;
  Mid m;
  zeit t_create;
  zeit t_last;
  unsigned int n_t;
  String dateiname;
  //list<callsign> nachbarn;

 protected:
  bool get_file( n_types , String&, String );

 public:
  nachrichten_descriptor();
  nachrichten_descriptor(const nachricht&, String );
  nachrichten_descriptor(String );

  nachricht& get_nachricht();
  void loesche_nachricht();
  void set_sendemarkierung();
  bool expired( int = -1 );
  inline zeit get_letzte_sendung()
    {
      return t_last;
    }
  inline unsigned int get_versuche()
    {
      return n_t;
    }
  inline Mid get_mid()
    {
      return m;
    }
  inline n_types get_typ( void )
    {
      return typ;
    }
};


class routing_descriptor
{
 protected:
  n_types typ;
  String dateiname;
  String pfadname;
  callsign recived_from;
  list<callsign> nachbarn;
  zeit t_create;

 protected:
  void init( void );
  bool get_file(n_types , String& );

 public:
  routing_descriptor();
  routing_descriptor(nachricht&, const callsign& );
  routing_descriptor(const String& );
  
  void add_nachbar( const callsign& );
  bool check_nachbar( const callsign& );
  void delete_nachbar( const callsign& );

  bool is_empty( void );
  void delete_nachricht( void );
  nachricht& get_nachricht();

  bool expired( int = -1 );
};


#endif
