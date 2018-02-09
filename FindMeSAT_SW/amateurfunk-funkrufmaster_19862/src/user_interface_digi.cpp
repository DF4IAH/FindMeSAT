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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 * List of other authors:                                                   *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *                                                                          *
 ****************************************************************************/

#include "user_interface.h"

#include "config.h"
#include "database.h"
#include "spoolfiles.h"
#include "digi.h"

extern config_file configuration;
extern digi_control digi;

void user_interface::digi_config( istream &cmd, ostream &ostr )
{
  subcommands subcmd;
  cmd >> subcmd;

  if ( subcmd == c_no_subcmd )
    {
      ostr << mldg.find(919) << cr;
      ostr << mldg.find(920) << cr;
      digi.show(ostr,cr);
    }
  else if (subcmd == c_sub_show )
    {
      try
	{
	  cut_blanks(cmd);
	  String pu;
	  pu.get(cmd,30);
	  pu.kuerze();
	  callsign call(pu);
	  digi.full_show(call,ostr,cr);
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(903) << cr;
	}
    }
  else if ( usr.is_sysop() )
    {
	  if (subcmd == c_sub_enable || subcmd == c_sub_disable )
	  {
		cut_blanks(cmd);
		String pu;
		pu.get(cmd,30);
		pu.kuerze();
		if (pu.slen()==0)
		{
		    if (subcmd == c_sub_enable)
		    {
			digi.enable(configuration);
	    		ostr << mldg.find(917) << cr;
		    }
		    else
		    {
			digi.disable(configuration);
	    		ostr << mldg.find(918) << cr;
		    }    
		}
		else
		{	
		    try
		    {
    			callsign call(pu);
			bool status=false;
			if (subcmd == c_sub_enable)
		    	    status=true;
			set_digi_status(call,status,ostr);
		    }
		    catch( Error_no_callsign )
		    {
			ostr << mldg.find(903) << cr;
		    }
		}
	    }
	  else switch(subcmd)
	    {
	      case c_sub_add      : add_digi(cmd,ostr);
	                            break;
	      case c_sub_del      : del_digi(cmd,ostr);
	                            break;
	      case c_sub_addlink  : add_digi_link(cmd,ostr);
	                            break;
	      case c_sub_dellink  : del_digi_link(cmd,ostr);
	                            break;
	      case c_sub_pfad     : set_digi_pfad(cmd, ostr);
	                            break;
	      case c_sub_slots    : set_digi_slot(cmd,ostr);
	                            break;
	      case c_sub_typ      : set_digi_typ(cmd,ostr);
	                            break;
	      default             : ostr << mldg.find(900) << cr;
	                            break;
	    } 
	}
      else
	{
	  ostr << mldg.find(904) << cr;
	  syslog logf(configuration);
	  logf.eintrag("Zugriff auf Digi-Konfiguration ohne Sysop-Privilegien!",LOGMASK_PRIVVERL);
	}
}


void user_interface::add_digi( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  callsign digi_call;
  String pu;
  pu.get(cmd,30);
  pu.kuerze();
  try
    {
      digi_call = callsign(pu);
      if (digi.add(digi_call))
      {
          ostr << mldg.find(906) << cr;
      }
      else
          ostr << mldg.find(905) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}

void user_interface::del_digi( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30);
  pu.kuerze();
  try
    {
      callsign call(pu);
      if ( digi.del(call) )
      ostr << mldg.find(907) << cr;
      else
	{
	  ostr << mldg.find(908) << cr;
	  ostr << mldg.find(909) << cr;
	}	
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}

void user_interface::add_digi_link( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  callsign digi_call;
  callsign link_call;
  String pu;
  cut_blanks(pu);
  pu.get(cmd,30,' ');
  pu.kuerze();
  try
    {
      digi_call = callsign(pu);
      cut_blanks(cmd);
      pu.get(cmd,30,' ');
      pu.kuerze();
      link_call = callsign(pu);        
      if (digi.add_link(digi_call, link_call))
      {
          ostr << mldg.find(910) << cr;
      }
      else
          ostr << mldg.find(912) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}

void user_interface::del_digi_link( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  callsign digi_call;
  callsign link_call;
  String pu;
  cut_blanks(pu);
  pu.get(cmd,30,' ');
  pu.kuerze();
  try
    {
      digi_call = callsign(pu);
      cut_blanks(cmd);
      pu.get(cmd,30,' ');
      pu.kuerze();
      link_call = callsign(pu);        
      if (digi.del_link(digi_call, link_call))
      {
          ostr << mldg.find(911) << cr;
      }
      else
          ostr << mldg.find(913) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}


void user_interface::set_digi_pfad( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30,' ');
  pu.kuerze();
  try
    {
      callsign call(pu);

      cut_blanks(cmd);
      String pu;
      pu.get(cmd,2000);
      pu.kuerze();
      connect_string cs(pu);

      if ( digi.set_pfad(call,cs) )
	ostr << mldg.find(813) << cr;
      else
	ostr << mldg.find(814) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
  catch( Error_syntax_fehler_in_connect_string )
    {
      ostr << mldg.find(809) << cr;
    }
}

void user_interface::set_digi_slot( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30,' ');
  pu.kuerze();
  try
    {
      callsign call(pu);
      cut_blanks(cmd);
      String pu;
      pu.get(cmd,20);
      pu.kuerze();
      int slot = pu.Stoi();
      if ( digi.set_slot(call,slot) )
	ostr << mldg.find(816) << cr;
      else
	ostr << mldg.find(817) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}

void user_interface::set_digi_status(callsign &call, bool status, ostream &ostr )
{
    if ( digi.set_status(call,status) )
    {
	if (status)
	    ostr << mldg.find(914) << " " << call << cr;
	else
	    ostr << mldg.find(915) << " " << call << cr;
	
    }
    else
        ostr << mldg.find(916) << cr;
}


void user_interface::set_digi_typ( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30,' ');
  pu.kuerze();
  try
    {
      callsign call(pu);

      cut_blanks(cmd);
      String pu;
      pu.get(cmd,2000);
      pu.kuerze();
      pu.upcase();

      if ( digi.set_typ(call,pu) )
	ostr << mldg.find(901) << pu << cr;
      else
        ostr << mldg.find(902) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(903) << cr;
    }
}

