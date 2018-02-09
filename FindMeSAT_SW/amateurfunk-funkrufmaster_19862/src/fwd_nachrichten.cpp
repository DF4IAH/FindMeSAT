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

#include <fstream.h>

#include "fwd_nachrichten.h"

extern ofstream fwd_log;

nachricht::nachricht( const Mid &m )
{
  m_id = m;
}

nachricht::~nachricht( void )
{}

void nachricht::PrintOn( ostream &strm ) const
{
  String outp;
  PrintOn(outp);
  strm << outp;
}

void nachricht::ScanFrom( istream &strm )
{
  String inp;
  inp.get(strm,999);
  //cerr << "Dateiscan! Zeile : " << inp << '.' << endl;
  ScanFrom(inp);
}

void nachricht::PrintOn( String & outp ) const
{
}

void nachricht::ScanFrom( const String &inp ) 
{
}

n_types nachricht::get_typ( void ) const 
{
  return n_keine;
}

String nachricht::get_feld( const String &inp, unsigned int &index, bool check_colon )
{
  String pu;

  if (check_colon)
    {
      for (unsigned int i = index ; i < inp.slen(); i++ )
	if (inp[i] == ':')
	  {
	    pu = inp.copy(index,i-index);
	    index = i+1;
	    //cerr << "Feld : " << pu << endl;
	    return pu;
	  }
      //cerr << "In Feld " << inp << " ab Position " << index << " keinen : erkannt." << endl;
      throw Error_wrong_message_format();
    }
  else
    {
      if (inp.slen() > index)
	pu = inp.copy(index,inp.slen()-index);
      else 
	pu = "";
    }
  //cerr << "Feld : " << pu << endl;
  return pu;
}

void nachricht::cut_feld( String &feld )
{
  feld.kuerze();
}


eigenschaften_nachricht::eigenschaften_nachricht( const Mid &m, String ver, protokoll_optionen opt ) : nachricht(m)
{
  Version = ver;
  Optionen = opt;
}

void eigenschaften_nachricht::PrintOn( String &outp ) const
{
  outp.append(String("E:"));
  outp.append(m_id+':');
  outp.append(Version+':');
  outp.append(Optionen.get_string());
}

void eigenschaften_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'E')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  Version = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,false);
	  cut_feld(puffer);
	  Optionen = protokoll_optionen(puffer);
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types eigenschaften_nachricht::get_typ( void ) const 
{
  return n_eigenschaften;
}

funkruf_nachricht::funkruf_nachricht( const Mid &m ) : nachricht(m)
{
}

void funkruf_nachricht::PrintOn( String &outp )const
{
  if (!dest.check())
    throw Error_destin_checksum_error();
  outp.append(String("F:")+m_id+':');
  outp.append(absender.call() + ':');
  outp.append(adr.adr()+':');
  outp.append(dest.get_string()+':');
  outp.append(String(domain)+':');
  outp.append(String(typ)+':');
  outp.append(itoS(priority)+':');
  outp.append(master.call()+':');
  outp.append(text);
}

void funkruf_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'F')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      absender = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      adr = adress(puffer);
	    }
	  catch( Error_no_adress )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      dest = destin(puffer);
	    }
	  catch( Error_no_destin )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  puffer.upcase();
	  if (puffer == "U" || puffer == "M" || puffer == "B")
	    domain = puffer[0] ;
	  else
	    throw Error_wrong_message_format();
	  puffer = get_feld(inp,index,true);
	  puffer.upcase();
	  if (puffer == "R" || puffer == "N" || puffer == "A" || puffer == "B")
	    typ = puffer[0];
	  else
	    throw Error_wrong_message_format();
	  puffer = get_feld(inp,index,true);
	  priority = puffer.Stoi();
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      master = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  text = get_feld(inp,index,false);
	  cut_feld(text);
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types funkruf_nachricht::get_typ( void ) const
{
  return n_funkrufe;
}

skyper_rubrik_nachricht::skyper_rubrik_nachricht( const Mid &m ) : nachricht(m)
{
}

void skyper_rubrik_nachricht::PrintOn( String &outp )const
{
  if (!dest.check())
    throw Error_destin_checksum_error();
  outp.append(String("B:")+m_id+':');
  outp.append(absender.call()+':');
  outp.append(board+':');
  outp.append(itoS(slot)+':');
  outp.append(dest.get_string()+':');
  outp.append(itoS(priority)+':');
  outp.append(itoS(lifetime)+':');
  outp.append(master.call()+':');
  outp.append(text);
}

