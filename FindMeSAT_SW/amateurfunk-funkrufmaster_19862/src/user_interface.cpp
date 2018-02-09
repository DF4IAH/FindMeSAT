/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2000,2001,2002,2003,2004 by Holger Flemming                *
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

#include <string.h>
#include <ctype.h>

#include "autoconf.h"
#include "user_interface.h"
#include "user_interface_cmds.h"
#include "globdef.h"
#include "config.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "logfile.h"
#include "fwd_router.h"
#include "talk.h"

extern config_file configuration;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif
extern fwd_router router;
extern talker talk;
extern callsign_database calls;

extern callsign G_mycall;

istream& operator>>( istream & strm, commands &cmd )
{
  char ch = ' ';
  char puffer[100];

  while ( strm.get(ch) && ( ch == ' ' )) ;
  strm.putback(ch);

  strm.get(puffer,99,' ');
  int length = strlen(puffer);

  if ( length < 1 )
    cmd = c_no_cmd;
  else
    {
      for (int i = 0; i < length; i++ )
	puffer[i] = toupper(puffer[i]);
      
      cmd = c_unk_cmd;

      int cmd_index = 0;

      while ( user_commands[cmd_index].name[0] != '\0' && cmd == c_unk_cmd)
	{
	  if ( strncmp(puffer,user_commands[cmd_index].name,length) == 0 )
	    cmd = user_commands[cmd_index].cmd;
	  cmd_index++;
	}
    }
  return strm;
}

istream& operator>>( istream & strm, subcommands &cmd )
{
  char ch = ' ';
  char puffer[100];

  while ( strm.get(ch) && ( ch == ' ' )) ;
  strm.putback(ch);

  strm.get(puffer,99,' ');
  int length = strlen(puffer);

  if ( length < 1 )
    cmd = c_no_subcmd;
  else
    {
      for (int i = 0; i < length; i++ )
	puffer[i] = toupper(puffer[i]);
      
      cmd = c_unk_subcmd;

      int cmd_index = 0;

      while ( user_subcommands[cmd_index].name[0] != '\0' && cmd == c_unk_subcmd)
	{
	  if ( strncmp(puffer,user_subcommands[cmd_index].name,length) == 0 )
	    cmd = user_subcommands[cmd_index].cmd;
	  cmd_index++;
	}
    }
  return strm;
}

