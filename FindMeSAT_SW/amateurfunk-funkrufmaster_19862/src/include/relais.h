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

#ifndef __RELAIS_H__
#define __RELAIS_H__

#include <iostream>
#include <list>

#include "callsign.h"
#include "locator.h"

using namespace std;

class relais
{
 public:
  enum relais_typ { rt_unknown, rt_fm, rt_atv, rt_digi, rt_bake, rt_rtty, 
                    rt_transponder };

 public:
  callsign call;
  double qrg;
  locator loc;
  String qth;
  relais_typ tp;

 public:
  bool ScanFrom( istream &, const relais& );
  String get_spoolmsg( void );

  friend bool cmpdist(relais&, relais& );
  friend bool cmpqrg(  relais&, relais& );
};

struct str_cmpdist : public binary_function<relais, relais, bool> 
{
  bool operator()(relais &r1, relais &r2) 
  { 
    return cmpdist(r1,r2); 
  }
};

struct str_cmpqrg : public binary_function<relais, relais, bool> 
{
  bool operator()(relais &r1, relais &r2) 
  { 
    return cmpqrg(r1,r2); 
  }
};

class relais_database
{
 protected:
  list<relais> relaise;
  list<relais>::iterator akt;

 public:
  void load( const String &);
  void sort_dist( void );
  void sort_qrg( void );
  relais_database find_type( relais::relais_typ );
  relais_database find_qrg( double, double );
  relais_database reduce_n( int );
  bool first( relais& );
  bool next( relais& );
  void PrintOn( ostream & );
};

inline ostream& operator<<(ostream &strm, relais_database &rdb )
{
  rdb.PrintOn(strm);
  return strm;
}

void spool_database(relais_database&, int , const String& );

#endif
