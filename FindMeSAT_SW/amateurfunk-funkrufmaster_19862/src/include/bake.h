/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002 by Holger Flemming                                    *
 *                                                                          *
 * This Program is free software; you can redistribute it and/or modify     *
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

#ifndef __BAKE_H__
#define __BAKE_H__

#include <vector>

using namespace std;

#include "autoconf.h"
#include "String.h"
#include "makros.h"
#include "config.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif



class Error_could_not_create_beacon
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_create_beacon()
    {
      cerr << "Error_could_not_create_beacon" << endl;
    }
#endif
};

// In den Bakenkonstruktoren kann die Ausnahme
// Error_could_not_create_beacon
// entstehen

class bake
{
 public:
  enum b_typ { b_kein, b_tx, b_status };

 protected:
  String baken_text;
  makros macros;

 public:
  bake(  );
  virtual ~bake( void );
  virtual void send( void );
  virtual void PrintOn( ostream& );
  virtual void show( ostream& , char );
  virtual b_typ get_typ( void );
};

typedef bake* t_pt_bake;
typedef vector<t_pt_bake> t_baken;
//typedef vector<bake> t_baken;

class tx_bake : public bake
{
 protected:
  int slot;
  int b_num;
  int num;
  String pfad;
  
 public:
  tx_bake( config_file & , int  );
  tx_bake( istream&, int, const String& );
  tx_bake( int, int, const String&, const String& );
  virtual void send( void );
  virtual void PrintOn( ostream& );
  virtual void show( ostream& , char );
  virtual b_typ get_typ( void );
  inline int get_slot( void )
    {
      return slot;
    }
};

class status_bake : public bake
{
  
 public:
  status_bake( config_file& );
  virtual void send( void );
  virtual void PrintOn( ostream& );
  virtual void show( ostream& , char );
  virtual b_typ get_typ( void );
};

void show_baken(t_baken&, ostream&, char );

#endif
