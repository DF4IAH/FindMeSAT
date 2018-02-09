/****************************************************************************
 *
 * String1: Erste Versionen der Klasse String (mit Index-Operator)
 *
 *  - Beispielprogramm zum Buch "Objektorientiertes Programmieren in C++",
 *    von Nicolai Josuttis, erschienen bei Addison-Wesley, ISBN 3-89319-637-4
 *
 *  - Abschnitt 3.9: Dynamische Komponenten
 *    Abschnitt 3.10: Weitere Aspekte dynamischer Komponenten
 *  
 ****************************************************************************/

/****************************************************************************
 *
 * Copyright 1994 by Nicolai Josuttis and Addison-Wesley.
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software for personal
 * and educational use is hereby granted without fee, provided that the above
 * copyright notice appear in all copies and that both that copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the names of Addison-Wesley or the authors are not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  Addison-Wesley and the authors make no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * ADDISON-WESLEY AND THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL ADDISON-WESLEY OR THE AUTHORS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 ****************************************************************************/


/****************************************************************************
 *
 *	String.cc / String.cpp
 *
 ****************************************************************************/

// Header-Datei der eigenen Klasse
#include "String.h"

// C-Header-Dateien fuer String-Funktionen
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <string>

#include "globdef.h"

#ifdef _IPUTIL_
#include "iputil.h"
#endif

/* Konstruktor aus C-String (char*)
 *  - Default fuer s: ""
 */

inline char* alloc_string(unsigned int &size)
{
  if (size % 2 != 0) 
    size++;
  char *s = new char [size];
  if (s == NULL)
    throw Error_no_mem_for_String();
  return s;
}

String::String (const char* s)
{
    len = strlen (s);             // Anzahl Zeichen
    size = len + 1;               // Size ist Anzahl Zeichen plus '\0'
    cstring = alloc_string(size); // Speicherplatz anlegen
    strncpy (cstring, s, size);   // Zeichen in Speicherplatz kopieren
    cstring[len]= (char) 0;
}

String::String (const char* s, int anz)
{
    len = anz;                    // Anzahl Zeichen
    size = len + 1;               // Size ist Anzahl Zeichen plus '\0'
    cstring = alloc_string(size); // Speicherplatz anlegen
    memcpy (cstring, s, size);   // Zeichen in Speicherplatz kopieren
}

/* Copy-Konstruktor
 */
String::String (const String& s)
{
    len = s.len;                       // Anzahl Zeichen uebernehmen
    size = len + 1;                    // Size ist Anzahl Zeichen plus '\0'
    cstring = alloc_string(size);      // Speicherplatz anlegen
    memcpy (cstring, s.cstring,len); // Zeichen kopieren
    cstring[len]= (char) 0;
}

/* Destruktor
 */
String::~String ()
{
    // Mit new angelegten Speicherplatz wieder freigeben
    delete [] cstring;
}

/* operator =
 *  - Zuweisung
 */
const String& String::operator= (const String& s)
{
    // Zuweisung eines Strings an sich selbst macht nichts
    if (this == &s) {
        return *this;                 // String zurueckliefern    
    }

    len = s.len;             // Anzahl Zeichen uebernehmen

    // falls Platz nicht reicht, vergroessern
    if (size < len + 1) {
        delete [] cstring;            // alten Speicherplatz freigeben
        size = len + 1;               // Anzahl Zeichen plus '\0'
        cstring = alloc_string(size);    // Speicherplatz anlegen
    }

    memcpy (cstring, s.cstring,len);      // Zeichen kopieren
    cstring[len]= (char) 0;
    return *this;            // geaenderten String zurueckliefern    
}

/* operator ==
 *  - vergleicht zwei Strings
 *  - globale Friend-Funktion, damit automatische
 *    Typumwandlung des ersten Operanden moeglich ist
 */
bool operator== (const String& s1, const String& s2)
{
  if (s1.len != s2.len)
    return false;
  else
    return memcmp(s1.cstring,s2.cstring,s1.len) == 0;
}

/*
bool operator!= (const String& s1, const String& s2)
{
  if (s1.len != s2.len)
    return true;
  else
    return strncmp(s1.cstring,s2.cstring,s1.len) != 0;
}
*/
bool operator< (const String &s1, const String &s2)
{
  //cerr << "Stringvergleich ." << s1 << ". (" << s1.len << ") < .";
  //cerr << s2 << ". (" << s2.len << ')' << endl;
  unsigned int cnt = s1.len;
  if (s2.len < cnt)
    cnt = s2.len;
  int r = memcmp(s1.cstring,s2.cstring,cnt);
  //cerr << "Ergebnis : " << r << endl;
  if (r == 0)
    return s1.len < s2.len;
  else
    return r < 0;
}

bool operator<= (const String &s1, const String &s2)
{
  unsigned int cnt = s1.len;
  if (s2.len < cnt)
    cnt = s2.len;
  int r = memcmp(s1.cstring,s2.cstring,cnt);
  if (r == 0)
    return s1.len <= s2.len;
  else
    return r <= 0;
}

