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

#include "html_references.h"


html_object_type html_link::object_typ( void ) const
{
  return ho_link;
}

html_link::html_link( const String&u, const String&t, const html_object &o )
{
  url = u;
  target = t;
  add(o);
}

String html_link::write( void )
{
   String out;

  out = String("<a href=\"")+url+String("\"");
  if (target.slen() > 0)
    out.append(String(" target=\"")+target+String("\""));
  out.append(String(">"));
  write_objs(out);
  out.append("</a>");
  return out;
}
