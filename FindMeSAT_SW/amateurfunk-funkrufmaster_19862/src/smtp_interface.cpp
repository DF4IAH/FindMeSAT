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

#include "smtp_interface.h"

#include "globdef.h"
#include "board.h"
#include "config.h"
#include "logfile.h"
#include "callsign.h"
#include "database.h"
#include "gruppe.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif


extern config_file configuration;
extern callsign_database calls;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

smtp_interface::smtp_interface( String &outp, bool ax25flag ) : interfaces(outp,ax25flag)
{
  try
    {
      server_adress = configuration.find("SMTP_SERVER_ADRESSE");
      state = wait_cmd;
      interface_id = 'M';
      path_finished = true;
      outp = "220-"+server_adress+" SMTP Funkruf-Master SMTP-Server Version ";
      outp.append(String(VERSION)+cr);
      outp.append(String("220 Achtung, ich spreche deutsch.")+cr);
      helo_done = false;
      mail_done = false;
      rcpt_done = false;
      single_adress_flag = false;
      group_name_flag = false;
      board_name_flag = false;
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(configuration);
      logf.eintrag("SMTP Server Adresse ist nicht definiert.",LOGMASK_PRGRMERR);
      throw Error_could_not_initialize_smtp_interface();
    }
}

smtp_interface::~smtp_interface( void )
{
}


bool smtp_interface::do_process( bool rx_flag, String &outp )
{
  bool quit;
  outp = "";
  if (rx_flag)
    {
      String inp;
      while (get_line(inp))
	state_maschine(inp,outp,quit);
    }
  else 
    quit = false;
  return !quit;
}

void smtp_interface::state_maschine( const String &inp, String &outp, bool &quit )
{
  switch(state)
    {
    case wait_cmd        : do_command(inp,outp,quit);
                           break;
    case wait_data       : input_data(inp,outp);
                           quit = false;
                           break;
    }
}

void smtp_interface::do_command( const String &inp, String &outp, bool &quit )
{
  String cmd;
  String rest;

  quit = false;
  int lp = inp.pos(' ');
  unsigned int l = inp.slen();

  if (lp > 0)
    {
      cmd = inp.copy(0,lp);
      if (l > (unsigned int) (lp+1) )
	rest = inp.copy(lp+1,l-lp-1);
      else
	rest = "";
    }
  else
    {
      cmd = inp;
      rest = "";
    }

  cmd.upcase();
  if (cmd == "HELO")
    do_helo(rest,outp);
  else if (cmd == "MAIL")
    do_mail(rest,outp);
  else if (cmd == "RCPT")
    do_rcpt(rest,outp);
  else if (cmd == "DATA")
    do_data(rest,outp);
  else if (cmd == "RSET")
    do_reset( rest,outp );
  else if (cmd == "NOOP")
    do_nop(rest,outp);
  else if (cmd == "QUIT")
    do_quit(rest,outp,quit);
  else if (cmd == "VRFY")
    do_verify(rest,outp);
  else
    outp.append(String("500 Unbekanntes Kommando")+cr);
}


void smtp_interface::do_helo( const String &inp, String &outp )
{
 
  String puffer = cut_blanks(inp);

  outp.append(String("250 ")+server_adress+" Hallo "+puffer+", schoen Sie zu treffen."+cr);
  helo_done = true;
}

void smtp_interface::do_mail( const String &inp, String &outp )
{
  if (!helo_done)
    outp.append(String("503 Sie haben sich noch nicht vorgestellt.")+cr);
  else
    {
      istringstream cmd(strtochar(inp));

      cut_blanks(cmd);
      char puffer[250];
      cmd.get(puffer,249,':');
      
      int length = strlen(puffer);
      for (int i = 0; i < length; i++ )
	puffer[i] = toupper(puffer[i]);

      if (strcmp(puffer,"FROM") == 0)
	{
	  char ch;
	  cmd >> ch;
	  if (ch == ':')
	    {
	      cut_blanks(cmd);
	      cmd.get(puffer,249);
	      String absender_str(puffer);
	      if (check_absender(absender_str,absender))
		{
		  outp.append(String("250 Ok")+cr);
		  mail_done = true;
		  single_adress_flag = false;
		  group_name_flag = false;
		  board_name_flag = false;
		}
	      else
		outp.append(String("550 Kein Funkamateur in Absender erkannt.")+cr);
	    }
	  else
	    outp.append(String("501 Ich haette hier einen Doppelpunkt erwartet.")+cr);
	}
      else
	outp.append(String("501 Ich haette hier 'from' erwartet.")+cr);
    }
}

