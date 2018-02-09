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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/


#include "passwd.h"

#include <crypt.h>

#include "config.h"
#include "database.h"

extern config_file configuration;
extern callsign_database calls;
extern callsign G_mycall;

consolpw::consolpw( bool ax )
{
  String flagstring;

  try
    {
      flagstring = configuration.find("PASSWORTFLAGS");
    }
  catch( Error_parameter_not_defined )
    {
      flagstring = String("0");
    }
  password_flags = (unsigned int) flagstring.Stoi();
  ax25_flag = ax;
}


bool consolpw::check_password( const callsign &call, const String &pw )
{
  if (!use_pw())
    return true;

  else
    {
      try
	{
	  database_entry eintrag = calls.find(call);
	  if (ax25_flag)
	    return pw == eintrag.get_name();
	  else
	    {
	      String cpw = eintrag.get_consol_pw();
	      if (cpw == "*" || cpw == "")

		if ( (password_flags & 2) != 0 )
		  return pw == eintrag.get_name();
		else
		  return false;

	      else
		{
		  char *cryptpw = crypt(strtochar(pw),"HF");
		  return String(cryptpw) == cpw;
		}
	    }
	}
      catch( Error_callsign_does_not_exist )
	{
	  return false;
	}
    }
}

bool consolpw::change_password( const callsign &call, const String& oldpw, const String &pw )
{
  if (!use_pw() || !user_change() || ax25_flag)
    return false;

  else
    {
      try
	{
	  database_entry eintrag = calls.find(call);
	  String cpw = eintrag.get_consol_pw();
	  bool flag = false;
	  if (cpw == "*" || cpw == "")
	    flag = oldpw == eintrag.get_name();
	  else
	    {
	      char *cryptpw = crypt(strtochar(oldpw),"HF");
	      flag = String(cryptpw) == cpw;
	    }
	  if (flag)
	    {
	      char *cryptpw =crypt(strtochar(pw),"HF");
	      eintrag.set_consol_pw(String(cryptpw),G_mycall);
	      calls.change(call,eintrag);
	      return true;
	    }
	  else
	    return false;
	}
      catch( Error_callsign_does_not_exist )
	{
	  return false;
	}
    }
}

bool consolpw::set_password( const callsign &call, const String &pw )
{
  if (!use_pw() /* || ax25_flag   Sysops sollen auch ueber AX25-Verbindungen
		   das Passwort setzen koennen. */)
    return false;

  else
    {
      try
	{
	  database_entry eintrag = calls.find(call);
	  char *cryptpw =crypt(strtochar(pw),"HF");
	  eintrag.set_consol_pw(String(cryptpw),G_mycall);
	  calls.change(call,eintrag);
	  return true;
	}
      catch( Error_callsign_does_not_exist )
	{
	  return false;
	}
    }
}
