/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Holger Flemming                                    *
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

#ifndef __FWD_AUTOROUTER__
#define __FWD_AUTOROUTER__

#include <list>
#include <vector>

#include "callsign.h"
#include "zeit.h"
#include "destin.h"
#include "fwd_nachrichten.h"

using namespace std;

class Error_routing_tab_corrupted
{
#ifdef _DEBUG_EXEC_
 public:
  Error_routing_tab_corrupted()
    {
      cerr << "Error_routing_tab_corrupted" << endl;
    }
#endif
};

class routing_tab_nachbar_eintrag
{
 public:
  callsign nachbar;
  destin ds;

 protected:
  double delay_n;
  double delay_n_1;
  double delay_m;

  zeit t_n;
  zeit t_n_1;
  zeit t_m;

 public:
  routing_tab_nachbar_eintrag( const callsign & );
  void rx_delay( double, double );
  void tx_delay( double );
  double tx_delay( void );
  double last_tx_delay( void );
  zeit last_tx_meldung( void );
  zeit last_rx_meldung( void );
  void messung( double );
  void init_nachbar( void );
};

typedef list<routing_tab_nachbar_eintrag> t_nachbarn;
typedef t_nachbarn::iterator t_nachbarn_it;


struct destin_info
{
  destin zielgebiet;
  double min_delay;
  callsign min_delay_call;
  vector<double> delay;
};

class routing_tab_entry
{
 protected:

  destin zielgebiet;
  bool local;
  zeit t_l;
 public:
  t_nachbarn nachbarn;

 public:
  routing_tab_entry( const destin &, bool = false );
  void set_local( bool );
  void add( const callsign& );
  void del( const callsign& );
  void rx_delay( const callsign& , double , double );
  double get_min_delay( callsign & );
  inline destin get_destin( void )
    {
      if (!zielgebiet.check())
	throw Error_routing_tab_corrupted();
      return zielgebiet;
    } 
  inline bool is_local( void )
    {
      return local;
    }
  struct destin_info get_info( void );
  void messung( const callsign&, double );
  void init_nachbar( const callsign& );
};

struct d_infos
{
  callsign to;
  destin zielgebiet;
  double delay;
};

typedef list<routing_tab_entry> t_routing_tab;
typedef t_routing_tab::iterator t_routing_tab_it;

struct nachbarn_t
{
  callsign call;
  double last_rtt;
};

class routing_tab
{
 protected:
  t_routing_tab eintraege;
  //  all_nachbar_tabs nachbar_tabellen;
  vector<struct nachbarn_t> all_nachbarn;

 protected:
  void add_d_info(vector<struct d_infos> &, t_nachbarn_it, double, const destin& );
  bool check_d_info(double , const t_routing_tab_it&, const t_nachbarn_it & , const callsign&, bool ) ;
  bool check_age(const t_routing_tab_it& , int  );

 public:
  routing_tab( void );
  // Aufbau der Tabelle

  void clear( void );
  void add( const callsign& );
  void del( const callsign& );

  bool exist( const destin & );
  void add( const destin &, bool );
  void del( const destin& );

  void set_local( const destin & );
  void rx_delay( const destin&, const callsign&, double );
  bool messung( const callsign &, double );
  void init_nachbar( const callsign & );

  vector<callsign> get_route( const destin& );
  vector<struct d_infos> get_d_infos( const destin& );
  vector<struct d_infos> get_d_infos( void );

  void clean_up( const destin & );
  void clean_up( void );

  vector<struct d_infos> reinit_corrupted_tab( void );

  vector<struct destin_info> get_infos( void );
  vector<struct destin_info> get_infos( const destin & );
  vector<callsign> get_nachbarcalls( void );

};

class autorouter
{
 protected:
  routing_tab rt;
  bool activ;

 protected:
  void create_init_message( const callsign & );
  void create_d_message( const destin &);
  void create_d_message( void );
  void clean_up( const destin & );
  void clean_up( void );
  void reinit_corrupted_routing_tab( void );
 public:
  autorouter( void );
  ~autorouter( void );

  void init( void );
  void enable( void );
  void disable( void );

  inline bool is_activ( void )
    {
      return activ;
    }
  void clear( void );

  void add( const callsign& );
  void del( const callsign& );

  void slave_connect( vector<destin> );
  void slave_discon( vector<destin> );

  void rx_d_message( const zielgebiets_nachricht& , const callsign& );
  void rtt_messung( const callsign&, double );
  void messung_gescheitert( const callsign& );
  void cyclic( void );

  vector<callsign> get_route(const destin &ds );
 
  inline vector<struct destin_info> get_infos( void ) 
    {
      return rt.get_infos();
    }
  inline vector<struct destin_info> get_infos( const destin &ds ) 
    {
      return rt.get_infos(ds);
    }
  inline vector<callsign> get_nachbarcalls( void ) 
    {
      return rt.get_nachbarcalls();
    }
};

#endif
