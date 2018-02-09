/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2003 by Holger Flemming                               *
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
 *                                                                          *
 ****************************************************************************/

#ifndef __SMTP_INTERFACE_H__
#define __SMTP_INTERFACE_H__

#include <iostream>

#include "String.h"
#include "adress.h"
#include "destin.h"
#include "interfaces.h"
#include "fwd_frontend.h"

using namespace std;

class Error_could_not_initialize_smtp_interface
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_initialize_smtp_interface()
    {
      cerr << "Error_could_not_initialize_smtp_interface" << endl;
    }
#endif
};

class smtp_interface : public interfaces
{
 protected:
  fwd_api fwd;

  enum states { wait_cmd, wait_data };

  states state;
  bool helo_done;
  bool mail_done;
  bool rcpt_done;

  String server_adress;

  bool single_adress_flag;
  adress single_adress;

  bool group_name_flag;
  String group_name;

  bool board_name_flag;
  String board_name;

  bool data_body;
  String message;
  callsign absender;




 protected:
  void state_maschine( const String &, String &, bool & );
  void do_command( const String &, String &, bool & );
  void do_helo(    const String &, String & );
  void do_mail(    const String &, String & );
  void do_rcpt(    const String &, String & );
  void do_data(    const String &, String & );
  void do_reset(   const String &, String & );
  void do_nop(     const String &, String & );
  void do_quit(    const String &, String &, bool & );
  void do_verify(  const String &, String & );
  void input_data( const String &, String & );

  bool send_message( void );
  bool page( const adress &, const String &, const destin & );
  bool page_group( void );
  bool page_board( void );

  void cut_adress( String & );
  bool check_absender( const String&, callsign& );
  bool check_first_part(const String& , callsign & );
  bool check_second_part( const String& , bool , callsign & );

 public:
  smtp_interface(String&, bool = false );
  ~smtp_interface();
  bool do_process(bool , String& );
};

bool is_ax25_smtp_sender( const callsign& );


#endif // __SMTP_INTERFACE_H__
