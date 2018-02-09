/****************************************************************************
 *                                                                          *
 *                                                                          *
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
 ****************************************************************************/

#ifndef __USER_INTERFACE_H__
#define __USER_INTERFACE_H__

#include <iostream>
#include <netinet/in.h>

#include "String.h"
#include "board.h"
#include "interfaces.h"
#include "user.h"
#include "gruppe.h"
#include "destin.h"
#include "makros.h"
#include "bake.h"
#include "slaves.h"
#include "zeit.h"
#include "texte.h"
#include "fwd_frontend.h"
#include "lang_support.h"
#include "passwd.h"
#include "relais.h"

using namespace std;

#define NULLCHAR ((char *)0)

#define TELNET_IAC  ((char) 255)
#define TELNET_WILL ((char) 251)
#define TELENT_WONT ((char) 252)
#define TELNET_DO   ((char) 253)
#define TELNET_DONT ((char) 254)

#define TELNET_OPTION_ECHO     ((char)  1)
#define TELNET_OPTION_HIDE     ((char) 45)
#define TELNET_OPTION_BINARY   ((char)  0)
#define TELNET_OPTION_TERMTYPE ((char) 24)
#define TELNET_OPTION_EOR      ((char) 25)


enum commands { c_no_cmd, c_unk_cmd, c_aktuell , c_help, c_info, c_locator, 
                c_name, c_newcall, c_typ, c_page, c_sysop, c_user, c_adduser,
                c_deluser, c_quit, c_addgrp, c_delgrp, c_addusrgrp, c_crontab,
                c_delusrgrp, c_groups, c_groupname, c_gpage, c_rubrik,
	        c_activate, c_dir, c_shutdown, c_dbsave, c_version, c_mkrub, 
                c_delrub, c_send , c_statistik, c_cstatus, c_who, c_syslog,
                c_log, c_param, c_rtext, c_wtext, c_slaves, c_showspool, 
                c_setslave, c_addslave, c_delslave, c_addbake, c_showbaken ,
                c_fwdstatus, c_iplog, c_saveslaves, c_setfwd, c_dbrequest , 
                c_sat, c_delbake, c_addfwd, c_delfwd, c_savefwd, c_talk, 
                c_wall, c_hlog, c_dxcluster, c_kill, c_wx, c_trace,
                c_rpclog, c_fwdlog, c_lang, c_setdir, c_connect, c_filedesc,
                c_zeit, c_passwd, c_dbtrans, c_gezeiten, c_relais , c_logmask,
		c_pagemask, c_fwdmask, c_rpcmask, c_destin, c_splog, c_digi,
		c_splogmask, c_exportmask};

enum subcommands { c_no_subcmd, c_unk_subcmd, c_sub_names, c_sub_messages,
                   c_sub_baken, c_sub_defaults, c_sub_enable, c_sub_disable,
		   c_sub_add, c_sub_del, c_sub_load, c_sub_fm10, c_sub_fm2,
		   c_sub_fm70, c_sub_fm23, c_sub_atv, c_sub_digi, 
		   c_sub_deflifetime,
		   c_sub_lifetime, c_sub_addboardop, c_sub_delboardop,
		   c_sub_harmonics,  c_sub_status, c_sub_kepler,  c_sub_lock,
		   c_sub_rotate, c_sub_reset,  c_sub_spool, c_sub_aktiv,
		   c_sub_passiv, c_sub_slots, c_sub_pfad, c_sub_passwort,
		   c_sub_destinations, c_sub_adddestin, c_sub_deldestin,
		   c_sub_prompt, c_sub_command, c_sub_temp, c_sub_druck,
		   c_sub_feucht, c_sub_richt, c_sub_geschw, c_sub_boen,
		   c_sub_nieder1, c_sub_nieder4, c_sub_niederm, c_sub_locator,
		   c_sub_show, c_sub_ar_enable, c_sub_ar_disable, c_sub_typ,
		   c_sub_addlink, c_sub_dellink,
		   c_sub_tx_wiederh };

istream& operator>>( istream& , commands& );
istream& operator>>( istream& , subcommands& );

  void rip(char *str);
  int ours(const char *buf, const char *search);


class user_interface : public interfaces
{
 protected:

  typedef struct sockaddr_in sadr;

  enum states { wait_callsign, wait_password, wait_cmd, wait_passwd, 
		wait_ax25pw, wait_text_input, wait_baken_text, wait_conv_text, 
		trace_mode, connect_mode, passwd_step1, passwd_step2,
                passwd_step3, quit_state, add_wx_step1, add_wx_step2,
		wait_pagetext, wait_gpagetext, wait_boardtext };

