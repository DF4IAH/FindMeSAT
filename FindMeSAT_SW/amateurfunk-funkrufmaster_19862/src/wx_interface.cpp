/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
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

#include <ctype.h>

#include "wx_interface.h"
#include "globdef.h"

extern wx_control wx;

/*
  ----------------------------------------------------------------------------
  Abschliessend folgen die Methoden der Klasse wx_interface
  ----------------------------------------------------------------------------
*/

bool wx_interface::check_tag( const String &line, const String &tag )
{
  return line.in(tag);
}

String wx_interface::get_part(const String &line, const String &tag )
{
  String rest;
  String val;
  unsigned int i = 0;
  int start;

  // Anfang des Tag-String in der Zeile ermitteln.
  start = line.pos(tag);
  val = String("");
  try
    {
      if (start >= 0)
	{
	  // Rest der Zeile nach dem Tagstring kopieren
	  rest=line.copy(start + tag.slen(),(line.slen()- start - tag.slen()));
	  // Bis zur naechsten Ziffer vorgehen
	  while(i < rest.slen() && !isdigit(rest[i])) 
	    i++;
	  // Wenn das Zeichen unmittelbar vor der ersten Ziffer ein '-' war
	  // wird dieses auch noch mitgenommen.
	  if ( i-1 >= 0 && i-i < rest.slen() && rest[i-1] == '-')
	    i--;
	  // Nun alle Zeichen, die keine Leerzeichen sind, benutzen
	  while(i < rest.slen() && !isspace(rest[i])) 
	    { 
	      val.append(String(rest[i])); 
	      i++; 
	    }
	}
    }
  catch( Error_String_index_out_of_range )
    {
#ifdef _DEBUG_ELOGS_
      cerr << "In wx_interface::get_part wurde ein String-Index ueberschritten." << endl;
#endif
    }
  return val;
}

/* wx_line untersucht eine Zeile die von der WX-Station empfangen wurde. 
   Handelt es sich um eine WX-Meldung, werden die Daten daraus 
   extrahiert und ein entsprechender Funkruf generiert.
*/
void wx_interface::wx_line( String line )
{
  last_activity = zeit();
  if ( maske & MASKE_TEMPERATUR )
    {
      if (check_tag(line,wx_cfg.temperatur))
	{
	  String val = get_part(line,wx_cfg.temperatur);
	  mdg.temp = (float) val.Stod();
	  maske &= ~MASKE_TEMPERATUR;
	}
    }

  if ( maske & MASKE_LUFTDRUCK )
    {
      if (check_tag(line,wx_cfg.luftdruck))
	{
	  String val = get_part(line,wx_cfg.luftdruck);
	  mdg.druck = (float) val.Stod();
	  maske &= ~MASKE_LUFTDRUCK;
	}
    }

  if ( maske & MASKE_LUFTFEUCHTE )
    {
      if (check_tag(line,wx_cfg.luftfeuchte))
	{
	  String val = get_part(line,wx_cfg.luftfeuchte);
	  mdg.feucht = (float) val.Stod();
	  maske &= ~MASKE_LUFTFEUCHTE;
	}
    }

  if ( maske & MASKE_WINDRICHTUNG )
    {
      if (check_tag(line,wx_cfg.windrichtung))
	{
	  String val = get_part(line,wx_cfg.windrichtung);
	  mdg.wind_dir = val.Stoi();
	  maske &= ~MASKE_WINDRICHTUNG;
	}
    }

  if ( maske & MASKE_WINDSPEED )
    {
      if (check_tag(line,wx_cfg.windgeschwindigkeit))
	{
	  String val = get_part(line,wx_cfg.windgeschwindigkeit);
	  mdg.wind_speed = val.Stoi();
	  maske &= ~MASKE_WINDSPEED;
	}
    }

  if ( maske & MASKE_BOEN )
    {
      if (check_tag(line,wx_cfg.boen))
	{
	  String val = get_part(line,wx_cfg.boen);
	  mdg.boen = val.Stoi();
	  maske &= ~MASKE_BOEN;
	}
    }

  if ( maske & MASKE_RAIN1 )
    {
      if (check_tag(line,wx_cfg.niederschlag1))
	{
	  String val = get_part(line,wx_cfg.niederschlag1);
	  mdg.rain_24 = (float) val.Stod();
	  maske &= ~MASKE_RAIN1;
	}
    }

  if ( maske & MASKE_RAIN4 )
    {
      if (check_tag(line,wx_cfg.niederschlag4))
	{
	  String val = get_part(line,wx_cfg.niederschlag4);
	  mdg.rain_4 = (float) val.Stod();
	  maske &= ~MASKE_RAIN4;
	}
    }

  if ( maske & MASKE_RAINM )
    {
      if (check_tag(line,wx_cfg.niederschlag_m))
	{
	  String val = get_part(line,wx_cfg.niederschlag_m);
	  mdg.rain_mn8 = (float) val.Stod();
	  maske &= ~MASKE_RAINM;
	}
    }
  if ( check_tag(line,wx_cfg.prompt) )
    {
//      maske = 0;
//  Wenn hier die Maske auf 0 gesetzt wird, dann DARF zwischendurch nicht der
//  Prompt auftauchen. Das ist aber problematisch. Daher ohne Check hier.

// Und was passiert jetzt, wenn die Wetterstation nicht alle DAten liefert,
// die KOnfiguriert wurden?
    }

}

