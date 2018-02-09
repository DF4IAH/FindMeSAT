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
 *                                                                          *
 ****************************************************************************/

#include "fwd_interface.h"

#include <sys/types.h>
#include <dirent.h>
#include <fstream.h>

#include "board.h"
#include "fwd_router.h"
#include "config.h"

extern fwd_router router;
extern config_file configuration;

void fwd_interface::init( bool ar )
{
  interface_id = 'F';
  spool_priv = 0;
  spool_bul = 0;
  spool_dest = 0;
  spool_sonst = 0;
  tx_queue.clear();
  //  rx_queue.clear();
  kommunikation_laeuft = false;
  verbindung_trennen = false;
  letzte_aktivitaet = zeit();
  t_w = DEFAULT_START_T_W;
  n_max = DEFAULT_START_N_MAX;
  fwd_timeout = DEFAULT_FWD_TIMEOUT;
  unack_max = DEFAULT_MAX_UNACKED;
  unack = 0;
  fehler_zaehler = 0;
  max_fehler_zaehler = DEFAULT_MAX_FEHLER_ZAEHLER;
  fehler = false;
  autorouter_enable = ar;
  eigene_optionen = protokoll_optionen();
  if (!autorouter_enable)
    eigene_optionen = eigene_optionen - protokoll_optionen(OPTION_ROUTINGAUSTAUSCH);
}

void fwd_interface::read_tx_queue( void )
{
  DIR *fwd_dir;
  unsigned int count_delete = 0;

  fwd_dir = opendir(strtochar(fwddirname));
  struct dirent *eintrag;

  if (fwd_dir != NULL)
    {
      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Verzeichnis geoeffnet.",FWDLOGMASK_IFDEBUG);
      tx_queue.clear();
      /* 
	 An dieser Stelle wird die TX-Queue geloescht. Die einzige
	 Nachricht die sich zu dieser Zeit in der Queue befunden haben 
	 kann, ist die Protokolleigenschaftsnachricht. 

	 Wenn diese Nachricht noch nicht bestaetigt worden war, wuerde 
	 in dieser Funktion ein zweiter Descriptor erzeugt, der auf die
	 gleiche Datei zeigt. Bei Eingehen einer Bestaetigung wuerde aber 
	 die Datei und nur _ein_ Descriptor geloescht. Der zweite Descriptor
	 wuerde auf eine nicht vorhandene Datei zeigen und beim Aussenden
	 Einen Fehler erzeugen!
      */
      // Die Anzahl unbestaetigter Nachrichten ist damit logischerweise
      // auch 0
      unack = 0;
      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Queue geloescht",FWDLOGMASK_IFDEBUG);
      spool_priv = 0;
      spool_bul = 0;
      spool_dest = 0;
      spool_sonst = 0;
      while ((eintrag = readdir(fwd_dir)) != NULL )
	{
	  String fname = String(eintrag->d_name);
	  fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Datei ")+fname,FWDLOGMASK_IFDEBUG);
	  if (fname != String(".") && fname != String("..") &&
	      fname != String("in") && fname != String("out"))
	    {
	      try
		{
		  bool desc_valid;
		  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Lege Nachrichtendescriptor an.",FWDLOGMASK_IFDEBUG);
		  nachrichten_descriptor ndesc(fwddirname+fname);
		  desc_valid = true;

		  if (fname.slen() > 0)
		    {

		      //Das Loeschen "ueberfluessiger" Nachrichten, wurde von
		      // hier in den Desturktor verschoben. 
		      // Dort koennen auch alte Eigenschaftsnachrichten
		      // geloescht werden.

		      if ( fname[0] == 'F' )
		        {
			  if (ndesc.expired(MAX_FWD_F_AGE))
			    {
			      count_delete++;
			      ndesc.loesche_nachricht();
			      desc_valid = false;
			    }
			  else
			    spool_priv++;
			}
		      else if ( fname[0] == 'S' ) 
		        {
			  if (ndesc.expired(MAX_FWD_S_AGE))
			    {
			      count_delete++;
			      ndesc.loesche_nachricht();
			      desc_valid = false;
			    }
			  else
			    spool_bul++;
			}
		      else if ( fname[0] == 'D' )
			spool_dest++;
		      else
			spool_sonst++;
		    }

		  if ( desc_valid )
		    {
		      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Fuege Descriptor in Queue ein.",FWDLOGMASK_IFDEBUG);
		      tx_queue.push_back(ndesc);
		    }
		}
	      catch( Error_could_not_open_msg_file )
		{
		  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Konnte Nachrichtendatei beim Einlesen der TX_Queue nicht oeffnen.",FWDLOGMASK_IFDEBUG);
		}
	      catch( Error_could_not_read_msg_file )
		{
		  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Konnte Nachrichtendatei beim Einlesen der TX_Queue nicht lesen.",FWDLOGMASK_IFDEBUG);
		  // Wenn die Datei nicht mehr gelesen werden kann, wird sie
		  // an dieser Stelle geloescht!
		  remove(strtochar(fwddirname+fname));
		}
	    }
	}
      closedir(fwd_dir);
    }
  if (count_delete != 0)
    fwd_log.eintrag(fwd_partner,itoS(count_delete)+" veraltete Nachrichten beim Einlesen des Spoolverzeichnisses geloescht.",FWDLOGMASK_INIT);
  
  router.set_spooled_messages(fwd_partner,spool_priv,spool_bul,spool_dest,spool_sonst);
}

