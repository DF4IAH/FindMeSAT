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

#ifndef __TRACE_H__
#define __TRACE_H__

#include <vector>

#include "String.h"

using namespace std;

class trace
{
 protected:
  bool activ, busy;
  int traced_thread;

  vector<String> puffer;

 public:
  trace( void );
  void input(int, const String& );
  void output(int, const String& );
  void close( int );

  bool new_trace( int );
  bool trace_activ( void );
  bool line_avalable( void );
  String get_line( void );
  void stop_trace( void );
};


#endif  // __TRACE_H__
