/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Holger Flemming                                    *
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

#ifndef __PASSWD_H__
#define __PASSWD_H__

#include "String.h"
#include "callsign.h"

class consolpw
{
 protected:

  unsigned int password_flags;
  bool ax25_flag;

  /* Die Passwort-Flags und ihre Bedeutung.
     .
     1 : Konsolenpasswort wird benutzt
     2 : Name als Initialpasswort
     4 : benutzer kann Passwort aendern.

  */

 public:
  consolpw( bool );
  inline bool use_pw( void )
    {
      return (password_flags & 1) != 0;
    }
  inline bool user_change( void )
    {
      return (password_flags & 4) != 0;
    }
  inline void set_ax25_flag( void )
    {
      ax25_flag = true;
    }
  inline bool ax25_connection( void )
    {
      return ax25_flag;
    }
  bool check_ax25password( const callsign &, char *, const String & );
  bool check_password( const callsign&, const String& );
  bool change_password( const callsign&, const String&, const String& );
  bool set_password( const callsign&, const String& );
};


#endif // __PASSWD_H__
