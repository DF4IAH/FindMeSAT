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

#include "talk.h"

talk_eintrag::talk_eintrag( const callsign &frm, const String &txt )
{
  from = frm;
  system = false;
  text = txt;
}

talk_eintrag::talk_eintrag( const String &txt )
{
  system = true;
  text = txt;
}

String talk_eintrag::get( void )
{
  String tmp;

  if (system)
    tmp = "SYSTEM:" + text;
  else
    tmp = from.str() + ':' + text;
  return tmp;
}

talk_member::talk_member( const callsign &c )
{
  call = c;
  texte.clear();
}

void talk_member::add_msg( const callsign &frm, const String &txt )
{
  talk_eintrag eintr(frm,txt);
  texte.push_back(eintr);
}

void talk_member::add_msg( const String &txt )
{
  talk_eintrag eintr(txt);
  texte.push_back(eintr);
}

bool talk_member::msg_avalable( void )
{
  list<talk_eintrag>::iterator it = texte.begin();
  return (it != texte.end());
}

String talk_member::get_msg( void )
{
  list<talk_eintrag>::iterator it = texte.begin();
  if (it != texte.end())
    {
      String tmp = it->get();
      texte.erase(it);
      return tmp;
    }

  return String("");
}


talker::talker( void )
{
  members.clear();
}

void talker::add_member( const callsign &call )
{
  talk_member mem(call);
  members.push_back(mem);
}

void talker::del_member( const callsign &c )
{
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    {
      if (it->call == c)
	{
	  members.erase(it);
	  return;
	}
    }
}

bool talker::send_msg( const callsign &frm, const callsign &to, const String &msg )
{
  bool flag = false;
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    {
      if (samecall(it->call,to))
	{
	  it->add_msg(frm,msg);
	  flag = true;
	}
    }
  return flag;
}

void talker::send_msg( const callsign &frm, const String &msg )
{
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    it->add_msg(frm,msg);
}

void talker::send_msg( const String &msg )
{
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    it->add_msg(msg);
}

bool talker::msg_avalable( const callsign &c )
{
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    if (it->call == c)
      return it->msg_avalable();

  return false;
}

String talker::get_msg( const callsign &c )
{
  for (list<talk_member>::iterator it = members.begin(); it != members.end(); ++it )
    if (it->call == c)
      return it->get_msg();

  return String("");
}