void fwd_interface::send_msg( nachrichten_descriptor &desc, String &outp )
{
  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Send-Message.",FWDLOGMASK_IFDEBUG);
  nachricht *msg = &(desc.get_nachricht());
  if ( msg->get_typ() == n_zeit )
    {
      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zeitmessung, aktuelle zeit eintragen",FWDLOGMASK_IFDEBUG);
      zeit_nachricht *zmsg = (zeit_nachricht*) msg;
      if (zmsg->typ == 'C')
	zmsg->t_orig = pzeit();
      else
	zmsg->t_tx = pzeit();
    } 
  try
    {
      String line = "";
      msg->PrintOn(line);
      fwd_log.eintrag(fwd_partner,String("< ")+line,FWDLOGMASK_IFIO);
      outp.append(line+String(cr));
    }
  catch( Error_destin_checksum_error )
    {
      fwd_log.eintrag(fwd_partner,"Pruefsummenfehler beim Aussenden eines Zielgebietes erkannt.",FWDLOGMASK_FWDERR);
    }
  // Eine Nachricht wurde ausgesendet, 
  // Wenn Nachricht zum ersten mal ausgesendet wird, dann Zaehler erhoehen.
  if (desc.get_versuche() == 0)
    unack++;
  desc.set_sendemarkierung();
}

