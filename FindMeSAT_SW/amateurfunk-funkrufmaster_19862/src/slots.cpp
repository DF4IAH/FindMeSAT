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

#include "slots.h"

ellipse::ellipse( void )
{
  bp1 = locator("AA00AA");
  bp2 = bp1;
  halbparameter = 0;
  abstand.entfernung = 0;
  abstand.richtung = 0;
  mittelpunkt = bp1;
}

ellipse::ellipse( const locator &p1, const localtor &p2, double h )
{
  bp1 = p1;
  bp2 = p2;
  halbparameter = h;

  abstand = bp1 - bp2;
  struct strecke halbe = abstand;
  halbe.entfernung /= 2.;
  mittelpunkt = bp1 + halbe;

  a00 = 1.;
  a01 = 0.;
  a10 = 0.;
  a11 = 0.;
  a20 = sqr( halbparameter / 2 + sqrt( sqr( halbparameter / 2) + sqr(halbe.entfernung)));
  a02 = sqr(halbparameter) / 2 + sqrt( sqr( sqr(halbparameter) / 2) + sqr(halbparameter * halbe.entfernung));

  mat_rot(-halbe.richtung);
}

void ellipse::mat_rot( double alpha )
{
  double chi = sin(alpha);
  double psi = cos(alpha);
  double chi2 = chi * chi;
  double psi2 = psi * psi;

  double b00, b01, b10, b11, b02, b20;

  b20 = a20 * psi2 + a02 * chi2 - a11 * psi * chi;
  b02 = a20 * chi2 + a02 * psi2 + a11 * psi * chi;
  b11 = 2 * a20 * psi * chi - 2 * a02 * psi * chi + a11 * psi2 - a11 * chi2;
  b10 = a10 * psi - a01 * chi;
  b01 = a10 * chi + a01 * psi;
  b00 = a00;

  a20 = b20;
  a02 = b02;
  a11 = b11;
  a01 = b01;
  a10 = b10;
  a00 = b00;
}

void ellipse::mat_rot( double w, double v )
{

  double b00, b01, b10, b11, b02, b20;

  b20 = a20;
  b02 = a02;
  b11 = a11;
  b10 = - 2 * a20 * w - a11 * v + a10;
  b01 = - 2 * a02 * v - a11 * w + a01;
  b00 = a20 * w * w + a02 * v * v + a11 * w * v - a10 * w - a01 * v + a00;

  a20 = b20;
  a02 = b02;
  a11 = b11;
  a01 = b01;
  a10 = b10;
  a00 = b00;
}

void ellipse::move( struct strecke d )
{
}


bool ellipse::solve_ellipse( double x, double &y1, double &y2)
{
  double p2 = (a11 * x + a01) / (2 *a02);
  double q = (a20 * sqr(x) + a10 * x + a00) / a02;
  double rad = sqr(p2) - q;

  if (rad >= 0)
    {
      y1 = p2 + sqrt(rad);
      y2 = p2 - sqrt(rad);
      return true;
    }  
  else
    return false;
}

bool ellipse::koordinaten( double &x_l, double &x_h, double &y_l, double &y_h, double x_m, ostream &pt_dat )
{
  double x,y;
  double y1,y2;
  bool ou_flag = true;
  bool pm_flag = false;
  bool flag = true;
  double step = +2.;

  x = x_m;

  while (flag)
    { 
      if (solve_ellipse(x,y1,y2))
	{
	  if (ou_flag)
	    y = y1;
	  else
	    y = y2;
	  pt_dat << x << "  " << y << endl;
	}
      else
	{
	  ou_flag = !ou_flag;
	  step = - step;
	}
      x = x + step;
      if (!ou_flag && (x < x_m))
	pm_flag = true;
      if (pm_flag && (x >= x_m))
	flag = false;
    }
}
