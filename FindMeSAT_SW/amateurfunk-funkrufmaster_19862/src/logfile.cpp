/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
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
 ****************************************************************************/

#include <vector.h>

#include "logfile.h"

#include "globdef.h"
#include "system_info.h"
#include "fwd_frontend.h"

#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif
extern callsign_database calls;
#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif
extern callsign G_mycall;

logfile::logfile( void )
{}

String logfile::get_filename( zeit zt )
{
  time_t t = zt.get_systime();
  String tmp("log_");

  struct tm *ts = gmtime(&t);
  int year = ts->tm_year - 100; // Jahre ab 2000
  if (year < 10)
    tmp.append( (char) (year + 48));
  else
    tmp.append( (char) (year + 55));

  int mon = ts->tm_mon + 1; // Monat
  if (mon < 10)
    tmp.append( (char) (mon + 48));
  else
    tmp.append( (char) (mon + 55));

  int day = ts->tm_mday;
  if (day < 10)
    tmp.append( (char) (day + 48));
  else
    tmp.append( (char) (day + 55));

  return tmp;
}


vector<String> logfile::get_lines( int &cnt, int anz, String part, char period )
{
  return get_lines(cnt,anz,part,period,zeit());
}

vector<String> logfile::get_lines( int &cnt, int anz, String part, char period, zeit bis )
{
  if (anz > MAX_LOG_LINES)
    anz = MAX_LOG_LINES;
  if (anz <= 0)
    anz = 5;

  String *lines = new String [anz];
  vector<String> tmp_lines;
  int index = 0;
  cnt = 0;
  String ln;
  String my_ln;

  int zurueck; // Anzahl der Tage, die der Logauszug zurueckgehen soll

  switch(period)
    {
      case 'h' : zurueck = 0;  // Nur heute
                 break;
      case 'w' : zurueck = 6; // Die Woche (6 Tage + heute )
	         break;
      case 'm' : zurueck = 30; // Monat (31 Tage)
	         break;
      case 'q' : zurueck = 90; // Viertel Jahr (91 Tage)
	         break;
      case 'y' : zurueck = 364; // Jahr, 365 Tage
	         break;
       default : zurueck = 0;
    }

  for (int tage = zurueck; tage >= 0; --tage)
    {
      zeit logzeit = bis - (tage * 86400);

      ifstream logf(strtochar(logfilename+get_filename(logzeit)));
      part.kuerze();

      while (logf)
	{
	  ln.getline(logf,999);
	  if (part.slen() == 0 && ln.slen()>0)
	    {
	      lines[index++] = ln;	  
	      cnt++;
	      if (index >= anz)
		index = 0;
	    }
	  else  // so kompliziert, in der Hoffnung, 
	        //dass es schneller laeuft....
	    {
	      my_ln=ln;
	      my_ln.upcase();
	      part.upcase();
	    }
	  if (part.slen() != 0 && my_ln.in(part) )
	    {
	      lines[index++] = ln;
	      cnt++;
	      if (index >= anz)
		index = 0;
	    }
	}
    }
  if (cnt <= anz)
    for (index = 0; index < cnt; index++)
      tmp_lines.push_back(lines[index]);
  else
    for (int i = 0; i < anz; i++)
      {
	tmp_lines.push_back(lines[index++]);
	if (index >= anz)
	  index = 0;
      }
  delete [] lines;
  return tmp_lines;
  /*
  strm << cnt << " Eintraege gefunden.";
  if (cnt > anz)
    strm << " Es wurden die letzten " << anz << " Eintraege angezeigt." << cr;
  else
    strm << " Es wurden alle Eintraege angezeigt." << cr;
  strm << cr << cr;
  */ 
}

