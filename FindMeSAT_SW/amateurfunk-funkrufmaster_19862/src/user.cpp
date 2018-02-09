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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "user.h"
#include "logfile.h"
#include "database.h"

extern callsign_database calls;
extern callsign G_mycall;

zeit user::http_pw_time;
uint32_t user::http_pw_sock_adr;
callsign user::http_pw_call;
String user::http_pw_correct;
bool user::http_pw_auth1_flag;
bool user::http_pw_sysop_flag;

// Konstruktor, ruft den Contruktor der Klasse callsign auf.

user::user( void )
{
  call = callsign();
  http_flag = false;
  sysop = false;
  pw_gesetzt = false;
  sockadr = 0;

  name = "";
  language = "dl";
  last_login = zeit(-1);
}

user::user( const callsign &c, bool sy ) : call(c)
{
  // Nobody is sysop from the beginning...
  // Naja, wenn das Programm aus einer Mailbox heraus aufgerufen wird, dann
  // manchmal eben doch.
  // Darher ist jetzt im Konstruktor auch das Sysop-Flag als Parameter eingebaut
  sysop = sy;
  pw_gesetzt = false;
  http_flag = false;
  try
    {
      database_entry ent = calls.find(call.get_nossidcall());
      name = ent.get_name();
      language = ent.get_language();
      loc = ent.get_locator();
      last_login = ent.get_last_login();
      ent.login();
      calls.change(ent.get_call(),ent);
    }
  catch( Error_callsign_does_not_exist )
    {
      database_entry ent(call.get_nossidcall(),adress(),pager_typ(),G_mycall);
      try
	{
	  calls.add(ent);
	  name = "";
	  language = "dl";
	  last_login = zeit(-1);
	}
      catch( Error_callsign_already_exists )
	{
	  name = "";
	  language = "dl";
	  last_login = zeit(-1);
	}
    }
}

user::user( const callsign &c, uint32_t sa ) : call(c)
{
  if ( http_pw_sysop_flag && 
       http_pw_call == call && 
       http_pw_sock_adr == sa  )

    if ( zeit() - http_pw_time < 900 )
      sysop = true;
    else
      {
	sysop = false;
	http_pw_sysop_flag = false;
      }

  else
    sysop = false;

  pw_gesetzt = false;
  http_flag = true;
  sockadr = sa;
  try
    {
      database_entry ent = calls.find(call.get_nossidcall());
      name = ent.get_name();
      language = ent.get_language();
      loc = ent.get_locator();
      last_login = ent.get_last_login();
      ent.login();
      calls.change(ent.get_call(),ent);
    }
  catch( Error_callsign_does_not_exist )
    {
      database_entry ent(call.get_nossidcall(),adress(),pager_typ(),G_mycall);
      try
	{
	  calls.add(ent);
	  name = "";
	  language = "dl";
	  last_login = zeit(-1);
	}
      catch( Error_callsign_already_exists )
	{
	  name = "";
	  language = "dl";
	  last_login = zeit(-1);
	}
    }
}