void skyper_rubrik_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'B')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      absender = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  board = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  slot = puffer.Stoi();
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      dest = destin(puffer);
	    }
	  catch( Error_no_destin )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  priority = puffer.Stoi();
	  puffer = get_feld(inp,index,true);
	  lifetime = puffer.Stoi();
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      master = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  text = get_feld(inp,index,false);
	  cut_feld(text);
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types skyper_rubrik_nachricht::get_typ( void ) const
{
  return n_skyper_board;
}

datenbankaenderung_nachricht::datenbankaenderung_nachricht( const Mid &m ) : nachricht(m)
{
}

void datenbankaenderung_nachricht::PrintOn( String &outp )const
{
  outp.append(String("C:")+m_id+':');
  outp.append(String(typ)+':');
  outp.append(rufzeichen.call()+':');
  outp.append(adr.adr()+':');
  outp.append(ptyp.get_string()+':');
  outp.append(loc.str()+':');
  outp.append(name+':');
  outp.append(tm.get_unix_zeit_string()+':');
  outp.append(master.call());
}

void datenbankaenderung_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'C')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  if (puffer == "A" || puffer == "R" || puffer == "C")
	    typ = puffer[0];
	  else
	    throw Error_wrong_message_format();
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      rufzeichen = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      adr = adress(puffer);
	    }
	  catch( Error_no_adress )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  ptyp = pager_typ(puffer);
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      loc = locator(puffer);
	    }
	  catch( Error_not_a_locator )
	    {
	      loc = locator();
	    }
	  name = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  tm = zeit(puffer.Stoi());
	  puffer = get_feld(inp,index,false);
	  cut_feld(puffer);
	  try
	    {
	      master = callsign(String(puffer));
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types datenbankaenderung_nachricht::get_typ( void ) const
{
  return n_aenderungen;
}




datenbankupdate_nachricht::datenbankupdate_nachricht( const Mid &m ) : nachricht(m)
{
}

void datenbankupdate_nachricht::PrintOn( String &outp )const
{
  outp.append(String("U:")+m_id+':');
  outp.append(String(typ)+':');
  outp.append(rufzeichen.call()+':');
  outp.append(adr.adr()+':');
  outp.append(ptyp.get_string()+':');
  outp.append(loc.str()+':');
  outp.append(name+':');
  outp.append(tm.get_unix_zeit_string()+':');
  outp.append(master.call());
}

void datenbankupdate_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'U')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  if (puffer == "A" || puffer == "R" || puffer == "C")
	    typ = puffer[0];
	  else
	    throw Error_wrong_message_format();
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      rufzeichen = callsign(puffer);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      adr = adress(puffer);
	    }
	  catch( Error_no_adress )
	    {
	      throw Error_wrong_message_format();
	    }
	  puffer = get_feld(inp,index,true);
	  ptyp = pager_typ(puffer);
	  puffer = get_feld(inp,index,true);
	  try
	    {
	      loc = locator(puffer);
	    }
	  catch( Error_not_a_locator )
	    {
	      loc = locator();
	    }
	  name = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,true);
	  tm = zeit(puffer.Stoi());
	  puffer = get_feld(inp,index,false);
	  cut_feld(puffer);
	  try
	    {
	      master = callsign(String(puffer));
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_wrong_message_format();
	    }
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types datenbankupdate_nachricht::get_typ( void ) const
{
  return n_update;
}





datenbankanforderung_nachricht::datenbankanforderung_nachricht( const Mid &m ) : nachricht(m)
{
}

void datenbankanforderung_nachricht::PrintOn( String &outp )const
{
  outp.append(String("R:")+m_id+':');
  outp.append(tm.get_unix_zeit_string());
}

