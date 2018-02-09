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

#include <ctype.h>
#include "autoconf.h"

#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "config.h"

extern config_file configuration;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif


int user_interface::get_slot( istream &cmd )
{
  char puffer[5];

  cmd.get(puffer[0]);
  if (isdigit(puffer[0]))
    {
      cmd.get(puffer[1]);
      if (isdigit(puffer[1]))
	{
	  cmd.get(puffer[2]);
	  if (puffer[2] == ' ')
	    {
	      puffer[2] = '\0';
	      return atoi(puffer);
	    }
	  else
	    {
	      cmd.putback(puffer[2]);
	      cmd.putback(puffer[1]);
	      cmd.putback(puffer[0]);
	      return -1;
	    }
	}
      else if (puffer[1] == ' ')
	{
	  return (int) puffer[0] - 48;
	}
      else
	{
	  cmd.putback(puffer[1]);
	  cmd.putback(puffer[0]);
	  return -1;
	}
    }
  else
    {
      cmd.putback(puffer[0]);
      return -1;
    }
}

int user_interface::get_lifetime( istream &cmd )
{
  char puffer[5];
  int i=0, z=0;
  int lt=0;
  cmd.get(puffer[0]);
  if (puffer[0]=='#')
    {
      while(puffer[i]!=' ' && i<4)
	{
	  i++;
	  cmd.get(puffer[i]);
	  if (!isdigit(puffer[i]))
	    break;
	}
      i--;
      z=i;
      while(i>0)
	{
#ifdef _DEBUG_ELOGS_
	  cerr << "#**" << i << ":" << puffer[i] << "***" << endl; 
#endif   
	  if (!isdigit(puffer[i])) lt=-9999;
	  lt=lt+(((int) puffer[i]-48)*(int(pow(10., (double) (z-i)))));    
	  i--;
	}
    }
  else
    {
      cmd.putback(puffer[0]);
      lt= -1;
    }
  if (lt < 0) lt=-1;
  return lt;
}

// Beschreiben einer Rubrik

