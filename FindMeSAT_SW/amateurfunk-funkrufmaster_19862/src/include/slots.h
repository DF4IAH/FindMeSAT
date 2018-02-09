/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Holger Flemming                                    *
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

#ifndef __SLOTS_H__
#define __SLOTS_H__

#include <iostream>

#include "locator.h"

using namespace std;

class ellipse
{
 protected:
  locator bp1,bp2;
  double halbparameter;

  struct strecke abstand;
  locator mittelpunkt;

  // Nun kommen die Funktionskoeffizienten der Quadratischen Gleichung,
  // die diese Ellipse beschreibt.

  double a00, a01, a10, a11, a20, a02;

 public:
  ellipse( void );
  ellipse( const locator&, const locator&, double );

  void mat_rot( double );
  void mat_move( double, double );

  void move(struct strecke );
  locator get_mitte( void );

  bool operator* ( ellipse & );
  void PrintOn( ostream & );
  void ScanFrom( istream & );

  bool solve_ellipse( double, double&, double& );
  bool koordinaten( double&, double&, double&, double&, double, ostream& );
};