syslog::syslog( config_file & cfg )// Konstruktor
{
  logfilename = cfg.find("LOGDIR")+String("sys/");
  try
    {
      logmask = (unsigned int) (cfg.find("LOGMASK").Stoi());
    }
  catch(Error_parameter_not_defined)
    {
      try
	{
	  int loglevel    = atoi(strtochar(cfg.find("LOGLEVEL")));
	  if (loglevel == 0)
	    logmask = LOGLEVEL_0;
	  else if (loglevel == 1)
	    logmask = LOGLEVEL_1;
	  else if (loglevel == 2)
	    logmask = LOGLEVEL_2;
	  else
	    logmask = LOGLEVEL_3;
	}
      catch(Error_parameter_not_defined )
	{
	  cerr << "Parameter LOGMASK oder LOGLEVEL nicht definiert, Wert auf 0 gesetzt." << endl;
	} 
    }
  try
    {
      sysop_calls = cfg.find("SYSOPS");
      def_destin = cfg.find("DEFAULT_DESTINATION");
      pagemask = cfg.find("SYSOP_INFO_MASK").Stoi();
    }
  catch( Error_parameter_not_defined )
    {
      pagemask = 0;
    }
}

void syslog::page(const String &msg, unsigned int mask )
{
  //fwd_api fwd;
#ifdef COMPILE_SLAVES
  if ((mask & pagemask) != 0)
    {
      zeit t;
      t.set_darstellung(t.f_zeitdatum_s);
      String m = t.get_zeit_string() + String(' ') + msg;
      vector<String> sysops = komma_separeted(sysop_calls);
      for (vector<String>::iterator it = sysops.begin(); it != sysops.end() ; ++it)
	{
	  try
	    {
	      callsign call(*it);
	      destin ds(def_destin);
	      database_entry ent = calls.find(call);
	      adress adr = ent.get_adr();
	      spool.spool_msg(adr,m,ds,20);
	      //fwd.fwd_msg(G_mycall,adr,ds,50,m);
	    }
	  catch( Error_no_callsign )
	    {}
	  catch( Error_could_not_open_file )
	    {}
	  catch( Error_could_not_open_directory )
	    {}
	  catch( Error_callsign_does_not_exist )
	    {}
	  catch( Error_no_destin )
	    {}
	}
    }
#endif
}

String syslog::maskid( unsigned int mask )
{
  String tmp('#');
  if (mask & LOGMASK_SIGNALS )  tmp.append( LOGMASK_ID_SIGNALS );
  if (mask & LOGMASK_STARTSTOP) tmp.append( LOGMASK_ID_STARTSTOP );
  if (mask & LOGMASK_PRGRMERR)  tmp.append( LOGMASK_ID_PRGRMERR );
  if (mask & LOGMASK_VERBERR)   tmp.append( LOGMASK_ID_VERBERR );
  if (mask & LOGMASK_PRGRMMDG)  tmp.append( LOGMASK_ID_PRGRMMDG );
  if (mask & LOGMASK_LOGINOUT)  tmp.append( LOGMASK_ID_LOGINOUT );
  if (mask & LOGMASK_CNTBPRCS)  tmp.append( LOGMASK_ID_CNTBPRCS );
  if (mask & LOGMASK_PRIVVERL)  tmp.append( LOGMASK_ID_PRIVVERL );
  if (mask & LOGMASK_PWLOG)     tmp.append( LOGMASK_ID_PWLOG );
  if (mask & LOGMASK_DBAENDRNG) tmp.append( LOGMASK_ID_DBAENDRNG );
  if (mask & LOGMASK_USERINPUT) tmp.append( LOGMASK_ID_USERINPUT );

  return tmp;
}

unsigned int syslog::maskid( String id )
{
  unsigned int mask = 0;
  id.upcase();

  if (id.slen() > 0 && id[0] == '#')
    {
      for (unsigned int i = 1; i < id.slen(); i++ )
	{
	  if      (id[i] == LOGMASK_ID_SIGNALS  ) mask |= LOGMASK_SIGNALS;
	  else if (id[i] == LOGMASK_ID_STARTSTOP) mask |= LOGMASK_STARTSTOP;
	  else if (id[i] == LOGMASK_ID_PRGRMERR ) mask |= LOGMASK_PRGRMERR;
	  else if (id[i] == LOGMASK_ID_VERBERR  ) mask |= LOGMASK_VERBERR;
	  else if (id[i] == LOGMASK_ID_PRGRMMDG ) mask |= LOGMASK_PRGRMMDG;
	  else if (id[i] == LOGMASK_ID_LOGINOUT ) mask |= LOGMASK_LOGINOUT;
	  else if (id[i] == LOGMASK_ID_CNTBPRCS ) mask |= LOGMASK_CNTBPRCS;
	  else if (id[i] == LOGMASK_ID_PRIVVERL ) mask |= LOGMASK_PRIVVERL;
	  else if (id[i] == LOGMASK_ID_PWLOG    ) mask |= LOGMASK_PWLOG;
	  else if (id[i] == LOGMASK_ID_DBAENDRNG) mask |= LOGMASK_DBAENDRNG;
	  else if (id[i] == LOGMASK_ID_USERINPUT) mask |= LOGMASK_USERINPUT;
	  else throw Error_wrong_id_in_logmask();
	}
    }
  else
    throw Error_in_logmask_syntax();
  return mask;
}

