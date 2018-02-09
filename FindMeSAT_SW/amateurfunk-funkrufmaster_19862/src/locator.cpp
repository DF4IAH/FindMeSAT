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

#include "locator.h"

laenge::laenge( double lae )		// Konstruktor der Laenge
{
  /* Einfach den Winkel in Bogenmass uebernehmen. Wenn der Winkel nicht zwischen 
     -PI und +PI liegt, dann liegt ein Fehler vor. */
  if ((lae < -PI) || (lae > PI)) 
    throw Error_Angle_Out_Of_Range();
  else
    l = lae;
}

/* Winkel aus Grad, Minuten und Sekunden zusammensetzen */

laenge::laenge( int gr, int min, float sec, bool ostwest )
{
  const double f = PI / 180;
  
  if ((gr < 0) || (gr > 180)) throw Error_Angle_Out_Of_Range();
  if ((min < 0) || (min > 60)) throw Error_Angle_Out_Of_Range();
  if ((sec < 0) || (sec > 60)) throw Error_Angle_Out_Of_Range();
  l = (double) gr * f + (double) min * f / 60 + sec * f / 3600;
  if (!ostwest) l = -l;
  if ((l < PI) || (l > PI)) throw Error_Angle_Out_Of_Range();
}

// Hier das gleiche nochmal fuer die Breite wie vorher fuer die Laenge

breite::breite(double br )
{
  if ((br < (-PI/2)) || (br > (PI/2))) 
    throw Error_Angle_Out_Of_Range();
  else
    b = br;
}

breite::breite( int gr, int min, float sec, bool nordsued )
{
  const double f = PI / 180;
  
  if ((gr < 0) || (gr > 90)) throw Error_Angle_Out_Of_Range();
  if ((min < 0) || (min > 60)) throw Error_Angle_Out_Of_Range();
  if ((sec < 0) || (sec > 60)) throw Error_Angle_Out_Of_Range();
  b = (double) gr * f + (double) min * f / 60 + sec * f / 3600;
  if (!nordsued) b = -b;
  if ((b < (PI/2)) || (b > (PI/2))) throw Error_Angle_Out_Of_Range();
}

// Konstruktor fuer den Locator. Der Locator wird als String uebergeben

locator::locator(String lc)
{
  try
    {
      const unsigned char asckor[6] = {65,65,48,48,65,65};
      const unsigned char maske[6] = {223,223,255,255,223,223};
      const double wertig[8] = {20.,10.,2.,1.,1./12,1./24,1./24,1./48};
      const double f = PI / 180;
      
      double WiInfo[6];
      int i = 0;
      
      // Locator MUSS 6 Zeichen haben !  
      if (lc.slen() != 6) 
	{
#ifdef _DEBUG_ELOGS_
	  cerr << "Fehler bei Locatorumwandlung von : " << lc << endl;
#endif // _DEBUG_ELOGS_
	  throw Error_not_a_locator();
	}
      // Buchstaben in Grossbuchstaben wandeln
      for (i=0;i<6;i++) lc[i] = toupper(lc[i]);
      
      // erste und letzte beiden Zeichen muessen Buchstaben, die beiden anderen 
      // Ziffern sein.
      if ((!isalpha(lc[0])) || (!isalpha(lc[1])) || (!isalpha(lc[4])) || (!isalpha(lc[5])))
	{
#ifdef _DEBUG_ELOGS_
	  cerr << "Fehler bei Locatorumwandlung von : " << lc << endl;
#endif // _DEBUG_ELOGS_
	  throw Error_not_a_locator();
	}
      if ((!isdigit(lc[2])) || (!isdigit(lc[3])))
	{
#ifdef _DEBUG_ELOGS_
	  cerr << "Fehler bei Locatorumwandlung von : " << lc << endl;
#endif // _DEBUG_ELOGS_
	  throw Error_not_a_locator();
	}
      // Aus den sechs Zeichen die entsprechenden WErtigkeiten rausfiltern
      for (i=0;i<6;i++)
	WiInfo[i] =  (double) ((lc[i] & maske[i]) - asckor[i]);
      
      try	// Nun versuchen, die Laenge und die breite zu konstruieren
	{
	  l = laenge( - PI + WiInfo[0] * f * wertig[0] 
                      + WiInfo[2] * f * wertig[2] 
                      + WiInfo[4] * f * wertig[4]
                      + f * wertig[6] );
	  b = breite( - PI/2 + WiInfo[1] * f * wertig[1] 
		      + WiInfo[3] * f * wertig[3] 
		      + WiInfo[5] * f * wertig[5]
		      + f * wertig[7] );
	}
      /* Wenn der Winkel ausserhalb des zulaessigen Bereiches war, dann war es kein gŸltiger 
	 Locator */
      catch(Error_Angle_Out_Of_Range)
	{
	  throw Error_not_a_locator();
	}
      
      loc = lc;	// Locatorstring zusaetzlich kopieren
      gesetzt = true;
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_not_a_locator();
    }
}

locator::locator( laenge lin, breite bin )
{
  const unsigned char asckor[6] = {65,65,48,48,65,65};
  //const unsigned char maske[6] = {223,223,255,255,223,223};
  const double wertig[8] = {20.,10.,2.,1.,1./12,1./24,1./24,1./48};
  const double f = PI / 180;

  double lg = (lin.rad() + PI) / f;
  double bg = (bin.rad() * PI) / f;

  char locchars[7];

  for (int i = 0; i < 6 ; i += 2 )
    {
      int fak = (int) (lg / wertig[i]);
      lg -= fak * wertig[i];
      locchars[i] = (char) (fak + asckor[i]);
    }

  for (int i = 1; i < 6 ; i += 2 )
    {
      int fak = (int) (bg / wertig[i]);
      bg -= fak * wertig[i];
      locchars[i] = (char) (fak + asckor[i]);
    }
  locchars[6] = '\0';
  loc = String(locchars);
  l = lin;
  b = bin;
  gesetzt = true;
}

