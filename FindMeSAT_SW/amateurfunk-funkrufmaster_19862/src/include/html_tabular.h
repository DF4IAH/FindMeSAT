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

#ifndef __HTML_TABULAR_H__
#define __HTML_TABULAR_H__

#include "html_objects.h"
#include "html_align.h"

#include <vector>


class html_tabular : public html_object
{
 protected:
  class cell
    {
    protected:
      html_align al;
      html_object mem;
      String color;
      bool head;
      int height,width;
      
    public:
      cell( html_align, bool, const html_object &, const String &, int = -1, int = -1 );
      String  write ( void );
    };
  
  class row
    {
    protected:
      vector<cell> cells;
      html_valign valign;
    public:
      row( void );
      void set_align( html_valign );
      void new_cell( html_align, bool,const html_object &, const String &, int = -1, int = -1 );
      String write( void );
    };
  
  vector<row> head;
  vector<row> foot;
  vector<row> body;
  
  row akt_row;
  int border;
  int spacing;
  int padding;
  String caption;
  html_object_type object_typ( void ) const;

 public:
  html_tabular( void );
  void new_row( void );
  void add_cell(  html_align, bool,const html_object & , String = "", int = -1, int = -1 );
  void set_rowalign( html_valign );
  void add_to_head( void );
  void add_to_foot( void );
  void add_to_body( void );
  void set_border( int );
  void set_spacing( int );
  void set_padding( int );
  void set_caption( const String & );
  
  String  write( void );
};

#endif // __HTML_TABULAR_H__
