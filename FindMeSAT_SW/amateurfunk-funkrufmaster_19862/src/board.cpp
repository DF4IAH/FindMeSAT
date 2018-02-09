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
 *                                                                          *
 ****************************************************************************/

#include "callsign.h"
#include "board.h"
#include "autoconf.h"

#include <fstream.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <map.h>

#include "cmdline.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "board.h"
#include "logfile.h"

extern config_file configuration;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

void board::read_board_sysops( istream &istr, list<struct board_sysop> &sysops)
{
  char ch;
  String dum;

  sysops.clear();
  istr >> ch;
  if (ch != '-')
    {
      istr.putback(ch);
      do
	{
	  struct board_sysop sysop;
	  
	  dum.get(istr,20,',');
	  dum.kuerze();
	  if (dum == "*")
	    sysop.wildcard = true;
	  else
	    {
	      sysop.wildcard = false;
	      try
		{
		  sysop.call = callsign(dum);
		}
	      catch ( Error_no_callsign )
		{}
	    }
	  istr >> ch;
	  if (ch == ',')
	    {
	      istr >> ch;
	      sysop.local = (ch == 'l');
	      istr >> ch;
	    }
	  sysops.push_back(sysop);
	}
      while ( ch == ';');
    }
  dum.getline(istr,1000);
}

void board::write_board_sysops( ostream &ostr, list<struct board_sysop> &sysops)
{
  if (sysops.size() == 0)
    ostr << '-' << endl;
  else
    {
      bool flag = false;
      for (list<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it )
	{
	  if (flag)
	    ostr << ';';
	  if (it->wildcard)
	    ostr << '*';
	  else
	    ostr << it->call;
	  ostr << ',';
	  if (it->local)
	    ostr << 'l';
	  else
	    ostr << 'f';
	  flag = true;
	}
      ostr << '-' << endl;
    }
}

board::board( String bname, config_file &cfg, bool create_flag, int create_code, int dlifetime , int tx_intv)
{
  try
    {
      delete_flag = false;
      String fname = bname;
      fname.lowcase();
      brd_dir = cfg.find("RUBRIKEN");
      
      ifstream brd_file(strtochar(brd_dir + fname));
      if (brd_file)
	{
	  changed = false;
	  name.getline(brd_file,99);
	  brd_file >> brd;
	  brd_file >> next_slot;
	  brd_file >> def_lifetime;

	  // Hier Kompatabilitaet zu altem Format wahren
	  char ch;
	  brd_file.get(ch);
	  if (ch == ',')
	    {
	      String puffer;

	      puffer.get(brd_file,100,',');
	      last_tx = zeit(puffer.Stoi());
	      brd_file.get(ch);
	      if ( ch == ',' )
		{
		  puffer.get(brd_file,100);
		  tx_intervall = puffer.Stoi();
		}
	      else
		{
		  tx_intervall = tx_intv;
		  changed = true;
		}
	    }
	  else
	    {
	      brd_file.putback(ch);
	      last_tx = zeit(0);
	      tx_intervall = tx_intv;
	      changed = true;
	    }

	  String dum;
	  dum.getline(brd_file,99);
	  read_board_sysops(brd_file,sysops);

	  for (int i = 0; i < 10; i++ )
	    {
	      String puffer;
	      char ch;

	      puffer.get(brd_file,100,',');
	      nachrichten[i].einspielung = zeit(puffer.Stoi());
	      brd_file.get(ch);

	      puffer.get(brd_file,100,',');
	      try
		{
		  nachrichten[i].dest = destin(puffer);
		}
	      catch( Error_no_destin )
		{}
	      brd_file.get(ch);

	      puffer.get(brd_file,100,',');
	      nachrichten[i].lifetime = puffer.Stoi();

	      brd_file.get(ch);
	      nachrichten[i].msg.getline(brd_file,999);
	      if (nachrichten[i].msg.slen() > 80)
		nachrichten[i].msg = nachrichten[i].msg.copy(0,80);
	    }
	}
      else
	if (create_flag)
	  {
	    name = bname;
	    brd = create_code;
	    changed = true;
	    next_slot = 1;
	    def_lifetime = dlifetime;
	    last_tx = zeit(0);
	    tx_intervall = tx_intv;
	    sysops.clear();
	    for (int i = 0; i < 10; i++ )
	      {
		nachrichten[i].einspielung = zeit(0);
		nachrichten[i].lifetime = 0;
		nachrichten[i].msg = String ("");
	      }
	  }
	else
	  {
	    syslog logf(cfg);
	    logf.eintrag("Kann Boarddatei "+brd_dir+fname+" nicht oeffnen.",LOGMASK_PRGRMERR);
	    throw Error_could_not_open_boardfile();
	  }
    }
  catch( Error_parameter_not_defined )
    {
      throw Error_boarddirectory_not_defined();
    }
}

