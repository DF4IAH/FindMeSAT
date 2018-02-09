/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2000,2001,2002 by Holger Flemming                          *
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

#include "logfile.h"
#include "default.h"
#include "cluster_interface.h"
#include "connector.h"
#include "cluster.h"
#include "talk.h"
#include "export.h"

extern bool shutdown_flag;
extern zeit shutdown_time;

extern config_file configuration;
extern cluster_interface dxc;
extern connection_control connector;
extern callsign G_mycall;

#ifdef COMPILE_CLUSTER
extern cluster_control cluster_cntl;
#endif

extern int kill_thread;
extern talker talk;

void user_interface::do_shutdown(  istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      cut_blanks(cmd);
      String min_string;
      min_string.get(cmd,80);
      min_string.kuerze();
      if (min_string.slen() == 0)
	shutdown_time = zeit() + 20;
      else
	shutdown_time = zeit() + 60 * min_string.Stoi();


      delta_t dt(shutdown_time - zeit());
      talk.send_msg(usr.user_call(),"FunkrufMaster-Shutdown in "+dt.get_string());
      ostr << mldg.find(56) << ' ' << dt << cr;
      ostr << mldg.find(6) << cr;
      logf.eintrag(usr.user_call(),"Funkruf-Server Shutdown",LOGMASK_STARTSTOP);
      shutdown_flag = true;
    }
  else
    {
      ostr << mldg.find(7) << cr;
      logf.eintrag(usr.user_call(),"Versuchter Server-Shutdown ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}


void user_interface::send( istream & cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      try
	{
	  boards rubriken;
	  subcommands subcmd;
	  cmd >> subcmd;
	  switch(subcmd)
	    {
	      case c_sub_names    : rubriken.send_names();
	                            ostr << mldg.find(8) << cr;
	                            break;
	      case c_sub_messages : rubriken.send_messages();
	                            ostr << mldg.find(9) << cr;
	                            break;
	      case c_sub_baken    : ostr << mldg.find(10) << cr;
	                            for (t_baken::iterator it = baken->begin() ; it != baken->end() ; it++)
				      (*it)->send();
	                            break;
	      case c_sub_defaults : {
		                      defaults defs;
				      defs.process();
				      ostr << mldg.find(19) << cr;
	                            }
	                            break;
	      default             : ostr << mldg.find(23) << cr;
	    }
	}
      catch( Error_could_not_create_boardnames )
	{
	  logf.eintrag("Konnte Rubriken nicht oeffnen", LOGMASK_PRGRMERR);
	}
    }
  else
    {
      ostr << mldg.find(11) << cr;
      logf.eintrag(usr.user_call(),"Versuchte Aussendung von Boarddaten ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}


String user_interface::get_file_name( istream &cmd , ostream &ostr, bool &gueltig)
{
  cut_blanks(cmd);
  String fname;
  fname.get(cmd,249,' ');
  if (fname.slen() == 0)
    {
      ostr << mldg.find(12) << cr;
      gueltig = false;
      return String("");
    }
  else if (fname.in(String("..")))
    {
      ostr << mldg.find(13) << cr;
      logf.eintrag(usr.user_call(),String("Zugriffsversuch auf suspekten Dateinamen ")+fname,LOGMASK_PRIVVERL);
      gueltig = false;
      return String("");
    }
  else
    {
      gueltig = true;
      return String(BASE_DIR) + fname;
    }
}

void user_interface::rtext( istream &cmd, ostream & ostr )
{
  bool gueltig;
  String fname = get_file_name(cmd,ostr,gueltig);
  if (usr.is_sysop())
    {
      if (gueltig)
	{
	  rt_text rt(fname,macros,cr);
	  ostr << rt;
	}
    }
  else
    {
      ostr << mldg.find(14) << cr;
      logf.eintrag(usr.user_call(),String("Versuch, Systemdatei ")+fname+String(" zu lesen, ohne Sysop-Privilegien"),LOGMASK_PRIVVERL);
    }
}

void user_interface::wtext_init(istream &cmd, ostream &ostr )
{
  bool gueltig;
  String fname = get_file_name(cmd,ostr,gueltig);
  if (usr.is_sysop())
    {
      if (gueltig)
	{
	  wtext_ptr = new wt_text(fname,macros,cr);
	  if (wtext_ptr->status())
	    {
	      state = wait_text_input;
	      ostr << mldg.find(15) << cr;
	    }
	  else
	    ostr << mldg.find(16) << cr;
	}
    }
  else
    {
      ostr << mldg.find(18) << cr;
      logf.eintrag(usr.user_call(),String("Versuch, Systemdatei ")+fname+String(" zu bescheiben, ohne Sysop-Privilegien"),LOGMASK_PRIVVERL);
    }
}

void user_interface::wtext(istream &cmd, ostream &ostr )
{
  String line;
  line.get(cmd,1000,ende);
  if (    (line.pos(String("***END")) != 0)
       && (line.pos(String("NNNN")) != 0)
       && (line.pos(String("//EX")) != 0) )
    {
      wtext_ptr->line(line);
    }
  else
    {
      delete wtext_ptr;
      ostr << mldg.find(18) << cr;
      state = wait_cmd;
    }
}

void user_interface::dxcluster(istream &cmd, ostream &ostr )
{
#ifdef COMPILE_CLUSTER
  subcommands subcmd;
  cmd >> subcmd;
  if (subcmd == c_no_subcmd)
    {
      ostr << mldg.find(762) << cr;
      ostr << mldg.find(757) << cr;
      cluster_cntl.PrintOn(ostr,cr);
    }
  else if (usr.is_sysop())
    {
      String pu;
      switch(subcmd)
	{
	  case c_sub_enable  : cluster_cntl.enable();
	                       ostr << mldg.find(763) << cr;
	                       break;
	  case c_sub_disable : cluster_cntl.disable();
	                       ostr << mldg.find(764) << cr;
	                       break;
          case c_sub_add     : try
	                         {
				   cut_blanks(cmd);
				   pu.get(cmd,20,' ');
				   callsign call(pu);
				   
				   cut_blanks(cmd);
				   pu.get(cmd,3000,' ');
				   String pu2=pu;
				   pu2.upcase();
				   dx_cluster_typ typ;
				   if (pu2 == "RAW")
				     {
				       typ = dx_raw;
				       cut_blanks(cmd);
				       pu.get(cmd,3000);
				     }
				   else if (pu2 == "PAV")
				     {
				       typ = dx_pav;
				       cut_blanks(cmd);
				       pu.get(cmd,3000);
				     }
				   else if (pu2 == "CLX")
				     {
				       typ = dx_clx;
				       cut_blanks(cmd);
				       pu.get(cmd,3000);
				     }
				   else 
				     {
				       typ = dx_sorted;
				     }
				   connect_string cs(pu);
				   
				   if (cluster_cntl.add(call,cs,typ))
				     ostr << mldg.find(765) << cr;
				   else
				     {
				       ostr << mldg.find(766) << cr;
				       ostr << mldg.find(767) << cr;
				     }
				 }
	                       catch( Error_no_callsign )
				 {
				   ostr << pu << mldg.find(702) << cr;
				 }
			       catch( Error_syntax_fehler_in_connect_string )
				 {
				   ostr << mldg.find(722) << ' ' << cr;
				   ostr << pu << ' ' << cr;
				   ostr << mldg.find(723) << cr;
				   ostr << mldg.find(724) << cr;
				 }
	                       break;
	  case c_sub_del     : try
	                         {
				   cut_blanks(cmd);
				   pu.get(cmd,20,' ');
				   callsign call(pu);
				   
				   if (cluster_cntl.del(call))
				     ostr << mldg.find(768) << cr;
				   else
				     {
				       ostr << mldg.find(769) << cr;
				       ostr << mldg.find(770) << cr;
				     }
				 }
	                       catch( Error_no_callsign )
				 {
				   ostr << pu << " " << mldg.find(702) << cr;
				 }
	                       break;
	  default            : ostr << mldg.find(23) << cr;
	}
  
    }
  else
    {
      ostr << mldg.find(540) << cr;
      logf.eintrag(usr.user_call(),"Versuch DX-Cluster-Config zu veraendern ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
#endif // COMPILE_CLUSTER
}

void user_interface::kill_th( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      cut_blanks(cmd);
      String puffer;
      puffer.get(cmd,20);
      puffer.kuerze();

      if (puffer.isnum())
	kill_thread = puffer.Stoi();
      else
	{
	  //ostr << '>' << puffer << '<' << cr;
	  ostr << mldg.find(24) << cr;
	}
    }
  else
    {
      ostr << mldg.find(25) << cr;
      logf.eintrag(usr.user_call(),"Versuch Thread zu toeten, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::connect( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      String puffer;

      cut_blanks(cmd);
      puffer.get(cmd,200,' ');
      puffer.kuerze();
      puffer.upcase();
      
      if (puffer == 'T' || puffer == 'A')
	{
	  char tp = puffer[0];
	  cut_blanks(cmd);
	  puffer.get(cmd,200);
	  puffer.kuerze();

	  connection_id = connector.connect(tp,puffer);
	  state = connect_mode;
	  ostr << mldg.find(26) << cr;
	}
      else 
	ostr << mldg.find(27) << cr;
    }  
  else
    {
      ostr << mldg.find(28) << cr;
      logf.eintrag(usr.user_call(),"Versuch Verbindung aufzubauen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::con_mode( istream &istr, ostream &ostr )
{
  String puffer;
  puffer.getline(istr,1000,ende);
  puffer.kuerze();
  if ( puffer == "~." )
    connector.discon(connection_id);
  else
    connector.send(connection_id,puffer,true);
  bool eol,fl;
  while ( connector.receive(connection_id,puffer,eol,fl) )
    {
      ostr << puffer;
      if (eol)
	ostr << cr;
    }
  if (fl)
    {
      ostr << mldg.find(29) << " " << G_mycall << cr;
      state = wait_cmd;
    }
}

void user_interface::con_mode( ostream &ostr )
{
  String puffer;
  bool eol,fl;

  while ( connector.receive(connection_id,puffer,eol,fl) )
    {
      ostr << puffer;
      if (eol)
	ostr << cr;
    }

  if (fl)
    {
      ostr << mldg.find(29) << " " << G_mycall << cr;
      state = wait_cmd;
    }
}

void user_interface::passwd( istream &cmd, ostream &ostr )
{
  if (cpw.use_pw())
    if (!cpw.ax25_connection() || usr.is_sysop())
      {
	cut_blanks(cmd);
	String puffer;
	puffer.get(cmd,100);
	puffer.kuerze();
	try
	  {
	    callsign call(puffer);
	    if (usr.is_sysop())
	      {
		passwd_call = call;
		check_old = false;
		ostr << mldg.find(30) << ' ';
		state = passwd_step2;
	      }
	    else if (samecall(call,usr.user_call()))
	      {
		if (cpw.user_change())
	        {
		    passwd_call = usr.user_call();
		    check_old = true;
		    ostr << mldg.find(32) << ' ';
		    state = passwd_step1;
	        }
		else
	    	    ostr << mldg.find(34) << cr;
	      }
	    else
	      {
		ostr << mldg.find(33) << cr;
		logf.eintrag(usr.user_call(),"Versuch Passwort zu setzen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
	      }
	  }
	catch( Error_no_callsign )
	  {
	    if (cpw.user_change())
	      {
		passwd_call = usr.user_call();
		check_old = true;
		ostr << mldg.find(32) << ' ';
		state = passwd_step1;
	      }
	    else
	      ostr << mldg.find(34) << cr;
	  }
      }
    else
      ostr << mldg.find(35) << cr << mldg.find(36) << cr;
  else
    ostr << mldg.find(37) << cr;
}

void user_interface::passwd_st1( istream &cmd, ostream &ostr )
{
  String puffer;
  puffer.get(cmd,100);
  puffer.kuerze();

  oldpw = puffer;
  ostr << mldg.find(30) << ' ';
  state = passwd_step2;
}

void user_interface::passwd_st2( istream &cmd, ostream &ostr )
{
  String puffer;
  puffer.get(cmd,100);
  puffer.kuerze();

  newpw1 = puffer;
  ostr << mldg.find(31) << ' ';
  state = passwd_step3;
}

void user_interface::passwd_st3( istream &cmd, ostream &ostr )
{
  String puffer;
  puffer.get(cmd,100);
  puffer.kuerze();

  newpw2 = puffer;

  if (newpw1 == newpw2)

    if (check_old)

      if (cpw.change_password(passwd_call,oldpw,newpw1))
	ostr << mldg.find(38) << cr;
      else
	ostr << mldg.find(39) << cr;

    else

      if (cpw.set_password(passwd_call,newpw1))
	{
	  ostr << mldg.find(41) << ' ' << passwd_call;
	  ostr << ' ' << mldg.find(42) << cr;
	}
      else
	{
	  ostr << mldg.find(41) << ' ' << passwd_call;
	  ostr << ' ' << mldg.find(43) << cr;
	}

  else
    ostr << mldg.find(40) << cr;

  state = wait_cmd;
}

void user_interface::spool_relais(relais::relais_typ tp, String brdname, double qrg1, double qrg2, istream &cmd, ostream &ostr )
{
  String pu;
  int first,last;
  cut_blanks(cmd);
  pu.get(cmd,19,' ');
  pu.kuerze();
  if (pu.slen() > 0)
    {
      first = pu.Stoi();
      cut_blanks(cmd);
      pu.get(cmd,19,' ');
      pu.kuerze();
      if (pu.slen() > 0)
	last = pu.Stoi();
      else 
	last = 10;
    }
  else
    {
      first = 1;
      last = 10;
    }
  
  relais_database r1 = rels.find_type(tp);
  relais_database r2 = r1.find_qrg(qrg1,qrg2);	
  r2.sort_dist();
  r1 = r2.reduce_n( (last - first + 1) * 2 );
  r1.sort_qrg();
  spool_database(r1,first,brdname);
  ostr << mldg.find(47) << ' ' << brdname << ' ' << mldg.find(48) << cr;
}

void user_interface::do_relais(istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      subcommands subcmd;
      cmd >> subcmd;
      switch(subcmd)
	{
	  case c_sub_load  : {
	                       bool gueltig;
			       String fname = get_file_name(cmd,ostr,gueltig);
			       if (gueltig)
				 {
				   rels.load(fname);
				   ostr << mldg.find(46) << cr;
				 }
			       else
				 ostr << mldg.find(13) << cr;
	                     }
			     break;
	  case c_sub_fm10  : spool_relais(relais::rt_fm,"Relais-10m",28.,29.7,cmd,ostr);
	                     break;
	  case c_sub_fm2   : spool_relais(relais::rt_fm,"Relais-2m",144.,146.,cmd,ostr);
	                     break;
	  case c_sub_fm70  : spool_relais(relais::rt_fm,"Relais-70cm",430.,440.,cmd,ostr);
	                     break;
	  case c_sub_fm23  : spool_relais(relais::rt_fm,"Relais-23cm",1240.,1300.,cmd,ostr);
	                     break;
	  case c_sub_atv   : spool_relais(relais::rt_atv,"ATV",0.,300000.,cmd,ostr);
	                     break;
	  case c_sub_digi  : spool_relais(relais::rt_digi,"Digipeater",0.,300000.,cmd,ostr);
	                     break;
	  case c_sub_baken : spool_relais(relais::rt_bake,"Baken",0.,300000.,cmd,ostr);
	                     break;
	  default          : ostr << mldg.find(49) << cr;
	}
    }
  else
    {
      ostr << mldg.find(50) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Relais-Datenbank",LOGMASK_PRIVVERL );
    }
}

void user_interface::exportmask( istream &cmd, ostream &ostr )
{
  syslog logf(configuration);

  if (usr.is_sysop())
    {
      String pu;
      cut_blanks(cmd);
      pu.get(cmd,19,' ');
      pu.kuerze();

      if (pu.slen() > 0)
	{
	  try
	    {
	      export_sys exp;
	      exp.set_mask(pu);
	      ostr << mldg.find(57) << cr;
	    }
	  catch( Error_could_not_initialize_exportsystem )
	    {
	      ostr << mldg.find(58) << cr;
	    }
	  catch( Error_wrong_id_in_exportmask )
	    {
	      ostr << mldg.find(59) << cr;
	    }
	  catch( Error_in_exportmask_syntax )
	    {
	      ostr << mldg.find(60) << cr;
	    }
	}
      else
	{
	  try
	    {
	      export_sys exp;
	      ostr << mldg.find(61) << ' ' << exp.get_mask() << cr;
	    }
	  catch( Error_could_not_initialize_exportsystem )
	    {
	      ostr << mldg.find(58) << cr;
	    }
	}
    }
  else
    {
      ostr << mldg.find(62) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Exportmaske",LOGMASK_PRIVVERL );
    }

}


