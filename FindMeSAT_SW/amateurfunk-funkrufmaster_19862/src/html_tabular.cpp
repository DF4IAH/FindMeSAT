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

#include "html_tabular.h"

html_tabular::cell::cell( html_align a, bool h, const html_object & o, const String &c,int he, int w)
{
  al = a;
  mem.add(o);
  color = c;
  head = h;
  height = he;
  width = w;
}

String  html_tabular::cell::write( void )
{
   String out;

  out = String("    ");
  if (head)
    out.append("<th");
  else
    out.append("<td");
  out.append(al.write());
  if (color != "")
    out.append(String(" bgcolor=\"")+color+String("\""));
  if (height != -1)
    out.append(String(" height=\"")+itoS(height)+String("\""));
  if (width != -1)
    out.append(String(" width=\"")+itoS(width)+String("\""));
  out.append('>');
  out.append(mem.write());
  if (head)
    out.append("</th>\n");
  else
    out.append("</td>\n");

  return out;
}

html_tabular::row::row( void )
{
  cells.clear();
}

void html_tabular::row::set_align( html_valign a )
{
  valign = a;
}

void html_tabular::row::new_cell(  html_align a, bool h,const html_object &o, const String &c, int he, int w)
{
  cell ce(a,h,o,c,he,w);
  cells.push_back(ce);
}

String html_tabular::row::write( void )
{
   String out;

  out=String("  <tr")+valign.write()+String(">\n");
  for (vector<cell>::iterator it = cells.begin(); it != cells.end(); ++it)
    {
      out.append(it->write());
    }
  out.append(String("</tr>\n"));

  return out;
}

html_object_type html_tabular::object_typ( void ) const
{
  return ho_tabular;
}

html_tabular::html_tabular( void )
{
  head.clear();
  foot.clear();
  body.clear();

  akt_row = row();

  border = -1;
  spacing = -1;
  padding = -1;
  caption = "";
}

void html_tabular::new_row( void )
{
  akt_row = row();
}

void html_tabular::add_cell( html_align a, bool h,const html_object & o, String color, int he, int w)
{
  akt_row.new_cell(a,h,o,color,he,w);
}

void html_tabular::set_rowalign( html_valign a )
{
  akt_row.set_align(a);
}

void html_tabular::add_to_head( void )
{
  head.push_back(akt_row);
}

void html_tabular::add_to_foot( void )
{
  foot.push_back(akt_row);
}

void html_tabular::add_to_body( void )
{
  body.push_back(akt_row);
}

void html_tabular::set_border( int b )
{
  border = b;
}

void html_tabular::set_spacing( int b )
{
  spacing = b;
}

void html_tabular::set_padding( int b )
{
  padding = b;
}

void html_tabular::set_caption( const String &c )
{
  caption = c;
}

String  html_tabular::write( void )
{
   String out;

  out = String("<table");
  if (border != -1)
    out.append(String(" border=")+itoS(border));
  if (spacing != -1)
    out.append(String(" cellspacing=")+itoS(spacing));
  if (padding != -1)
    out.append(String(" cellpadding=")+itoS(padding));

  out.append(String(">"));

  if (caption.slen() > 0)
    out.append(String("<caption>")+caption+String("</caption>"));

  out.append(String("\n"));

  bool hf_flag = ( head.size() >0 || foot.size() > 0);
  if (hf_flag)
    {
      out.append("<thead>\n");
      for (vector<row>::iterator it = head.begin(); it != head.end(); ++it )
	out.append(it->write());
      out.append("</thead>\n");

      out.append("<tfoot>\n");
      for (vector<row>::iterator it = foot.begin(); it != foot.end(); ++it )
	out.append(it->write());
      out.append("</tfoot>\n");
    }

  if (hf_flag)
    out.append("<tbody>\n");
  for (vector<row>::iterator it = body.begin(); it != body.end(); ++it )
    out.append(it->write());
  if (hf_flag)
    out.append("</tbody>\n");
  out.append("</table>\n");

  return out;
}
