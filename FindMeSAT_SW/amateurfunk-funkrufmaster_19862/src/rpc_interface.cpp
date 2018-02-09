/****************************************************************************
 *                                                                          *
 *	based on  scp.c	Slave Control Protocol                              *
 *	04.05.2001,	Klaus Hennemann                                     *
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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 * List of other authors:                                                   *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 *  		                                			    *
 ****************************************************************************/


#include "rpc_interface.h"
#include "globdef.h"

extern config_file configuration;
extern spoolfiles spool;
extern slave_control slaves;

// Im Konstruktor werden alle benoetigten Variablen initialisiert und der erste
// Connect-Schritt zum RPC durchgefuehrt.
rpc_interface::rpc_interface(String &outp, bool ax_flag, const callsign &rpc, const connect_string &cs, const String &bake, const String &slots, const String& path, slave::slave_mode mo, const String &pw) : interfaces(outp,ax_flag)
{
  last_activity = zeit();
  passwort=pw;
  slave_call=rpc;
  scp.seq_tx=0;		// Sende-Sequenznummer
  scp.seq_ack=0;	// Ack-Sequenznummer
  scp.last_sync=0;	// Letzte RTC-Synchronisation
  scp.last_scan=0;	// Letzter Spooler-Scan
  scp.slots=0;		// freizugebene Slots
  scp.slots_unlocked=0; // Anzahl freigegebener Slots
  bc_last_gen = 0;      // Zeitpunkt der letzten Bakengenerierung
  bc_interval = 60;     // Bakeninterval in Sekunden
  max_age=MAX_FUNKRUF_AGE;	// max. Alter eines Funkrufes ==> global.h
  spool_dir=path;	// Spool-Dir
  set_connect_path(cs); // Connect-Pfad
  first_connect(outp);
  interface_id = 'R';
  status=START;
  
  if (mo==slave::m_activ) mode=AKTIV;
  else                    mode=PASSIVE;
  synctimer=SYNC_SAMPLE_CNT;
  want_sync=false;

  if(mode) slots_str=slots;
  
}

// Der Destruktor
rpc_interface::~rpc_interface( void )
{
  if (connect_failed)
    slaves.set_status(slave_call,slave::sts_gescheitert);
  else
    slaves.set_status(slave_call,slave::sts_getrennt);
}

// wird aufgerufen, wenn Daten anliegen

