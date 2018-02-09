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
#include <time.h>

#include "autoconf.h"
#include "config.h"
#include "crontab.h"
#include "database.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#ifdef COMPILE_SAT
#include "sat.h"
#endif
#ifdef COMPILE_TIDE
#include "gezeiten.h"
#endif
#include "talk.h"
#include "trace.h"
#include "fwd_autorouter.h"
#include "fwd_router.h"
#include "system_info.h"

extern config_file configuration;
extern callsign_database calls;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif
#ifdef COMPILE_SAT
extern sat satelliten;
#endif
extern talker talk;
extern trace tracer;
#ifdef COMPILE_TIDE
extern gezeiten_control tiden;
#endif
extern autorouter a_router;
extern fwd_router router;

extern cpu_messung glob_cpu_messung;
/*****************************************************************************/
/* Hilfe ausgeben                                                            */
/*                                                                           */
void user_interface::help(istream &cmd, ostream &ostr)
{
  htext hlp(configuration,macros,usr.language,cr);
  String help_kommando;

  cut_blanks(cmd);
  help_kommando.get(cmd,999,' ');
  if (!hlp.print(ostr,help_kommando,usr))
    {
      ostr << mldg.find(500) << ' ' << help_kommando << ' ';
      ostr << mldg.find(501) << cr;
      ostr << mldg.find(502) << cr << cr;
      hlp.print(ostr,"",usr);
    }
}

