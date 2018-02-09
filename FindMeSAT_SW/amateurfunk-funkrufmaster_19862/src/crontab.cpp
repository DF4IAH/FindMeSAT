/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; yopu can redistribute ist and/or modify   *
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
 *                                                                          *
 ****************************************************************************/

#include "crontab.h"

#include <fstream.h>
#include <stdio.h>
#include <time.h>

#include "globdef.h"
#include "config.h"
#include "logfile.h"

crontab_time::crontab_time( void )
{
  mins.clear();
  stds.clear();
  wts.clear();
  tgs.clear();
  mons.clear();
}

bool crontab_time::operator== ( zeit & z )
{
  time_t t = z.get_systime();
  struct tm *tstruct = gmtime(&t);
  if (tstruct != NULL )
    {
      bool flag1 = true;
      bool flag2 = false;
      if (mins.size() > 0)
	{
	  for (vector<int>::iterator i = mins.begin() ; i != mins.end() ; i++ )
	    if (*i == tstruct->tm_min) flag2 = true;
	  flag1 = flag1 && flag2;
	}

      flag2 = false;
      if (stds.size() > 0)
	{
	  for (vector<int>::iterator i = stds.begin() ; i != stds.end() ; i++ )
	    if (*i == tstruct->tm_hour) flag2 = true;
	  flag1 = flag1 && flag2;
	}

      flag2 = false;
      if (wts.size() > 0)
	{
	  for (vector<int>::iterator i = wts.begin() ; i != wts.end() ; i++ )
	    if (*i == tstruct->tm_wday) flag2 = true;
	  flag1 = flag1 && flag2;
	}

      flag2 = false;
      if (tgs.size() > 0)
	{
	  for (vector<int>::iterator i = tgs.begin() ; i != tgs.end() ; i++ )
	    if (*i == tstruct->tm_mday) flag2 = true;
	  flag1 = flag1 && flag2;
	}

      flag2 = false;
      if (mons.size() > 0)
	{
	  for (vector<int>::iterator i = mons.begin() ; i != mons.end() ; i++ )
	    if (*i-1 == tstruct->tm_min) flag2 = true;
	  flag1 = flag1 && flag2;
	}
      return flag1;
    }
  else
    throw Error_no_utc_avalable();

}

vector<int> get_nums( istream& strm )
{
  char puffer[100] = "";
  char *ptr = puffer;
  char ch = ' ';
  int i = 1;
  vector<int> ints;

  ints.clear();
  while ( strm && ch == ' ') strm.get(ch);	// Leerzeichen uebergehen

  while ( strm && ch != ' ' && ch != ',') 
    {
      *(ptr++) = ch;
      if (i++ > 99) throw Error_crontab_number_read_error();
      strm.get(ch);
    }
  strm.putback(ch);
  *ptr= '\0';

  if (*puffer == '*')
    return ints;

  String pu1(puffer);
  unsigned int l = pu1.slen();
  for (unsigned j=0; j< l; j++) // Steht ein Schraegstrich im Text
    {
      if(pu1[j]=='/')
	{
	  String pu2;
	  unsigned int i1 = pu1.pos('-');
	  unsigned int i2 = j;
	  if ( pu1.slen() >= i2 && i2 > i1 && i1 > 0)
	    {
	      pu2=pu1.copy(0, i1);
	      int low=pu2.Stoi();

	      pu2 = pu1.copy(i1+1,i2-i1-1);
	      int high = pu2.Stoi();

	      pu2 = pu1.copy(i2+1,l-i2-1);
	      int delta=pu2.Stoi();

	      for (i=low; i<=high; i+=delta)
		ints.push_back(i);
	    }
	  return ints;
	}
    }

  ints.push_back(pu1.Stoi());
  return ints;
}



