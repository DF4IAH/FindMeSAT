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

#include "config.h"
#include "database.h"
#include "spoolfiles.h"
#include "wx_interface.h"

extern config_file configuration;
extern callsign_database calls;
extern spoolfiles spool;
extern wx_control wx;

void user_interface::wx_config( istream &cmd, ostream &ostr )
{
  subcommands subcmd;
  cmd >> subcmd;

  if ( subcmd == c_no_subcmd )
    {
      ostr << mldg.find(824) << cr;
      ostr << mldg.find(757) << cr;
      wx.show(ostr,cr);
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
	  wx.full_show(call,ostr,cr);
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(803) << cr;
	}
    }
  else
    {
      if ( usr.is_sysop() )
	{
	  switch(subcmd)
	    {
	      case c_sub_enable   : wx.enable(configuration);
	                            ostr << mldg.find(800) << cr;
	                            break;
	      case c_sub_disable  : wx.disable(configuration);
	                            ostr << mldg.find(801) << cr;
	                            break;
	      case c_sub_add      : add_wx(cmd,ostr);
	                            break;
	      case c_sub_del      : del_wx(cmd,ostr);
	                            break;
	      case c_sub_pfad     : set_wx_pfad(cmd, ostr);
	                            break;
	      case c_sub_slots    : set_wx_slot(cmd,ostr);
	                            break;
	      case c_sub_locator  : set_wx_locator(cmd,ostr);
	                            break;
	      case c_sub_prompt   : set_wx_prompt(cmd,ostr);
	                            break;
	      case c_sub_command  : set_wx_command(cmd,ostr);
	                            break;
	      case c_sub_temp     : set_wx_temperatur(cmd,ostr);
	                            break;
	      case c_sub_druck    : set_wx_luftdruck(cmd,ostr);
	                            break;
	      case c_sub_feucht   : set_wx_luftfeuchte(cmd,ostr);
	                            break;
	      case c_sub_richt    : set_wx_windrichtung(cmd,ostr);
	                            break;
	      case c_sub_geschw   : set_wx_windgeschwindigkeit(cmd,ostr);
	                            break;
	      case c_sub_boen     : set_wx_boen(cmd,ostr);
	                            break;
	      case c_sub_nieder1  : set_wx_niederschlag1(cmd,ostr);
	                            break;
	      case c_sub_nieder4  : set_wx_niederschlag4(cmd,ostr);
	                            break;
	      case c_sub_niederm  : set_wx_niederschlag_m(cmd,ostr);
	                            break;
	      default             : ostr << mldg.find(823) << cr;
	                            break;
	    } 
	}
      else
	{
	  ostr << mldg.find(822) << cr;
	  syslog logf(configuration);
	  logf.eintrag("Zugriff auf WX-Konfiguration ohne Sysop-Privilegien!",LOGMASK_PRIVVERL);
	}
    }
}


void user_interface::add_wx( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30);
  pu.kuerze();
  try
    {
      add_wx_call = callsign(pu);
      state = add_wx_step1;
      ostr << mldg.find(802);
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::add_wx_1( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30);
  pu.kuerze();
  try
    {
      add_wx_loc = locator(pu);
      state = add_wx_step2;
      ostr << mldg.find(804) << cr;
    }
  catch( Error_not_a_locator )
    {
      ostr << mldg.find(805) << cr;
      ostr << mldg.find(815) << cr;
    }
}

void user_interface::add_wx_2( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,3000);
  pu.kuerze();
  try
    {
      connect_string cs(pu);

      if ( wx.add(add_wx_call) )
	{
	  if ( wx.set_loc(add_wx_call,add_wx_loc) &&
	       wx.set_pfad(add_wx_call,cs) )
	    {
	      ostr << mldg.find(806) << cr;
	      ostr << mldg.find(807) << cr;
	    }
	  else
	    {
	      wx.del(add_wx_call);
	      ostr << mldg.find(808) << cr;
	    }
	}
      else
	ostr << mldg.find(808) << cr;
      state = wait_cmd;
    }
  catch( Error_syntax_fehler_in_connect_string )
    {
      ostr << mldg.find(809) << cr;
      ostr << mldg.find(815) << cr;
    }
}

void user_interface::del_wx( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,30);
  pu.kuerze();
  try
    {
      callsign call(pu);
      if ( wx.del(call) )
	ostr << mldg.find(810) << cr;
      else
	{
	  ostr << mldg.find(811) << cr;
	  ostr << mldg.find(812) << cr;
	}
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_pfad( istream &cmd, ostream &ostr )
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

      if ( wx.set_pfad(call,cs) )
	ostr << mldg.find(813) << cr;
      else
	ostr << mldg.find(814) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
  catch( Error_syntax_fehler_in_connect_string )
    {
      ostr << mldg.find(809) << cr;
    }
}

void user_interface::set_wx_slot( istream &cmd, ostream &ostr )
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

      if ( wx.set_slot(call,slot) )
	ostr << mldg.find(816) << cr;
      else
	ostr << mldg.find(817) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_locator( istream &cmd, ostream &ostr )
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
      locator loc(pu);

      if ( wx.set_loc(call,loc) )
	ostr << mldg.find(818) << cr;
      else
	ostr << mldg.find(819) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
  catch( Error_not_a_locator )
    {
      ostr << mldg.find(805) << cr;
    }
}

void user_interface::set_wx_prompt( istream &cmd, ostream &ostr )
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

      if ( wx.set_prompt(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_command( istream &cmd, ostream &ostr )
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

      if ( wx.set_command(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_temperatur( istream &cmd, ostream &ostr )
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

      if ( wx.set_temperatur(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_luftdruck( istream &cmd, ostream &ostr )
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

      if ( wx.set_luftdruck(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_luftfeuchte( istream &cmd, ostream &ostr )
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

      if ( wx.set_luftfeuchte(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_windrichtung( istream &cmd, ostream &ostr )
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

      if ( wx.set_windrichtung(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_windgeschwindigkeit( istream &cmd, ostream &ostr )
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

      if ( wx.set_windgeschwindigkeit(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_boen( istream &cmd, ostream &ostr )
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

      if ( wx.set_boen(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_niederschlag1( istream &cmd, ostream &ostr )
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

      if ( wx.set_niederschlag1(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_niederschlag4( istream &cmd, ostream &ostr )
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

      if ( wx.set_niederschlag4(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

void user_interface::set_wx_niederschlag_m( istream &cmd, ostream &ostr )
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

      if ( wx.set_niederschlag_m(call,pu) )
	ostr << mldg.find(820) << cr;
      else
	ostr << mldg.find(821) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(803) << cr;
    }
}

