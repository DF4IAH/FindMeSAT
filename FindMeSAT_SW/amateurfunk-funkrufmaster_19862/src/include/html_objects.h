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
 *  		                                			    *
 ****************************************************************************/

#ifndef __HTML_OBJECTS_H__
#define __HTML_OBJECTS_H__

#include <vector>

#include "String.h"

enum html_object_type { ho_none, ho_align, ho_valign, ho_form_text_input,
			ho_form_password_input, ho_form_submit, ho_form_reset,
			ho_form_select, ho_form_radio, ho_form_check,
			ho_form, ho_frames, ho_noframe, ho_object, ho_html, 
			ho_head, ho_title, ho_body, ho_paartag, ho_link, 
			ho_tabular };


String line_text2html( const String & );

class html_object
{
 protected:
  String text;
  vector<html_object*> objects;

 protected:
  virtual html_object_type object_typ( void ) const;

 public:
  html_object( void );
  html_object( String );
  html_object( const char * );
  html_object( char );
  html_object( const html_object & );
  virtual ~html_object( void );

  const html_object& operator= ( const html_object & );
  void add( const html_object & );
  void write_objs( String & );
  virtual String write( void );
};

class html : public html_object
{
 protected:
  virtual html_object_type object_typ( void ) const;

 public:
  html( void );

  String  write( void );
};

class html_head : public html_object
{
 protected:

  virtual html_object_type object_typ( void ) const;

 public:
  html_head( void );

  String  write( void );
};

class html_title : public html_object
{
 protected:
  String title;

  virtual html_object_type object_typ( void ) const;

 public:
  html_title( const String & );

  String  write( void );
};

class html_body : public html_object
{
 protected:
  String bgcolor;
  String bgpicture_url;

  virtual html_object_type object_typ( void ) const;

 public:
  html_body( void );

  void set_bgcolor( const String & );
  void set_bg_url( const String & );
  String write( void );
};

#endif
