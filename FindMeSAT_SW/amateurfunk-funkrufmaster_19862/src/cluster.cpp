/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003-2004 by Holger Flemming                               *
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


#include "cluster.h"
#include "config.h"
#include "logfile.h"

extern config_file configuration;

int cluster_control::wait_time( void  )
{

  if (scheiter_zaehler == 0) 
    return 0;
  else if (scheiter_zaehler < 3)
    return WAIT_UNTIL_CONN_RETRY;
  else if (scheiter_zaehler < 5)
    return 5 * WAIT_UNTIL_CONN_RETRY;
  else if (scheiter_zaehler < 7)
    return 15 * WAIT_UNTIL_CONN_RETRY;
  else if (scheiter_zaehler < 11)
    return 60 * WAIT_UNTIL_CONN_RETRY;
  else if (scheiter_zaehler < 20)
    return 120 * WAIT_UNTIL_CONN_RETRY;
  else 
    return 1440 * WAIT_UNTIL_CONN_RETRY;
}

cluster_control::cluster_control()
{
  activ = false;
  cluster.clear();
}

void cluster_control::init( void )
{
  int cluster_anz = 0;
  cluster.clear();
  syslog logf(configuration);
  try
    {
      String anz = configuration.find("DX_CLUSTER_ANZAHL");
      cluster_anz = anz.Stoi();
    }
  catch( Error_parameter_not_defined )
    {
      cluster_anz = 0;
    }

  try
    {
      String en_str = configuration.find("DX_CLUSTER");
      activ = en_str == "JA";
    }
  catch( Error_parameter_not_defined )
    {
      activ = false;
    }
  for ( int i = 1 ; i <= cluster_anz ; i++ )
    {
      try
	{
  	  String type_str = "";
	  char parameter_name[200];
	  sprintf(parameter_name,"DX_CLUSTER_%d",i);
	  String call_str = configuration.find(String(parameter_name));

	  sprintf(parameter_name,"DX_CLUSTER_PFAD_%d",i);
	  String pfad_str = configuration.find(String(parameter_name));

	  sprintf(parameter_name,"DX_CLUSTER_TYP_%d",i);
	  type_str = configuration.find(String(parameter_name));
	  struct dx_cluster cl;
	  cl.dx_cl = callsign(call_str);
	  cl.dx_cluster_pfad = connect_string(pfad_str);
	  if (type_str == "RAW")
	    cl.typ = dx_raw;
	  else if (type_str == "PAV")
	    cl.typ = dx_pav;
	  else if (type_str == "CLX")
	    cl.typ = dx_clx;
	  else
	    cl.typ = dx_sorted;

	  cluster.push_back(cl);
	}
      catch( Error_parameter_not_defined )
	{
	  logf.eintrag("Cluster-Konfigurationsparameter nicht definiert.",LOGMASK_PRGRMERR);
	}
      catch( Error_no_callsign )
	{	  
	  logf.eintrag("Cluster-Konfigurationsparameter enthaelt falsches Rufzeichen",LOGMASK_PRGRMERR);
	}
      catch( Error_syntax_fehler_in_connect_string )
	{	  
	  logf.eintrag("Cluster-Konfigurationsparameter enthaelt falschen Connect-Pfad.",LOGMASK_PRGRMERR);
	}
    }
  status = st_getrennt;
  last_status_change = zeit();
  scheiter_zaehler = 0;
  cluster_iterator = cluster.begin();
}

cluster_control::~cluster_control( void )
{
  cluster.clear();
}

bool cluster_control::cluster_connect( callsign &call, connect_string &pfad, dx_cluster_typ &typ )
{
  if (activ)
    {
      if (status == st_getrennt && zeit() - last_status_change > 5)
	{
	  if (cluster_iterator != cluster.end() )
	    {
	      call = cluster_iterator->dx_cl;
	      pfad = cluster_iterator->dx_cluster_pfad;
	      typ = cluster_iterator->typ;
	      return true;
	    }
	  else
	    return false;
	}
      else if (status == st_gescheitert && zeit() - last_status_change > wait_time() )
	{
	  if (cluster_iterator != cluster.end() )
	    {
	      call = cluster_iterator->dx_cl;
	      pfad = cluster_iterator->dx_cluster_pfad;
	      typ = cluster_iterator->typ;
	      return true;
	    }
	  else
	    return false;
	}
    }
  return false;
}

void cluster_control::aufbau( void )
{
  status = st_aufbau;
  last_status_change = zeit();
}

void cluster_control::gescheitert( void )
{
  status = st_gescheitert;
  last_status_change = zeit();
  cluster_iterator++;
  if (cluster_iterator == cluster.end())
    {
      cluster_iterator = cluster.begin();
      scheiter_zaehler++;
    }
}

void cluster_control::established( void )
{
  status = st_aktiv;
  last_status_change = zeit();
  scheiter_zaehler = 0;
}

