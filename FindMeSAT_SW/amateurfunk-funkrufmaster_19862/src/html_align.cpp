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

#include "html_align.h"

html_object_type html_align::object_typ( void ) const
{
  return ho_align;
}

html_align::html_align( t_align a )
{
  al = a;
}

String html_align::write( void )
{
   String out;

  if (al != al_no)
    {
      out = " align=\"";
      switch(al)
	{
	  case al_no     : break;
	  case al_left   : out.append("left\"");
	                   break;
 	  case al_right  : out.append("right\"");
	                   break;
	  case al_center : out.append("center\"");
	                   break;
	}
    }
  else
    out = "";

  return out;
}

html_object_type html_valign::object_typ( void ) const
{
  return ho_valign;
}

html_valign::html_valign( t_valign a )
{
  val = a;
}

String html_valign::write( void )
{
   String out;

  if (val != val_no)
    {
      out = " valign=\"";
      switch(val)
	{
	  case val_no     : break;
	  case val_top    : out.append("top\"");
	                    break;
 	  case val_buttom : out.append("buttom\"");
	                    break;
	  case val_center : out.append("center\"");
	                    break;
	}
    }
  else
    out = "";

  return out;
}