void smtp_interface::do_rcpt( const String &inp, String &outp )
{
  if (!helo_done)
    outp.append(String("503 Sie haben sich noch nicht vorgestellt.")+cr);
  else if (!mail_done)
    outp.append(String("503 Bitte erst das 'MAIL' Kommando.")+cr);
  else if (rcpt_done)
    outp.append(String("552 Kann leider nur jeweils eine Zieladresse annehmen.")+cr);
  else
    {
      istringstream cmd(strtochar(inp));

      cut_blanks(cmd);
      char puffer[250];
      cmd.get(puffer,249,':');
      
      int length = strlen(puffer);
      for (int i = 0; i < length; i++ )
	puffer[i] = toupper(puffer[i]);

      if (strcmp(puffer,"TO") == 0)
	{
	  char ch;
	  cmd >> ch;
	  if (ch == ':')
	    {
	      cut_blanks(cmd);
	      cmd.get(puffer,249,'@');
	      String username(puffer);
	      username.upcase();
	      if(username[0]=='<') 	      
	      {
	        unsigned int i;
		for(i=0; i<username.slen()-1; i++)
			username[i]=username[i+1];
		username[i]='\0';
	      }
	      cmd >> ch;
	      if (ch == '@')
		{
		  cmd.get(puffer,249);
		  String adresse(puffer);
		  cut_adress(adresse);
		  adresse.lowcase();
		  server_adress.lowcase();
		  if (adresse == server_adress)
		    {
		      try
			{
			  callsign call(username);
			  database_entry eintrag = calls.find(call);
			  single_adress = eintrag.get_adr();
			  single_adress_flag = true;
			  rcpt_done = true;
			  outp.append(String("250 Empfaenger OK.")+cr);
			}
		      catch( Error_callsign_does_not_exist )
			{
			  outp.append(String("550 Der Benutzer ist nicht bekannt.")+cr);
			}
		      catch( Error_no_callsign )
			{
			  try
			    {
			      gruppen grps(configuration);
			      gruppe grp = grps.find(username);
			      group_name = username;
			      group_name_flag = true;
			      rcpt_done = true;
			      outp.append(String("250 Empfaenger OK.")+cr);
			    }
			  catch( Error_group_does_not_exist )
			    {
			      try
				{
				  board brd(username,configuration);
				  board_name = username;
				  board_name_flag = true;
				  rcpt_done = true;
				  outp.append(String("250 Empfaenger OK.")+cr);
				}
			      catch( Error_could_not_open_boardfile )
				{
				  outp.append(String("550 Keine Maillingliste oder Rubrik mit diesem Namen bekannt.")+cr);
				}
			    }
			}
		    }
		  else
		    outp.append(String("550 Kann nur Mails fuer lokalen Server ")+server_adress+" bearbeiten."+cr);
		}
	      else
		outp.append(String("553 Erwarte '@' in Mailboxnamen.")+cr);
	    }
	  else
	    outp.append(String("501 Erwarte Doppelpunkt hinter to.")+cr);
	}
      else
	outp.append(String("501 Erwarte 'TO'.")+cr);
    }
}

void smtp_interface::do_data( const String &inp, String &outp )
{
  if (!helo_done)
    outp.append(String("503 Sie haben sich noch nicht vorgestellt.")+cr);
  else if (!mail_done)
    outp.append(String("503 Sie haben noch kein 'MAIL' Kommando angegeben.")+cr);
  else if (!rcpt_done)
    outp.append(String("503 Sie haben noch keinen Empfaenger mit 'RCPT TO' angegeben.")+cr);
  else
    {
      state = wait_data;
      outp.append(String("354-Geben Sie den Text ein.")+cr);
      outp.append(String("354 Schliessen Sie den Text mit einem Punkt '.' ab.")+cr);
      data_body = false;
      message = String("");
    }
}

void smtp_interface::do_reset( const String &inp, String &outp )
{
  helo_done = false;
  mail_done = false;
  rcpt_done = false;
  single_adress_flag = false;
  group_name_flag = false;
  board_name_flag = false;
  outp.append(String("250 Mail Dienst zurueck gesetzt.")+cr);
}

