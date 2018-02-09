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

#ifndef __IMPORT_H__
#define __IMPORT_H__

#include <iostream>
#include <fstream>
#include <sstream>

#include "String.h"
#include "logfile.h"
#include "bake.h"
#include "config.h"
#include "user.h"
#include "user_interface.h"

using namespace std;

class import_system
{
 protected:
  String import_dir_name;
  bool activ;
  syslog logf;
  t_baken baken;
  bool sysop_flag;

 protected:
  void delete_import_file( String& );
  void read_import_file( String&, bool );

 public:
  import_system(  config_file&, t_baken& );
  void import( void );
};

#endif
