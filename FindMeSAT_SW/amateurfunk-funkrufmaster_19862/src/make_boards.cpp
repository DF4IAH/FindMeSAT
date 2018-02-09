/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
 *                                                                          *
 * Thist Program is free software; yopu can redistribute ist and/or modify  *
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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

#include "make_boards.h"
#include "board_data.h"

#include "config.h"
#include "board.h"
#include "logfile.h"

extern config_file configuration;

bool make_boards( void )
{
  int i = 0;
  syslog logf(configuration);
  try
    {
      while ( i < 100 &&  board_daten[i].board_id != 0 )
	{
	  board brd(board_daten[i].name,configuration,true,board_daten[i].board_id,board_daten[i].def_lifetime,board_daten[i].def_tx_intv);
	  i++;
	}
      return true;
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Boarddatei "+board_daten[i].name+" konnte nicht geoeffnet werden.",LOGMASK_PRGRMERR);
      return false;
    }
  catch (Error_could_not_create_boardfile )
    {
      logf.eintrag("Boarddatei "+board_daten[i].name+" konnte nicht erzeugt werden.",LOGMASK_PRGRMERR);
      return false;
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Das Board-Verzeichnis ist nicht definiert.",LOGMASK_PRGRMERR);
      return false;
    }
}