void user_interface::rubrik(istream &cmd, ostream &ostr )
{
  char ch;
  // Leerzeichen abtrennen
  cut_blanks(cmd);
  page_boardname="";
  
  try
    {
      while (cmd.get(ch) && (ch != ' ') && (ch != '@'))
	{
	  page_boardname = page_boardname + (String) ch;
	}
      
      if (page_boardname != "")
	{
	  board page_board(page_boardname,configuration);
	  
	  if (usr.is_sysop())
	    perm = board::perm_forw;
	  else
	    perm = page_board.get_permission(usr.user_call());
	  
	  if (perm == board::perm_no)
	    {
	      ostr << mldg.find(124) << cr;
	      return;
	    }
	  
	  cmd.putback(ch);
	  
	  try
	    {
	      cmd >> page_ds;
	    }
	  catch( Error_no_destin )
	    {
	      page_ds = get_default_destin();
	    }
	  
	  // Leerzeichen abtrennen
	  cut_blanks(cmd);
	  
	  page_slot = get_slot(cmd);
	  
	  if (page_slot != -1)
	    cut_blanks(cmd);
	  
	  page_lifetime = get_lifetime(cmd);
	  
	  if (page_lifetime>page_board.get_deflt()) 
	    page_lifetime=page_board.get_deflt();
	  
	  if (page_lifetime != -1)
	    cut_blanks(cmd);
	  
	  String msg;
	  cmd >> msg;
	  
	  if (msg.slen()<=0)
	  {
            ostr << mldg.find(605) << " " << page_boardname << " " << mldg.find(606) << cr;
	    state=wait_boardtext;
	    return;
	  }
	  
#ifdef COMPILE_SLAVES
	  try
	    {
	      int board = page_board.get_brd_id();
#endif
	      int slot;
	      if (page_slot == -1)
		slot = page_board.get_slot();
	      else
		slot = page_slot;
	      
	      page_board.set_msg(msg,slot,page_ds,page_lifetime);
	      
	      bool flag1 = false,flag2 = false;
	      
#ifdef COMPILE_SLAVES
	      flag1 = spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,page_ds,128);
#endif
	      in_msg++;
	      if (perm == board::perm_forw)
		flag2 = fwd.fwd_bul(usr.user_call(),page_boardname,page_slot,page_ds,128,page_lifetime,msg);
	      
	      if ( flag1 || flag2 ) 
		ostr << mldg.find(100) << cr ;
	      else
		ostr << mldg.find(101) << cr << flush;
#ifdef COMPILE_SLAVES
	    }
	  catch(Error_could_not_open_file )
	    {
	      logf.eintrag(usr.user_call(),"Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
	    }
#endif
	}
      else
	{
	  ostr << mldg.find(118) << cr;
	}
    }
  catch( Error_could_not_open_boardfile )
    {
      ostr << mldg.find(102) << ' ' << page_boardname << ' ' << mldg.find(103) << cr << flush;
      ostr << mldg.find(104) << cr << flush;
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag(usr.user_call(),"Nicht moeglich, Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}


void user_interface::rubrik_text(istream &cmd, ostream &ostr )
{
    String msg;
    cmd >> msg;

    if (msg.slen()<=0)
    {
        ostr << mldg.find(603) << cr;
        state=wait_cmd;
	return;
    }
	  
#ifdef COMPILE_SLAVES
    try
    {
	board page_board(page_boardname,configuration);
	int board = page_board.get_brd_id();
#endif
	int slot;
	if (page_slot == -1)
	    slot = page_board.get_slot();
	else
	    slot = page_slot;
	      
	page_board.set_msg(msg,slot,page_ds,page_lifetime);
	      
	bool flag1 = false,flag2 = false;
	      
#ifdef COMPILE_SLAVES
	flag1 = spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,page_ds,128);
#endif
	in_msg++;
	if (perm == board::perm_forw)
	    flag2 = fwd.fwd_bul(usr.user_call(),page_boardname,page_slot,page_ds,128,page_lifetime,msg);
	      
	if ( flag1 || flag2 ) 
	    ostr << mldg.find(100) << cr ;
	else
	    ostr << mldg.find(101) << cr << flush;
#ifdef COMPILE_SLAVES
	state=wait_cmd;
    }
    catch(Error_could_not_open_file )
    {
	logf.eintrag(usr.user_call(),"Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
#endif
    catch( Error_could_not_open_boardfile )
    {
      ostr << mldg.find(102) << ' ' << page_boardname << ' ' << mldg.find(103) << cr << flush;
      ostr << mldg.find(104) << cr << flush;
    }
    catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
    catch( Error_could_not_create_boardfile )
    {
      logf.eintrag(usr.user_call(),"Nicht moeglich, Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}


void user_interface::dir_all( ostream &ostr, const boards &brds )
{
  ostr << mldg.find(132) << cr;
  ostr << mldg.find(133) << cr;
  vector<String> names = brds.get_board_names();
  int cnt = 0;
  for (vector<String>::iterator it = names.begin(); it != names.end(); ++it)
    {
      board brd(*it,configuration);
      ostr << setw(3);
      ostr.setf(ios::right, ios::adjustfield);
      ostr << brd.get_brd_id() << " : ";
      ostr << setw(13);
      ostr.setf(ios::left, ios::adjustfield);
      ostr << brd.get_name();
      ostr.setf(ios::right, ios::adjustfield);
      zeit newest=brd.get_newest_time();
      if (newest>9)
	{
	  delta_t last_msg(zeit() - newest);
	  ostr << "(" << last_msg << ")         ";
	}
      else
	ostr << "( --:--  )         ";

      if (cnt++ == 1)
	{
	  ostr << cr;
	  cnt = 0;
	}
    }
    if (cnt == 1)	// bei ungerader Rubrikenzahl in die neue Zeile
	ostr << cr;
}

void user_interface::dir_board( ostream &ostr, board brd )
{
  ostr << mldg.find(134) << ' ' << brd.get_name() << cr;
  ostr << mldg.find(135) << " " << brd.get_brd_id() << cr;
  ostr << mldg.find(136) << " " << brd.get_deflt() << cr;
      
  if (brd.get_tx_intv() == 0)
    ostr << mldg.find(137) << cr;
  else
    {
      delta_t dt(brd.get_tx_intv());
      ostr << mldg.find(138) << ' ' << dt;
      zeit last_tx = brd.get_last_tx();
      last_tx.set_darstellung(zeit::f_zeit_s);
      ostr << mldg.find(139) << " " << last_tx << cr;
    }

  
  if (usr.is_sysop())
    {
      bool flag = false;
      vector<struct board_sysop> sysops = brd.get_sysops();
      for (vector<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it)
	{
	  if (flag)
	    ostr << ", ";
	  
	  if (it->wildcard)
	    ostr << "*";
	  else 
	    ostr << it->call;
	  
	  if (it->local)
	    ostr << " " << mldg.find(140);
	  else
	    ostr << " " << mldg.find(141);
	  
	  flag = true;
	}
      
      ostr << cr;
    }
  
  for (int i = 1 ; i <= 10 ; i++)
    {
      ostr << mldg.find(142)<< ' ' << setw(2) << i;
      ostr << ' ' << mldg.find(143) << ' ';
      zeit t = brd.get_time(i);
      t.set_darstellung(zeit::f_zeitdatum_s);
      ostr << t << ' ' << mldg.find(144) << ' ';
      t = t + brd.get_lifetime(i) * 86400;
      t.set_darstellung(zeit::f_datum_s);
      try
	{
	  ostr << t << ' ' << mldg.find(145) << ' ' << brd.get_destin(i) << cr;
	}
      catch( Error_destin_checksum_error )
	{
	  ostr << t << ' ' << mldg.find(145) << ' ' << mldg.find(146) << cr;
	}
      ostr << brd.get_msg(i) << cr;
      if (brd.get_msg(i).slen() > 0 && brd.get_msg(i).slen() < 80 )
        ostr << cr;
    }
}

void user_interface::show_dir( istream &cmd, ostream &ostr )
{
  try
    {
      boards brds;
      
      char ch;
      // Leerzeichen abtrennen
      cut_blanks(cmd);
      String boardname;
      try
	{
	  while (cmd.get(ch) && (ch != ' '))
	    boardname = boardname + (String) ch;

	  if (boardname != String("") && boardname != String("*"))
	    {
	      board brd(boardname,configuration);
	      dir_board(ostr,brd);
	    }
	  else
	    dir_all(ostr,brds);
	}
      catch( Error_could_not_open_boardfile )
	{
	  ostr << mldg.find(102) << ' ' << boardname << ' ' << mldg.find(103) << cr;
	}      
    }
  catch( Error_could_not_create_boardnames )
    {
      ostr << mldg.find(105) << cr;
    }
}
	   

void user_interface::make_dir(istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char ch;
      // Leerzeichen abtrennen
      cut_blanks(cmd);
      String boardname;
      try
	{
	  while (cmd.get(ch) && (ch != ' '))
	    boardname = boardname + (String) ch;

	  board test_brd(boardname,configuration);
	  ostr << mldg.find(102) << ' ' << boardname << ' ' << mldg.find(106) << cr;
	}
      catch( Error_could_not_open_boardfile )
	{
	  cut_blanks(cmd);
	  
	  int code;
	  cmd >> code;
	  if (code >= 100 || (code >= 65 && code < 90))
	    {
	      try
		{
		  board brd(boardname,configuration,true,code);
		  ostr << mldg.find(102) << ' ' << boardname << ' ' << mldg.find(107) << cr;
		}
	      catch( Error_could_not_create_boardfile )
		{
		  ostr << mldg.find(108) << cr;
		  logf.eintrag(usr.user_call(),"Konnte Board-Datei nicht anlegen",LOGMASK_PRGRMERR);
		}
	    }
	  else
	    ostr << mldg.find(109) << cr;
	}
      catch( Error_boarddirectory_not_defined )
	{
	  ostr << mldg.find(110) << cr;
	  logf.eintrag(usr.user_call(),"Parameter RUBRIKEN nicht definiert",LOGMASK_PRGRMERR);
	}
      catch( Error_could_not_create_boardfile )
	{
	  ostr << mldg.find(108) << cr;
	  logf.eintrag(usr.user_call(),"Konnte Board-Datei nicht anlegen",LOGMASK_PRGRMERR);
	}
    }
  else
    {
      ostr << mldg.find(111) << cr;
      logf.eintrag(usr.user_call(),"Versuch, Rubrik anzulegen ohne Sysop-Privilegien.",LOGMASK_PRIVVERL);
    }
}

void user_interface::delete_dir( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      char ch;
      // Leerzeichen abtrennen
      cut_blanks(cmd);
      String boardname;
      try
	{
	  while (cmd.get(ch) && (ch != ' '))
	    boardname = boardname + (String) ch;

	  board brd(boardname,configuration);

	  if (brd.get_brd_id() >= 100 ||
              ( brd.get_brd_id() >= 65 && brd.get_brd_id() < 90))
	    {
	      brd.delete_brd();
	      ostr << mldg.find(102) << ' ' << boardname << ' ' << mldg.find(112) << cr;
	    }
	  else
	    ostr << mldg.find(113) << cr;
	}
      catch( Error_could_not_open_boardfile )
	{
	  ostr << mldg.find(112) << ' ' << boardname << ' ' << mldg.find(114) << cr;
	}
      catch( Error_boarddirectory_not_defined )
	{
	  ostr << mldg.find(110) << cr;
	  logf.eintrag(usr.user_call(),"Parameter RUBRIKEN nicht definiert",LOGMASK_PRGRMERR);
	}
    }
  else
    {
      ostr << mldg.find(115) << cr;
      logf.eintrag(usr.user_call(),"Versuch, Rubrik zu loeschen ohne Sysop-Privilegien.",LOGMASK_PRIVVERL);
    }
}

void user_interface::set_tx_intervall( istream &cmd, ostream &ostr )
{
  char puffer[50];

  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  pu.kuerze();

  if (pu == String(""))
    {
      ostr << mldg.find(130) << cr;
      return;
    }
  board brd(pu,configuration);

  cut_blanks(cmd);
  pu.get(cmd,20);
  pu.kuerze();

  if (pu == String(""))
    {
      ostr << mldg.find(131) << cr;
      return;
    }

  int txi = 60 * pu.Stoi();

  brd.set_intv(txi);
}


void user_interface::setdefaultlt(istream &cmd,ostream &ostr)
{
  char puffer[50];

  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  if (pu==String(""))
  {
    ostr << mldg.find(126) << cr;
    return;
  }
  board brd(pu,configuration);
  
  cut_blanks(cmd);
  pu.get(cmd,20);
  
  int lt = pu.Stoi();
  if (lt >= 0 && lt <=999)
    {
      brd.set_def_lt(lt); 
      ostr << mldg.find(120) << cr;
    }
  else
    ostr << mldg.find(125) << cr;
}

void user_interface::setlt(istream &cmd,ostream &ostr)
{
  char puffer[50];
  int nr=0; int lt=0;
  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  if (pu==String(""))
  {
    ostr << mldg.find(126) << cr;
    return;
  }
  board brd(pu,configuration);

  cut_blanks(cmd);
  pu.get(cmd,20,' ');
  pu.kuerze();
  if (pu!=String(""))
  {
    nr=pu.Stoi();
    if (nr>0 && nr <=10)
    {
      cut_blanks(cmd);
      pu.get(cmd,20,' ');
      pu.kuerze();
      if (pu!=String(""))
      {
	lt=pu.Stoi();
	if (lt >= 0 && lt <=999)
	{
    	    brd.set_lt(nr, lt); 
    	    ostr << mldg.find(128) << cr;
	}
        else
        {
	    ostr << mldg.find(125) << cr;
        }
      }
      else
      {
        ostr << mldg.find(126) << cr;
      }
     }
     else
     {
        ostr << mldg.find(127) << cr;
     }
  }
  else
  {
    ostr << mldg.find(127) << cr;
  }  
}

void user_interface::add_board_op(istream &cmd,ostream &ostr)
{
  char puffer[50];

  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  if (pu==String(""))
  {
    ostr << mldg.find(126) << cr;
    return;
  }
  board brd(pu,configuration);
  
  cut_blanks(cmd);
  try
    {
      pu.get(cmd,20,' ');
      pu.kuerze();
      callsign call;
      bool wildcard;
      if (pu == "*")
	wildcard = true;
      else
	{
	  call = callsign(pu);
	  wildcard = false;
	}
      cut_blanks(cmd);
      pu.get(cmd,20);
      pu.kuerze();
      pu.upcase();

      bool local = !(pu == "FORWARD");

      brd.add_sysop(call,local,wildcard);
      ostr << mldg.find(121) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(122) << cr;
    }
}

void user_interface::del_board_op(istream &cmd,ostream &ostr)
{
  char puffer[50];

  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  if (pu==String(""))
  {
    ostr << mldg.find(126) << cr;
    return;
  }
  board brd(pu,configuration);
  
  cut_blanks(cmd);
  try
    {
      pu.get(cmd,20,' ');
      pu.kuerze();
      callsign call;
      bool wildcard;
      if (pu == "*")
	wildcard = true;
      else
	{
	  call = callsign(pu);
	  wildcard = false;
	}

      brd.del_sysop(call,wildcard);
      ostr << mldg.find(123) << cr;
    }
  catch( Error_no_callsign )
    {
      ostr << mldg.find(122) << cr;
    }
}

void user_interface::del_board_msg(istream &cmd,ostream &ostr)
{
  char puffer[50];
  int nr=0;

  cut_blanks(cmd);
  cmd.get(puffer,49,' ');
  String pu(puffer);
  pu.lowcase();
  if (pu==String(""))
  {
    ostr << mldg.find(126) << cr;
    return;
  }

  board brd(pu,configuration);
  
  cut_blanks(cmd);
  pu.get(cmd,20,' ');
  pu.kuerze();
  if (pu!=String(""))
  {
    nr=pu.Stoi();
    if (nr>0 && nr <=10)
    {
      brd.del_msg(nr);
      ostr << mldg.find(129) << cr;
    }
    else
    {
      ostr << mldg.find(127) << cr;
    }
  }
  else
    ostr << mldg.find(126) << cr;
}

void user_interface::set_dir( istream &cmd, ostream &ostr )
{
  if (usr.is_sysop())
    {
      String pu;
      try
	{

	  subcommands subcmd;
	  cmd >> subcmd;

	  switch(subcmd)
	    {
	      case c_sub_lifetime   : setlt(cmd,ostr);
	                              break;
	      case c_sub_deflifetime : setdefaultlt(cmd,ostr);
	                              break;
	      case c_sub_addboardop : add_board_op(cmd,ostr);
	                              break;
	      case c_sub_del        : del_board_msg(cmd,ostr);
	    			      break;
	      case c_sub_delboardop : del_board_op(cmd,ostr);
	                              break;
	      case c_sub_tx_wiederh : set_tx_intervall(cmd,ostr);
	                              break;
	      case c_unk_subcmd     : ostr << mldg.find(116) << ' ';
//		                      ostr << cmd << ' ';
	                              ostr << mldg.find(117) << cr;
				      break;
	      default               : ostr << mldg.find(118) << cr;
	    }
	}
      catch( Error_could_not_open_boardfile )
	{
	  ostr << mldg.find(102) << ' ' << mldg.find(103) << cr;
	  ostr << mldg.find(104) << cr;
	}
    }
  else
    {
      ostr << mldg.find(119) << cr;
      logf.eintrag(usr.user_call(),"Nichtprivilegierter Zugriffsversuch auf Boardeigenschaften",LOGMASK_PRIVVERL);
    }
}
