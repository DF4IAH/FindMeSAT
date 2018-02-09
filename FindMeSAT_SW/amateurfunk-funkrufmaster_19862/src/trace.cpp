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


#include "trace.h"

trace::trace( void )
{
  activ = false;
  busy = false;
  traced_thread = 0;
  puffer.clear();
}

void trace::input( int thread_id, const String &line )
{
  if (activ && thread_id == traced_thread )
    {
      puffer.push_back(String("<--  : ")+line);
    }
}

void trace::output( int thread_id, const String &line )
{
  if (activ && thread_id == traced_thread )
    {
      puffer.push_back(String("-->  : ")+line);
    }
}

void trace::close( int thread_id )
{
  if (activ && thread_id == traced_thread )
    {
      puffer.push_back("Thread Disconnected.");
      activ = false;
    }
}

bool trace::new_trace( int thread_id )
{
  if (!busy)
    {
      activ = true;
      busy = true;
      traced_thread = thread_id;
      puffer.clear();
      return true;
    }
  else
    return false;
}

bool trace::trace_activ( void )
{
  return activ;
}

bool trace::line_avalable( void )
{
  return puffer.size() != 0;
}

String trace::get_line( void )
{
  String tmp;
  if (puffer.size() != 0)
    {
      vector<String>::iterator it = puffer.begin();
      if (it != puffer.end() )
	{
	  tmp = *it;
	  puffer.erase(it);
	}
    }
  return tmp;
}

void trace::stop_trace( void )
{
  activ = false;
  busy = false;
  traced_thread = 0;
  puffer.clear();
}
