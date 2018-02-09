/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Holger Flemming                                    *
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

#include "callsign.h"
#include "relais.h"
#include "config.h"
#include "logfile.h"
#include "destin.h"
#include "board.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif

locator my_loc;

extern config_file configuration;
extern callsign G_mycall;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

bool relais::ScanFrom( istream &strm, const relais &last )
{
  String line;
  line.getline(strm,150);

  String puffer;
  try
    {
      puffer = line.copy(0,6);

      call = callsign(puffer);

      puffer = line.copy(10,10);
      qrg = puffer.Stod();

      try
	{
	  puffer = line.copy(40,6);
	  loc = locator(puffer);
	}
      catch( Error_not_a_locator )
	{
	  if (samecall(call,last.call))
	    loc = last.loc;
	}

      puffer = line.copy(47,19);
      puffer.kuerze();
      if (puffer.slen() < 3 && samecall(call,last.call) )
	qth = last.qth;
      else
	qth = puffer;

      puffer = line.copy(71,2);
      if (puffer == "FM")
	tp = rt_fm;
      else if (puffer == "PR")
	tp = rt_digi;
      else if (puffer == "AT")
	tp = rt_atv;
      else if (puffer == "BC")
	tp = rt_bake;
      else if (puffer == "RY")
	tp = rt_rtty;
      else if (puffer == "TP")
	tp = rt_transponder;
      else
	tp = rt_unknown;

      return true;

    }
  catch( Error_String_index_out_of_range )
    {
      return false;
    }
  catch( Error_no_callsign )
    {
      return false;
    }
}

String relais::get_spoolmsg( void )
{
  String msg = qth+String("                   ");
  msg = msg.copy(0,13) + String(' ') + loc.str();
  msg.append(call.str());
  while (msg.slen() < 26)
    msg.append(String(' '));
  msg.append(String("    "));
  msg.append(dtoS(qrg,10,4));

  return msg;
}

bool cmpdist ( relais &r1, relais &r2 )
{
  struct strecke abst1 = r1.loc - my_loc;
  struct strecke abst2 = r2.loc - my_loc;

  return abst1.entfernung < abst2.entfernung;
}

bool cmpqrg(relais &r1, relais &r2 )
{
  return r1.qrg < r2.qrg;
}

void relais_database::load(const String &fname )
{
  ifstream ifst(strtochar(fname));

  relais last;
  relais akt;

  relaise.clear();

  if (ifst)
    {
      while (!ifst.eof())
	{
	  if (akt.ScanFrom(ifst,last))
	    {
	      relaise.push_back(akt);
	      last = akt;
	    }
	}
    }
}

void relais_database::sort_dist( void )
{
  try
    {
      my_loc=locator(configuration.find("LOCATOR"));
      relaise.sort(str_cmpdist());
    }
  catch( Error_parameter_not_defined )
    {
      syslog slog(configuration);
      slog.eintrag("Parameter LOCATOR nicht definiert!",LOGMASK_PRGRMERR);
    }
}

void relais_database::sort_qrg( void )
{
  relaise.sort(str_cmpqrg());
}

relais_database relais_database::find_type( relais::relais_typ typ )
{
  relais_database new_db;

  new_db.relaise.clear();

  for (list<relais>::iterator it = relaise.begin(); it != relaise.end() ; ++it )
    {
      if (it->tp == typ )
	new_db.relaise.push_back(*it);
    }

  return new_db;
}


relais_database relais_database::find_qrg(double low, double high )
{
  relais_database new_db;

  new_db.relaise.clear();

  for (list<relais>::iterator it = relaise.begin(); it != relaise.end() ; ++it )
    {
      if (it->qrg >= low && it->qrg <= high )
	new_db.relaise.push_back(*it);
    }

  return new_db;
}

relais_database relais_database::reduce_n (int max )
{
  relais_database new_db;
  int i = 0;

  new_db.relaise.clear();

  for (list<relais>::iterator it = relaise.begin(); i < max && it != relaise.end() ; ++it )
    {
      new_db.relaise.push_back(*it);
      i++;
    }

  return new_db;
}

bool relais_database::first( relais &r )
{
  akt = relaise.begin();
  if (akt != relaise.end() )
    {
      r = *akt;
      return true;
    }
  else
    return false;
}

bool relais_database::next( relais &r )
{
  ++akt;
  if (akt != relaise.end() )
    {
      r = *akt;
      return true;
    }
  else
    return false;
}

void relais_database::PrintOn( ostream &strm )
{
  for (list<relais>::iterator it = relaise.begin(); it != relaise.end(); ++it)
    strm << it->get_spoolmsg() << endl;
}

void spool_database(relais_database &r, int start_slot, const String &brdname)
{
  syslog logf(configuration);
  destin ds = get_default_destin();
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht abschicken
      board brd(brdname,configuration);
      int board = brd.get_brd_id();
      int slot = start_slot;
      int ln = 1;
      String line;
      relais rel;

      if (r.first(rel))
	{
	  line = rel.get_spoolmsg();
	  ln++;
	  while (r.next(rel))
	    {
	      line.append(rel.get_spoolmsg());
	      ln++;
	      if (ln > 2)
		{
		  brd.set_msg(line,slot,ds);
#ifdef COMPILE_SLAVES
		  spool.spool_bul(G_mycall,zeit(),board,slot,line,false,ds,128);
#endif
		  line = String("");
		  ln = 1;
		  slot++;
		}
	    }
	}
    }
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Relais Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Relais Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}