/*****************************************************************************/
/* Versionsinformation und Systeminfo ausgeben                               */
/*                                                                           */
void user_interface::version( istream &cmd, ostream &ostr )
{
  system_info sysinf;
  int port;
  String eingabe;
  cut_blanks(cmd);
  eingabe.get(cmd,200,' ');

  if (eingabe==String("*"))
  {
      ostr << cr;
      ostr << "FunkrufMaster  Version " << VERSION << cr;
      ostr << "by DH4DAI, DH6BB" << cr;
      ostr << mldg.find(503) << ' ' << __DATE__ << ' ';
      ostr << mldg.find(504) << ' ' << __TIME__ << cr;
      ostr << sysinf.get_compile_options() << cr << cr;
      ostr << macros.get_linux_version() << cr << cr;
      ostr << mldg.find(505) << ' ' << sysinf.get_cpu_typ() << cr;
      ostr << mldg.find(506) << ' ' << sysinf.get_cpu_speed() << ' ';
      ostr << mldg.find(508) << ' ' << sysinf.get_bogomips() << ' ';
      ostr << mldg.find(507) << cr << cr;
      ostr << mldg.find(509) << ' ';
      PrintMem(ostr,sysinf.get_total_memory());
      ostr << ' ' << mldg.find(510) << ' ';
      PrintMem(ostr,sysinf.get_used_memory());
      ostr << ' ' << mldg.find(511) << ' ';
      PrintMem(ostr,sysinf.get_free_memory());
      ostr << ' ' << mldg.find(512) << cr;
      ostr << mldg.find(513) << ' ';
      PrintMem(ostr,sysinf.get_total_swap());
      ostr << ' ' << mldg.find(510) << ' '; 
      PrintMem(ostr,sysinf.get_used_swap());
      ostr << ' ' << mldg.find(514);
      ostr << cr << cr;
      ostr << mldg.find(515) << cr;
      PrintMem(ostr,sysinf.get_pmem_lck());
      ostr << ' ' << mldg.find(516) << ' ';
      PrintMem(ostr,sysinf.get_pmem_rss());
      ostr << ' ' << mldg.find(517) << ' ';
      PrintMem(ostr,sysinf.get_pmem_data());
      ostr << ' ' << mldg.find(518) << ' ';
      PrintMem(ostr,sysinf.get_pmem_stack());
      ostr << ' ' << mldg.find(519) << cr;
      PrintMem(ostr,sysinf.get_pmem_exec());
      ostr << ' ' << mldg.find(520) << ' ';
      PrintMem(ostr,sysinf.get_pmem_lib());
      ostr << ' ' << mldg.find(521) << ' ';
      PrintMem(ostr,sysinf.get_pmem_size());
      ostr << ' ' << mldg.find(522) << ' ';
      ostr << cr << cr;
      ostr << mldg.find(575) << ' ' << sysinf.get_descriptor_anz();
      ostr << cr << cr; 
      ostr << mldg.find(523) << ' ' << start_time << cr;
      ostr << mldg.find(524) << ' ';
      delta_t ut(zeit() - start_time);
      ostr << ut << cr;
      cpu_time tms = glob_cpu_messung.stop();
      ut = delta_t(tms.user_time);
      ostr << mldg.find(525) << ' ' << ut << cr;
      ut = delta_t(tms.sys_time);
      ostr << mldg.find(526) << ' ' << ut << cr;

      zeit sys_start = linux_start();
      ostr << mldg.find(527) << ' ' << sys_start << cr;
      ut = delta_t(zeit() - sys_start);
      ostr << mldg.find(528) << ' ' << ut << cr;
      ostr << setfill(' ');
      ostr << mldg.find(529) << ' ' << calls.get_size() << ' ';
      ostr << '(' << calls.get_size_pager() << ' ' << mldg.find(589) << ')' << cr;
      if (timeout == 0)
	ostr << mldg.find(530) << cr;
      else
	ostr << mldg.find(531) << ' ' << delta_t(timeout) << cr;

      try 
        { 
    	    port=configuration.find("TELNET_PORT").Stoi(); 
	    if (port>0)
    		ostr << mldg.find(650) << ' ' << port << cr;
	} 
	catch( Error_parameter_not_defined ) {}
      try 
        { 
    	    port=configuration.find("HTTP_PORT").Stoi(); 
	    if (port>0)
    		ostr << mldg.find(651) << ' ' << port << cr;
	} 
	catch( Error_parameter_not_defined ) {}
      try 
        { 
    	    port=configuration.find("SMTP_PORT").Stoi(); 
	    if (port>0)
    		ostr << mldg.find(652) << ' ' << port << cr;
	} 
	catch( Error_parameter_not_defined ) {}
      try 
        { 
    	    port=configuration.find("FWD_PORT").Stoi(); 
	    if (port>0)
    		ostr << mldg.find(653) << ' ' << port << cr;
	} 
	catch( Error_parameter_not_defined ) {}
      ostr << cr;
  }
  else
  {
      ostr << cr;
      ostr << "FunkrufMaster Version " << VERSION << cr;
      ostr << "by DH4DAI, DH6BB" << cr;
      ostr << mldg.find(523) << ' ' << start_time << cr;
      ostr << mldg.find(524) << ' ';
      delta_t ut(zeit() - start_time);
      ostr << ut << cr;
      zeit sys_start = linux_start();
      ostr << mldg.find(527) << ' ' << sys_start << cr;
      ut = delta_t(zeit() - sys_start);
      ostr << mldg.find(528) << ' ' << ut << cr; 
      ostr << mldg.find(515) << ' ';
      PrintMem(ostr,sysinf.get_pmem_size());
      ostr << cr << cr;
  }
}

/*****************************************************************************/
/* Nachrichtenstatistik ausgeben                                                 */
/*    
                                                                       */
void user_interface::statistik(  ostream &ostr )
{
  ostr << cr;
  ostr << mldg.find(532) << cr;
  ostr << mldg.find(533) << cr;
#ifdef COMPILE_SLAVES
  ostr << mldg.find(534) << cr;
  ostr << mldg.find(535) << cr;
  spoolstatistic stat = spool.get_stat();
  stat.set_cr(cr);
  ostr << stat;
#endif
  ostr << mldg.find(594) << cr;
  ostr << mldg.find(535) << cr;
  fwdstatistic stat2 = router.get_stat();
  stat2.set_cr(cr);
  ostr << stat2;
}