void fwd_interface::rcvd_msg( const String&inp, String &outp )
{
  try
    {
      char ch;
      String line;
      ch = inp[0];
      switch(ch)
	{
	  case 'A' : {
	               bestaetigungs_nachricht ack_msg("");
		       ack_msg.ScanFrom(inp);
		       fwd_log.eintrag(fwd_partner,"Fwd-Intf: Bestaetigungsnachricht empfangen.",FWDLOGMASK_IFIO);
		       rcvd_ack_msg(ack_msg);
	             }
	             break;
	  case 'C' : {
	               datenbankaenderung_nachricht cmsg("");
		       cmsg.ScanFrom(inp);
		       fwd_log.eintrag(fwd_partner,"Fwd-Intf: Datenbankaenderungsnachricht empfangen",FWDLOGMASK_IFIO);
		       send_ack_msg(cmsg.m_id,outp);
		       router.route_message(cmsg,true,fwd_partner);
		       fwd_log.eintrag(fwd_partner,"Fwd-Intf: Datenbankaenderungsnachricht an Rouer gegeben.",FWDLOGMASK_IFDEBUG);
	             }
	             break;
	  case 'E' : {
	               eigenschaften_nachricht pmsg("");
		       pmsg.ScanFrom(inp);
		       fwd_log.eintrag(fwd_partner,"Fwd-Intf: Eigenschaften Nachricht emfpangen.",FWDLOGMASK_IFIO);
		       if (kommunikation_laeuft)
			 {
			   fwd_log.eintrag(fwd_partner,"Fwd-Intf: Kommunikation laeuft bereits.",FWDLOGMASK_IFDEBUG);
			   send_ack_msg(pmsg.m_id,outp);
			 }
		       else
			 {
			   gemeinsame_optionen = pmsg.Optionen * eigene_optionen;
			   fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Gemeinsame Optionen ermittelt ")+gemeinsame_optionen.get_string(),FWDLOGMASK_IFDEBUG); 
			   kommunikation_laeuft = true;
			   fwd_log.eintrag(fwd_partner,"Fwd-Intf: Kommunikation laeuft.",FWDLOGMASK_IFDEBUG);
			   send_ack_msg(pmsg.m_id,outp);
			   fwd_log.eintrag(fwd_partner,"Fwd-Intf: Router etablierte Verbindung melden.",FWDLOGMASK_IFDEBUG);
			   router.connection_established(fwd_partner,gemeinsame_optionen,thread_id);
			   fwd_log.eintrag(fwd_partner,"Fwd-Intf: Sendequeue einlesen.",FWDLOGMASK_IFDEBUG);
			   read_tx_queue();
			   fwd_log.eintrag(fwd_partner,"Fwd-Intf: Sende-Queue eingelesen.",FWDLOGMASK_IFDEBUG);
			 }
	              }
	              break;
	   case 'F' : {
	                funkruf_nachricht fmsg("");
			fmsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Funkrufnachricht empfangen.",FWDLOGMASK_IFIO);
			send_ack_msg(fmsg.m_id,outp);
			if (!gemeinsame_optionen.check_option(OPTION_SKYPERBOARDS) &&
			    is_board_msg(fmsg) )
			  {
			    try
			      {
				skyper_rubrik_nachricht smsg("");
				smsg = conv_funkruf_to_board(fmsg);
				router.route_message(smsg,true,fwd_partner);
			      }
			    catch( Error_could_not_convert_message )
			      {
			      }
			  }
			else
			  router.route_message(fmsg,true,fwd_partner);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Funkrufnachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
	              }
	              break;
	   case 'R' : {
	                datenbankanforderung_nachricht rmsg("");
			rmsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Anfordeungsnachricht emfpangen.",FWDLOGMASK_IFIO);
			send_ack_msg(rmsg.m_id,outp);
			router.route_message(rmsg,true,fwd_partner);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Anfordeungsnachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
		      }
	              break;
	   case 'U' : {
	                datenbankupdate_nachricht umsg("");
			umsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Update-Nachricht empfangen.",FWDLOGMASK_IFIO);
			send_ack_msg(umsg.m_id,outp);
			router.route_message(umsg,true,fwd_partner);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Update-Nachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
		      }
	              break;
	   case 'B' : {
	                skyper_rubrik_nachricht smsg("");
			smsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Skyper-Board-nachricht empfangen.",FWDLOGMASK_IFIO);
			send_ack_msg(smsg.m_id,outp);
			router.route_message(smsg,true,fwd_partner);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Skyper-Board-nachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
	              }
	              break;
	   case 'Z' : {
		        zeit_nachricht zmsg("MID");
			zmsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zeit-nachricht empfangen.",FWDLOGMASK_IFIO);
			send_ack_msg(zmsg.m_id,outp);
			if (gemeinsame_optionen.check_option(OPTION_ZEITMESSUNG))
			  {
			    router.route_message(zmsg,true,fwd_partner);
			    fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zeit-nachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
			  }
			else
			  {
			    fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zeitnachricht empfangen, ohne gesetzte Option Z",FWDLOGMASK_FWDERR);
			    fehler_zaehler++;
			  }
	              }
	              break;
	   case 'D' : {
	                zielgebiets_nachricht dmsg("");
			dmsg.ScanFrom(inp);
			fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zielgebiets-nachricht empfangen.",FWDLOGMASK_IFIO);
			send_ack_msg(dmsg.m_id,outp);
			if (gemeinsame_optionen.check_option(OPTION_ROUTINGAUSTAUSCH))
			  {
			    router.route_message(dmsg,true,fwd_partner);
			    fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zielgebiets-nachricht an Router gegeben.",FWDLOGMASK_IFDEBUG);
			  }
			else
			  {
			    fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zielgebietsnachricht empfangen, ohne gesetzte Option D",FWDLOGMASK_FWDERR);
			    fehler_zaehler++;
			  }
	              }
	              break;
	   default  : {
	                line = inp;
			line.upcase();
			if(line.in("RECONNECTED"))
			  {
			    verbindung_trennen = true;
			  }
			else
			  {
			    fwd_log.eintrag(fwd_partner,"Nachricht mit unbekanntem Kennbuchstaben empfangen.",FWDLOGMASK_FWDERR);
			    fwd_log.eintrag(fwd_partner,line,FWDLOGMASK_FWDERR);
			    fehler_zaehler++;
			  }
	              }
	}
    }
  catch( Error_wrong_message_typ )
    {
      fwd_log.eintrag(fwd_partner,"Falscher Nachrichtentyp empfangen.",FWDLOGMASK_FWDERR);
      fehler_zaehler++;
    }
  catch( Error_wrong_message_format )
    {
      fwd_log.eintrag(fwd_partner,"Falsches Nachrichtenformat empfangen.",FWDLOGMASK_FWDERR);
      fehler_zaehler++;
    }
}

