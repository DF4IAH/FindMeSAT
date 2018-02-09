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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include "fwd_nachrichten.h"

#include <iostream>

using namespace std;

class spoolstatistic
{
 protected:
  int local_private[240];
  int local_bul[240];
  int fwd_private[240];
  int fwd_bul[240];
  int last_index;
  char cr;

  int get_index( void );

 public:
  spoolstatistic( void );
  void spool(bool local, bool priv);
  int get_local_private( bool );
  int get_local_bul( bool );
  int get_fwd_private( bool );
  int get_fwd_bul( bool );
  void printOn(ostream& );
  inline void set_cr(char ch)
    {
      cr = ch;
    }
  String page_msg(void );
};

class fwdstatistic
{
 protected:
  int in_fr[240];
  int in_db[240];
  int in_zeit[240];
  int in_dest[240];

  int out_fr[240];
  int out_db[240];
  int out_zeit[240];
  int out_dest[240];

  int last_index;
  char cr;

  int get_index( void );

 public:
  fwdstatistic( void );
  void spool( bool, n_types);  // bool : in_out_flag, in = true
  int get_in_fr( bool );
  int get_in_db( bool );
  int get_in_zeit( bool );
  int get_in_dest( bool );

  int get_out_fr( bool );
  int get_out_db( bool );
  int get_out_zeit( bool );
  int get_out_dest( bool );

  void printOn(ostream& );
  inline void set_cr(char ch)
    {
      cr = ch;
    }
};

inline ostream& operator<<( ostream &strm, spoolstatistic &s )
{
  s.printOn(strm);
  return strm;
}

inline ostream& operator<<( ostream &strm, fwdstatistic &s )
{
  s.printOn(strm);
  return strm;
}


#endif