/*****************************************************************************/
/* Kommando ausfuehren                                                       */
/*                                                                           */
void user_interface::do_command(const String& inp, String &outp, bool &quit)
{
  ostringstream ostr;
  istringstream cmd(strtochar(inp));

  quit = false;
  commands c ;
  // ersten Buchstaben des Kommandos
  cmd >> c;
  switch(c)
    {
      case c_no_cmd    : ostr << mldg.find(1) << cr << flush;
	                 break;
      case c_unk_cmd   : ostr << mldg.find(2) << cr << flush;
	                 break;
      case c_aktuell   : {
                           atext at(configuration,macros,usr.language,cr); 
			   ostr << at; // Aktuelltext abfragen
                         }
	                 break;  
      case c_activate  : activate(cmd,ostr);
                         break;
      case c_groups    : show_groups(cmd,ostr);
                         break;
      case c_groupname : groupname(cmd,ostr);
                         break;
      case c_gpage     : page_group(cmd,ostr);
                         break;
      case c_help      : help(cmd,ostr); // Hilfstext abfragen
                         break;
      case c_info      : {
	                   itext it(configuration,macros,usr.language,cr); 
      		           ostr << it; // Infotext abfragen
                         }
	                 break;
      case c_locator   : set_loc(cmd,ostr); // Locator eintragen
                         break;
      case c_name      : set_name(cmd,ostr); // Name eintragen
                         break;
      case c_lang      : set_lang(cmd,ostr); // Sprache setzen
                         break;
      case c_newcall   : newcall(cmd,ostr); // Neues Call setzen
                         break;			 
      case c_page      : page(cmd,ostr); // Funkruf absenden
                         break;
      case c_rubrik    : rubrik(cmd,ostr); // Rubrik beschreiben
                         break;
      case c_dir       : show_dir(cmd,ostr); // Verzeichnis aller Rubriken
                         break;
      case c_mkrub     : make_dir(cmd,ostr);
                         break;
      case c_delrub    : delete_dir(cmd,ostr);
                         break;
      case c_setdir    : set_dir(cmd,ostr);
                         break;
      case c_sysop     : if (sysop_socket)
    			 {
			   usr.sysop = 1;
			   logf.eintrag(usr.user_call(),"Passwortabfrage erfolgreich (sysop socket)", LOGMASK_PWLOG);
			 }
			 else
    			 {
	                   String zahlen;
			   usr.auth1(configuration,zahlen);
			   ostr << zahlen << cr; 
			   // Benutzer will Sysop werden
			   state = wait_passwd;
                         }
		         break;
      case c_typ       : set_typ(cmd,ostr);
                         break;
      case c_user      : show_user(cmd,ostr); 
	                 // Liste aller Rufzeichen in der Datenbank ausgeben
                         break;
      case c_adduser   : add_user(cmd,ostr); // Benutzer hinzufuegen
                         break;
      case c_deluser   : del_user(cmd,ostr); // Benutzer aus Datenbank loeschen
                         break;
      case c_addgrp    : add_group(cmd,ostr);
                         break;
      case c_delgrp    : del_group(cmd,ostr);
                         break;
      case c_addusrgrp : add_user_to_group(cmd,ostr);
                         break;
      case c_delusrgrp : del_user_from_group(cmd,ostr);
                         break;
      case c_quit      : state = quit_state;  // Session beenden
	                 quit_state_entry = zeit();
	                 ostr << "73 es awdh!" << cr;
                         break;
      case c_shutdown  : do_shutdown(cmd,ostr); // Server herunter fahren
                         break;
      case c_dbsave    : dbsave(ostr);
                         break;
      case c_version   : version(cmd,ostr);
                         break;
      case c_zeit      : zeit_datum(ostr);
                         break;
      case c_send      : send(cmd,ostr);
                         break;
#ifdef COMPILE_SLAVES
      case c_statistik : statistik(ostr);
                         break;
#endif
      case c_cstatus   : 
      case c_who       : cstatus(ostr);
                         break;
      case c_syslog    : slog(cmd,ostr);
                         break;
      case c_log       : ulog(cmd,ostr);
                         break;
      case c_rtext     : rtext(cmd,ostr);
                         break;
      case c_wtext     : wtext_init(cmd,ostr);
                         break;
      case c_param     : param(cmd,ostr);
	                 break;
      case c_filedesc  : descr(ostr);
	                 break;
      case c_destin    : destinations(cmd,ostr);
	                 break;
#ifdef COMPILE_SLAVES
      case c_slaves    : show_slaves(cmd,ostr);
	                 break;
      case c_showspool : spool.show(ostr,cr);
	                 break;
      case c_setslave  : setslave(cmd,ostr);
	                 break;
      case c_addslave  : add_slave(cmd,ostr);
                         break;
      case c_delslave  : del_slave(cmd,ostr);
	                 break;
      case c_addbake   : add_bake(cmd,ostr);
	                 break;
      case c_saveslaves : save_slaves(ostr);
	                 break;
      case c_delbake   : del_bake(cmd,ostr);
	                 break;
#endif
      case c_showbaken : ostr << mldg.find(756) << cr;
                         ostr << mldg.find(757) << cr;
			 show_baken(*baken,ostr,cr);
	                 break;
      case c_fwdstatus : showfwd(cmd,ostr);
	                 break;
      case c_iplog     : ilog(cmd,ostr);
	                 break;
      case c_setfwd    : setfwd(cmd,ostr);
	                 break;
      case c_addfwd    : addfwd(cmd,ostr);
	                 break;
      case c_delfwd    : delfwd(cmd,ostr);
	                 break;
      case c_savefwd   : savefwd(ostr);
	                 break;
      case c_dbrequest : dbrequest(cmd,ostr);
	                 break;
      case c_dbtrans   : dbtrans(cmd,ostr);
	                 break;
#ifdef COMPILE_SAT
      case c_sat       : sat(cmd,ostr);
	                 break;
#endif
#ifdef COMPILE_TIDE
      case c_gezeiten  : gezeiten(cmd,ostr);
	                 break;
#endif			 
      case c_dxcluster : dxcluster(cmd,ostr);
	                 break;
      case c_talk      : talk_to(cmd,ostr);
	                 break;
      case c_kill      : kill_th(cmd,ostr);
	                 break;
      case c_wall      : wall(cmd,ostr);
	                 break;
#ifdef COMPILE_WX
      case c_wx        : wx_config(cmd,ostr);
	                 break;
#endif
#ifdef COMPILE_DIGISTATUS
      case c_digi      : digi_config(cmd,ostr);
	                 break;
#endif
      case c_hlog      : hlog(cmd,ostr);
	                 break;
      case c_fwdlog    : flog(cmd,ostr);
	                 break;
      case c_rpclog    : rlog(cmd,ostr);
	                 break;
      case c_splog     : splog(cmd,ostr);
	                 break;
      case c_logmask   : logmask(cmd,ostr);
	                 break;
      case c_pagemask  : pagemask(cmd,ostr);
	                 break;
      case c_fwdmask   : fwdlogmask(cmd,ostr);
	                 break;
      case c_rpcmask   : rpclogmask(cmd,ostr);
	                 break;
      case c_splogmask : spoollogmask(cmd,ostr);
	                 break;
      case c_exportmask: exportmask(cmd,ostr);
	                 break;
      case c_trace     : trace(cmd,ostr);
	                 break;
      case c_crontab   : crontab(cmd,ostr);
	                 break;
      case c_connect   : connect(cmd,ostr);
	                 break;
      case c_passwd    : passwd(cmd,ostr);
	                 break;
      case c_relais    : do_relais(cmd,ostr);
	                 break;
      default          : ostr << mldg.find(44) << ' ';
	                 ostr << G_mycall << ' ' << mldg.find(45) << cr;
    } 
  outp.append(ostr);
}

