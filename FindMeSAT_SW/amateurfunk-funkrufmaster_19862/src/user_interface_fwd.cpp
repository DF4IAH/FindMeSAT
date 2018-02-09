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

#include "fwd_router.h"
#include "fwd_frontend.h"
#include "fwd_autorouter.h"

extern fwd_router router;
extern autorouter a_router;

void user_interface::setfwd_reset( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      String puffer;
      cut_blanks(cmd);
      puffer.get(cmd,200,' ');
      puffer.kuerze();
      try
	{
	  callsign call(puffer);
	  cut_blanks(cmd);
	  puffer.get(cmd,200,' ');
	  puffer.kuerze();
	  if (router.reset(call))
	    ostr << mldg.find(349) << cr;
	  else
	    ostr << mldg.find(348) << cr;
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      ostr << mldg.find(308) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Forward zu setzen",LOGMASK_PRIVVERL);
    }
}

void user_interface::addfwd( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      String puffer;
      cut_blanks(cmd);
      puffer.get(cmd,200,' ');
      puffer.kuerze();
      try
	{
	  callsign call(puffer);

	  cut_blanks(cmd);
	  puffer.get(cmd,200,' ');
	  puffer.kuerze();
	  puffer.upcase();

	  if (puffer == 'T')
	    {
	      char tp = puffer[0];
	      cut_blanks(cmd);
	      puffer.get(cmd,200);
	      puffer.kuerze();

	      if (router.add_partner(call,tp,puffer))
		ostr << mldg.find(316) << cr;
	      else
		{
		  ostr << mldg.find(317) << cr;
		  ostr << mldg.find(318) << cr;
		}
	    }
	  else if (puffer == 'A')
	    {
	      char tp = puffer[0];
	      cut_blanks(cmd);
	      puffer.get(cmd,200);
	      puffer.kuerze();
	      try
		{
		  connect_string cs(puffer);
		  if (router.add_partner(call,tp,puffer))
		    ostr << mldg.find(316) << cr;
		  else
		    {
		      ostr << mldg.find(317) << cr;
		      ostr << mldg.find(318) << cr;
		    }
		}
	      catch( Error_syntax_fehler_in_connect_string )
		{
		  ostr << mldg.find(319) << cr;
		  ostr << mldg.find(320) << cr;
		}
	    }
	  else
	    {
	      ostr << mldg.find(321) << cr;
	      ostr << mldg.find(320) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	  ostr << mldg.find(320) << cr;
	}
    }
  else
    {
      ostr << mldg.find(308) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Forward zu setzen",LOGMASK_PRIVVERL);
    }
}

void user_interface::delfwd( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      String puffer;
      cut_blanks(cmd);
      puffer.get(cmd,200,' ');
      puffer.kuerze();
      try
	{
	  callsign call(puffer);

	  if (router.del_partner(call))
	    ostr << mldg.find(322) << cr;
	  else
	    {
	      ostr << mldg.find(323) << cr;
	      ostr << mldg.find(324) << cr;
	      ostr << mldg.find(325) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	  ostr << mldg.find(323) << cr;
	}
    }
  else
    {
      ostr << mldg.find(308) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Forward zu setzen",LOGMASK_PRIVVERL);
    }
}



void user_interface::addfwddestin( istream &cmd, ostream &ostr )
{
  String puffer;

  cut_blanks(cmd);
  puffer.get(cmd,200,' ');
  puffer.kuerze();

  try
    {
      callsign call(puffer);

      cut_blanks(cmd);
      puffer.get(cmd,200);
      puffer.kuerze();

      destin ds(puffer);

      if (router.add_destin(call,ds))
	ostr << mldg.find(328) << cr;
      else
	{
	  ostr << mldg.find(329) << cr;
	  ostr << mldg.find(330) << cr;
	  ostr << mldg.find(331) << cr;
	}
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(215) << cr;
      ostr << mldg.find(329) << cr;
    }
  catch( Error_no_destin )
    {
      ostr << mldg.find(729) << cr;
      ostr << mldg.find(329) << cr;
    }
}


