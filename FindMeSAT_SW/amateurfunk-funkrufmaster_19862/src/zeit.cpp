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


#include "zeit.h"

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <sys/timeb.h>

extern void do_gettimeofday(struct timeval *tv);

// Defaultkonstruktor

zeit::zeit( void )
{
  zt = time(NULL);
  local = false;
  darstellung = f_zeitdatum_l;
}

zeit::zeit( time_t t, bool lc )
{
  zt = t;
  local = lc;
  darstellung = f_zeitdatum_l;
}

// Die Vergliechsoperatoren vergleichen lediglich die arithmetische Zeit
bool zeit::operator< ( const zeit &z ) const
{
  return zt < z.zt;
}

bool zeit::operator> ( const zeit &z ) const
{
  return zt > z.zt;
}

bool zeit::operator== ( const zeit &z ) const
{
  return zt == z.zt;
}

bool zeit::operator!= ( const zeit &z ) const
{
  return zt != z.zt;
}

bool zeit::operator>= ( const zeit &z ) const
{
  return zt >= z.zt;
}

bool zeit::operator<= ( const zeit &z ) const
{
  return zt <= z.zt;
}

zeit zeit::operator! ( void )
{
  zeit tmp;
  tmp.zt = (time_t) (( (int) (zt / 60)) * 60);
  tmp.local = local;
  tmp.darstellung = darstellung;
  return tmp;
}
 
ostream& operator<< ( ostream &strm, const zeit &z )
{
  // ausgabe der Zeit im Format
  // Wochentag, Tag im Monat, Monat, Jahr, Stunden, Minuten, Sekunden 
  // Formatierung:
  // Sun, 3. Jan 1988 15:14:13

  struct tm zeit_str;

  if (z.local)
    zeit_str = *localtime(&(z.zt));
  else
    zeit_str = *gmtime(&(z.zt));

  char puffer[100];
  char format[100];

  switch(z.darstellung)
    {
      case zeit::f_zeit_s      : strcpy(format,"%H:%M");
	                         break;
      case zeit::f_zeit        : strcpy(format,"%H:%M:%S");
                                 break;
      case zeit::f_datum_s     : strcpy(format,"%d.%m.%y");
                                 break;
      case zeit::f_datum       : strcpy(format,"%d.%m.");
                                 break;
      case zeit::f_datum_l     : strcpy(format,"%a, %d. %b %Y");
                                 break;
      case zeit::f_zeitdatum   : strcpy(format,"%d.%m. %H:%M");
                        	 break;
      case zeit::f_zeitdatum_s : strcpy(format,"%d.%m.%y %H:%M:%S");
                                 break;
      case zeit::f_zeitdatum_l : strcpy(format,"%a, %d. %b %Y  %H:%M:%S");
                                 break;
    }
  strftime(puffer,99,format,&zeit_str);
  strm << puffer;
  return strm;
}

// eingabe einer Zeit aus einem Stream

istream& operator>>( istream& strm, zeit & z )
{
  char puffer[100];
  struct tm zeit_str;
  char ch;

  // Zunaechst alle vorweg stehenden Leerzeichen uebergehen
  strm >> ch;
  while ( ch == ' ' ) strm >> ch;
  strm.putback(ch);

  // Zunaechst wird eine zeitstruktur vom typ struct tm erzeugt und
  // die Komponenten stueckweise eingelesen.

  strm.get(puffer,99,',');
  if (strcmp(puffer,"Sun") == 0) zeit_str.tm_wday = 0;
  else if (strcmp(puffer,"Mon") == 0) zeit_str.tm_wday = 1;
  else if (strcmp(puffer,"Tue") == 0) zeit_str.tm_wday = 2;
  else if (strcmp(puffer,"Wen") == 0) zeit_str.tm_wday = 3;
  else if (strcmp(puffer,"Thu") == 0) zeit_str.tm_wday = 4;
  else if (strcmp(puffer,"Fri") == 0) zeit_str.tm_wday = 5;
  else if (strcmp(puffer,"Sat") == 0) zeit_str.tm_wday = 6;
  else throw Error_wrong_data_time_string_format();

  strm >> ch;
  if (ch != ',') throw Error_wrong_data_time_string_format();

  strm >> zeit_str.tm_mday;

  strm >> ch;
  if (ch != '.') throw Error_wrong_data_time_string_format();

  strm >> ch;
  while ( ch == ' ' ) strm >> ch;
  strm.putback(ch);

  strm.get(puffer,99,' ');
       if (strcmp(puffer,"Jan") == 0) zeit_str.tm_mon = 0;
  else if (strcmp(puffer,"Feb") == 0) zeit_str.tm_mon = 1;
  else if (strcmp(puffer,"Mar") == 0) zeit_str.tm_mon = 2;
  else if (strcmp(puffer,"Apr") == 0) zeit_str.tm_mon = 3;
  else if (strcmp(puffer,"May") == 0) zeit_str.tm_mon = 4;
  else if (strcmp(puffer,"Jun") == 0) zeit_str.tm_mon = 5;
  else if (strcmp(puffer,"Jul") == 0) zeit_str.tm_mon = 6;
  else if (strcmp(puffer,"Aug") == 0) zeit_str.tm_mon = 7;
  else if (strcmp(puffer,"Sep") == 0) zeit_str.tm_mon = 8;
  else if (strcmp(puffer,"Oct") == 0) zeit_str.tm_mon = 9;
  else if (strcmp(puffer,"Nov") == 0) zeit_str.tm_mon = 10;
  else if (strcmp(puffer,"Dec") == 0) zeit_str.tm_mon = 11;
  else throw Error_wrong_data_time_string_format();

  strm >> zeit_str.tm_year;
  zeit_str.tm_year -= 1900;

  strm >> zeit_str.tm_hour;

  strm >> ch;
  if (ch != ':') throw Error_wrong_data_time_string_format();

  strm >> zeit_str.tm_min;

  strm >> ch;
  if (ch != ':') throw Error_wrong_data_time_string_format();

  strm >> zeit_str.tm_sec;

  zeit_str.tm_isdst = -1;
  // Aus den Komponenten wird jetzt die arithmetische Zeit erzeugt.
  z.zt = mktime(&zeit_str);
  z.local = true;
  return strm;
}

