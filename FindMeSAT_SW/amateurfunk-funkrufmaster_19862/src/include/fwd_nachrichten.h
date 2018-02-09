/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
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
 *                                                                          *
 ****************************************************************************/

#ifndef __FWD_NACHRICHTEN_H__
#define __FWD_NACHRICHTEN_H__

#include <iostream>

#include "mid.h"
#include "String.h"
#include "fwd_protokoll.h"
#include "callsign.h"
#include "adress.h"
#include "destin.h"
#include "database.h"
#include "locator.h"
#include "zeit.h"


using namespace std;

#define CR (char) 13

class Error_wrong_message_typ
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_message_typ()
    {
      cerr << "Error_wrong_message_typ" << endl;
    }
#endif
};

class Error_wrong_message_format
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_message_format()
    {
      cerr << "Error_wrong_message_format" << endl;
    }
#endif
};


/* Die Basisklasse Nachricht, von der alle anderen Nachrichten abgeleitet werden.
   Die Klasse enthaelt die MID, die in jeder Nachricht vorhanden ist und 
   Virtuelle Prototypen aller Methoden, die von allen Nachrichten bereitgestellt
   werden muessen.
*/

enum n_types { n_keine, n_eigenschaften, n_funkrufe, n_aenderungen, n_update,
		 n_updateanforderung, n_bestaetigung, n_skyper_board, 
	       n_zeit, n_destination };

// typedef en_n_types n_types;

class nachricht
{
 public:

 public:
  Mid m_id;
 public:
  nachricht(const Mid& );
  String get_feld( const String&, unsigned int &, bool = true);
  void cut_feld( String& );
  virtual ~nachricht( void );
  void PrintOn( ostream & )const;
  void ScanFrom( istream & );
  virtual void PrintOn( String& ) const;
  virtual void ScanFrom( const String & );
  virtual n_types get_typ(void ) const ;
};

inline ostream& operator<<( ostream &strm, const nachricht &n )
{
  n.PrintOn(strm);
  return strm;
}

inline istream& operator>>(istream &strm, nachricht &n )
{
  n.ScanFrom(strm);
  return strm;
}

/* 
   Eigenschaften-nachricht dient zur uebermittlung der Protokolleigenschaften
*/

class eigenschaften_nachricht : public nachricht
{
 public:
  String Version;
  protokoll_optionen Optionen;

 public:
  eigenschaften_nachricht( const Mid & , String = PROT_VERSION, protokoll_optionen = protokoll_optionen());
  void PrintOn( String& )const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const ;
};

// Funkrufnachricht
class funkruf_nachricht : public nachricht
{
 public:
  callsign absender;
  adress adr;
  destin dest;
  char domain;
  char typ;
  unsigned int priority;
  callsign master;
  String text;

 public:
  funkruf_nachricht( const Mid & );
  void PrintOn( String& )const;
  void ScanFrom(const String& );
  n_types get_typ( void )const;
};

class skyper_rubrik_nachricht : public nachricht
{
 public:
  callsign absender;
  String board;
  int slot;
  destin dest;
  unsigned int priority;
  int lifetime;
  callsign master;
  String text;

 public:
  skyper_rubrik_nachricht( const Mid & );
  void PrintOn( String & )const;
  void ScanFrom( const String & );
  n_types get_typ( void )const ;
};

// Datenbankaenderungsnachricht
class datenbankaenderung_nachricht : public nachricht
{
 public:
  char typ;
  callsign rufzeichen;
  adress adr;
  pager_typ ptyp;
  locator loc;
  String name;
  zeit tm;
  callsign master;

 public:
  datenbankaenderung_nachricht( const Mid & );
  void PrintOn( String& )const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const;
};

// Datenbankupdatenachricht
class datenbankupdate_nachricht : public nachricht
{
 public:
  char typ;
  callsign rufzeichen;
  adress adr;
  pager_typ ptyp;
  locator loc;
  String name;
  zeit tm;
  callsign master;

 public:
  datenbankupdate_nachricht( const Mid & );
  void PrintOn( String& )const;
  void ScanFrom(const String& );
  n_types get_typ( void )const;
};

// Datenbankaenderungsanforderungsnachricht
class datenbankanforderung_nachricht : public nachricht
{
 public:
  zeit tm;

 public:
  datenbankanforderung_nachricht( const Mid & );
  void PrintOn( String& )const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const;
};

// Bestaetigungsnachricht
class bestaetigungs_nachricht : public nachricht
{
 public:
  zeit tm;

 public:
  bestaetigungs_nachricht( const Mid & );
  void PrintOn( String& ) const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const;
};

// Zeituebermittlungsnachricht dient zur Laufzeit und Uhrdifferenzmessung
class zeit_nachricht : public nachricht
{
 public:
  int version;
  unsigned int li;
  char typ;
  unsigned int stratum;
  int praez;
  String ident;
  pzeit t_ref;
  pzeit t_orig;
  pzeit t_rx1;
  pzeit t_tx;

 public:
  zeit_nachricht( const Mid & );
  void PrintOn( String& ) const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const;
};

//Zielgebietsnachricht dient zur Weiterleitung von Routinginformationen
class zielgebiets_nachricht : public nachricht
{
 public:
  destin zielgebiet;
  double delay;

 public:
  zielgebiets_nachricht( const Mid & );
  void PrintOn( String& ) const;
  void ScanFrom(const String& );
  n_types get_typ( void ) const;
};



#endif
