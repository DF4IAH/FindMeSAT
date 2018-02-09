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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#ifndef __GLOBDEF_H__
#define __GLOBDEF_H__

// Autokonfigurations-Include-File einbinden
#include "autoconf.h"

#define _IPUTIL_ 1

// Einige Globale Definitionen.

#define VERSION "0.99p "

#ifdef _DEBUG_			// Alles zum debuggen
    #define DEBUG_PFAD "log/"
    #define _DEBUG_ELOGS_  1	// Meldungen und Infos aus dem Programm
    #define _DEBUG_EXEC_   1	// Meldungen aus try/catch
//    #define _DEBUG_DIGI_   1	// Digi- und Linkueberwachung
//    #define _DEBUG_ARLOGS_ 1	// FWD-Autorouter
#endif

#define RUB_ASTRO      "Astro"
#define RUB_WETTER     "Wetter"
#define RUB_DXKW       "DX-KW"
#define RUB_DXKWCW     "DX-KW-CW"
#define RUB_DX50       "DX-50MHz"
#define RUB_DXVUSHF    "DX-V-U-SHF"
#define RUB_DXWX       "HF-Conds"
#define RUB_WSJT       "WSJT"
#define RUB_BAKE       "Bake"
#define RUB_STATUSBAKE "Statusbake"
#define RUB_SAT        "Satelliten"
#define RUB_SATPOS     "Sat-Position"
#define RUB_STATISTIK  "Statistik"
#define RUB_GEZEITEN   "Gezeiten"
#define RUB_DIGI       "Digipeater"
#define RUB_DIGI_STAT  "Digi-Status"
#define RUB_LINK_STAT  "Link-Status"


#define MAX_LOG_LINES                    4000

#define MAX_POLL_WAIT_TIME                  2.

#define WAIT_UNTIL_CONN_RETRY              60
#define WAIT_UNTIL_FWD_CONN_RETRY         600

// Max. Alter eines Funkrufes im Spool-Dir. Wird durch PURGE in der Crontab
// aufgerufen. Zeit in Sekunden.
#define MAX_FUNKRUF_AGE                 14400


// Nun folgen die globalen Definitionen fuer das Forwarding. Dabei
// handelt es sich um einige Zeitkonstanten und um
// Mindest Nachrichtenanzahlen, bei deren Ueberschreitung das Forwarding
// gestartet wird.

#define DEFAULT_CONNECTION_SETUP_TIMEOUT  240

#define DEFAULT_CLUSTER_TIMEOUT          3600
#define DEFAULT_AXHTP_TIMEOUT             600

#define DEFAULT_START_T_W                 120
#define DEFAULT_START_N_MAX                 3
#define DEFAULT_MAX_UNACKED               200
#define DEFAULT_FWD_TIMEOUT               300
#define DEFAULT_MAX_FEHLER_ZAEHLER          0

#define DYN_PARA_T_W                        0.03

#define WAIT_AFTER_MESSBURST              900
#define WAIT_AFTER_MESSUNG                 60
#define MESSUNGEN_IN_BURST                  4

#define Q_MINUS                             0.5
#define Q_PLUS                              1.05
#define Q_PLUS_ABS                          0.1
#define Q_MINUS_ABS                         5.

#define AUTOROUTER_REFRESH               3600
#define AUTOROUTER_LOCAL_REFRESH         3600
#define AUTOROUTER_MAX_ENTRY_AGE1        3600
#define AUTOROUTER_MAX_ENTRY_AGE2        7200
#define MAX_DESTIN_DELAY                  600   //max. Laufzeit die angenommen wird in Sekunden

#define MAX_MID_TAB_SIZE               128000
#define MAX_MID_ENTRIES_WITHOUT_SAVE      250
#define MAX_TIME_BETWEEN_MID_SAVE        3600

#define START_FWD_PERS                      0
#define START_FWD_BUL                       0
#define START_FWD_DEST                     10
#define START_FWD_SONST                    10

#define MAX_FWD_AGE                     86400
#define MAX_FWD_F_AGE	                21600 // Pers.-Nachrichten 6 Stunden
#define MAX_FWD_S_AGE	               172800 // Board-Nachrichten 48 Stunden

enum t_tcp_ports { p_login, p_fwd
#ifdef COMPILE_HTTP
		   , p_http
#endif
#ifdef COMPILE_SMTP
		   ,p_smtp
#endif 
};

#define AX25_IP_ADDRESS            0x0000002c
#define AX25_IP_MASK               0x000000ff
#define MTU_SIZE                   255

// Protokolloptionen

#define OPTION_SKYPERBOARDS                'B'
#define OPTION_ZEITMESSUNG                 'Z'
#define OPTION_ROUTINGAUSTAUSCH            'D'

#define THIS_VERSION_OPTIONEN            "BZD"

#define AUTOROUTER_OPTIONEN               "ZD"

#endif
