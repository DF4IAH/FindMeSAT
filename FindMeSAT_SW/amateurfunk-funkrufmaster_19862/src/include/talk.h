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

#ifndef __TALK_H__
#define __TALK_H__

#include <list>

#include "callsign.h"
#include "String.h"

using namespace std;

class talk_eintrag
{
 protected:
  callsign from;
  bool system;
  String text;

 public:
  talk_eintrag(const callsign&, const String& );
  talk_eintrag( const String & );
  String get( void );
};

class talk_member
{
 public:
  callsign call;
 protected:
  list<talk_eintrag> texte;

 public:
  talk_member( const callsign& );
  void add_msg( const callsign&, const String& );
  void add_msg( const String& );
  bool msg_avalable( void );
  String get_msg( void );
};

class talker
{
 protected:
  list<talk_member> members;

 public:
  talker( void );
  void add_member( const callsign &);
  void del_member( const callsign& );
  bool send_msg( const callsign&, const callsign&, const String & );
  void send_msg( const callsign&, const String & );
  void send_msg( const String & );
  bool msg_avalable( const callsign& );
  String get_msg( const callsign& );
};


#endif // __TALK_H__