void smtp_interface::do_nop( const String &cmd, String &outp )
{
  outp.append(String("250 OK")+cr);
}

void smtp_interface::do_quit( const String &cmd, String &outp, bool &quit )
{
  quit = true;
  outp.append("221-"+server_adress+" beendet die Verbindung."+cr);
  outp.append(String("221 Ich freue mich auf ein Wiedersehen!")+ cr);
}

void smtp_interface::do_verify( const String &inp, String &outp )
{
  char puffer[250];
  char ch;
  istringstream cmd(strtochar(inp));

  cmd.get(puffer,249,'@');
  String username(puffer);
  cmd >> ch;
  if (ch == '@')
    {
      cmd.get(puffer,249);
      String adresse(puffer);
      if (adresse == server_adress)
	{
	  try
	    {
	      callsign call(username);
	      database_entry eintrag = calls.find(call);
	      rcpt_done = true;
	      outp.append("250 ");
	      outp.append(eintrag.get_name());
	      outp.append(" <"+username+"@"+server_adress+">"+cr);
	    }
	  catch( Error_callsign_does_not_exist )
	    {
	      outp.append(String("550 Der Benutzer ist nicht bekannt.")+cr);
	    }
	  catch( Error_no_callsign )
	    {
	      outp.append(String("550 Der Benutzer ist nicht bekannt.")+cr);
	    }
	}
      else
	outp.append(String("550 Nur Benutzer von lokalen Server bekannt.")+cr);
    }
  else
    outp.append(String("501 Adresse muss '@' enthalten.")+cr);
}

void smtp_interface::input_data( const String &inp, String &outp )
{
  if (!data_body)
    {
      if (!inp.in(':'))
	if (inp.slen() > 0)
	  if (inp[0] != ' ')
	    data_body = true;
    }

  if (inp.slen() > 0)
    if (inp[0] != '.')
      {
	if (message.slen() < 80)
	  message.append(inp);
	
	if (message.slen() > 80)
	  message = message.copy(0,80);

      }
    else
      {
	if (send_message())
	  outp.append(String("250 Nachricht wird ausgesendet.")+cr);
	else 
	  outp.append(String("451 Konnte Nachricht nicht aussenden.")+cr);
	state = wait_cmd;
      }
}

bool smtp_interface::send_message( void )
{
  if (single_adress_flag)
    {
      destin ds;
      ds = get_default_destin();
      return page(single_adress,message,ds);
    }
  else if (group_name_flag)
    return page_group();
  else if (board_name_flag)
    return page_board();
  else
    return false;
}

bool smtp_interface::page( const adress &adr, const String &msg, const destin &ds )
{
  bool flag1 = false, flag2 = false;

#ifdef COMPILE_SLAVES
  try
    {
      flag1 = spool.spool_msg(false,absender,G_mycall,zeit(),adr,msg,false,ds,64);
#endif
      flag2 = fwd.fwd_msg(absender,adr,ds,64,msg);
      return flag1 || flag2;
#ifdef COMPILE_SLAVES
    }
  // Exceptions abfangen
  catch ( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
      return false;
    }
#endif
}

bool smtp_interface::page_group( void )
{
  bool flag = false;

  try
    {
      gruppen grps(configuration);
      gruppe grp = grps.find(group_name);

      destin ds;
      ds = get_default_destin();

      try
	{
	  callsign call = grp.get_first();
	  database_entry eintrag = calls.find(call);
	  adress adr = eintrag.get_adr();
	  
	  flag = page(adr,message,ds );

	  while (flag)
	    {
	      call = grp.get_next();
	      eintrag = calls.find(call);
	      adress adr = eintrag.get_adr();
	      flag = page(adr,message,ds );
	    }
	}
      catch( Error_no_more_call_in_group )
	{
	}
      catch( Error_callsign_does_not_exist )
	{
	}
    }
  catch( Error_group_does_not_exist )
    {
      flag = false;
    }
  catch( Error_could_not_open_groupfile )
    {
      flag = false;
    }

  return flag;
}


