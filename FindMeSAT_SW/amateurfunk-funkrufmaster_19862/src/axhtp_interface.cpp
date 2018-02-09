/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; you can redistribute ist and/or modify    *
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
 *                                                                          *
 ****************************************************************************/

#include "axhtp_interface.h"
#include "config.h"

extern config_file configuration;



void axhtp_interface::do_tx_auth( String &out )
{
  //cerr << "m2" << endl;
  // Hier muss noch wirklich was zur Authentifizierung eingebaut werden.
  out.append(cr);
  state = ahs_wait_for_auth_answer;
}

bool axhtp_interface::do_rx_auth( void )
{
  //cerr << "m3" << endl;
  String auth_line;

  if ( get_line( auth_line ) )
    {
      // Auch dies muiss noch implementiert werden.
      state = ahs_auth_respond;
      return true;
    }
  else
    return false;
}

void axhtp_interface::do_auth_respond( String & out )
{
  //cerr << "m4" << endl;
  out.append(cr);
  state = ahs_wait_cmd;
}

void axhtp_interface::do_error_respond( String &out )
{
  String tx_content;
  mime_type o_m_typ;
  int clength;
  zeit date;

  //cerr << "m5" << endl;
  htmgen.do_error_msg(status,tx_content,o_m_typ,clength,date);

  String header_line = itoS(status) + String(':');
  header_line.append(itoS(clength)+String(','));
  switch(o_m_typ)
    {
      case mime_none : header_line.append("unknown,");
	break;
    case mime_jpg  : header_line.append("image/jpeg,");
      break;
    case mime_gif  : header_line.append("image/gif,");
      break;
    case mime_bin  : header_line.append("application/x-binary,");
      break;
    case mime_wav  : header_line.append("audio/wav,");
      break;
    case mime_txt  : header_line.append("text/plain,");
      break;
    case mime_html : header_line.append("text/html,");
      break;
    }
  header_line.append( date.get_unix_zeit_string() );
  header_line.append(cr);
  
  out.append(header_line);
  
  out.append(tx_content);
  
  state = ahs_wait_cmd;
}

void axhtp_interface::do_respond( String &out )
{
  //cerr << "m6" << endl;
  String tx_content;
  mime_type o_m_typ;
  int clength;
  zeit date;

  url.kuerze();
  status = htmgen.do_url(url,tx_content,o_m_typ,clength,date,*baken);

  if (status == 200)
    {
      String header_line = "200:";
      header_line.append(itoS(clength)+String(','));
      switch(o_m_typ)
	{
	  case mime_none : header_line.append("unknown,");
	                   break;
	  case mime_jpg  : header_line.append("image/jpeg,");
	                   break;
	  case mime_gif  : header_line.append("image/gif,");
	                   break;
	  case mime_bin  : header_line.append("application/x-binary,");
	                   break;
	  case mime_wav  : header_line.append("audio/wav,");
	                   break;
	  case mime_txt  : header_line.append("text/plain,");
	                   break;
	  case mime_html : header_line.append("text/html,");
	                   break;
	}
      header_line.append( date.get_unix_zeit_string() );
      header_line.append(cr);

      out.append(header_line);

      if (cmd == 'G')
	out.append(tx_content);

      state = ahs_wait_cmd;
    }
  else
    state = ahs_send_error;
}


bool axhtp_interface::read_file( void )
{
  //cerr << "m7" << endl;
  int ipuflen = input_puffer.slen();
  if (ipuflen > 0)
    {
      if (ipuflen < in_bytes_left )
	{
	  i_puffer.append(input_puffer);
	  input_puffer = "";
	  in_bytes_left -= ipuflen;
	}
      else if (ipuflen == in_bytes_left )
	{
	  i_puffer.append(input_puffer);
	  input_puffer = "";
	  in_bytes_left -= ipuflen;
	  state = ahs_respond;
	}
      else
	{
	  String tmp_puffer = input_puffer.copy(0,in_bytes_left);
	  i_puffer.append(tmp_puffer);
	  in_bytes_left = 0;
	  state = ahs_respond;
	}
      return true;
    }
  else
    return false;
}

