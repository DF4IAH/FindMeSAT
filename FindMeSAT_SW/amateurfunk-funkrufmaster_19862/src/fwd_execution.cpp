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


#include "fwd_execution.h"
#include "autoconf.h"

#include "fwd_router.h"
#include "mid.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "config.h"
#include "logfile.h"
#include "board.h"

extern callsign G_mycall;
extern fwd_router router;
extern callsign_database calls;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

extern config_file configuration;


void check_and_send_entry( const database_entry &eintrag, zeit von, const callsign &from )
{
  // Es werden nur Eintraege bearbeitet, die seit der Zeit 'von' bearbeitet
  // wurden und eine gueltige Pageradresse enthalten.
  //
  if ( eintrag.get_last_change() > von && eintrag.get_adr().gueltig() )
    {
      bool flag = false;
      int count = 0;
      Mid m;
      while (count < 2 && !flag)
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
	  datenbankupdate_nachricht umsg(m);
	  
	  umsg.typ = 'A';
	  umsg.rufzeichen = eintrag.get_call();
	  umsg.adr = eintrag.get_adr();
	  umsg.ptyp = eintrag.get_geraet();
	  umsg.loc = eintrag.get_locator();
	  umsg.name = eintrag.get_name();
	  umsg.tm = eintrag.get_last_change();
	  umsg.master = eintrag.get_server();
	  
	  router.route_message( umsg,false,G_mycall,from);
	}
      else
	throw Error_could_not_gen_updatemessage();
    }
}

