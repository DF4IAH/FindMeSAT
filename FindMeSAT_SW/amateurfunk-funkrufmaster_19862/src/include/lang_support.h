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

#ifndef __LANG_SUPPORT_H__
#define __LANG_SUPPORT_H__

#include <iostream>
#include <map>

#include "String.h"
#include "config.h"
#include "logfile.h"

using namespace std;

class Error_message_not_defined
{
#ifdef _DEBUG_EXEC_
 public:
  Error_message_not_defined()
    {
      cerr << "Error_message_not_defined" << endl;
    }
#endif
};

class Error_message_file_not_defined
{
#ifdef _DEBUG_EXEC_
 public:
  Error_message_file_not_defined()
    {
      cerr << "Error_message_file_not_defined" << endl;
    }
#endif
};

class Error_could_not_open_messagefile
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_open_messagefile()
    {
      cerr << "Error_could_not_open_messagefile" << endl;
    }
#endif
};

class Error_message_file_format_error
{
#ifdef _DEBUG_EXEC_
 public:
  Error_message_file_format_error()
    {
      cerr << "Error_message_file_format_error" << endl;
    }
#endif
};

class user_meldungen
{
 private:

  struct ltstr
    {
      bool operator()(unsigned int i1, unsigned int i2) const
      {
	return i1 < i2;
      }
    };
  //typedef array_map<String,String> t_lang;
  typedef map<unsigned int,String,ltstr> t_lang;
  t_lang Meldungen;
  String filename;
  bool loaded;
  syslog logf;


 public:
  user_meldungen(config_file & );
  void load(const String& );

  String find(unsigned int );  
};

#endif // __LANG_SUPPORT_H__
