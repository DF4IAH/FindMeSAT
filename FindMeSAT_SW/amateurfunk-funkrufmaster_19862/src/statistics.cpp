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
#include "statistics.h"

extern callsign G_mycall;

spoolstatistic::spoolstatistic( void )
{
  for (int i = 0 ; i < 240 ; i++)
    {
      local_private[i] = 0;
      local_bul[i] = 0;
      fwd_private[i] = 0;
      fwd_bul[i] = 0;
    }
  last_index = 0;
  cr = '\n';
}

int spoolstatistic::get_index( void )
{
  zeit t;
  int index = t.get_stunden() * 6 + (t.get_minuten() / 10);
  while (last_index != index)
    {
      last_index++;
      if (last_index >= 240) 
	last_index = 0;
      local_private[last_index] = 0;
      local_bul[last_index] = 0;
      fwd_private[last_index] = 0;
      fwd_bul[last_index] = 0;
    }
  return index;
}

void spoolstatistic::spool( bool local, bool priv)
{
  int index = get_index();
  if (local)
    if (priv)
      local_private[index]++;
    else
      local_bul[index]++;
  else
    if (priv)
      fwd_private[index]++;
    else
      fwd_bul[index]++;
}

int spoolstatistic::get_local_private( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += local_private[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += local_private[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += local_private[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += local_private[i];
  return anz;
}

int spoolstatistic::get_local_bul( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += local_bul[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += local_bul[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += local_bul[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += local_bul[i];
  return anz;
}

int spoolstatistic::get_fwd_private( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += fwd_private[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += fwd_private[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += fwd_private[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += fwd_private[i];
  return anz;
}

int spoolstatistic::get_fwd_bul( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += fwd_bul[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += local_private[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += fwd_bul[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += fwd_bul[i];
  return anz;
}

void spoolstatistic::printOn(ostream &strm )
{
  strm << "letzte Stunde " << setw(9) << get_local_private(true) << "  ";
  strm << setw(13) << get_local_bul(true) << "  ";
  strm << setw(9) << get_fwd_private(true) << "  ";
  strm << setw(13) << get_fwd_bul(true) << cr;
  strm << "letzter Tag   " << setw(9) << get_local_private(false) << "  ";
  strm << setw(13) << get_local_bul(false) << "  ";
  strm << setw(9) << get_fwd_private(false) << "  ";
  strm << setw(13) << get_fwd_bul(false) << cr;
  strm << cr << cr;
}

String spoolstatistic::page_msg( void  )
{
  // Ausgabeformat der Spool-Statistik als Funkruf:
  // 0        1         2
  // 12345678901234567890
  //
  // 
  // Rufe,   ##.##. ##:##
  // DB0IUZ,    ges. ####
  //  Priv ####  Bul ####
  // FPriv #### FBul ####

  String puffer;
  ostringstream msg;

  int p = get_local_private(true);
  int b = get_local_bul(true);
  int fp = get_fwd_private(true);
  int fb = get_fwd_bul(true);
  zeit zt;

  zt.set_darstellung(zeit::f_zeitdatum);
  msg << "Rufe,   " << zt;
  callsign mcall = G_mycall;
  mcall.set_format(true);
  mcall.set_nossid(true);
  msg << mcall << ",    ges. ";
  msg << setw(4) << p+b+fp+fb;
  msg << " Priv " << setw(4) << p << "  Bul " << setw(4) << b;
  msg << "FPriv " << setw(4) << fp << " FBul " << setw(4) << fb;
  msg << ends;

  puffer = "";
  puffer.append(msg);
  
  return puffer;
}

fwdstatistic::fwdstatistic( void )
{
  for (int i = 0 ; i < 240 ; i++)
    {
      in_fr[i] = 0;
      in_db[i] = 0;
      in_zeit[i] = 0;
      in_dest[i] = 0;

      out_fr[i] = 0;
      out_db[i] = 0;
      out_zeit[i] = 0;
      out_dest[i] = 0;
    }
  last_index = 0;
  cr = '\n';
}

int fwdstatistic::get_index( void )
{
  zeit t;
  int index = t.get_stunden() * 6 + (t.get_minuten() / 10);
  while (last_index != index)
    {
      last_index++;
      if (last_index >= 240) 
	last_index = 0;
      in_fr[last_index] = 0;
      in_db[last_index] = 0;
      in_zeit[last_index] = 0;
      in_dest[last_index] = 0;

      out_fr[last_index] = 0;
      out_db[last_index] = 0;
      out_zeit[last_index] = 0;
      out_dest[last_index] = 0;
    }
  return index;
}

void fwdstatistic::spool( bool in_out, n_types typ)
{
  int index = get_index();
  switch (typ)
    {
      case n_funkrufe            : 
      case n_skyper_board        : if (in_out)
                                     in_fr[index]++;
                                   else
                                     out_fr[index]++;
	                           break;
      case n_aenderungen         :
      case n_update              :
      case n_updateanforderung   : if (in_out)
                                     in_db[index]++;
                                   else
                                     out_db[index]++;
	                           break;

      case n_zeit                : if (in_out)
                                     in_zeit[index]++;
                                   else
                                     out_zeit[index]++;
	                           break;

      case n_destination         : if (in_out)
                                    in_dest[index]++;
                                   else
                                    out_dest[index]++;
	                           break;

      default                    : {}
    }
}

int fwdstatistic::get_in_fr( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += in_fr[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += in_fr[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += in_fr[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += in_fr[i];
  return anz;
}

int fwdstatistic::get_in_db( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += in_db[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += in_db[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += in_db[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += in_db[i];
  return anz;
}

int fwdstatistic::get_in_zeit( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += in_zeit[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += in_zeit[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += in_zeit[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += in_zeit[i];
  return anz;
}

int fwdstatistic::get_in_dest( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += in_dest[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += in_dest[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += in_dest[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += in_dest[i];
  return anz;
}

int fwdstatistic::get_out_fr( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += out_fr[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += out_fr[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += out_fr[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += out_fr[i];
  return anz;
}

int fwdstatistic::get_out_db( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += out_db[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += out_db[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += out_db[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += out_db[i];
  return anz;
}

int fwdstatistic::get_out_zeit( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += out_zeit[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += out_zeit[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += out_zeit[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += out_zeit[i];
  return anz;
}

int fwdstatistic::get_out_dest( bool hour )
{
  int index = get_index();
  int anz = 0;
  if (hour)
    if (index < 5)
      {
	for (int i = 0; i <= index; i++)
	  anz += out_dest[i];
	for (int i = 239-6+index ; i < 240 ; i++)
	  anz += out_dest[i];
      }
    else
      {
	for (int i = index-6; i <= index; i++)
	  anz += out_dest[i];
      }
  else
    for (int i = 0 ; i < 240 ; i++ )
      anz += out_dest[i];
  return anz;
}

void fwdstatistic::printOn(ostream &strm )
{
  strm << "lst hour           " << "  in     ";
  strm << setw(9) << get_in_fr(true) << " ";
  strm << setw(9) << get_in_db(true) << " ";
  strm << setw(9) << get_in_zeit(true) << " ";
  strm << setw(9) << get_in_dest(true) << " ";
  strm << cr;

  strm << "                   " << " out     ";
  strm << setw(9) << get_out_fr(true) << " ";
  strm << setw(9) << get_out_db(true) << " ";
  strm << setw(9) << get_out_zeit(true) << " ";
  strm << setw(9) << get_out_dest(true) << " ";
  strm << cr;

  strm << "lst day            " << "  in     ";
  strm << setw(9) << get_in_fr(false) << " ";
  strm << setw(9) << get_in_db(false) << " ";
  strm << setw(9) << get_in_zeit(false) << " ";
  strm << setw(9) << get_in_dest(false) << " ";
  strm << cr;

  strm << "                   " << " out     ";
  strm << setw(9) << get_out_fr(false) << " ";
  strm << setw(9) << get_out_db(false) << " ";
  strm << setw(9) << get_out_zeit(false) << " ";
  strm << setw(9) << get_out_dest(false) << " ";
  strm << cr;
}