bool rpc_interface::do_line( const String &rx_line, String &outp )
{
  rpclog rpclogf(configuration);
  ostringstream ostr;

  time_rx = systime();
  time_t curtime=time(NULL);

  rpclogf.eintrag(slave_call,"< " + rx_line,RPCLOGMASK_DATEN);
  if (status==START)     //Erstmal die empfangene SID auswerten.
    {
      if(scp_check_sid(ostr)==-1)
	{
	  outp.append(ostr);  
	  return false;
	}
      if (status==WAITACK_PW)
	{
	  outp.append(ostr);
	  return true;
	}
    }
  
  if (status==WAITACK_PW)
    {
      if (rx_line[0]!=MSG_ACK)
	{
	  rpclogf.eintrag(slave_call,"Falsches Passwort",RPCLOGMASK_ERROR);
	  return false;
	}
      else
	{
	  status=INIT;
	}
    }
  
  if (status==INIT)
    {
      if (sp_init(max_age * 60) == -1)
	return false;
      init_scp();
      status=TXSYNC;
      synctimer = SYNC_SAMPLE_CNT;
      if (!mode) status=TXPAGE;	
    }
  if (status==TXSYNC)
    {
      // ggf. RTC des Slaves mit unserer Uhr synchronisieren
      if ((curtime - scp.last_sync >= INTERVAL_SYNC ) && (mode))
	{
	  if (scp_sync_slave(ostr, true) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  status=RXSYNC;
	  outp.append(ostr);
	  return true;
	}
      else
	{
	  status=SLOTS;
	  return true;
	}
    }
  
  if (status==RXSYNC)
    {	
      want_sync=false;
      if (synctimer<0) //Wir sind fertig mit synchronisieren ;=))
	{
	  if (rx_line[0] != MSG_ACK) 
	    {
	      rpclogf.eintrag(slave_call,"Zeitsynchronisation fehlgeschlagen",RPCLOGMASK_ERROR);
	      rpclogf.eintrag(slave_call,rx_line,RPCLOGMASK_ERROR);
	      return false;
	    }
	  scp.last_sync = curtime;	
	  synctimer=SYNC_SAMPLE_CNT;
	  status=SLOTS;
	}
      else
	{
	  if (scp_sync_slave(ostr, false) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  else
	    {
	      outp.append(ostr);
	      return true;
	    }
	}
    }
  
  if (status==SLOTS)
    {
      // Wenn die Sendeslots noch nicht gesetzt wurden, muss das jetzt gemacht werden.
      if ((!scp.slots_unlocked) && (mode))
	{
	  if (scp_unlock(ostr) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  else
	    {
	      status=RXSLOTS;
	      outp.append(ostr);
	      return true;
	    }
	}
      status=TXPAGE;
    }
  
  if (status==RXSLOTS)
    {
      if (rx_line[0] != MSG_ACK) 
	{
	  rpclogf.eintrag(slave_call,"Freischalten der Sendeslots fehlgeschlagen",RPCLOGMASK_ERROR);
	  rpclogf.eintrag(slave_call,rx_line,RPCLOGMASK_ERROR);
	  return false;
	}
      scp.slots_unlocked = 1;
      status=TXPAGE;
    }
  
  if (status==RXPAGE)
    {
      if (rx_line[0] == MSG_NUM) 
	{
	  if (scp_ack() == -1)
	    return false;
	  else
	    status=TXPAGE;
	}
      else
	{
	  rpclogf.eintrag(slave_call,"Protokollverstoss!!",RPCLOGMASK_ERROR);
	  rpclogf.eintrag(slave_call,rx_line,RPCLOGMASK_ERROR);
	  return false;
	}
      if (want_sync==true)
	{
	  // Es wird noch auf eine Bestaetigung gewartet, daher duerfen keine neuen
	  // Funkrufe geschickt werden.
	  if (pq_cnt(&scp.pq) > 0)
	    {
	      status=RXPAGE;
	      return true;
	    }
	  
	  want_sync=false;
	  if (scp_sync_slave(ostr, true) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  else
	    {
	      status=RXSYNC;
	      outp.append(ostr);
	      return true;
	    }
	}
    }
  
  if (status==TXPAGE)
    {
      // Es wird noch auf eine Bestaetigung gewartet, daher duerfen keine neuen
      // Funkrufe geschickt werden.
      if (pq_cnt(&scp.pq) > 0)
	{
	  status=RXPAGE;
	  return true;
	}
      
      // ggf. nach neuen Funkrufen gucken und diese an den Slave schicken.
      if (curtime - scp.last_scan >= INTERVAL_SCAN)
	{
	  if (scp_send_pages(ostr) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  else
	    {
	      scp.last_scan = curtime;
	      outp.append(ostr);
	      return true;
	    }
	}
      outp.append(ostr);
    }
  return true;
}

bool rpc_interface::do_process( bool rx_flag, String &outp )
{
  rpclog rpclogf(configuration);

  outp =  "";

  if(slaves.stop_connection(slave_call))
    {
      rpclogf.eintrag(slave_call,"Verbindung wird abgebaut",RPCLOGMASK_CONNECTION);
      return false;
    }

  if (rx_flag)
    {
      last_activity = zeit();

      while (get_line(rx_line) && do_line(rx_line,outp) )  
	//mal sehen was anliegt
	{
	}
    }
  else
    {
      ostringstream ostr;
      // Verbindung steht bereits
      time_t curtime=time(NULL);
      
      // Es wird noch auf eine Bestaetigung gewartet, daher duerfen keine neuen
      // Funkrufe geschickt werden.
      if (pq_cnt(&scp.pq) > 0)
	{
	  if (status==TXPAGE)
	    status=RXPAGE;
	  return true;
	}
      
      if ((curtime - scp.last_sync >= INTERVAL_SYNC) && (mode))
	{
	  if(status==TXPAGE)
	    {
	      if (scp_sync_slave(ostr, true) == -1)
		{
		  outp.append(ostr);
		  return false;
		}
	      else
		{
		  status=RXSYNC;
		  outp.append(ostr);
		  return true;
		}
	    }
	  else	//hm, dann muessen wir ja im RXPAGE sein oder noch nicht soweit
	    want_sync=true;
	}
      else if (curtime - scp.last_scan >= INTERVAL_SCAN && status==TXPAGE)
	{
	  if (scp_send_pages(ostr) == -1)
	    {
	      outp.append(ostr);
	      return false;
	    }
	  else
	    {
	      scp.last_scan = curtime;
	      outp.append(ostr);
	      return true;
	    }
	}
      return true;
    }
  return true;
}

void rpc_interface::init_scp()
{
    slaves.set_status(slave_call,slave::sts_aktiv);
//erstmal initialisieren
    pq_init(&scp.pq);
//Alle alten Funkrufe aus dem Spool-Verzeichnis entfernen.
    sp_age();
    pq_clean(&scp.pq);
    scp.last_sync = 0;
    scp.last_scan = 0;
    scp.seq_tx  = 0;
    scp.seq_ack = 0;
    scp.slots_unlocked = 0;
}


// Wenn ein Master einen SCP-faehigen Funkruf-Slave connected muss dieser den
// SID des Slaves zurueckschicken. Wenn der Slave eine Authentifizierung
// erwartet, werden hinter dem SID sofort die 5 Zahlen fuer das 
// BayCom-Passwort-Verfahren gesendet.
// Im Fehlerfall wird -1 zurueckgegeben, ansonsten 0.
// Erwarten wir ein ACK auf unser Passwort, wird 1 zurueckgegeben.
int rpc_interface::scp_check_sid (ostream &ostr)
{
  rpclog rpclogf(configuration);

	char version[100], prot[100];
	char line[rx_line.slen()+2];
	int items;
	int pwnum[5];
	unsigned i;

	for(i=0; i<rx_line.slen(); i++)  line[i]=rx_line[i];
	line[i]='\0';
// SID in seine Bestandteile zerlegen und in den vorgesehenen Variablen speichern.
	items = sscanf(line, 
	               "[%99[^-]-SCP-%99[^]]] %u %u %u %u %u",
		       version,
		       prot,
		       pwnum,
		       pwnum + 1,
		       pwnum + 2,
		       pwnum + 3,
		       pwnum + 4);

// Es gibt nur zwei moegliche Item-Anzahlen:
// 2  Es wurde nur der SID uebertragen
// 7  SID + BayCom-PW-Zahlen
	if (items != 2 && items != 7) 
	{
	    rpclogf.eintrag(slave_call,"Kann SID nicht decodieren",RPCLOGMASK_ERROR);
	    rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
   	    return -1;
	}

// Pruefen, ob der Slave alle notwendigen Protokollelemente beherrscht.
	if (strspn("#234567", prot) != 7) 
	{
	    rpclogf.eintrag(slave_call,"RPC konnte Kommando nicht dekodieren",RPCLOGMASK_ERROR);
	    rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
	    return -1;
	}
        slaves.set_version(slave_call,String(version));
// Wenn keine Authentifizierung stattfinden muss, sind wir jetzt schon fertig.
	if (items == 2)
	{
	    status=INIT;
    	    return 0;
	}

	// BayCom-PW Authentifizierung
	if (passwort!="")
	{
	    ostr << check_pw(pwnum) << "\r";
	    status=WAITACK_PW;
	    return 1;
	}
	else
	{
	    rpclogf.eintrag(slave_call,"RPC-Passwort nicht definiert.", RPCLOGMASK_ERROR);
	    return -1;
	}
}


// Holt Funkrufe aus dem Spooler und gibt sie an den Slave weiter.
// Im Fehlerfall wird -1 zurueckgegeben, ansonsten 0.
int rpc_interface::scp_send_pages (ostream &ostr)
{
	struct Page *pn;
	char Sequenz[150];
	rpclog rpclogf(configuration);
        bc_add(&scp.pq);
	sp_get(&scp.pq, 10);
	if (pq_cnt(&scp.pq) == 0)
		return 0;

	scp.seq_ack = scp.seq_tx;

// Je nach Funkruf-Typ werden Kommandos an den Slave geschickt.
	pn = scp.pq.first;

	while (pn != NULL) 
	{
		struct Page *p;
		int speed, error;

		p = pn;
		pn = pn->next;

		error = 0;

		switch (p->speed) 
		{
		case 512:
			speed = 0;
			break;
			
		case 1200:
			speed = 1;
			break;
			
		case 2400:
			speed = 2;
			break;
			
		default:
			speed = 1;
		}
		switch (p->type) 
		{
		case P_TYPE_NUMERIC:
    			sprintf(Sequenz,"#%02X 5:%X:%lX:%X:%s\r",
				    scp.seq_tx,
				    speed,
				    p->address,
				    p->func,
				    p->text);
			ostr << Sequenz;
			out_msg++;
    rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
			break;				  
	
		case P_TYPE_RAW:
    			sprintf(Sequenz,"#%02X 7:%X:%X:%s\r",
				    scp.seq_tx,
				    speed,
				    p->frame,
				    p->text);
			ostr << Sequenz;
			out_msg++;
    rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
			break;				 

		case P_TYPE_ALPHA:
    			sprintf(Sequenz,"#%02X 6:%X:%lX:%X:%s\r",
				    scp.seq_tx,
				    speed,
				    p->address,
				    p->func,
				    p->text);
			ostr << Sequenz;
			out_msg++;
    rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
    			break;				  
		
		default:
			pq_dequeue(&scp.pq, p);
			sp_remove(p);
			free(p);
			continue;
		}

		if (error)
			return -1;
		scp.seq_tx = (scp.seq_tx + 1) & 0xff;
	}
    status=RXPAGE;
    return 0;
}


// Wertet die asynchronen Bestaetigungen, die vom Slave empfangen wurden aus.
// Im Fehlerfall wird -1 zurueckgegeben, ansonsten 0.
int rpc_interface::scp_ack ()
{
  rpclog rpclogf(configuration);

	struct Page *p;
	unsigned int seq;
	char msg;
	int retry = 0;
	char line[rx_line.slen()+2];
	unsigned int i=0;
	
	for (i=0; i<rx_line.slen(); i++) line[i]=rx_line[i];
	line[i]='\0';

	if (sscanf(line, "#%02x %c", &seq, &msg) != 2) 
	{
		rpclogf.eintrag(slave_call.str(),"'Failed to interprete ack",RPCLOGMASK_ERROR);
		rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
		return -1;
	}

	if (seq > 0xff) 
	{
		rpclogf.eintrag(slave_call,"Seq number out of range: " + itoS(seq),RPCLOGMASK_ERROR);
		return -1;
	}

// Bis zu dieser Sequenznummer sind alle Funkrufe positiv bestaetigt.
// Sie muessen also aus der Queue und aus dem Spooler entfernt werden.
	while ((p = scp.pq.first) != NULL && unsigned(scp.seq_ack) != seq) 
	{
		pq_dequeue(&scp.pq, p);
		sp_remove(p);
		free(p);
		scp.seq_ack = (scp.seq_ack + 1) & 0xff;
	}

	if (unsigned(scp.seq_ack) != seq) 
	{
		rpclogf.eintrag(slave_call,"Master/Slave Seqenz-synchronisationsfehler!",RPCLOGMASK_ERROR);
		return -1;
	}

// Je nach Antwort muessen noch zusaetzliche Aktionen ausgefuehrt werden.
	switch (msg) 
	{
	case MSG_ACK:
		break;
		
	case MSG_RETRY:
		retry = 1;
		// kein break
		
	case MSG_NACK:
		if ((p = scp.pq.first) == NULL) 
		{
		        rpclogf.eintrag(slave_call,"SCP Protocoll Fehler. Falsche Sequenz!",RPCLOGMASK_ERROR);
			return -1;
		}

		pq_dequeue(&scp.pq, p);

		if (msg == MSG_NACK)
			sp_remove(p);

		free(p);

		scp.seq_ack = (scp.seq_ack + 1) & 0xff;
		break;
	
	default:
	        rpclogf.eintrag(slave_call,"Falsche Antwort vom RPC",RPCLOGMASK_ERROR);
	        rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
		return -1;
	}
// Wenn die PageQueue nun leer ist, koennen wir sofort nachgucken, ob neue 
// Eintrage vorhanden sind. Das wird bewerkstelligt, indem scp->last_scan
// auf 0 gesetzt wird.
// Falls der Slave ein Retry gemeldet hat, warten wir besser
// doch den Scan-Zyklus ab!
	if (!retry && pq_cnt(&scp.pq) == 0)
		scp.last_scan = 0;
	
	return 0;
}


// Synchronisiert die RTC des Slaves mit unserer Systemzeit.
// Die Vorgehensweise dabei ist folgende:
// Es werden SYNC_SAMPLE_CNT Echo-Request-Pakete an den Slave geschickt. 
// Der Slave Antwortet auf jedes Paket mit einem Echo-Reply incl. der 
// Slave-Systemzeit, bei dem das Paket empfangen wurde. Das Paket, das am 
// schnellsten beantwortet wurde wird zur Berechnung des Korrekturfaktors
// der Slave-RTC benutzt.
// Eine statistische Auswertung aller Messwerte kann an dieser Stelle nicht 
// benutzt werden, da die Messwerte zufaelligen Charakter haben.
// Im Fehlerfall wird -1 zurueckgegeben, ansonsten 0.
int rpc_interface::scp_sync_slave (ostream &ostr, bool TXRX)
{
    char Sequenz[150];
    static systime_t min_rtt, adj;
    char line[rx_line.slen()+2];
    unsigned i=0;
    unsigned int resp_tx, resp_slave;
    systime_t time_slave=0, rtt;
    rpclog rpclogf(configuration);

    if ( synctimer == SYNC_SAMPLE_CNT )
      {
	min_rtt = 0xffff;
	adj = 0;
      }

    if (TXRX)	//TX
      {		
	time_tx = systime();
	sprintf(Sequenz,"2:%04X\r", time_tx);
	ostr << Sequenz;
	rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
	synctimer--;
	ack=false;
	return 0;
      }
    else //RX
      {
	for (i=0; i<rx_line.slen(); i++) line[i]=rx_line[i];
	line[i]='\0';
	if (!ack)
	  {
	    if ( sscanf(line, "2:%x:%x", &resp_tx, &resp_slave) != 2 ||
    		 resp_slave > 0xffff ||
		 resp_tx > 0xffff)
	      {
	        rpclogf.eintrag(slave_call,"Falsche Antwort auf 2:",RPCLOGMASK_ERROR);
    		rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
		return -1;
	      }
	    
	    if (resp_tx != time_tx) 
	      {
	        rpclogf.eintrag(slave_call,"Synchronistaion verloren!" ,RPCLOGMASK_ERROR);
    		return -1;
	      }
    	    time_slave = resp_slave;
	    ack=true;
	    // Wenn niedrigste RTT, dann RTC-Korrekturwert berechnen und speichern.
    	    rtt = time_rx - time_tx;    
	    if (rtt < min_rtt) 
	      {
		min_rtt = rtt;
		adj = (time_tx + rtt / 2) - time_slave;
	      }
	    return 0;
	  }
	else
	  {
	    if (*line != MSG_ACK) 
	      {
	        rpclogf.eintrag(slave_call,"Keine Antwort auf 2",RPCLOGMASK_ERROR);
    		rpclogf.eintrag(slave_call,line,RPCLOGMASK_ERROR);
		return -1;
	      }
	    // RTC des Slaves anpassen, falls das notwendig ist.
	    if(synctimer==0)
	      {
		sprintf(Sequenz,"%04X", adj);
		slaves.set_diff(slave_call,adj);
		sprintf(Sequenz,"3:+%04X\r", adj);
		ostr << Sequenz;
		rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
		synctimer --;
		return 0;	
	      }
    	    time_tx = systime();
	    sprintf(Sequenz,"2:%04X\r", time_tx);
	    ostr << Sequenz;
	    rpclogf.eintrag(slave_call,"> " + String(Sequenz),RPCLOGMASK_DATEN);
	    synctimer--;
	    ack=false;
	    return 0;
	  }
      }
}


// Schaltet die Sendeslots des Slaves frei
int rpc_interface::scp_unlock (ostream &ostr)
{

	String Logpuffer;
	char line[rx_line.slen()+2];
	unsigned i=0;
	rpclog rpclogf(configuration);
	
	for (i=0; i<rx_line.slen(); i++) line[i]=rx_line[i];
	line[i]='\0';

// Dem Slave die Sende-Slots mitteilen
        ostr << "4:" << slots_str << "\r";
	rpclogf.eintrag(slave_call,"> 4:" + String(slots_str),RPCLOGMASK_DATEN);
	status=RXSLOTS;
        return 0;
}


// Setzt das Spool-Verzeichnis, in dem die Funkruf-Dateien erwartet werden.
// Im Fehlerfall wird -1 zurueckgegeben, ansonsten 0.
int rpc_interface::sp_init (int maxage)
{
	rpclog rpclogf(configuration);

  DIR *dir;
  if (!(dir = opendir(strtochar(spool_dir))))
    {
      rpclogf.eintrag(slave_call,"Kann nicht aus dem Spool-Dir lesen " + spool_dir,RPCLOGMASK_ERROR);
      return -1;
    }
  else
    {
      closedir(dir);
      return 0;
    }
}


// Entfernt das NewLine am Ende des Strings <str>
void rpc_interface::sp_chomp (char *str)
{
	int len = strlen(str);
	for (int i=0; i<len; i++)
	{
	    if(str[i]=='\n')
	    {
		str[i]='\0';
		break;
	    }
	    if(iscntrl(str[i]))
		str[i]=' ';
	}
	if (len>83) str[83]='\0';	// Funkruf auf 82 Zeichen begrenzen
	// 82 Zeichen sind 80 Zeichen Text effektiv
}


// Liest den Funkruf aus dem File <file> ein und speichert ihn in der PageQueue <pq>. 
// Es wird die Anzahl der hinzugefuegten Funkrufe zurueckgegeben.
int rpc_interface::sp_addpage (struct PageQueue *pq, char *file)
{
	rpclog rpclogf(configuration);

	char path[PATH_MAX + 1];
	struct Page *p;
	FILE *f;
	struct stat s;

	snprintf(path, sizeof(path), "%s%s", strtochar(spool_dir), file);
	if (stat(path, &s) == -1) 
	{
    		rpclogf.eintrag(slave_call,"Failed to stat " + String(path),RPCLOGMASK_ERROR);
		return -1;
	}

	if (time(NULL) - s.st_mtime >= max_age) 
	{
		if (unlink(path)!=0)
		    rpclogf.eintrag(slave_call,"Kann Funkruf nicht loeschen: " + String(path),RPCLOGMASK_ERROR);
		return 0;
	}

	if ( !(p = (struct Page*) malloc(sizeof(struct Page))) ) 
	{
    		rpclogf.eintrag(slave_call,"Failed to malloc new page",RPCLOGMASK_ERROR);
		return -1;
	}

	memset(p, 0, sizeof(*p));

	if ( !(f = fopen(path, "r")) ) 
	{
    		rpclogf.eintrag(slave_call,"Failed to open page file " + String(path),RPCLOGMASK_ERROR);
		free(p);
		return -1;
	}

	if (fscanf(f, "%lu.%u\n", &p->address, &p->func) != 2 || fgets(p->text, sizeof(p->text), f) == NULL)
	{
		fclose(f);
		free(p);
		return 0;
	}
	fclose(f);
	sp_chomp(p->text);

	strcpy(p->file, file);
	
	p->type  = P_TYPE_ALPHA;
	p->speed = 1200;
	p->func &= 3;
	pq_enqueue(pq, p);
	return 1;
}


// Durchsucht das Spool-Verzeichnis nach neuen Funkrufen und 
// fuegt sie in die Page-Queue ein. 
int rpc_interface::sp_get (struct PageQueue *pq, int cnt)
{
	rpclog rpclogf(configuration);

	DIR *dir;
	struct dirent *de;
	int left=cnt;

	if ( !(dir = opendir(strtochar(spool_dir))) ) 
	{
    		rpclogf.eintrag(slave_call,"Kann Spool-Dir nicht oeffnen: " + spool_dir,RPCLOGMASK_ERROR);
		return -1;
	}
	while (left > 0 && (de = readdir(dir)) != NULL) 
	{
		if (!strstr(de->d_name, "FUNKRUF"))
			continue;
		if (sp_addpage(pq, de->d_name) > 0)
			left--;
	}
	closedir(dir);
	return (cnt - left);
}


// Entfernt alle alten Funkrufe aus dem Spool-Verzeichnis.
void rpc_interface::sp_age ()
{
	rpclog rpclogf(configuration);

	DIR *dir;
	struct dirent *de;
	time_t cur_time;

	if ( !(dir = opendir(strtochar(spool_dir))) ) 
	{
    		rpclogf.eintrag(slave_call,"Kann Spool-Dir nicht oeffnen: " + spool_dir,RPCLOGMASK_ERROR);
		return;
	}

	cur_time = time(NULL);

	while ((de = readdir(dir)) != NULL) 
	{
		struct stat s;
		char path[PATH_MAX + 1];
		
		if (!strstr(de->d_name, "FUNKRUF"))
			continue;

		snprintf(path, sizeof(path), "%s/%s", strtochar(spool_dir), de->d_name);

		if (stat(path, &s) == -1) 
		{
		  rpclogf.eintrag(slave_call,"Failed to stat " + String(path),RPCLOGMASK_ERROR);
			continue;
		}

		if (cur_time - s.st_mtime >= max_age)
		  if (unlink(path)!=0)
		    rpclogf.eintrag(slave_call,"Kann Funkruf nicht loeschen: " + String(path),RPCLOGMASK_ERROR);
	}
	closedir(dir);
}


// Entfernt die Datei, die zum Funkruf <p> gehoert aus dem Spool-Verzeichnis.
void rpc_interface::sp_remove (struct Page *p)
{
	rpclog rpclogf(configuration);

	char path[PATH_MAX + 1];

	if (!*p->file)
		return;

	snprintf(path, sizeof(path), "%s/%s", strtochar(spool_dir), p->file);
	if (unlink(path)!=0)
	{
	  rpclogf.eintrag(slave_call,"Kann Funkruf nicht loeschen: " + String(path),RPCLOGMASK_ERROR);
	}
}


// Initialisiert die Page-Queue <pq>
void rpc_interface::pq_init (struct PageQueue *pq)
{
	memset(pq, 0, sizeof(*pq));
}


// Fuegt den Funkruf <p> am Ende der PageQueue <pq> ein.
void rpc_interface::pq_enqueue (struct PageQueue *pq, struct Page *p)
{
	if (pq->last) 
	{
		p->next = NULL;
		p->prev = pq->last;

		pq->last->next = p;
		pq->last = p;
	} 
	else 
	{
		p->prev = NULL;
		p->next = NULL;

		pq->first = p;
		pq->last = p;
	}
	pq->pcnt++;
}


// Entfernt den Funkruf <p> aus der Queue <pq>. Die
// Funkruf-Instanz wird dabei nicht geloescht.
void rpc_interface::pq_dequeue (struct PageQueue *pq, struct Page *p)
{
	if (p->prev)
		p->prev->next = p->next;
	else
		pq->first = pq->first->next;

	if (p->next)
		p->next->prev = p->prev;
	else
		pq->last = pq->last->prev;

	pq->pcnt--;
}


// Entfernt alle Funkrufe aus der Queue <pq>
void rpc_interface::pq_clean (struct PageQueue *pq)
{
	struct Page *p;
	while ((p = pq->first) != NULL) 
	{
		pq_dequeue(pq, p);
		free(p);
	}
	pq_init(pq);
}


// Gibt die Anzahl der Funkrufe zurueck, die in <pq> enthalten sind.
unsigned long rpc_interface::pq_cnt (struct PageQueue *pq)
{
	return pq->pcnt;
}


// Gibt die Systemzeit, wie sie auf dem RPC verwendet wird zurueck.
systime_t rpc_interface::systime ()
{
#if defined(__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 2)
	struct timeb tb;
	ftime(&tb);
	return (systime_t)((tb.time * 10 +  tb.millitm / 100) & 0xffff);
	
#else
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return (systime_t)((tv.tv_sec * 10 + tv.tv_usec / 100000) & 0xffff);
#endif	
}


// Generiert die Skyper-Time-Bake und fuegt sie der PageQueue <pq> hinzu. 
// Die Zeitangabe ist dabei in UTC.
void rpc_interface::bc_add_time (struct PageQueue *pq)
{
	struct Page *p;
	struct tm *tm;
	time_t curtime;
	
	if ( !(p = (struct Page*) malloc(sizeof(struct Page))) ) 
		return;

	memset(p, 0, sizeof(*p));

	curtime = time(NULL);
	tm = gmtime(&curtime);

	p->type    = P_TYPE_NUMERIC;
	p->speed   = 1200;
	p->address = 2504;
	p->func    = 0;

// Laut Infos von DB6KH und DG1KWA darf	sich der Sekundenwert nur in dem
// Interval [0..29] bewegen.
// Das muss noch verifiziert werden... (TODO)
	snprintf(p->text, sizeof(p->text),
		 "%02u%02u%02u   %02u%02u%02u",
		 tm->tm_hour,
		 tm->tm_min,
		 (tm->tm_sec >= 30) ? 29 : tm->tm_sec,
		 tm->tm_mday,
		 tm->tm_mon + 1,
		 tm->tm_year % 100);

	pq_enqueue(pq, p);
}


// Fuegt der PageQueue einen Zeit-Baken-Funkruf hinzu, wenn das erforderlich ist.
void rpc_interface::bc_add (struct PageQueue *pq)
{
	time_t curtime;
	curtime = time(NULL);

	if (curtime - bc_last_gen < bc_interval)
		return;

	bc_add_time(pq);
	bc_last_gen = curtime;
}

// Passwort-Antwort zusammenbasteln. Antwort ist immer 80 Zeichen lang.
// Erst erzeugen wir einen String zwischen 0 und 75 Zeichen, dann unsere
// richtigen 5 Zeichen und dann fuellen wir bis 80 Zeichen auf.
String rpc_interface::check_pw(int frage[])
{
  String Antwort="";
  int a=rand() % (75);
	rpclog rpclogf(configuration);
	  
  for (int i=0; i<=a; i++)
      Antwort=Antwort + passwort[rand() % (passwort.slen())];
  for (int i=0; i<5; i++)
      Antwort=Antwort + passwort[frage[i]-1];
  for (int i=a+5; i<79; i++)
      Antwort=Antwort + passwort[rand() % (passwort.slen())];
    rpclogf.eintrag( slave_call,"> " + String(Antwort),RPCLOGMASK_DATEN);

  return Antwort;
}
