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

#include "mid.h"
#include "callsign.h"
#include "globdef.h"

extern callsign G_mycall;

mid::mid(const String &filename, int tab_size )
{
  mids_unsorted.clear();
  mids_sorted.clear();

  midlistfile = filename;
  max_size = tab_size;

  load();
}

mid::mid( void )
{
  mids_unsorted.clear();
  mids_sorted.clear();

  midlistfile = "";
  max_size = 8096;
}

mid::~mid( void )
{
  save();
  clear();
}

void mid::load( void )
{
  mids_unsorted.clear();
  mids_sorted.clear();

  if ( midlistfile != "" )
    {
      ifstream mid_file(strtochar(midlistfile));
      if (mid_file)
	while (!mid_file.eof())
	  {
	    Mid ak_mid;
	    
	    mid_file >> ak_mid;
	    
	    mids_unsorted.push_back(ak_mid);
	    mids_sorted.insert(ak_mid);	
	  }
    }
}

void mid::save( void )
{
  if (midlistfile != "" )
    {
      ofstream midfile(strtochar(midlistfile));
      
      t_mids_unsorted::iterator it;
      for (it = mids_unsorted.begin() ; it != mids_unsorted.end() ; ++it )
	midfile << *it << endl;
    }
}

void mid::clear( void )
{
  mids_unsorted.clear();
  mids_sorted.clear();
}

bool mid::check_mid( Mid &m )
{
  t_mids_sorted::iterator it = mids_sorted.find(m);
  if (it == mids_sorted.end() )
    {
      if (mids_unsorted.size() > max_size) 
	{
	  t_mids_unsorted::iterator it1 = mids_unsorted.begin();
	  t_mids_sorted::iterator it2 = mids_sorted.find(*it1);

	  mids_unsorted.erase(it1);
	  mids_sorted.erase(it2);
	}
      mids_unsorted.push_back(m);
      mids_sorted.insert(m);

      if ( ( entries_without_save++ > MAX_MID_ENTRIES_WITHOUT_SAVE ) ||
	   ( zeit() - last_save > MAX_TIME_BETWEEN_MID_SAVE ) )
	{
	  save();
	  entries_without_save = 0;
	  last_save = zeit();
	}	

      return false;
    }
  else
    return true;
}

Mid mid::get_newmid( void )
{
  time_t zeit = time(NULL);
  Mid m;

  bool flag = true;
  int i = 1;
  while (i < 200 && flag )
    {
      m = itoS(zeit,0,true) + itoS(i++) + G_mycall.call();
      m.kuerze();
      flag = check_mid(m);
    }
  if (flag) throw Error_could_not_gen_new_mid();
  return m;
}
