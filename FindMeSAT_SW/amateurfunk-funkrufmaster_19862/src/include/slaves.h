/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002 by Holger Flemming                                    *
 *                                                                          *
 * This Program is free software; you can redistribute it and/or modify     *
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

#ifndef __SLAVES_H__
#define __SLAVES_H__

#include <iostream>
#include <list>

#include "callsign.h"
#include "String.h"
#include "connect_string.h"
#include "zeit.h"
#include "bake.h"
#include "destin.h"

using namespace std;

class Error_wrong_parameter_name_in_slave_conf
{
 protected:
  String parameter_name;

 public:
  Error_wrong_parameter_name_in_slave_conf( const String &p)
    {
      parameter_name = p;
#ifdef _DEBUG_EXEC_
      cerr << "Error_wrong_parameter_name_in_slave_conf: " << p << endl;
#endif
    }
  inline String get_parameter_name( void )
    {
      return parameter_name;
    }
};

class Error_wrong_file_format_in_slave_conf
{
 protected:
  String line;

 public:
  Error_wrong_file_format_in_slave_conf(const String &l)
    {
      line = l;
#ifdef _DEBUG_EXEC_
      cerr << "Error_wrong_file_format_in_slave_conf" << endl;
      cerr << line << endl;
#endif
    }
  inline String get_error_line( void )
    {
      return line;
    }
};

class Error_spooldir_undefined
{
#ifdef _DEBUG_EXEC_
 public:
  Error_spooldir_undefined()
    {
      cerr << "Error_spooldir_undefined" << endl;
    }
#endif
};

class Error_wrong_slave_callsign
{
 protected:
  String call_string;

 public:
  Error_wrong_slave_callsign(const String &c)
    {
      call_string = c;
#ifdef _DEBUG_EXEC_
      cerr << "Error_wrong_slave_callsign: " << c << endl;
#endif
    }
};

class Error_slavedef_starts_with_wrong_parameter
{
 protected:
  String parameter;

 public:
  Error_slavedef_starts_with_wrong_parameter(const String &p)
    {
      parameter = p;
#ifdef _DEBUG_EXEC_
      cerr << "Error_slavedef_starts_with_wrong_parameter: " << p << endl;
#endif
    }
};

class slave
{
 public:
  enum slave_conn_status { sts_disabled, sts_spool, sts_getrennt, sts_aufbau, sts_aktiv , sts_trennen_uz_disablen, sts_trennen_uz_spoolen, sts_disablen, sts_spoolen, sts_gescheitert };

  enum slave_mode {m_passiv, m_activ };

 protected:
  int scheiter_zaehler;
  callsign call;
  String pfad;
  String slots;
  String bake;
  String passwd;
  String version;
  double diff;
  connect_string connect_pfad;
  own_destins dests;
  slave_conn_status c_status;
  slave_mode mode;
  zeit last_status_change;
  t_baken my_baken;
  int b_num;

 public:
  slave(istream&, const String&, const callsign&, t_baken& );
  slave(const callsign&, const String&, const connect_string&, const String&, const String&, slave_mode, const own_destins& );
  void PrintOn(ostream & );

  bool add_pocbake( const String&, int, t_baken& );
  bool del_pocbake( int, t_baken& );
  bool check_pocbake( int );
  inline void status( slave_conn_status stat )
    {
      c_status = stat;
      if (stat == sts_gescheitert)
      {
        if(scheiter_zaehler==0)
            last_status_change = zeit();
	scheiter_zaehler++;
      }
      else if (stat == sts_aktiv)
      {
	scheiter_zaehler = 0;
        last_status_change = zeit();
      }
      else if (stat == sts_getrennt )
      {
        last_status_change = zeit();
      }
    }
  inline zeit last_change( void ) const
    {
      return last_status_change;
    }
  inline void n_gescheitert( int i) 
    {
      scheiter_zaehler=i;
    }
  inline int n_gescheitert( void ) const
    {
      return scheiter_zaehler;
    }
  inline void set_mode( slave_mode mo )
    {
      mode = mo;
    }
  inline void set_version( const String &vers )
    {
      version = vers;
    }
  inline String get_version( void ) const
    {
      return version;
    }
  inline slave_mode get_mode( void ) const
    {
      return mode;
    }
  inline slave_conn_status status( void ) const
    {
      return c_status;
    }
  inline callsign get_call( void ) const
    {
      return call;
    }
  inline String get_pfad( void ) const
    {
      return pfad;
    }
  inline double get_diff( void ) const
    {
      return diff;
    }
  inline void set_diff( double &adj )
    {
      diff = adj;
    }
  inline String get_slots( void ) const
    {
      return slots;
    }
  inline void set_slots( const String &sl ) 
    {
      slots = sl;
    }
  inline String get_bake( void ) const
    {
      return bake;
    }
  inline void set_bake( const String &b )
    {
      bake = b;
    }
  inline connect_string get_connect_pfad( void ) const
    {
      return connect_pfad;
    }
  inline void set_connect_pfad( const connect_string &cs )
    {
      connect_pfad = cs;
    }
  inline String get_passwd( void ) const
    {
      return passwd;
    }
  inline void set_passwd( const String &pw )
    {
      passwd = pw;
    }
  inline own_destins get_destinations( void ) const
    {
      return dests;
    }
  inline void set_destinations( const own_destins &d )
    {
      dests = d;
    }
};

class slave_control
{
 protected:
  list<slave> slaves;
  String basis_pfad;
  bool geaendert;

  int wait_time(list<slave>::iterator  );
 public:
  slave_control( void );
  ~slave_control(void ); 
  bool load( t_baken & );
  bool save( void  );
  vector<slave> get_slave( const callsign & );
  vector<slave> get_slave( void );
  void set_status( const callsign&, slave::slave_conn_status );
  bool set_slots( const callsign& , const String& );
  bool set_bake( const callsign&, const String& );
  bool set_version( const callsign&, const String& );
  bool set_diff( const callsign&, double );
  bool set_con_pfad( const callsign&, const connect_string& );
  bool set_passwd( const callsign&, const String & );
  bool set_destinations( const callsign&, const own_destins & );
  bool start_connection( callsign&, String& , connect_string&, String &, String &, slave::slave_mode &, String& );
  bool stop_connection( const callsign& );
  bool add_slave( const callsign&, const connect_string&, const String&, const String& );
  bool del_slave( const callsign& );
  bool add_pocbake( const callsign&, const String&, int, t_baken& );
  bool del_pocbake( const callsign&, int, t_baken& );
  bool check_pocbake( const callsign&, int );
  bool enable_slave( const callsign& );
  bool disable_slave( const callsign & );
  bool spool_slave( const callsign &);
  bool activ_slave( const callsign & );
  bool passiv_slave( const callsign & );
  bool reset_slave( const callsign & );
  void purge(void);
};

#endif  // __SLAVES_H__
