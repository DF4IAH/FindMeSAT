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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 ****************************************************************************/

#ifndef __LOGFILE_H__
#define __LOGFILE_H__

#include <iostream>
#include <vector>
#include <netinet/in.h>

#include "globdef.h"
#include "String.h"
#include "database.h"
#include "config.h"
#include "callsign.h"
#include "zeit.h"
#include "destin.h"
#include "tcp.h"

// Die Klasse logfile dient dazu, ein automatisches Logfile
// Mit den wichtigsten Ereignissen zu fuehren

/*
 Der Loglevel wird ab sofort durch eine Logfile-Maske ersetzt.
 In dieser Maske ist bitweise kodiert, welche Logmeldungen freigegeben
 werden sollen.

 Die Zuordnung der Bits zu bestimmten Log-Ereignissen wird im folgenden mit
 Bitdefinitionen durchgefuehrt:
*/


#define LOGMASK_NO           0  // Keine Logeintraege
#define LOGMASK_SIGNALS      1  // Externe Signale
#define LOGMASK_STARTSTOP    2  // Programm Start- und Stoppmeldungen
#define LOGMASK_PRGRMERR     4  // Programmfehlermeldungen
#define LOGMASK_VERBERR      8  // Fehler beim Verbindungsaufbau
#define LOGMASK_PRGRMMDG    16  // Programm-Meldungen
#define LOGMASK_LOGINOUT    32  // Login- und Logout-Meldungen
#define LOGMASK_CNTBPRCS    64  // Crontabprozesse, wie Baken, Boardnamen, Msg-Wiederholung
#define LOGMASK_PRIVVERL   128  // Privilegienverletzung, Nur fuer Sysops erlaubter Zugriff
#define LOGMASK_PWLOG      256  // Password-Eingaben
#define LOGMASK_DBAENDRNG  512  // Aenderungen der Datenbank durch den Sysop
#define LOGMASK_USERINPUT 1024  // benutzereingaben

#define LOGMASK_ID_SIGNALS    'S'
#define LOGMASK_ID_STARTSTOP  'B'
#define LOGMASK_ID_PRGRMERR   'E'
#define LOGMASK_ID_VERBERR    'V'
#define LOGMASK_ID_PRGRMMDG   'M'
#define LOGMASK_ID_LOGINOUT   'L'
#define LOGMASK_ID_CNTBPRCS   'C'
#define LOGMASK_ID_PRIVVERL   'P'
#define LOGMASK_ID_PWLOG      'A'
#define LOGMASK_ID_DBAENDRNG  'D'
#define LOGMASK_ID_USERINPUT  'U'

#define FWDLOGMASK_NO           0  // Keine Logeintraege
#define FWDLOGMASK_INIT         1  // Initialisierungsmeldungen
#define FWDLOGMASK_CONSTRTSTP   2  // Verbindungsaufbau und Beendigung
#define FWDLOGMASK_IFDEBUG      4  // Debugmeldungen des Forward-Interfaces
#define FWDLOGMASK_IFIO         8  // Ein- und Ausgaben des Forward-Interfaces
#define FWDLOGMASK_FWDERR      16  // Fehler im Forward
#define FWDLOGMASK_ROUTER      32  // Routing-aktionen
#define FWDLOGMASK_RTNCHBR     64  // Router, Nachbarerkennung
#define FWDLOGMASK_DESCR      128  // Meldungen der Descriptoren
#define FWDLOGMASK_RXMSG      256  // Im Forward empfangene Meldungen
#define FWDLOGMASK_TIMEMES    512  // Ereignisse in der Zeitmessung
#define FWDLOGMASK_AUTOROUTE 1024  // Ereignisse des Autorouters

#define FWDLOGMASK_ID_INIT        'I'
#define FWDLOGMASK_ID_CONSTRTSTP  'C'
#define FWDLOGMASK_ID_IFDEBUG     'F'
#define FWDLOGMASK_ID_IFIO        'V'
#define FWDLOGMASK_ID_FWDERR      'E'
#define FWDLOGMASK_ID_ROUTER      'R'
#define FWDLOGMASK_ID_RTNCHBR     'N'
#define FWDLOGMASK_ID_DESCR       'D'
#define FWDLOGMASK_ID_RXMSG       'M'
#define FWDLOGMASK_ID_TIMEMES     'Z'
#define FWDLOGMASK_ID_AUTOROUTE   'A'

#define RPCLOGMASK_NO           0  // Keine Logeintraege
#define RPCLOGMASK_INIT         1  // Initialisierungen
#define RPCLOGMASK_CONNECTION   2  // Verbindungsauf- und -abbau
#define RPCLOGMASK_ERROR        4  // Fehler
#define RPCLOGMASK_DATEN        8  // Uebertragene Daten

#define RPCLOGMASK_ID_INIT        'I'
#define RPCLOGMASK_ID_CONNECTION  'C'
#define RPCLOGMASK_ID_ERROR       'E'
#define RPCLOGMASK_ID_DATEN       'D'

