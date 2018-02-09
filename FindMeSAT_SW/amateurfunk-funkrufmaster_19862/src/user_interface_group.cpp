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

/*****************************************************************************/
/* Funkruf-Gruppe anlegen                                                    */
/*                                                                           */
void user_interface::add_group( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop() )
    {
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);
      char puffer[1000];
      if (cmd.get(puffer,999,' '))
	{
	  String gname(puffer);
	  gname.upcase();
	  cut_blanks(cmd);

	  String beschr;
	  cmd >> beschr;

	  try
	    {
	      grps.add_group(gname,beschr);
	      ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(401) << cr << flush;
	    }
	  catch( Error_group_already_exist )
	    {
	      ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(402) << cr << flush;
	    }
	  catch( Error_could_not_open_groupfile )
	    {
	      ostr << mldg.find(403) << cr;
	    }
	  catch( Error_no_group_name )
	    {
	      ostr << mldg.find(416) << cr;
	    }
	}
      else
	ostr << mldg.find(404) << cr ;
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Gruppendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

/*****************************************************************************/
/* Funkruf-Gruppe loeschen                                                   */
/*                                                                           */
void user_interface::del_group(istream &cmd, ostream &ostr)
{
  if (usr.is_sysop() )
    {
      char puffer[200];
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);

      cmd.get(puffer,199,' ');
      String gname(puffer);
      gname.upcase();
      try
	{
	  grps.del_group(gname);
	  ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(405) << cr << flush;
	}
      catch( Error_group_does_not_exist )
	{
	  ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(406) << cr << flush;
	}
      catch( Error_group_not_empty )
	{
	  ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(407) << cr;
	  ostr << mldg.find(408) << cr;
	}
      catch( Error_could_not_open_groupfile )
	{
	  ostr << mldg.find(403) << cr;
	}
      catch( Error_no_group_name )
	{
	  ostr << mldg.find(416) << cr;
	}
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Gruppendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

/*****************************************************************************/
/* Beschreibung einer Funkruf-Gruppe aendern                                 */
/*                                                                           */
void user_interface::groupname(istream &cmd, ostream &ostr)
{
  if (usr.is_sysop() )
  {
      char puffer[200];
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      String gname(puffer);
      gname.upcase();
      String beschr;
      cmd >> beschr;

      try
	{
	  gruppe grp = grps.find(gname);
	  grp.change_beschr(beschr);
	  ostr << mldg.find(409) << ' ' << gname << ' ' << mldg.find(410) << cr;
	}
      catch( Error_group_does_not_exist )
	{
	  ostr << mldg.find(400) << ' ' << gname << ' ' << mldg.find(406) << cr << flush;
	}
      catch( Error_could_not_open_groupfile )
	{
	  ostr << mldg.find(403) << cr;
	}
      catch( Error_no_group_name )
	{
	  ostr << mldg.find(416) << cr;
	}  
  }
  else
  {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Gruppendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
  } 
}	

/*****************************************************************************/
/* User zu einer Funkruf-Gruppe hinzufuegen                                  */
/*                                                                           */
void user_interface::add_user_to_group( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop() )
    {
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);

      char puffer[1000];
      if (cmd.get(puffer,999,' '))
	{
	  String gname(puffer);
	  gname.upcase();
	  cut_blanks(cmd);
          callsign call;
	  try
	    {
	      cmd >> call;
	      try
		{
		  calls.find(call).get_adr();  // Ist Call ueberhaupt in der Datenbank?
		  gruppe grp = grps.find(gname);
		  grp.find_call(call);	// Call schon in der Gruppe?
		  grp.add_call(call);
		  ostr << mldg.find(411) << ' ' << call << ' ';
		  ostr << mldg.find(412) << ' ' << gname << ' ';
		  ostr << mldg.find(413) << cr;
		}
	      catch( Error_group_does_not_exist )
		{
		  ostr << mldg.find(400) << ' ' << gname << ' ';
		  ostr << mldg.find(406) << cr;
		}
	      catch( Error_could_not_open_groupfile )
		{
		  ostr << mldg.find(403) << cr;
		}
	      catch( Error_Callsign_already_in_group )
		{
                  ostr << mldg.find(411) << ' ' << call << ' ';
		  ostr << mldg.find(414) << ' ' << gname << ' ';
		  ostr << mldg.find(415) << cr;
		}
	      catch( Error_no_group_name )
		{
		  ostr << mldg.find(404) << cr;
		}
	    }
	  catch( Error_callsign_does_not_exist )
	    {
              ostr << mldg.find(212) << ' ' << call << ' ';
	      ostr << mldg.find(221) << cr;
	    }
	  catch( Error_no_callsign )
	    {
	      ostr << mldg.find(215) << cr << flush;
	    }
	}
      else
	ostr << mldg.find(416) << cr << flush;
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Gruppendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

/*****************************************************************************/
/* User aus Funkruf-Gruppe loeschen                                          */
/*                                                                           */
void user_interface::del_user_from_group( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop() )
    {
      // Alle Leerzeichen ueberspringen
      cut_blanks(cmd);

      char puffer[1000];
      if (cmd.get(puffer,999,' '))
	{
	  String gname(puffer);
	  gname.upcase();
	  cut_blanks(cmd);
	  try
	    {
	      callsign call;
	      cmd >> call;
	      try
		{
		  gruppe grp = grps.find(gname);
		  grp.del_call(call);
		  ostr << mldg.find(411) << ' ' << call << ' ';
		  ostr << mldg.find(417) << ' ' << gname << ' ';
		  ostr << mldg.find(418) << cr;
		}
	      catch( Error_group_does_not_exist )
		{
		  ostr << mldg.find(400) << ' ' << gname << ' ';
		  ostr << mldg.find(406) << cr;
		}
	      catch( Error_could_not_open_groupfile )
		{
		  ostr << mldg.find(403) << cr;
		}
	      catch( Error_Callsign_not_in_group )
		{
		  ostr << mldg.find(411) << ' ' << call << ' ';
		  ostr << mldg.find(419) << cr;
		}
	      catch( Error_no_group_name )
		{
		  ostr << mldg.find(404) << cr;
		}
	    }
	  catch( Error_no_callsign )
	    {
	      ostr << mldg.find(215) << cr << flush;
	    }
	}
      else
	ostr << mldg.find(416) << cr << flush;
    }
  else
    {
      ostr << mldg.find(205) << cr << flush;
      logf.eintrag(usr.user_call(),"Zugriffsversuch auf Gruppendatenbank ohne Sysopstatus",LOGMASK_PRIVVERL);
    } 
}