ostream& operator< ( ostream &strm, const zeit &z )
{
  strm << z.zt;
  return strm;
}

istream& operator> ( istream &strm, zeit &z )
{
  char ch;
  strm >> ch;
  while (( ch == ',' ) || ( ch == ' ')) strm >> ch;
  strm.putback(ch);
  strm >> z.zt;
  return strm;
}

String zeit::get_zeit_string( void ) const
{
  // ausgabe der Zeit im Format
  // Wochentag, Tag im Monat, Monat, Jahr, Stunden, Minuten, Sekunden 
  // Formatierung:
  // Sun, 3. Jan 1988 15:14:13

  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  char puffer[100];
  char format[100];

  switch(darstellung)
    {
      case f_zeit_s      : strcpy(format,"%H:%M");
	                   break;
      case f_zeit        : strcpy(format,"%H:%M:%S");
                           break;
      case f_datum       : strcpy(format,"%d.%m.");
                           break;
      case f_datum_s     : strcpy(format,"%d.%m.%y");
                           break;
      case f_datum_l     : strcpy(format,"%a, %d. %b %Y");
                           break;
      case f_zeitdatum   : strcpy(format,"%d.%m. %H:%M");
                           break;
      case f_zeitdatum_s : strcpy(format,"%d.%m.%y %H:%M:%S");
                           break;
      case f_zeitdatum_l : strcpy(format,"%a, %d. %b %Y  %H:%M:%S");
                           break;
    }
  strftime(puffer,99,format,&zeit_str);
  return String(puffer);
}

String zeit::get_unix_zeit_string( void ) const
{
  return itoS(zt);
}

int zeit::get_stunden( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_hour;
}

int zeit::get_minuten( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_min;
}

int zeit::get_tag( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_mday;
}

int zeit::get_monat( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_mon + 1;
}

int zeit::get_jahr( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_year + 1900;
}

int zeit::get_tage( void )
{
  struct tm zeit_str;

  if (local)
    zeit_str = *localtime(&(zt));
  else
    zeit_str = *gmtime(&(zt));

  return zeit_str.tm_mday;
}


zeit linux_start( void )
{
  zeit zt;
  ifstream proc("/proc/uptime");
  if (proc)
    {
      int uptime;
      proc >> uptime;
      zt.zt -= uptime;
    }
  return zt;
}

zeit get_datum( int day, int mon, int year )
{
  struct tm zeit_str;

  zeit_str.tm_mday = day;
  zeit_str.tm_mon = mon-1;
  zeit_str.tm_year = year - 1900;
  zeit_str.tm_sec = 0;
  zeit_str.tm_min = 0;
  zeit_str.tm_hour = 12;
  zeit z;
  // Aus den Komponenten wird jetzt die arithmetische Zeit erzeugt.
  z.zt = mktime(&zeit_str);
  z.local = true;
  return z;
}

zeit get_datum( istream &strm )
{
  char ch;
  int day, mon, year;
  strm >> day;

  strm >> ch;
  if ( ch != '.')
    throw Error_wrong_data_time_string_format();

  strm >> mon;

  strm >> ch;
  if ( ch != '.')
    throw Error_wrong_data_time_string_format();

  strm >> year;
  if (year < 50)
    year += 100;
  else if (year >= 100)
    year -= 1900;

  return get_datum(day,mon,year);
}


