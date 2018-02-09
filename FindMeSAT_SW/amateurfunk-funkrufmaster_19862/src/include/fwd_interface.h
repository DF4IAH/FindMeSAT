/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002,2003 by Holger Flemming                               *
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

#ifndef __FWD_INTERFACE_H__
#define __FWD_INTERFACE_H__

#include <list>
#include <iostream>

//#include <unistd.h>

#include "globdef.h"
#include "String.h"
#include "callsign.h"
#include "fwd_protokoll.h"
#include "fwd_nachrichten.h"
#include "fwd_descriptoren.h"
#include "mid.h"

#include "interfaces.h"

using namespace std;

class Error_could_not_init_fwd_interface
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_init_fwd_interface()
    {
      cerr << "Error_could_not_init_fwd_interface" << endl;
    }
#endif
};

class Error_messagetyp_not_supported
{
#ifdef _DEBUG_EXEC_
 public:
  Error_messagetyp_not_supported()
    {
      cerr << "Error_messagetyp_not_supported" << endl;
    }
#endif
};

class Error_could_not_convert_message
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_convert_message()
    {
      cerr << "Error_could_not_convert_message" << endl;
    }
#endif
};



class fwd_interface : public interfaces
{
 protected:
  fwdlog fwd_log;
  callsign fwd_partner;
  bool autorouter_enable;
  bool kommunikation_laeuft;
  bool verbindung_trennen;
  bool fehler;
  zeit letzte_aktivitaet;
  int t_w;
  int fwd_timeout;
  unsigned int n_max;
  unsigned int unack_max;
  unsigned int unack;
  unsigned int fehler_zaehler;
  unsigned int max_fehler_zaehler;

  int spool_priv,spool_bul,spool_dest,spool_sonst;


  protokoll_optionen eigene_optionen;
  protokoll_optionen gemeinsame_optionen;

  typedef list<nachrichten_descriptor> t_queue;
  typedef t_queue::iterator t_queue_it;

  t_queue tx_queue;

  String fwddirname;

 protected:
  void init( bool );
  void read_tx_queue( void );
  void send_msg(nachrichten_descriptor& desc, String&);
  void rcvd_msg( const String&, String & );
  void rcvd_ack_msg( bestaetigungs_nachricht& );
  void send_ack_msg( const Mid& , String& );
  void check_tx_queue( String& );
  void poll(String& );

  funkruf_nachricht conv_board_to_funkruf( const skyper_rubrik_nachricht& );
  skyper_rubrik_nachricht conv_funkruf_to_board( const funkruf_nachricht& );
  bool is_board_msg( const funkruf_nachricht& );

 public:
  fwd_interface(String&, callsign , bool, connect_string&, bool );
  fwd_interface(String&, callsign, callsign, bool, bool );
  ~fwd_interface();
  bool do_process( bool, String & );
};


#endif
