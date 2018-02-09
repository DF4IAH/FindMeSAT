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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "slaves.h"
#include "config.h"
#include "logfile.h"
#include "spoolfiles.h"

extern config_file configuration;
extern spoolfiles spool;

slave::slave( istream &strm, const String &pfd , const callsign &c, t_baken &baken)
{
  bool flag = false;
  scheiter_zaehler = 0;
  b_num = 0;
  call = c;
  pfad = pfd+call.str()+String('/');
  my_baken.clear();
  diff = 0.;

  char puffer[255];
  // Jede Zeile einzeln holen
  while (!flag && strm.getline(puffer,254))
    // Kommentarzeilen nicht weiter bearbeiten
    if (*puffer != '#')
      {
	istringstream input(puffer);
	
	char parameter[60];
	char value[200];
	char ch;
	
	// Einen Input-String-Stream aus der Zeile erzeugen
	// und parameter und Value daraus extrahieren
	input.get(parameter,59,'=');
	String Par(parameter);
	if (Par == String("ENDE"))
	  flag = true;
	else if (Par == String("POCBAKE"))
	  {
	    tx_bake *bptr = new tx_bake(strm,b_num++,pfad);
	    baken.push_back(bptr);
	    my_baken.push_back(bptr);
	  }
	else
	  {
	    try
	      {
		input >> ch;
		if (ch == '=')
		  {
		    input.get(value,199);
		    String Val(value);  
		    Val.kuerze();
		    if (Par == String("PFAD"))
		      {
			if (Val != "")
			  connect_pfad = connect_string(Val);
		      }
		    else if (Par == String("SLOTS"))
		      slots = Val;
		    else if (Par == String("BAKE"))
		      bake = Val;
		    else if (Par == String("PASSWORD"))
		      passwd = Val;
		    else if (Par == String("DESTINATIONS"))
		      dests = own_destins(Val);
		    else if (Par == String("STATUS"))
		      if (Val == String("DISABLED"))
			c_status = sts_disabled;
		      else if (Val == String("SPOOL"))
			c_status = sts_spool;
		      else
			c_status = sts_getrennt;
		    else if (Par == String("MODE"))
		      if (Val == String("PASSIV"))
			mode = m_passiv;
		      else if (Val == String("AKTIV") || Val == String("ACTIV") )
			mode = m_activ;
		      else
			{
			  throw Error_wrong_parameter_name_in_slave_conf(Par);
			}
		    else
		      {
			throw Error_wrong_parameter_name_in_slave_conf(puffer);
		      }
		  }
		else
		  {
		    throw Error_wrong_file_format_in_slave_conf(puffer);
		  }
	      }
	    catch( Error_syntax_fehler_in_connect_string )
	      {
		throw Error_wrong_file_format_in_slave_conf(puffer);
	      }
	    catch( Error_no_destin )
	      {
		throw Error_wrong_file_format_in_slave_conf(puffer);
	      }

	  }
      }
  if (c_status == sts_spool || c_status == sts_getrennt )
    spool.add_dir(pfad,dests);
}

slave::slave(const callsign &ca, const String &pfd, const connect_string &con_pfad, const String &slts, const String &b, slave_mode mo, const own_destins& ds )
{
  scheiter_zaehler = 0;
  call = ca;
  pfad = pfd;
  slots = slts;
  bake = b;
  connect_pfad = con_pfad;
  c_status = sts_disabled;
  last_status_change = zeit();
  my_baken.clear();
  mode = mo;
  passwd = String("");
  dests = ds;
}

void slave::PrintOn( ostream &strm )
{
  call.set_format(false);
  strm << "SLAVE=" << call << endl;
  strm << "PFAD=" << connect_pfad << endl;
  strm << "SLOTS=" << slots << endl;
  strm << "BAKE=" << bake << endl;
  strm << "PASSWORD=" << passwd << endl;
  strm << "DESTINATIONS=" << dests << endl;
  strm << "STATUS=";
  switch(c_status)
    {
      case sts_disabled            : 
      case sts_trennen_uz_disablen :
      case sts_disablen            : strm << "DISABLED";
                                     break;
      case sts_spool               : 
      case sts_trennen_uz_spoolen  : 
      case sts_spoolen             : strm << "SPOOL";
                                     break;
      default                      : strm << "AKTIV";
    }
  strm << endl;
  strm << "MODE=";
  switch( mode )
    {
      case m_passiv : strm << "PASSIV";
                      break;
      case m_activ  : strm << "AKTIV";
                      break;
    }
  strm << endl;
  for (t_baken::iterator it = my_baken.begin(); it != my_baken.end(); ++it)
    {
      (*(*it)).PrintOn(strm);
    }
  strm << "ENDE" << endl;
}

