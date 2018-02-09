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

#include "slaves.h"

extern slave_control slaves;
extern config_file configuration;

void user_interface::show_slave( const slave &sl, ostream &ostr )
{
  ostr << " ";
  callsign call = sl.get_call();
  call.set_format(true);
  call.set_nossid(true);
  ostr << call << " :  ";

  String stat_string;
  switch (sl.status())
    {
      case slave::sts_disabled            : stat_string = mldg.find(773);
                                            break;
      case slave::sts_spool               : stat_string = mldg.find(774);
                                            break;
      case slave::sts_getrennt            : stat_string = mldg.find(775);
                                            break;
      case slave::sts_aufbau              : stat_string = mldg.find(776);
                                            break;
      case slave::sts_aktiv               : stat_string = mldg.find(777);
	                                    break;
      case slave::sts_trennen_uz_disablen :
      case slave::sts_trennen_uz_spoolen  : stat_string = mldg.find(778);
                                            break;
      case slave::sts_disablen            :
      case slave::sts_spoolen             : stat_string = mldg.find(779);
                                            break;
      case slave::sts_gescheitert         : stat_string = mldg.find(780);
    }
  stat_string.append("           ");
  ostr << stat_string.copy(0,10);
  delta_t dt(zeit() - sl.last_change());
  ostr << dt << "  ";
  String mode_string;
  switch( sl.get_mode() )
    {
      case slave::m_passiv : mode_string = mldg.find(781);
                             break;
      case slave::m_activ  : mode_string = mldg.find(782);
                             break;
    }
  mode_string.append("           ");
  ostr << mode_string.copy(0,8);
  ostr << setw(16) << sl.get_slots();

  ostr << "  ";
  ostr << sl.get_version();

}

void user_interface::show_slave_extended( const slave &sl, ostream &ostr )
{
  callsign call = sl.get_call();
  call.set_format(true);
  call.set_nossid(false);
  ostr << mldg.find(783) << ' ' << call << cr;
  ostr << mldg.find(784) << ' ';

  switch (sl.status())
    {
      case slave::sts_disabled            : ostr << mldg.find(773);
                                            break;
      case slave::sts_spool               : ostr << mldg.find(774);
                                            break;
      case slave::sts_getrennt            : ostr << mldg.find(775);
                                            break;
      case slave::sts_aufbau              : ostr << mldg.find(776);
                                            break;
      case slave::sts_aktiv               : ostr << mldg.find(777);
	                                    break;
      case slave::sts_trennen_uz_disablen :
      case slave::sts_trennen_uz_spoolen  : ostr << mldg.find(778);
                                            break;
      case slave::sts_disablen            :
      case slave::sts_spoolen             : ostr << mldg.find(779);
                                            break;
      case slave::sts_gescheitert         : ostr << mldg.find(780);
    }
  ostr << cr;
  ostr << mldg.find(785) << ' ';
  delta_t dt(zeit() - sl.last_change());
  ostr << dt << cr;
  ostr << mldg.find(786) << ' ';
  switch( sl.get_mode() )
    {
      case slave::m_passiv : ostr << mldg.find(781);
                             break;
      case slave::m_activ  : ostr << mldg.find(782);
                             break;
    }
  ostr << cr;
  ostr << mldg.find(787) << ' ' << sl.get_bake() << cr;
  ostr << mldg.find(788) << ' ' << sl.get_slots() << cr;
  ostr << mldg.find(796) << ' ' << sl.get_version() << cr;
  ostr << mldg.find(797) << ' ';
  dt = delta_t(sl.get_diff());
  ostr << dt << cr;
  ostr << mldg.find(789) << ' ' << sl.get_connect_pfad() << cr;
  ostr << mldg.find(790) << ' ';
  if (sl.get_passwd().slen() > 0)
    ostr << mldg.find(791) << " (" << sl.get_passwd().slen() << ")" << cr;
  else
    ostr << mldg.find(792) << cr;
  ostr << mldg.find(793) << ' ' << sl.get_destinations() << cr;
  ostr << cr;
}

void user_interface::show_slave_list( vector<slave> sl, ostream &ostr )
{
  for (vector<slave>::iterator it = sl.begin(); it != sl.end(); ++it )
    {
      show_slave(*it,ostr);
      ostr << cr;
    }
}

