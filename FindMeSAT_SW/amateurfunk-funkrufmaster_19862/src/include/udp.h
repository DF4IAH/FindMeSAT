#ifndef __UDP_H__
#define __UDP_H__

#include <iostream>
#include <sstream>

#include <netinet/in.h>


#include "String.h"
#include "iperrors.h"

using namespace std;

class udp_oposite
{
  friend class udp_client;
  friend class udp_server;

 private:
  struct sockaddr_in op_addr;

  udp_oposite( struct sockaddr_in& );

 public:
  udp_oposite( void );
  udp_oposite( const String&, int );
  void set_ipaddress( const String & );
  void set_port(int );
  friend bool operator==( const udp_oposite&, const udp_oposite& );
  friend bool operator!=( const udp_oposite&, const udp_oposite& );

};

class udp_client
{
 private:

  int sockfd;

 public:
  udp_client( void );
  ~udp_client( void );
  int sendmsg(const udp_oposite&, char* );
  int sendmsg(const udp_oposite&, char * , int );
  int recvmsg(udp_oposite&, char *, int );
  inline int select_descr( void )
    {
      return sockfd;
    } 
};

class udp_server
{
 private:

  int sockfd;

 public:
  udp_server( int );
  ~udp_server( void );
  int sendmsg(const udp_oposite&, char* );
  int sendmsg(const udp_oposite&, char * , int );
  int recvmsg(udp_oposite&, char *, int );
  inline int select_descr( void )
    {
      return sockfd;
    } 
};

#endif