bool operator> (const String &s1, const String &s2)
{
  unsigned int cnt = s1.len;
  if (s2.len < cnt)
    cnt = s2.len;
  int r = memcmp(s1.cstring,s2.cstring,cnt);
  if (r == 0)
    return s1.len > s2.len;
  else
    return r > 0;
}

bool operator>= (const String &s1, const String &s2)
{
  unsigned int cnt = s1.len;
  if (s2.len < cnt)
    cnt = s2.len;
  int r = memcmp(s1.cstring,s2.cstring,cnt);
  if (r == 0)
    return s1.len > s2.len;
  else
    return r > 0;
}

/* operator +
 *  - haengt zwei Strings hintereinander
 *  - globale Friend-Funktion, damit automatische
 *    Typumwandlung des ersten Operanden moeglich ist
 */
String operator+ (const String& s1, const String& s2)
{
    // uninitialisierten Summen-String erzeugen
    String sum (s1.len + s2.len);
    // sum ist mit (char) 0 vorinitialisiert!
//    String sum (s1.len + s2.len + 1);

    // Summen-Zeichenfolge darin initialisieren
    memcpy (sum.cstring, s1.cstring,s1.len);
    char *pt2 = sum.cstring + s1.len;
    memcpy(pt2,s2.cstring,s2.len);
    

    // Kopie von Summen-String zurueck
    return sum;
}

/* Konstruktor fuer uninitialisierten String bestimmter Laenge
 *  - intern fuer operator+
 */
String::String (unsigned l)
{
    len = l;                      // Anzahl Zeichen uebernehmen
    size = len + 1;               // Size ist Anzahl Zeichen plus '\0'
    cstring = alloc_string(size);    // Speicherplatz anlegen
    memset(cstring,(char) 0,size);
}

/* Ausgabe auf Stream
 */
void String::printOn (ostream& strm) const
{
    // Zeichenfolge einfach ausgeben
    strm << cstring;
}

void String::scanFrom (istream& strm)
{
    char c;

    len = 0;         // zunaechst ist der gelesene String leer

    strm >> ws;      // fuehrende Trennzeichen ueberlesen

    /* solange Input-Stream "strm" nach Einlesen
     * eines Zeichens in "c" in Ordnung ist
     */
    while (strm.get(c)) {       // >> wuerde Trennzeichen ueberlesen

        /* Newline schliesst Stringeingabe ab
         *  - '\n' => RETURN
         */
        if (c == '\n') {
            cstring [len] = '\0';
            return;
        }

        /* falls der Platz nicht mehr ausreicht,
         * muss mehr geschaffen werden
         */
        if (len+1 >= size) {
            char* tmp = cstring;       // Zeiger auf alten Speicherplatz
            size += 80;                // 80 Zeichen mehr Platz
            cstring = alloc_string(size);  // neuen Speicherplatz anlegen
            strncpy (cstring, tmp,len);     // Zeichen kopieren
            delete [] tmp;             // alten Speicherplatz freigeben
        }

        // neues Zeichen eintragen
        cstring [len] = c;
        len ++;
    }

    /* Einlese-Ende durch Fehler oder EOF
     *  - Stringende-Kennzeichen eintragen
     */
    cstring [len] = '\0';
}


/* Operator [] fuer Variablen
 */
char& String::operator [] (unsigned i)
{
    // Index nicht im erlaubten Bereich ?
    if (i >= len)
      throw Error_String_index_out_of_range();
    return cstring[i];
}

/* Operator [] fuer Konstanten
 */
const char String::operator [] (unsigned i) const
{
    // Index nicht im erlaubten Bereich ?
    if (i >= len)
      throw Error_String_index_out_of_range();
    return cstring[i];
}

const char * strtochar( const String& st)
{
  return st.cstring;
}


String::String (char ch )
{
  len = 1;                      // Anzahl Zeichen
  size = len + 1;               // Size ist Anzahl Zeichen plus '\0'
  cstring = alloc_string(size);    // Speicherplatz anlegen
  *cstring = ch;                // Zeichen in Speicherplatz kopieren
  cstring[1] = '\0';        // '\0' anhaengen
}

String String::part( unsigned int start, unsigned int plen ) const
{
  if (plen == 0)
    return String("");
  else
    {
      if ( start + plen > len )
	throw Error_String_index_out_of_range();

      String puffer(plen);
      memcpy(puffer.cstring,&cstring[start],plen);
      puffer.cstring[puffer.len] = '\0';
      return puffer;
    }
}

void String::upcase( void )
{
  for (unsigned int i = 0 ; i < len ; i++)
    cstring[i] = toupper(cstring[i]);
}

void String::upcase( unsigned int start, unsigned int stop )
{
  for ( unsigned int i = start ; i <= stop && i < len ; i++ )
    cstring[i] = toupper(cstring[i]);
}

void String::lowcase( void )
{
  for (unsigned int i = 0 ; i < len ; i++)
    cstring[i] = tolower(cstring[i]);
}

void String::lowcase( unsigned int start, unsigned int stop )
{
  for ( unsigned int i = start ; i <= stop && i < len ; i++ )
    cstring[i] = tolower(cstring[i]);
}

