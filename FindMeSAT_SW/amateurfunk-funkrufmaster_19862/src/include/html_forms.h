/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2003 by Holger Flemming                               *
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
 * Some Ideas from:                                                         *
 * The HTTP-Server for the BayCom Mailbox                                   *
 * (c) by DL8MBT, DG9MHZ, OE3DZW et. al.                                    *
 *                                                                          *
 * List of other authors:                                                   *
 *  		                                			    *
 ****************************************************************************/

#ifndef __HTML_FORMS_H__
#define __HTML_FORMS_H__

#include <vector>

#include "String.h"
#include "html_objects.h"



class html_form_text_input : public html_object
{
 protected:
  String name;
  String value;
  int size;
  html_object_type object_typ( void ) const;

 public:
  html_form_text_input( const String&, const String&, int );
  ~html_form_text_input( void );

  String write( void );
};

class html_form_password_input : public html_object
{
 protected:
  String name;
  String value;
  int size;
  html_object_type object_typ( void ) const;

 public:
  html_form_password_input( const String&, const String&, int );
  ~html_form_password_input( void );

 String write( void );
};

class html_form_submit : public html_object
{
 protected:
  String value;
  html_object_type object_typ( void ) const;

 public:
  html_form_submit( const String& );
  ~html_form_submit();

  String write( void );
};

class html_form_reset : public html_object
{
 protected:
  String value;
  html_object_type object_typ( void ) const;

 public:
  html_form_reset( const String& );
  ~html_form_reset();

  String write( void );
};

class html_form_select : public html_object
{
 protected:
  struct option
  {
    String value;
    String text;
    bool selected;
  };
  vector<struct option> options;
  String name;
  bool selected_option;
  html_object_type object_typ( void ) const;

 public:
  html_form_select( const String & );
  ~html_form_select( );

  void add_option(const String &, const String & , bool = false);
  void select_option( const String & );
  String  write( void );
};


class html_form_radio : public html_object
{
 protected:
  String name;
  String value;

  html_object_type object_typ( void ) const;

 public:
  html_form_radio( const String &, const String &, const html_object & );
  ~html_form_radio( );

  String  write( void );
};

class html_form_check : public html_object
{
 protected:
  String name;
  String value;

  html_object_type object_typ( void ) const;

 public:
  html_form_check( const String &, const String &, const html_object & );
  ~html_form_check( );

  String  write( void );
};

class html_form : public html_object
{
 public:
  enum f_method { m_get, m_post };
 protected:
  f_method met;
  String action;
  String target;
  html_object_type object_typ( void ) const;

 public:
  html_form( f_method, const String & );
  void set_target( const String & );
  String write( void );
};
#endif // __HTML_FORMS_H__