void fwd_interface::rcvd_ack_msg( bestaetigungs_nachricht &ack_msg )
{
  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Empfange Bestaetigung.",FWDLOGMASK_IFDEBUG);
  fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Bestaetigungs-MID : >")+ack_msg.m_id+"<",FWDLOGMASK_IFDEBUG);
  for (t_queue_it it = tx_queue.begin(); it != tx_queue.end(); it++)
    {
      fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Nachricht aus Sende-queue, MID : >")+it->get_mid()+"<",FWDLOGMASK_IFDEBUG);
      
      if (ack_msg.m_id == it->get_mid())
	{
	  if ( it->get_versuche() == 1)
	    {
	      // Zunaechst letzte Sendezeit und Anzahl der benoetigten 
	      // Sendeversuche aus DEscriptor ermitteln.
	      zeit last_tx = it->get_letzte_sendung();
	      //int n_tries = it->get_versuche();
	      // zeit zwischen Senden und empfangen:
	      double t_rtt = (double) (zeit() - last_tx);
	      double t_w_d = (double) t_w;
	      // Und jetzt t_w dynamisch anpassen
	      double t_w_neu = ( 1 - DYN_PARA_T_W ) * t_w_d + 
		                 DYN_PARA_T_W       * ( 2 * t_rtt + 10 );
	      fwd_log.eintrag(fwd_partner,String("Fwd-Intf: t_w-Anpassung :") + itoS(t_w) + String(",") + itoS((int) t_rtt) + String(",") + itoS((int) t_w_neu),FWDLOGMASK_IFDEBUG);
	      t_w = (int) t_w_neu;
	      // n_max wird derzeit noch nicht dynamisch angepasst
	    }
	  else
	    fwd_log.eintrag(fwd_partner,String("Fwd-Intf: keine t_w-Anpassung : mehr als ein Versuch"),FWDLOGMASK_IFDEBUG);
	  
	  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Nachricht aus Sende-Queue loeschen.",FWDLOGMASK_IFDEBUG);
	  it->loesche_nachricht();
	  // Bestaetigung wurde empfangen, unack-Zaehler erniedrigen
	  unack--;
	  // Wenn eine Bestaetigungsnachricht empfangen wurde heisst das,
	  // dass erfolgreich eine Nachricht an den Partner versendet wurde
	  // also Zaehler erhoehen
	  out_msg++;
	  if ( it->get_typ() == n_funkrufe )
	    spool_priv--;
	  else if (it ->get_typ() == n_skyper_board )
	    spool_bul--;
	  else if (it->get_typ() == n_destination )
	    spool_dest--;
	  else
	    spool_sonst--;
	  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Descriptor aus Liste nehmen.",FWDLOGMASK_IFDEBUG);
	  tx_queue.erase(it);
	  router.set_spooled_messages(fwd_partner,spool_priv,spool_bul,spool_dest,spool_sonst);
	  return;
	}
    }
}

void fwd_interface::send_ack_msg(const Mid &m, String &outp )
{
  // Wenn eine Bestaetigungsnachricht versendet wird, heisst das, dass 
  // erfolgreich eine Nachricht empfangen wurde, also Zaehler erhoehen
  in_msg++;
  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Sende Bestaetigung.",FWDLOGMASK_IFDEBUG);
  bestaetigungs_nachricht ack_msg(m);
  String line = "";
  ack_msg.PrintOn(line);
  outp.append(line+cr);
  fwd_log.eintrag(fwd_partner,String("< ")+line,FWDLOGMASK_IFIO);
}