board::~board( void )
{
  if (delete_flag)
    {
      String fname = name;
      fname.lowcase();
      remove(strtochar(brd_dir + fname) );
    }
  else
    if (changed)
      {
	String fname = name;
	fname.lowcase();
	ofstream brd_file(strtochar(brd_dir + fname));
	if (brd_file)
	  {      
	    brd_file << name << endl;
	    brd_file << brd << endl;
	    brd_file << next_slot << endl;
	    brd_file << def_lifetime << ',';
	    brd_file < last_tx;
	    brd_file << ',' << tx_intervall << endl;
	    write_board_sysops(brd_file,sysops);
	    for (int i = 0 ; i < 10 ; i++ )
	      {
		brd_file < nachrichten[i].einspielung;
		try
		  {
		    brd_file << ',' << nachrichten[i].dest << ',';
		  }
		catch( Error_destin_checksum_error )
		  {
                    syslog logf(configuration);
		    logf.eintrag("Destination-Pruefsummenfehler in Boarddatei",LOGMASK_PRGRMERR);
		    brd_file << ",@DL.EU,";
		  }
		brd_file << nachrichten[i].lifetime << ',';
		if (nachrichten[i].msg.slen() > 80)
		  nachrichten[i].msg = nachrichten[i].msg.copy(0,80);
		brd_file << nachrichten[i].msg << endl;
	      }
	  }
	else
	  {
	    syslog logf(configuration);
	    logf.eintrag("Kann Boarddatei "+brd_dir+fname+" nicht oeffnen.",LOGMASK_PRGRMERR);
	    throw Error_could_not_create_boardfile();
	  }
      }
}

void board::add_sysop( const callsign &call, bool local, bool wildcard )
{
  struct board_sysop sysop;

  sysop.wildcard = wildcard;
  sysop.call = call;
  sysop.local = local;
  sysops.push_back(sysop);
  changed = true;
}

void board::del_sysop( const callsign &call, bool wildcard )
{
  for (list<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it )
    {
      changed = true;
      if (wildcard && it->wildcard)
	{
	  sysops.erase(it);
	  return;
	}
      else if (samecall(call,it->call))
	{
	  sysops.erase(it);
	  return;
	}
    }
}

vector<struct board_sysop> board::get_sysops( void )
{
  vector<struct board_sysop> tmp;

  for (list<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it )
    tmp.push_back(*it);

  return tmp;
}

board::permissions board::get_permission( const callsign &call )
{
  for (list<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it )
    {
      if (it->wildcard)
	{
	  if (it->local)
	    return perm_local;
	  else
	    return perm_forw;
	}
      else if (samecall(it->call,call))
	{
	  if (it->local)
	    return perm_local;
	  else
	    return perm_forw;
	}
    }
  return perm_no;
}

void board::purge( void )
{
  changed=false;
  for (int i = 0; i < 10; i++)
    {
      if ( nachrichten[i].einspielung != zeit(0) && 
         ((nachrichten[i].einspielung + (nachrichten[i].lifetime * 86400)) < zeit()) )
	{
	  changed=true;
	  syslog logf(configuration);
	  logf.eintrag("Purge: Loesche Nachricht "+nachrichten[i].msg,LOGMASK_PRGRMMDG);
	  nachrichten[i].einspielung = zeit(0);
	  nachrichten[i].lifetime = 0;
	  nachrichten[i].msg = String("");
	}
    }
}

