/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000,2002 by Holger Flemming                               *
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

#ifndef __USER_H__
#define __USER_H__

#include <stdlib.h>
#include <iostream>
#include <string.h>

#include "globdef.h"
#include "callsign.h"
#include "config.h"
#include "String.h"
#include "logfile.h"

using namespace std;

// Die Klasse User verwaltet alle Daten, die ueber den momentanen 
// Benutzer des Programmes notwendig sind

class user
{
 private:
  callsign call;
  bool http_flag;
  bool pw_gesetzt;
  uint32_t sockadr;
  char correct[6];

  static zeit http_pw_time;
  static uint32_t http_pw_sock_adr;
  static callsign http_pw_call;
  static String http_pw_correct;
  static bool http_pw_auth1_flag;
  static bool http_pw_sysop_flag;

 public:
  bool sysop;
  String name;
  locator loc;
  String language;
  zeit last_login;

 public:
  user( void );
  user(const callsign& , bool = false );
  user( const callsign&, uint32_t );
  bool auth1( config_file & , String& );
  bool auth2( config_file & , const String&  );
  void logoff( void );
  inline bool is_sysop( void ) const
    {
      return sysop;
    }
  inline callsign user_call ( void )
    {
      return call;
    }
  inline bool is_pw_set( void )
    {
      return pw_gesetzt;
    }
};


#endif