String syslog::logmaskid( void )
{
  return maskid(logmask);
}

String syslog::pagemaskid( void )
{
  return maskid(pagemask);
}

unsigned int syslog::logmaskid( const String &id )
{
  logmask = maskid(id);
  return logmask;
}

unsigned int syslog::pagemaskid( const String &id )
{
  pagemask = maskid(id);
  return pagemask;
}


void syslog::eintrag( const callsign & usr, const String &msg, unsigned int mask )
{
  
  if ((mask & logmask) != 0)  
    {
      zeit t;
      callsign call = usr;
      call.set_format(true);
      t.set_darstellung(t.f_zeitdatum_s);
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      // Logeintrag besteht aus Zeitstempel, Rufzeichen des Benutzers und Nachricht
      logf << t << " " << maskid(mask) << ":" << call << " ";
      if (msg.slen() > 900)
	logf << msg.copy(0,900);
      else
	logf << msg;
      logf  << endl;
#ifdef _DEBUG_ELOGS_
      cerr << t << " " << maskid(mask) << ":" << call << " " << msg << endl;
#endif
      page(maskid(mask)+String(' ')+call.call()+String(' ')+msg,mask);
    }
}

void syslog::eintrag( const String &msg, unsigned int mask )
{
  
  if ((mask & logmask) != 0)  
    {
      zeit t;
      t.set_darstellung(t.f_zeitdatum_s);
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      // Logeintrag besteht aus Zeitstempel, Rufzeichen des Benutzers und Nachricht
      logf << t << " " << maskid(mask) << ":" << "SYSTEM" << "    ";
      if (msg.slen() > 900)
	logf << msg.copy(0,900);
      else
	logf << msg;
      logf  << endl;
#ifdef _DEBUG_ELOGS_
      cerr << t << " " << maskid(mask) << ":" << "SYSTEM" << "    " << msg << endl;
#endif
      page(maskid(mask)+String(' ')+String("SYSTEM    ")+msg,mask);
    }
  else
    {
    }
}


userlog::userlog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("user/");
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR nicht definiert." << endl;
    } 
}

void userlog::eintrag(zeit login, zeit logout, callsign usr, int in_bytes, int out_bytes, char con, char interf, unsigned int in_msg, unsigned int out_msg )
{
  login.set_darstellung(login.f_zeitdatum_s);
  //logout.set_darstellung(logout.f_zeit);
  usr.set_nossid(false);
  usr.set_format(true);
  ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
  delta_t dt( logout - login );
  logf << login << ", ";
  logf << dt << " #" << con << interf << " : " << usr;
  logf << "   ";
  PrintMem(logf,in_bytes);
  logf << "   ";
  PrintMem(logf,out_bytes);
  logf << "   ";
  PrintMem(logf,in_msg);
  logf << "   ";
  PrintMem(logf,out_msg);
  logf << endl;
}

iplog::iplog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("ip/");
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR nicht definiert." << endl;
    } 
}

