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

#ifndef __FWD_PROTOKOLL_H__
#define __FWD_PROTOKOLL_H__

#include <iostream>
#include "String.h"
#include "globdef.h"

using namespace std;

#define PROT_VERSION "1.00"

#define TIME_PROT_VERSION 1

/* Klasse zur Bearbeitung von Protokolloptionen. Wichtig ist der 
   Operator*, der die Schnittmenge aus zwei Sätzen von 
   Protokolloptionen bildet.
*/
 
class protokoll_optionen
{
 protected:
  String optionen;
 public:
  protokoll_optionen(String = String(THIS_VERSION_OPTIONEN));
  bool check_option(char );
  protokoll_optionen operator*( protokoll_optionen );
  protokoll_optionen operator+( protokoll_optionen );
  protokoll_optionen operator-( protokoll_optionen );
  void PrintOn( ostream& )const ;
  inline String get_string( void ) const
    {
      return optionen;
    }
};

inline ostream& operator<<( ostream &strm, const protokoll_optionen &n )
{
  n.PrintOn(strm);
  return strm;
}


#endif
