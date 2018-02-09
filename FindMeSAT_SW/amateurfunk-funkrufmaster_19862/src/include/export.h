/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
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

#ifndef __HTML_EXPORT_H__
#define __HTML_EXPORT_H__

#include <iostream>
#include <fstream>

#include "String.h"
#include "wx.h"
#include "callsign.h"
#include "adress.h"
#include "destin.h"
#include "zeit.h"


#define EXPORT_MASK_NO         0
#define EXPORT_MASK_DX         1
#define EXPORT_MASK_WX         2
#define EXPORT_MASK_PRIV       4
#define EXPORT_MASK_BUL        8
#define EXPORT_MASK_CONDX     16

#define EXPORT_MASK_ID_DX      'D'
#define EXPORT_MASK_ID_WX      'W'
#define EXPORT_MASK_ID_PRIV    'P'
#define EXPORT_MASK_ID_BUL     'B'
#define EXPORT_MASK_ID_CONDX   'C'


class Error_could_not_initialize_exportsystem
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_initialize_exportsystem()
    {
      cerr << "Error_could_not_initialize_exportsystem" << endl;
    }
#endif
};

class Error_could_not_open_export_file
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_export_file()
    {
      cerr << "Error_could_not_open_export_file" << endl;
    }
#endif
};

class Error_wrong_id_in_exportmask
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_id_in_exportmask()
    {
      cerr << "Error_wrong_id_in_exportmask" << endl;
    }
#endif
};

class Error_in_exportmask_syntax
{
#ifdef _DEBUG_EXEC_
 public:
  Error_in_exportmask_syntax()
    {
      cerr << "Error_in_exportmask_syntax" << endl;
    }
#endif
};

class export_sys
{
 protected:
  unsigned int maske;
  String exportpfad;

 protected:
  void get_file( ofstream&, char );

 public:
  export_sys( void );

  String get_mask( void );
  void set_mask( String  );

#ifdef COMPILE_CLUSTER
  void write_dx( const String& );
  void write_condx( const String& );
#endif
#ifdef COMPILE_WX
  void write_wx( const wx_meldung & );
#endif
  void write_user(const callsign&, const callsign&, zeit, adress, String, bool, const destin&, unsigned int );
  void write_bul( const callsign&, zeit, int, int, String, bool, const destin&, unsigned int, bool = false);
};





#endif // __HTML_EXPORT_H__
