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

#include "recv_select.h"
#include "iputil.h"

#include <sys/time.h>


rx_tx_select::rx_tx_select( void )
{
  FD_ZERO(&rx_set);
  FD_ZERO(&tx_set);

  maxfd = 0;
  ready_desc = 0;
}

void rx_tx_select::clear( void )
{
  FD_ZERO(&rx_set);
  FD_ZERO(&tx_set);

  maxfd = 0;
  ready_desc = 0;
}

void rx_tx_select::rset( const client &cli )
{
  int fd = cli.select_descr();
  FD_SET(fd,&rx_set);
  if ((fd+1) > maxfd)
    maxfd = fd + 1;
}

void rx_tx_select::rset( const server &srv )
{
  int fd = srv.select_descr();
  FD_SET(fd,&rx_set);
  if ((fd+1) > maxfd)
    maxfd = fd + 1;
}

bool rx_tx_select::rtest( const client &cli )
{
  int fd = cli.select_descr();
  int r = FD_ISSET(fd,&rx_ans);
  return r != 0;
}

bool rx_tx_select::rtest( const server &srv )
{
  int fd = srv.select_descr();
  int r = FD_ISSET(fd,&rx_ans);
  return r != 0;
}

void rx_tx_select::rset_stdio( void )
{
  FD_SET(0,&rx_set);
  if (maxfd == 0) 
    maxfd = 1;
}

bool rx_tx_select::rtest_stdio( void )
{
  int r = FD_ISSET(0,&rx_ans);
  return r != 0;
}

// *******************************************************************
// 
//  Hier fangen die setz und Testfunktionen fuer die Schreibpuffer an
//

void rx_tx_select::wset( const client &cli )
{
  int fd = cli.select_descr();
  FD_SET(fd,&tx_set);
  if ((fd+1) > maxfd)
    maxfd = fd + 1;
}

void rx_tx_select::wset( const server &srv )
{
  int fd = srv.select_descr();
  FD_SET(fd,&tx_set);
  if ((fd+1) > maxfd)
    maxfd = fd + 1;
}

bool rx_tx_select::wtest( const client &cli )
{
  int fd = cli.select_descr();
  int r = FD_ISSET(fd,&tx_ans);
  return r != 0;
}

bool rx_tx_select::wtest( const server &srv )
{
  int fd = srv.select_descr();
  int r = FD_ISSET(fd,&tx_ans);
  return r != 0;
}

void rx_tx_select::wset_stdio( void )
{
  FD_SET(1,&tx_set);
  if (maxfd < 2) 
    maxfd = 2;
}

bool rx_tx_select::wtest_stdio( void )
{
  int r = FD_ISSET(1,&tx_ans);
  return r != 0;
}



void rx_tx_select::poll( float maxtime )
{
  bcopy(&rx_set,&rx_ans,sizeof(rx_set));
  bcopy(&tx_set,&tx_ans,sizeof(tx_set));

  struct timeval timeout;
  timeout.tv_sec = (int) maxtime;
  timeout.tv_usec = (int) ((maxtime - timeout.tv_sec) * 1000000.);

  bool flag = true;
  while (flag)
    {
      ready_desc = select(maxfd,&rx_ans,&tx_ans,NULL,&timeout);
      if (ready_desc < 0)
	{
	  if (errno != EINTR)
	    throw Error_while_select(errno);
	}
      else
	flag = false;
    }
}