void fwd_execution::exec_msg(const nachricht &msg, const callsign &from )
{
  n_types typ;
  typ = msg.get_typ();

  if (typ == n_funkrufe )
    {
#ifdef COMPILE_SLAVES
      funkruf_nachricht fmsg = (*(funkruf_nachricht*) &msg);
      spool.spool_msg(false,fmsg.absender,fmsg.master,zeit(0),fmsg.adr,fmsg.text,true,fmsg.dest,fmsg.priority);
#endif
    }
  else if (typ == n_skyper_board )
    {
#ifdef COMPILE_SLAVES
      skyper_rubrik_nachricht smsg = (*(skyper_rubrik_nachricht*) &msg);
      try
	{
	  board brd(smsg.board,configuration);
	  int board = brd.get_brd_id();
	  int slot;
	  if (smsg.slot == -1)
	    slot = brd.get_slot();
	  else
	    slot = smsg.slot;
	  brd.set_msg(smsg.text,slot,smsg.dest,smsg.lifetime);

	  spool.spool_bul(smsg.master,zeit(0),board,slot,smsg.text,true,smsg.dest,smsg.priority);
	}
      catch( Error_could_not_open_boardfile )
	{
	  fwdlog logf(configuration);
	  logf.eintrag("Nachricht fuer unbekanntes Board "+smsg.board+" im Fwd empfangen.",FWDLOGMASK_FWDERR);
	}
#endif
    }
  else if (typ == n_aenderungen )
    {
      datenbankaenderung_nachricht cmsg = (*(datenbankaenderung_nachricht*) &msg);
      bool exist = false;
      bool local_gueltig = false;
      zeit last_local_change;
      database_entry eintrag;

      try
	{
	  eintrag = calls.find(cmsg.rufzeichen);
	  exist = true;
	  last_local_change = eintrag.get_last_change();
	  local_gueltig = eintrag.get_adr().gueltig();
	}
      catch( Error_callsign_does_not_exist )
	{
	  exist = false;
	}
      if (exist)
	{
	  if ((last_local_change < cmsg.tm) || !local_gueltig )
	    if (cmsg.typ == 'A' || cmsg.typ == 'C')
	      {
		eintrag.set_adr(cmsg.adr,cmsg.master,cmsg.tm);
		eintrag.set_geraet(cmsg.ptyp,cmsg.master,cmsg.tm);
		eintrag.set_loc(cmsg.loc,cmsg.master,cmsg.tm);
		eintrag.set_name(cmsg.name,cmsg.master,cmsg.tm);
		calls.change(cmsg.rufzeichen,eintrag);
	      }
	    else if (cmsg.typ == 'R')
	      {
		calls.del(cmsg.rufzeichen);
	      }
	}
      else
	if (cmsg.typ == 'A' || cmsg.typ == 'C')
	  {
	    try
	      {
		eintrag.set_call(cmsg.rufzeichen,cmsg.master,cmsg.tm);
		eintrag.set_adr(cmsg.adr,cmsg.master,cmsg.tm);
		eintrag.set_geraet(cmsg.ptyp,cmsg.master,cmsg.tm);
		eintrag.set_loc(cmsg.loc,cmsg.master,cmsg.tm);
		eintrag.set_name(cmsg.name,cmsg.master,cmsg.tm);
		calls.add(eintrag);
	      }
	    catch( Error_callsign_already_exists )
	      {
	      }
	  }
    }
  else if (typ == n_update)
    {
      datenbankupdate_nachricht cmsg = (*(datenbankupdate_nachricht*) &msg);
      bool exist = false;
      bool local_gueltig = false;
      zeit last_local_change;
      database_entry eintrag;

      try
	{
	  eintrag = calls.find(cmsg.rufzeichen);
	  exist = true;
	  last_local_change = eintrag.get_last_change();
	  local_gueltig = eintrag.get_adr().gueltig();
	}
      catch( Error_callsign_does_not_exist )
	{
	  exist = false;
	}
      if (exist)
	{
	  if ((last_local_change < cmsg.tm) || !local_gueltig )
	  {
	    if (cmsg.typ == 'A' || cmsg.typ == 'C')
	      {
		eintrag.set_adr(cmsg.adr,cmsg.master,cmsg.tm);
		eintrag.set_geraet(cmsg.ptyp,cmsg.master,cmsg.tm);
		eintrag.set_loc(cmsg.loc,cmsg.master,cmsg.tm);
		eintrag.set_name(cmsg.name,cmsg.master,cmsg.tm);
		calls.change(cmsg.rufzeichen,eintrag);
	      }
	    else if (cmsg.typ == 'D')
	      {
		calls.del(cmsg.rufzeichen);
	      }
	   }
	  else  // Mal sehen, ob wir neue Infos einpflegen koennen
	    {
	      if (cmsg.typ == 'A' || cmsg.typ == 'C')
		{
		  try
		    {
		      database_entry Variable = calls.find(cmsg.rufzeichen);
		      locator Loc=Variable.get_locator();
		      String name=Variable.get_name();
		      pager_typ ger=Variable.get_geraet();
		      bool changed = false;
		      
		      if (name=="" && cmsg.name!="")
			{
			  eintrag.set_name(cmsg.name,cmsg.master,cmsg.tm);
			  changed=true;
			}
		      
		      // ToDo: Das "NO DEFINED" aus der database nehmen
		      if (ger ==pager_typ("NO DEFINED") && cmsg.ptyp !=pager_typ("NO DEFINED"))
			{
			  eintrag.set_geraet(cmsg.ptyp,cmsg.master,cmsg.tm);
			  changed=true;
			}
		      
		      if (!Loc.ok() && cmsg.loc.ok())
			{
			  eintrag.set_loc(cmsg.loc,cmsg.master,cmsg.tm);
			  changed=true;
			}
		      
		      if (changed)
			calls.change(cmsg.rufzeichen,eintrag);
		    }		    
		  catch( Error_callsign_does_not_exist )
		    {
		    }	    
		}
	    }
	}
      else
	if (cmsg.typ == 'A' || cmsg.typ == 'C')
	  {
	    try
	      {
		eintrag.set_call(cmsg.rufzeichen,cmsg.master,cmsg.tm);
		eintrag.set_adr(cmsg.adr,cmsg.master,cmsg.tm);
		eintrag.set_geraet(cmsg.ptyp,cmsg.master,cmsg.tm);
		eintrag.set_loc(cmsg.loc,cmsg.master,cmsg.tm);
		eintrag.set_name(cmsg.name,cmsg.master,cmsg.tm);
		calls.add(eintrag);
	      }
	    catch(  Error_callsign_already_exists )
	      {
	      }
	  }
    }
  else if (typ == n_updateanforderung)
    {
      try
	{
	  datenbankanforderung_nachricht rmsg = (*(datenbankanforderung_nachricht*) &msg);
	  zeit von = rmsg.tm;
	  database_entry eintrag;
	  if (calls.first(eintrag))
	    {
	      check_and_send_entry(eintrag,von,from);
	      while (calls.next(eintrag))
		check_and_send_entry(eintrag,von,from);
	    }
	}
      catch(  Error_could_not_gen_updatemessage )
	{
	  fwdlog logf(configuration);
	  logf.eintrag("Konnte Updateanforderung nicht ausfuehren.",FWDLOGMASK_FWDERR);
	}
    }
}
