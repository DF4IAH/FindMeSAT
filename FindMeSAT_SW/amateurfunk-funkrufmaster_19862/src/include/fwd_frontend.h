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

#ifndef __FWD_FRONTEND_H__
#define __FWD_FRONTEND_H__

#include "adress.h"
#include "String.h"
#include "callsign.h"
#include "destin.h"
#include "zeit.h"
#include "database.h"



class fwd_api
{
 protected:
  bool check_and_send_entry( const database_entry&, const zeit &, const zeit & );

 public:
  bool fwd_msg( const callsign&, const adress&, const destin&, unsigned int, const String& );
  bool fwd_bul( const callsign&, const String&, int, const destin&, unsigned int, int, const String& );
  bool fwd_usr( char ch, const callsign& ); 
  bool fwd_update_request( const zeit& , const callsign&);
  int fwd_database_transmitt( const zeit&, const zeit& );
};

#endif
