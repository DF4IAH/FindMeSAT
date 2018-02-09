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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/


#ifndef __ZEIT_H__
#define __ZEIT_H__

#include <asm/types.h>
#include <sys/times.h>

#include <time.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "stdio.h"
#include "unistd.h"
#include "String.h"

using namespace std;

class Error_wrong_data_time_string_format
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_data_time_string_format()
    {
      cerr << "Error_wrong_data_time_string_format" << endl;
    }
#endif
};

class Error_wrong_ptime_syntax
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_ptime_syntax()
    {
      cerr << "Error_wrong_ptime_syntax" << endl;
    }
#endif
};

// Die Klasse zeit representiert einen eine definierte Systemzeit. Diese Zeit kann
// als String ( mit samt Datum ) ausgegeben und eingelesen werden. 
// Beim Aufruf des Default-Konstruktors wird automatisch die aktuelle Zeit uebernommen.
//
class zeit
{
 public:
  enum darst { f_zeit_s, f_zeit, f_datum, f_datum_s, f_datum_l, f_zeitdatum, f_zeitdatum_s, f_zeitdatum_l };

  /* Darstellungsformen:
     f_zeit_s      : HH:MM
     f_zeit        : HH:MM:SS
     f_datum       : dd.mm.
     f_datum_s     : dd.mm.yy
     f_datum_l     : wtg, dd. Mon YYYY
     f_zeitdatum   : dd.mm. HH:MM
     f_zeitdatum_s : dd.mm.yy HH:MM
     f_zeitdatum_l : wtg, dd. Mon YYYY  HH:MM:SS
  */

 private:
  time_t zt;     // Arithmetische Zeit
  bool local;    // Flag ob bei Ausgaben Lokalzeit oder UTC ausgegeben
                 // werden soll. Auf die interne Darstellung hat dieses 
                 // Flag keinen Einfluss
  darst darstellung;

 public:
  zeit( void );
  zeit( time_t , bool = false);

  inline time_t get_systime( void )
    {
      return zt;
    }
  inline void set_local( void )
    {
      local = true;
    }

  inline void set_utc( void )
    {
      local = false;
    }
  inline void set_darstellung( darst d )
    {
      darstellung = d;
    }
  bool operator<( const zeit& ) const;
  bool operator>( const zeit& ) const;
  bool operator==( const zeit& ) const;
  bool operator!=( const zeit& ) const;
  bool operator<=( const zeit& ) const;
  bool operator>=( const zeit& ) const;

  zeit operator!( void );
  inline time_t operator-( const zeit &z ) const
    {
      return zt - z.zt;
    }
  inline zeit operator-( int dt ) const
    {
      return zeit(zt - dt);
    }
  inline zeit operator+(int dt ) const
    {
      return zeit(zt + dt);
    }
  inline zeit operator+=( int dt )
    {
      zt += dt;
      return *this;
    }
  inline zeit operator-=( int dt )
    {
      zt -= dt;
      return *this;
    }
  friend ostream& operator<<( ostream&, const zeit& );
  friend istream& operator>>( istream&, zeit& );
  friend ostream& operator<( ostream&, const zeit& );  // Die letzten beiden 
                                                 // Operatoren dienen dazu,
  friend istream& operator>( istream&, zeit& );  // Zeiten in einer Datei 
                                                 // etwas Maschienennaeher
                                                 // zu speichern
  String get_zeit_string( void ) const;
  String get_unix_zeit_string( void ) const;
  int get_stunden( void );
  int get_minuten( void );
  int get_tag( void );
  int get_monat( void );
  int get_jahr( void );
  int get_tage( void );
  friend zeit linux_start( void );
  friend zeit get_datum( int, int, int ); 
  friend zeit get_datum( istream & ); // Liest ein Datum in der Form TT.MM.JJ
                                      // ein und gibt den Zeitpunkt 12 Uhr
                                      // Zurueck.
};                                               

class pzeit
{
 protected:
  time_t sec;
  __u32 frac;

  bool local;

 public:
  pzeit( void );
  pzeit( zeit );
  pzeit( int, int , bool = false);
  pzeit( double , bool = false );
  pzeit( const String& );

  inline void set_local( void )
    {
      local = true;
    }

  inline void set_utc( void )
    {
      local = false;
    }

  bool operator<( const pzeit& ) const;
  bool operator>( const pzeit& ) const;
  bool operator==( const pzeit& )  const;
  bool operator!=( const pzeit& ) const;
  bool operator<=( const pzeit& ) const;
  bool operator>=( const pzeit& ) const;

  double operator-( const pzeit & ) const;
  double operator-( const zeit & ) const;
  pzeit operator-( double ) const;
  pzeit operator+( double ) const;
  pzeit operator+=( double );
  pzeit operator-=( double );
  
  String get_unix_zeit_string( void ) const ;

  friend ostream& operator<<( ostream&, const pzeit& );
  friend ostream& operator<( ostream&, const pzeit& );  // Die letzten beiden Operatoren dienen dazu,
  friend istream& operator>( istream&, pzeit& );  // Zeiten in einer Datei etwas Maschienennaeher
};


class delta_t
{
 protected:
  int dt;
  int hundertstel;

  bool no_sign;
  bool no_space;

 public:
  inline delta_t(void )
    {
      no_sign = false;
      no_space = false;
      dt = 0;
      hundertstel = 0;
    }
  inline delta_t( int t )
    {
      no_sign = false;
      no_space = false;
      dt = t;
      hundertstel = 0;
    }
  inline delta_t( time_t t )
    {
      no_sign = false;
      no_space = false;
      dt = (int) t;
      hundertstel = 0;
    }
  delta_t( float t );
  delta_t( double t );
  inline int t( void )
    {
      return dt;
    }

  inline void set_no_sign( void )
    {
      no_sign = true;
    }
  inline void clr_no_sign( void )
    {
      no_sign = false;
    }
  inline void set_no_space( void )
    {
      no_space = true;
    }
  inline void clr_no_space( void )
    {
      no_space = false;
    }
  String get_string( void );
  friend ostream& operator<< ( ostream & strm, delta_t );
};

class cpu_time
{
 public:
  double sys_time;
  double user_time;

 public:
  cpu_time operator+( const cpu_time& );
  cpu_time operator+=( const cpu_time& );
};

class cpu_messung
{
 protected:
  static long sys_clk_tck;
  struct tms start;

 public:
  cpu_messung( void );
  cpu_time stop( void );
  void init( void );
};

#endif
