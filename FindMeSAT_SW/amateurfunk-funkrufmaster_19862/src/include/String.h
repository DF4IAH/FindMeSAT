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
 *	String.h
 *
 ****************************************************************************/

#ifndef _String_h
#define _String_h

#include "globdef.h"

#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

class Error_String_index_out_of_range
{
#ifdef _DEBUG_EXEC_
 public:
  Error_String_index_out_of_range()
    {
      cerr << "Error_String_index_out_of_range" << endl;
    }
#endif
};

class Error_no_mem_for_String
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_mem_for_String()
    {
      cerr << "Error_no_mem_for_String" << endl;
    }
#endif
};

/*
  Achtung:
  Bei allen Methoden der String-Klasse ist mit den Ausnahmen
  
  Error_no_mem_for_String             und
  Error_String_index_out_of_rang
  
  zu rechnen!
  
*/

class String {
  
 protected:
  char*    cstring;    // Zeichenfolge als dynamischer C-String
  unsigned len;        // Anzahl Zeichen (ohne '\0')
  unsigned size;       // Speicherplatzgroesse von "cstring"
  
 public:
  // Default- und char*-Konstruktor
  String (const char* = "");
  String (const char*, int );

  // Vergleichen von Strings
  friend bool operator== (const String&, const String&);
  friend bool operator!= (const String&, const String&);
  friend bool operator<  (const String&, const String&);
  friend bool operator<= (const String&, const String&);
  friend bool operator>  (const String&, const String&);
  friend bool operator>= (const String&, const String&);
  
  // Hintereinanderhaengen von Strings
  friend String operator+ (const String&, const String&);
  
  // Ausgabe mit Streams
  void printOn (ostream& = cout) const;
  
  // Eingabe mit Streams
  void scanFrom (istream& = cin);
  
  // Aufgrund dynamischer Komponenten
  String (const String&);                   // Copy-Konstruktor
  const String& operator= (const String&);  // Zuweisung
  ~String ();                               // Destruktor
  
  // Index-Operator
  char&      operator[] (unsigned);         // fuer Variablen
  const char operator[] (unsigned) const;   // fuer Konstanten
  
  // Anzahl Zeichen liefern
  const unsigned int slen ( void ) const
    {
      return len;
    }
  
  // Erweiterung von H.Flemming:
  // Friend-Funktion zur Erzeugung einer Char-Kette aus einem String
  
  // Konstruktor von char
  String( char );
  String part ( unsigned int, unsigned int ) const;
  void upcase( void );
  void upcase( unsigned int  , unsigned int  );
  void lowcase( void );
  void lowcase( unsigned int  , unsigned int  );
  bool isnum( void );
  bool in( const String& ) const;
  int pos( const String& ) const;
  String copy( unsigned int, unsigned int ) const;
  void append( const String& );  // Anhaengen eines zweiten Strings
  void append( ostringstream & );   // Anhängen eines Streams
  void get(istream &, unsigned int, char = '\n');
  void getline(istream &, unsigned int, char = '\n');
#ifdef _IPUTIL_
  bool socket_read( int, int );
  bool socket_readln( int, int );
#endif
  int Stoi( void ) const;
  double Stod( void ) const;
  void kuerze( void );
  
  friend const char* strtochar( const String& );
 protected:
  /* Konstruktor fuer uninitialisierten String bestimmter Laenge
   * (intern fuer operator+())
   */
  String (unsigned);
};

// Standard-Ausgabeoperator
inline ostream& operator<< (ostream& strm, const String& s)
{
  s.printOn (strm);    // String auf Stream ausgeben
  return strm;         // Stream zurueckliefern
}

// Standard-Eingabeoperator
inline istream& operator>> (istream& strm, String& s)
{
  s.scanFrom (strm);   // String von Stream einlesen
  return strm;         // Stream zurueckliefern
}

/* Operator !=
 *  - als Umsetzung auf Operator == inline implementiert
 */
inline bool operator!= (const String& s1, const String& s2) {
  return !(s1==s2);
}

String itoS( int, int = 0, bool = false );
//String itoS( unsigned int, int = 0, bool = false );
String dtoS( double, int = 0, int = 0 );
void cut_feld( String& );


#endif  /* _String_h */
  
