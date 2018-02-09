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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 * Some Ideas from:                                                         *
 * The HTTP-Server for the BayCom Mailbox                                   *
 * (c) by DL8MBT, DG9MHZ, OE3DZW et. al.                                    *
 *                                                                          *
 ****************************************************************************/

#include "http_interface.h"

#include <ctype.h>

#include "callsign.h"
#include "zeit.h"
#include "database.h"
#include "config.h"
#include "texte.h"
#include "user_interface.h"
#include "system_info.h"
#include "fwd_autorouter.h"
#include "fwd_router.h"

extern callsign_database calls;
extern config_file configuration;

extern callsign G_mycall;
extern fwd_router router;
extern autorouter a_router;


void http_interface::base64bin( const String &in, String &out )
{
  long outword = 0;

  unsigned int index = 0;
  out = String("");  
  while (index < in.slen()-3 )
    {
      for (unsigned int i = 0; i < 4 ; i++)
	{
	  outword <<= 6;
	  char a = in[index+i];
	  int b;
	  if (isalpha (a) && isupper(a))
	    b = (int) a - 65;
	  else if (isalpha(a) && islower(a))
	    b = (int) a - ((int) 'a' - 26);
	  else if (isdigit(a) )
	    b = (int) a + 4;
	  else if (a == '+')
	    b = 62;
	  else if (a == '-')
	    b = 63;
	  else 
	    b = 0;
	  outword |= b;
	}
      out.append(String((char) ((outword >> 16) & 255)));
      out.append(String((char) ((outword >> 8) & 255)));
      out.append(String((char) ((outword) & 255)));

      index += 4;
    } 
}

void http_interface::get_authorization( const String &line, bool c )
{
  String search;
  String loginpw;

  cookie = c;
  if (cookie)
    search = s_mycall;
  else
    search = "Basic";

  int ind = line.pos(search);
  unsigned int index;
  if (ind >= 0)
    {
      index = (unsigned int) ind;
      index += search.slen() + 1;
      if (index < line.slen() )
	{
	  userpass = line.copy(index,line.slen() - index);
	  userpass.kuerze();
	  base64bin(userpass,loginpw);
	  ind = loginpw.pos(':');
	  if (ind >= 0)
	    {
	      index = (unsigned int) ind;
	      pw = loginpw.copy(index+1,loginpw.slen() - index - 1);
	      login_id = loginpw.copy(0,index);
	      pw.kuerze();
	      login_id.kuerze();
	    }
	}
    }
}

void http_interface::get_field(const String &line, String &field )
{
  int ind = line.pos(':');
  unsigned int index;
  if (ind >= 0)
    {
      index = (unsigned int) ind+1;
      while (index < line.slen() && line[index] == ' ')
	index++;
      field = line.copy(index,line.slen() - index);
    }
}

void http_interface::get_contentlength( const String &line )
{
  String S_contentlength;
  get_field(line,S_contentlength);
  contentlength = S_contentlength.Stoi();
}

void http_interface::get_request( void )
{
  String method_str;
  String uri_str;
  String protocol_str;
  
  // Methode in einzelne Fragmente ausfspalten

  String inp;

  if (get_line(inp))
    {
      unsigned int j=0,start=0;
      for (unsigned int i = 0; i < inp.slen() ; i++ )
	if (inp[i] == ' ')
	  {
	    method_str = inp.copy(0,i);
	    j = i;
	    break;
	  }
      
      for (unsigned int i = j; i < inp.slen() ; i++ )
	if (inp[i] != ' ')
	  {
	    start = i;
	    j = i;
	    break;
	  }
      
      for (unsigned int i = j; i < inp.slen() ; i++ )
	if (inp[i] == ' ')
	  {
	    uri_str = inp.copy(start,i-start);
	    j = i;
	    break;
	  }
      
      for (unsigned int i = j; i < inp.slen() ; i++ )
	if (inp[i] != ' ')
	  {
	    start = i;
	    j = i;
	    break;
	  }
      
      protocol_str = inp.copy(start,inp.slen() - start );
      
      // Methode ueberpruefen.
      if (method_str == "GET") 
	met = m_get;
      else if (method_str == "HEAD")
	met = m_head;
      else if (method_str == "POST")
	met = m_post;
      else
	{
	  met = m_wrong;
	  status = 501;
	  return;
	}
      
      // Nun URI untersuchen
      // ist ein ".." in der uri enthalten? Nicht zulaessig zum Schutz vor
      // "../../etc/passwd" o.ae.
      if (uri_str.in(".."))
	{
	  status = 403;
	  return;
	}
      else
	url = uri_str;
      
      // Protokoll ueberpruefen.
      
      if (protocol_str.copy(0,5) != String( "HTTP/") )
	{
	  status = 400;
	  return;
	}
      state = ht_wait_headers;
    }
}

void http_interface::get_headers( void )
{
  String line;

  while (get_line(line))
    {
      if (line.slen() < 3)
	{
	  state = ht_check_auth;
	}
      else if (line.slen() > 13 && line.copy(0,13) == String("Authorization"))
		get_authorization(line,false);
      else if (line.slen() >  6 && line.copy(0,6) == String("Cookie"))
		get_authorization(line,true);
      else if (line.slen() >  4 && line.copy(0,4) == String("Host") )
		get_field(line,host);
      else if (line.slen() > 14 && line.copy(0,14) == String("Content-length"))
		get_contentlength(line);
      else if (line.slen() > 10 && line.copy(0,10) == String("User-Agent"))
		get_field(line,user_agent);
    }
}

