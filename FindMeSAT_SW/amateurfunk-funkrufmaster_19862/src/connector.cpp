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

#include "connector.h"


connection::connection( void )
{
  typ = 'A';
  adresse = "";
  gestartet = false;
  connected = false;
  disconnect_angefordert = false;
  disconnected = false;
  in_puffer.clear();
  out_puffer.clear();
}

connection::connection( char t, const String &ad)
{
  typ = t;
  adresse = ad;
  gestartet = false;
  connected = false;
  disconnect_angefordert = false;
  disconnected = false;
  in_puffer.clear();
  out_puffer.clear();
}

char connection::get_typ( void )
{
  return typ;
}

String connection::get_adresse( void )
{
  return adresse;
}

void connection::established( void )
{ 
  connected = true;
  puffer_entry ent;
  ent.line = String("*** Connected to ")+adresse;
  ent.eol_flag = true;
  in_puffer.push_back(ent);
}

void connection::disconnect( void )
{
  disconnected = true;
}

void connection::recv_line( const String &line, bool eol )
{
  puffer_entry ent;
  ent.line = line;
  ent.eol_flag = eol;
  in_puffer.push_back(ent);
}

bool connection::send_line( String &line,  bool &eol, bool &disc )
{
  disc = disconnect_angefordert;
  vector<puffer_entry>::iterator it;
  it = out_puffer.begin();
  if ( it != out_puffer.end() )
    {
      line = it->line;
      eol = it->eol_flag;
      out_puffer.erase(it);
      return true;
    }
  else
    return false;
}

void connection::send( const String &line, bool eol )
{
  puffer_entry ent;
  ent.line = line;
  ent.eol_flag = eol;
  out_puffer.push_back(ent);
}

bool connection::receive( String &line, bool &eol, bool &disc )
{
  disc = disconnected;
  vector<puffer_entry>::iterator it;
  it = in_puffer.begin();
  if ( it != in_puffer.end() )
    {
      line = it->line;
      eol = it->eol_flag;
      in_puffer.erase(it);
      return true;
    }
  else
    return false;
}

void connection::discon( void  )
{
  disconnect_angefordert = true;
}


connection_control::connection_control( void )
{
  verbindungen.clear();
  next_id = 0;
}

bool connection_control::start_connection( char &t, String &ad, int &i )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (!it->gestartet)
	{
	  it->gestartet = true;
	  t = it->get_typ();
	  ad = it->get_adresse();
	  i = it->id;
	  return true;
	}
    }
  return false;
}

void connection_control::established( int id )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	it->established();
    }
}

void connection_control::disconnected( int id )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	it->disconnect();
    }
}

void connection_control::received( int id, const String &line, bool eol )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	it->recv_line(line,eol);
    }
}

bool connection_control::to_send( int id, String &line, bool &eol, bool &disc )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	return it->send_line(line,eol,disc);
    }
  disc = true;
  return false;
}

bool connection_control::disc_request( int id )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	return it->disc_request();
    }
  return false;
}

int connection_control::connect( char t, const String &ad )
{
  connection c(t,ad);
  c.id = next_id++;
  verbindungen.push_back(c);
  return c.id;
}

void connection_control::discon( int id )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	it->discon();
    }
}

void connection_control::send( int id, const String &line, bool eol )
{
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	it->send(line,eol);
    }
}

bool connection_control::receive( int id, String &line, bool &eol, bool &disc )
{   
  for (list<connection>::iterator it = verbindungen.begin(); it != verbindungen.end(); ++it )
    {
      if (id == it->id)
	{
	  bool fl = it->receive(line,eol,disc);
	  if (!fl && disc)
	    verbindungen.erase(it);
	  return fl;
	}
    }
  disc = true;
  return false;
}

 