bool slave::add_pocbake( const String &text, int slot, t_baken &baken )
{
  if (slot <= 10)
    slot += 32;

  for (t_baken::iterator it = my_baken.begin(); it != my_baken.end(); ++it)
    if ((*it)->get_typ() == bake::b_tx)
      {
	tx_bake *bptr = (tx_bake*) (*it);
	if ( bptr->get_slot() == slot )
	  return false;
      }

  tx_bake *baken_ptr = new tx_bake(slot,b_num++,pfad,text);
  baken.push_back(baken_ptr);
  my_baken.push_back(baken_ptr);
  return true;
}

bool slave::del_pocbake( int slot, t_baken &baken )
{
  if (slot <= 10)
    slot += 32;

  for (t_baken::iterator it = my_baken.begin(); it != my_baken.end(); ++it)
    if ((*it)->get_typ() == bake::b_tx)
      {
	tx_bake *bptr = (tx_bake*) (*it);
	if ( bptr->get_slot() == slot )
	  {
	    my_baken.erase(it);

	    for (it = baken.begin(); it != baken.end(); ++it )
	      if (bptr == (tx_bake*) (*it))
		{
		  baken.erase(it);
		  break;
		}

	    delete bptr;
	    return true;
	  }
      }
  return false;
}

bool slave::check_pocbake( int slot )
{
  if (slot <= 10)
    slot += 32;

  for (t_baken::iterator it = my_baken.begin(); it != my_baken.end(); ++it)
    if ((*it)->get_typ() == bake::b_tx)
      {
	tx_bake *bptr = (tx_bake*) (*it);
	if ( bptr->get_slot() == slot )
	  return true;
      }

  return false;
}

slave_control::slave_control( void )
{
  slaves.clear();
  geaendert = false;
}

slave_control::~slave_control(void )
{
  save();
  slaves.clear();
}

bool slave_control::load( t_baken &baken )
{
  char filename[255];
  geaendert = false;


  try
    {
      basis_pfad = configuration.find("SPOOLDIR");
    }
  catch( Error_parameter_not_defined )
    {
      throw Error_spooldir_undefined();
    }
  char *path = getenv("FUNKRUF");  // Umgebungsvariable "FUNKRUF" abfragen
  if (path == NULL)                // Vorhanden ? 
    {
      // Nein ! Konfigurationsfile steht dann unter /etc"
      strcpy(filename,"/etc/slave.conf");
    }
  else
    {
      // ja ! Konfigurationsfile steht dann unter 
      // $FUNKRUF/etc
      strcpy(filename,path);
      strcat(filename,"etc/slave.conf");
    }


  ifstream in(filename);
  if (in)
    {
      char puffer[255];
      // Jede Zeile einzeln holen
      while (in.getline(puffer,254))
	// Kommentarzeilen nicht weiter bearbeiten
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
		

		if (Par == String("SLAVE"))
		  {
		    try
		      {
			slave slave(in,basis_pfad,callsign(Val),baken);
			slaves.push_back(slave);
		
		      }
		    catch( Error_no_callsign )
		      {
			throw Error_wrong_slave_callsign(Val);
		      }
		  }
		else
		  throw Error_slavedef_starts_with_wrong_parameter(Par);
	      }
	    else
	      throw Error_wrong_file_format_in_slave_conf(puffer);
	  }
      return true;
    }
  else
    return false;
}

bool slave_control::save( void )
{
  char filename[255];

  if (geaendert)
    {
      char *path = getenv("FUNKRUF");  // Umgebungsvariable "FUNKRUF" abfragen
      if (path == NULL)                // Vorhanden ? 
	{
	  // Nein ! Konfigurationsfile steht dann unter /etc"
	  strcpy(filename,"/etc/slave.conf");
	}
      else
	{
	  // ja ! Konfigurationsfile steht dann unter 
	  // $FUNKRUF/etc
	  strcpy(filename,path);
	  strcat(filename,"etc/slave.conf");
	}
      
      
      ofstream out(filename);
      if (out)
	{
	  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
	    {
	      it->PrintOn(out);
	      out << '#' << endl;
	      out << '#' << endl;
	    }
	  geaendert = false;
	  return true;
	}
      else
	return false;
    }
  else
    return false;
}

vector<slave> slave_control::get_slave( const callsign &call )
{
  vector<slave> tmp;

  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    if (samecall(call,it->get_call()))
    tmp.push_back(*it);

  return tmp;
}

vector<slave> slave_control::get_slave( void )
{
  vector<slave> tmp;
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    tmp.push_back(*it);

  return tmp;
}

