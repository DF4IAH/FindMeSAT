/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
 *                                                                          *
 * This Program is free software; yopu can redistribute ist and/or modify   *
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

#include "ax25.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netax25/axconfig.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define	SA	struct sockaddr

ax25_client::ax25_client( void )
{
}

ax25_client::ax25_client( int fd, struct full_sockaddr_ax25 m, struct full_sockaddr_ax25 s )
{
  sockfd = fd;
  max = m;
  sax = s;
  sockt = SOCK_AX25;
  status = CONNECTED;
}

ax25_client::ax25_client(callsign slavecall, callsign mycall, const String &port, vector<callsign> l2_digis )
{
  char prt[100];
  char *prtcall;


  sockt = SOCK_AX25;
  strncpy(prt,strtochar(port),99);
  if ( !(prtcall = ax25_config_get_addr(prt)))
    throw Error_ax25_config_get_addr();

  mycall.set_format(false);
  mycall.set_nossid(false);
  if (ax25_aton_entry(strtochar(mycall.call()),max.fsa_ax25.sax25_call.ax25_call) < 0 
      || ax25_aton_entry(prtcall,max.fsa_digipeater[0].ax25_call) < 0 )
    throw Error_ax25_master_call();

  slavecall.set_format(false);
  slavecall.set_nossid(false);
  if (ax25_aton_entry(strtochar(slavecall.call()),sax.fsa_ax25.sax25_call.ax25_call) < 0)
    throw Error_ax25_slave_call();

  sax.fsa_ax25.sax25_ndigis = 0;
  for (vector<callsign>::iterator it = l2_digis.begin() ; it != l2_digis.end(); ++it )
    {
      it->set_format(false);
      it->set_nossid(false);
      if (ax25_aton_entry(strtochar(it->call()),sax.fsa_digipeater[sax.fsa_ax25.sax25_ndigis++].ax25_call) < 0)
	  throw Error_ax25_l2digi_call();
    }
  sockfd = socket( AF_AX25, SOCK_SEQPACKET, 0 );
  if (sockfd < 0 )
    throw Error_could_not_gen_socket();
  max.fsa_ax25.sax25_family = AF_AX25;
  max.fsa_ax25.sax25_ndigis = 1;                                     
  sax.fsa_ax25.sax25_family = AF_AX25;

  if (bind(sockfd,(struct sockaddr *) &max,sizeof(max)) < 0 )
    {
      close(  );
      throw Error_could_not_bind_socket(errno);
    }
  if (connect(sockfd,(struct sockaddr *) &sax,sizeof(sax)) < 0 )
    {
      if (errno == ETIMEDOUT)
	{
	  close();
	  throw Error_connection_timed_out();
	}
      else if (errno == ECONNREFUSED)
	{
	  close();
	  throw Error_connection_refused();
	}
      else if (errno == EHOSTUNREACH) 
	{
	  close();
	  throw Error_host_unreachable();
	}
      else if (errno == ENETUNREACH) 
	{
	  close(  );
	  throw Error_net_unreachable();
	}
      else 
	{
	  close();
	  throw Error_connect_failed();
	}
    }
  else
    status = CONNECTED;
}