#define SPOOLLOGMASK_NO         0 // Keine Logeintraege
#define SPOOLLOGMASK_PERS       1 // persoenliche Nachrichten
#define SPOOLLOGMASK_BUL        2 // Skyper-Rubriken-Nachrichten
#define SPOOLLOGMASK_REP        4 // Wiederholung von Rubrikeninhalten
#define SPOOLLOGMASK_NAMES      8 // Rubrikennamen

#define SPOOLLOGMASK_ID_PERS      'P'
#define SPOOLLOGMASK_ID_BUL       'B'
#define SPOOLLOGMASK_ID_REP       'R'
#define SPOOLLOGMASK_ID_NAMES     'N'

/*
Um den Mechanismus zu frueheren Versionen kompatibel zu halten, bei dem lediglich
Loglevel vorgegeben waren, werden fuer die bisherigen Loglevel Standard Log-Masken
definiert. Ist im Konfigurationsfile lediglich der Parameter Loglevel angegeben, 
wird dieser Loglevel damit automatisch in die entsprechende Standard-Log-Maske 
umgesetzt.
*/

// Loglevel 0 : Keine Meldungen
#define LOGLEVEL_0 LOGMASK_NO   
// Loglevel 1 : Nur Systemmeldungen
#define LOGLEVEL_1 LOGMASK_SIGNALS + LOGMASK_STARTSTOP + LOGMASK_PRGRMERR + LOGMASK_PRGRMMDG + LOGMASK_VERBERR
// Loglevel 2 : Zusaetzlich Logins, Logouts und zeitgesteuerte Prozesse
#define LOGLEVEL_2 LOGLEVEL_1 + LOGMASK_LOGINOUT + LOGMASK_CNTBPRCS + LOGMASK_PRIVVERL + LOGMASK_PWLOG
// Loglevel 3 : Alle Ereignisse, also auch Benutzereingaben
#define LOGLEVEL_3 LOGLEVEL_2 + LOGMASK_USERINPUT + LOGMASK_DBAENDRNG


class Error_wrong_id_in_logmask
{
#ifdef _DEBUG_EXEC_
 public:
  Error_wrong_id_in_logmask()
    {
      cerr << "Error_wrong_id_in_logmask" << endl;
    }
#endif
};

class Error_in_logmask_syntax
{
#ifdef _DEBUG_EXEC_
 public:
  Error_in_logmask_syntax()
    {
      cerr << "Error_in_logmask_syntax" << endl;
    }
#endif
};

class logfile
{
 protected:
  String logfilename;

 public:
  logfile( void );
  String get_filename( zeit zt );
  vector<String> get_lines( int&, int = 15, String = "", char = 'h');
  vector<String> get_lines( int&, int, String, char, zeit );
};

class syslog : public logfile
{
 protected:
  unsigned int logmask;
  unsigned int pagemask;
  String sysop_calls;
  String def_destin;

  void page(const String&, unsigned int );
  String maskid(unsigned int );
  unsigned int maskid( String  );


 public:
  syslog( config_file& );
  void eintrag( const callsign&, const String&, unsigned int );
  void eintrag(const String&, unsigned int );
  String logmaskid( void );
  unsigned int logmaskid( const String & );
  String pagemaskid( void );
  unsigned int  pagemaskid( const String & );
};

class userlog : public logfile
{

 public:
  userlog( config_file& );
  void eintrag( zeit login, zeit logout,  callsign, int = 0, int = 0, char = 'A', char = 'U' , unsigned int = 0, unsigned int = 0);
};

class iplog : public logfile
{
  typedef struct sockaddr_in sadr;

 public:
  iplog( config_file& );
  void eintrag(uint32_t , uint16_t , t_tcp_ports , String = "Zugriff");
};

class httplog : public logfile
{

 public:
  httplog( config_file& );
  void eintrag(uint32_t, const String&, const String&, const String& );
};

class fwdlog : public logfile
{
 protected:
  unsigned int logmask;
  String maskid(unsigned int );
  unsigned int maskid( String  );

 public:
  fwdlog( config_file& );
  void eintrag( const callsign&, const String&, unsigned int );
  void eintrag( const String&, unsigned int );
  String logmaskid( void );
  unsigned int logmaskid( const String & );
};

class rpclog : public logfile
{
  unsigned int logmask;

  String maskid( unsigned int  );
  unsigned int maskid( String  );

 public:
  rpclog( config_file& );
  void eintrag( const callsign & , const String &, unsigned int  );
  String logmaskid( void );
  unsigned int logmaskid( const String & );
};

class spoollog : public logfile
{
 protected:
  unsigned int logmask;

  String maskid( unsigned int  );
  unsigned int maskid( String  );

 public:
  spoollog( config_file& );
  String logmaskid( void );
  unsigned int logmaskid( const String & );
  void eintrag( const callsign&, const callsign&, zeit, const destin& , const String &, unsigned int , unsigned int );
};
#endif
