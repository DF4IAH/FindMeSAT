/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
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

#ifndef __LOCATOR_H__
#define __LOCATOR_H__

#include <iostream>
#include <math.h>
#include <ctype.h>

using namespace std;

#include "String.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795029
#endif // PI

#ifndef R_ERDE
#define R_ERDE 6378.16
#endif
inline double sqr( double x )
{
  return(x * x );
}

class Error_Angle_Out_Of_Range
{
#ifdef _DEBUG_EXEC_
 public:
  Error_Angle_Out_Of_Range()
    {
      cerr << "Error_Angle_Out_Of_Range" << endl;
    }
#endif
};

class Error_not_a_locator
{
#ifdef _DEBUG_EXEC_
 public:
  Error_not_a_locator()
    {
      cerr << "Error_not_a_locator" << endl;
    }
#endif
};

struct strecke
{
  double entfernung;
  double richtung;
};

class laenge
{
  private:
  double l;
  
  public:
  laenge( double lae = 0 );
  laenge( int gr, int min, float sec, bool ostwest );
  inline double rad( void )
  {
    return( l );
  }
  inline double operator-( laenge l2 )
  {
    return( l - l2.l );
  }
  inline double operator+( laenge l2 )
  {
    return( l + l2.l );
  }
  inline laenge operator+( double dl )
  {
    return( laenge( l + dl ) );
  }
};

class breite
{
  private:
  double b;
  
  public:
  breite( double br = 0 );
  breite( int gr, int min, float sec, bool nordsued );
  inline double rad( void )
  {
    return( b );
  }
  inline double operator-( breite b2 )
  {
    return( b - b2.b );
  }
  inline double operator+( breite b2 )
  {
    return( b + b2.b );
  }
};

class locator
{
  private:
  laenge l;
  breite b;
  String loc;
  bool gesetzt;
  
  public:
  inline locator( void )
  {
    l = laenge(0);
    b = breite(0);
    loc = String("");
    gesetzt = false;
  }
  locator( String );
  locator( laenge, breite );
  inline String str( void ) const
    {
      return loc;
    }
  String asynop( void );
  bool asynop( const String &, const String& );
  void printOn (ostream& = cout ) const;
  void scanFrom ( istream& = cin );
  double distance( locator& );
  double direction( locator& );
  void add( struct strecke );
  inline bool ok( void )
    {
      return( gesetzt );
    }
  inline laenge get_laenge( void)
    {
      return l;
    }
  inline breite get_breite( void )
    {
      return b;
    }
};

inline ostream& operator<< (ostream& strm, const locator& l)
{
  l.printOn(strm);	// Locator auf Stream ausgeben
  return strm;		// Stream zurueck  liefern
}

inline istream& operator>> (istream& strm, locator &l )
{
  l.scanFrom(strm);
  return strm;
}

inline struct strecke operator-( locator &startpunkt, locator &endpunkt )
{
  struct strecke tmp;
  tmp.entfernung = startpunkt.distance(endpunkt);
  tmp.richtung = startpunkt.direction(endpunkt);
  return tmp;
}

inline locator operator+( locator &startpunkt, struct strecke s )
{
  locator tmp = startpunkt;
  startpunkt.add(s);
  return tmp;
}

#endif