void user_interface::cstatus(  ostream &ostr )
{
  system_info sysinf;
  bool first_flag = true;
  ostr << mldg.find(536) << cr;
  ostr << mldg.find(537) << cr;
  struct thread_info ln;
  while (sysinf.get_tlist(first_flag,ln))
    {
      ostr << setw(4) << ln.pid;
      ostr << ' ' << ln.direction << ' ';
      ostr << ' ' << ln.state << ' ';
      ostr << ln.typ;
      ostr << ln.call;

      ostr << ln.login_zeit << ' ';
      ostr << ln.user_zeit << ' ';
      ostr << ln.sys_zeit << "   ";
      PrintMem(ostr,ln.in_bytes);
      ostr << "   ";
      PrintMem(ostr,ln.out_bytes);
      ostr << cr;
      first_flag = false;
    }
  ostr << cr << cr;
}

void user_interface::descr( ostream &ostr )
{
  if (usr.is_sysop())
    {
      ostr << mldg.find(576) << cr;
      ostr << "-------------------------------------------------------" << cr;
      system_info sysinf;
      sysinf.Print_descriptoren(ostr,cr);
      ostr << cr << cr;
    }
  else
    {
      ostr << mldg.find(577) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf Dateidescriptoren zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::zeit_datum( ostream &ostr )
{
  zeit jetzt;

  jetzt.set_darstellung(zeit::f_zeitdatum_l);
  ostr << cr << cr << jetzt << cr;
}

void user_interface::log( istream &cmd, ostream &ostr, logfile &log )
{
  cut_blanks(cmd);
  char ch;
  char period = 'h';
  int cnt;
  vector<String> lines;

  String anz_str("");
  int anz;
  String part;
  zeit logdate = zeit();

  cmd.get(ch);
  if (ch == '-')
    while (cmd.get(ch) && isalpha(ch))
      
      if (ch == 'd')
	logdate = get_datum(cmd);
      else
	period = ch;
  
  else
    cmd.putback(ch);

  while (cmd.get(ch) && isdigit(ch))
    anz_str.append(ch);
  cmd.putback(ch);
  
  if (anz_str == "")
    anz = 15;
  else
    anz = anz_str.Stoi();
  
  cmd >> part;

  lines = log.get_lines(cnt,anz,part,period,logdate);

  for (vector<String>::iterator it = lines.begin(); it != lines.end(); ++it )
    ostr << *it << cr;

  ostr << cnt << ' ' << mldg.find(597) << cr;
  if ((int) lines.size() == cnt)
    ostr << mldg.find(5100) << cr;
  else
    ostr << mldg.find(598) << ' ' << lines.size() << ' ' << mldg.find(599) << cr;
}
  
void user_interface::slog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      log(cmd,ostr,logf);
    }
  else
    {
      ostr << mldg.find(538) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf Systemlog zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::ilog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      iplog il(configuration);
      log(cmd,ostr,il);
    }
  else
    {
      ostr << mldg.find(539) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf IP-log zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::hlog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      httplog hl(configuration);
      log(cmd,ostr,hl);
    }
  else
    {
      ostr << mldg.find(566) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf HTTP-log zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::flog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      fwdlog fl(configuration);
      log(cmd,ostr,fl);
    }
  else
    {
      ostr << mldg.find(573) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf Forward-log zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::rlog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      rpclog rl(configuration);
      log(cmd,ostr,rl);
    }
  else
    {
      ostr << mldg.find(574) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf RPC-log zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::ulog( istream &cmd, ostream &ostr )
{
  ostr << mldg.find(591) << cr;
  ostr << mldg.find(537) << cr;
  log(cmd,ostr,ulogf);

}

void user_interface::splog( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      spoollog spl(configuration);
      log(cmd,ostr,spl);
    }
  else
    {
      ostr << mldg.find(596) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf Spool-log zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::logmask( istream &cmd, ostream &ostr )
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
	      unsigned int mask = logf.logmaskid(pu);
	      configuration.set("LOGMASK",itoS((int) mask));
	      configuration.save();
	      ostr << mldg.find(51) << cr;
	    }
	  catch( Error_wrong_id_in_logmask )
	    {
	      ostr << mldg.find(52) << cr;
	    }
	  catch( Error_in_logmask_syntax )
	    {
	      ostr << mldg.find(53) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(55) << ' ' << logf.logmaskid() << cr;
	}
    }
  else
    {
      ostr << mldg.find(54) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Logmaske",LOGMASK_PRIVVERL );
    }

}