/* Im Konstruktor werden alle benoetigten Variablen initialisiert und der erste
   Connect-Schritt zum Cluster durchgefuehrt.
*/
void wx_interface::set_maske( void )
{
  maske = MASKE_ALL;
  
  if ( wx_cfg.temperatur.slen() == 0 )
    maske &= ~MASKE_TEMPERATUR;
  if ( wx_cfg.luftdruck.slen() == 0 )
    maske &= ~MASKE_LUFTDRUCK;
  if ( wx_cfg.luftfeuchte.slen() == 0 )
    maske &= ~MASKE_LUFTFEUCHTE;
  if ( wx_cfg.windrichtung.slen() == 0 )
    maske &= ~MASKE_WINDRICHTUNG;
  if ( wx_cfg.windgeschwindigkeit.slen() == 0 )
    maske &= ~MASKE_WINDSPEED;
  if ( wx_cfg.boen.slen() == 0 )
    maske &= ~MASKE_BOEN;
  if ( wx_cfg.niederschlag1.slen() == 0 )
    maske &= ~MASKE_RAIN1;
  if ( wx_cfg.niederschlag4.slen() == 0 )
    maske &= ~MASKE_RAIN4;
  if ( wx_cfg.niederschlag_m.slen() == 0 )
    maske &= ~MASKE_RAINM;
}

wx_interface::wx_interface(String &outp, bool ax_flag, wx_config_file &wx_confg) : interfaces(outp,ax_flag)
{
  wx_cfg = wx_confg;  // Daten in klasseninterner Variablen speichern
  set_maske();
  set_connect_path(wx_cfg.pfad);
  first_connect(outp);
  interface_id = 'W';
  last_activity = zeit();

  mdg.call = wx_cfg.station;
  mdg.loc = wx_cfg.loc;

}

// Der Destruktor bleibt leer
wx_interface::~wx_interface( void )
{
}


bool wx_interface::do_process( bool rx_flag, String &outp )
{
  outp = "";
  if (rx_flag)
    {
      String inp;
      while (get_line(inp))
	{
	  // Steht in der Zeile "reconnected" ? Dann wird der Prozess beendet 	  
	  if (inp.in("*** reconnected") || inp.in("Reconnected to"))
	    return false;
	  else
	    {
	      if (connect_prompt) 
		{
		  if (maske != 0)
		    wx_line(inp);
		  
		  if (maske == 0)
		    {
		      wx.meldung(wx_cfg.slot,mdg);
		      in_msg++;
		      return false;
		    }
		}
	      else 
		{
		  if(inp.in(wx_cfg.prompt))    // Prompt der WX-Station erkannt?
		    {
		      outp.append(wx_cfg.command+char(13));  // Befehl senden
		      last_activity = zeit();
 		      connect_prompt=true;
		    }
		}
	    }
	}
    }
  return true;
}

