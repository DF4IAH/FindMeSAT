/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2001-2004 by Holger Flemming                               *
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
 *  		                                			    *
 ****************************************************************************/


#include "thread.h"
#include "system_info.h"
#include "logfile.h"
#include "config.h"
#include "trace.h"
#include "iputil.h"

int thread::last_id = 0;

extern config_file configuration;
extern trace tracer;

thread::thread()
{
  id = 0;
  discon = false;
  wait_for_connect = false;
  tcp_ax25_flag = false;
  cli_sock = NULL;
  intf = NULL;
  input_puffer = "";
  output_puffer = "";
  typ = t_nodef;
  startzeit = zeit();
  cpu_zeiten.sys_time = 0.;
  cpu_zeiten.user_time = 0.;
  in_bytes = 0;
  out_bytes = 0;
}

thread::thread(client &cl, interfaces &i, types tp, bool fl )
{
  id = get_next_id();
  cli_sock = &cl;
  intf = &i;
  intf->set_thread_id(id);
  input_puffer = "";
  output_puffer = "";
  tcp_ax25_flag = cli_sock->get_socket_type() != SOCK_AX25;
  in_out_coming_flag = fl;
  if (tcp_ax25_flag)
    end_of_line = (char) 10;
  else 
    end_of_line = (char) 13;
  typ = tp;
  startzeit = zeit();
  cpu_zeiten.sys_time = 0.;
  cpu_zeiten.user_time = 0.;
  in_bytes = 0;
  out_bytes = 0;
  discon = false;
  wait_for_connect = false;
}



bool thread::get_char( char  &ch )
{
  bool flag = false;

  cpu_messung m;
  if (cli_sock != NULL)
    {
      try
	{
	  cli_sock->get(ch);
	  in_bytes++;
	  flag = true;
	}
      catch( Error_while_reading_socket )
	{
	  flag = false;
	}
    }
  cpu_zeiten += m.stop();
  return flag;
}
 
int thread::gets( String &s, size_t maxlen )
{
  int bytes=0;
  
  cpu_messung m;
  if (cli_sock != NULL)
    {
      try
	{
	  cli_sock->get(s,maxlen);
	  bytes = s.slen();
	  in_bytes += (unsigned int ) bytes;
	  tracer.input(id,s);
	}
      catch( Error_while_reading_socket )
	{
	  bytes = 0;
	}
    }
  cpu_zeiten += m.stop();
  return bytes;
}

int thread::write( const String &s )
{
  int bytes=0;
  
  cpu_messung m;
  if (cli_sock != NULL)
    try
      {
	bytes = s.slen();
	cli_sock->put(s);
	out_bytes += (unsigned int) bytes;
	tracer.output(id,String(s));
      }
    catch( Error_while_writing_socket )
      {
	bytes = 0;
      }
  cpu_zeiten += m.stop();
  return bytes;
}

int thread::write( const String &s, int max_l )
{
  int bytes=0;
  
  cpu_messung m;
  if (cli_sock != NULL)
    try
      {
	bytes = cli_sock->put(s,max_l);
	out_bytes += (unsigned int) bytes;
	tracer.output(id,String(s));
      }
    catch( Error_while_writing_socket )
      {
	bytes = 0;
      }
  cpu_zeiten += m.stop();
  return bytes;
}

bool thread::do_interface(const String &inp, String &outp)
{
  bool flag = false;
  if (!discon)
    if (intf != NULL)
      {
	try
	  {
	    outp = "";
	    cpu_messung m;	    
	    flag = intf->process(inp,outp);
	    cpu_zeiten += m.stop();
	  }
	catch( Error_message_not_defined )
	  {
	    outp.append("Schwerwiegendes Problem mit Sprachendatei"+'\n');
	    syslog logf(configuration);
	    logf.eintrag("Fehlender Eintrag in Sprachdatei",LOGMASK_PRGRMERR);
	  }
      }
    else
      throw Error_try_to_do_NULL_Interface();
  return flag;
}

void thread::do_close( void )
{
  discon = true;
}

bool thread::do_discon( void )
{
  if (intf != NULL && cli_sock != NULL)
    if (discon && output_puffer.slen() == 0)
      {
	char cid;
	if (tcp_ax25_flag)
	  cid = 'T';
	else
	  cid = 'A';
	intf->set_io(cid,in_bytes,out_bytes);
	tracer.close(id);
	delete intf;
	intf = NULL;
	cli_sock->close();
	delete cli_sock;
	cli_sock = NULL;
	return true;
      }
    else
      return false;
  else
    return false;
}

void thread::do_kill( void )
{
  if (intf != NULL && cli_sock != NULL)
    {
      char cid;
      if (tcp_ax25_flag)
	cid = 'T';
      else
	cid = 'A';
      intf->set_io(cid,in_bytes,out_bytes);
      tracer.close(id);
      delete intf;
      intf = NULL;
      cli_sock->close();
      delete cli_sock;
      cli_sock = NULL;
    }
}

struct thread_info thread::get_info( void )
{
  struct thread_info tmp;

  tmp.pid = id;
  tmp.call = intf->get_connected_digi();
  tmp.call.set_format(true);
  tmp.call.set_nossid(false);
  if ( in_out_coming_flag )
    tmp.direction = '<';
  else 
    tmp.direction = '>';
  if (discon)
    tmp.state = 'D';
  else if ( wait_for_connect )
    tmp.state = 'C';
  else 
    tmp.state = 'E';

  switch(typ)
    {
      case t_nodef    : tmp.typ = "Unknown   ";
	                break;
      case t_ax25_con : tmp.typ = "AX25-User ";
	                break;
      case t_tcpcon   : tmp.typ = "TCP-User  ";
	                break;
      case t_dxclu    : tmp.typ = "DX-Cl.    ";
	                break;
      case t_digi     : tmp.typ = "DIGI-Chk. ";
	                break;
      case t_wx       : tmp.typ = "WX        ";
	                break;
      case t_rpc      : tmp.typ = "RPC       ";
                        break;
      case t_fwd      : tmp.typ = "Fwd       ";
                        break;
      case t_smtp     : tmp.typ = "SMTP-In   ";
	                break;
      case t_http     : tmp.typ = "HTTP      ";
                        break;
      case t_axhtp    : tmp.typ = "AXHTP     ";
                        break;
      case t_outgoing : tmp.typ = "Outgoing  ";
                        break;
    }
  tmp.login_zeit = delta_t(zeit()-startzeit);
  tmp.user_zeit = delta_t(cpu_zeiten.user_time);
  tmp.sys_zeit = delta_t(cpu_zeiten.sys_time);
  tmp.in_bytes = in_bytes;
  tmp.out_bytes = out_bytes;

  return tmp;
}

int thread::get_next_id()
{
  int new_id=last_id++;
/*
// ToDo: ID auf <= 9999 begrenzen, dabei keine Doppelvergabe
  if (new_id>9999)
    { new_id=0; last_id=0; }

// Grrr... Warum geht das nicht ??
   for (t_threads_it it = threads.begin() ; it != threads.end();++it)
   {
cerr << "GetID: " << it->get_id() << endl;
    if (it->get_id() == new_id) 
    {
cerr << "SameID: " << it->get_id() << endl;
	new_id++;
	last_id++;
	it=threads.begin();
    }
  }
cerr << "ID: " << new_id << endl;  
*/
  return new_id;
} 