void axhtp_interface::analyse_command( bool &quit_flag )
{
  //cerr << "m8" << endl;

  if (cmd_line.slen() > 1)
    {
      if (cmd_line[1] == ':')
	{
	  cmd = cmd_line[0];
	  if ( cmd != 'G' && cmd != 'H' && cmd != 'A' && cmd != 'Q' )
	    status = 400;

	  if (cmd == 'Q')
	    quit_flag = true;

	  else if (cmd == 'A')
	    state = ahs_send_auth_question;

	  else 
	    {
	      int begin,index;
	      int cmd_line_len = (int) cmd_line.slen();
	      String len_str,typ_str;
	      
	      begin = 2;
	      index = 2;
	      
	      while (index < cmd_line_len )
		{
		  if (cmd_line[index] == ',')
		    {
		      len_str = cmd_line.copy(begin,index-begin);
		      begin = index+1;
		      index = begin;
		    }
		  else
		    index++;
		}
	      
	      while (index < cmd_line_len)
		{
		  if (cmd_line[index] == ',')
		    {
		      typ_str = cmd_line.copy(begin,index-begin);
		      begin = index+1;
		      index = begin;
		    }
		  else
		    index++;
		}
	      
	      if (begin < cmd_line_len )
		url = cmd_line.copy(begin,cmd_line.slen()-begin);
	      else
		status = 400;

	      i_content_length = len_str.Stoi();
	      if (typ_str == "text/html")
		inp_m_typ = mime_html;
	      else if (typ_str == "image/jpeg")
		inp_m_typ = mime_jpg;
	      else if (typ_str == "image/gif")
		inp_m_typ = mime_gif;
	      else if (typ_str == "application/x-binary")
		inp_m_typ = mime_bin;
	      else if (typ_str == "audio/wav")
		inp_m_typ = mime_wav;
	      else if (typ_str == "text/plain")
		inp_m_typ = mime_txt;
	      else 
		inp_m_typ = mime_none;
      

	      if (i_content_length > 0)
		{
		  state = ahs_read_file;
		  in_bytes_left = i_content_length;
		  i_puffer = "";
		  return;
		}
	      else
		state = ahs_respond;
	      
	    }
	}
    }

  if (status != 200 )
    state = ahs_send_error;
}

void axhtp_interface::do_command( void  )
{
  //cerr << "m9" << endl;
  status = 200;
  if ( get_line( cmd_line ) )
    state = ahs_analyse_command;
  else
    state = ahs_no_command_available;
}

void axhtp_interface::state_machine( String & outp, bool &quit )
{
  //cerr << "m10" << endl;
  bool loop_flag = true;
  quit = false;
  while (loop_flag && !quit)
    {
      switch(state)
	{
	  case ahs_wait_cmd             : do_command();
	                                  break;
	  case ahs_no_command_available : state = ahs_wait_cmd;
	                                  loop_flag = false;
	                                  break;
	  case ahs_analyse_command      : analyse_command(quit);
	                                  break;
	  case ahs_read_file            : loop_flag = read_file();
	                                  break;
	  case ahs_respond              : do_respond(outp);
	                                  break;
	  case ahs_send_error           : do_error_respond(outp);
	                                  break;
	  case ahs_send_auth_question   : do_tx_auth(outp);
	                                  break;
	  case ahs_wait_for_auth_answer : loop_flag = do_rx_auth();
	                                  break;
	  case ahs_auth_respond         : do_auth_respond(outp);
	                                  break;
	}
    }
}


axhtp_interface::axhtp_interface( String &outp, t_baken& bk, const user &u ) : interfaces(outp,true), cpw(false), htmgen(configuration)
{
  //cerr << "m11" << endl;
  usr = u;
  htmgen.set_user(usr);
  outp = String("[AXHTP - 1.0]") + cr;
  baken = &bk;
  interface_id = 'H';
  gegenstation = usr.user_call();
  state = ahs_wait_cmd;
  // Connect ging von der Gegenseite aus, daher :
  path_finished = true;
}


axhtp_interface::~axhtp_interface( void )
{
}

bool axhtp_interface::do_process(bool rx_flag, String&outp )
{
  //cerr << "m12" << endl;
  bool quit = false;
  outp = "";

  if (rx_flag)
    {
      last_akt = zeit();
      state_machine(outp,quit);
    }
  else
    if (zeit() - last_akt > DEFAULT_AXHTP_TIMEOUT )
      quit = true; 

  return !quit;
}
