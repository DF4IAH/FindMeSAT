/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
 *                                                                          *
 * Thist Program is free software; yopu can redistribute ist and/or modify  *
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
 ****************************************************************************/

#include "adress.h"


#include <ctype.h>
#include <stdio.h>
#include <iomanip>

adress::adress( void ) // Default-Konstruktor
{
  main = 0;
  sub = 0;
}

adress::adress( const String &ad ) // Konstruktor
{                           // Die Adresse wird in Form
  main = 0;                 // Eines Strings angegeben
  sub = 0;
  try
    {
      unsigned int i = 0;
      char ch = ad[i++];   // Mit erstem Zeichen anfangen
      
      while (isdigit(ch) ) // Auf Ziffer testen
	{
	  main *= 10;           
	  main += (int) ch - 48; // ersten zahlenwert umwandeln
	  if (i < ad.slen() )
	    ch = ad[i++];
	  else
	    ch = ' ';
	}
      
      if ( ch == '.' )       // Punkt ueberpruefen
	{
	  if (i < ad.slen() )
	    ch = ad[i++];
	  else
	    ch = ' ';
	  
	  while (isdigit(ch) ) // und zweiten Zahlenwert wandeln
	    {
	      sub *= 10;
	      sub += (int) ch - 48;
	      if (i < ad.slen() )
		ch = ad[i++];
	      else
		ch = ' ';
	    }
	}
      else throw Error_no_adress();
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_no_adress();
    }
}

adress::adress( int m, int s )
{
  main = m;
  sub = s;
}


String adress::adr( void ) const // Adresse in Form eines Strings
{                                // Ausgeben
  char ad[20];

  sprintf(ad,"%d.%d",main,sub);
  return String(ad);
}

bool adress::operator==( const adress& adr ) const 
{
  return main == adr.main && sub == adr.sub;
}

bool adress::operator!=( const adress& adr ) const
{
  return main != adr.main || sub != adr.sub;
}

ostream& operator<<( ostream &strm, const adress &ad)
{
  strm <<  ad.main;
  strm << "."  << ad.sub;
  return strm;
}

istream& operator>>( istream& strm, adress& ad )
{
  ad.main = 0;
  ad.sub = 0;

  char ch;

  // Ersteinmal fuehrende Leerzeichen und Kommata beseitigen
  while (strm.get(ch) && ((ch == ',') || ( ch == ' '))) ;
  strm.putback(ch);

  // Adresse beim Einlesen aus einem Stream auf aehnliche
  // Weise wandeln, wie beim Einlesen aus String
  while (strm.get(ch) && isdigit(ch))
    {
      ad.main *= 10;
      ad.main += (int) ch - 48;
    }

  if ( ch == '.' )
    {
      while (strm.get(ch) && isdigit(ch))
	{
	  ad.sub *= 10;
	  ad.sub += (int) ch - 48;
	}
    }
  else 
    throw Error_no_adress();
  return strm;
}
