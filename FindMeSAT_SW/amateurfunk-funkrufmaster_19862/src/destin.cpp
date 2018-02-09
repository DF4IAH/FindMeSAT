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

#include "destin.h"
#include "config.h"
#include "logfile.h"


extern config_file configuration;

void destin::calc_ps( void )
{
  unsigned short tmp = 0;

  for (vector<String>::iterator it = felder.begin(); it != felder.end(); ++it)
    for (unsigned int i = 0 ; i < it->slen(); ++i)
      tmp += (*it)[i];

  tmp <<= 4;
  tmp += (felder.size() & 0xf );
  pruefsumme = tmp;
}

bool destin::check_ps( void ) const
{
  unsigned short tmp = 0;

  for (unsigned int fi = 0; fi < felder.size(); ++fi)
    for (unsigned int i = 0 ; i < felder[fi].slen(); ++i)
      tmp += felder[fi][i];

  tmp <<= 4;
  tmp += (felder.size() & 0xf );
#ifdef _DEBUG_ELOGS_
  if (pruefsumme != tmp)
    {
      bool first = true;
      for (unsigned int fi = 0; fi < felder.size(); ++fi)
	{
	  if (first)
	    {
	      cerr << '@';
	      first = false;
	    }
	  else
	    cerr << '.';
	  
	  cerr << felder[fi];
	}
      cerr << endl;
      cerr << "Pruefsumme " << pruefsumme << " Berechnet : " << tmp << endl; 
    }
#endif
  return (pruefsumme == tmp);
}

destin::destin(const String &d )
{ 
  felder.clear();
  unsigned int index = 0;
  unsigned int len = d.slen();
  while (index < len && ((d[index] == ' ') || (d[index] == ',')))
    index++; 
  // Fuehrende Leerzeichen und Kommata loeschen

  if ( d[index] != '@' ) // erstes Zeichen muss at-sign sein.
    throw Error_no_destin();
  else
    {
      index++;
      while (  index < len && 
	      (   isalnum(d[index]) 
	       || d[index] == '.' 
	       || d[index] == '*') )
	{
	  String tmp;
	  if (d[index] == '.') 
	    index++;
	  while (index < len && (isalnum(d[index]) || d[index] == '*') )
	    {
	      tmp.append( String( (char) toupper(d[index]) ) );
	      index++;
	    }
	  felder.push_back(tmp);
	}
    }
  calc_ps();
}

ostream& operator<< ( ostream& strm, const destin &d )
{
  if (!d.check_ps() )
    throw Error_destin_checksum_error();
  else
    {
      bool first = true;
      strm << '@';   // Ausgabe an Stream mit vorangestelltem at-sign
      for (unsigned int i = 0; i < d.felder.size() ; i++)
	{
	  if (first)
	    first = false;
	  else
	    strm << '.';
	  strm << d.felder[i];
	}
    }
  return strm;
}

istream& operator>> ( istream& strm, destin &d )
{
  char ch;
  d.felder.clear();
  while (strm.get(ch) && ((ch == ' ') || (ch == ','))) ; 
  // Fuehrende Leerzeichen und Kommata loeschen

  if ( ch != '@' ) // erstes Zeichen muss at-sign sein.
    {
      strm.putback(ch);
      throw Error_no_destin();
    }
  else
    {
      strm.get(ch);
      while ( !strm.eof() && (isalnum(ch) || ch == '.' || ch == '*') )
	{
	  String tmp;
	  if (ch == '.') 
	    strm.get(ch);
	  while (!strm.eof() && (isalnum(ch) || ch == '*') )
	    {
	      tmp.append( String( (char) toupper(ch) ) );
	      strm.get(ch);
	    }
	  d.felder.push_back(tmp);
	}
    }
  d.calc_ps();
  return strm;
}

bool operator== ( const destin &d1, const destin &d2 )
{
  if (! (d1.check_ps() && d2.check_ps() ) )
    throw Error_destin_checksum_error();
  else
    if (d1.felder.size() == d2.felder.size())
      {
	bool flag = true;
	for (unsigned int i = 0 ; i < d1.felder.size() && flag; i++)
	  flag = d1.felder[i] == d2.felder[i];
	return flag;
      }
    else
      return false;
  return false;
}

