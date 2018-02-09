/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Jens Schoon                                        *
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
 * Jens Schoon, DH6BB             email : dh6bb@darc.de                     *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *  		                                			    *
 * List of other authors:                                                   *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *  		                                			    *
 ****************************************************************************/

#ifndef __RPC_INTERFACE_H__
#define __RPC_INTERFACE_H__

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <features.h>

#include "String.h"
#include "board.h"
#include "interfaces.h"
#include "slaves.h"
#include "spoolfiles.h"

using namespace std;

typedef char text_t[501];
typedef unsigned short systime_t;

//Timer (alle Angaben in Sekunden)
#define INTERVAL_SYNC		(30*60)
#define INTERVAL_SCAN		30

//Wartezeit (in Sekunden), bevor ein erneuter Connect des Slaves versucht wird.
#define CONNECT_RETRY_DELAY	30	

//Anzahl der RTT-Messwerte beim Synchronisieren der Slave-RTC
#define SYNC_SAMPLE_CNT		10

//Gesamtanzahl der Funkruf-Slots
#define SLOT_CNT		16
//Meldungen des Slaves
#define MSG_ACK			'+'
#define MSG_NACK		'-'
#define MSG_RETRY		'%'
#define MSG_NUM			'#'

//Funkruf-Typen
#define P_TYPE_RAW	0	/* Raw-Funkruf */
#define P_TYPE_NUMERIC	1	/* Numerischer Funkruf */
#define P_TYPE_ALPHA	2	/* Alpha-Numerischer Funkruf */

#define PASSIVE		false
#define AKTIV		true

#define START		0
#define INIT		1
#define TXSYNC		2
#define RXSYNC		3
#define SLOTS		4
#define RXSLOTS		5
#define TXPAGE		6
#define RXPAGE		7
#define WAITACK_PW	8

class rpc_interface : public interfaces
{
protected:
  zeit last_activity;
  String slots_str;	// Sendeslots des Slave
  time_t bc_last_gen;	// Zeitpunkt der letzten Bakengenerierung
  int bc_interval;	// Bakeninterval in Sekunden
  systime_t time_rx, time_tx;
  int status;
  bool mode;
  bool ack;
  bool want_sync;
  String rx_line;
  String spool_dir;
  callsign slave_call;
  int synctimer;
  int max_age;		// max. Alter eines Funkrufes
  int *timeout;
  String passwort;
  
  struct Page 
  {
	struct Page	*prev;		// Zeiger auf vorigen Funkruf
	struct Page	*next;		// Zeiger auf naechsten Funkruf
	char		file[NAME_MAX + 1]; //Zugehoerige Spool-Datei	
	unsigned int	type;		// P_TYPE_RAW, P_TYPE_NUMERIC, ...
	unsigned int	speed;		// Sendegeschwindigkeit in bps
	unsigned long	address;	// Pager-Adresse
	unsigned int	func;		// Zustand der Funktions-Bits
	unsigned int	frame;		// sende-Frame
	text_t 		text;
  };

  struct PageQueue 
  {
	struct Page	*first;
	struct Page	*last;
	unsigned long	pcnt;
  };

 struct SCP 
  {
	int			seq_tx;		// Sende-Sequenznummer
	int			seq_ack;	// Ack-Sequenznummer
	time_t			last_sync;	// Letzte RTC-Synchronisation
	time_t			last_scan;	// Letzter Spooler-Scan
	unsigned int		slots;
	int			slots_unlocked;
	struct PageQueue	pq;
  }scp;


protected:
  systime_t systime ();
  int scp_check_sid (ostream&);
  int scp_send_pages (ostream&);
  int sp_init (int);
  void sp_chomp (char*);
  int sp_addpage (struct PageQueue *pq, char *file);
  int sp_get (struct PageQueue *pq, int);
  void sp_age (void);
  void sp_remove (struct Page *p);
  int scp_unlock (ostream&);
  int scp_ack ();
  void pq_init (struct PageQueue *pq);
  void pq_enqueue (struct PageQueue *pq, struct Page *p);
  void pq_dequeue (struct PageQueue *pq, struct Page *p);
  void pq_clean (struct PageQueue *pq);
  unsigned long pq_cnt (struct PageQueue *pq);
  int scp_sync_slave (ostream&, bool);
  void bc_add_text (struct PageQueue *pq);
  void bc_add_time (struct PageQueue *pq);
  void bc_add (struct PageQueue *pq);
  void init_scp(void);
  String check_pw(int[]);
  bool do_line( const String &, String & );

public:
  rpc_interface(String&, bool ax_flag, const callsign&, const connect_string&, const String&, const String&, const String& ,slave::slave_mode, const String&);
  ~rpc_interface();
  bool do_process( bool, String & );
};

#endif
