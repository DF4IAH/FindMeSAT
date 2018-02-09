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

#include "connect_interface.h"

#include "connector.h"

extern connection_control connector;

connect_interface::connect_interface(String &outp, bool ax_flag, int i, connect_string& con_pf) : interfaces(outp,ax_flag)
{
  outp = "";
  id = i;
  interface_id = 'C';
  running = false;
  set_connect_path(con_pf);
  first_connect(outp);
}

connect_interface::connect_interface(String &outp, bool ax_flag, int i) : interfaces(outp,ax_flag)
{
  id = i;
  interface_id = 'C';
  running = false;
  path_finished = true;
}

bool connect_interface::do_process(bool rx_flag, String &outp)
{
  outp = "";
  if (!running)
    {
      connector.established(id);
      running = true;
    }
  if (rx_flag)
    {
      String inp;

      // Alle kompletten Zeilen uebermitteln
      while ( get_line(inp) )
	connector.received(id,inp,true);

      // Falls noch Restliche Zeichen ohne Zeilenendezeichen vorhanden
      // sind, werden diese auch noch uebermittelt.
      if (input_puffer.slen() != 0 )
	{
	  connector.received(id,input_puffer,false);
	  input_puffer = "";
	}
    }
  bool eol,fl;
  String puffer;
  while ( connector.to_send(id,puffer,eol,fl) )
    {
      outp.append(puffer);
      if (eol)
	outp.append(cr);
    }
  return !fl;
}

// Durch den Umbau auf die neue Interfacestruktur ist es zur Zeit nicht
// mehr moeglich, waerend des Connect-Aufbaus die Verbindung wieder
// abzubrechen. Dies muss noch geaendert werden.

connect_interface::~connect_interface( void )
{
  connector.disconnected(id);
}
