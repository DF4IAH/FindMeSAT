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

#ifndef __HTML_FRAMES_H__
#define __HTML_FRAMES_H__

#include <vector>

#include "String.h"
#include "html_objects.h"

enum t_scroll { sc_no, sc_yes, sc_auto };

class html_frames : public html_object
{
 protected:
  
  class html_frame
    {
    protected:
      int size;
      bool pixel;
      enum t_scroll scrolling;
      String src;
      String name;
      
    public:
      html_frame( int, bool, enum t_scroll, const String&, const String& );
      String  write( void );
      String write_size( void );
    };
  bool row_flag;
  vector<html_frame> frame_eintraege;
  String alt_src;
  
  html_object_type object_typ( void ) const;

 public:
  
  html_frames( bool = true);
  void new_frame( int, bool, enum t_scroll, const String&, const String& );
  void set_alt_src( const String & );
  String write( void );
};

class html_noframe : public html_object
{
 protected:
  html_object_type object_typ( void ) const;

 public:
  html_noframe( void );

  String  write( void );
};
#endif // __HTML_FRAMES_H__