void slave_control::set_status( const callsign& call, slave::slave_conn_status stat )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  if ( stat == slave::sts_getrennt || stat == slave::sts_gescheitert )
	    {
	      slave::slave_conn_status old_stat = it->status();
	      if ( old_stat == slave::sts_trennen_uz_disablen || 
		   old_stat == slave::sts_disablen )
	   	{
		  it->status(slave::sts_disabled);
		  String pfad = it->get_pfad();
		  spool.del_dir(pfad);
		}
	      else if (old_stat == slave::sts_trennen_uz_spoolen ||
		       old_stat == slave::sts_spoolen)
	    	it->status(slave::sts_spool);
	      // Wenn die Verbindung bereits getrennt wird, ohne dass sie
	      // vorher aktiv wurde, kann von einem Scheitern ausgegangen 
	      // werden.
	      else if (stat == slave::sts_getrennt &&
		       old_stat == slave::sts_aufbau )
		it->status(slave::sts_gescheitert);
	      else
		it->status(stat);
	    }
	  else
	    it->status(stat);
	}
    }
}

bool slave_control::set_slots( const callsign &call, const String &sl )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_slots(sl);
	  geaendert = true;
	  return true;
	}
    }
  return false;
}

bool slave_control::set_bake(const callsign &call, const String &bake )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_bake(bake);
	  geaendert = true;
	  return true;
	}
    }
  return false;
}

bool slave_control::set_diff(const callsign &call, double diff )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  if (diff>0x7FFF) // Diff immer wird positiv angegeben
	    diff-=0xFFFF;  // Kann aber natuerlich auch negativ sein.
	  diff/=10; // diff wird in 1/10 Sekunden angegeben.
	  it->set_diff(diff);
	  return true;
	}
    }
  return false;
}

bool slave_control::set_version(const callsign &call, const String &version )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_version(version);
	  return true;
	}
    }
  return false;
}

bool slave_control::set_con_pfad(const callsign &call, const connect_string &cs )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_connect_pfad(cs);
	  geaendert = true;
	  return true;
	}
    }
  return false;
}

bool slave_control::set_passwd(const callsign &call, const String &pw )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_passwd(pw);
	  geaendert = true;
	  return true;
	}
    }
  return false;
}

bool slave_control::set_destinations(const callsign &call, const own_destins &d )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->set_destinations(d);
	  geaendert = true;
	  return true;
	}
    }
  return false;
}

int slave_control::wait_time(list<slave>::iterator it  )
{
  int cnt = it->n_gescheitert();

  if (cnt == 0) 
    return 0;
  else if (cnt < 3)
    return WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 5)
    return 5 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 7)
    return 15 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 11)
    return 60 * WAIT_UNTIL_CONN_RETRY;
  else if (cnt < 20)
    return 120 * WAIT_UNTIL_CONN_RETRY;
  else 
    return 1440 * WAIT_UNTIL_CONN_RETRY;
}


bool slave_control::start_connection( callsign &call, String &pfad, connect_string &con_pfad, String &slots, String &bake, slave::slave_mode&m , String &pw)
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (it->status() == slave::sts_getrennt ||
	  ( it->status() == slave::sts_gescheitert && 
	    zeit() - it->last_change() > wait_time(it) ))
	{
	  call = it->get_call();
	  pfad = it->get_pfad();
	  con_pfad = it->get_connect_pfad();
	  slots = it->get_slots();
	  bake = it->get_bake();
	  m = it->get_mode();
	  pw = it->get_passwd();
	  return true;
	}
    }
  return false;
}

bool slave_control::stop_connection( const callsign &call )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  if (it->status() == slave::sts_disablen)
	    {
	      it->status(slave::sts_trennen_uz_disablen);
	      return true;
	    }
	  else if (it->status() == slave::sts_spoolen )
	    {
	      it->status(slave::sts_trennen_uz_spoolen);
	      return true;
	    }
	}
    }
  return false;
}

bool slave_control::add_slave(const callsign &call, const connect_string &con_pfad, const String &slots, const String &bake )
{
  for (list<slave>::iterator it = slaves.begin() ; it != slaves.end() ; ++it )
    if (samecall(it->get_call(),call))
      return false;

  String pfad = basis_pfad+call.str()+String('/');

  DIR *test = opendir(strtochar(pfad));
  if (test == NULL)
    {
      mode_t mode = S_IRUSR+S_IWUSR+S_IXUSR+S_IRGRP+S_IXGRP+S_IROTH+S_IXOTH;
      if (mkdir(strtochar(pfad),mode) == 0)
	{
	  own_destins ds("@DL.EU");
	  slave sl(call,pfad,con_pfad,slots,bake,slave::m_passiv,ds);
	  slaves.push_back(sl);
	  geaendert = true;
	  return true;
	}
      else
	{
	  closedir(test);
	  return false;
	}
    }
  else
    {
      own_destins ds("@DL.EU");
      slave sl(call,pfad,con_pfad,slots,bake,slave::m_passiv,ds);
      slaves.push_back(sl);
      geaendert = true;
      closedir(test);
      return true;
    }
}