void datenbankanforderung_nachricht::ScanFrom( const String &inp )
{
  String puffer;
  try
    {
      if (inp[0] == 'R')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  puffer = get_feld(inp,index,false);
	  cut_feld(puffer);
	  tm = zeit(puffer.Stoi());
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types datenbankanforderung_nachricht::get_typ( void ) const
{
  return n_updateanforderung;
}





bestaetigungs_nachricht::bestaetigungs_nachricht( const Mid &m ) : nachricht(m)
{
}

void bestaetigungs_nachricht::PrintOn( String &outp )const
{
  outp.append(String("A:")+m_id);
}

void bestaetigungs_nachricht::ScanFrom( const String &inp )
{
  String puffer;

  try
    {
      if (inp[0] == 'A')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,false);
	  cut_feld(m_id);
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types bestaetigungs_nachricht::get_typ( void ) const
{
  return n_bestaetigung;
}


zeit_nachricht::zeit_nachricht( const Mid &m ) : nachricht(m)
{
}

void zeit_nachricht::PrintOn( String &outp ) const
{
  outp.append(String("Z:")+m_id+':');
  outp.append(itoS(version)+':');
  outp.append(itoS(li)+':');
  outp.append(String(typ)+':');
  outp.append(itoS(stratum)+':');
  outp.append(itoS(praez)+':');
  outp.append(ident+':');
  outp.append(t_ref.get_unix_zeit_string()+':');
  outp.append(t_orig.get_unix_zeit_string()+':');
  outp.append(t_rx1.get_unix_zeit_string()+':');
  outp.append(t_tx.get_unix_zeit_string());
}

void zeit_nachricht::ScanFrom( const String &inp )
{
  String str;

  try
    {
      if (inp[0] == 'Z')
	{
	  if (inp[1] != ':')
	    {
	      //cerr << "Zweites zeichen kein : " << endl;
	      throw Error_wrong_message_format();
	    }
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  
	  str = get_feld(inp,index,true);
	  cut_feld(str);
	  version = str.Stoi();
	  
	  str = get_feld(inp,index,true);
	  cut_feld(str);
	  li = str.Stoi();
	  
	  str = get_feld(inp,index,true);
	  cut_feld(str);
	  if ( str.slen() > 0)
	    typ = str[0];
	  
	  str = get_feld(inp,index,true);
	  cut_feld(str);
	  stratum = str.Stoi();
	  
	  str = get_feld(inp,index,true);
	  cut_feld(str);
	  praez = str.Stoi();
	  
	  ident = get_feld(inp,index,true);
	  cut_feld(ident);

	  try
	    {
	      str = get_feld(inp,index,true);
	      cut_feld(str);
	      t_ref = pzeit(str);
	      
	      str = get_feld(inp,index,true);
	      cut_feld(str);
	      t_orig = pzeit(str);
	      
	      str = get_feld(inp,index,true);
	      cut_feld(str);
	      t_rx1 = pzeit(str);
	      
	      str = get_feld(inp,index,false);
	      cut_feld(str);
	      t_tx = pzeit(str);
	    } 
	  catch( Error_wrong_ptime_syntax )
	    {
	      throw Error_wrong_message_format();
	    }
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types zeit_nachricht::get_typ( void ) const
{
  return n_zeit;
}


zielgebiets_nachricht::zielgebiets_nachricht( const Mid &m ) : nachricht(m)
{
}

void zielgebiets_nachricht::PrintOn( String &outp ) const
{
  if (!zielgebiet.check())
    throw Error_destin_checksum_error();
  outp.append(String("D:")+m_id+':');
  outp.append(zielgebiet.get_string()+':');
  outp.append(dtoS(delay,8,3));
}

void zielgebiets_nachricht::ScanFrom(const String &inp )
{
  String str;
  try
    {
      if (inp[0] == 'D')
	{
	  if (inp[1] != ':')
	    throw Error_wrong_message_format();
	  unsigned int index = 2;
	  m_id = get_feld(inp,index,true);
	  
	  try
	    {
	      str = get_feld(inp,index,true);
	      cut_feld(str);
	      zielgebiet = destin(str);
	      
	      str = get_feld(inp,index,false);
	      cut_feld(str);
	      delay = str.Stod();
	      if (delay > MAX_DESTIN_DELAY) delay = -1.0;
	    }
	  catch( Error_no_destin )
	    {
	      throw Error_wrong_message_format();
	    }
	}
      else
	{
	  throw Error_wrong_message_typ();
	}
    }
  catch( Error_String_index_out_of_range )
    {
      throw Error_wrong_message_format();
    }
}

n_types zielgebiets_nachricht::get_typ( void ) const
{
  return n_destination;
}