String locator::asynop( void )
{
  String tmp;
  char gruppe[7];

  gruppe[6] = '\0';

  gruppe[0] = '9';
  gruppe[1] = '9';

  int br = (int) (fabs(b.rad()) * 1800 / PI);

  for (int i = 2; i >= 0 ; i--)
    {
      int st = br % 10;
      gruppe[i+2] = (char) (st + 48);
      br = br / 10;
    }

  tmp = String(gruppe) + " ";

  int q;
  if (b.rad() >= 0 && l.rad() >= 0)
    q = 1;
  else if (b.rad() < 0 && l.rad() >= 0)
    q = 2;
  else if (b.rad() < 0 && l.rad() < 0 )
    q = 3;
  else 
    q = 4;

  gruppe[0] = (char) (q + 48);

  int la = (int) (fabs(l.rad()) * 1800 / PI);

  for (int i = 2; i >= 0 ; i--)
    {
      int st = la % 10;
      gruppe[i+2] = (char) (st + 48);
      la = la / 10;
    }

  tmp.append(String(gruppe));

  return tmp;
}

bool locator::asynop( const String &gr1, const String &gr2 )
{
  if (gr1.slen() == 5 && gr2.slen() == 5)
    {
      if (gr1[0] != '9' || gr1[1] != '9')
	return false;

      int quadrant = (int) gr2[0] - 48;

      int la = 0;
      for (unsigned int i = 2; i < 5; i++ )
	{
	  int st = (int) gr1[i] - 48;
	  la = la * 10 + st;
	}

      if ( quadrant == 2 || quadrant == 3 )
	la = -la;

      int br = 0;
      for (unsigned int i = 1; i < 5; i++ )
	{
	  int st = (int) gr2[i] - 48;
	  br = br * 10 + st;
	}

      if (quadrant == 3 || quadrant == 4)
	br = -br;

      double l2 = (double) la / 1800 * PI;
      double b2 = (double) br / 1800 * PI;

      *this = locator(l2,b2);
      return true;
    }
  return false;
}

void locator::printOn (ostream& strm ) const
{
  strm << loc;	// Einfach Locator-String ausgeben
}

/* Der Operator - berechnet den Abstand zwischen zwei gegebenen Locator-Feldern.
   Die Entfernungsberechnung geschieht nach dem Cosinussatz der sphaerischen 
   Geometrie. */

void locator::scanFrom( istream& strm )
{
  char ch;
  cerr << "Einlesen eines Locators ! " << endl;
  cerr << "Ueberpruefen, ob Komma voransteht : ";
  strm >> ch;
  cerr << ch << endl;
  if ( ch == ',' ) strm >> ch;
  cerr << "Voranstehende Leerzeihen beseitigen " << endl;
  while (   ch == ' ' ) 
    {
      strm >> ch;
      cerr << ch << '*' ;
    }
  cerr << endl;
  strm.putback(ch);

  char puffer[7];
  strm.get(puffer,7,',');
  cerr << "Eingelsener Puffer : >" << puffer << "< " << endl;
  if (*puffer == '\0') 
    gesetzt = false;
  else
    *this = locator(String(puffer));
}


double locator::distance( locator & endpunkt )
{
  double gk_winkel;
  double c;
  const double erdradius = R_ERDE;
  
  c = sin(b.rad()) * sin(endpunkt.b.rad()) 
    + cos(b.rad()) * cos(endpunkt.b.rad()) 
    * cos(endpunkt.l - l);
  gk_winkel = acos(c);
  return gk_winkel * erdradius;
}

double locator::direction( locator &endpunkt )
{
  double gk_winkel, dir_winkel;
  double c,d;

  c = sin(b.rad()) * sin(endpunkt.b.rad()) 
    + cos(b.rad()) * cos(endpunkt.b.rad()) 
    * cos(endpunkt.l - l);
  gk_winkel = acos(c);

  d = ((sin(endpunkt.b.rad()) * sin(b.rad()) * c) / 
       (cos(b.rad()) * sin(gk_winkel)));
  dir_winkel = acos(d);

  if ((l.rad() > 0) && endpunkt.l.rad() < 0)
    return dir_winkel;
  else if (l.rad() > endpunkt.l.rad())
    return - dir_winkel;
  else 
    return dir_winkel;
}

void locator::add( struct strecke s )
{
  const double erdradius = R_ERDE;
  double fl;
  breite b2;
  laenge l2;
  // Zunaechst wird der Winkel berechnet, den die Distanz ueber den
  // Erdmittelpunkt aufspannt.
  double gk_winkel =  s.entfernung / erdradius;

  // Nun wird die Breite des neuen Punktes berechnet.
  double c = sin(b.rad()) * cos(gk_winkel) 
    + cos(b.rad()) * sin(gk_winkel) * cos(s.richtung);
  b2 = breite(asin(c));

  c = ( cos(gk_winkel) - sin(b2.rad()) * sin(b.rad()) ) 
    / ( cos(b2.rad()) * cos(b.rad()));
  double delta_l = acos(c);


  if (s.richtung > 0) 
    fl = l.rad() + delta_l;
  else
    fl = l.rad() - delta_l;

  if (fl < - PI)
    fl += 2 * PI;
  else if (fl > PI)
    fl -= 2 * PI;

  l2 = laenge(fl);

  *this = locator(l2,b2);
}