void user_interface::init_interface( String &outp )
{
  macros.set_user(usr);
  gegenstation = usr.user_call();
  talk.add_member(gegenstation);
  try
    {
      try
	{
	  mldg.load(usr.language);
	}
      catch( Error_could_not_open_messagefile )
	{
	  mldg.load("dl");
	  outp.append(mldg.find(239)+' '+usr.language+' '+mldg.find(240)+cr);
	  outp.append(mldg.find(241)+cr);
	  usr.language = "dl";
	}
      // Instanz des Connecttextes anlegen
      ctext ct(configuration,macros,usr.language,cr);
      if (!samecall(usr.user_call(),G_mycall))
	logf.eintrag(usr.user_call(),"Login",LOGMASK_LOGINOUT);
      outp.append("FunkrufMaster                         V ");
      outp.append(String(VERSION)+cr+cr);

      ostringstream ostr;
      ostr << ct << cr;
      outp.append(ostr);

   }
  catch( Error_could_not_open_messagefile )
    {
      logf.eintrag("Sprachdatei dl kann nicht geoeffnet werden.",LOGMASK_PRGRMERR);
    }
  catch( Error_message_file_format_error )
    {
      logf.eintrag("Sprachdatei dl hat falsches Dateiformat.",LOGMASK_PRGRMERR);
    }
}

void user_interface::get_callsign( const String &inp, String &outp )
{
  // Zielrufzeichen aus Eingabe holen
  try
    {
      //cerr << "Warte auf Rufzeichen." << endl;
      //cerr << "Eingabe : " << inp << endl;
      user_id = callsign(inp);
      //ostr << TELNET_IAC << TELNET_DONT << TELNET_OPTION_ECHO;
      //ostr << (char) 133;
      outp = "Password : ";
      //outp.append(String(TELNET_IAC)+String(TELENT_WONT)+String(TELNET_OPTION_ECHO));
      state = wait_password;
    }
  catch( Error_no_callsign )
    {
      outp = "Callsign : ";
    }
}


void user_interface::check_ax25password( const String &inp, String &correct, String &outp, bool &quit )
{
  quit = false;
  if (inp.in(correct))
    {
      state = wait_cmd;
      init_interface(outp);
    }
  else
    {
      outp = String("Access denied!")+cr;
      quit = true;
    }
}


void user_interface::get_password( const String &inp, String &outp, bool &quit )
{
  //ostr << TELNET_IACTELNET_IAC << TELNET_DO << TELNET_OPTION_ECHO;
  //ostr << (char) 131;
  quit = false;
  //outp.append(String(TELNET_IAC)+String(TELNET_DO)+String(TELNET_OPTION_ECHO));
  if (cpw.check_password(user_id,inp))
    {
      usr = user(user_id.call(),false);
      state = wait_cmd;
      init_interface(outp);
    }
  else
    {
      outp = String("Access denied!")+cr;
      if (++passwd_tries > 3)
	quit = true;
      else
	{
	  outp = "Callsign : ";
	  state = wait_callsign;
	}
    }
}


