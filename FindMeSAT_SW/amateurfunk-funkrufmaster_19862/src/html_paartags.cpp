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
 *                                                                          *
 *                                                                          *
 ****************************************************************************/


#include "html_paartags.h"

html_object_type html_paartag::object_typ( void ) const
{
  return ho_paartag;
}

html_paartag::html_paartag( const String &t )
{
  tag = t;
}

String  html_paartag::write( void )
{
   String out;

  out = String("<")+tag+String(">");
  for (vector<html_object*>::iterator it = objects.begin(); it != objects.end(); ++it )
    {
      html_object *optr = *it;
      out.append(optr->write());
    }
  out.append(String("</")+tag+String(">"));

  return out;

}

