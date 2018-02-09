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
 *  		                                			    *
 ****************************************************************************/

#include "export.h"

#include "config.h"
#include "logfile.h"

extern config_file configuration;

void export_sys::get_file( ofstream &spoolfile, char id )
{
  String fname;
  int i = 0;
  bool found = false;
  // Maximal 100 mal versuchen, ein File zu oeffnen
  while (i < 100 && !found)
    {
      ostringstream fn;
      fn << exportpfad;
      // Filename besteht aus EXPORT.<id>.
      fn << "EXPORT.";
      fn << id << '.';
      fn << rand() % 32768 << ends;
      fname = "";
      fname.append(fn);
      if (!ifstream(strtochar(fname)))
	{
	  // Testen ob das File bereits existiert.
	  spoolfile.open(strtochar(fname));
	  found = true;
	}
      i++;
	}
  if (!found) throw Error_could_not_open_export_file();
}

export_sys::export_sys( void )
{
  try
    {
      maske = configuration.find("EXPORTMASK").Stoi();
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(configuration);
      logf.eintrag("Parameter EXPORTMASK nicht definiert, wird auf 0 gesetzt.",LOGMASK_PRGRMERR);
      maske = 0;
      configuration.set("EXPORTMASK","0");
      configuration.save();
    }
  if (maske != 0)
    {
      try
	{
	  exportpfad = configuration.find("EXPORTDIR");
	}
      catch( Error_parameter_not_defined )
	{
	  syslog logf(configuration);
	  logf.eintrag("Parameter EXPORTDIR nicht definiert.",LOGMASK_PRGRMERR);
	  maske = 0;
	  configuration.set("EXPORTMASK","0");
	  configuration.save();
	  throw Error_could_not_initialize_exportsystem();
	}
    }
}

String export_sys::get_mask( void )
{
  String tmp('#');
  if (maske & EXPORT_MASK_DX    )  tmp.append( EXPORT_MASK_ID_DX    );
  if (maske & EXPORT_MASK_CONDX )  tmp.append( EXPORT_MASK_ID_CONDX );
  if (maske & EXPORT_MASK_WX    )  tmp.append( EXPORT_MASK_ID_WX    );
  if (maske & EXPORT_MASK_PRIV  )  tmp.append( EXPORT_MASK_ID_PRIV  );
  if (maske & EXPORT_MASK_BUL   )  tmp.append( EXPORT_MASK_ID_BUL   );
  return tmp;
}

void export_sys::set_mask( String id )
{
  unsigned int mask = 0;
  id.upcase();

  if (id.slen() > 0 && id[0] == '#')
    {
      for (unsigned int i = 1; i < id.slen(); i++ )
	{
	  if      (id[i] == EXPORT_MASK_ID_DX    ) mask |= EXPORT_MASK_DX;
	  else if (id[i] == EXPORT_MASK_ID_CONDX ) mask |= EXPORT_MASK_CONDX;
	  else if (id[i] == EXPORT_MASK_ID_WX    ) mask |= EXPORT_MASK_WX;
	  else if (id[i] == EXPORT_MASK_ID_PRIV  ) mask |= EXPORT_MASK_PRIV;
	  else if (id[i] == EXPORT_MASK_ID_BUL   ) mask |= EXPORT_MASK_BUL;
	  else throw Error_wrong_id_in_exportmask();
	}
    }
  else
    throw Error_in_exportmask_syntax();

  maske = mask;
  configuration.set("EXPORTMASK",itoS(maske));
  configuration.save();
}

#ifdef COMPILE_CLUSTER
void export_sys::write_dx( const String &dx_meldung )
{
  if ((maske & EXPORT_MASK_DX) != 0)
    {
      ofstream of;
 
      try
	{
	  get_file(of,EXPORT_MASK_ID_DX);
	  of << dx_meldung << endl;
	  of.close();
	}

      catch( Error_could_not_open_export_file )
	{
	  maske &= (~EXPORT_MASK_DX);
	}
    }
}

void export_sys::write_condx( const String &dx_meldung )
{
  if ((maske & EXPORT_MASK_CONDX) != 0)
    {
      ofstream of;
 
      try
	{
	  get_file(of,EXPORT_MASK_ID_CONDX);
	  of << dx_meldung << endl;
	  of.close();
	}

      catch( Error_could_not_open_export_file )
	{
	  maske &= (~EXPORT_MASK_CONDX);
	}
    }
}
#endif

#ifdef COMPILE_WX
void export_sys::write_wx( const wx_meldung &wx )
{
  if ((maske & EXPORT_MASK_WX) != 0)
    {
      ofstream of;
 
      try
	{
	  get_file(of,EXPORT_MASK_ID_WX);
	  of << wx.asynop_msg() << endl;
	  of.close();
	}

      catch( Error_could_not_open_export_file )
	{
	  maske &= (~EXPORT_MASK_WX);
	}
    }
}
#endif

void export_sys::write_user( const callsign &sender, const callsign &master, zeit t_master, adress adr, String msg, bool from_fwd, const destin &d, unsigned int prio)
{
  if ((maske & EXPORT_MASK_PRIV) != 0)
    {
      ofstream of;
 
      try
	{
	  get_file(of,EXPORT_MASK_ID_PRIV);
	  t_master.set_darstellung(zeit::f_zeitdatum_l);
	  of << master << endl;
	  of << t_master << endl;
	  if (from_fwd)
	    of << 'F' << endl;
	  else
	    of << 'L' << endl;
	  of << sender << endl;
	  of << adr << endl;
	  of << d << endl;
	  of << msg << endl;
	  of << prio << endl;
	  of.close();
	}

      catch( Error_could_not_open_export_file )
	{
	  maske &= (~EXPORT_MASK_PRIV);
	}
    }
}

void export_sys::write_bul( const callsign &master, zeit t_master, int board, int slot, String msg, bool from_fwd, const destin &d, unsigned int prio, bool  rep_flag )
{
  if (!rep_flag && ((maske & EXPORT_MASK_BUL) != 0) )
    {
      ofstream of;
 
      try
	{
	  get_file(of,EXPORT_MASK_ID_BUL);
	  t_master.set_darstellung(zeit::f_zeitdatum_l);
	  of << master << endl;
	  of << t_master << endl;
	  if (from_fwd)
	    of << 'F' << endl;
	  else
	    of << 'L' << endl;
	  of << board << endl;
	  of << slot << endl;
	  of << d << endl;
	  of << msg << endl;
	  of << prio << endl;
	  of.close();
	}

      catch( Error_could_not_open_export_file )
	{
	  maske &= (~EXPORT_MASK_BUL);
	}
    }
}
