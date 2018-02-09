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


#include "html_objects.h"

#include "html_align.h"
#include "html_forms.h"
#include "html_frames.h"
#include "html_paartags.h"
#include "html_references.h"
#include "html_tabular.h"

#include "stdlib.h"


String line_text2html( const String &line )
{
  String tmp = "";


  for (unsigned int i = 0; i < line.slen(); i++ )
    {
      char ch = line[i];
      switch(ch)
	{
	  case 'ä' : tmp.append("&auml;");
	             break;
	  case 'ö' : tmp.append("&ouml;");
	             break;
	  case 'ü' : tmp.append("&uuml;");
	             break;
	  case 'ß' : tmp.append("&szlig;");
	             break;
	  case 'Ä' : tmp.append("&Auml;");
	             break;
	  case 'Ö' : tmp.append("&Ouml;");
	             break;
	  case 'Ü' : tmp.append("&Uuml;");
	             break;
	  case '<' : tmp.append("&lt;");
	             break;
	  case '>' : tmp.append("&gt;");
	             break;
	  case '&' : tmp.append("&amp;");
	             break;
	  default  : tmp.append(ch);
	}
    }
  return tmp;
}

html_object_type html_object::object_typ( void ) const
{
  return ho_object;
}

html_object::html_object( void )
{
  text = "";
  objects.clear();
}

html_object::html_object( String t )
{
  text = t;
  objects.clear();
}

html_object::html_object( const char  *t )
{
  text = String(t);
  objects.clear();
}

html_object::html_object( char  c )
{
  text = String(c);
  objects.clear();
}

html_object::html_object( const html_object & obj )
{
  text = obj.text;
  vector<html_object*> tmp = obj.objects;

  for (vector<html_object*>::iterator it = tmp.begin(); it != tmp.end(); ++it )
    add(*(*it));
}

html_object::~html_object( void )
{
  for (vector<html_object*>::iterator it = objects.begin(); it != objects.end(); ++it )
    {
      free( (void*) *it);
    }
  objects.clear();
}

const html_object& html_object::operator=( const html_object & obj )
{
  if (this != &obj)
    {
      text = obj.text;
      vector<html_object*> tmp = obj.objects;
      for (vector<html_object*>::iterator it = tmp.begin(); it != tmp.end(); ++it )
	add(*(*it));
    }

  return *this;
}

void html_object::add( const html_object &obj )
{
  html_object_type objtyp = obj.object_typ();
  html_object *optr;
  switch(objtyp)
    {
    case ho_none:
      optr = NULL;
      break;
    case ho_align  : 
      {
	html_align *tmp = new html_align(*( html_align* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_valign : 
      {
	html_valign *tmp = new html_valign(*( html_valign* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_text_input : 
      {
	html_form_text_input *tmp = new html_form_text_input(*( html_form_text_input* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_password_input : 
      {
	html_form_password_input *tmp = new html_form_password_input(*( html_form_password_input* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_submit : 
      {
	html_form_submit *tmp = new html_form_submit(*( html_form_submit* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_reset: 
      {
	html_form_reset *tmp = new html_form_reset(*( html_form_reset* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_select : 
      {
	html_form_select *tmp = new html_form_select(*( html_form_select* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_radio : 
      {
	html_form_radio *tmp = new html_form_radio(*( html_form_radio* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form_check : 
      {
	html_form_check *tmp = new html_form_check(*( html_form_check* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_form : 
      {
	html_form *tmp = new html_form(*( html_form* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_frames : 
      {
	html_frames *tmp = new html_frames(*( html_frames* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_noframe : 
      {
	html_noframe *tmp = new html_noframe(*( html_noframe* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_object : 
      {
	optr = new html_object(obj);
      }
      break;
    case ho_html : 
      {
	html *tmp = new html(*( html* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_head : 
      {
	html_head *tmp = new html_head(*( html_head* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_title : 
      {
	html_title *tmp = new html_title(*( html_title* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_body : 
      {
	html_body *tmp = new html_body(*( html_body* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_paartag : 
      {
	html_paartag *tmp = new html_paartag(*( html_paartag* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_link : 
      {
	html_link *tmp = new html_link(*( html_link* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    case ho_tabular : 
      {
	html_tabular *tmp = new html_tabular(*( html_tabular* ) &obj);
	optr = (html_object* ) tmp;
      }
      break;
    }
  if (optr != NULL)
    objects.push_back(optr);
}


void html_object::write_objs( String &out )
{
  for (vector<html_object*>::iterator it = objects.begin(); it != objects.end(); ++it )
    {
      html_object *optr = *it;
      out.append(optr->write());
    }
}

String  html_object::write( void )
{
  String out;

  out = text;
  write_objs(out);
  return out;
}


html_object_type html::object_typ( void ) const
{
  return ho_html;
}

html::html( void )
{
  objects.clear();
}

String  html::write( void )
{
  String out;

  out="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n";
  out.append("   \"http://www.w3.org/TR/HTNL4.01/strict.dtd\">\n");
  out.append("<html>\n");
  write_objs(out);
  out.append("\n</html>\n");

  return out;
}

html_object_type html_head::object_typ( void ) const
{
  return ho_head;
}

html_head::html_head( void )
{
  objects.clear();
}

String  html_head::write( void )
{
   String out;

  out = "<head>\n";
  write_objs(out);
  out.append("\n</head>\n");

  return out;
}

html_object_type html_title::object_typ( void ) const
{
  return ho_title;
}

html_title::html_title( const String &t )
{
  title = t;
}

String  html_title::write( void )
{
   String out;

  out = String("<title>")+title+String("</title>\n");
  return out;
}

html_object_type html_body::object_typ( void ) const
{
  return ho_body;
}

html_body::html_body( void )
{
  bgcolor = "";
  bgpicture_url = "";
}

void html_body::set_bgcolor( const String &c )
{
  bgcolor = c;
}

void html_body::set_bg_url( const String &u )
{
  bgpicture_url = u;
}
 
String  html_body::write( void )
{
   String out;

  out = String("<body ");
  if (bgcolor.slen() > 0)
    out.append(String("style=\"background-color:#")+bgcolor+String("\""));
  else if (bgpicture_url.slen() > 0)
    out.append(String("style=\"background-image:url(")+bgpicture_url+String(")\""));

  out.append(String(">\n"));
  write_objs(out);
  out.append("\n</body>\n");
  return out;
}