void iplog::eintrag(uint32_t adr, uint16_t pt, t_tcp_ports port, String kommentar)
{
  zeit zt;
  zt.set_darstellung(zt.f_zeitdatum_s);
  ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
  logf << zt << " : ";
  for (int i = 0 ; i < 4 ; i++ )
    {
      logf << setw(3) << (adr & 0xff);
      if (i != 3) 
	logf << '.';
      adr >>= 8;
    }
  logf << "   ";
  logf << setw(5) << htons(pt) << "  ";

  switch(port)
    {
      case p_login : logf << "Telnet Login     ";
                     break;
      case p_fwd   : logf << "Fwd Verbindung   ";
	             break;
#ifdef COMPILE_HTTP
      case p_http  : logf << "HTTP Anforderung ";
                     break;
#endif
#ifdef COMPILE_SMTP
      case p_smtp  : logf << "SMTP Verbindung  ";
                     break;
#endif
    }
  if (kommentar.slen() > 900)
    logf << kommentar.copy(0,900) << endl;
  else
    logf << kommentar << endl;

}


httplog::httplog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("http/");
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR nicht definiert." << endl;
    } 
}

void httplog::eintrag(uint32_t adr, const String &url, const String &user_agent,
		      const String &host)
{
  zeit zt;
  zt.set_darstellung(zt.f_zeitdatum_s);
  ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
  logf << zt << " : ";
  //uint32_t adr = sa.sin_addr.s_addr;
  //uint16_t pt = sa.sin_port;
  for (int i = 0 ; i < 4 ; i++ )
    {
      logf << setw(3) << (adr & 0xff);
      if (i != 3) 
	logf << '.';
      adr >>= 8;
    }
  logf << "   ";
  //logf << setw(5) << pt << endl;
  logf << endl;
  logf << "   Host       : " << host << endl;
  logf << "   URL        : " << url << endl;
  logf << "   User-AGent : " << user_agent << endl;
  logf << endl;
}

rpclog::rpclog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("rpc/");
      logmask = (unsigned int) (cfg.find("RPCLOGMASK").Stoi());
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR oder RPCLOGMASK nicht definiert." << endl;
    } 
}

String rpclog::maskid( unsigned int mask )
{
  String tmp('#');
  if (mask & RPCLOGMASK_INIT       )  tmp.append( RPCLOGMASK_ID_INIT );
  if (mask & RPCLOGMASK_CONNECTION )  tmp.append( RPCLOGMASK_ID_CONNECTION );
  if (mask & RPCLOGMASK_ERROR      )  tmp.append( RPCLOGMASK_ID_ERROR );
  if (mask & RPCLOGMASK_DATEN      )  tmp.append( RPCLOGMASK_ID_DATEN );

  return tmp;
}

unsigned int rpclog::maskid( String id )
{
  unsigned int mask = 0;
  id.upcase();

  if (id.slen() > 0 && id[0] == '#')
    {
      for (unsigned int i = 1; i < id.slen(); i++ )
	{
	  if      (id[i] == RPCLOGMASK_ID_INIT       ) mask |= RPCLOGMASK_INIT;
	  else if (id[i] == RPCLOGMASK_ID_CONNECTION ) mask |= RPCLOGMASK_CONNECTION;
	  else if (id[i] == RPCLOGMASK_ID_ERROR      ) mask |= RPCLOGMASK_ERROR;
	  else if (id[i] == RPCLOGMASK_ID_DATEN      ) mask |= RPCLOGMASK_DATEN;
	  else throw Error_wrong_id_in_logmask();
	}
    }
  else
    throw Error_in_logmask_syntax();
  return mask;
}

String rpclog::logmaskid( void )
{
  return maskid(logmask);
}

unsigned int rpclog::logmaskid( const String &id )
{
  logmask = maskid(id);
  return logmask;
}


void rpclog::eintrag( const callsign & rpc, const String &msg, unsigned int mask )
{
  
  if ((mask & logmask) != 0)  
    {
      zeit t;
      callsign call = rpc;
      call.set_format(true);
      t.set_darstellung(t.f_zeitdatum_s);
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      // Logeintrag besteht aus Zeitstempel, Rufzeichen des Benutzers und Nachricht
      logf << t << " " << maskid(mask) << ":" << call << " ";
      if (msg.slen() > 900)
	logf << msg.copy(0,900);
      else
	logf << msg;
      logf << endl;
    }
}

