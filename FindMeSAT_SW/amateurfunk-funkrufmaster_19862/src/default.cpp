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

#include "autoconf.h"
#include "callsign.h"
#include "default.h"
#include "config.h"
#include "destin.h"
#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
#include "logfile.h"

extern config_file configuration;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif


skyper_board_default::skyper_board_default( istream& strm )
{
  boardname.get(strm,99,',');
  char ch;
  strm >> ch;
  strm >> slot;
  //  if (slot <= 10)
  //    slot += 32;
  text.getline(strm,250);

  text = String ("");
  for (int i = 0; i < 4; i++)
    {
      String l;
      l.getline(strm,250);
      l.append("                     ");
      text.append(l.copy(0,20));
    }
}


void skyper_board_default::process( void )
{
#ifdef COMPILE_SLAVES
  syslog logf(configuration);

  try
    {  
      destin ds;
      ds = get_default_destin();
      board brd(boardname,configuration);
      int board = brd.get_brd_id();
      brd.set_msg(text,slot,ds);
      
      spool.spool_bul(G_mycall,zeit(),board,slot,text,false,ds,128);
    }
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Nicht moeglich Boarddatei zu oeffnen.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
#endif
}






defaults::defaults( void )
{
  try
    {
      dateiname = configuration.find("DEFAULTS");
      activ = true;
    }
  catch( Error_parameter_not_defined )
    {
      activ = false;
    }
}

void defaults::process( void )
{
  if (activ)
    {
      ifstream input(strtochar(dateiname));
      if (input)
	{
	  while( !input.eof() )
	    {
	      char puffer[200];
	      input.getline(puffer,199);
	      if ( *puffer != '\0' && *puffer != '#' )
		{
		  if (strcmp(puffer,"BOARD") == 0)
		    {
		      skyper_board_default def(input);
		      def.process();
		    }
		}
	    }
	}
      else
	activ = false;
    }
}
