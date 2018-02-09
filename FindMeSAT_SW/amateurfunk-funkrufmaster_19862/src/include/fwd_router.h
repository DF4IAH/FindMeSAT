/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
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

#ifndef __FWD_ROUTER_H__
#define __FWD_ROUTER_H__

#include "globdef.h"

#include <iostream>
#include <vector>
#include <list>
#include <netinet/in.h>

#include "destin.h"
#include "String.h"
#include "callsign.h"
#include "zeit.h"
#include "fwd_descriptoren.h"
#include "mid.h"
#include "statistics.h"

using namespace std;

class Error_wrong_fwd_file_format
{
 protected:
  String line;

 public:
  Error_wrong_fwd_file_format(const String &l)
    {
      line = l;
#ifdef _DEBUG_EXEC_
      cerr << "Error_wrong_fwd_file_format" << endl;
      cerr << line << endl;
#endif
    }
  inline String get_error_line( void )
    {
      return line;
    }
};

class Error_wrong_fwd_connection_typ
{
 protected: 
  char conn_typ;

 public:
  Error_wrong_fwd_connection_typ( char c)
    {
      conn_typ = c;
#ifdef _DEBUG_EXEC_
     cerr << "Error_wrong_fwd_connection_typ: " << c << endl;
#endif
    }
  inline char get_conn_typ( void )
    {
      return conn_typ;
    }
};

class Error_wrong_fwd_call
{
 protected:
  String call_string;

 public:
  Error_wrong_fwd_call( const String &c)
    {
      call_string = c;
#ifdef _DEBUG_EXEC_
      cerr << "Error_wrong_fwd_call: " << c << endl;
#endif
    }
  inline String get_wrong_call( void )
    {
      return call_string;
    }
};

class Error_could_not_open_fwd_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_fwd_file()
    {
      cerr << "Error_could_not_open_fwd_file" << endl;
    }
#endif
};

class Error_could_not_load_fwd_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_load_fwd_file()
    {
      cerr << "Error_could_not_load_fwd_file" << endl;
    }
#endif
};

class Error_could_not_init_router
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_init_router()
    {
      cerr << "Error_could_not_init_router" << endl;
    }
#endif
};

class Error_request_for_non_neighbor
{
#ifdef _DEBUG_EXEC_
 public:
  Error_request_for_non_neighbor()
    {
      cerr << "Error_request_for_non_neighbor" << endl;
    }
#endif
};

class Error_wrong_ip_address
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_ip_address()
    {
      cerr << "Error_wrong_ip_address()" << endl;
    }
#endif
};

class Error_no_message_available
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_message_available()
    {
      cerr << "Error_no_message_available()" << endl;
    }
#endif
};

enum fwd_conn_status { st_getrennt, st_aufbau, st_aktiv, st_gescheitert };

struct neighbor_info
{
  callsign call;
  char typ;
  String address;
  fwd_conn_status stat;
  int sch_cnt;
  zeit last_change;
  unsigned int n_pers;
  unsigned int n_bul;
  unsigned int n_dest;
  unsigned int n_sonst;
  int t_w;
  int n_max;
  int unack;
  int fehler_zaehler;
  protokoll_optionen options;
  vector<int> akt_thrds;
  vector<double> rtt;
  vector<double> offset;
  double mean_rtt;
  double mean_offset;
};

class neighbor_tab_entry
{
 private:
  callsign nachbar;
  char typ;
  String adress;
  bool autorouting;
  vector<destin> dests;
  fwd_conn_status c_status;
  int scheiter_zaehler;
  zeit letzte_status_aenderung;

  double rtt[16];
  double offset[16];
  zeit last_messung;
  int messungen_in_burst;
  bool messung_in_progress;

 public:
  protokoll_optionen options;
  vector<int> aktive_verbindungen;
  bool no_first_connect;

 public:
  unsigned int n_pers, n_bul, n_dest, n_sonst;
  unsigned int int_n_pers, int_n_bul, int_n_dest, int_n_sonst;

  int t_w, n_max, unack,fehler_zaehler;

 public:
  neighbor_tab_entry(istream &, istream & );
  neighbor_tab_entry( const callsign&, char, const String& );
  ~neighbor_tab_entry( void );
  void enable_autorouting( void );
  void disable_autorouting( void );
  bool add_destin( const destin & );
  bool del_destin( const destin & );
  bool check_destin( const destin &);
  bool update_pfad( const callsign&, char, const String& );
  bool reset( const callsign& );

  void init_messtab( void );
  bool now_messung( void );
  void messung_started( void );
  void messung( double, double );
  double get_rtt( void );
  double get_offset( void );

  inline char get_typ( void )
    {
      return typ;
    }
  inline String get_adress( void )
    {
      return adress;
    }
  inline bool get_autorouting( void )
    {
      return autorouting;
    }
  void status( fwd_conn_status );
  inline fwd_conn_status status( void )
    {
      return c_status;
    }
  inline int n_gescheitert( void )
    {
      return scheiter_zaehler;
    }
  inline callsign get_call( void )
    {
      return nachbar;
    }
  inline zeit get_last_change( void )
    {
      return letzte_status_aenderung;
    }
  struct neighbor_info get_info( void );
  void save( ostream& );
};


class fwd_router
{
 protected:
  bool activ;
  bool geaendert;
  String base_fwddirname;
  String fwddirname;
  list<neighbor_tab_entry> neighbor_tab;
  list<neighbor_tab_entry>::iterator pos;
  list<routing_descriptor> descriptors;
  mid mid_tab;
  mid mid_tab_routinginf;
  fwdstatistic stat;

  void read_neighbor_tab( void );
  void clear_neighbor_tab( void );
  void save_neighbor_tab( void );
  int wait_time( list<neighbor_tab_entry>::iterator );
  bool check_mid( nachricht & );

 public:
  fwd_router( void );
  ~fwd_router( void );
  bool enable( void );
  bool disable( void );
  void load_fwd( void );
  void save_fwd( void );
  bool add_partner(  const callsign&, char, const String& );
  bool del_partner( const callsign& );
  bool add_destin( const callsign&, const destin& );
  bool del_destin( const callsign&, const destin& );
  bool set_pfad( const callsign&, char, const String& );
  bool reset( const callsign& );
  bool enable_autorouting( const callsign & );
  bool disable_autorouting( const callsign & );
  void log_message(nachricht &);

  void read_spool( void );
  bool route_message( nachricht&, bool, callsign  , callsign  = callsign("N0N"));
  bool message_avalable( const callsign& );
  nachricht& get_tx_message( const callsign& );
  bool start_connection(callsign &, char &, String &, bool& );
  void connection_started( const callsign & );
  void connection_established( const callsign &, const protokoll_optionen&  , int);
  void connection_closed( const callsign & , int);
  void connection_failed( const callsign & , int = -1);
  vector<struct neighbor_info> get_infos( bool & );
  vector<struct neighbor_info> get_infos( bool &, const callsign& );
  String get_hostname( const String & );
  String get_portstring( const String & );
  bool is_nachbar( char , callsign& , bool& );
  bool is_nachbar(uint32_t , callsign&, bool& );
  Mid get_mid( bool = false );
  void process_rtt_messung( void );
  nachricht & do_rtt_messung( void );
  bool rx_rtt_messung(const callsign&, const zeit_nachricht&, zeit_nachricht& );
  void set_spooled_messages( const callsign&, int, int, int, int );
  void set_interface_parameter( const callsign&, int, int, int, int );
  fwdstatistic get_stat( void );
};


#endif
 