  states state;
  zeit quit_state_entry;
  user usr;
  gruppen grps;
  makros macros;
  t_baken *baken;
  int timeout;
  zeit last_akt;
  bool warning;
  wt_text *wtext_ptr;
  fwd_api fwd;
  user_meldungen mldg;
  consolpw cpw;

  callsign baken_gen_call;
  int baken_gen_slot;

  callsign conv_partner;

  callsign user_id;
  int passwd_tries;
  
  int connection_id;

  callsign passwd_call;
  bool check_old;
  String oldpw,newpw1,newpw2;
  bool sysop_socket;


  callsign page_call;
  destin page_ds;
  String page_gname;
  gruppe page_grp;

  String page_boardname;
  int page_slot;
  int page_lifetime;
  board::permissions perm;
  
#ifdef COMPILE_WX
  callsign add_wx_call;
  locator add_wx_loc;
#endif
  relais_database rels;


  String correct;


 protected:
  int get_slot( istream& );
  int get_lifetime( istream& );
  void rubrik(istream&, ostream&  );
  void rubrik_text(istream&, ostream&  );
  void dir_all( ostream &, const boards & );
  void dir_board( ostream &, board );
  void show_dir( istream&, ostream&  );
  void make_dir(istream &, ostream& );
  void set_tx_intervall( istream &, ostream & );
  void setdefaultlt(istream &,ostream &);
  void setlt(istream &, ostream& );
  void add_board_op(istream &,ostream &);
  void del_board_op(istream &,ostream &);
  void del_board_msg(istream &,ostream &);
  void set_dir(istream &, ostream& );
  void delete_dir(istream &, ostream& );
  void do_page(callsign &, ostream&, destin &, String &, bool );
  void page_group(istream &, ostream&);
  void page_grouptext(istream &, ostream&);
  void page(istream &, ostream& );
  void page_text(istream &, ostream& );
  void activate(istream &, ostream&);
  void set_name( istream &, ostream& );
  void set_loc( istream &, ostream& );
  void set_typ(  istream &, ostream& );
  void set_lang(  istream &, ostream& );
  void add_user(istream &, ostream&);
  void del_user(istream &, ostream&);
  void add_group( istream &, ostream& );
  void del_group(istream &, ostream& );
  void add_user_to_group( istream &, ostream& );
  void del_user_from_group( istream &, ostream& );
  void show_groups( istream &, ostream&);
  void show_user(const database_entry &, ostream & );
  void show_user(istream &, ostream&);
  void groupname(istream &, ostream&);
  void newcall(istream &, ostream&);
  void dbsave(  ostream& );
  void do_shutdown(  istream&, ostream& );
  void version( istream &, ostream& );
  void send( istream &, ostream& );
  void help(istream &, ostream&);
  void statistik( ostream& );
  void cstatus(  ostream& );
  void kill_th( istream&, ostream& );
  void log( istream&, ostream&, logfile& );
  void slog( istream&, ostream&  );
  void ulog( istream&, ostream& );
  void ilog( istream&, ostream& );
  void hlog( istream&, ostream& );
  void rlog( istream&, ostream& );
  void flog( istream&, ostream& );
  void splog( istream&, ostream& );
  void logmask( istream&, ostream& );
  void pagemask( istream&, ostream& );
  void fwdlogmask( istream&, ostream& );
  void rpclogmask( istream&, ostream& );
  void spoollogmask( istream&, ostream &);
  void exportmask( istream &, ostream & );

  void param( istream&, ostream& );
  void descr( ostream& );
  void zeit_datum( ostream& );
  String get_file_name( istream&, ostream&, bool& );
  void rtext( istream&, ostream& );
  void wtext( istream&, ostream& );
  void spool_relais(relais::relais_typ , String , double , double, istream& , ostream&  );
  void do_relais(istream &, ostream & );
  void wtext_init( istream&, ostream& );
  void show_slave( const slave &, ostream & );
  void show_slave_extended( const slave &, ostream & );
  void show_slave_list( vector<slave> , ostream & );
  void show_slaves( istream&, ostream& );
  void setslave(istream&, ostream& );
  void save_slaves( ostream& );
  void add_sat( istream&, ostream& );
  void del_sat( istream&, ostream& );
  void sat( istream&, ostream& );
  void add_ort( istream &, ostream & );
  void del_ort( istream &, ostream & );
  void set_kepler( istream &, ostream & );
  void set_sat_status( istream &, ostream & );
  void set_harmonics( istream &, ostream & );
  void gezeiten( istream &cmd, ostream &ostr );
  String satstatus(String);
  void wall( istream &, ostream & );
  void talk_to( istream &, ostream & );
  void conv_modus( istream &, ostream & );
  void trace( istream&, ostream& );
  void crontab( istream&, ostream& );
  void trace_modus( istream&, ostream& );
  void trace_modus(  ostream& );
  void dxcluster( istream &, ostream & );
  void show_destins( vector<struct destin_info> , bool ,ostream &);
  void destinations(istream&, ostream& );
  void connect( istream&, ostream& );
  void con_mode( istream &, ostream & );
  void con_mode( ostream & );

