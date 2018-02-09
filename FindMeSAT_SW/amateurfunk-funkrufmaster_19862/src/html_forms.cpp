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

#include "html_forms.h"



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


html_object_type html_form_text_input::object_typ( void ) const
{
  return ho_form_text_input;
}

html_form_text_input::html_form_text_input( const String &n, const String &v, int s )
{
  name = n;
  value = v;
  size = s;
}

html_form_text_input::~html_form_text_input( void )
{
}

String html_form_text_input::write( void )
{
   String out;

  out = "<input type=\"text\" ";
  out.append(String("name=\"")+name+String('\"'));
  out.append(String(" value=\"")+value+String('\"'));
  out.append(String(" size=")+itoS(size));
  out.append('>');
  
  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_password_input::object_typ( void ) const
{
  return ho_form_password_input;
}

html_form_password_input::html_form_password_input( const String &n, const String &v, int s )
{
  name = n;
  value = v;
  size = s;
}

html_form_password_input::~html_form_password_input( void )
{
}

String html_form_password_input::write( void )
{
   String out;

  out = "<input type=\"password\" ";
  out.append(String("name=\"")+name+String('\"'));
  out.append(String(" value=\"")+value+String('\"'));
  out.append(String(" size=")+itoS(size));
  out.append('>');
  
  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_submit::object_typ( void ) const
{
  return ho_form_submit;
}

html_form_submit::html_form_submit( const String &v )
{
  value = v;
}

html_form_submit::~html_form_submit( void )
{
}

String html_form_submit::write( void )
{
   String out;

  out = "<input type=\"submit\"";
  out.append(String(" value=\"")+value+String("\">"));

  return out;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_reset::object_typ( void ) const
{
  return ho_form_reset;
}

html_form_reset::html_form_reset( const String &v )
{
  value = v;
}

html_form_reset::~html_form_reset( void )
{
}

String html_form_reset::write( void )
{
   String out;

  out = "<input type=\"reset\"";
  out.append(String(" value=\"")+value+String("\">"));

  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_select::object_typ( void ) const
{
  return ho_form_select;
}

html_form_select::html_form_select( const String &n )
{
  name = n;
  selected_option = false;
  options.clear();
}

html_form_select::~html_form_select( void )
{
  options.clear();
}

void html_form_select::add_option( const String &v, const String &t, bool sel )
{
  struct option tmp;

  tmp.value = v;
  tmp.text = t;

  if (!selected_option)
    {
      tmp.selected = sel;
      selected_option = sel;
    }
  else
    tmp.selected = false;

  options.push_back(tmp);
}

void html_form_select::select_option( const String &v )
{
  for (vector<struct option>::iterator it = options.begin(); it != options.end(); ++it )
    it->selected = (it->value == v );
}

String html_form_select::write( void )
{
   String out;

  out = String("<select name=\"")+name+String("\">\n");

  for (vector<struct option>::iterator it = options.begin(); it != options.end(); ++it )
    {
      out.append(String("<option "));
      if (it->selected)
	out.append(String("selected "));
      out.append(String("value=\"")+it->value+String("\">"));
      out.append(it->text+String("</option>\n"));
    }
  out.append("</select>");

  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_radio::object_typ( void ) const
{
  return ho_form_radio;
}

html_form_radio::html_form_radio( const String &n, const String &v, const html_object &o )
{
  name = n;
  value = v;
  add(o);
}

html_form_radio::~html_form_radio( void )
{
}

String html_form_radio::write( void )
{
   String out;

  out = String("<input type=\"radio\" name=\"")+name+String("\"");
  out.append(String(" value=\"")+value+String("\">"));
  write_objs(out);
  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form_check::object_typ( void ) const
{
  return ho_form_check;
}

html_form_check::html_form_check( const String &n, const String &v, const html_object &o )
{
  name = n;
  value = v;
  add(o);
}

html_form_check::~html_form_check( void )
{
}

String html_form_check::write( void )
{
   String out;

  out = String("<input type=\"checkbox\" name=\"")+name+String("\"");
  out.append(String(" value=\"")+value+String("\">"));
  write_objs(out);
  return out;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

html_object_type html_form::object_typ( void ) const
{
  return ho_form;
}

html_form::html_form( f_method m, const String &a )
{
  met = m;
  action = a;
  target = "";
  objects.clear();
}

void html_form::set_target( const String &t )
{
  target = t;
}

String  html_form::write( void )
{

   String out;

  out = "<form method=";
  if (met == m_get)
    out.append("get");
  else
    out.append("post");

  out.append(String(" action=\"")+action+String('\"'));
  if (target.slen() > 0)
    out.append(String(" target=\"")+target+String('\"'));

  out.append(String(">\n"));

  write_objs(out);

  out.append("\n</form>\n");

  return out;
}
