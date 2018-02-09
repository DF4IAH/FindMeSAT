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

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include <list>

using namespace std;

#include "globdef.h"
#include "zeit.h"
#include "callsign.h"
#include "connect_string.h"

enum dx_cluster_typ { dx_raw, dx_pav, dx_clx, dx_sorted };

struct dx_cluster
{
  callsign dx_cl;
  connect_string dx_cluster_pfad;
  dx_cluster_typ typ;
};

class cluster_control
{
 public:
  enum dx_conn_status { st_getrennt, st_aufbau, st_aktiv, st_gescheitert };
 private:
  list<dx_cluster> cluster;
  list<dx_cluster>::iterator cluster_iterator;
  dx_conn_status status;
  zeit last_status_change;
  int scheiter_zaehler;
  bool activ;

  int wait_time( void  );

 public:
  cluster_control( void );
  ~cluster_control( void );
  void init( void );

  bool cluster_connect( callsign&, connect_string&, dx_cluster_typ& );
  void aufbau( void );
  void gescheitert( void );
  void established( void );
  void getrennt( void );
  void enable( void );
  void disable( void );
  bool add(const callsign&, const connect_string&, dx_cluster_typ );
  bool del(const callsign& );
  void PrintOn( ostream &, char = '\n' );
  inline bool enabled( void )
    {
      return activ;
    }
};

inline ostream & operator<<( ostream &strm, cluster_control &cl )
{
  cl.PrintOn(strm);
  return strm;
}


#endif // __CLUSTER_H__
