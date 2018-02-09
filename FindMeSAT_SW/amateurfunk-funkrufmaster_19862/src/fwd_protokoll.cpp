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

#include "fwd_protokoll.h"

// Konstruktor
protokoll_optionen::protokoll_optionen( String opt )
{
  optionen = opt;
}

// Ueberpruefen, ob eine bestimmt Option im gegebenen Satz der 
// Protokolloptionen vorhanden ist.
bool protokoll_optionen::check_option(char opt )
{
  return optionen.in(String(opt));
}

// Bilden der Schnittmenge zweier Optionssaetze.
protokoll_optionen protokoll_optionen::operator*( protokoll_optionen po )
{
  // Temporaer wir ein neuer Satz angelegt.
  protokoll_optionen tmp(String(""));
  // Die Optionen des ersten Satzes werden durchgegangen
  for (unsigned int i = 0; i < optionen.slen(); i++)
    {
      // Ist diese Option auch im zweiten Satz vorhanden?
      // Dann wird sie auch in den temporaeren Optionssatz eingetragen
      if (po.optionen.in(String(optionen[i])))
	tmp.optionen.append(String(optionen[i]));
    }
  // Zurueck gegeben wird der temporaere Optionssatz, der jetzt
  // die Schnittmenge enthaelt
  return tmp;
}

// Bilden der Vereinigungsmenge zweier Optionssaetze.
protokoll_optionen protokoll_optionen::operator+( protokoll_optionen po )
{
  // Temporaer wir ein neuer Satz angelegt.
  protokoll_optionen tmp = *this;
  // Die Optionen des zweiten Satzes werden durchgegangen
  for (unsigned int i = 0; i < po.optionen.slen(); i++)
    {
      // Ist diese Option nicht im ersten Satz vorhanden?
      // Dann wird sie auch in den temporaeren Optionssatz eingetragen
      if (!tmp.optionen.in(String(po.optionen[i])))
	tmp.optionen.append(String(po.optionen[i]));
    }
  // Zurueck gegeben wird der temporaere Optionssatz, der jetzt
  // die Vereinigungsmenge enthaelt
  return tmp;
}

// Bilden der Differenzmenge zweier Optionssaetze.
protokoll_optionen protokoll_optionen::operator-( protokoll_optionen po )
{
  // Temporaer wir ein neuer Satz angelegt.
  protokoll_optionen tmp("");
  // Die Optionen des ersten Satzes werden durchgegangen
  for (unsigned int i = 0; i < optionen.slen(); i++)
    {
      // Ist diese Option nicht im zweiten Satz vorhanden?
      // Dann wird sie auch in den temporaeren Optionssatz eingetragen
      if (!po.optionen.in(String(optionen[i])))
	tmp.optionen.append(String(optionen[i]));
    }
  // Zurueck gegeben wird der temporaere Optionssatz, der jetzt
  // die Differenzmenge enthaelt
  return tmp;
}


void protokoll_optionen::PrintOn( ostream& strm ) const
{
  strm << optionen;
}