ax25_client::ax25_client(callsign slavecall, callsign mycall, const String &port, vector<callsign> l2_digis, bool &connected )
{
  char prt[100];
  char *prtcall;


  sockt = SOCK_AX25;
  strncpy(prt,strtochar(port),99);
  if ( !(prtcall = ax25_config_get_addr(prt)))
    throw Error_ax25_config_get_addr();

  mycall.set_format(false);
  mycall.set_nossid(false);
  if (ax25_aton_entry(strtochar(mycall.call()),max.fsa_ax25.sax25_call.ax25_call) < 0 
      || ax25_aton_entry(prtcall,max.fsa_digipeater[0].ax25_call) < 0 )
    throw Error_ax25_master_call();

  slavecall.set_format(false);
  slavecall.set_nossid(false);
  if (ax25_aton_entry(strtochar(slavecall.call()),sax.fsa_ax25.sax25_call.ax25_call) < 0)
    throw Error_ax25_slave_call();

  sax.fsa_ax25.sax25_ndigis = 0;
  for (vector<callsign>::iterator it = l2_digis.begin() ; it != l2_digis.end(); ++it )
    {
      it->set_format(false);
      it->set_nossid(false);
      if (ax25_aton_entry(strtochar(it->call()),sax.fsa_digipeater[sax.fsa_ax25.sax25_ndigis++].ax25_call) < 0)
	  throw Error_ax25_l2digi_call();
    }
  sockfd = socket( AF_AX25, SOCK_SEQPACKET, 0 );
  if (sockfd < 0 )
    throw Error_could_not_gen_socket();
  int flags = fcntl(sockfd,F_GETFL,0);
  fcntl(sockfd,F_SETFL, flags | O_NONBLOCK );

  max.fsa_ax25.sax25_family = AF_AX25;
  max.fsa_ax25.sax25_ndigis = 1;                                     
  sax.fsa_ax25.sax25_family = AF_AX25;

  if (bind(sockfd,(struct sockaddr *) &max,sizeof(max)) < 0 )
    {
      close(  );
      throw Error_could_not_bind_socket(errno);
    }
  if (connect(sockfd,(struct sockaddr *) &sax,sizeof(sax)) < 0 )
    {
      connected = false;
      if (errno == EINPROGRESS) 
	{
	  fcntl(sockfd,F_SETFL, flags );
	  return;
	}
      if (errno == ETIMEDOUT)
	{
	  close();
	  throw Error_connection_timed_out();
	}
      else if (errno == ECONNREFUSED)
	{
	  close();
	  throw Error_connection_refused();
	}
      else if (errno == EHOSTUNREACH) 
	{
	  close();
	  throw Error_host_unreachable();
	}
      else if (errno == ENETUNREACH) 
	{
	  close(  );
	  throw Error_net_unreachable();
	}
      else 
	{
	  close();
	  throw Error_connect_failed();
	}
    }
  else
    {
      fcntl(sockfd,F_SETFL, flags );
      status = CONNECTED;
      connected = true;
    }
}

callsign ax25_client::get_mycall( void )
{
  char us[12];
  strcpy(us,ax25_ntoa(&max.fsa_ax25.sax25_call));
  return callsign(String(us));
}

callsign ax25_client::get_slavecall( void )
{
  char us[12];
  strcpy(us,ax25_ntoa(&sax.fsa_ax25.sax25_call));
  return callsign(String(us));
}


//-------------------------------------------------------------------



ax25_server::ax25_server( callsign mycall, String &port, int backlog)
{
  char prt[100];
  char *prtcall;


  max.fsa_ax25.sax25_family = AF_AX25;
  max.fsa_ax25.sax25_ndigis = 1;                                     

  strncpy(prt,strtochar(port),99);
  if ( !(prtcall = ax25_config_get_addr(prt)))
    throw Error_ax25_config_get_addr();

  mycall.set_format(false);
  mycall.set_nossid(false);
  if (ax25_aton_entry(strtochar(mycall.call()),max.fsa_ax25.sax25_call.ax25_call) < 0 
      || ax25_aton_entry(prtcall,max.fsa_digipeater[0].ax25_call) < 0 )
    throw Error_ax25_master_call();

  if ( (listenfd = socket(AF_AX25, SOCK_SEQPACKET, 0 )) < 0 )
    throw Error_could_not_gen_socket();
  if (bind(listenfd,(struct sockaddr *) &max, sizeof(max)) < 0 )
    if (errno == EADDRINUSE )
      throw Error_address_in_use();
    else
      {
	throw Error_could_not_bind_socket(errno);
      }
  if ( listen( listenfd, backlog ) < 0 )
    throw Error_could_not_set_to_listen();

}

ax25_server::~ax25_server( void )
{
  close();
}

void ax25_server::do_accept( client* &cl  )
{
  bool term_flag = false;
  int connfd;
  
  socklen_t clilen;
  struct full_sockaddr_ax25 cliaddr;
  cl = NULL;

  while( !term_flag)
    {
      clilen = sizeof( cliaddr);
      if ( (connfd = accept( listenfd, (SA * ) &cliaddr, &clilen )) < 0 )
	{
	  if ((errno != EINTR) && (errno != ECONNABORTED )) 
	    throw Error_while_accepting_connection();

	}
      else
	{
	  cl = new ax25_client(connfd,max,cliaddr);
	  term_flag = true;	  
	}
    }
}

void init_ax25( void )
{
  if (!ax25_config_load_ports())
    throw Error_ax25_config_load_ports();
}