boards::boards(  )
{
  try
    {
      brd_dir = configuration.find("RUBRIKEN");
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(configuration);
      logf.eintrag("Parameter RUBRIKEN nicht definiert.",LOGMASK_PRGRMERR);
      throw Error_could_not_create_boardnames();
    }
}

void boards::send_names( void )
{
#ifdef COMPILE_SLAVES
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL)
    {
      spool.spool_brd_name(String("BAKE"),95);
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	{
	  if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	    {
	      board brd((String) entry->d_name,configuration);
	      spool.spool_brd_name(brd.get_name(),brd.get_brd_id());
	    }
	}
      closedir(boards);
    }
#endif
}

int boards::send_messages( void )
{
#ifdef COMPILE_SLAVES
  int anz = 0;
  purge();
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL)
    {
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	{
	  if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	    {
	      board brd((String) entry->d_name,configuration);
	      int board = brd.get_brd_id();
	      if (brd.get_deflt() == 0) 
	        continue;
	      if (board != 95 && brd.ready_for_tx() )
		{
		  for (int i = 1; i <= 10 ; i++ )
		    {
		      String msg = brd.get_msg(i);
		      if (msg.slen() > 0 )
			{
			  destin ds = brd.get_destin(i);
			  spool.spool_bul(G_mycall,brd.get_time(i),board,i,msg,false,ds,196,true);
			  anz++;
			}
		    }
		  brd.set_tx();
		}
	    }
	}
      closedir(boards);
    }
  return anz;
#else
  return 0;
#endif
}

int boards::send_all_messages( void )
{
#ifdef COMPILE_SLAVES
  int anz = 0;
  purge();
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL)
    {
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	{
	  if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	    {
	      board brd((String) entry->d_name,configuration);
	      int board = brd.get_brd_id();
	      if (brd.get_deflt() == 0) 
	        continue;
	      if (board != 95)
	      {
		for (int i = 1; i <= 10 ; i++ )
		  {
		    String msg = brd.get_msg(i);
		    if (msg.slen() > 0 )
		      {
			destin ds = brd.get_destin(i);
			spool.spool_bul(G_mycall,brd.get_time(i),board,i,msg,false,ds,196,true);
			anz++;
		      }
		  }
		brd.set_tx();
	      }
	    }
	}
      closedir(boards);
    }
  return anz;
#else
  return 0;
#endif
}

void boards::purge( void )
{
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL)
    {
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	{
	  if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	    {
	      board brd((String) entry->d_name,configuration);
	      brd.purge();
	    }
	}
      closedir(boards);
    }
}

bool boards::get_board_by_id( int soll_bid, board &mbrd )
{
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL)
    {
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	{
	  if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	    {
	      board brd((String) entry->d_name,configuration);
	      if (brd.get_brd_id() == soll_bid )
		{
		  mbrd = brd;
		  return true;
		}
	    }
	}
      closedir(boards);
    }
  return false;
}

vector<String> boards::get_board_names( void ) const
{
  t_dir directory;
  DIR *boards;
  boards = opendir(strtochar(brd_dir));
  if (boards != NULL )
    {
      struct dirent *entry;
      while ((entry = readdir(boards)) != NULL)
	if ( (strcmp(entry->d_name,".") != 0) && (strcmp(entry->d_name,"..")))
	  {
	    try
	      {
		board brd(entry->d_name,configuration);
		directory[brd.get_brd_id()] = String(entry->d_name);
	      }
	    catch( Error_could_not_open_boardfile )
	      {}
	  }
      closedir(boards);
    }
  vector<String> tmp;
  for (t_dir::iterator it = directory.begin(); it != directory.end(); ++it )
    tmp.push_back(it->second);

  return tmp;
}

