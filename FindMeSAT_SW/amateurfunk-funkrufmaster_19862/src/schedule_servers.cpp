/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2001-2004 by Holger Flemming                               *
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
 *  		                                			    *
 ****************************************************************************/


#include "schedule.h"

extern config_file configuration;

#ifdef COMPILE_TCP
void schedule::start_tcp_server( void )
{
  try
    {
      if (configuration.find("TCP_CONNECT") == "JA")
	{
	  int port = get_portnummer(p_login);
	  server_tcp = new tcp_server(port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter TCP_CONNECT nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("IP-Adresse oder Port in Benutzung",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte TCP-Socket nicht an Adresse binden.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Konnte TCP-Socket nicht auf 'Listen' setzen.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}

void schedule::start_tcp_fwd_server( void )
{
  try
    {
      if (configuration.find("TCP_FWD") == "JA")
	{
	  int port = get_portnummer(p_fwd);
	  server_tcp_fwd = new tcp_server(port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter TCP_FWD nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Fwd-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("IP-Adresse oder Port fuer Fwd in Benutzung",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Fwd-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte FWD-TCP-Socket nicht an Adresse binden.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Fwd-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Konnte FWD-TCP-Socket nicht auf 'Listen' setzen.",LOGMASK_PRGRMERR);
      logf.eintrag("TCP-Fwd-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}

#ifdef COMPILE_SMTP
void schedule::start_tcp_smtp_server( void )
{
  try
    {
      if (configuration.find("SMTP_SERVER") == "JA")
	{
	  int port = get_portnummer(p_smtp);
	  server_tcp_smtp = new tcp_server(port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter SMTP_SERVER nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("SMTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("IP-Adresse oder Port fuer SMTP in Benutzung",LOGMASK_PRGRMERR);
      logf.eintrag("SMTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte SMTP-TCP-Socket nicht an Adresse binden.",LOGMASK_PRGRMERR);
       logf.eintrag("SMTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Konnte SMTP-TCP-Socket nicht auf 'Listen' setzen.",LOGMASK_PRGRMERR);
      logf.eintrag("SMTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}
#endif

#ifdef COMPILE_HTTP
void schedule::start_tcp_http_server( void )
{
  try
    {
      if (configuration.find("HTTP_SERVER") == "JA")
	{
	  int port = get_portnummer(p_http);
	  server_tcp_http = new tcp_server(port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter HTTP_SERVER nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("HTTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("IP-Adresse oder Port fuer HTTP in Benutzung",LOGMASK_PRGRMERR);
      logf.eintrag("HTTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Konnte HTTP-TCP-Socket nicht an Adresse binden.",LOGMASK_PRGRMERR);
      logf.eintrag("HTTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Konnte HTTP-TCP-Socket nicht auf 'Listen' setzen.",LOGMASK_PRGRMERR);
      logf.eintrag("HTTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}
#endif // COMPILE_HTTP
#endif // COMPILE_TCP
#ifdef COMPILE_AX25

void schedule::start_ax25_server( void )
{
  try
    {
      if (configuration.find("AX25_CONNECT") == "JA")
	{
	  String port = configuration.find("AX25_PORT");
	  server_ax25 = new ax25_server(mycall,port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter AX25_CONNECT oder AX25_PORT nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("AX25-Port konnte nicht ermittelt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_ax25_master_call )
    {
      logf.eintrag("AX25-Rufzeichen konnte nicht in Systemformat uebersetzt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_gen_socket )
    {
      logf.eintrag("Kann AX25-Socket nicht erzeugen",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("Fehler: AX25-Adresse ist bereits in Benutzung.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Fehler: AX25-Adresse konnte nicht an Socket gebungen werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Fehler: AX25-Socket konnte nicht auf 'Listen' gesetzt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}

#ifdef COMPILE_AXHTP
void schedule::start_axhtp_server( void )
{
  try
    {
      if (configuration.find("AXHTP_SERVER") == "JA")
	{
	  String port = configuration.find("AXHTP_PORT");
	  String axhtp_call_str = configuration.find("AXHTP_CALL");
	  callsign axhtp_call(axhtp_call_str);
	  server_axhtp = new ax25_server(axhtp_call,port);
	}
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter AXHTP_SERVER, AXHTP_PORT oder AXHTP_CALL nicht definiert.",LOGMASK_PRGRMERR);
      logf.eintrag("AXHTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_no_callsign )
    {
      logf.eintrag("Parameter AXHTP_CALL enthaelt kein gueltiges Rufzeichen.",LOGMASK_PRGRMERR);
      logf.eintrag("AXHTP-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_ax25_config_get_addr )
    {
      logf.eintrag("AX25-Port konnte nicht ermittelt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_ax25_master_call )
    {
      logf.eintrag("AX25-Rufzeichen konnte nicht in Systemformat uebersetzt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_gen_socket )
    {
      logf.eintrag("Kann AX25-Socket nicht erzeugen",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_address_in_use )
    {
      logf.eintrag("Fehler: AX25-Adresse ist bereits in Benutzung.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch( Error_could_not_bind_socket )
    {
      logf.eintrag("Fehler: AX25-Adresse konnte nicht an Socket gebungen werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
  catch(Error_could_not_set_to_listen )
    {
      logf.eintrag("Fehler: AX25-Socket konnte nicht auf 'Listen' gesetzt werden.",LOGMASK_PRGRMERR);
      logf.eintrag("AX25-Server nicht gestartet.",LOGMASK_PRGRMMDG);
    }
}
#endif // COMPILE_AXHTP
#endif // COMPILE_AX25
