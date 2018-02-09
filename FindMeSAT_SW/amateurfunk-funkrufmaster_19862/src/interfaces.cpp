/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "interfaces.h"

extern config_file configuration;

extern callsign G_mycall;

void interfaces::cut_blanks( istream & strm )
{
  char ch;
  
  while (strm.get(ch) && ((ch == ' ') || ch == (char) 255)) 
    {
      if (ch == (char) 255)
	{
#ifdef _DEBUG_ELOGS_ 
	  cerr << "IAC erkannt" << endl;
#endif
	  strm.get(ch);
#ifdef _DEBUG_ELOGS_ 
	  cerr << "OP-Code : " << (int) ch << endl;
#endif
	  if (ch >= (char) 250 && ch <= (char) 254)
	    {
	      strm.get(ch);
#ifdef _DEBUG_ELOGS_ 
	      cerr << "Option : " << (int) ch;
#endif
	    }
	}
    }
  strm.putback(ch);
}

String interfaces::cut_blanks( const String &cmd )
{
  for (unsigned int i = 0; i < cmd.slen(); i++ )
    if (cmd[i] != ' ' && cmd.slen()-i > 0)
      return cmd.copy(i,cmd.slen()-i);
  
  return "";
}

void interfaces::cut_blank( istream & strm ) // 1 Blank
{
  char ch;

  if (strm.get(ch) && ((ch == ' ') || ch == (char) 255)) 
    {
      if (ch == (char) 255)
	{
#ifdef _DEBUG_ELOGS_ 
	  cerr << "IAC erkannt" << endl;
#endif
	  strm.get(ch);
#ifdef _DEBUG_ELOGS_ 
	  cerr << "OP-Code : " << (int) ch << endl;
#endif
	  if (ch >= (char) 250 && ch <= (char) 254)
	    {
	      strm.get(ch);
#ifdef _DEBUG_ELOGS_ 
	      cerr << "Option : " << (int) ch;
#endif
	    }
	}
      strm.get(ch);
    }
  strm.putback(ch);
}

void interfaces::cut_line(String &s )
{
  try
    {
      int l = s.slen();
      for (int i = 0 ; i < l ; i++)
	if ((s[i] == (char) 13) || (s[i] == (char) 10))
	  s[i] = ' ';
      s.kuerze();  
    }
  catch( Error_String_index_out_of_range )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "Indexverletzung in 'cut_line' " << endl;
#endif
    }
}

bool interfaces::get_line( String &inp )
{
  // Jetzt Groesse des Eingangs-Puffers ermitteln
  int l = input_puffer.slen();
  //cerr << "get_line(), Inputbuffer : " << l << endl;
  if (l > 0)
    {
      // Es befinden sich Zeichen im Eingangspuffer
      // Alle zeichen durchlaufen, aber nur solange keine Zeile
      // extrahiert wurde
      for (unsigned int i = 0; i < (unsigned int) l; i++)
	if ( input_puffer[i] == ende)
	  {
	    //cerr << "eol gefungen" << endl;
	    // Eine Zeilen-Ende-Zeichen wurde gefunden. Damit wurde
	    // eine komplette Zeile gefunden!
	    // Zeile jetzt extrahieren
	    inp = input_puffer.copy(0,i+1);
	    cut_line(inp);
	    // Rest des Input-Puffers ohne Zeilen-Ende-Zeichen wieder
	    // Eingangspuffer schreiben in
	    if (l-i-1 > 0)
	      input_puffer = input_puffer.copy(i+1,l-i-1);
	    else
	      input_puffer = "";
	    //cerr << "Input : ." << inp << "." << endl;
	    //cerr << "Rest  : " << endl << input_puffer << endl << endl;
	    return true;
	  }
    }
  return false;
}


/* Fuer den Autoconnect werden neben der Instanz der Connect-Pfad-Klasse
   c_path zwei boolsche Variablen benutzt:

   1. path_finished ist so lange false, wie der Zieldigi noch nicht
      erreicht wurde.

   2. wait_connect zeigt an, dass aktuell auf die Connect-Meldung
      eines zwischen-Ziels oder des Zieldigis gewartet wird. In der
      Instanz der Klasse callsign "wait_digi" ist dann der Digi 
      eingetragen, auf dessen connect-Meldung gewartet wird.

   Die Integer-Variable next_digi zeigt an, welcher Digi im Connect-Pfad
   als naechster connectet werden muss.

*/