bool operator!= ( const destin &d1, const destin &d2 )
{
  return !(d1 == d2);
}

/* destin::in(const destin &d ) ueberprueft, ob sich die Funkrufzone d im 
   Bereich des Objektes der Klasse destin befindet.

   Beispiel:
   d ist @db0iuz.o.dl.eu, *this enthaelt @dl.eu, dann ist die bedingung 
   erfuellt. Enthaelt *this dagegen @l.dl.eu, ist die Bedingung nicht erfuellt.

   Ebenfalls nicht erfuellt ist die Bedingung, wenn d nur @dl.eu enthaelt 
   und *this dagegen @db0iuz.o.dl.eu. 
*/
/*
   Anmerkung DH6BB:
   Hier muss noch einiges umgebaut werden. Man sollte an "DH6BB @DL" pagen
   koennen. Oder aber an "DH6BB @I" (fuer Distrikt "I"). Das geht derzeit
   noch nicht!!
   Weiterhin sollte auch "DH6BB @DB0WHV" gehen. Wie verwalten wir das?
   Das wird problematisch beim routen.......
*/

bool destin::in( const destin &d ) const 
{
  if (!check_ps() || !d.check_ps() )
    throw Error_destin_checksum_error();
  else
    {
      if (*this == d)
	return true;
      else if (felder.size() == 1 && felder[0] == String('*'))
	return true;
      else
	{
	  unsigned int ind1 = felder.size();
	  unsigned int ind2 = d.felder.size();
	  if (ind1 < ind2)
	    {
	      bool flag = true;
	      while (flag && ind1 >0)
		flag = felder[--ind1] == d.felder[--ind2];
	      return flag;
	    }
	  else
	    return false;
	}
    }
  return false;
}

String destin::get_string( void ) const
{
  if (!check_ps() )
    throw Error_destin_checksum_error();

  String tmp('@');
  bool first = true;
  for (unsigned int i = 0 ; i < felder.size() ; i++ )
    {
      if (first)
	first = false;
      else
	tmp.append('.');
      tmp.append(felder[i]);
    }
  return tmp;
}

destin get_default_destin( void )
{
  try
    {
      String def_string = configuration.find("DEFAULT_DESTINATION");
      return destin(def_string);
    }
  catch( Error_parameter_not_defined )
    {
      syslog slog(configuration);
      slog.eintrag("Parameter DEFAULT_DESTINATION nicht definiert",LOGMASK_PRGRMERR);
      return destin();
    }
  catch( Error_no_destin )
    {
      syslog slog(configuration);
      slog.eintrag("Parameter DEFAULT_DESTINATION enthaelt keine Destination",LOGMASK_PRGRMERR);
      return destin();
    }
}


own_destins::own_destins( void )
{
  dests.clear();
}

own_destins::own_destins( const String &dstring )
{
  destin dest;
  
  istringstream dstream(strtochar(dstring));
  
  int i = 0;
  while (!dstream.eof() && i < 20) 
    {
      // Alle, aber maximal 20 Destinations auslesen und
      dstream >> dest;
      // In Datenbank abspeichern
      dests.push_back(dest);
      i++;
    }
}

bool own_destins::check_destin( const destin &d )
{
  for (t_dests::iterator it = dests.begin(); it != dests.end(); ++it )
  {
    if (d.in(*it))
    {
      return true;
    }
  }
  return false;
}

void own_destins::PrintOn( ostream &strm ) const
{
  t_dests tmp = dests;

  t_dests::iterator it = tmp.begin();
  if (it != tmp.end())
    {
      strm << *it;
      ++it;
      while ( it != tmp.end() )
	{
	  strm << ',' << *it;
	  ++it;
	}
    }
}

String own_destins::get_string( void ) const
{
  String ts;

  t_dests tmp = dests;

  t_dests::iterator it = tmp.begin();
  if (it != tmp.end())
    {
      ts = it->get_string();
      ++it;
      while ( it != tmp.end() )
	{
	  ts.append(String(',')+it->get_string());
	  ++it;
	}
    }
  return ts;
}