void fwd_interface::check_tx_queue( String &outp )
{

  fwd_log.eintrag(fwd_partner,"Fwd-Intf: ueberpruefe Sende-Queue.",FWDLOGMASK_IFDEBUG);
  for (t_queue_it it = tx_queue.begin(); it != tx_queue.end(); it++)
    {

      // Wenn die Anzahl der unbestaetigten, ausgesendeten Nachrichten
      // das Maximum ueberschreitet, werden keine Nachrichten mehr 
      // ausgesendet.

      if (unack >= unack_max)
	return;

      fwd_log.eintrag(fwd_partner,"Fwd-Intf: ueberpruefe Nachricht",FWDLOGMASK_IFDEBUG);
      unsigned int n = it->get_versuche();
      fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Anzahl der bisherigen Versuche : ")+itoS(n),FWDLOGMASK_IFDEBUG);
      if (n > n_max)
	verbindung_trennen = true;
      else
	{
	  delta_t dt = zeit() - it->get_letzte_sendung();
	  fwd_log.eintrag(fwd_partner,String("Fwd-Intf: Zeit seit letzter Aussendung : ")+dt.get_string(),FWDLOGMASK_IFDEBUG);
	  if (n == 0 || zeit() - it->get_letzte_sendung() > t_w )
	    send_msg(*it,outp);
	}
    }
}

funkruf_nachricht fwd_interface::conv_board_to_funkruf( const skyper_rubrik_nachricht &smsg )
{
  funkruf_nachricht fmsg(smsg.m_id);
  
  try
    {
      fmsg.absender = smsg.absender;
      fmsg.adr = adress(4520,3);
      fmsg.dest = smsg.dest;
      fmsg.domain = 'B';
      fmsg.typ = 'A';
      fmsg.priority = smsg.priority;
      fmsg.master = smsg.master;
      board brd(smsg.board,configuration);
      int bid = brd.get_brd_id();
      fmsg.text = String( (char) (bid + 0x1f) );
      fmsg.text.append( String( (char) (0x20) ) );
      
      for (unsigned int i = 0;i < smsg.text.slen(); i++ )
	fmsg.text.append( String( (char) ((int) smsg.text[i]+1) ) ); 
    }
  catch( Error_could_not_open_boardfile )
    {
      fwdlog logf(configuration);
      logf.eintrag(String("Nachricht fuer unbekanntes Board ")+smsg.board+" im Fwd empfangen.",FWDLOGMASK_FWDERR);
      throw Error_could_not_convert_message();
    }
  
  return fmsg;
  
}

bool fwd_interface::is_board_msg( const funkruf_nachricht &fmsg )
{
  return (fmsg.adr == adress(4520,3)) && fmsg.domain == 'B' && fmsg.typ == 'A';
}

skyper_rubrik_nachricht fwd_interface::conv_funkruf_to_board( const funkruf_nachricht &fmsg )
{
  skyper_rubrik_nachricht smsg(fmsg.m_id);
  if (is_board_msg(fmsg) )
    {
      try
	{
	  smsg.absender = fmsg.absender;
	  smsg.dest = fmsg.dest;
	  smsg.priority = fmsg.priority;
	  smsg.lifetime = 14;
	  smsg.slot = -1;
	  smsg.master = fmsg.master;
	  int bid = ((int) fmsg.text[0]) - 0x1f;
	  
	  board brd(RUB_WETTER,configuration);
	  boards brds;
	  if ( !brds.get_board_by_id(bid,brd))
	    throw Error_could_not_convert_message();

	  smsg.board = brd.get_name();
	  smsg.text = String("");
	  for (unsigned int i = 2;i < fmsg.text.slen(); i++ )
	    smsg.text.append( String( (char) ((int) fmsg.text[i]-1) ) ); 
	}
      catch( Error_could_not_open_boardfile )
	{
	  fwdlog logf(configuration);
	  logf.eintrag(String("Kann Standardboard ")+String(RUB_WETTER)+" nicht oeffnen",FWDLOGMASK_FWDERR);
	  throw Error_could_not_convert_message();
	}
    }
  else
    throw Error_could_not_convert_message();


  return smsg;
}

fwd_interface::fwd_interface(String &outp, callsign partner, bool ax_flag, connect_string& con_pf, bool ar) : interfaces(outp,ax_flag), fwd_log(configuration)
{
  // Initialisieren und Namen des Forward-Spool-Directories holen
  init(ar);  
  fwddirname = configuration.find("FWDDIR") + partner.str() + String('/');
  fwd_partner = partner;
  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Linkaufbau",FWDLOGMASK_CONSTRTSTP);
  // Protokolleigenschaften-Nachricht und dazugehoerigen
  // Descriptor erzeugen und in Sendequeue eintragen.
  try
    {
      Mid m = router.get_mid();
      eigenschaften_nachricht pmsg(m,PROT_VERSION,eigene_optionen);
      nachrichten_descriptor pmsg_desc(pmsg,fwddirname);
      tx_queue.push_back(pmsg_desc);
      spool_sonst++;
      set_connect_path(con_pf);
      first_connect(outp);
    }
  catch( Error_could_not_gen_new_mid )
    {
      throw Error_could_not_init_fwd_interface();
    }
}