void user_interface::state_maschine( const String &inp, String &outp, bool &quit )
{
  ostringstream ostr;
  istringstream cmd(strtochar(inp));

  switch(state)
    {
      case wait_callsign   : get_callsign(inp,outp);
	                     quit = false;
                             break;
      case wait_ax25pw     : check_ax25password(inp,correct,outp,quit);
                             break;
      case wait_password   : get_password(inp,outp,quit);
                             break;
      case wait_cmd        : do_command(inp,outp,quit);
                             break;
      case wait_passwd     : usr.auth2(configuration,inp);
	                     state = wait_cmd;
			     quit = false;
                             break;
      case wait_text_input : wtext(cmd,ostr);
                             quit = false;
                             break;
      case wait_pagetext   : page_text(cmd,ostr);
                             quit = false;
                             break;
      case wait_gpagetext  : page_grouptext(cmd,ostr);
                             quit = false;
                             break;
      case wait_boardtext  : rubrik_text(cmd,ostr);
                             quit = false;
                             break;
#ifdef COMPILE_SLAVES
      case wait_baken_text : add_baken_text(cmd,ostr);
                             quit = false;
	       		     break;
#endif
      case wait_conv_text  : conv_modus(cmd,ostr);
                             quit = false;
  			     break;
      case trace_mode      : trace_modus(cmd,ostr);
                             quit = false;
  			     break;
      case connect_mode    : con_mode(cmd,ostr);
                             quit = false;
  			     break;
      case passwd_step1    : passwd_st1(cmd,ostr);
                             quit = false;
  			     break;
      case passwd_step2    : passwd_st2(cmd,ostr);
                             quit = false;
  			     break;
      case passwd_step3    : passwd_st3(cmd,ostr);
                             quit = false;
  			     break;
      case quit_state      : if (zeit() - quit_state_entry > 1)
                             quit = true;
                             break;
#ifdef COMPILE_WX
      case add_wx_step1    : add_wx_1(cmd,ostr);
                             quit = false;
                             break;
      case add_wx_step2    : add_wx_2(cmd,ostr);
                             quit = false;
                             break;
#endif
      default              : state = wait_cmd;
                             break;
    }
  outp.append(ostr);
}


user_interface::user_interface( String &outp, user &us, bool ax25_flag , t_baken &bs) : interfaces(outp,ax25_flag), grps(configuration), mldg(configuration), cpw(ax25_flag)
{
  // Zunaechst alle wichtigen Klassen uebernehmen und Zeiger darauf festhalten
  outp = "";
  state = wait_cmd;
  usr = us;
  baken = &bs;
  last_akt = zeit();
  warning = false;
  interface_id = 'U';
  sysop_socket = false; // bei AX25-Verbindungen ist die Sysop-Nachfrage
                        // grundsaetzlich erforderlich!
  path_finished = true; // Der Connect geschieht immer von aussen, daher
                        // keine connects mehr

  try
    {
      timeout = 60 * configuration.find("INFOBOX_TIMEOUT").Stoi();
    }
  catch( Error_parameter_not_defined )
    {
      timeout = 0;
    }

  try
    {
      database_entry ent = calls.find(usr.user_call().get_nossidcall());

      if (ent.get_ax25_pw().slen()>5)
	{
	  state=wait_ax25pw;
	  int pos[5];
	  int l=ent.get_ax25_pw().slen();
	  bool flag;
	  String passwd=ent.get_ax25_pw();
	  outp.append(G_mycall.call()+">");
	  for (int i = 0;i < 5;i++)
	    {
	      do
		{
		  flag = false;
		  pos[i] = rand() % (l);
		  for (int j = 0;j < i; j++)
		    if (pos[j] == pos[i])
		      flag = true;
		}
	      while (flag);
	      // Bei der Ausgabe wird eine eins addiert, um Zahlen zwischen 1 und 80 zu erhalten
	      outp.append(String(' ')+itoS(pos[i]+1));	      
	    }
	  outp.append(String(cr));
	  // In correct werden die fuenf korrekten Zeichen geschrieben.
	  correct="";
	  for (int i = 0;i < 5;i++)
	    correct.append(passwd[pos[i]]);
	}
    }
  catch( Error_callsign_does_not_exist )
    {
    }
  if (state==wait_cmd)
    {
      init_interface(outp);
      if (usr.is_sysop())
	outp.append(usr.user_call().call()+" de "+G_mycall.call()+" *=>");
      else
 	outp.append(usr.user_call().call()+" de "+G_mycall.call()+" =>");
    }
}

