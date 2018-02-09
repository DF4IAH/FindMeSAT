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
 ****************************************************************************/

#include "connect_string.h"

#include <iostream.h>

connect_string::connect_string( )
{
  digis.clear();
  l2_digis.clear();
  n_digis = 0;
  n_l2_digis = 0;
}

connect_string::connect_string( const String & st )
{
  // Die Klasse connect_String definiert sich aus einem String des
  // Formats "port:mycall>digi_1,[(l2_digi_1,l2_digi_2,...,l2_digi_j)]
  // digi_2,...,digi_i,...,digi_n-1,digi_n"
  // Die Anzahl n der Digis ist dabei beliebig, Die Anzahl der L2-Digis j
  // ist auf 7 begrenzt. Jeder Digi kann eine SSID enthalten
  unsigned int len = st.slen();           // Laenge des Strings ermitteln
  unsigned int index = 0;                 // Indexzaehler
  try
    {
      while (index < len && st[index] != ':') // Doppelpunkt suchen
	index++;
      port = st.copy(0,index);
      
      unsigned int beg = ++index;             // Indexzeiger fuer Beginn eines 
      // Feldes
      String tmp;                             // Temporaerer String
      while (index < len && st[index] != '>') // '>' suchen
	index++;
      tmp = st.copy(beg,index-beg);       // Mycall herauskopieren
      mycall = callsign(tmp);             // Und Callsignklasse daraus erzeugen
      
                                          // ERsten Digi extrahieren 
      beg = ++index;                      // Anfang des Feldes merken
      while (index < len && 
	     st[index] != ',' &&              // naechstes Komma oder '(' suchen
	     st[index] != '(' )
	index++;
      tmp = st.copy(beg,index-beg);           // Digi herauskopieren
      digis.push_back(callsign(tmp));         // In Vektor speichern
      
      bool flag = false;                      // Flagvariable definieren
      n_l2_digis = 0;                         // n_l2_digis initialisieren
      if (index < len && st[index] == '(')    // L2-Digis vorhanden
	{
	  while (!flag && st[index]!=')')     // Alle L2-Digis extrahieren
	    {
	      beg = ++index;                  // Anfang des Feldes merken
	      while (index < len              // Naechstes Komma oder 
		     && st[index] != ','      // oder ')' suchen
		     && st[index] != ')' )
		index++;                      //
	      tmp = st.copy(beg,index-beg);   // Digi herauskopieren und
	      if (len-index==1) index=len;    // workaround dh6bb 25.09.02
	      flag = index == len;            // String zuende?
	      l2_digis.push_back(callsign(tmp));  // im Vektor speichern
	      n_l2_digis++;                   // Ein L2-Digi mehr
	    }
	}
      flag = ++index >= len;                  // Connect-String ausgewertet?
      n_digis = 1;                            // n_digis initialisieren
      while (!flag)                           // Alle Digis extrahieren
	{
	  beg = index;                        // Anfang des Feldes merken
	  while (index < len && st[index] != ',')
	    index++;                          // Naechstes Komma suchen
	  flag = index == len;                // String zuende?
	  tmp = st.copy(beg,index-beg);       // Digi herauskopieren
	  digis.push_back(callsign(tmp));     // Callsignklasse kopieren und in
	  index++;                            // Vektor speichern
	  n_digis++;                          // Ein Digi mehr 
	}
    }
  catch( Error_no_callsign )
    {
      throw Error_syntax_fehler_in_connect_string();
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_syntax_fehler_in_connect_string();
    }

}

void connect_string::PrintOn( ostream &strm )
{
  bool l2=false;
  strm << port << ':';
  mycall.set_format(false);
  strm << mycall << '>';
  vector<callsign>::iterator it1 = digis.begin();
  if ( it1 != digis.end() )
    {
      it1->set_format(false);
      strm << *it1;
      ++it1;
      if (n_l2_digis > 0)
	{
	  l2=true;
	  strm << '(';
	  vector<callsign>::iterator it2 = l2_digis.begin();
	  if (it2 != l2_digis.end())
	    {
	      it2->set_format(false);
	      strm << *it2;
	    }
	  ++it2;
	  while ( it2 != l2_digis.end() )
	    {
	      strm << ',';
	      it2->set_format(false);
	      strm << *it2;
	      ++it2;
	    }
	  strm << ')';
	}
      while ( it1 != digis.end() )
	{
	  if(!l2) strm << ',';
	  l2=false;
	  it1->set_format(false);
	  strm << *it1;
	  ++it1;
	}
    }
} 

String connect_string::get_string( void )
{
  String tmp;

  tmp = port + String(':');
  bool l2=false;
  mycall.set_format(false);
  tmp.append(mycall.call()+String('>'));
  vector<callsign>::iterator it1 = digis.begin();
  if ( it1 != digis.end() )
    {
      it1->set_format(false);
      tmp.append(it1->call());
      ++it1;
      if (n_l2_digis > 0)
	{
	  l2=true;
	  tmp.append('(');
	  vector<callsign>::iterator it2 = l2_digis.begin();
	  if (it2 != l2_digis.end())
	    {
	      it2->set_format(false);
	      tmp.append(it2->call());
	    }
	  ++it2;
	  while ( it2 != l2_digis.end() )
	    {
	      tmp.append(',');
	      it2->set_format(false);
	      tmp.append(it2->call());
	      ++it2;
	    }
	  tmp.append(')');
	}
      while ( it1 != digis.end() )
	{
	  if(!l2) tmp.append(',');
	  l2=false;
	  it1->set_format(false);
	  tmp.append(it1->call());
	  ++it1;
	}
    }

  return tmp;
} 