void user_interface::delfwddestin( istream &cmd, ostream &ostr )
{
  String puffer;

  cut_blanks(cmd);
  puffer.get(cmd,200,' ');
  puffer.kuerze();

  try
    {
      callsign call(puffer);

      cut_blanks(cmd);
      puffer.get(cmd,200);
      puffer.kuerze();

      destin ds(puffer);

      if (router.del_destin(call,ds))
	ostr << mldg.find(332) << cr;
      else
	{
	  ostr << mldg.find(333) << cr;
	  ostr << mldg.find(330) << cr;
	  ostr << mldg.find(334) << cr;
	}
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(215) << cr;
      ostr << mldg.find(333) << cr;
    }
  catch( Error_no_destin )
    {
      ostr << mldg.find(729) << cr;
      ostr << mldg.find(333) << cr;
    }
}

void user_interface::setfwd_pfad( istream &cmd, ostream &ostr )
{
  String puffer;
  cut_blanks(cmd);
  puffer.get(cmd,200,' ');
  puffer.kuerze();
  try
    {
      callsign call(puffer);
      
      cut_blanks(cmd);
      puffer.get(cmd,200,' ');
      puffer.kuerze();
      puffer.upcase();
      
      if (puffer == 'T')
	{
	  char tp = puffer[0];
	  cut_blanks(cmd);
	  puffer.get(cmd,200);
	  puffer.kuerze();
	  
	  if (router.set_pfad(call,tp,puffer))
	    ostr << mldg.find(339) << cr;
	  else
	    {
	      ostr << mldg.find(317) << cr;
	      ostr << mldg.find(318) << cr;
	    }
	}
      else if (puffer == 'A')
	{
	  char tp = puffer[0];
	  cut_blanks(cmd);
	  puffer.get(cmd,200);
	  puffer.kuerze();
	  try
	    {
	      connect_string cs(puffer);
	      if (router.set_pfad(call,tp,puffer))
		ostr << mldg.find(339) << cr;
	      else
		{
		  ostr << mldg.find(340) << cr;
		  ostr << mldg.find(341) << cr;
		}
	    }
	  catch( Error_syntax_fehler_in_connect_string )
	    {
	      ostr << mldg.find(319) << cr;
	      ostr << mldg.find(338) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(321) << cr;
	  ostr << mldg.find(320) << cr;
	}
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(215) << cr;
      ostr << mldg.find(338) << cr;
    }
}

void user_interface::setfwd_ar_enable( istream &cmd, ostream &ostr )
{
  String puffer;
  cut_blanks(cmd);
  puffer.get(cmd,200,' ');
  puffer.kuerze();
  
  if (puffer != "")
    {
      try
	{
	  callsign call(puffer);
	  if ( router.enable_autorouting(call) )
	    {
	      ostr << mldg.find(342) << cr;
	    }
	  else
	    {
	      ostr << mldg.find(343) << cr;
	      ostr << mldg.find(324) << cr;
	      ostr << mldg.find(325) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      a_router.enable();
      ostr << mldg.find(344) << cr;
    }
}

void user_interface::setfwd_ar_disable( istream &cmd, ostream &ostr )
{
  String puffer;
  cut_blanks(cmd);
  puffer.get(cmd,200,' ');
  puffer.kuerze();

  if (puffer != "")
    {
      try
	{
	  callsign call(puffer);
	  if ( router.disable_autorouting(call) )
	    {
	      ostr << mldg.find(345) << cr;
	    }
	  else
	    {
	      ostr << mldg.find(346) << cr;
	      ostr << mldg.find(324) << cr;
	      ostr << mldg.find(325) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      a_router.disable();
      ostr << mldg.find(347) << cr;
    }
}

void user_interface::setfwd( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      subcommands subcmd;
      cmd >> subcmd;
      switch(subcmd)
	{
	  case c_sub_enable    : if (router.enable())
	                           ostr << mldg.find(300) << cr;
	                         else
				   {
				     ostr << mldg.find(301) << cr;
				     ostr << mldg.find(302) << cr;
				     ostr << mldg.find(303) << cr;
				   }
	                         break;
	  case c_sub_disable   : if (router.disable())
	                           ostr << mldg.find(304) << cr;
	                         else
				   {
				     ostr << mldg.find(305) << cr;
				     ostr << mldg.find(306) << cr;
				   }
	                         break;
	  case c_sub_adddestin : addfwddestin(cmd,ostr);
	                         break;
	  case c_sub_deldestin : delfwddestin(cmd,ostr);
	                         break;
          case c_sub_pfad      : setfwd_pfad(cmd,ostr);
	                         break;
	  case c_sub_reset     : setfwd_reset(cmd,ostr);
				 break;				 
	  case c_sub_ar_enable : setfwd_ar_enable(cmd,ostr);
	                         break;
	  case c_sub_ar_disable: setfwd_ar_disable(cmd,ostr);
	                         break;
	  default              : ostr << mldg.find(307) << cr;
	}
    }
  else
    {
      ostr << mldg.find(308) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Forward zu setzen",LOGMASK_PRIVVERL);
    }
}

void user_interface::savefwd( ostream &ostr )
{
  if (usr.is_sysop())
    {
      router.save_fwd();
      ostr << mldg.find(326) << cr;
    }
  else
    {
      ostr << mldg.find(327) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Forward zu speichern",LOGMASK_PRIVVERL);
    }
}

void user_interface::dbrequest( istream &cmd, ostream &ostr )
{
  fwd_api fapi;

  if (usr.is_sysop())
    {
      try
	{
	  char puffer[200];
	  zeit von;
	  String pu;
	  cut_blanks(cmd);
	  cmd.get(puffer,19,' ');
	  pu = String(puffer);
	  callsign call(pu);

	  cut_blanks(cmd);
	  cmd.get(puffer,199);
	  pu = String(puffer);

	  if (pu.slen() > 0)
	    {
	      int days = pu.Stoi();
	      if (days > 7305)
		von = zeit(0);
	      else
		{
		  zeit zt;
		  von = zeit(zt - days * 86400);
		}
	    }
	  else
	    von = zeit(0);

	  fapi.fwd_update_request(von,call);
	  if (von == zeit(0))
	    ostr << mldg.find(309) << cr;
	  else
	    ostr << mldg.find(310) << ' ' << von << ' ' << mldg.find(311) << cr;
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(312) << cr;
	}
    }
  else
    {
      ostr << mldg.find(313) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Datenbank anzufordern",LOGMASK_PRIVVERL);
    }
}

void user_interface::dbtrans( istream &cmd, ostream &ostr )
{
  fwd_api fapi;

  if (usr.is_sysop())
    {
      zeit von;
      zeit bis;
      String pu;
      cut_blanks(cmd);
      pu.get(cmd,199,' ');
      pu.kuerze();
      if (pu.slen() > 0)
	{
	  int days = pu.Stoi();
	  zeit t1 = zeit(zeit() - days * 86400);
	  
	  String pu;
	  cut_blanks(cmd);
	  pu.get(cmd,199,' ');
	  pu.kuerze();
	  if (pu.slen() > 0)
	    {
	      int days = pu.Stoi();
	      zeit t2 = zeit(zeit() - days * 86400);
	      
	      von = t2;
	      bis = t1;
	    }
	  
	  else
	    {
	      von = t1;
	      bis = zeit();
	    }
	}
      else
	{
	  von = zeit(0);
	  bis = zeit();
	}
      
      int cnt = fapi.fwd_database_transmitt(von,bis);
      ostr << mldg.find(335) << ' ' << cnt << ' ' << mldg.find(336) << cr;
    }
  else
    {
      ostr << mldg.find(337) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Datenbank weiterzuleiten",LOGMASK_PRIVVERL);
    }
}

void user_interface::showfwdnb_full( struct neighbor_info info, ostream &ostr )
{
  info.call.set_format(true);
  info.call.set_nossid(true);
  ostr << " " << info.call << " : " << cr;
  ostr << "-------------------------------------------------------------" << cr;
  ostr << mldg.find(351) << ' ' << info.typ << cr;
  ostr << mldg.find(352) << ' ' << info.address << cr;
  ostr << mldg.find(353) << ' ';
  switch(info.stat)
    {
    case st_getrennt    : ostr << mldg.find(354);
      break;
    case st_aufbau      : ostr << mldg.find(355);
      break;
    case st_aktiv       : ostr << mldg.find(356);
      break;
    case st_gescheitert : ostr << mldg.find(357);
      ostr << "  (" << info.sch_cnt << ")";
      break;
    }
  ostr << '.' << cr;
  ostr << mldg.find(358) << ' ';
  delta_t dt(zeit() - info.last_change);
  ostr << dt << cr;
  ostr << mldg.find(359) << ' ';
  vector<int>::iterator it = info.akt_thrds.begin();
  if ( it != info.akt_thrds.end() )
    {
      ostr << *it;
      
      while ( ++it != info.akt_thrds.end() )
	ostr << ", " << *it;
    }
  ostr << cr;
  ostr << mldg.find(365) << cr;
  ostr << mldg.find(366) << ' ' << info.n_pers << cr;
  ostr << mldg.find(367) << ' ' << info.n_bul << cr;
  ostr << mldg.find(368) << ' ' << info.n_dest << cr;
  ostr << mldg.find(369) << ' ' << info.n_sonst << cr;
  ostr << mldg.find(370) << cr;
  dt = delta_t(info.t_w);
  ostr << mldg.find(371) << ' ' << dt << cr;
  ostr << mldg.find(372) << ' ' << info.n_max << cr;
  ostr << mldg.find(373) << ' ' << info.unack << cr;
  ostr << mldg.find(374) << ' ' << info.fehler_zaehler << cr;
  ostr << mldg.find(360) << ' ';
  if (info.mean_rtt != -1)
    {
      dt = delta_t(info.mean_rtt);
      ostr << dt;
    }
  else
    ostr << " -----";
  ostr << cr;
  ostr << "               ";
  int i = 0;
  
  vector<double>::iterator itd;
  
  for (itd = info.rtt.begin(); itd != info.rtt.end(); ++itd)
    {
      if (*itd != -1)
	{
	  dt = delta_t(*itd);
	  ostr << dt << "";
	}
      else
	ostr << " -----  ";
      if (++i == 8)
	{
	  ostr << cr;
	  ostr << "               ";
	}
    }
  ostr << cr;
  ostr << mldg.find(361) << ' ';
  if (info.mean_offset != -9999)
    {
      dt = delta_t(info.mean_offset);
      ostr << dt;
    }
  else
    ostr << " -----";
  ostr << cr;
  ostr << "               ";
  i = 0;
  for (itd = info.offset.begin(); itd != info.offset.end(); ++itd)
    {
      if (*itd != -9999)
	{
	  dt = delta_t(*itd);
	  ostr << dt << "";
	}
      else
	ostr << " -----  ";
      if (++i == 8)
	{
	  ostr << cr;
	  ostr << "               ";
	}
    }
  ostr << cr;
}

void user_interface::showfwdnb( struct neighbor_info info, ostream &ostr )
{
  info.call.set_format(true);
  info.call.set_nossid(true);
  ostr << " " << info.call << " : ";
  String trm;
  switch(info.stat)
    {
    case st_getrennt    : trm = mldg.find(362)+"         "; 
      ostr << trm.copy(0,9);
      break;
    case st_aufbau      : trm = mldg.find(363)+"         "; 
      ostr << trm.copy(0,9);
      break;
    case st_aktiv       : trm = mldg.find(364)+"         "; 
      ostr << trm.copy(0,9);
      break;
    case st_gescheitert : ostr << " --- ";
      if (info.sch_cnt > 99)
	ostr << "(**)";
      else
	ostr << '(' << setw(2) << info.sch_cnt << ')';
      break;
    }
  delta_t dt(zeit() - info.last_change);
  ostr << " " << dt << " ";
  ostr << setw(5) << info.n_pers  << " ";
  ostr << setw(5) << info.n_bul   << " ";
  ostr << setw(5) << info.n_sonst << " ";
  ostr << setw(5) << info.unack << "  ";
  if (info.mean_rtt != -1)
    {
      dt = delta_t(info.mean_rtt);
      ostr << dt << " ";
    }
  else
    ostr << " -----   ";
  
  if (info.mean_offset != -9999)
    {
      dt = delta_t(info.mean_offset);
      ostr << dt << " ";
    }
  else
    ostr << " -----   ";
  
  ostr << info.options;
  ostr << cr;
}


void user_interface::showfwd( istream &cmd, ostream &ostr )
{
  vector<struct neighbor_info> nbs;
  bool akt;
  bool full_info = false;

  try
    {
      String pu;
      cut_blanks(cmd);
      pu.get(cmd,19,' ');
      callsign call(pu);

      nbs = router.get_infos(akt,call);
      full_info = true;
    }
  catch( Error_no_callsign )
    {
      nbs = router.get_infos(akt);
    }
  if (akt)
    {
      if (!full_info)
	{
	  ostr << mldg.find(314) << cr;
	  ostr << mldg.find(315) << cr;
	}
      for (vector<struct neighbor_info>::iterator it = nbs.begin(); it != nbs.end(); ++it )
	if (full_info)
	  showfwdnb_full(*it,ostr);
	else
	  showfwdnb(*it,ostr);

    }
  else
    ostr << mldg.find(350) << cr;
}