bool slave_control::del_slave(const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     if (it->status() == slave::sts_disabled)
       {
	 slaves.erase(it);
	 geaendert = true;
	 return true;
       }
 return false;
}

bool slave_control::add_pocbake( const callsign &call, const String &text, int slot, t_baken &baken)
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     {
       if (it->add_pocbake(text,slot,baken))
	 {
	   geaendert = true;
	   return true;
	 }
       else
	 return false;
     }
 return false;
}

bool slave_control::del_pocbake( const callsign &call, int slot, t_baken &baken)
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     {
       if (it->del_pocbake(slot,baken))
	 {
	   geaendert = true;
	   return true;
	 }
       else
	 return false;
     }
 return false;
}

bool slave_control::check_pocbake( const callsign &call, int slot)
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     return it->check_pocbake(slot);

 return false;
}


bool slave_control::enable_slave( const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     if (it->status() == slave::sts_disabled)
       {
	 it->status(slave::sts_getrennt);
	 spool.add_dir(it->get_pfad(),it->get_destinations());
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_spool)
       {
	 it->status(slave::sts_getrennt);
	 geaendert = true;
	 return true;
       }
 return false;
}

bool slave_control::reset_slave( const callsign &call )
{
  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      if (samecall(call,it->get_call()))
	{
	  it->n_gescheitert(0);
	  it->status(slave::sts_getrennt);
	  geaendert = true;
	  return true;
         }
    }
  return false;
}

bool slave_control::disable_slave( const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     if (it->status() == slave::sts_spool)
       {
	 it->status(slave::sts_disabled);
	 spool.del_dir(it->get_pfad());
	 geaendert = true;
	 return true;
       }
     else if ( it->status() == slave::sts_getrennt     || 
	       it->status() == slave::sts_gescheitert  )
       {
	 it->status(slave::sts_disabled);
	 spool.del_dir(it->get_pfad());
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_aufbau)
       {
	 it->status(slave::sts_disablen);
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_aktiv)
       {
	 it->status(slave::sts_disablen);
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_spoolen)
       {
	 it->status(slave::sts_disablen);
	 geaendert = true;
	 return true;
       }
     else
       return false;
 return false;
}

bool slave_control::spool_slave( const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     if (it->status() == slave::sts_disabled)
       {
	 it->status(slave::sts_spool);
	 spool.add_dir(it->get_pfad(),it->get_destinations());
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_getrennt)
       {
	 it->status(slave::sts_spool);
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_aufbau)
       {
	 it->status(slave::sts_spoolen);
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_aktiv)
       {
	 it->status(slave::sts_spoolen);
	 geaendert = true;
	 return true;
       }
     else if (it->status() == slave::sts_disablen)
       {
	 it->status(slave::sts_spoolen);
	 geaendert = true;
	 return true;
       }
     else
       return false;
 return false;
}

bool slave_control::activ_slave( const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     {
       it->set_mode(slave::m_activ);
       geaendert = true;
       return true;
     }
 return false;
}

bool slave_control::passiv_slave( const callsign &call )
{
 for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
   if (samecall(call,it->get_call()))
     {
       it->set_mode(slave::m_passiv);
       geaendert = true;
       return true;
     }
 return false;
}

// Entfernt alle alten Funkrufe aus dem Spool-Verzeichnis.
void slave_control::purge()
{
  DIR *dir;
  struct dirent *de;
  time_t cur_time;
  callsign call;
  syslog slog(configuration);

  for (list<slave>::iterator it = slaves.begin(); it != slaves.end(); ++it )
    {
      call=it->get_call();
      String pfad = basis_pfad+call.str()+String('/');

	if ( !(dir = opendir(strtochar(pfad))) ) 
	{
	    slog.eintrag("Kann Spool-Dir zum Purgen nicht oeffnen: " + pfad ,LOGMASK_PRGRMERR);
	    return;
	}

	cur_time = time(NULL);
#ifdef _DEBUG_ELOGS_
	slog.eintrag("Purge Spool-Dir: " + pfad ,LOGMASK_PRGRMMDG);
#endif
	while ((de = readdir(dir)) != NULL) 
	{
		struct stat s;
		char path[(pfad.slen() + 25)];
		
		if (!strstr(de->d_name, "FUNKRUF"))
			continue;

		snprintf(path, sizeof(path), "%s/%s", strtochar(pfad), de->d_name);

		if (stat(path, &s) == -1) 
		{
  	    		slog.eintrag("Purge->Kann Attribute nicht lesen: " + pfad ,LOGMASK_PRGRMERR);
			continue;
		}

		if (cur_time - s.st_mtime >= MAX_FUNKRUF_AGE)
		  if (unlink(path)!=0)
		    {
		      slog.eintrag("Purge->Kann nicht loeschen: " + pfad ,LOGMASK_PRGRMERR);
		    }
	}
	closedir(dir);
    }
}
