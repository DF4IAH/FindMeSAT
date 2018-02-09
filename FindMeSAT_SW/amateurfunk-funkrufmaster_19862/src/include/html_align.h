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

#ifndef __HTML_ALIGN_H__
#define __HTML_ALIGN_H__

#include "String.h"
#include "html_objects.h"

class html_align : public html_object
{
 public:
  enum t_align { al_no, al_left, al_right, al_center };

 protected:
  t_align al;
  html_object_type object_typ( void ) const;

 public:
  html_align( t_align = al_no);
  inline void set_left( void )
    {
      al = al_left;
    }
  inline void set_right( void )
    {
      al = al_right;
    }
  inline void set_center( void )
    {
      al = al_center;
    }
 String  write( void );
};

class html_valign : public html_object
{
 public:
  enum t_valign { val_no, val_top, val_buttom, val_center };
  html_object_type object_typ( void ) const;

 protected:
  t_valign val;

 public:
  html_valign(t_valign = val_no );
  inline void set_top( void )
    {
      val = val_top;
    }
  inline void set_buttom( void )
    {
      val = val_buttom;
    }
  inline void set_center( void )
    {
      val = val_center;
    }
  String  write( void );
};





#endif // __HTML_ALIGN_H__