void http_interface::check_auth( void )
{
  if (pw == "" && login_id == "")
    {
      status = 401;
      return;
    }
  try
    {
      callsign call(login_id);
      if (!cpw.check_password(call,pw))
	{
	  status = 401;
	  return;
	}
      usr = user(call,sock_adr);
      mak = makros(usr);
      htmgen.set_user(usr);
      gegenstation = call;
    }
  catch( Error_no_callsign )
    {
      status = 401;
    }
  catch(  Error_callsign_does_not_exist )
    {
      status = 401;
    }
}
 



void http_interface::send_file_info( void )
{
}

const String & http_interface::status_phrase( int status )
{
  static String out;

  if (status == 200)
    out = "OK";
  else if (status == 201)
    out = "Created";
  else if (status == 202)
    out = "Accepted";
  else if (status == 204)
    out = "No Content";
  else if (status == 301)
    out = "Moved Permanently";
  else if (status == 302)
    out = "Moved Temporarily";
  else if (status == 304)
    out = "Not Modified";
  else if (status == 400)
    out = "Bad Request";
  else if (status == 4001)
    out = "Unauthorized";
  else if (status == 403)
    out = "Forbidden";
  else if (status == 404)
    out = "Not Found";
  else if (status == 500)
    out = "internal Server Error";
  else if (status == 501)
    out = "Not Implemented";
  else if (status == 502)
    out = "Bad Gateway";
  else if (status == 503)
    out = "Service Unavailable";
  else if (status == 0)
    out = "Unknown";
  else
    out = "";

  return out;
}

void http_interface::print_header(String &outp, int clength, zeit date )
{
  outp.append(String("HTTP/1.0 ")+itoS(status)+" "+status_phrase(status));
  outp.append( String( '\n' ) );
  outp.append(String("Date: ")+date.get_zeit_string() + String(" GMT"));
  outp.append( String( '\n' ) );
  outp.append(String("Server: Funkruf Master HTTP-Interface / Version ")+String(VERSION));
  outp.append( String( '\n' ) );

  if (status == 401)
    {
      outp.append("WWW-Authenticate: Basic realm=\"");
      outp.append(G_mycall.str()+"\"");
      outp.append( String( (char) 10 ) );
      outp.append("Set-Cookie: FRM"+G_mycall.str());
      outp.append("=; path=/;expires="+zeit().get_zeit_string());
      outp.append( String( (char) 10 ) );
    }
  else if (status == 200 && url == "/" &&  !cookie )
    {
      outp.append("Set-Cookie: "+G_mycall.str()+"="+userpass);
      zeit z = zeit() + COOKIE_MAXAGE;
      outp.append("; path=/;"+z.get_zeit_string());
      outp.append( String( '\n' ) );
    }
  outp.append("Content-type: ");
  switch (m_typ)
    {
      case mime_none : outp.append("text/html"); break;
      case mime_jpg  : outp.append("image/jpeg") ; break;
      case mime_gif  : outp.append("image/gif") ; break;
      case mime_bin  : outp.append("aplication/x-binary") ; break;
      case mime_wav  : outp.append("audio/wav") ; break;
      case mime_txt  : outp.append("text/plain"); break;
      case mime_html : outp.append("text/html"); break;
    }
  outp.append( String( '\n' ) );
  outp.append( String("Content-Length: ")+itoS(clength) );
  outp.append( String( '\n' ) );
  // Zum Abschluss der Header-Felder eine leere Zeile senden!

  outp.append( String( '\n' ) );


}


void http_interface::ausgabe( String &outp )
{
  String tx_content;
  int clength;
  zeit date;

  if (status == 200)
    status = htmgen.do_url(url,tx_content,m_typ,clength,date,*baken);

  if (status != 200)
    htmgen.do_error_msg(status,tx_content,m_typ,clength,date);

  print_header(outp,clength,date);
  if ( met != m_head )
    outp.append(tx_content);

  state = ht_ready;
} 

bool http_interface::state_machine(  String &outp )
{
  bool quit_flag = true;


  if ( state == ht_wait_method)
    { 
      get_request();
      quit_flag = true;
    }
  if ( state == ht_wait_headers )
    {
      get_headers();
      quit_flag = true;
    }
  if ( state == ht_check_auth )
    {
      check_auth();	
      state = ht_ausgabe;
      quit_flag = true;
    }
  if ( state == ht_ausgabe )
    {
      ausgabe(outp);
      quit_flag = true;
      // return quit_flag;
    }
  if ( state == ht_ready )
    {      
      quit_flag = false;
    }
  return quit_flag;
}
bool http_interface::do_process( bool rx_flag, String&outp )
{
  outp = "";
  return state_machine(outp);
}
 

http_interface::http_interface( String &outp, t_baken &bk, uint32_t adr ) : interfaces(outp,false), cpw(false), htmgen(configuration)
{
  outp = "";
  status = 200;
  met = m_no;
  m_typ = mime_txt;
  state = ht_wait_method;
  url = String("");
  host = String("");
  contentlength= 0;
  rcvd_contentlength = 0;

  start = false;
  cookie = false;

  baken = &bk;
  interface_id = 'H';
  path_finished = true;
  sock_adr = adr;

  if ( adr & AX25_IP_MASK == AX25_IP_ADDRESS)
    cpw.set_ax25_flag();

  htmgen.set_cpw(cpw);

}

http_interface::~http_interface( void )
{
  httplog hlog(configuration);

  hlog.eintrag(sock_adr,url,user_agent,host);
}