void user_interface::pagemask( istream &cmd, ostream &ostr )
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
	      unsigned int mask = logf.pagemaskid(pu);
	      configuration.set("SYSOP_INFO_MASK",itoS((int) mask));
	      configuration.save();
	      ostr << mldg.find(51) << cr;
	    }
	  catch( Error_wrong_id_in_logmask )
	    {
	      ostr << mldg.find(52) << cr;
	    }
	  catch( Error_in_logmask_syntax )
	    {
	      ostr << mldg.find(53) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(55) << ' ' << logf.pagemaskid() << cr;
	}
    }
  else
    {
      ostr << mldg.find(54) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Logmaske",LOGMASK_PRIVVERL );
    }

}

void user_interface::fwdlogmask( istream &cmd, ostream &ostr )
{
  syslog logf(configuration);
  fwdlog flogf(configuration);

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
	      unsigned int mask = flogf.logmaskid(pu);
	      configuration.set("FWDLOGMASK",itoS((int) mask));
	      configuration.save();
	      ostr << mldg.find(51) << cr;
	    }
	  catch( Error_wrong_id_in_logmask )
	    {
	      ostr << mldg.find(52) << cr;
	    }
	  catch( Error_in_logmask_syntax )
	    {
	      ostr << mldg.find(53) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(55) << ' ' << flogf.logmaskid() << cr;
	}
    }
  else
    {
      ostr << mldg.find(54) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Logmaske",LOGMASK_PRIVVERL );
    }

}

void user_interface::rpclogmask( istream &cmd, ostream &ostr )
{
  syslog logf(configuration);
  rpclog rlogf(configuration);

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
	      unsigned int mask = rlogf.logmaskid(pu);
	      configuration.set("RPCLOGMASK",itoS((int) mask));
	      configuration.save();
	      ostr << mldg.find(51) << cr;
	    }
	  catch( Error_wrong_id_in_logmask )
	    {
	      ostr << mldg.find(52) << cr;
	    }
	  catch( Error_in_logmask_syntax )
	    {
	      ostr << mldg.find(53) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(55) << ' ' << rlogf.logmaskid() << cr;
	}
    }
  else
    {
      ostr << mldg.find(54) << cr;
      syslog logf(configuration);
      logf.eintrag("Nicht privilegierter Zugriff auf Logmaske",LOGMASK_PRIVVERL );
    }

}

void user_interface::spoollogmask( istream &cmd, ostream &ostr )
{
  syslog logf(configuration);
  spoollog splogf(configuration);

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
	      unsigned int mask = splogf.logmaskid(pu);
	      configuration.set("SPOOLLOGMASK",itoS((int) mask));
	      configuration.save();
	      ostr << mldg.find(51) << cr;
	    }
	  catch( Error_wrong_id_in_logmask )
	    {
	      ostr << mldg.find(52) << cr;
	    }
	  catch( Error_in_logmask_syntax )
	    {
	      ostr << mldg.find(53) << cr;
	    }
	}
      else
	{
	  ostr << mldg.find(55) << ' ' << splogf.logmaskid() << cr;
	}
    }
  else
    {
      ostr << mldg.find(54) << cr;
      logf.eintrag("Nicht privilegierter Zugriff auf Logmaske",LOGMASK_PRIVVERL );
    }

}

