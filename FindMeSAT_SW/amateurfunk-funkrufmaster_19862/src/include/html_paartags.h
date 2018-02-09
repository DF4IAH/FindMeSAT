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

#ifndef __HTML_PAARTAGS_H__
#define __HTML_PAARTAGS_H__

#include "html_objects.h"
#include "String.h"

class html_paartag : public html_object
{
 protected: 
  String tag;

  html_object_type object_typ( void ) const;

 public:
  html_paartag( const String & = "" );
  String write( void );
};

class html_italic : public html_paartag
{
 public:
  html_italic( void )
    {
      tag = 'i';
    }
};

class html_fett : public html_paartag
{
 public:
  html_fett( void )
    {
      tag = 'b';
    }
};

class html_absatz : public html_paartag
{
 public:
  html_absatz( void )
    {
      tag = 'p';
    }
};

class html_ueberschrift : public html_paartag
{
 public:
  html_ueberschrift( int i )
    {
      tag = String('h')+itoS(i);
    }
};


#endif // __HTML_PAARTAGS_H__