fwd_interface::fwd_interface(String &outp, callsign partner, callsign gegen, bool ax_flag, bool ar) : interfaces(outp,ax_flag), fwd_log(configuration)
{
  // Initialisieren und Namen des Forward-Spool-Directories holen
  init(ar);  
  fwddirname = configuration.find("FWDDIR") + partner.str() + String('/');
  fwd_partner = partner;
  gegenstation = gegen;
  // Der connect ging von der Gegenseite aus. Daher :
  path_finished = true;
  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Verbindung akzeptiert.",FWDLOGMASK_CONSTRTSTP);
  // Protokolleigenschaften-Nachricht und dazugehoerigen
  // Descriptor erzeugen und in Sendequeue eintragen.
  try
    {
      Mid m = router.get_mid();
      eigenschaften_nachricht pmsg(m,PROT_VERSION,eigene_optionen);
      nachrichten_descriptor pmsg_desc(pmsg,fwddirname);
      tx_queue.push_back(pmsg_desc);
      // Verbindung mit Forwardpartner wurde von diesem aufgebaut und steht
      // somit.
      path_finished = true;
      check_tx_queue(outp);
    } 
  catch( Error_could_not_gen_new_mid )
    {
      throw Error_could_not_init_fwd_interface();
    }
}

bool fwd_interface::do_process(bool rx_flag, String &outp)
{
  outp = "";

  if (rx_flag)
    {
      letzte_aktivitaet = zeit();
      {
	String line;
	while (get_line(line))
	  {
	    String l2 = line;
	    l2.upcase();
	    if ( l2.in("RECONNECTED") )
	      {
		fehler = false;
		verbindung_trennen = true;
	      }
	    else
	      {
		fwd_log.eintrag(fwd_partner,String("> ")+line,FWDLOGMASK_IFIO);
		rcvd_msg(line,outp);
		if (fehler_zaehler > max_fehler_zaehler)
		  {
		    fwd_log.eintrag(fwd_partner,"Fehlerzaehler ueberschreitet Maximum",FWDLOGMASK_FWDERR);
		    fehler = true;
		    verbindung_trennen = true;
		  }
		else if (kommunikation_laeuft)
		  poll(outp);
	      }
	  }
      }
    }
  else
    {
      if (kommunikation_laeuft)
	{
	  poll(outp);
	  if (zeit() - letzte_aktivitaet > fwd_timeout)
	    verbindung_trennen = true;
	  return !verbindung_trennen;
	}
      else
	if (zeit() - letzte_aktivitaet > DEFAULT_CONNECTION_SETUP_TIMEOUT )
	  {
	    fehler = true;
	    return false;
	  }
    }
  fwd_log.eintrag(fwd_partner,String("< ")+outp,FWDLOGMASK_IFIO);
  router.set_interface_parameter(fwd_partner,t_w,n_max,unack,fehler_zaehler);
  return !verbindung_trennen;
}


