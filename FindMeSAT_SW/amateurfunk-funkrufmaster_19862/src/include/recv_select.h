/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2003 by Holger Flemming                               *
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
#ifndef __RECV_SELECT_H__
#define __RECV_SELECT_H__

#include <sys/select.h>

#include "autoconf.h"
#include "socket.h"


class Error_while_select
{
 protected:
  int e;

 public:
  Error_while_select( int en)
    {
      e = en;
#ifdef _DEBUG_EXEC_
      cerr << "Error_while_select, Fehlernummer : " << e << endl;
#endif
    }

  int get_enum( void )
    {
      return e;
    }
};

class rx_tx_select
{
 private:
  fd_set rx_set,tx_set,rx_ans,tx_ans;
  int maxfd;
  int ready_desc;

 public:
  rx_tx_select( void );
  void clear( void );
  void rset(const client& );
  void rset(const server& );
  void rset_stdio( void );
  bool rtest(const client& );
  bool rtest(const server& );
  bool rtest_stdio( void );

  void wset(const client& );
  void wset(const server& );
  void wset_stdio( void );
  bool wtest(const client& );
  bool wtest(const server& );
  bool wtest_stdio( void );



  void poll( float );
  inline int ready_inputs( void )
    {
      return ready_desc;
    }

};



#endif