  void passwd( istream &, ostream & );
  void passwd_st1( istream &, ostream & );
  void passwd_st2( istream &, ostream & );
  void passwd_st3( istream &, ostream & );

  void setslave_enable( const callsign &, ostream & );
  void setslave_disable(  const callsign &, ostream & );
  void setslave_reset( const callsign &, ostream & );
  void setslave_spool( const callsign &, ostream & );
  void setslave_activ( const callsign &, ostream & );
  void setslave_passiv( const callsign &, ostream & );
  void setslave_slots( const callsign &, istream &, ostream & );
  void setslave_bake( const callsign &, istream &, ostream & );
  void setslave_pfad( const callsign &, istream &, ostream & );
  void setslave_passwd( const callsign &, istream &, ostream & );
  void setslave_destin( const callsign &, istream &, ostream & );
  void get_callsign(istream &, callsign& );

  void add_slave( istream&, ostream& );
  void del_slave(istream&, ostream& ); 
  void add_bake(istream&, ostream& );
  void add_baken_text( istream&, ostream& );
  void del_bake(istream&, ostream& );

  void addfwd( istream&, ostream& );
  void delfwd( istream&, ostream& );
  void addfwddestin( istream &, ostream & );
  void delfwddestin( istream &, ostream & );
  void setfwd( istream&, ostream& );
  void setfwd_pfad( istream &, ostream & );
  void setfwd_reset( istream &, ostream & );
  void setfwd_ar_enable( istream &, ostream & );
  void setfwd_ar_disable( istream &, ostream & );
  void savefwd( ostream& );
  void dbrequest(istream&, ostream& );
  void dbtrans(istream&, ostream& );
  void showfwdnb_full( struct neighbor_info , ostream & );
  void showfwdnb( struct neighbor_info , ostream &  );
  void showfwd(istream&, ostream& );

  void do_command(const String&, String&, bool & );
  void init_interface( String&);
  void get_callsign( const String&, String& );
  void get_password( const String&, String& , bool & );
  void check_ax25password( const String&, String &, String&, bool & );
  void state_maschine(const String&, String&, bool & );

#ifdef COMPILE_WX
  void wx_config( istream &, ostream & );
  void add_wx( istream &, ostream & );
  void add_wx_1( istream &, ostream & );
  void add_wx_2( istream &, ostream & );
  void del_wx( istream &, ostream & );
  void set_wx_pfad( istream &, ostream & );
  void set_wx_slot( istream &, ostream & );
  void set_wx_locator( istream &, ostream & );
  void set_wx_prompt( istream &, ostream & );
  void set_wx_command( istream &, ostream & );
  void set_wx_temperatur( istream &, ostream & );
  void set_wx_luftdruck( istream &, ostream & );
  void set_wx_luftfeuchte( istream &, ostream & );
  void set_wx_windrichtung( istream &, ostream & );
  void set_wx_windgeschwindigkeit( istream &, ostream & );
  void set_wx_boen( istream &, ostream & );
  void set_wx_niederschlag1( istream &, ostream & );
  void set_wx_niederschlag4( istream &, ostream & );
  void set_wx_niederschlag_m( istream &, ostream & );
#endif

#ifdef COMPILE_DIGISTATUS
  void digi_config( istream &, ostream & );
  void add_digi( istream &, ostream & );
  void del_digi( istream &, ostream & );
  void add_digi_link( istream &, ostream & );
  void del_digi_link( istream &, ostream & );
  void set_digi_pfad( istream &, ostream & );
  void set_digi_slot( istream &, ostream & );
  void set_digi_typ( istream &, ostream & );
  void set_digi_status( callsign &, bool, ostream & );
#endif

 public:
  user_interface(String&, uint32_t , uint16_t, bool, t_baken&);
  user_interface(String&, user&, bool, t_baken&);
  ~user_interface();
  bool do_process(bool , String& );
};

#endif