void fwd_interface::poll( String &outp )
{
  try
    {
      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Polle nach neuen Nachrichten.",FWDLOGMASK_IFDEBUG);
      while( router.message_avalable(fwd_partner)  )
	{
	  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Neue Nachricht verfuegbar.",FWDLOGMASK_IFDEBUG);
	  nachricht *msg_ptr = &router.get_tx_message(fwd_partner);
	  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Zeiger auf Nachricht erhalten.",FWDLOGMASK_IFDEBUG);

	  try
	    {
	      if (msg_ptr->get_typ() == n_zeit &&
		  !gemeinsame_optionen.check_option(OPTION_ZEITMESSUNG) )
		throw Error_messagetyp_not_supported();
	      if (msg_ptr->get_typ() == n_destination &&
		  !gemeinsame_optionen.check_option(OPTION_ROUTINGAUSTAUSCH) )
		throw Error_messagetyp_not_supported();
	      
	      if (msg_ptr->get_typ() == n_skyper_board &&
		  !gemeinsame_optionen.check_option(OPTION_SKYPERBOARDS)  )
		{
		  try
		    {
		      funkruf_nachricht fmsg = conv_board_to_funkruf(*(skyper_rubrik_nachricht*) msg_ptr);
		      fwd_log.eintrag(fwd_partner,"Fwd-Intf: Funkrufnachricht erzeugt.",FWDLOGMASK_IFDEBUG);
		      nachrichten_descriptor desc(fmsg,fwddirname);
		      tx_queue.push_back(desc);
		      spool_priv++;
		      fwd_log.eintrag(fwd_partner,"Fwd-Intf: und in Sendequeue eingetragen.",FWDLOGMASK_IFDEBUG);
		    }
		  catch( Error_could_not_convert_message )
		    {}
		}
	      else
		{
		  
		  if ( msg_ptr->get_typ() == n_funkrufe )
		    spool_priv++;
		  else if (msg_ptr ->get_typ() == n_skyper_board )
		    spool_bul++;
		  else if (msg_ptr->get_typ() == n_destination )
		    spool_dest++;
		  else
		    spool_sonst++;
		  
		  nachrichten_descriptor desc(*msg_ptr,fwddirname);
		  tx_queue.push_back(desc);
		  fwd_log.eintrag(fwd_partner,"Fwd-Intf: Und in Sendequeue eingetragen.",FWDLOGMASK_IFDEBUG);
		}
	    }
	  catch( Error_could_not_open_msg_file )
	    {
	      fwd_log.eintrag(fwd_partner,"Kann im Interfacespoolfile keine Datei anlegen.",FWDLOGMASK_FWDERR);
	    }
	  catch( Error_could_not_write_msg_file )
	    {
	      fwd_log.eintrag(fwd_partner,"Kann Interfacespoolfile nicht schreiben. Moeglicherweise ist Destination Korrupiert.",FWDLOGMASK_FWDERR);
	    }
	}
      check_tx_queue(outp);
    }
  catch( Error_wrong_message_typ )
    {
      fwd_log.eintrag(fwd_partner,"Kann Nachricht nicht wieder einlesen, falscher Typ",FWDLOGMASK_FWDERR);
      verbindung_trennen = true;
    }
  catch( Error_wrong_message_format )
    {
      fwd_log.eintrag(fwd_partner,"Kann Nachricht nicht wieder einlesen,Formatfehler",FWDLOGMASK_FWDERR);
      verbindung_trennen = true;
    }
  catch( Error_request_for_non_neighbor )
    {
      fwd_log.eintrag(fwd_partner,"Router kennt Nachbar nicht mehr",FWDLOGMASK_FWDERR);
      verbindung_trennen = true;
    }
  catch( Error_no_message_available )
    {
      fwd_log.eintrag(fwd_partner,"Nachrichtenzaehlung im Router inkonsistent",FWDLOGMASK_FWDERR);
    }
  catch( Error_messagetyp_not_supported )
    {
      fwd_log.eintrag(fwd_partner,"Router erzeugt von Nachbarn nicht unterstuetzten Nachrichtentyp",FWDLOGMASK_FWDERR);
    }
  router.set_spooled_messages(fwd_partner,spool_priv,spool_bul,spool_dest,spool_sonst);

}

fwd_interface::~fwd_interface( void )
{
  // Vor dem Aufloesen des Interfaces nochmal die Sendequeue durchgehen
  // und nicht mehr benoetigte Nachrichten loeschen.

  for (t_queue_it it = tx_queue.begin(); it != tx_queue.end(); ++it )
    {
      if ( it->get_typ() == n_eigenschaften ||
	   it->get_typ() == n_zeit ||
	   it->get_typ() == n_destination )

	it->loesche_nachricht();

      if (it->expired() )
	it->loesche_nachricht();
    }

  // Nun Sendequeue loeschen
  tx_queue.clear();

  fwd_log.eintrag(fwd_partner,"Fwd-Intf : Verbindung getrennt.",FWDLOGMASK_CONSTRTSTP);
  if (fehler)
    router.connection_failed(fwd_partner,thread_id);
  else
    router.connection_closed(fwd_partner,thread_id);
  router.set_spooled_messages(fwd_partner,spool_priv,spool_bul,spool_dest,spool_sonst);
  router.set_interface_parameter(fwd_partner,t_w,n_max,unack,fehler_zaehler);
}