fwdlog::fwdlog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("fwd/");
      logmask = (unsigned int) (cfg.find("FWDLOGMASK").Stoi());
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR oder FWDLOGMASK nicht definiert." << endl;
    } 
}

String fwdlog::maskid( unsigned int mask )
{
  String tmp('#');
  if (mask & FWDLOGMASK_INIT )       tmp.append( FWDLOGMASK_ID_INIT );
  if (mask & FWDLOGMASK_CONSTRTSTP ) tmp.append( FWDLOGMASK_ID_CONSTRTSTP );
  if (mask & FWDLOGMASK_IFDEBUG )    tmp.append( FWDLOGMASK_ID_IFDEBUG );
  if (mask & FWDLOGMASK_IFIO )       tmp.append( FWDLOGMASK_ID_IFIO );
  if (mask & FWDLOGMASK_FWDERR )     tmp.append( FWDLOGMASK_ID_FWDERR );
  if (mask & FWDLOGMASK_ROUTER )     tmp.append( FWDLOGMASK_ID_ROUTER );
  if (mask & FWDLOGMASK_RTNCHBR )    tmp.append( FWDLOGMASK_ID_RTNCHBR );
  if (mask & FWDLOGMASK_DESCR )      tmp.append( FWDLOGMASK_ID_DESCR );
  if (mask & FWDLOGMASK_RXMSG )      tmp.append( FWDLOGMASK_ID_RXMSG );
  if (mask & FWDLOGMASK_TIMEMES )    tmp.append( FWDLOGMASK_ID_TIMEMES);
  if (mask & FWDLOGMASK_AUTOROUTE )  tmp.append( FWDLOGMASK_ID_AUTOROUTE);

  return tmp;
}

unsigned int fwdlog::maskid( String id )
{
  unsigned int mask = 0;
  id.upcase();

  if (id.slen() > 0 && id[0] == '#')
    {
      for (unsigned int i = 1; i < id.slen(); i++ )
	{
	  if      (id[i] == FWDLOGMASK_ID_INIT       ) mask |= FWDLOGMASK_INIT;
	  else if (id[i] == FWDLOGMASK_ID_CONSTRTSTP ) mask |= FWDLOGMASK_CONSTRTSTP;
	  else if (id[i] == FWDLOGMASK_ID_IFDEBUG    ) mask |= FWDLOGMASK_IFDEBUG;
	  else if (id[i] == FWDLOGMASK_ID_IFIO       ) mask |= FWDLOGMASK_IFIO;
	  else if (id[i] == FWDLOGMASK_ID_FWDERR     ) mask |= FWDLOGMASK_FWDERR;
	  else if (id[i] == FWDLOGMASK_ID_ROUTER     ) mask |= FWDLOGMASK_ROUTER;
	  else if (id[i] == FWDLOGMASK_ID_RTNCHBR    ) mask |= FWDLOGMASK_RTNCHBR;
	  else if (id[i] == FWDLOGMASK_ID_DESCR      ) mask |= FWDLOGMASK_DESCR;
	  else if (id[i] == FWDLOGMASK_ID_RXMSG      ) mask |= FWDLOGMASK_RXMSG;
	  else if (id[i] == FWDLOGMASK_ID_TIMEMES    ) mask |= FWDLOGMASK_TIMEMES;
	  else if (id[i] == FWDLOGMASK_ID_AUTOROUTE  ) mask |= FWDLOGMASK_AUTOROUTE;
	  else throw Error_wrong_id_in_logmask();
	}
    }
  else
    throw Error_in_logmask_syntax();
  return mask;
}

String fwdlog::logmaskid( void )
{
  return maskid(logmask);
}

unsigned int fwdlog::logmaskid( const String &id )
{
  logmask = maskid(id);
  return logmask;
}

void fwdlog::eintrag( const callsign & partner, const String &msg, unsigned int mask )
{
  
  if ((mask & logmask) != 0)  
    {
      zeit t;
      callsign call = partner;
      call.set_format(true);
      call.set_nossid(false);
      t.set_darstellung(t.f_zeitdatum_s);
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      logf << t << " " << maskid(mask) << ":" << call << " ";
      if (msg.slen() > 900)
	logf << msg.copy(0,900);
      else
	logf << msg;
      logf << endl;
    }
}

