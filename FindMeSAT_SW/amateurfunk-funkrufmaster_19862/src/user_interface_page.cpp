/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "user_interface.h"

#include "autoconf.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "database.h"

extern callsign_database calls;
extern callsign G_mycall;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif
/*****************************************************************************/
/* Allgemeine Funktion zur aussendung eines Funkrufs                         */
/*                                                                           */
void user_interface::do_page(callsign &call, ostream &ostr, destin &ds, String &mesg, bool activierung )
{
  try
    {
      // Adresse zum Rufzeichen suchen
      adress adr = calls.find(call).get_adr();
      if(!adr.gueltig()) //User mit ID 0.0 nicht anpagen
      {
        ostr << mldg.find(212) << ' ' << call << ' ';
        ostr << mldg.find(221) << cr << flush;
	return;
      }

      if (mesg!="")
      {
      // Ueberpruefen, ob der Server das angegebene Zielgebiet versorgt
#ifdef COMPILE_SLAVES
      try
	{
#endif
	  ostr << mldg.find(600) << ' ' << call;
	  if (calls.find(call).get_name().slen() > 0)
	    ostr << " (" << calls.find(call).get_name() << ")";
	  bool flag1 = false;
	  unsigned int prio;
	  if (activierung)
	    prio = 32;
	  else
	    prio = 64;
#ifdef COMPILE_SLAVES
	  flag1 = spool.spool_msg(activierung,usr.user_call(),G_mycall,zeit(),adr,mesg,false,ds,prio);
	  in_msg++;
#endif
	  bool flag2 = fwd.fwd_msg(usr.user_call(),adr,ds,prio,mesg);
	  if ( flag1 || flag2 )
	    ostr << ' ' << mldg.find(601) << cr;
	  else
	    ostr << ' ' << mldg.find(602) << cr << flush;
#ifdef COMPILE_SLAVES
	}
      // Exceptions abfangen
      catch ( Error_could_not_open_file )
	{
	  logf.eintrag(usr.user_call(),"Nicht moeglich Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
	}
#endif
      }
      else
      {
          ostr << mldg.find(603) << cr << flush;
      }
    }
  catch ( Error_callsign_does_not_exist )
    {
      ostr << mldg.find(212) << ' ' << call << ' ';
      ostr << mldg.find(221) << cr << flush;
    } 
}

void user_interface::page_group(istream &cmd, ostream &ostr)
{
  char ch;
  String msg;

  // Leerzeichen abtrennen
  cut_blanks(cmd);
  // Zielgruppe aus Eingabe holen
  char puffer[1000];
  if (cmd.get(puffer,999,' '))
    {
      page_gname=String(puffer);
      page_gname.upcase();
      try
	{
	  page_grp = grps.find(page_gname);

	  try
	    {
	      cmd >> page_ds;
	    }
	  catch( Error_no_destin )
	    {
	      page_ds = get_default_destin();
	    }
	  cmd >> ch;
	  if (ch != ' ') cmd.putback(ch);
	  
	  // Nachricht holen
	  cmd >> msg;

	  if(msg!="")
	  {
	  try
	    {
	      callsign call = page_grp.get_first();	  
	      do_page(call,ostr, page_ds, msg, false );
	      while (true)
		{
		  call = page_grp.get_next();
		  do_page(call,ostr, page_ds, msg, false );
		}
	    }
	  catch( Error_no_more_call_in_group )
	    {
	    }
	  }
          else
          {
    	      ostr << mldg.find(605) << " " << page_gname << " "  << mldg.find(606) << cr;
	      state=wait_gpagetext;
          }
	}
      catch( Error_group_does_not_exist )
	{
	  ostr << mldg.find(400) << ' ' << page_gname << ' ';
	  ostr << mldg.find(406) << cr << flush;
	}
      catch( Error_could_not_open_groupfile )
	{
	  ostr << mldg.find(403) << cr;
	}
    }
else
  ostr << mldg.find(404) << cr << flush;
}


void user_interface::page_grouptext(istream &cmd, ostream &ostr)
{
    String msg;
    cmd >> msg;

    if(msg!="")
    {
	try
	{
    	    callsign call = page_grp.get_first();	  
    	    do_page(call,ostr, page_ds, msg, false );
    	    while (true)
	    {
		call = page_grp.get_next();
		do_page(call,ostr, page_ds, msg, false );
	    }
	}
	catch( Error_no_more_call_in_group )
	{
	}
	state=wait_cmd;
    }
    else
    {
        ostr << mldg.find(603) << cr << flush;
        state=wait_cmd;
    }
}

/*****************************************************************************/
/* Absenden eines Funkrufs                                                   */
/*                                                                           */
void user_interface::page(istream &cmd, ostream &ostr )
{
  char ch;
  String msg;

  // Leerzeichen abtrennen
  cut_blanks(cmd);
  // Zielrufzeichen aus Eingabe holen
  try
    {
      cmd >> page_call;
      
      // Zielgebiet aus Eingabe holen
      // Wenn kein gueltiges Ziel, beginnend mit dem at-sign folgt,
      // erzeugt der >>-Operator automatisch ein @*-Ziel  
      try
	{
	  cmd >> page_ds;
	}
      catch( Error_no_destin )
	{
	  page_ds = get_default_destin();
	}
      cmd >> ch;
      if (ch != ' ') cmd.putback(ch);
      
      // Nachricht holen
      cmd >> msg;

      if (msg.slen()>0)
      {      
#ifdef _DEBUG_ELOGS_
        cerr << "Rufzeichen  : " << page_call << endl;
        cerr << "Destination : " << page_ds << endl;
        cerr << "Nachricht   : " << msg << endl;
#endif
        do_page(page_call,ostr, page_ds, msg, false );
      }
      else
      {
        ostr << mldg.find(605) << " " << page_call << " " << mldg.find(606) << cr;
	state=wait_pagetext;
      }
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(215) << cr << flush;
    }
}

void user_interface::page_text(istream &cmd, ostream &ostr )
{
      String msg;
      cmd >> msg;
      if (msg.slen()>0)
      {      
#ifdef _DEBUG_ELOGS_
        cerr << "Rufzeichen  : " << page_call << endl;
        cerr << "Destination : " << page_ds << endl;
        cerr << "Nachricht   : " << msg << endl;
#endif
        do_page(page_call,ostr, page_ds, msg, false );
	state=wait_cmd;
      }
      else
      {
        ostr << mldg.find(603) << cr << flush;
	state=wait_cmd;
      }
}


void user_interface::activate(istream &cmd, ostream &ostr )
{
  String msg;
  
  // Leerzeichen abtrennen
  cut_blanks(cmd);
  // Zielrufzeichen aus Eingabe holen
  callsign call;
  destin ds;
  try
    {
      cmd >> call;
      
      // Zielgebiet aus Eingabe holen
      // Wenn kein gueltiges Ziel, beginnend mit dem at-sign folgt,
      // erzeugt der >>-Operator automatisch ein @*-Ziel
      
    }
  catch( Error_no_callsign )
    {
      call = usr.user_call();
    }
  if (samecall(call,usr.user_call()) || usr.is_sysop())
    {
      msg = "Ihr Funkrufempfaenger wurde nun aktiviert.";
      
      do_page(call,ostr, ds, msg, true );
      ostr << mldg.find(604) << cr << flush;
    }
  else
    { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Aktivierungsversuch ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    } 
}
