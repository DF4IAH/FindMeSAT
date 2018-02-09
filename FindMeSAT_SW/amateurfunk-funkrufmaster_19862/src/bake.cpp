/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; you can redistribute it and/or modify     *
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

#include "bake.h"
#include "autoconf.h"

#include <stdio.h>
#include <iostream.h>
#include <fstream.h>

#include "callsign.h"
#include "board.h"
#include "zeit.h"
#include "logfile.h"


extern config_file configuration;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

bake::bake(  )
{
}

bake::~bake( void )
{}

void bake::send( void )
{
  //  send, allgemein
}

void bake::PrintOn(ostream& strm)
{
  strm << '#' << endl;
}

void bake::show( ostream &ostr, char cr )
{
  ostr << cr;
}

bake::b_typ bake::get_typ( void )
{
  return b_kein;
}

tx_bake::tx_bake( config_file &cfg, int i )
{
  char parameterstring[40];
  try
    {
      b_num = i;
      sprintf(parameterstring,"BAKE_%i",i);
      baken_text = cfg.find(String(parameterstring));
      sprintf(parameterstring,"BAKEN_SLOT_%i",i);
      slot = atoi(strtochar(cfg.find(String(parameterstring))));
      if (slot <= 10)
	slot += 32;
      sprintf(parameterstring,"BAKEN_PFAD_%i",i);
      pfad = cfg.find(String(parameterstring));
      num = 0;
    }
  catch(Error_parameter_not_defined )
    {
      throw Error_could_not_create_beacon();
    }
}

tx_bake::tx_bake( istream &strm, int bn, const String &pfd )
{
  b_num = bn;
  pfad = pfd;

  bool t_defined = false;
  bool s_defined = false;
  char puffer[255];

  while ( (!t_defined || !s_defined) && strm.getline(puffer,254) )
    {
      if (*puffer != '#')
	{
	  istringstream input(puffer);
	    char parameter[60];
	    char value[200];
	    char ch;
	    
	    // Einen Input-String-Stream aus der Zeile erzeugen
	    // und parameter und Value daraus extrahieren
	    input.get(parameter,59,'=');
	    input >> ch;
	    if (ch == '=')
	      {
		input.get(value,199);
		String Par(parameter);
		String Val(value);

		if (Par == String("TEXT"))
		  {
		    baken_text = Val;
		    t_defined = true;
		  }
		else if (Par == String("SLOT"))
		  {
		    slot = Val.Stoi();
		    if (slot <= 10)
		      slot += 32;
		    s_defined = true;
		  }
		else
		  {
		    cerr << "Fehler Unbekannter Bakenparameter " << puffer << endl;
		  }
	      }
	}
    }
  if (!t_defined || !s_defined)
    throw Error_could_not_create_beacon();
    
}

tx_bake::tx_bake( int sl, int n, const String &pf, const String &text )
{
  slot = sl;
  b_num = n;
  pfad = pf;
  baken_text = text;
  num = 0;
}

void tx_bake::PrintOn( ostream &strm )
{
  strm << "POCBAKE" << endl;
  strm << "TEXT=" << baken_text << endl;
  strm << "SLOT=" << slot-32 << endl;
}

void tx_bake::show( ostream &strm, char cr )
{
  strm << "Senderbake :" << cr;
  strm << "Slot : " << slot-32 << cr;
  strm << "Pfad : " << pfad << cr;
  strm << "Text : " << cr << baken_text << cr;
}

void tx_bake::send( void )
{
  String filename = pfad + "FUNKRUF.BAKE.9999";
  filename.append(String( (char) (64 + b_num) ) ); 
  ofstream ostrm(strtochar(filename));
  ostrm << "4520.3\r";
  ostrm << (char) (95+0x1f) ;
  ostrm << (char) slot;
  String line = macros.line(baken_text);
  for (unsigned int i = 0;i < line.slen();i++)
    ostrm << (char) ( (int) line[i] + 1 );
  if (num++ % 2 == 1) ostrm << (char) 33;
  ostrm.close();
}

bake::b_typ tx_bake::get_typ( void )
{
  return b_tx;
}



status_bake::status_bake( config_file &cfg )
{
  try
    { 
      baken_text = cfg.find(String("STATUSBAKE"));    
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Bakenparameter fuer Status-Bake nicht definiert" << endl;
      throw Error_could_not_create_beacon();
    }
}


void status_bake::send( void )
{
#ifdef COMPILE_SLAVES
  syslog logf(configuration);
  try
    {
      String msg = macros.line(baken_text);
      board brd(RUB_STATUSBAKE,configuration);
      int brd_id = brd.get_brd_id();
      int slot = brd.get_slot();
      destin ds = get_default_destin();
      brd.set_msg(msg,slot,ds);
      spool.spool_bul(G_mycall,zeit(),brd_id,slot,msg,false,ds,96);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Kann Rubrik STATUSBAKE nicht oeffnen.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Kann Rubrik STATUSBAKE nicht anlegen.",LOGMASK_PRGRMERR);
    }
#endif
}

void status_bake::PrintOn( ostream& strm )
{
  strm << '#' << endl;
}

void status_bake::show( ostream &strm, char cr )
{
  strm << "Statusbake, Text : " << cr;
  strm << baken_text << cr;
}

void show_baken(t_baken &baken, ostream &ostr, char cr )
{
  for (t_baken::iterator it = baken.begin(); it != baken.end(); ++it)
    {
      (*it)->show(ostr,cr);
      ostr << cr;
      ostr << "------------------------------------------------------" << cr;
    }
  ostr << cr << cr;
}

bake::b_typ status_bake::get_typ( void )
{
  return b_status;
}