void user_interface::show_slaves( istream &cmd, ostream &ostr )
{
  cut_blanks(cmd);
  char puffer[20];
  cmd.get(puffer,19,' ');
  String pu(puffer);
  if (pu == String(""))
    {
      ostr << mldg.find(700) << cr;
      ostr << mldg.find(701) << cr;
      vector<slave> sl = slaves.get_slave();
      show_slave_list(sl,ostr);
    }
  else
    {
      try
	{
	  callsign call(pu);
	  vector<slave> sl = slaves.get_slave(call);
	  vector<slave>::iterator it = sl.begin();
	  if (it != sl.end())
	    show_slave_extended(*it,ostr);
	  else
	    {
	      ostr << mldg.find(794) << ' ' << call;
	      ostr  << ' ' << mldg.find(795) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << pu << mldg.find(702) << cr;
	} 
    }
}

void user_interface::setslave_enable( const callsign &csgn , ostream &ostr )
{
  if (slaves.enable_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(704) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(705) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_reset( const callsign &csgn , ostream &ostr )
{
  if (slaves.reset_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(771) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(772) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_disable(  const callsign &csgn , ostream &ostr )
{
  if (slaves.disable_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(707) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(708) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_spool( const callsign &csgn , ostream &ostr )
{
  if (slaves.spool_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(709) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(710) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_activ( const callsign &csgn , ostream &ostr )
{
  if (slaves.activ_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(711) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(712) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_passiv( const callsign &csgn , ostream &ostr )
{
  if (slaves.passiv_slave(csgn))
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(713) << cr;
    }
  else
    {
      ostr << mldg.find(703) << ' ' << csgn << ' ';
      ostr << mldg.find(714) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_slots( const callsign &csgn, istream &cmd, ostream &ostr )
{
  char puffer[200];
  String pu;
  cut_blanks(cmd);
  cmd.get(puffer,19,' ');
  pu = String(puffer);
  if (slaves.set_slots(csgn,pu))
    {
      ostr << mldg.find(715) << cr;
      ostr << mldg.find(716) << cr;
    }
  else
    {
      ostr << mldg.find(717) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_bake( const callsign &csgn, istream &cmd, ostream &ostr )
{
  char puffer[200];
  String pu;

  cut_blanks(cmd);
  cmd.get(puffer,79);
  pu = String(puffer);
  if (slaves.set_bake(csgn,pu))
    {
      ostr << mldg.find(718) << cr;
      ostr << mldg.find(716) << cr;
    }
  else
    {
      ostr << mldg.find(719) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_pfad( const callsign &csgn, istream &cmd, ostream &ostr )
{
  char puffer[200];
  String pu;

  try
    {

      cut_blanks(cmd);
      cmd.get(puffer,199,' ');
      pu = String(puffer);
      connect_string cs(pu);
      if (slaves.set_con_pfad(csgn,cs))
	{
	  ostr << mldg.find(720) << cr;
	  ostr << mldg.find(716) << cr;
	}
      else
	{
	  ostr << mldg.find(721) << cr;
	  ostr << mldg.find(706) << cr;
	}
    }
  catch( Error_syntax_fehler_in_connect_string )
    {
      ostr << mldg.find(722) << ' ' << cr;
      ostr << pu << ' ' << cr;
      ostr << mldg.find(723) << cr;
      ostr << mldg.find(724) << cr;
      ostr << mldg.find(721) << cr;
    }
}

void user_interface::setslave_passwd( const callsign &csgn, istream &cmd, ostream &ostr )
{
  char puffer[200];
  String pu;
  cut_blanks(cmd);
  cmd.get(puffer,79);
  pu = String(puffer);
  if (slaves.set_passwd(csgn,pu))
    {
      ostr << mldg.find(725) << cr;
      ostr << mldg.find(716) << cr;
    }
  else
    {
      ostr << mldg.find(726) << cr;
      ostr << mldg.find(706) << cr;
    }
}

void user_interface::setslave_destin( const callsign &csgn, istream &cmd, ostream &ostr )
{
  try
    {
      char puffer[200];
      String pu;
      cut_blanks(cmd);
      cmd.get(puffer,79);
      pu = String(puffer);
      own_destins dests(pu);
      if (slaves.set_destinations(csgn,dests))
	{
	  ostr << mldg.find(727) << cr;
	  ostr << mldg.find(716) << cr;
	}
      else
	{
	  ostr << mldg.find(728) << cr;
	  ostr << mldg.find(706) << cr;
	}
    }
  catch( Error_no_destin )
    {
      ostr << mldg.find(729) << cr;
    }
}

void user_interface::get_callsign( istream &cmd, callsign &call )
{
  String pu;

  cut_blanks(cmd);
  pu.get(cmd,20,' ');  
  call = callsign(pu);
}

void user_interface::setslave(istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      try
	{
	  callsign csgn;
	  
	  subcommands subcmd;
	  cmd >> subcmd;

	  get_callsign(cmd,csgn);

	  switch(subcmd)
	    {
	      case c_sub_enable         : setslave_enable(csgn,ostr);
	                                break;
	      case c_sub_disable        : setslave_disable(csgn,ostr);
	                                break;
	      case c_sub_reset          : setslave_reset(csgn,ostr);
	                                break;
	      case c_sub_spool          : setslave_spool(csgn,ostr);
	                                break;
	      case c_sub_aktiv          : setslave_activ(csgn,ostr);
	                                break;
	      case c_sub_passiv         : setslave_passiv(csgn,ostr);
	                                break;
	      case c_sub_slots          : setslave_slots(csgn,cmd,ostr);
	                                break;
	      case c_sub_baken          : setslave_bake(csgn,cmd,ostr);
	                                break;
	      case c_sub_pfad           : setslave_pfad(csgn,cmd,ostr);
	                                break;
	      case c_sub_passwort       : setslave_passwd(csgn,cmd,ostr);
	                                break;
	      case c_sub_destinations : setslave_destin(csgn,cmd,ostr);
	                                break;
	      case c_no_subcmd        :	ostr << mldg.find(732) << cr;
	                                break;
	      default                 : ostr << mldg.find(730) << ' ';
		                        ostr << mldg.find(731) << cr;
		                        break;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      ostr << mldg.find(733) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Zugriffsversuch auf Slavesteuerung",LOGMASK_PRIVVERL);
    }
}

void user_interface::add_slave( istream& cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char puffer[200];
      String pu;
      try
	{
	  cut_blanks(cmd);
	  cmd.get(puffer,19,' ');
	  pu = String(puffer);
	  callsign csgn(pu);
	  // wir setzen einen dummy-connect-pfad. Ansonsten Kracht es, wenn die
	  // config gelesen wird und der Pfad noch leer ist..
	  connect_string cs= configuration.find("AX25_PORT")+":"+configuration.find("MYCALL")+">"+pu;
	  String bake = String("Funkrufsender ") + csgn.str();
	  String slots;
	  if (slaves.add_slave(csgn,cs,slots,bake))
	    {
	      ostr << mldg.find(734) << cr;
	      ostr << mldg.find(735) << cr;
	      ostr << mldg.find(736) << cr;
	    }
	  else
	    {
	      ostr << mldg.find(737) << cr;
	    } 
	}
      catch( Error_no_callsign )
	{
	  ostr <<  mldg.find(215) << cr;
	}
    }
  else
    {
      ostr <<  mldg.find(738) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Slave einzurichten",LOGMASK_PRIVVERL);
    }
}
	
void user_interface::del_slave( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char puffer[200];
      String pu;
      try
	{
	  cut_blanks(cmd);
	  cmd.get(puffer,19,' ');
	  pu = String(puffer);
	  callsign csgn(pu);

	  if (slaves.del_slave(csgn))
	    {
	      ostr << mldg.find(739) << cr;
	      ostr << mldg.find(740) << cr;
	      ostr << mldg.find(741) << cr;
	    }
	  else
	    {
	      ostr << mldg.find(742) << cr;
	      ostr << mldg.find(743) << cr;
	    } 
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      ostr << mldg.find(744) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Slave zu loeschen",LOGMASK_PRIVVERL);
    }
}  

void user_interface::add_bake( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char puffer[200];
      String pu;
      try
	{
	  cut_blanks(cmd);
	  cmd.get(puffer,19,' ');
	  pu = String(puffer);
	  callsign csgn(pu);

	  cut_blanks(cmd);
	  cmd.get(puffer,19);
	  pu = String(puffer);

	  baken_gen_call = csgn;
	  baken_gen_slot = pu.Stoi();

	  if (slaves.check_pocbake(baken_gen_call,baken_gen_slot))
	    {
	      ostr << mldg.find(760) << cr;
	      ostr << mldg.find(761) << cr;
	    }
	  else
	    {
	      state = wait_baken_text;
	      ostr << mldg.find(745) << cr;
	      ostr << mldg.find(746) << cr;
	      ostr << mldg.find(747) << cr;
	      ostr << mldg.find(748) << cr;
	      ostr << mldg.find(749) << cr;
	      ostr << mldg.find(750) << cr;
	    }
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      ostr << mldg.find(751) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Bake einzurichten",LOGMASK_PRIVVERL);
    }

}

void user_interface::add_baken_text( istream &cmd, ostream &ostr )
{
  String text;
  text.getline(cmd,90,ende);

  if (slaves.add_pocbake(baken_gen_call,text,baken_gen_slot,*baken))
    ostr << mldg.find(752) << cr;
  else
    ostr << mldg.find(753) << cr;

  state = wait_cmd;
}

void user_interface::del_bake( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char puffer[200];
      String pu;
      try
	{
	  cut_blanks(cmd);
	  cmd.get(puffer,19,' ');
	  pu = String(puffer);
	  callsign csgn(pu);

	  cut_blanks(cmd);
	  cmd.get(puffer,19);
	  pu = String(puffer);
	  int slot = pu.Stoi();

	  if (slaves.del_pocbake(csgn,slot,*baken))
	    ostr << mldg.find(758) << cr;
	  else
	    ostr << mldg.find(759) << cr;
	}
      catch( Error_no_callsign )
	{
	  ostr << mldg.find(215) << cr;
	}
    }
  else
    {
      ostr << mldg.find(751) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Bake zu loeschen",LOGMASK_PRIVVERL);
    }
}

void user_interface::save_slaves( ostream &ostr )
{
  if (usr.is_sysop())
    {
      slaves.save();
      ostr << mldg.find(755) << cr;
    }
  else
    {
      ostr << mldg.find(754) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Versuch, Slaves zu speichern",LOGMASK_PRIVVERL);
    }
}