bool String::isnum( void )
{
  for (unsigned int i = 0 ; i < len ; i++)
    if (!isdigit(cstring[i]))
      return false;

  return true;
}

bool String::in( const String & part ) const
{
  return ( pos(part) != -1 );
}

int String::pos( const String &part ) const
{
  if (part.len > len)
    return -1;

  char *pt = cstring;

  for (unsigned int i = 0; i < (len - part.len) + 1; i++ )
    {
      if (memcmp(pt,part.cstring,part.len) == 0)
	return i;
      pt++;
    }

  return -1;
}

String String::copy( unsigned int start, unsigned int laenge) const
{
  return part(start,laenge);
}

void String::append( const String &ap )
{
  if (len + ap.len + 1  > size)     // Laenger als vorhandener Speicherplatz ?
    {
      char *cptr;                 // Speicherbereich fuer neuen String holen
      size = len + ap.len +1;
      cptr = alloc_string(size);       // Laenge des Speicherbereiches
      memcpy(cptr,cstring,len);          // Inhalt kopieren
      delete [] cstring;             // Alten Speicherbereich freigeben
      cstring = cptr;                // cstring-Zeiger auf neuen Bereich setzen
    }

  char *pt = cstring + len;          // Zeiger auf Speicherposition nach String
  len = len + ap.len;                // Neue Stringlaenge berechnen
  memcpy(pt,ap.cstring,ap.len);      // String zusammenfuegen
  cstring[len] = '\0';
}

void String::append( ostringstream &ostr )
{
  ostr << '\0';
  string st = ostr.str();
  unsigned int len2 = strlen(st.data());

  if (len + len2 + 1  > size)     // Laenger als vorhandener Speicherplatz ?
    {
      char *cptr;                 // Speicherbereich fuer neuen String holen
      size = len + len2 +1;
      cptr = alloc_string(size);       // Laenge des Speicherbereiches
      memcpy(cptr,cstring,len);          // Inhalt kopieren
      delete [] cstring;             // Alten Speicherbereich freigeben
      cstring = cptr;                // cstring-Zeiger auf neuen Bereich setzen
    }

  char *pt = cstring + len;          // Zeiger auf Speicherposition nach String
  len = len + len2;                // Neue Stringlaenge berechnen
  memcpy(pt,st.data(),len2);      // String zusammenfuegen
  cstring[len] = '\0';
}


void String::get( istream &strm, unsigned int nmax, char ende )
{
  if (nmax+1 > size)
    {
      char *cptr;
      size = nmax + 1;
      cptr = alloc_string(size);
      delete [] cstring;
      cstring = cptr;
    }
  strm.get(cstring,nmax,ende);
  len = strlen(cstring);
}

void String::getline( istream &strm, unsigned int nmax, char ende )
{
  if (nmax+1 > size)
    {
      char *cptr;
      size = nmax + 1;
      cptr = alloc_string(size);
      delete [] cstring;
      cstring = cptr;
    }
  strm.getline(cstring,nmax,ende);
  len = strlen(cstring);
}

#ifdef _IPUTIL_
bool String::socket_read( int socketfd, int n_max )
{
  if (n_max+1 > (int) size)
    {
      char *cptr;
      size = n_max + 1;
      cptr = alloc_string(size);
      delete [] cstring;
      cstring = cptr;
    }
  //int n_read = readn(socketfd,cstring,n_max);
  int n_read = read(socketfd,cstring,n_max);
  if (n_read > 0)
    {
      len = n_read;
      cstring[len] = '\0';
      return true;
    }
  else
    {
      len = 0;
      return false;
    }
}

bool String::socket_readln( int socketfd, int n_max )
{
  if (n_max+1 > (int) size)
    {
      char *cptr;
      size = n_max + 1;
      cptr = alloc_string(size);
      delete [] cstring;
      cstring = cptr;
    }
  int n_read = readline(socketfd,cstring,n_max);
  if (n_read > 0)
    {
      len = n_read;
      cstring[len] = '\0';
      return true;
    }
  else
    {
      len = 0;
      return false;
    }
}
#endif

int String::Stoi( void ) const
{
  return atoi(cstring);
}


double String::Stod( void ) const
{
  double d;
  sscanf(cstring,"%lf",&d);
  return d;
}

String itoS( int i , int l, bool hexflag )
{
  ostringstream buf;

  if (l > 0) 
    buf << setw(l);
  if (hexflag)
    buf << hex;
  buf << i << '\0';
  string st = buf.str();
  String tmp = String(st.data());


  return tmp;
}



String dtoS( double d, int l, int prec )
{


  char puffer[100];
  char frmt[100];
  sprintf(frmt,"%%%d.%dlf",l,prec);
  sprintf(puffer,frmt,d);

  String tmp(puffer);
  return tmp;
}

void String::kuerze( void )
{
  int l = (int) len;
  if (l > 1)
    {
      for (int i = l-1 ; i >= 0 ; i--)
        if ( cstring[i] != ' ' && cstring[i] != '\0')
	  {
	    if (i != l-1)
	      {
	        cstring[i+1] = '\0';
	        len = i+1;
	      }
	    return;
	  }
      *cstring = '\0';
      len = 0;
    }
}