user_interface::user_interface( String &outp, uint32_t adr, uint16_t port, bool ax25_flag , t_baken &bs) : interfaces(outp,ax25_flag), grps(configuration), mldg(configuration), cpw(ax25_flag)
{
  // Abfragen, ob und welches User-Passwort verwendet werden soll
  if ( (adr & AX25_IP_MASK) == AX25_IP_ADDRESS )
  {
#ifdef _DEBUG_ELOGS_
cerr << "AX25-Flag fuer PW ist TRUE!" << endl;  
#endif
    cpw.set_ax25_flag();
  }
  else
  {
#ifdef _DEBUG_ELOGS_
cerr << "AX25-Flag fuer PW ist FALSE!" << endl;  
#endif
  }
#ifdef _DEBUG_ELOGS_
cerr << "Flag   : " << adr << endl;
cerr << "Mask   : " << AX25_IP_MASK << endl;
cerr << "Abfrage: " << AX25_IP_ADDRESS << endl;
cerr << "Summe  : " << (adr & AX25_IP_MASK) << endl;
#endif

  // Testen ob die Sysopnachfrage erforderlich ist.
  sysop_socket = (htonl(adr) == INADDR_LOOPBACK && htons(port) < 1024);

  // Zunaechst alle wichtigen Klassen uebernehmen und Zeiger darauf festhalten
  state = wait_callsign;
  baken = &bs;
  last_akt = zeit();
  warning = false;
  // Connect wurde von der Gegenseite aufgebaut, deshalb ist :
  path_finished = true;
  interface_id = 'U';
  try
    {
      timeout = 60 * configuration.find("INFOBOX_TIMEOUT").Stoi();
    }
  catch( Error_parameter_not_defined )
    {
      timeout = 0;
    }
  // dl9sau: if we do not call mldg.load() at least as dummy here,
  //         then there's no language definition available for
  //         message #3 (timeout in <2m) in the phase 'Callsign:' or
  //         'Password'. Unschoen: man kann nicht davon ausgehen, dass
  //         Sprachunterstuetzung fuer 'dl' installiert ist. Eine
  //         Abstraktion fuer 'default' waere wuenschenswert.
  mldg.load("dl");
  outp = "Callsign : ";
}


user_interface::~user_interface( void )
{
  talk.del_member(gegenstation);
  if (usr.is_sysop())
    interface_id = 'S';
  if (!samecall(usr.user_call(),G_mycall))
    logf.eintrag(usr.user_call(),"Logout",LOGMASK_LOGINOUT);
}

bool user_interface::do_process( bool rx_flag, String &outp )
{
  bool quit = false;
  String inp;
  outp = "";

  if (rx_flag)
    {
      //cerr << "do_process fuer user_interface." << endl;
      //cerr << "Status : " << state << endl; 
      last_akt = zeit();
      while (get_line(inp))
	{
	  cerr << '.' << inp << '.' << endl;
	  warning = false;
	  logf.eintrag(usr.user_call(),inp,LOGMASK_USERINPUT);

	  while (talk.msg_avalable(usr.user_call()))
	    outp.append(talk.get_msg(usr.user_call())+cr);

	  if (state == wait_cmd)
	    outp.append(cr);
	  
	  // Kommando ausfuehren
	  state_maschine(inp,outp,quit);
	  if (!quit && state == wait_cmd)
	    if (usr.is_sysop())
	      outp.append(usr.user_call().call()+" de "+G_mycall.call()+" *=>");
	    else
	      outp.append(usr.user_call().call()+" de "+G_mycall.call()+" =>");
	  
	}
    }
  else
    {
      ostringstream ostr;

      if (state == trace_mode)
	{
	  trace_modus(ostr);
	  outp.append(ostr);
	  quit = false;
	}
      else if (state == connect_mode )
	{
	  con_mode(ostr);
	  outp.append(ostr);
	  quit = false;
	}
      else if (state == quit_state )
	{
	  if (zeit() - quit_state_entry > 1)
	    quit = true;
	  else
	    quit = false;
	}
      else
	{
	  while (talk.msg_avalable(usr.user_call()))
	    ostr << talk.get_msg(usr.user_call()) << cr;
	  
	  if (timeout == 0)
	    {
	      outp.append(ostr);
	      quit = false;
	    }
	  else
	    {
	      int diff = zeit() - last_akt;
	      if (!warning && diff + 120 > timeout)
		{
		  ostr << mldg.find(3) << cr;
		  warning = true;
		}
	      if (diff > timeout)
		{
		  delta_t dt(diff);
		  ostr << mldg.find(4) << ' ' << dt << ' ' << mldg.find(5) << cr << cr;
		  outp.append(ostr);
		  quit = true;
		}
	      else
		{
		  outp.append(ostr);
		  quit = false;
		}
	    }
	}
    }
  
  return (!quit);
}    
