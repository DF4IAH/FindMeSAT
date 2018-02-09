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
 *  		                                			    *
 ****************************************************************************/

#ifndef __WX_INTERFACE_H__
#define __WX_INTERFACE_H__

#include <iostream>

#include "interfaces.h"
#include "String.h"
#include "wx.h"

using namespace std;


#define MASKE_TEMPERATUR      1
#define MASKE_LUFTDRUCK       2
#define MASKE_LUFTFEUCHTE     4
#define MASKE_WINDRICHTUNG    8
#define MASKE_WINDSPEED      16
#define MASKE_BOEN           32
#define MASKE_RAIN1          64
#define MASKE_RAIN4         128
#define MASKE_RAINM         256

#define MASKE_ALL           511

class wx_interface : public interfaces
{
 protected:
  wx_config_file wx_cfg; // Hier werden die Daten der Konfiguration gespeichert
  wx_meldung mdg;
  unsigned int maske;
  destin ds;
  zeit last_activity;

 protected:
  bool check_tag( const String &, const String & );
  String get_part(const String &, const String & );
  void wx_line(String);
  void set_maske( void );

 public:
  wx_interface(String &, bool, wx_config_file& );
  ~wx_interface();
  bool do_process( bool, String & );
};

#endif
