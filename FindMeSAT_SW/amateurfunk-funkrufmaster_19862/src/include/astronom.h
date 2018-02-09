/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
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

#ifndef __ASTRONOM_H__
#define __ASTRONOM_H__

#include "locator.h"
#include "logfile.h"
#include "zeit.h"
#include "destin.h"

#ifndef PI
#define PI		3.141592654
#endif

class Error_sonne_staendig_sichtbar
{
#ifdef _DEBUG_EXEC_
 public:
  Error_sonne_staendig_sichtbar()
    {
      cerr << "Error_sonne_staendig_sichtbar" << endl;
    }
#endif
};

class Error_sonne_nie_sichtbar
{
#ifdef _DEBUG_EXEC_
 public:
  Error_sonne_nie_sichtbar()
    {
      cerr << "Error_sonne_nie_sichtbar" << endl;
    }
#endif
};

class Error_mond_staendig_sichtbar
{
#ifdef _DEBUG_EXEC_
 public:
  Error_mond_staendig_sichtbar()
    {
      cerr << "Error_mond_staendig_sichtbar" << endl;
    }
#endif
};

class Error_mond_nie_sichtbar
{
#ifdef _DEBUG_EXEC_
 public:
  Error_mond_nie_sichtbar()
    {
      cerr << "Error_mond_nie_sichtbar" << endl;
    }
#endif
};

struct eklip_koor
{
  double laenge;
  double breite;
};

struct rekdek
{
  double rektaszension;
  double deklination;
};


class astro_daten
{
 protected:
  locator loc;
  bool activ;
  syslog logf;

  double ma1,mb1,ma2,mb2,ma3,mb3,ma4,mb4,ma5,mb5;
  double neigung_erdach;
  double t_umlauf_sonne;
  double gamma;
  double exentr;
  zeit bezug;

  double periode( double );
 protected:
  double deklination( double , double , double  );
  double rektaszension( double , double , double , double  );
  void sonne_pos( zeit, double&, double& );
  void mond_pos( zeit, double&, double& );
  zeit aufgang( int &, zeit , double , double , double );
  zeit untergang( int &, zeit , double , double , double );
  void sonne(zeit = zeit(), int = 1  );
  void mond(zeit = zeit(), int = 2 );
  void spool_msg( const String& , int );
 public:
  astro_daten( void );
  void process( void );
};



#endif // __ASTRONOM_H__
