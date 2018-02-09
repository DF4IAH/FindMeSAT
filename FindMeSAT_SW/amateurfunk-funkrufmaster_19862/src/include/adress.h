/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
 *                                                                          *
 * Thist Program is free software; yopu can redistribute ist and/or modify  *
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
 


#ifndef __ADRESS_H__
#define __ADRESS_H__

#include <iostream>
#include "String.h"

using namespace std;

class Error_adress_already_exist
{
#ifdef _DEBUG_EXEC_
 public:
  Error_adress_already_exist()
    {
      cerr << "Error_adress_already_exist" << endl;
    }
#endif
};

class Error_no_adress
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_adress()
    {
      cerr << "Error_no_adress" << endl;
    }
#endif
};

// Klasse adress dient dazu, eine Pageradresse zu representieren.
//

class adress
{
 private:
  int main,sub;

 public:
  adress();

  // adress( const String& ) : Moegliche Ausnahme : Error_no_adress
  adress( const String& );
  adress( int, int );

  inline int get_pocadr( void )
    {
      return main;
    };
  String adr() const;
  bool operator==( const adress& ) const;
  bool operator!=(const adress& ) const;
  friend ostream& operator<< (ostream&, const adress& );

  // Operator>> : Moegliche Ausnahme : Error_no_adress
  friend istream& operator>> (istream&, adress& );
  inline bool gueltig( void )
    {
      return (main != 0 || sub != 0);
    }
};

#endif