/* ----------------------------------------------------------------
   Nun folgen die Methoden der Klasse pzeit, die eine Praezisionszeit fuer
   Laufzeitmessungen zur Verfuegung stellt.
   -------------------------------------------------------------------
*/

pzeit::pzeit( void )
{
  struct timeb tb;

  ftime(&tb);

  __u64 h = 0;
  h |= (__u64) ((((__u32) tb.millitm) << 22) & 0xffc00000);
  h |= (__u64) (rand() & 0x003fffff );
  h *= 4398046511ULL;
  h >>= 32;
  frac = (__u32) h;
  sec = tb.time;
  
  local = false;
}

pzeit::pzeit( zeit zt )
{
  sec = zt.get_systime();
  frac = rand() << 1;

  local = false;
}

pzeit::pzeit( int s, int f, bool lo )
{
  sec = s;
  frac = f;
  local = lo;
}

pzeit::pzeit( double d, bool l )
{
  double fr;
  double ip;
  fr = modf(d,&ip);

  sec = (int) ip;
  frac = (int) (fr * (double) UINT_MAX);
  local = l;
}

pzeit::pzeit( const String &inp )
{
  int p = inp.pos('.');

  if (p > 0 && (unsigned int) p < inp.slen()-1)
    {
      String s1 = inp.copy(0,p);
      String s2 = inp.copy(p+1,inp.slen()-p-1);
      sec = s1.Stoi();
      frac = s2.Stoi();
    }
  else
    throw Error_wrong_ptime_syntax();
}

bool pzeit::operator<( const pzeit &zt ) const
{
  if ( sec < zt.sec )
    return true;
  else if (sec == zt.sec && frac < zt.frac )
    return true;
  else 
    return false;
}

bool pzeit::operator>( const pzeit &zt ) const
{
  if ( sec > zt.sec )
    return true;
  else if (sec == zt.sec && frac > zt.frac )
    return true;
  else 
    return false;
}

bool pzeit::operator==( const pzeit &zt ) const 
{
  return (sec == zt.sec && frac == zt.frac );
}

bool pzeit::operator!=( const pzeit &zt ) const
{
  return (sec != zt.sec || frac != zt.frac );
}

bool pzeit::operator<=( const pzeit &zt ) const
{
  return *this < zt || *this == zt;
}

bool pzeit::operator>=( const pzeit &zt ) const
{
  return *this > zt || *this == zt;
}

double pzeit::operator-( const pzeit &zt ) const
{
  double tmp = (double) sec - zt.sec;
  tmp += ((double) frac - (double) zt.frac) / (double) UINT_MAX;
  return tmp;
}

double pzeit::operator-( const zeit &zt ) const
{
  return *this - pzeit(zt);
}

pzeit pzeit::operator-( double d ) const
{
  double fr;
  double ip;
  pzeit tmp;
  
  fr = modf(d,&ip);

  tmp.sec = sec - (int) ip;
  tmp.frac = frac - (int) (fr * (double) UINT_MAX);

  return tmp;
}

pzeit pzeit::operator+( double d ) const
{
  double fr;
  double ip;
  pzeit tmp;
  
  fr = modf(d,&ip);

  tmp.sec = sec - (int) ip;
  tmp.frac = frac - (int) (fr * (double) UINT_MAX);

  return tmp;
}

pzeit pzeit::operator-=( double d ) 
{
  double fr;
  double ip;
  fr = modf(d,&ip);

  sec -= (int) ip;
  frac -= (int) (fr * (double) UINT_MAX);
  return *this;
}

pzeit pzeit::operator+=( double d ) 
{
  double fr;
  double ip;
  fr = modf(d,&ip);

  sec += (int) ip;
  frac += (int) (fr * (double) UINT_MAX);
  return *this;
}

String pzeit::get_unix_zeit_string( void ) const
{
  String tmp;
  tmp = itoS(sec);
  tmp.append('.');
  tmp.append(itoS(frac));
  return tmp;
}

ostream& operator<<( ostream &ostr, const pzeit &zt )
{
  zeit se(zt.sec);
  se.set_darstellung(zeit::f_zeitdatum_s);
  ostr << se << ',';
  double f = ((double) zt.frac) / ((double) UINT_MAX);
  int k = (int) (f*1000.);
  ostr << setw(3) << setfill('0') << k;
  return ostr;
}

ostream& operator<( ostream &ostr, const pzeit &zt )
{
  ostr << zt.sec << '.' << zt.frac;
  return ostr;
}