void cluster_control::getrennt( void )
{
  if (status == st_aufbau)
    gescheitert();
  else
    {
      status = st_getrennt;
      last_status_change = zeit();
    }
}

void cluster_control::enable( void )
{
  activ = true;
  cluster_iterator = cluster.begin();
  configuration.set("DX_CLUSTER","JA");
}

void cluster_control::disable( void )
{
  activ = false;
  cluster_iterator = cluster.begin();
  configuration.set("DX_CLUSTER","NEIN");
}

bool cluster_control::add( const callsign &call, const connect_string &pfad, dx_cluster_typ typ )
{
  if (!activ)
    {
      dx_cluster cl;
      cl.dx_cl = call;
      cl.dx_cluster_pfad = pfad;
      cl.typ = typ;
      cluster.push_back(cl);
      
      int anz = cluster.size();
      char parameter_name[200];
      
      configuration.set("DX_CLUSTER_ANZAHL",itoS(anz));
      sprintf(parameter_name,"DX_CLUSTER_%d",anz);
      configuration.set(String(parameter_name),call.call());
      sprintf(parameter_name,"DX_CLUSTER_PFAD_%d",anz);
      String puffer = "";
      ostringstream os;
      os << cl.dx_cluster_pfad << '\0';
      puffer.append(os);
      configuration.set(String(parameter_name),puffer);
      sprintf(parameter_name,"DX_CLUSTER_TYP_%d",anz);
      String typs;
      switch (typ)
	{
	  case dx_raw    : typs = "RAW";
	                   break;
	  case dx_pav    : typs = "PAV";
	                   break;
	  case dx_clx    : typs = "CLX";
	                   break;
	  case dx_sorted : typs = "SORT";
	                   break;
	}
      configuration.set(String(parameter_name),typs);
      configuration.save();
      return true;
    }
  return false;
}


bool cluster_control::del( const callsign &call )
{
  if (!activ)
    {
      for (list<dx_cluster>::iterator it = cluster.begin() ; it != cluster.end() ; ++it )
	{
	  if (samecall(it->dx_cl,call))
	    {
	      cluster.erase(it);

	      int anz = cluster.size();
	      int i = 1;
	      char parameter_name[50];

	      configuration.set("DX_CLUSTER_ANZAHL",itoS(anz));
	      for (list<dx_cluster>::iterator it2 = cluster.begin() ; it2 != cluster.end() ; ++it2)
		{
		  sprintf(parameter_name,"DX_CLUSTER_%d",i);
		  configuration.set(String(parameter_name),it2->dx_cl.call());
		  sprintf(parameter_name,"DX_CLUSTER_PFAD_%d",i);
		  String puffer = "";
		  ostringstream os;
		  os << it2->dx_cluster_pfad << '\0';
		  puffer.append(os);
		  configuration.set(String(parameter_name),puffer);
		  String typs;
		  if (it2->typ == dx_raw)
		  {
		    sprintf(parameter_name,"DX_CLUSTER_TYP_%d",i);
		    typs = "RAW";
    		    configuration.set(String(parameter_name),typs);
		  }
		  i++;
		}

	      try
		{
		  sprintf(parameter_name,"DX_CLUSTER_%d",i);
		  configuration.clear(String(parameter_name));
		  sprintf(parameter_name,"DX_CLUSTER_PFAD_%d",i);
		  configuration.clear(String(parameter_name));
		  sprintf(parameter_name,"DX_CLUSTER_TYP_%d",i);
		  configuration.clear(String(parameter_name));
		}
	      catch( Error_parameter_not_defined )
		{
		  syslog slog(configuration);
		  slog.eintrag("Inkonsistenz in DX-Cluster-Konfiguration.",LOGMASK_PRGRMERR);
		}
	      configuration.save();
	      return true;
	    }
	}
    }
  return false;
}

void cluster_control::PrintOn( ostream &strm, char cr )
{
  for (list<dx_cluster>::iterator it = cluster.begin(); it != cluster.end(); ++it )
    {
      if (it == cluster_iterator && activ)
	strm << "-->";
      else
	strm << "   ";

      callsign call = it->dx_cl;
      call.set_format(true);
      strm << call << "   ";
      if (it->typ == dx_raw)
	strm << "RAW  ";
      else
	strm << "     ";
      strm << it->dx_cluster_pfad << cr;
    }
  strm << cr;

  switch(status)
    {
      case st_getrennt    : strm << "getrennt      ";
                            break;
      case st_aufbau      : strm << "aufbau        ";
                            break;
      case st_aktiv       : strm << "aktiv         ";
                            break;
      case st_gescheitert : strm << "gschtrt. (";
	                    strm << setw(3) << scheiter_zaehler << ")";
			    break;
    }
  strm << " ";
  delta_t dt = zeit() - last_status_change;
  strm << dt << cr;
}