/* set_connect_path initialisiert den Auto-Connecter. Der Connect-Pfad
   wird initialisiert und die beiden boolschen Variablen path_finished
   und wait_connect auf false gesetzt, da das Ziel noch nicht erreicht ist
   und auch keine connect_Meldung abgewartet wird.

   Der Pfad muss mit dem ersten Digi begonnen werden. Daher next_digi = 1

*/
void interfaces::set_connect_path( const connect_string & cs)
{
  c_path = cs;
  path_finished = false;
  wait_connect = false;
  connect_prompt = false;
  next_digi = 1;
  gegenstation = c_path.get_digi(0);
}

/* connect_path ueberprueft die Eingabe, ob ein Digi erreicht wurde und
   fuehrt dann den naechsten connect aus.

*/
void interfaces::connect_path( String inp, String &outp )
{
  syslog logf(configuration);
  outp = "";
  String line;

  if (!path_finished)  // Wenn das Ziel erreicht wurde, ist nichts mehr zu tun
    {
      if (wait_connect) // Wird auf eine connect-Meldung gewartet?
	{
	  inp.upcase();     // Eingabe in Grossbuchstaben wandeln 

	  // Verstehen, wozu das gut ist, tu ich jetzt nicht, aber
	  // ich hab's mal uebernommen...

          while (get_line(line));	// ganzen Puffer leeren

	  if (inp.in(String("FAILURE")))
	    {
	      connect_failed = true;
	      wait_connect = false;
	    }
	  else if (inp.in(String("RECONNECTED")))
	    {
	      connect_failed = true;
	      wait_connect = false;
	    }
	  else if (inp.in(String("LOOP DETECTED")))
	    {
	      connect_failed = true;
	      wait_connect = false;
	    }
	  else if (inp.in(String("BUSY FROM")))
	    {
	      connect_failed = true;
	      wait_connect = false;
	    }
	  else if (inp.in(String("CONNECTED TO")))     // connect_meldung ?
	    {
	      if (inp.in(wait_digi.call()))             // Der erwartete Digi ?
		if (next_digi < c_path.get_digi_anz())  // Muessen noch weitere Digis
		  {                                     // connectet werden
		    // Ja: Ein Connect-Befehl fuer den naechsten Digi wird erzeugt und
		    // ausgegeben. wait_connect wird auf true gesetzt und der naechste
		    // Digi in wait_digi eingetragen
		    outp = "c ";
		    outp.append(c_path.get_digi(next_digi).call());
		    outp.append( (char) 13 );

		    wait_connect = true;
		    gegenstation = wait_digi;
		    wait_digi = c_path.get_digi(next_digi++);
		  }
		else
		  {
		    // Nein, dann ist das Ziel erreicht. path_finished wird auf
		    // true gesetzt und wait_connect auf false.
		    path_finished = true;
		    wait_connect = false;
		    gegenstation = wait_digi;
		  }
	      else
		{
		  // Da eine Connect_meldung mit dem falschen Digi-Rufzeichen
		  // Eingetroffen ist, muss irgend etwas falsch gelaufen sein. Der
		  // Verbindungsaufbau ist gescheitert, connect_failed wird auf
		  // true gesetzt.
		  connect_failed = true;
		  wait_connect = false;
		  // Da dieses ereignis sehr dubios ist, gibt's auch einen Eintrag
		  // ins Logfile
		  logf.eintrag("Fehlerhafte Connectmeldung beim Verbindungsaufbau",LOGMASK_PRGRMERR);
		}
	    }
	}
      else 
	// Es wird auf keine connect-Meldung gewartet. Sofern noch ein Digi 
	// zu connecten ist, kann dies sofort geschehen
	if (next_digi > c_path.get_digi_anz())
	  {
	    outp = "C ";
	    outp.append(c_path.get_digi(next_digi).call());
	    outp.append( (char) 13 );
	    wait_connect = true;
	    wait_digi = c_path.get_digi(next_digi++);
	  }
	else
	  {
	    path_finished = true;
	    wait_connect = false;
	  } 
    }
}