bool user::auth1( config_file &cfg , String& out)
{
  // Hier wird eine einfache funktion zur Authentifizierung als 
  // Sysop eingebaut. Es wird dabei das Baycom-Verfahren benutzt.

  if (http_flag)
    {
      if (http_pw_auth1_flag)
	if (zeit() - http_pw_time < 120)
	  return false;
	else
	  http_pw_auth1_flag = false;
      
      if (http_pw_sysop_flag)
	if (zeit() - http_pw_time < 900)
	  return false;
	else
	  http_pw_sysop_flag = false;
    }
  
  ifstream passwd_file;
  
  try
    {
      String passwd_name;
      // Holen des Dateinamens des Passwordstrings aus dem 
      // Konfiguratioansfile und anschliessendes Oeffnen des 
      // Files
      passwd_name = cfg.find("PASSWORD");
      passwd_file.open(strtochar(passwd_name));
      
      if (passwd_file)
	{
	  // Passwordstring nach passwd...
	  char passwd[100];
	  passwd_file >> passwd;
	  int pos[5];
	  
	  // Nun werden fuenf Zufallszahlen zwischen 0 und 79 erzeugt und
	  // Ausgegeben
	  try
	    {
	      out.append(cfg.find("MYCALL")+String("> "));
	    }
	  catch( Error_parameter_not_defined )
	    {
	      syslog logf(cfg);
	      logf.eintrag("Parameter MYCALL nicht definiert.",LOGMASK_PRGRMERR);
	      out.append("PW ");
	    }
	  int l = strlen(passwd);
	  if (l < 10)
	    {
	      syslog logf(cfg);
	      logf.eintrag("Passwortstring zu kurz!.",LOGMASK_PRGRMERR);
	      out.append("Passwort-String zu kurz, Sysop-Login nicht moeglich.");
	      return false;
	    }
	  for (int i = 0;i < 5;i++)
	    {
	      bool flag;
	      do
		{
		  flag = false;
		  pos[i] = rand() % (l);
		  for (int j = 0;j < i; j++)
		    if (pos[j] == pos[i])
		      flag = true;
		}
	      while (flag);
	      // Bei der Ausgabe wird eine eins addiert, um Zahlen zwischen 1 und 80 zu erhalten
	      out.append(String(" ")+itoS(pos[i]+1));
	    }
	  // In correct werden die fuenf korrekten Zeichen
	  // geschrieben.
	  for (int i = 0;i < 5;i++)
	    correct[i] = passwd[pos[i]];
	  correct[5] = '\0';
	  pw_gesetzt = true;
	  
	  if (http_flag)
	    {
	      http_pw_time = zeit();
	      http_pw_sock_adr = sockadr;
	      http_pw_call = call;
	      http_pw_correct = String(correct);
	      http_pw_auth1_flag = true;
	    }
	  return true;
	}
      return false;
    }
  // Nun noch Abfangen einer moeglicherweise aufgetretenen Exception
  catch ( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter PASSWORD nicht gesetzt.",LOGMASK_PRGRMERR);
      return false;
    }
}

bool user::auth2( config_file &cfg , const String &in )
{	
  if (http_flag)
    if (http_pw_auth1_flag && zeit() - http_pw_time < 120 &&
	http_pw_call == call &&
	http_pw_sock_adr == sockadr )
      {
	if ( in.in(http_pw_correct) )
	  {
	    // Ja, dann ist der User fortan Sysop...
	    sysop = true;
	    
	    http_pw_sysop_flag = true;
	    http_pw_auth1_flag = false;
	    http_pw_time = zeit();
	    
	    // Und Eintrag ins Logfile
	    syslog logf(cfg);
	    logf.eintrag(call,"Passwortabfrage erfolgreich",LOGMASK_PWLOG);
	    return true;
	  }
	else
	  {
	    // Nein, Dann nur Eintrag ins Logfile
	    syslog logf(cfg);
	    logf.eintrag(call,"Passwortabfrage gescheitert",LOGMASK_PWLOG);
	    
	    http_pw_sysop_flag = false;
	    http_pw_auth1_flag = false;
	    return true;
	  }
      }
    else
      return false;
  else
    if (pw_gesetzt)
      {  
	// Nun wird ueberprueft, ob die korrekten Zeichen im
	// Antwortstring enthalten sind.
	if ( in.in(String(correct)) )
	  {
	    // Ja, dann ist der User fortan Sysop...
	    sysop = true;
	    // Und Eintrag ins Logfile
	    syslog logf(cfg);
	    logf.eintrag(call,"Passwortabfrage erfolgreich",LOGMASK_PWLOG);
	  }
	else
	  {
	    // Nein, Dann nur Eintrag ins Logfile
	    syslog logf(cfg);
	    logf.eintrag(call,"Passwortabfrage gescheitert",LOGMASK_PWLOG);
	  }
	pw_gesetzt = false;    
	return true;
      }
    else
      return false;
}


void user::logoff( void )
{
  if (sysop)
    {
      sysop = false;
      if (http_flag && http_pw_sysop_flag && 
	  http_pw_call == call && http_pw_sock_adr == sockadr)
	{
	  http_pw_sysop_flag = false;
	  http_pw_auth1_flag = false;
	}
    }
}
