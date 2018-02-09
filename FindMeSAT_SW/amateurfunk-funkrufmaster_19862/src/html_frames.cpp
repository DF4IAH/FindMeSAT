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

#include "html_frames.h"
#include "html_references.h"


html_frames::html_frame::html_frame( int s, bool p, enum t_scroll sc, const String &sr, const String &n )
{
  size = s;
  pixel = p;
  scrolling = sc;
  src = sr;
  name = n;
}

String  html_frames::html_frame::write( void )
{
   String out;

  out = String("  <frame ");
  out.append(String(" scrolling "));
  switch(scrolling)
    {
      case sc_no   : out.append(String("no"));
                     break;
      case sc_yes  : out.append(String("yes"));
                     break;
      case sc_auto : out.append(String("auto"));
                     break;
    }
  out.append(String(" src=\"")+src+String("\""));
  out.append(String(" name=\"")+name+String("\">"));

  return out;
}

String  html_frames::html_frame::write_size( void )
{
   String out;

  if (size == -1)
    out = String("*");
  else
    {
      out = itoS(size);
      if (!pixel)
	out.append(String('%'));
    }

  return out;
}

html_object_type html_frames::object_typ( void ) const
{
  return ho_frames;
}

html_frames::html_frames( bool r )
{
  frame_eintraege.clear();
  alt_src = "";
  row_flag = r;
}

void html_frames::new_frame( int s, bool p, enum t_scroll sc, const String &sr, const String &n )
{
  html_frame f(s,p,sc,sr,n);
  frame_eintraege.push_back(f);
}

void html_frames::set_alt_src( const String &s )
{
  alt_src = s;
}

String  html_frames::write( void )
{
   String out;

  out = String("<frameset ");
  if (row_flag)
    out.append(String("rows"));
  else
    out.append(String("columns"));

  out.append("=\"");
  vector<html_frame>::iterator it = frame_eintraege.begin();
  while (it != frame_eintraege.end())
    {
      out.append(it->write_size());
      if (++it != frame_eintraege.end())
	out.append(String(','));
    }
  out.append(String("\">\n"));
  for (it = frame_eintraege.begin(); it != frame_eintraege.end(); ++it )
    out.append(it->write()+String('\n'));

  out.append(String("</frameset>\n"));
  
  return out;
}

html_object_type html_noframe::object_typ( void ) const
{
  return ho_noframe;
}

html_noframe::html_noframe( void )
{
}

String  html_noframe::write( void )
{
   String out;

  out = "<noframe>\n";
  write_objs(out);
  out.append("\n</noframe>\n");

  return out;
}
