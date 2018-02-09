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

#include "fwd_frontend.h"

#include "fwd_nachrichten.h"
#include "fwd_router.h"
#include "spoolfiles.h"
#include "config.h"
#include "logfile.h"

extern callsign G_mycall;
extern fwd_router router;
extern callsign_database calls;
extern config_file configuration;

bool fwd_api::fwd_msg( const callsign &sender, const adress &adr, const destin &ds, unsigned int pri, const String &msg )
{
  try
    {
      Mid mid = router.get_mid();
      
      funkruf_nachricht fmsg(mid);
      fmsg.absender = sender;
      fmsg.adr = adr;
      fmsg.dest = ds;
      fmsg.domain = 'U';
      fmsg.typ = 'A';
      fmsg.priority = pri;
      fmsg.master = G_mycall;
      if ( msg.slen() > 80 )
	fmsg.text = msg.copy(0,80);
      else
	fmsg.text = msg;
      
      return router.route_message(fmsg,false,G_mycall);
    }
  catch( Error_could_not_gen_new_mid )
    {
      fwdlog logf(configuration);
      logf.eintrag("Fehler beim Generieren einer MID fuer Funkrufnachricht.",FWDLOGMASK_FWDERR);
      return false;
    }
  return false;
}

bool fwd_api::fwd_bul(  const callsign& sender, const String& board, int slot, const destin &ds, unsigned int priority, int lifetime, const String &msg )
{
  try
    {
      Mid mid = router.get_mid();
      skyper_rubrik_nachricht smsg(mid);
      
      smsg.absender = sender;
      smsg.board = board;
      smsg.slot = slot;
      smsg.dest = ds;
      smsg.priority = priority;
      smsg.lifetime = lifetime;
      smsg.master = G_mycall;
      if ( msg.slen() > 80 )
	smsg.text = msg.copy(0,80);
      else
	smsg.text = msg;
      
      return router.route_message(smsg,false,G_mycall);
    }
  catch( Error_could_not_gen_new_mid )
    {
      fwdlog logf(configuration);
      logf.eintrag("Fehler beim Generieren einer MID fuer Funkrufnachricht.",FWDLOGMASK_FWDERR);
      return false;
    }
  return false;
}



bool fwd_api::fwd_usr( char ch, const callsign &call )
{
  try
    {
      Mid mid = router.get_mid();
      datenbankaenderung_nachricht cmsg(mid);
      
      cmsg.typ = ch;
      cmsg.rufzeichen = call;
      
      if (ch == 'A' || ch == 'C')
	{
	  database_entry eintrag = calls.find(call);
	  cmsg.adr = eintrag.get_adr();
	  // 
	  // Datensaetze ohne gueltige pageradresse sollen nichts ins
	  // Forward kommen.
	  if (!cmsg.adr.gueltig())
	    return false;
	  cmsg.ptyp = eintrag.get_geraet();
	  cmsg.loc = eintrag.get_locator();
	  cmsg.name = eintrag.get_name();
	  cmsg.tm = eintrag.get_last_change();
	  cmsg.master = eintrag.get_server();
	}
      else
	{
	  cmsg.tm = zeit();
	  cmsg.master = G_mycall;
	}
      return router.route_message(cmsg,false,G_mycall);

    }
  catch( Error_callsign_does_not_exist )
    {
      fwdlog logf(configuration);
      logf.eintrag("Fehler, Datenbankaenderungsnachricht fuer nicht existierendes Rufzeichen.",FWDLOGMASK_FWDERR);
      return false;
    }
  catch( Error_could_not_gen_new_mid )
    {
      fwdlog logf(configuration);
      logf.eintrag("Fehler beim Generieren einer MID fuer Datanbanknachricht.",FWDLOGMASK_FWDERR);
      return false;
    }
}

bool fwd_api::fwd_update_request( const zeit &von, const callsign &an )
{
  try
    {
      Mid mid = router.get_mid();
      datenbankanforderung_nachricht rmsg(mid);
      
      rmsg.tm = von;
      
      return router.route_message(rmsg,false,G_mycall,an);
    }
  catch( Error_could_not_gen_new_mid )
    {
      fwdlog logf(configuration);
      logf.eintrag("Fehler beim Generieren einer MID fuer Anforderungsnachricht.",FWDLOGMASK_FWDERR);
      return false;
    }
}

bool fwd_api::check_and_send_entry( const database_entry& eintrag, const zeit &von, const zeit &bis )
{
  zeit last_ch = eintrag.get_last_change();
  callsign from = eintrag.get_server();

  if ( eintrag.get_adr().gueltig() )
    if (last_ch >= von && last_ch <= bis && samecall(from,G_mycall) )
      {
	bool flag = false;
	int count = 0;
	Mid m;
	while (count < 5 && !flag)
	  {
	    try
	      {
		m = router.get_mid();
		flag = true;
	      }
	    catch( Error_could_not_gen_new_mid )
	      {
		sleep(1);
		count++;
	      }
	  }
	if (flag)
	  {
	    datenbankaenderung_nachricht cmsg(m);
	  
	  cmsg.typ = 'C';
	  cmsg.rufzeichen = eintrag.get_call();
	  cmsg.adr = eintrag.get_adr();
	  cmsg.ptyp = eintrag.get_geraet();
	  cmsg.loc = eintrag.get_locator();
	  cmsg.name = eintrag.get_name();
	  cmsg.tm = eintrag.get_last_change();
	  cmsg.master = eintrag.get_server();
	  
	  return router.route_message( cmsg,false,G_mycall);
	}
    }
  return false;
}

int fwd_api::fwd_database_transmitt( const zeit& von, const zeit &bis )
{
  database_entry eintrag;
  int zaehler = 0;

  if (calls.first(eintrag))
    {
      if (check_and_send_entry(eintrag,von,bis))
	zaehler++;
      while( calls.next(eintrag) )
	if (check_and_send_entry(eintrag,von,bis))
	  zaehler++;
    }
  return zaehler;
}