istream& operator>> ( istream &strm, crontab_time & ct )
{
  bool flag = true;
  char ch = ' ';
  vector<int> ints;
  while (flag)	
    {
      ints = get_nums(strm);
      if (ints.size() > 0)
	for (vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
	  ct.mins.push_back(*i);

      strm.get(ch);
      flag = ch == ',';
    }
    
  flag = true;  
  while (flag)	
    {
      ints = get_nums(strm);
      if (ints.size() > 0)
	for (vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
	  ct.stds.push_back(*i);

      strm.get(ch);
      flag = ch == ',';
    }
  
  flag = true;
  while (flag)	
    {
      ints = get_nums(strm);
      if (ints.size() > 0)
	for (vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
	  ct.wts.push_back(*i);

      strm.get(ch);
      flag = ch == ',';
    }
    
  flag = true;
  while (flag)	
    {
      ints = get_nums(strm);
      if (ints.size() > 0)
	for (vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
	  ct.tgs.push_back(*i);

      strm.get(ch);
      flag = ch == ',';
    }
  
  flag = true;
  while (flag)	
    {
      ints = get_nums(strm);
      if (ints.size() > 0)
	for (vector<int>::iterator i = ints.begin(); i != ints.end(); ++i)
	  ct.mons.push_back(*i);

      strm.get(ch);
      flag = ch == ',';
    }

  return strm;
}

ostream& operator<<( ostream& strm, crontab_time& t )
{
  if (t.mins.size() > 0)
    {
      vector<int>::iterator it = t.mins.begin();
      while (it != t.mins.end())
	{
	  strm << *it;
	  if (++it != t.mins.end() ) strm <<',';
	}
      strm << " ";
    }
  else 
    strm << "* ";
  if (t.stds.size() > 0)
    {
      vector<int>::iterator it = t.stds.begin();
      while (it != t.stds.end())
	{
	  strm << *it;
	  if (++it != t.stds.end() ) strm <<',';
	}
      strm << " ";
    }
  else
    strm << "* ";
  if (t.wts.size() > 0)
    {
      vector<int>::iterator it = t.wts.begin();
      while (it != t.wts.end())
	{
	  strm << *it;
	  if (++it != t.wts.end() ) strm <<',';
	}
      strm << " ";
    }
  else
    strm << "* ";
  if (t.tgs.size() > 0)
    {
      vector<int>::iterator it = t.tgs.begin();
      while (it != t.tgs.end())
	{
	  strm << *it;
	  if (++it != t.tgs.end() ) strm <<',';
	}
      strm << " ";
    }
  else
    strm << "* ";
  if (t.mons.size() > 0)
    {
      vector<int>::iterator it = t.mons.begin();
      while (it != t.mons.end())
	{
	  strm << *it;
	  if (++it != t.mons.end() ) strm <<',';
	}
      strm << " ";
    }
  else
    strm << "* ";
  return strm;
}

crontab_command::crontab_command( ct_cmd_typ cmd )
{
  typ = cmd;
  cmd_string = "";
}

istream& operator>>( istream& str, crontab_command &cc )
{
  char ch = ' ';
  while (str && ch == ' ') str.get(ch);
  str.putback(ch);
  str >> cc.cmd_string;
  cc.cmd_string.kuerze();
       if (cc.cmd_string == String("bake")) cc.typ = ct_cmd_bake;
  else if (cc.cmd_string == String("boards")) cc.typ = ct_cmd_boards;
  else if (cc.cmd_string == String("messages")) cc.typ = ct_cmd_brdmsg;
  else if (cc.cmd_string == String("save")) cc.typ = ct_cmd_dbsave;
  else if (cc.cmd_string == String("sat")) cc.typ = ct_cmd_sat;
  else if (cc.cmd_string == String("statistik")) cc.typ = ct_cmd_statistics;
  else if (cc.cmd_string == String("wx")) cc.typ = ct_cmd_wx;
  else if (cc.cmd_string == String("gezeiten")) cc.typ = ct_cmd_gezeiten;
  else if (cc.cmd_string == String("astro")) cc.typ = ct_cmd_astro;
  else if (cc.cmd_string == String("purge")) cc.typ = ct_cmd_purge;
  else if (cc.cmd_string == String("digi")) cc.typ = ct_cmd_digi;
  else cc.typ = ct_cmd_user_defined;

  return str;
}

ostream& operator<<( ostream& strm, crontab_command &cc )
{
  switch(cc.typ)
    {
      case ct_cmd_no           : strm << "NO";
                                 break;
      case ct_cmd_user_defined : strm << "USER: " << cc.cmd_string;
                                 break;
      case ct_cmd_bake         : strm << "BAKE";
                                 break;
      case ct_cmd_boards       : strm << "BOARDS";
	                         break;
      case ct_cmd_brdmsg       : strm << "MESSAGES";
	                         break;
      case ct_cmd_dbsave       : strm << "SAVE";
	                         break;
      case ct_cmd_sat          : strm << "SAT";
	                         break;
      case ct_cmd_statistics   : strm << "STATISTIK";
	                         break;
      case ct_cmd_wx           : strm << "WX";
	                         break;
      case ct_cmd_gezeiten     : strm << "GEZEITEN";
	                         break;
      case ct_cmd_purge        : strm << "PURGE";
	                         break;
      case ct_cmd_digi         : strm << "DIGI";
	                         break;
      case ct_cmd_astro        : strm << "ASTRO";
    }
  return strm;
}

crontab_entry::crontab_entry( void )
{
}

bool crontab_entry::get_entry( istream &strm )
{
  char puffer[1000];

  strm.getline(puffer,999);
  if (*puffer == '#' || *puffer == '\0')
    return false;
  else
    {
      istringstream ln(puffer);
      ln >> time >> Kommando;
      return true;
    }
}

ostream& operator<< ( ostream & strm, crontab_entry &ent )
{
  strm << ent.time << "     " << ent.Kommando;
  return strm;
}

crontab_command crontab_entry::test_entry( void )
{
  zeit zt;
  if (time == zt )
    return Kommando;
  else
    return crontab_command(ct_cmd_no);
}

crontab::crontab( config_file &cfg )
{
  entries.clear();
  commands.clear();
  try
    {
      String crontab_file = cfg.find("CRONTAB");
      ifstream ct_file(strtochar(crontab_file));
      while (ct_file)
	{
	  crontab_entry entry;
	  if (entry.get_entry(ct_file))
	    entries.push_back(entry);
	}
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(cfg);
      logf.eintrag("Parameter CRONTAB nicht definiert.",LOGMASK_PRGRMERR);
    }
}

const vector<crontab_command> & crontab::test_crontab( void )
{
  commands.clear();
  zeit t;
  t = !t;
  if (last_check < t)
    {
      last_check = t;
      for (vector<crontab_entry>::iterator it = entries.begin(); it != entries.end(); it++)
	{
	  crontab_command cmd = it->test_entry();
	  if (cmd.get_typ() != ct_cmd_no)
	    commands.push_back(cmd);
	}
    }
  return commands;
}

ostream& operator<< ( ostream& strm, crontab &ct )
{
  for( vector<crontab_entry>::iterator it = ct.entries.begin() ; it != ct.entries.end() ; it++ )
    strm << *it << endl;
  return strm;
}
