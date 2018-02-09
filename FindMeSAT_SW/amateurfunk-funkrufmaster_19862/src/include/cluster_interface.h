/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002,2003 by Holger Flemming                               *
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
 *  		                                			    *
 ****************************************************************************/

#ifndef __CLUSTER_INTERFACE_H__
#define __CLUSTER_INTERFACE_H__

#include <iostream>

using namespace std;

#include "String.h"
#include "interfaces.h"
#include "zeit.h"
#include "bake.h"
//#include "destin.h"
#include "cluster.h"

class Error_could_not_init_cluster_interface
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_init_cluster_interface()
    {
      cerr << "Error_could_not_initialize_cluster_interface" << endl;
    }
#endif
};

class cluster_interface : public interfaces
{
 protected:
  destin ds;
  bool activ;
  dx_cluster_typ dx_typ;
  zeit last_activity;

 protected:
  void dx_line(String );
  void dx_wx_line( String );
  void gpage_Aurora(String);
  t_baken baken;

 public:
  cluster_interface(String &, bool, dx_cluster_typ, const connect_string& );
  cluster_interface( void );
  ~cluster_interface();
  bool do_process( bool, String & );
};
class cluster_if
{
};


#endif