void user_interface::param( istream& cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      cut_blanks(cmd);
      String parameter;
      String wert;
      parameter.get(cmd,249,' ');
      parameter.kuerze();
      if (parameter == String(""))
	configuration.printOn(ostr,cr);
      else
	{
	  parameter.upcase();
	  cut_blanks(cmd);
	  wert.get(cmd,249);
	  wert.kuerze();
	  configuration.set(parameter,wert);
	  configuration.save();
	  //configuration.load();	// Und config neu laden.  Warum dieses ?? (DH4DAI)
	}
    }
  else
    {
      ostr << mldg.find(540) << cr;
      logf.eintrag(usr.user_call(),"Versuch auf Systemparameter zuzugreifen, ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

#ifdef COMPILE_TIDE

void user_interface::add_ort( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      pu.upcase();
      switch (tiden.add(pu))
	{
	  case 0: ostr << mldg.find(582) << " " << pu;
	          ostr << " " << mldg.find(551) << cr;
		  break;
	  case 1: ostr << mldg.find(582) << " " << pu;
	          ostr << " " << mldg.find(553) << cr;
		  break;
	  case 2: ostr << mldg.find(582) << " " << pu;
	          ostr << " " << mldg.find(555) << cr;
		  break;
	  case 3: ostr << mldg.find(582) << " " << pu;
	          ostr << " " << mldg.find(583) << cr;
		  break;
	}
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::del_ort( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      pu.upcase();
      switch (tiden.del(pu))
	{
	  case true  : ostr << mldg.find(582) << " " << pu;
	               ostr << " " << mldg.find(552) << cr;
		       break;
	  case false : ostr << mldg.find(582) << " " << pu;
	               ostr << " " << mldg.find(554) << cr;
		       break;
	}
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::set_harmonics( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    if(tiden.set_harmonics(pu))
      ostr << mldg.find(584) << cr;
    else
      ostr << mldg.find(585) << cr;
  else
    ostr << mldg.find(556) << cr;
}

void user_interface::gezeiten( istream &cmd, ostream &ostr )
{
  subcommands subcmd;
  cmd >> subcmd;
  if (subcmd == c_no_subcmd )
    tiden.show(ostr,mldg,cr);
  else if (usr.is_sysop())
    {
      switch(subcmd)
	{
	  case c_sub_add       : add_ort(cmd,ostr);
	                         break;
	  case c_sub_del       : del_ort(cmd,ostr);
	                         break;
	  case c_sub_harmonics : set_harmonics(cmd,ostr);
	                         break;
	  case c_sub_enable    : tiden.status(true);
	                         break;
	  case c_sub_disable   : tiden.status(false); 
	                         break;
	  default              : ostr << mldg.find(587);
                                 ostr << ' ';
				 ostr << mldg.find(542) << cr;
	}
    }
  else
    ostr << mldg.find(588) << cr;
}
#endif

#ifdef COMPILE_SAT

void user_interface::add_sat( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      pu.upcase();
      switch (satelliten.add(pu))
	{
	case 0:
	  {
		    	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(551) << cr;
			    break;
	  }
	case 1:
	  {
	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(553) << cr;
	    break;
	  }
	case 2:
	  {
	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(555) << cr;
	    break;
	  }
	case 3:
	  {
	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(565) << cr;
	    break;
	  }
	}
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::del_sat( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      pu.upcase();
      switch (satelliten.del(pu))
	{
	case 0:
	  {
	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(552) << cr;
	    break;
	  }
	case 1:
	  {
	    ostr << mldg.find(550) << " " << pu << " " << mldg.find(554) << cr;
	    break;
	  }
	}
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::set_kepler( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      if(satelliten.kepler(pu)==0)
	{
	  ostr << mldg.find(558) << cr;
	}
      else
	{
	  ostr << mldg.find(557) << cr;
	}
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::set_sat_status( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,79,' ');
  if (pu!="")
    {
      pu.upcase();
      if(satelliten.slot(pu))
	ostr << mldg.find(567) << " " << satstatus(satelliten.get_status()) << cr;
      else
	ostr << mldg.find(568) << cr;
    }
  else
    {
      ostr << mldg.find(556) << cr;
    }
}

void user_interface::sat( istream &cmd, ostream &ostr )
{
  struct stat attribut;
  
  subcommands subcmd;
  cmd >> subcmd;
  if (subcmd == c_no_subcmd )
    {
      try
	{
	  if (stat(strtochar(configuration.find("KEPLER")), &attribut)==-1)
	    {
	      logf.eintrag("Fehler beim Einlesen der Attribute von kepler.txt",LOGMASK_PRGRMERR);
	      ostr << mldg.find(544) << " " << configuration.find("KEPLER");
	    }
	  else
	    {
	      configuration.find("SAT");	//dummy, damit nichts schief geht
	      ostr << mldg.find(545) << cr;
	      ostr << mldg.find(546) << cr;
	      ostr << mldg.find(547) << " ";
	      ostr << satstatus(satelliten.get_status()) << cr;
	      ostr << mldg.find(559) << " ";
	      ostr << configuration.find("KEPLER") << cr;
	      ostr << mldg.find(548) << " ";
	      ostr << zeit(attribut.st_mtime) << cr;
	      ostr << mldg.find(549) << " ";
	      ostr << satelliten.get_sat_list() << cr;
    	    }
	}
      catch(Error_parameter_not_defined)
	{
	  logf.eintrag("Parameter KEPLER oder SAT nicht definiert",LOGMASK_PRGRMERR);
	}
    }
  else if (usr.is_sysop())
    {
      switch(subcmd)
	{
	  case c_sub_add    : add_sat(cmd,ostr);
	                      break;
	  case c_sub_del    : del_sat(cmd,ostr);
	                      break;
	  case c_sub_kepler : set_kepler(cmd,ostr);
	                      break;
	  case c_sub_status : set_sat_status(cmd,ostr);
	                      break;
	  default           : ostr << mldg.find(541) << ' ';
	                      ostr << mldg.find(542) << cr;
	}
    }
  else
    ostr << mldg.find(560) << cr;
}

String user_interface::satstatus(String Status)
{
    if (Status=="LOCK")
	return ("feste Slots");
    if (Status=="ROTATE")
	return ("rotierende Slots");
    if (Status=="DISABLE")
	return ("Disabled");
	return ("unbekannt");
}
#endif

void user_interface::wall( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String msg;

  msg.get(cmd,200);
  msg.kuerze();

  talk.send_msg(usr.user_call(),msg);

  ostr << mldg.find(561) << cr;
}

void user_interface::talk_to( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  String pu;
  pu.get(cmd,200,' ');
  pu.kuerze();

  try
    {
      callsign call(pu);

      String msg;
      msg.get(cmd,200);
      msg.kuerze();

      if (msg.slen() > 0 && msg != String (' '))
	{
	  if (talk.send_msg(usr.user_call(),call,msg))
	    ostr << mldg.find(562) << ' ' << call << cr;
	  else
	    ostr << call << ' ' << mldg.find(563) << cr;
	}
      else
	{
	  ostr << mldg.find(564) << cr;
	  state = wait_conv_text;
	  conv_partner = call;
	}
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(215) << cr;
    }
}

void user_interface::conv_modus(  istream &cmd, ostream &ostr )
{
  String msg;
  msg.get(cmd,1000);
  msg.kuerze();
  if (msg == "/q" || msg == "/Q")
    state = wait_cmd;
  else
    if (!talk.send_msg(usr.user_call(),conv_partner,msg))
      {
	ostr << conv_partner << ' ' << mldg.find(563) << cr;
	state = wait_cmd;
      }
}


void user_interface::trace( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      cut_blanks(cmd);
      String pu;
      pu.get(cmd,200,' ');
      pu.kuerze();
      if (pu=="")
      {
          ostr << mldg.find(595) << cr;
	  return;
      }

      int tid = pu.Stoi();


      if ( tid != thread_id )
	
	if (tracer.new_trace( tid ) )
	  {
	    ostr << mldg.find(569) << cr;
	    state = trace_mode;
	  }
	else
	  {
	    ostr << mldg.find(570) << cr;
	    ostr << mldg.find(571) << cr;
	  }
      else
	ostr << mldg.find(590) << cr;
    }  
  else
    {
      ostr << mldg.find(572) << cr;
      logf.eintrag(usr.user_call(),"Versuch zu tracen ohne Sysop-Privilegien",LOGMASK_PRIVVERL);
    }
}

void user_interface::trace_modus( istream &cmd, ostream &ostr )
{
  String msg;
  msg.get(cmd,1000);
  msg.kuerze();
  if (msg == "/q" || msg == "/Q")
    {
      state = wait_cmd;
      tracer.stop_trace();
    }
  else
    {
      while (tracer.line_avalable())
	ostr << tracer.get_line() << cr;

      if (!tracer.trace_activ())
	{
	  state = wait_cmd;
	  tracer.stop_trace();
	}
    }
}

void user_interface::trace_modus( ostream &ostr )
{
  while (tracer.line_avalable())
    ostr << tracer.get_line() << cr;
  
  if (!tracer.trace_activ())
    {
      state = wait_cmd;
      tracer.stop_trace();
    }
}

void user_interface::show_destins( vector<struct destin_info> infos, bool all,
				   ostream &ostr)
{
  if (all)
    {
      ostr << mldg.find(593) << cr;
      vector<callsign> nbs = a_router.get_nachbarcalls();

      ostr << "                                     ";
      for (vector<callsign>::iterator it = nbs.begin(); it != nbs.end(); ++it)
	{
	  it->set_format(true);
	  it->set_nossid(true);
	  ostr << *it << "   ";
	}
      ostr << cr;
    }
  for ( vector<struct destin_info>::iterator it2 = infos.begin(); it2 != infos.end(); ++it2)
    {
      try
	{
	  String d_string = it2->zielgebiet.get_string();
	  String fill("                         ");
	  delta_t dt;
	  if (d_string.slen() < 25)
	    {
	      ostr << fill.copy(0,25-d_string.slen() );
	    }
	  ostr << d_string << "  ";

	  if (it2->min_delay != -1.)
	    {
	      dt = delta_t(it2->min_delay);
	      ostr << dt << "  ";
	    }
	  else
	    ostr << " -- --    ";
	  
	  if (all)
	    {    
	      for (vector<double>::iterator it3 = it2->delay.begin() ; it3 != it2->delay.end() ; ++it3 )
		{
		  if (*it3 != -1.)
		    {
		      dt = delta_t(*it3);
		      ostr << dt << " ";
		    }
		  else
		    ostr << " -- --   ";
		}
	    }
	  else
	    {
	      ostr << it2->min_delay_call;
	    }
	  ostr << cr;
	}
      catch( Error_destin_checksum_error )
	{
	  throw Error_routing_tab_corrupted();
	}
    }
}

void user_interface::destinations( istream &cmd, ostream &ostr )
{
  String puffer;
  bool all_flag = false;
  vector<struct destin_info> infos;

  cut_blanks(cmd);
  puffer.getline(cmd,999,ende);
  puffer.kuerze();
  if (puffer.slen() > 0)
    {
      if ( puffer == '*' )
	{
	  all_flag = true;
	  cut_blanks(cmd);
	  puffer.getline(cmd,999,ende);
	  puffer.kuerze();
	}
    }
  if (puffer.slen() > 0)
    {
      try
	{
	  destin ds(puffer);
	  infos = a_router.get_infos(ds);
	}
      catch( Error_no_destin )
	{
	  ostr << mldg.find(592) << cr;
	}
    }
  else
    infos = a_router.get_infos();
  show_destins(infos,all_flag,ostr);
}

void user_interface::crontab( istream &cmd, ostream &ostr )
{
/*
ToDo ......
    crontab ctab(configuration);
    cut_blanks(cmd);
    String pu;
    pu.get(cmd,200,' ');
    pu.kuerze();
    if (pu=="")
    {
      ostr << ctab << cr;
    }
    else if (usr.is_sysop())
    {
      ostr << "Aendern der Crontab muss noch implementiert werden...." << cr;
    }
    else
    {
//        ostr << mldg.find(572) << cr;
      logf.eintrag(usr.user_call(),"Versuch Crontab zu aendern ohne Sysop-Privilegien",LOGMASK_PRIVVERL);  
    }
*/    
}