bool smtp_interface::page_board( void )
{
  bool flag = false;

  try
    {
      board brd(board_name,configuration);

      board::permissions perm = brd.get_permission(absender);

      if (perm != board::perm_no)
	{
	  destin ds;
	  ds = get_default_destin();
	  
#ifdef COMPILE_SLAVES
	  try
	    {
	      
	      int board = brd.get_brd_id();
#endif
	      int slot = brd.get_slot();
	      //bul.msg = msg;
	      brd.set_msg(message,slot,ds);
	      
#ifdef COMPILE_SLAVES
	      flag = spool.spool_bul(G_mycall,zeit(),board,slot,message,false,ds,128);
#endif
	      if (perm == board::perm_forw)
		fwd.fwd_bul(absender,board_name,-1,ds,128,14,message);
#ifdef COMPILE_SLAVES
	    }
	  catch(Error_could_not_open_file )
	    {
	      flag = false;	  
	      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
	    }
#endif
	}
    }
  catch( Error_could_not_open_boardfile )
    {
      flag = false;
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
      flag = false;
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Boarddatei zu speichern.",LOGMASK_PRGRMERR);
      flag = false;
    }

  return flag;
}

void smtp_interface::cut_adress( String &adr )
{
  unsigned int l = adr.slen();
  for (unsigned int i = 0 ; i < l ; i++)
    if (!isalnum(adr[i]) && adr[i] != '.' && adr[i] != '-' )
      {
	adr = adr.copy(0,i);
	return;
      }
}

bool smtp_interface::check_absender( const String &adr, callsign &call)
{
  unsigned int l = adr.slen();
  unsigned int at = 0;

  for (unsigned int i = 0 ; i < l ; i++)
    if (adr[i] == '@')
      at = i;

  if (at != 0)
    {
      String first_part;
      String second_part;
      if(adr[0]=='<') 
           first_part = adr.copy(1,at-1);
      else
           first_part = adr.copy(0,at);        
      second_part = adr.copy(at+1,l-at-1);
      second_part.lowcase();

      bool fp_flag = check_first_part(first_part,call);
      return check_second_part(second_part,fp_flag,call);
    }
  else
    return false;
} 

bool smtp_interface::check_first_part(const String& f_part, callsign &call )
{
  try
    {
      callsign c(f_part);
      call = c;
      return true;
    }
  catch( Error_no_callsign )
    {
      return false;
    }
}

bool smtp_interface::check_second_part( const String& s_part, bool got_callsign, callsign &call )
{
  if (!got_callsign)
    {
      unsigned int li = 0;
      unsigned int pi = 0;
      unsigned int l = s_part.slen();
      bool callsign_erkannt = false;
      for (unsigned int i = 0; i < l && !callsign_erkannt; i++ )
	if (s_part[i] == '.')
	  {
	    pi = i;
	    String part = s_part.copy(li+1,pi-li-1);
	    li = pi;
	    try
	      {
		callsign c(part);
		call = c;
		callsign_erkannt = true;
	      }
	    catch( Error_no_callsign )
	      {
		// callsign_erkannt = false;
	      }
	  }
	else if (i == l - 1)
	  {
	    pi = i;
	    String part = s_part.copy(li+1,pi-li);
	    li = pi;
	    try
	      {
		callsign c(part);
		call = c;
		callsign_erkannt = true;
	      }
	    catch( Error_no_callsign )
	      {
		// callsign_erkannt = false;
	      }
	  }
      if (!callsign_erkannt)
	return false;
    }
  return s_part.in("ampr.org");
}

bool is_ax25_smtp_sender( const callsign &call )
{
  try
    {
      String all_sender = configuration.find("AX_25_SMTP_SENDER");
      vector<String> sender = komma_separeted(all_sender);
      for (vector<String>::iterator it = sender.begin() ; it != sender.end() ; ++it )
	{
	  try
	    {
	      callsign ca(*it);
	      if (ca == call)
		  return true;
	    }
	  catch( Error_no_callsign )
	    {
	      syslog slog(configuration);
	      slog.eintrag("Parameter AX_25_SMTP_SENDER enthaelt ungueltiges Rufzeichen",LOGMASK_PRGRMERR);
	    }
	}
    }
  catch( Error_parameter_not_defined )
    {
      syslog slog(configuration);
      slog.eintrag("Parameter AX_25_SMTP_SENDER nicht definiert",LOGMASK_PRGRMMDG);
    }
  return false;
}
