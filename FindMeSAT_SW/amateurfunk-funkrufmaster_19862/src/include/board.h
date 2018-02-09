/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
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

#ifndef __BOARD_H__
#define __BOARD_H__

#include <iostream>
#include <list>
#include <vector>

using namespace std;

#include "String.h"
#include "zeit.h"
#include "callsign.h"
#include "destin.h"
#include "config.h"


class Error_could_not_open_boardfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_boardfile()
    {
      cerr << "Error_could_not_open_boardfile" << endl;
    }
#endif
};

class Error_could_not_create_boardfile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_create_boardfile()
    {
      cerr << "Error_could_not_create_boardfile" << endl;
    }
#endif
};

class Error_boarddirectory_not_defined
{
#ifdef _DEBUG_EXEC_
 public:
  Error_boarddirectory_not_defined()
    {
      cerr << "Error_boarddirectory_not_defined" << endl;
    }
#endif
};

class Error_could_not_create_boardnames
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_create_boardnames()
    {
      cerr << "Error_could_not_create_boardnames" << endl;
    }
#endif
};

struct board_sysop
{
  callsign call;
  bool local;
  bool wildcard;
};

class board
{
 public:
  enum permissions { perm_no, perm_local, perm_forw };

 protected:

  struct nachricht
  {
    zeit einspielung;
    destin dest;
    int lifetime;
    String msg;
  };

  String name;
  String brd_dir;
  int brd;
  int next_slot;
  int def_lifetime;

  zeit last_tx;
  int tx_intervall;

  list<struct board_sysop> sysops;
  struct nachricht nachrichten[10];
  bool changed;
  bool delete_flag;

  void read_board_sysops( istream &, list<struct board_sysop> &);
  void write_board_sysops( ostream &, list<struct board_sysop> &);

 public:
  // board( .. ) : Moegliche Ausnahmen:
  // Error_could_not_open_boardfile
  // Error_boarddirectory_not_defined
  board( String, config_file&, bool = false, int = 0, int = 30, int = 86400 );

  // ~board() : Moegliche Ausnahmen:
  // Error_could_not_create_boardfile
  ~board();

  inline int get_brd_id( void ) const
    {
      return brd;
    }
  inline int get_slot( void )
    {
      changed = true;
      int ret_slot = next_slot++;
      if (next_slot > 10) next_slot = 1;
      return ret_slot;
    }
  inline String get_name( void ) const
    {
      return name;
    }
  inline int get_deflt( void ) const
    {
      return def_lifetime;
    }
  inline void delete_brd( void )
    {
      delete_flag = true;
    }
  inline int get_tx_intv( void ) const
    {
      return tx_intervall;
    }
  inline zeit get_last_tx( void ) const
    {
      return last_tx;
    }
  inline String get_msg( int i ) const
    {
      if (i > 0 && i <= 10)
	return nachrichten[i-1].msg;
      else
	return String("");
    }
  inline zeit get_newest_time( void ) const
    {
      zeit newest=zeit(0);
      for (int i=1; i<=10; i++)
      {
	if((nachrichten[i-1].einspielung) > newest)
	    newest=nachrichten[i-1].einspielung;
      }	    
      return newest;
    }
  inline zeit get_time( int i ) const
    {
      if (i > 0 && i <= 10)
	return nachrichten[i-1].einspielung;
      else
	return zeit(0);
    }
  inline destin get_destin( int i ) const
    {
      if (i > 0 && i <= 10)
	return nachrichten[i-1].dest;
      else
	return destin();
    }
  inline int get_lifetime( int i ) const
    {
      if (i > 0 && i <= 10)
	return nachrichten[i-1].lifetime;
      else
	return 0;
    }
  inline void set_msg( String msg, int i ,const destin &d, int lt = -1)
    {
      if (i > 0 && i <= 10)
	{
	  changed = true;
	  nachrichten[i-1].einspielung = zeit();
	  nachrichten[i-1].dest = d;
	  if (lt == -1)
	    nachrichten[i-1].lifetime = def_lifetime;
	  else
	    nachrichten[i-1].lifetime = lt;
	  nachrichten[i-1].msg = msg;
	}
    }
  inline void del_msg(int i)
    {
      if (i > 0 && i <= 10)
	{
	  changed = true;
	  nachrichten[i-1].einspielung = zeit(0);
	  nachrichten[i-1].dest = get_destin(i);
	  nachrichten[i-1].lifetime = def_lifetime;
	  nachrichten[i-1].msg = "";
	}
    }
  inline void set_def_lt( int lt )
    {
      def_lifetime = lt;
      changed = true;
    }

  inline void set_lt( int i, int lt )
    {
      if (i > 0 && i <= 10)
      {
        nachrichten[i-1].lifetime = lt;
        changed = true;
      }
    }
  inline bool ready_for_tx( void )
    {
      if (tx_intervall == 0)
	return false;
      else
	return zeit() - last_tx > tx_intervall;
    }
  inline void set_tx( void )
    {
      last_tx = zeit();
      changed = true;
    }
  inline void set_intv( int tx_intv )
    {
      tx_intervall = tx_intv;
    }
  void add_sysop( const callsign&, bool, bool );
  void del_sysop( const callsign&, bool );
  vector<struct board_sysop> get_sysops( void );
  board::permissions get_permission( const callsign & );
  void purge( void );
  //void show( ostream &, char , bool , bool = false);
};

class boards
{
 private:
  String brd_dir;


  struct ltnum
    {
      bool operator() (int i1, int i2 ) const
      {
	return i1 < i2;
      }
    };

  typedef map<int,String,ltnum> t_dir;

 public:
  // boards() : Moegliche Ausnahmen:
  // Error_could_not_create_boardnames
  boards(  );
  void send_names( void );
  int send_messages( void );
  int send_all_messages( void );
  void purge( void );
  bool get_board_by_id( int, board& );
  vector<String> get_board_names( void ) const;
  //void show( ostream &, char , bool = false);
};


#endif
