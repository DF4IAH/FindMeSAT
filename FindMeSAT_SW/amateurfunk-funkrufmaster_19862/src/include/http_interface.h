/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2003 by Holger Flemming                               *
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
 * Some Ideas from:                                                         *
 * The HTTP-Server for the BayCom Mailbox                                   *
 * (c) by DL8MBT, DG9MHZ, OE3DZW et. al.                                    *
 *                                                                          *
 * List of other authors:                                                   *
 *  		                                			    *
 ****************************************************************************/

#ifndef __HTTP_INTERFACE_H__
#define __HTTP_INTERFACE_H__

#include <iostream>

#include "interfaces.h"
#include "String.h"
#include "user.h"
#include "makros.h"
#include "bake.h"
#include "passwd.h"
#include "html_generator.h"

using namespace std;

#define COOKIE_MAXAGE 3600

class http_interface : public interfaces
{

 protected:

  typedef struct sockaddr_in sadr;

  int status;
  enum method    { m_no, m_wrong, m_get, m_head, m_post };
  enum ht_states { ht_wait_method, ht_wait_headers, ht_check_auth, ht_ausgabe, 
  		   ht_ready };

  method met;
  mime_type m_typ;
  ht_states state;
  String url;
  String host;
  String user_agent;
  int contentlength;
  int rcvd_contentlength;

  String s_mycall;
  bool start;
  bool cookie;
  bool url_is_file;

  consolpw cpw;
  String userpass;

  String pw;
  String login_id;

  user usr;
  makros mak;
  html_generator htmgen;
  t_baken *baken;
  uint32_t sock_adr;


 protected:
  void base64bin( const String &, String &  );
  void get_authorization( const String &, bool  );
  void get_field(const String &, String & );
  void get_contentlength( const String & );
  void get_request( void );
  void get_headers( void );
  void check_auth( void );
  void send_file_info( void );
  void print_error(String & );
  const String & status_phrase( int  );
  void print_header(String &, int, zeit);
  void ausgabe( String & );
 
  bool state_machine( String & );

 public:
  http_interface(String &, t_baken&, uint32_t );
  ~http_interface();
  bool do_process( bool, String & );
};


#endif

