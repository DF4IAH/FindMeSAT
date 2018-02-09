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

#include "database.h"

extern callsign_database calls;

extern callsign G_mycall;

void user_interface::set_name( istream &cmd, ostream &ostr )
{
  
  String name;
  char puffer[200];

  // Alle Leerzeichen ueberspringen
  cut_blanks(cmd);
  cmd.get(puffer,199,' ');
  callsign call;
  try
    {
      call = callsign(String(puffer));
      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      name = String(puffer);
    }
  catch(Error_no_callsign)
    {
      name = String(puffer);
      call = usr.user_call().get_nossidcall();
    }
  if (samecall(call,usr.user_call()) || usr.is_sysop())
    {
      try
	{
	  database_entry entry = calls.find(call);
	  if (name.in(',') || name.in(':'))  // Da will doch tatsaechlich einer
    	  {                                  // ein Komma oder ein Doppelpunkt
	    ostr << mldg.find(243) << cr;    // in seinen Namen setzen.  
	    return;                          // Das bringt aber unsere Database 
	  }                                  // oder den FWD durcheinander ;=((
	  entry.set_name(name,G_mycall);
	  calls.change(call,entry);
	  ostr << mldg.find(200) << ' ' << name << ' ' << mldg.find(201) << cr;
	  fwd.fwd_usr('C',call);
	}
      catch( Error_callsign_does_not_exist )
	{
	  ostr << mldg.find(202) << cr << flush;
	  ostr << mldg.find(203) << cr << flush;
	  ostr << mldg.find(204) << cr << flush;
	}
    }
  else 
    { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

void user_interface::set_loc( istream &cmd, ostream &ostr )
{
  //struct user_change chng;

  //chng.typ = ct_change;

  String Loc;
  callsign call;
  char puffer[200];

  // Alle Leerzeichen ueberspringen
  cut_blanks(cmd);
  cmd.get(puffer,199,' ');
  try
    {
      call = callsign(String(puffer));
      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      Loc = String(puffer);
    }
  catch(Error_no_callsign)
    {
      Loc = String(puffer);
      call = usr.user_call().get_nossidcall();
    }
  if (samecall(call,usr.user_call()) || usr.is_sysop())
    {
      try
	{
	  locator loc(Loc);
	  try
	    {
	      database_entry entry = calls.find(call);
	      entry.set_loc(loc,G_mycall);
	      calls.change(call,entry);
	      ostr << mldg.find(206) << ' ' << loc << ' ' << mldg.find(201) << cr;
	      fwd.fwd_usr('C',call);
	    }
	  catch( Error_callsign_does_not_exist )
	    {
	      ostr << mldg.find(202) << cr << flush;
	      ostr << mldg.find(207) << cr << flush;
	      ostr << mldg.find(204) << cr << flush;
	    }
	}
      catch( Error_not_a_locator )
	{
	  ostr << mldg.find(208) << cr << flush;
	}
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

void user_interface::set_typ( istream &cmd, ostream &ostr )
{
  String Typ;
  callsign call;
  char puffer[200];

  // Alle Leerzeichen ueberspringen
  cut_blanks(cmd);
  cmd.get(puffer,199,' ');
  try
    {
      call = callsign(String(puffer));
      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      Typ = String(puffer);
    }
  catch(Error_no_callsign)
    {
      Typ = String(puffer);
      call = usr.user_call().get_nossidcall();
    }
  if (samecall(call,usr.user_call()) || usr.is_sysop())
    {
//      try
	{
	  pager_typ tp(Typ);
	  try
	    {
	      database_entry entry = calls.find(call);
    	      adress adr = calls.find(call).get_adr();
              if(!adr.gueltig()) //User mit ID 0.0 kann keinen Pager-Typ setzen!
              {
    		ostr << mldg.find(242) << cr;
    		ostr << mldg.find(204) << cr << flush;
		return;
    	      }
	      entry.set_geraet(tp,G_mycall);
	      calls.change(call,entry);
	      ostr << mldg.find(209) << ' ' << tp << ' ' << mldg.find(201) << cr;
	      fwd.fwd_usr('C',call);
	    }
	  catch( Error_callsign_does_not_exist )
	    {
	      ostr << mldg.find(202) << cr << flush;
	      ostr << mldg.find(210) << cr << flush;
	      ostr << mldg.find(204) << cr << flush;
	    }
	}
/*
????? was soll das hier ????
      catch( Error_not_a_locator )
	{
	  ostr << mldg.find(211) << cr << flush;
	}
      catch ( Error_Angle_Out_Of_Range )
	{
	  logf.eintrag(usr.user_call(),"Locator-Fehler: Winkel ist nicht im korrekten Bereich",LOGMASK_PRGRMERR);
	}
*/	
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

void user_interface::set_lang( istream &cmd, ostream &ostr )
{
  String name;
  char puffer[200];

  // Alle Leerzeichen ueberspringen
  cut_blanks(cmd);
  cmd.get(puffer,199,' ');
  callsign call;
  try
    {
      call = callsign(String(puffer));
      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      name = String(puffer);
    }
  catch(Error_no_callsign)
    {
      name = String(puffer);
      call = usr.user_call().get_nossidcall();
    }
  if (samecall(call,usr.user_call()) || usr.is_sysop())
    {
      try
	{
	  mldg.load(name); // Sprachfile ueberhaupt vorhanden?
	  database_entry entry = calls.find(call);
	  entry.set_language(name,G_mycall);
	  calls.change(call,entry);
	  ostr << mldg.find(233) << ' ' << name << ' ' << mldg.find(201) << cr;
	  ostr << mldg.find(234) << cr;
	  fwd.fwd_usr('C',call);
	}
      catch( Error_callsign_does_not_exist )
	{
	  ostr << mldg.find(202) << cr << flush;
	  ostr << mldg.find(203) << cr << flush;
	  ostr << mldg.find(204) << cr << flush;
	}
      catch( Error_could_not_open_messagefile )
	{
	  ostr << mldg.find(239) << ' ' << name << ' ' << mldg.find(240) << cr;
	}
    }
  else 
    { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 

}


/*****************************************************************************/
/* Benutzer in Rufzeichendatenbank eintragen                                 */
/*                                                                           */
void user_interface::add_user(istream &cmd, ostream &ostr )
{
  callsign call;
  
  // Uberpruefen ob Benutzer Sysop ist
  if (usr.is_sysop())
    {
      // Leerzeichen abtrennen
      cut_blanks(cmd);	  
      // Rufzeichen und Adresse holen
      adress adr;
      pager_typ ger;
      try
	{
	  cmd >> call;
	  cmd >> adr >> ger;
	  database_entry daten;
	  try
	  {
	    daten = calls.find_adress(adr);
	    ostr << mldg.find(238) << ' ' << adr << ' ' << mldg.find(214) << ": ";
	    ostr << daten.get_call() << cr << flush;
	    return;
	  }
	  catch( Error_adress_does_not_exist)
	  {
          database_entry entry = database_entry(call,adr,ger,G_mycall);
	  try
	    {
	      // rufzeichen in Datenbank eintragen
	      calls.add(entry);
	      ostr << mldg.find(212) << ' ' << call << ' ' << mldg.find(201) << cr << flush;
	      fwd.fwd_usr('A',call);
	      logf.eintrag(usr.user_call(),"Rufzeichen "+call.str()+" in Datenbank eingetragen",LOGMASK_DBAENDRNG);
	      destin ds;
	      String msg = mldg.find(213);
	      do_page(call,ostr, ds, msg, true );
	    }
	  catch (Error_callsign_already_exists )
	    {
	      database_entry entry2 = calls.find(call);
	      if (!entry2.get_adr().gueltig())
		{
		  entry2.set_adr(adr,G_mycall);
		  entry2.set_geraet(ger,G_mycall);
		  calls.change(call,entry2);
		  ostr << mldg.find(212) << ' ' << call << ' ' << mldg.find(201) << cr << flush;
		  fwd.fwd_usr('C',call);
		  logf.eintrag(usr.user_call(),"Rufzeichen "+call.str()+" in Datenbank eingetragen",LOGMASK_DBAENDRNG);
		  destin ds;
		  String msg = mldg.find(213);
		  do_page(call,ostr, ds, msg, true );
		}
	      else
		ostr << mldg.find(212) << ' ' << call << ' ' << mldg.find(214) << cr << flush;
	    }
	}
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr << flush;
	}
      catch( Error_no_adress )
	{
	  ostr << mldg.find(216) << cr << flush;
	}
    } 
  else 
    { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}
  


 
/*****************************************************************************/
/* Benutzer aus Rufzeichendatenbank entfernen                                */
/*                                                                           */
void user_interface::del_user(istream &cmd, ostream &ostr)
{

  // struct user_change chng;
  callsign call;

  // Zunaechst ueberpruefen, ob der Benutzer des Programms Sysop ist
  if (usr.is_sysop())
    {
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);
      try
	{
	  // rufzeichen aus Eingabe holen
	  cmd >> call;
	  adress adr;
	  cmd >> adr;
	  try
	    {
	      database_entry call_entry = calls.find(call);
	      if ( call_entry.get_adr() == adr)
		{
		  // Rufzeichen aus Datenbank loeschen
		  calls.del(call);
		  fwd.fwd_usr('R',call);
		  ostr << mldg.find(212) << ' ' << call << ' ' << mldg.find(217) << cr << flush;
		  logf.eintrag(usr.user_call(),"Rufzeichen "+call.str()+" aus Datenbank geloescht",LOGMASK_DBAENDRNG);
		}
	      else
		{
		  ostr << mldg.find(218) << cr;
		  ostr << mldg.find(219) << cr;
		  ostr << mldg.find(220) << cr;
		}
	    }
	  catch (Error_callsign_does_not_exist )
	    {
	      ostr << mldg.find(212) << ' ' << call << ' ' << mldg.find(221) << cr << flush;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr << flush;
	}
      catch( Error_no_adress )
	{
	  ostr << mldg.find(216) << cr;
	}
    } 
  else 
    { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}  

/*****************************************************************************/
/* Benutzer hat ein neues Rufzeichen                                         */
/*                                                                           */
void user_interface::newcall(istream &cmd, ostream &ostr )
{
  callsign old_call;
  callsign new_call;
  String Loc;
  String name;
  adress adr;
  pager_typ ger;

  if (usr.is_sysop())      // Ueberpruefen ob Benutzer Sysop ist
  {
    try
    {
      cut_blanks(cmd);	  
      cmd >> old_call;
      try
      { 
        cmd >> new_call;

        database_entry old_daten = calls.find(old_call);
        adr = (adress) old_daten.get_adr();
        ger = (pager_typ) old_daten.get_geraet();
  
        database_entry entry = database_entry(new_call,adr,ger,G_mycall);
      
        // Rufzeichen in Datenbank eintragen
	try
	{
    	    calls.add(entry);
	}
        catch (Error_callsign_already_exists )
        {
	    database_entry entry2 = calls.find(new_call);
	    if (!entry2.get_adr().gueltig())	// Eintrag mit "0.0"
	    {
    		try
		{
    		    calls.add(entry);
		}
    		catch (Error_callsign_already_exists )
		{}
	    }
	    else	// Wir erzwingen einen Fehler. Wird (hoffentlich)
	    {	        // durch ein "catch" richtig abgefangen.
    		calls.add(entry);
            }
	}
 	 
        // Locator setzen
        locator loc(old_daten.get_locator());
        entry.set_loc(loc,G_mycall);
        calls.change(new_call,entry);
 	 
        // Name setzen
        name = old_daten.get_name();
        entry.set_name(name,G_mycall);
        calls.change(new_call,entry);
        logf.eintrag(usr.user_call(),"Rufzeichen "+ new_call.str() +" in Datenbank eingetragen",LOGMASK_DBAENDRNG);
 	 
        // altes Rufzeichen aus Datenbank loeschen
        calls.del(old_call);
        ostr << old_call << ' ' << mldg.find(222) << ' ' << new_call << "." << cr << flush;
        logf.eintrag(usr.user_call(),"Rufzeichen "+ old_call.str() +" aus Datenbank ausgetragen",LOGMASK_DBAENDRNG);
	fwd.fwd_usr('R',old_call);
	fwd.fwd_usr('A',new_call);
      }

      // Exceptions abfangen
      catch (Error_callsign_already_exists )
      {
	    ostr << mldg.find(212) << ' ' << new_call << ' ' << mldg.find(214) << cr << flush;
      }
      catch( Error_no_callsign )
      {
         ostr << mldg.find(215) << cr << flush;
      }
    }
    
    catch( Error_callsign_does_not_exist )
    {
      ostr << mldg.find(212) << ' ' << old_call << ' ' << mldg.find(221) << cr;
    }
    catch( Error_no_callsign )
    {
       ostr << mldg.find(215) << cr << flush;
    }

  }
  else 
  { 
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
  } 
}

void user_interface::show_user(const database_entry &dat, ostream &ostr )
{
  callsign call = dat.get_call();
  call.set_format(true);
  call.set_nossid(false);
  ostr << setw(6) << call << ' ';
  adress adr = dat.get_adr();
  pager_typ geraet = dat.get_geraet();
  ostr << setw(7) << adr << ' ' << setw(15) << geraet << ' ';
  locator loc = dat.get_locator();
  ostr << setw(6) << loc << ' ';
  ostr.setf(ios::left, ios::adjustfield );
  String name = dat.get_name();
  ostr << setw(20) << name << ' ';
  ostr.setf(ios::right, ios::adjustfield );
  zeit last_change = dat.get_last_change();
  last_change.set_darstellung(zeit::f_datum_s);
  call = dat.get_server();  
  call.set_format(true);
  call.set_nossid(true);
  ostr << last_change << ' ' << call;
  ostr << cr;
}

void user_interface::show_user(istream &cmd, ostream &ostr )
{
  bool all_flag = false;

  callsign call;
  String eingabe;
  String eingabe2;
  cut_blanks(cmd);

  eingabe.get(cmd,200,' ');
  cut_blanks(cmd);
  eingabe2.get(cmd,200,' ');
  if(eingabe2==String("+"))
    all_flag = true;
  try
    {
      call = callsign(eingabe);
      database_entry daten = calls.find(call);
      zeit tm = daten.get_last_change();
      tm.set_darstellung(zeit::f_zeitdatum_l);
      ostr << mldg.find(223) << ' ' << call << ':' << cr;
      ostr << cr;
      ostr << mldg.find(224) << ' ' << daten.get_adr() << cr;
      ostr << mldg.find(225) << ' ' << daten.get_name() << cr;
      ostr << mldg.find(226) << ' ' << daten.get_locator() << cr;
      ostr << mldg.find(227) << ' ' << daten.get_geraet() << cr;
      ostr << mldg.find(228) << ' ' << tm << cr;
      ostr << mldg.find(229) << ' ' << daten.get_server() << cr;
      ostr << mldg.find(235) << ' ' << daten.get_language() << cr;
      ostr << mldg.find(236) << ' ';
      tm = daten.get_last_login();
      if (tm != zeit(-1))
	{
	  tm.set_darstellung(zeit::f_zeitdatum_l);
	  ostr << tm << cr;
	}
      else
        {
	    ostr << mldg.find(237) << cr;
	}
      ostr << cr << flush;
      vector<database_entry> daten2 = calls.get_other_pager(call);
      for ( vector<database_entry>::iterator it = daten2.begin(); it != daten2.end(); ++it )
	show_user(*it,ostr);
    }
  catch( Error_callsign_does_not_exist )
    {
      vector<database_entry> daten3 = calls.get_user(eingabe,all_flag);
      ostr << mldg.find(244) << cr;
      ostr << mldg.find(245) << cr;
      for ( vector<database_entry>::iterator it = daten3.begin(); it != daten3.end(); ++it )
	show_user(*it,ostr);
      ostr << daten3.size() << ' ' << mldg.find(246);
      ostr << ' ' << calls.get_size() << ' ' << mldg.find(247) << cr;
    }
  catch( Error_no_callsign )
    {
      if (eingabe == String(""))
	ostr << mldg.find(230) << ' ' << calls.get_size() << ' ' << mldg.find(231) << cr;     
      else
	{
	  if (eingabe == String("+"))
	    all_flag = true;

	  String suche;
	  if (eingabe == String("*") || eingabe == String("+"))
	    suche = "";
	  else
	    suche = eingabe;

	  vector<database_entry> daten4 = calls.get_user(suche,all_flag);
	  ostr << mldg.find(244) << cr;
	  ostr << mldg.find(245) << cr;
	  for ( vector<database_entry>::iterator it = daten4.begin(); it != daten4.end(); ++it )
	    show_user(*it,ostr);
	  ostr << daten4.size() << ' ' << mldg.find(246);
	  ostr << ' ' << calls.get_size() << ' ' << mldg.find(247) << cr;
	}
    }
}


/*****************************************************************************/
/* Datenbank sichern                                                         */
/*                                                                           */
void user_interface::dbsave(  ostream &ostr )
{
  if (usr.is_sysop())
    {
      calls.save();
      ostr << mldg.find(232) << cr;
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Backupversuch der Rufzeichendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

