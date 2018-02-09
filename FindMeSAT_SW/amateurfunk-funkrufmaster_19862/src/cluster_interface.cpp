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
 * Jens Schoon, DH6BB	          email : dh6bb@darc.de	                    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *  		                                			    *
 ****************************************************************************/

#include "cluster_interface.h"

#include <stdlib.h>
#include <vector.h>

#include "globdef.h"
#include "board.h"
#include "spoolfiles.h"
#include "user_interface.h"
#include "cluster.h"
#include "export.h"

extern callsign G_mycall;
extern config_file configuration;
extern spoolfiles spool;
extern cluster_control cluster_cntl;

/* dx_line untersucht eine Zeile die vom DX-Cluster empfangen wurde. 
   Handelt es sich um eine DX-Meldung, werden die Daten daraus 
   extrahiert und ein entsprechender Funkruf generiert.
*/
void cluster_interface::dx_line( String line )
{
  vector<String> words;
  String dx_line;
  unsigned int k = 0;
  unsigned int l = 0;

  // Als erstes wird die Clustermeldung ueber die Export-Funktion 
  // ausgegeben.
  try
    {
      export_sys exp;
      exp.write_dx(line);
    }
  catch( Error_could_not_initialize_exportsystem )
    {
      // Hat nicht geklappt...
    }
  // Zunaechst wird die Zeile nach Leerzeichen untersucht und
  // in die Bestandteile zerlegt.
  for (unsigned int i = 0; i < line.slen() ; i++)
    {
      if (line[i] == ' ')
	{
	  // Ist ein Leerzeichen gefunden worden, wird der vorherige 
	  // Textabschnitt in den String word[] geschrieben
	  words.push_back(line.copy(l,i-l));
	  k++;
	  // Alle Leerzeichen vor dem naechsten Textabschnitt entfernen
	  while (i < line.slen() && line[i] == ' ') 
	    i++;
	  l = i;
	}
    }
  dx_line=line;
  
  if (dx_typ != dx_raw)
    {
      // 0  1  2        3      4     5    6   7          N
      // DX de DH6BB:   1234.5 DK0WO Test zum testen     12:34Z
      l=5;
      dx_line="";
      dx_line.append(words[4]); dx_line.append(" "); dx_line.append(words[3]);
      if (dx_line.slen() <20) do {dx_line.append(" ");} while (dx_line.slen() <20);
      while (l <= k-1) { dx_line.append(words[l]); dx_line.append(" "); l++;}
      if (dx_line.slen() <(60)) do {dx_line.append(" ");} while (dx_line.slen() <(60));
      dx_line.append(words[0]); dx_line.append(" ");
      dx_line.append(words[1]); dx_line.append(" ");
      dx_line.append(words[2]); dx_line.append(" ");
      dx_line.append(line.copy(70,5));
    }
  
  
  // Frequenz in eine Integerzahl wandeln und anhand der
  // Groesse die richtige Skyper-Rubrik auswaehlen.
  int qrg = atoi(strtochar(words[3]));
  String rub;
  if (qrg < 50000)
    {  // nach CW suchen
      if(( qrg >= 1800 && qrg < 1838) || 	
	 (qrg >= 3500 && qrg < 3580) ||
	 (qrg >= 7000 && qrg < 7035) ||
	 (qrg >= 10100 && qrg < 10140) ||
	 (qrg >= 14000 && qrg < 14070) ||
	 (qrg >= 18068 && qrg < 18100) ||
	 (qrg >= 21000 && qrg < 21080) ||
	 (qrg >= 24890 && qrg < 24920) ||
	 (qrg >= 28000 && qrg < 28050))
	{
	  rub = RUB_DXKWCW;
	}
      else
	{
	  rub = RUB_DXKW;
	}
    }
  
  else if (qrg < 60000)
    {
      rub = RUB_DX50;
    }
  else
    {
      // nach Satellitenfrequenzen suchen
      if (( qrg >=   145800 && qrg <=   146000) ||
	  ( qrg >=   435000 && qrg <=   438000) ||
	  ( qrg >=  1260000 && qrg <=  1270000) ||
	  ( qrg >=  2400000 && qrg <=  2450000) ||
	  ( qrg >=  5650000 && qrg <=  5668000) ||
	  ( qrg >=  5830000 && qrg <=  5850000) ||
	  ( qrg >= 10450000 && qrg <= 10500000) ||
	  ( qrg >= 24000000 && qrg <= 24048000))
	{
	  rub = RUB_SAT;
	}
      else if ( qrg >= 144360 && qrg <= 144380 )
	{
	  rub = RUB_WSJT;
	}
      else
	{
	  rub = RUB_DXVUSHF;
	}
    }
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht 
      // abschicken
      board brd(rub,configuration);
      int board = brd.get_brd_id();
      int slot = brd.get_slot();
      String msg = dx_line;
      brd.set_msg(msg,slot,ds);

      // Nachricht ins Spoolverzeichnis schreiben
      spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,ds,96);
      in_msg++;
    }
  // Moegliche Fehler-Exceptions abfangen
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("DX-Cluster Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, DX-Cluster Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}


void cluster_interface::gpage_Aurora(String Aurora_Text)
{
  String output;
  String myCall=G_mycall.call();
  String input_zeile = "gpage Aurora ";
  input_zeile.append(Aurora_Text);
  user usr(myCall,false);      
  user_interface interface(output,usr,false,baken);
  interface.process(input_zeile,output);
  interface.set_io(0,0,'X');
}




// WWV - Meldungen zurechtbasteln
void cluster_interface::dx_wx_line( String line )
{
  String rub = RUB_DXWX;  
  vector<String> words;
  String Aurora_Call="DK0WCY";
  String Aurora_Text="Achtung! "+ Aurora_Call;
  unsigned int k = 0;
  unsigned int l = 0;

  // Als erstes wird die Clustermeldung ueber die Export-Funktion 
  // ausgegeben.
  try
    {
      export_sys exp;
      exp.write_condx(line);
    }
  catch( Error_could_not_initialize_exportsystem )
    {
      // Hat nicht geklappt...
    }

  //Hier basteln wir uns unsere Aurora-Warnmeldung an die Gruppe "AURORA"
  // Zunaechst wird die Zeile nach Leerzeichen untersucht und
  // in die Bestandteile zerlegt.
  for (unsigned int i = 0; i < line.slen() ; i++)
  {
      if (line[i] == ' ')
      {
	  // Ist ein Leerzeichen gefunden worden, wird der vorherige 
	  // Textabschnitt in den String word[] geschrieben
	  words.push_back(line.copy(l,i-l));
	  k++;
	  // Alle Leerzeichen vor dem naechsten Textabschnitt entfernen
	  while (i < line.slen() && line[i] == ' ') 
	    i++;
	  l = i;
      }
  }
  words.push_back( line.copy(l,line.slen()-l) );
  if (k >= 12)
    {
      words[k].lowcase();

      if(samecall(words[2],Aurora_Call) && words[k]=="au=aurora")
        {
          //Aurora-Warnung ausgeben!
          Aurora_Text.append(" meldet Aurora!!!!");
          gpage_Aurora(Aurora_Text);
        }
      else if(samecall(words[2],Aurora_Call) && words[k]=="au=strong")
        {
          //Strong Aurora-Warnung ausgeben!
          Aurora_Text.append(" meldet Strong-Aurora!!!!");
          gpage_Aurora(Aurora_Text);
        }
      else if(samecall(words[2],Aurora_Call) && words[k]=="au=no")
        {
          //No Aurora-Warnung!
        }
      else
        {}
    }
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht 
      // abschicken
      board brd(rub,configuration);
      int board = brd.get_brd_id();
      int slot = brd.get_slot();
      String msg = line;
      brd.set_msg(msg,slot,ds);

      // Nachricht ins Spoolverzeichnis schreiben
      spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,ds,96);
      in_msg++;
    }
  // Moegliche Fehler-Exceptions abfangen
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("DX-Cluster Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, DX-Cluster Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}

// Im Konstruktor werden alle benoetigten Variablen initialisiert und der erste
// Connect-Schritt zum Cluster durchgefuehrt.
cluster_interface::cluster_interface(String &outp, bool ax_flag, dx_cluster_typ typ, const connect_string &con_pf) : interfaces(outp,ax_flag)
{
  outp = "";
  activ = false;
  dx_typ = typ;
  ds = get_default_destin();
  set_connect_path(con_pf);
  first_connect(outp);
  interface_id = 'D';
  last_activity = zeit();
}

// Der Destruktor bleibt leer
cluster_interface::~cluster_interface( void )
{
  cluster_cntl.getrennt();
}

bool cluster_interface::do_process( bool rx_flag, String &outp )
{
  if (!cluster_cntl.enabled())
    return false;

  if (rx_flag)

    {
      last_activity = zeit();
      
      if (!activ)
	{
	  activ = true;
	  cluster_cntl.established();
	}
      // Eine Zeile vom Cluster wird eingelesen
      String line;
      while (get_line(line))
	{
#ifdef _DEBUG_ELOGS_
          cerr << "zeile : " << line << endl;
#endif	  
	  // Steht in der Zeile "reconnected" ? Dann wird der Prozess beendet 
	  if (line.in("*** reconnected") || line.in("Reconnected to"))
	    return false;
	  else
	    {
	      // Beginnt die Zeile mit "DX de"? Dann handelt es sich um eine
	      // DX-Meldung, die analysiert werden muss
	      try
		{
		  if (line.copy(0,5) == String("DX de"))
		    dx_line(line);
		  else if (line.copy(0,3) == String("WWV"))
		    dx_wx_line(line);
		  else if (line.copy(0,3) == String("WCY"))
		    dx_wx_line(line);
		}
	      catch( Error_String_index_out_of_range )
		{
		  // vermutlich war die Zeile leer...
		}	    
	    }
	}
    }
  else
    if (activ && zeit() - last_activity > DEFAULT_CLUSTER_TIMEOUT )
      return false;

  return true;
}