/*****************************************************************************/
/* Moegliche Funkruf-Gruppen anzeigen                                        */
/*                                                                           */
void user_interface::show_groups( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  char puffer[200];
  
  cmd.get(puffer,199,' ');
  String gname(puffer);
  
  if (gname == "")
    {
      ostr << mldg.find(420) << cr;
      ostr << "--------------------------------------------------------------------" << cr;
      vector<struct gruppe_info> infos= grps.get_infos();
      for ( vector<struct gruppe_info>::iterator it = infos.begin(); it != infos.end(); ++it )
	{
	  ostr << setw(9);
	  ostr.setf(ios::left, ios::adjustfield );
	  ostr << it->name << ' ';
	  ostr << setw(5) << it->anz << "  ";
	  ostr << it->info << cr << flush;
	}
    }
  else
    {
      try
	{
	  gname.upcase();
	  gruppe grp = grps.find(gname);
	  ostr << mldg.find(400) << ' ' << gname << cr;
	  ostr << mldg.find(422) << cr; 
	  ostr << grp.get_info() << cr << cr;
	  ostr << mldg.find(423) << cr;
	  int pos = 0;
	  
	  try
	    {
	      callsign call = grp.get_first();
	      call.set_format(true);
	      ostr << call << " ";
	      try
		{
		  ostr.setf(ios::left, ios::adjustfield );
		  ostr << setw(29);
		  ostr << calls.find(call).get_name();
		}
	      catch( Error_callsign_does_not_exist )
		{
		  ostr << "Call nicht in Datenbank ";
		}
	      while (true)
		{
		  if (pos++ == 1)
		    {
		      pos = 0;
		      ostr << cr;
		    }
		  callsign call = grp.get_next();
		  call.set_format(true);
		  ostr << call << " ";
		  try
		    {
		      ostr.setf(ios::left, ios::adjustfield);
		      ostr << setw(29);
		      ostr << calls.find(call).get_name();
		    }
		  catch( Error_callsign_does_not_exist )
		    {
		      ostr << "Call nicht in Datenbank ";
		    }
		}
	    }
	  catch( Error_no_more_call_in_group )
	    {}
	    if (pos==1) ostr << cr;
	    ostr << cr;
	} 
         
      catch( Error_group_does_not_exist )
	{
	  ostr << mldg.find(400) << ' ' << gname << ' ';
	  ostr << mldg.find(406) << cr;
	}
      catch( Error_could_not_open_groupfile )
	{
	  ostr << mldg.find(403) << cr;
	}
      catch( Error_no_group_name )
	{
	  ostr << mldg.find(421) << cr;
	}
    }
}