/* Beim ersten connect, muss keine Eingabe ueberprueft werden. Sofern 
   ueberhaupt ein Digipeater connected werden muss, kann dies gleich
   erfolgen.
*/
void interfaces::first_connect(String &outp )
{
  outp = "";

  if (!wait_connect && !path_finished)       // Ueberpruefen, ob nicht doch auf
  {                                          // ein connect gewartet wird oder
				             // das Ziel bereits erreicht ist 
    if (next_digi < c_path.get_digi_anz())  // Muss noch ein Digi connectet werden?
      {
	// Connect-Befehl erzeugen und ausgeben.
	// Dann wait_conect und wait_digi setzen
	outp = "C ";
	outp.append(c_path.get_digi(next_digi).call());
	outp.append( (char) 13 );
	wait_connect = true;
	wait_digi = c_path.get_digi(next_digi++); 
      }
    else
      {
	// Es muessen keine Digis mehr connectet werden
	// Das Ziel ist erreicht
	path_finished = true;
	wait_connect = false;
      }
   }
}


interfaces::interfaces( String &outp, bool ax25_flag ) : logf(configuration), ulogf(configuration)
{
  // im Konstruktor werden Zeiger auf den Ausgabestream, das
  // Konfigurationsfile und das Logfile geseichert.
  outp = "";
  connect_failed=false;
  gegenstation = callsign();
  login = zeit();
  in_msg = 0;
  out_msg = 0;
  thread_id = -1;
  if (ax25_flag)
    {
      //cerr << "AX25-Verbindung erkannt." << endl;
      cr = (char) 13;
      ende = (char) 13;
    }
  else
    {
      //cerr << "IP-Verbindung erkannt." << endl;
      cr = '\n';
      ende = '\n';
    }
}

// Der Virtuelle Destruktor ist leer
interfaces::~interfaces( void )
{
  // Um die eintragung von FlexNet-Laufzeitmessungen ins benutzerlog zu 
  // vermeiden, gleichzeitig jedoch RPC und Clusterverbindungen mit
  // Stationen mit gelichem Rufzeichen ins Log zu schreiben, werden nur
  // Verbindungen zu Stationen mit anderem Rufzeichen oder mit 
  // anderem Verbindungstyp als Benutzerverbindung ins Log geschrieben.
  if (interface_id != 'U' || !samecall(gegenstation,G_mycall))
    ulogf.eintrag(login,zeit(),gegenstation,in_b,out_b,connection_id,interface_id,in_msg,out_msg);
}

// Die Methode do_process ist virtuell und muss in den jeweiligen
// abgeleiteten Klassen implementiert werden.

bool interfaces::do_process( bool rx_flag, String &outp )
{
  outp = "";
  return false;
}

bool interfaces::process( const String &inp, String &outp )
{
  input_puffer.append(inp);
  outp = "";
  last_activity = zeit();

  // Solange noch nicht der Zieldigi erreicht wurde, wird weiter
  // connected
  if (!path_finished)
    // Wenn connect_failed gesetzt wurde, dann ist der connect gescheitert,
    // der Prozess ist beendet (Rueckgabewert true)
    if (connect_failed)
      return false;
    else if (inp.slen() > 0)
      {
	String line;
	if (get_line(line))
	  {
	    // Ansonsten wird der Connect fortgefuehrt
	    connect_path(inp,outp);
	    return true;
	  }
	else
	  return true;
      }
    else
      {
	if ( zeit() - last_activity > DEFAULT_CONNECTION_SETUP_TIMEOUT )
	  return false;
	else
	  return true;
      }
  else
    return do_process(inp.slen() > 0,outp);
}


callsign interfaces::get_connected_digi( void )
{
  return gegenstation;
}

void interfaces::set_io( char cid, int i, int o)
{
  in_b = i;
  out_b = o;

  if (in_b < 0) 
    in_b = 0;
  if (out_b < 0)
    out_b = 0;

 connection_id = cid;
}