void fwdlog::eintrag( const String &msg, unsigned int mask )
{
  
  if ((mask & logmask) != 0)  
    {
      zeit t;
      t.set_darstellung(t.f_zeitdatum_s);
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      logf << t << " " << maskid(mask) << ":          ";
      if (msg.slen() > 900)
	logf << msg.copy(0,900);
      else
	logf << msg;
      logf << endl;
    }
}

spoollog::spoollog( config_file & cfg )// Konstruktor
{
  try
    {
      logfilename = cfg.find("LOGDIR")+String("spool/");
      logmask = (unsigned int) (cfg.find("SPOOLLOGMASK").Stoi());
    }
  catch(Error_parameter_not_defined )
    {
      cerr << "Parameter LOGDIR oder FWDLOGMASK nicht definiert." << endl;
    } 
}

String spoollog::maskid( unsigned int mask )
{
  String tmp('#');
  if (mask & SPOOLLOGMASK_PERS  )    tmp.append( SPOOLLOGMASK_ID_PERS  );
  if (mask & SPOOLLOGMASK_BUL   )    tmp.append( SPOOLLOGMASK_ID_BUL   );
  if (mask & SPOOLLOGMASK_REP   )    tmp.append( SPOOLLOGMASK_ID_REP   );
  if (mask & SPOOLLOGMASK_NAMES )    tmp.append( SPOOLLOGMASK_ID_NAMES );

  return tmp;
}

unsigned int spoollog::maskid( String id )
{
  unsigned int mask = 0;
  id.upcase();

  if (id.slen() > 0 && id[0] == '#')
    {
      for (unsigned int i = 1; i < id.slen(); i++ )
	{
	  if      (id[i] == SPOOLLOGMASK_ID_PERS  ) mask |= SPOOLLOGMASK_PERS;
	  else if (id[i] == SPOOLLOGMASK_ID_BUL   ) mask |= SPOOLLOGMASK_BUL;
	  else if (id[i] == SPOOLLOGMASK_ID_REP   ) mask |= SPOOLLOGMASK_REP;
	  else if (id[i] == SPOOLLOGMASK_ID_NAMES ) mask |= SPOOLLOGMASK_NAMES;
	  else throw Error_wrong_id_in_logmask();
	}
    }
  else
    throw Error_in_logmask_syntax();
  return mask;
}

String spoollog::logmaskid( void )
{
  return maskid(logmask);
}

unsigned int spoollog::logmaskid( const String &id )
{
  logmask = maskid(id);
  return logmask;
}

void spoollog::eintrag(  const callsign &absender, const callsign &master, zeit absenderzeit, const destin &ds, const String &text, unsigned int prio, unsigned int mask )
{
  if ( (mask & logmask) != 0)
    {
      //Logzeit
      zeit t;
      t.set_darstellung(t.f_zeitdatum_s);
      
      //Kopie der absenderzeit anlegen
      zeit t_abs = absenderzeit;
      t_abs.set_darstellung(t.f_zeitdatum_s);
      
      // Kopie des Absenders anlegen
      callsign c_abs = absender;
      c_abs.set_format(true);
      c_abs.set_nossid(false);
      
      // Kopie des Masters anlegen
      callsign c_m = master;
      c_m.set_format(true);
      c_m.set_nossid(false);
      
      // Zielgebiet in String kopieren
      String ds_string;
      try
	{
	  ds_string = ds.get_string();
	  if (ds_string.slen() > 15)
	    ds_string = ds_string.copy(0,12) + String("...");
	}
      catch( Error_destin_checksum_error )
	{
	  ds_string = "corrupted dest!";
	}
      ofstream logf(strtochar(logfilename+get_filename(zeit())), ios::app );
      logf <<  t << " " << maskid(mask) << " " << c_abs << " " << c_m << " ";
      logf << t_abs << " ";
      //logf.set(ios::right);  
      logf << setw(15) << ds_string << " " << setw(3) << prio;
      logf << " :  ";
      if (text.slen() > 900)
	logf << text.copy(0,900);
      else
	logf << text;
      logf << endl;
    } 
}
