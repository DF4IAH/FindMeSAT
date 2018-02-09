/****************************************************************************
 *                                                                          *
 * Userinterface for the ham-radio POCSAG-Server                            *
 * Copyright (C) 2002 by Holger Flemming                                    *
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

#include "makros.h"
#include "logfile.h"
#include "system_info.h"

extern config_file configuration;
extern callsign_database calls;


String makros::get_linux_version( void )
{
  ifstream proc("/proc/version");
  if (proc)
    {
      String line;
      line.getline(proc,255);
      unsigned int i = 0;
      while (i < line.slen() && !isdigit(line[i]))
	i++;
      while (i < line.slen() && line[i] != ' ')
	i++;
      return line.copy(0,i);
    }
  else
    return String("");
}
 
makros::makros( user u )
{
  usr = u;
}

void makros::set_user( const user &u )
{
  usr = u;
}

String makros::line( String & ln )
{
  system_info sysinf;

  syslog slog(configuration);
  unsigned int beg_index = 0;
  unsigned int lauf_index = 0;
  unsigned int laenge = ln.slen();
  unsigned int stop = 0;
  String tmp("");
  String tmp2;
  ifstream infile;
  char file[200];
  String filename;
  struct stat stat_buffer;
  while (lauf_index < laenge)
    {
      if (ln[lauf_index] == '%')
	{
	  tmp2 = ln.copy(beg_index,lauf_index-beg_index);
	  tmp.append(tmp2);
	  char makro_zeichen = ln[++lauf_index];
	  beg_index = ++lauf_index;
	  zeit t;
	  zeit linux_st = linux_start();
	  delta_t dt;
	  int anz;
	  long int mem;
	  switch(makro_zeichen)
	    {
  /* String nach '%' durchsucht und gegebenenfalls ergaenzt.    */
  /*     '%'  wieder durch ein %         (also %% im Text)      */
  /*     '%d' durch das aktuelle Datum (dd.mm.yy).              */
  /*     '%D' Letzte Aenderung einer Datei "%D/tmp/test.txt"    */
  /*     '%f' Text aus filenamen einlesen  "%f/tmp/test.txt "   */
  /*     '%t' durch die aktuelle Uhrzeit (HH:MM).               */
  /*     '%u' Uptime des Funkruf-Benutzerinterfaces             */
  /*     '%U' Uptime des Linux-Systems                          */
  /*     '%m' MyCall des Servers mit SSID                       */
  /*     '%M' Speicherbedarf des Masters                        */
  /*     '%N' Name des Users ausgehend von Call Ohne SSID       */
  /*     '%P' Name des Users ausgehend von Call MIT SSID        */
  /*     '%n' Anzahl der User in der Datenbank                  */
  /*     '%V' Linux Version                                     */
  /*     '%v' Pager Version                                     */
  /*     '%c' User-Call mit SSID                                */
  /*     '%C' User-Call ohne SSID                               */

 	      case '%' : tmp.append('%');
		         break;
  	      case 'd' : t.set_darstellung (zeit::f_datum_s);
		         tmp.append(t.get_zeit_string());
			 break;
			 
	      case 'D' : stop=lauf_index; 
		         while ( stop < laenge && !isspace(ln[stop]) ) 
			   stop++;
			 filename = ln.copy(lauf_index, stop-lauf_index);
			 stat(strtochar(filename),&stat_buffer);
			 t = zeit(stat_buffer.st_mtime);
			 t.set_darstellung (zeit::f_datum_s);
			 tmp.append(t.get_zeit_string());
			 lauf_index=stop+1;
			 beg_index=stop+1;
			 break;
			 

  	      case 'f':  stop=lauf_index; 
		         while ( stop < laenge && !isspace(ln[stop]) ) 
			   stop++;
			 strcpy( file, strtochar( ln.copy(lauf_index, stop-lauf_index) ) );
			 infile.open(file);
			 if (infile)
			   {
			     infile.getline(file, 80);
			     tmp.append(file);
			     infile.close();
			   }
	    	         else
			   slog.eintrag(String("Konnte Filemakro ")+file+String("nicht ersetzen"),LOGMASK_PRGRMERR);
			 lauf_index=stop+1;
			 beg_index=stop+1;
                         break;

	      case 't' : t.set_darstellung (zeit::f_zeit_s);
			 tmp.append(t.get_zeit_string());
			 break;
	      case 'U' : dt = delta_t(t-linux_st);
			 tmp.append(dt.get_string());
			 break;
	      case 'u' : dt = delta_t(t-start_time);
			 tmp.append(dt.get_string());
			 break;
	      case 'm' : tmp.append(configuration.find("MYCALL"));
		         break;
	      case 'M' : mem = sysinf.get_pmem_size();
		         tmp.append(MemtoS(mem));
		         break;
	      case 'N' : try
		           {
			     database_entry ent = calls.find(usr.user_call().get_nossidcall());
			     String name = ent.get_name();
			     tmp.append(name);
			   }
	                 catch( Error_callsign_does_not_exist )
			   {
			     tmp.append(String("  "));
			   }
	                 break;
	      case 'P' : try
		           {
			     database_entry ent = calls.find(usr.user_call());
			     String name = ent.get_name();
			     tmp.append(name);
			   }
	                 catch( Error_callsign_does_not_exist )
			   {
			     tmp.append(String("  "));
			   }
	                 break;
	      case 'n' : anz = calls.get_size();
		         char ctmp[20];
			 sprintf(ctmp,"%5d",anz);
		         tmp.append(String(ctmp));
			 break;
	      case 'c' : tmp.append(usr.user_call().call());
			 break;
	      case 'C' : tmp.append(usr.user_call().get_nossidcall().str());
			 break;
	      case 'v' : tmp.append(String(VERSION));
			 break;
	      case 'V' : tmp.append(get_linux_version());
		         break;
	    }
	}
      else
	lauf_index++;
    }
  if (beg_index < laenge)
    {
      tmp2 = ln.copy(beg_index,laenge-beg_index);
      tmp.append(tmp2);
    }
  return tmp;
}
