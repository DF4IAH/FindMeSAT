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

#include "callsign.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iomanip.h>

bool callsign::ispref( String &rz, unsigned int &index )
{
  char c;
  
  c = rz[index++];
  if (isdigit(c))
  { 
    // Erstes Zeichen ist eine Ziffer. Dann muss ein Buchstabe folgen
    
    c = rz[index++];
    if (isalpha(c))
    {
      // Korrekt, Prefix ist gegeben      
      return(true);
    }
    else
    {
      // Dies ist kein gueltiges Rufzeichen
      return(false);
    }
  }
  else 	// Erstes Zeichen war keine Ziffer
  if (isalpha(c))
  {
  	// Erstes Zeichen ist ein buchstabe. Dann >kann< weiterer Buchstabe folgen 	
    c = rz[index];
    if (isalpha(c))
    {
      	// Es folgt weiterer Buchstabe: Dann index anpassen, Gueltiger Prefix !  	
      index++;
      return(true);
    }
    else
    {
      	// Es folgte kein weiterer Buchstabe. 
      	// Fuer Prefixe wie S5 oder Y2 (Gab's mal, fuer die, die sich nicht mehr erinnern...
      	// muss ueberprueft werden, ob ggf. ZWEI Ziffern folgen
      if (isdigit(c))
      {
        // Es folgt zumindest schonmal eine Ziffer...
        c = rz[index+1];
        if (isdigit(c)) index++;// Es folgt eine zweite Ziffer. Dann muss index erhoeht werden
        			// weil die erste Ziffer zum Praefix gehoert
      }	
      
      // Auf jedenfall ein gueltiger prefix !	
      return(true);
    }
  }
  else return(false);	// Kein gueltiger Prefix !!!
}

bool callsign::issuf( String &rz, unsigned int &index )
{
  char c;
  
  c = rz[index++];			// Erstes Zeichen holen
  if (isalpha(c))			// Buchstabe ?
  {					// Ja ! Damit schonmal korrekter Suffix
    if (index < rz.slen())		// Noch weitere Zeichen im String ?
    {					// Ja !
      c = rz[index];			// zeichen holen
      if (isalpha(c))			// Buchstabe ?
      {					// Ja ! Weiter suchen !
        index++;			// Index erhoehen
        if (index < rz.slen())		// Noch weitere Zeichen im String ?
        {				// Ja !
          c = rz[index];		// Dieses zeichen holen
          if (isalpha(c))		// buchstabe ?
          {				// Ja !
            index++;			// Index erhoehen
            return(true);		// Suffix hat drei Buchstaben. Ist OK !
          }			
          else return(true);		// Letztes Zeichen kein Buchstabe, 
            				// Trotzdem OK
        }
        else return(true);		// Suffix ist nach zwei Zeichen zu ende
        				// OK !
      }
      else return(true);		// zweites Zeichen kein Buchstabe
      					// Trotzdem OK
    }
    else return(true);			// Suffix ist nach einem Buchstaben zu ende
    		  			// OK
  }
  else return(false);			// Erstes Zeichen kein Buchstabe
 		   			// Damit kein gueltiges Suffix !!!
}

bool callsign::iscall( String &rz, unsigned int &index )
{
  char c;
  
  try					// Exeptions wegen Bereichsueberschreitung abfangen
  {
    if (ispref(rz,index))		// Auf Prefix ueberpruefen
      {    
	c = rz[index++];		// Naechstes Zeichen
	if (isdigit(c))			// Ziffer ?
	  if (issuf(rz,index))	        // Auf Suffix ueberpruefen
	    return  true;               // Gueltiges Rufzeichen
	  else
	    return false;               // Suffix stimmt nicht, kein gueltiges Rufzeichen
	else
	  return false;                 // Dem Praefix folgt keine Ziffer, kein gueltiges Rufzeichen
      }
    else
      return false;                     // Kein gueltiger Praefix, dann auch kein gueltiges Rufzeichen
  }
  catch (Error_String_index_out_of_range)
  {
    return(false);                      // Bereichsueberschreitung, kein guzeltiges Rufzeichen im String
  }
}



