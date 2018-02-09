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

#ifndef __AXHTP_INTERFACE_H__
#define __AXHTP_INTERFACE_H__

#include "interfaces.h"
#include "passwd.h"
#include "user.h"
#include "bake.h"
#include "html_generator.h"

class axhtp_interface : public interfaces
{
 protected:

  enum global_state { ahs_wait_cmd, ahs_no_command_available, 
		      ahs_analyse_command, ahs_read_file, ahs_respond,
		      ahs_send_error, ahs_send_auth_question, 
		      ahs_wait_for_auth_answer,ahs_auth_respond };

  global_state state;
  consolpw cpw;
  user usr;
  t_baken *baken;
  zeit last_akt;
  int status;
  html_generator htmgen;

  String cmd_line;

  char cmd;
  String url;
  int i_content_length;
  mime_type inp_m_typ;
  String i_puffer;
  int in_bytes_left;

  String requested_url;
  bool send_content_flag;

 protected:
  void do_error_respond( String & );
  void do_respond( String & );
  bool read_file( void );
  void analyse_command( bool & );
  void do_command( void  );
  void state_machine( String & , bool & );
  void do_tx_auth( String & );
  bool do_rx_auth( void );
  void do_auth_respond( String &  );

 public:
  axhtp_interface(String &, t_baken&, const user& );
  ~axhtp_interface();
  bool do_process( bool, String & );
};

#endif // __AXHTP_INTERFACE_H__