istream& operator>( istream &strm, pzeit &zt )
{
  char ch;
  strm >> ch;
  while ( (ch == ',') || ( ch == ' ' ) )
    strm >> ch;
  strm.putback(ch);

  strm >> zt.sec;
  strm >> ch;
  if ( ch == '.' )
    strm >> zt.frac;
  else
    throw Error_wrong_ptime_syntax();

  return strm;
}


/* ----------------------------------------------------------------
   Nun folgen die Methoden der Klasse delta_t, die Zeitdifferenzen
   behandelt
   -------------------------------------------------------------------
*/

delta_t::delta_t( float t )
{
  no_sign = false;
  no_space = false;
  
  bool m_flag = t < 0;
  
  t = fabs(t);
  dt = (int) t;
  hundertstel = (int) ((t - dt) * 100);
  if (m_flag) 
    dt = - dt;
}

delta_t::delta_t( double t )
{
  no_sign = false;
  no_space = false;
  
  bool m_flag = t < 0;
  
  t = fabs(t);
  dt = (int) t;
  hundertstel = (int) ((t - dt) * 100);
  if (m_flag) 
    dt = - dt;
}

String delta_t::get_string( void )
{
  String puffer = "";
  ostringstream buf;

  buf << *this;
  buf << '\0';

  puffer.append(buf);
  return puffer;
}

ostream& operator<< ( ostream& strm, delta_t t )
{
  bool m_flag = t.dt < 0;
  int abs_dt;
  int use_dt;
  int num;
  char unit;

  if (m_flag)
    abs_dt = -t.dt;
  else
    abs_dt = t.dt;

  if (t.no_sign)
    {
      use_dt = abs_dt;
      num = 2;
    }
  else
    {
      use_dt = t.dt;
      num = 3;
    }

  strm << setfill(' ');
  if (abs_dt < 60)
    if (t.hundertstel == 0)
      {
	strm << setw(num + 3) << t.dt;
	unit = 's';
      }
    else
      {
	strm << setw(num) << t.dt << ",";
	strm << setw(2) << setfill('0') << t.hundertstel;
	unit = 's';
      }
  else if (abs_dt < 3600)  
    {
      strm << setw(num) << t.dt / 60 << ":"; 
      strm << setw(2) << setfill('0') << abs_dt % 60;
      unit = 'm';
    }
  else if (abs_dt < 86400) 
    {
      strm << setw(3) << t.dt / 3600 << ":";
      strm << setw(2) << setfill('0') << (abs_dt % 3600) / 60;
      unit = 'h';
    }
  else if (abs_dt < 8640000)
    {
      strm << setw(3) << t.dt / 86400 << ":";
      strm << setw(2) << setfill('0') << (abs_dt % 86400) / 3600;
      unit = 'd';
    }
  else if (abs_dt < 31536000)
    {
      strm << setw(3) << t.dt / 604800 << ":";
      strm << setw(2) << setfill('0') << (abs_dt % 604800) / 86400;
      unit = 'w';
    }
  else
    {
      strm << setw(3) << t.dt / 31536000 << ":";
      strm << setw(2) << setfill('0') << (abs_dt % 31536000) / 604800;
      unit = 'y';
    }
  if (!t.no_space)
    strm << ' ';
  strm << unit;
  strm << setfill(' ');
  return strm;
}


cpu_time cpu_time::operator+( const cpu_time &t2)
{
  cpu_time tmp;
  tmp.sys_time = sys_time + t2.sys_time;
  tmp.user_time = user_time + t2.user_time;
  return tmp;
}

cpu_time cpu_time::operator+=( const cpu_time &t2 )
{
  sys_time += t2.sys_time;
  user_time += t2.user_time;
  return *this;
}

cpu_messung::cpu_messung( void )
{
  if (sys_clk_tck == 0)
    sys_clk_tck = 100;

  times(&start);
}

cpu_time cpu_messung::stop( void )
{
   struct tms ende;

   times(&ende);
   long du = (ende.tms_utime - start.tms_utime);
   long ds = (ende.tms_stime - start.tms_stime);

   cpu_time tmp;
   tmp.sys_time =  (double) ds / (double) sys_clk_tck;
   tmp.user_time = (double) du / (double) sys_clk_tck;
   return tmp;
}

void cpu_messung::init( void )
{
  sys_clk_tck = sysconf(_SC_CLK_TCK);
#ifdef _DEBUG_ELOGS_
  cerr << "Ermitelter Sys-clock-Zyklus : " << sys_clk_tck << endl;
#endif
  if (sys_clk_tck == 0)
    {
      sys_clk_tck = 100;
#ifdef _DEBUG_ELOGS_
      cerr << "Auf 100 gesetzt." << endl;
#endif
    }
}
