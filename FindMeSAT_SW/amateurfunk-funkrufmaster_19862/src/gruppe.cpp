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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "gruppe.h"
#include "config.h"
#include "logfile.h"

extern callsign_database calls;
extern config_file configuration;

gruppe::gruppe( void )
{
  beschreibung = "";
  rufzeichenliste.clear();
  index = rufzeichenliste.end();
  filename = "";
  name = "";
  changed = false;
  dataset = false;
}

gruppe::gruppe( const String &pfad, const String &gname, const String &beschr )
{
  name = gname;
  filename = pfad+gname;
  ifstream strm(strtochar(filename));
  dataset = true;
  if (!strm)
    {
      beschreibung = beschr;
      rufzeichenliste.clear();
      index = rufzeichenliste.end();
      changed = true;
    }
  else
    {
      char puffer[256];
      beschreibung = "";
      rufzeichenliste.clear();
      index = rufzeichenliste.end();
      while (strm.getline(puffer,255))
	{
	  if (*puffer == '#' || *puffer == '\0') ; // Kommentarzeile oder Leerzeile
	  else
	    if (*puffer == '~')
	      beschreibung = String(puffer+1);
	    else
	      rufzeichenliste.push_back(callsign(String(puffer)));
	}
      changed = false;
    }
}


gruppe::~gruppe( void )
{
  if (changed && dataset)
    {
      ofstream strm(strtochar(filename), ios::out | ios::trunc);
      if (strm)
	{
	  strm << "# Funkruf-Gruppe" << endl;
	  strm << "# Dieses File wurde automatisch erstellt, bitte nicht" << endl;
	  strm << "# von Hand editieren." << endl;
	  strm << '~' << beschreibung << endl;
	  for (index = rufzeichenliste.begin() ; index != rufzeichenliste.end() ; ++index)
	    strm << *index << endl;
	}
      else 
	throw Error_could_not_open_groupfile();
    }
  rufzeichenliste.clear();
}


void gruppe::add_call( callsign &c )
{
  rufzeichenliste.push_back(c);
  changed = true;
}

void gruppe::change_beschr( const String &beschr )
{
  beschreibung = beschr;
  changed = true;
}

void gruppe::find_call( callsign &c )
{
  t_rufzeichenliste::iterator beg = rufzeichenliste.begin();
  t_rufzeichenliste::iterator en = rufzeichenliste.end();
  t_rufzeichenliste::iterator it = find(beg,en,c);
  if (it != en)
    throw Error_Callsign_already_in_group();
}


void gruppe::del_call( callsign &c )
{
  t_rufzeichenliste::iterator beg = rufzeichenliste.begin();
  t_rufzeichenliste::iterator en = rufzeichenliste.end();
  t_rufzeichenliste::iterator it = find(beg,en,c);
  if (it != en)
    {
      rufzeichenliste.erase(it);
      changed = true;
    }
  else
    throw Error_Callsign_not_in_group();
}

callsign gruppe::get_first( void ) 
{
  index = rufzeichenliste.begin();
  if (index != rufzeichenliste.end())
    return *index;
  else
    throw Error_no_more_call_in_group();
}

callsign gruppe::get_next( void ) 
{
  if (index != rufzeichenliste.end())
    {
      index++;
      if (index != rufzeichenliste.end())
	return *index;
      else
	throw Error_no_more_call_in_group();
    }
  else
    throw Error_no_more_call_in_group();
}

gruppen::gruppen( config_file &cfg )
{
  try
    {
      gdb_pathname = cfg.find("GRUPPEN");
    }
  catch( Error_parameter_not_defined )
    {
      throw Error_could_not_open_groups();
    }
}


void gruppen::add_group( const String &gname, const String &bsr )
{
  if (gname == "")
    throw Error_no_group_name();
  else
    {
      String gdb_filename = gdb_pathname+gname;
      ifstream strm( strtochar(gdb_filename) );
      if (!strm)
	gruppe(gdb_pathname,gname,bsr);
      else
	throw Error_group_already_exist();
    }
}

void gruppen::del_group( const String &gname )
{
  if (gname == "")
    throw Error_no_group_name();
  else
    {
      String gdb_filename = gdb_pathname+gname;
      ifstream strm( strtochar(gdb_filename) );
      if (strm)
	{
	  gruppe grp(gdb_pathname,gname);
	  if (grp.get_anzahl() == 0)
	    remove( strtochar(gdb_filename) );
	  else
	    throw Error_group_not_empty();
	}
      else 
	throw Error_group_does_not_exist();
    }
}

gruppe gruppen::find( const String &gname )
{
  if (gname == "")
    throw Error_no_group_name();
  else
    {
      String gdb_filename = gdb_pathname+gname;
      ifstream strm( strtochar(gdb_filename) );
      if (strm)
	return gruppe(gdb_pathname,gname);
      else
	throw Error_group_does_not_exist();
    }
}

vector<struct gruppe_info> gruppen::get_infos( void )
{
  DIR *grp_dir = opendir(strtochar(gdb_pathname));
  struct dirent *eintrag;
  vector<struct gruppe_info> tmp;

  while ((eintrag  = readdir(grp_dir)) != NULL )
    {
      if ( (strcmp(eintrag->d_name,".") != 0) && (strcmp(eintrag->d_name,"..")) )
	{
	  struct gruppe_info info;
	  info.name = String(eintrag->d_name);
	  gruppe grp(gdb_pathname,eintrag->d_name);
	  info.anz = grp.get_anzahl();
	  info.info = grp.get_info();
	  tmp.push_back(info);
	}
    }
  closedir(grp_dir);
  
  return tmp;
}