// Default-Konstruktor
callsign::callsign( void )
{
  callsg = String("");
  ssid = 0;
  formatiert = false;
  nossid = false;
}
// Konstruktor mit String als Prameter
callsign::callsign( String call )
{
  char ch;
  unsigned int ind = 0;
  ssid = 0;
  formatiert = false;
  nossid = false;
  for (unsigned int i = 0; i < call.slen() ; i++ ) 
    call[i] = toupper(call[i]);

  if (iscall(call,ind))
    {
      callsg = call.part(0,ind);
      if (ind < call.slen() )
	{
	  ch = call[ind++];
	  if ( ch == '-' )  // ueberpruefen ob noch eine SSID folgt.
	    while ( (ind < call.slen()) && isdigit(ch = call[ind++]) )
	      {
		ssid *= 10;
		ssid += (int) ch - 48;
	      }
	  if (ssid > 15) ssid = 0;
	}
      else ssid = 0;
    }
  else 
    {
#ifdef _DEBUG_ELOGS_
      cerr << "String ." << call << ". enthaelt kein Rufzeichen." << endl;
#endif // _DEBUG_ELOGS_
      throw Error_no_callsign();
    }
} 

String callsign::call( void ) const
{
  char c[10];
  // Wenn nossid gesetzt ist, dann wird generell keine ssid ausgegeben
  if (nossid)
    return callsg;
  else
    // Wenn die SSID 0 ist, dann ebenfalls nur Rufzeichen ohne SSID
    // zurueck geben.
    if (ssid != 0)
      {
	sprintf(c,"%s-%d",strtochar(callsg),ssid);
	return String(c);
      }
    else // Sonst kann der Rufzeichen-String uebergeben werden
      return callsg;
}

ostream& operator<< (ostream & strm, const callsign &cl )
{
  if (cl.formatiert)
    {
      strm << setw(6);
      strm.setf(ios::right, ios::adjustfield );
    }
  strm << cl.callsg;

  if (!cl.nossid)
    if (cl.formatiert)
      if (cl.ssid != 0)
	{
	  strm << '-';
	  strm << setw(2);
	  strm.setf(ios::left, ios::adjustfield );
	  strm << cl.ssid;
	  strm.setf(ios::right, ios::adjustfield );
	}
      else
	strm << "   ";
    else
      if (cl.ssid != 0)
	strm << '-' << cl.ssid;
  return strm;
}

istream& operator>> (istream& strm, callsign &cl )
{
  char ch;
  unsigned int ind = 0;
  String call;

  // Zunaechst mal alle Trennzeichen (, und Leerzeichen) aus dem Stream entfernen.
  while (strm.get(ch) && ((ch == ',') || ( ch == ' '))) ;
  strm.putback(ch);


  // Hier Zeichen aus dem Stream auf aehnliche Weise auswerten, wie
  // beim Konstruktor aus einem String.
  call = String("");
  while (strm.get(ch) && (isalnum(ch) || (ch == '-')) && (ind < 9))
    {
      ch = toupper(ch);
      call = call+String(ch);
      ind++;
    }
  strm.putback(ch);
  cl.ssid = 0;
  ind = 0;

  for (unsigned int i = 0; i < call.slen() ; i++ ) 
    call[i] = toupper(call[i]);
  
  if (cl.iscall(call,ind))
    {
      cl.callsg = call.part(0,ind);
      if (ind < call.slen() )
	{
	  ch = call[ind++];
	  if ( ch == '-' )  // ueberpruefen ob noch eine SSID folgt.
	    while ( (ind < call.slen()) && isdigit(ch = call[ind++]) )
	      {
		cl.ssid *= 10;
		cl.ssid += (int) ch - 48;
	      }
	  if (cl.ssid > 15) cl.ssid = 0;
	}
      else cl.ssid = 0;
    }
  else throw Error_no_callsign();
  return strm;
} 
 

// Zwei Rufzeichen sind gleich, wenn die Rufzeichenstrings und die SSIDs
// identisch sind.

bool operator==( const callsign &cl1, const callsign &cl2 )
{
  bool help = cl1.callsg == cl2.callsg;
  return (help && (cl1.ssid == cl2.ssid) );
}

// Zwei Rufzeichen sind nicht gleich, wenn sich die Rufzeichenstrings oder 
// die SSIDs unterscheiden

bool operator!=( const callsign &cl1, const callsign &cl2 )
{
  bool help = cl1.callsg != cl2.callsg;
  return (help || (cl1.ssid != cl2.ssid) );
}

// samecall ist etwas schwaecher als der Gleichheitsoperator. Hier wird 
// lediglich ueberprueft, ob die beiden Rufzeichen identisch sind. Die
// SSIDs koennen verscheiden sein.

bool samecall( const callsign &cl1, const callsign &cl2 )
{
  return cl1.callsg == cl2.callsg;
}
 
callsign callsign::get_nossidcall( void ) const
{
  callsign tmp;
  tmp.callsg = callsg;
  tmp.ssid = 0;
  tmp.formatiert = false;
  tmp.nossid = false;
  return tmp;
}
