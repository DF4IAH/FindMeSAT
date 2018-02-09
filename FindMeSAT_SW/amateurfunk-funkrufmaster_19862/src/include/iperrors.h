#ifndef __IPERRORS_H__
#define __IPERRORS_H__

#include <iostream>

using namespace std;

// Fehlerklassen, die von der IP-Software benutzt werden.

class Error_hostname_look_up
{
#ifdef _DEBUG_EXEC_
 public:
  Error_hostname_look_up()
    {
      cerr << "Error_hostname_look_up" << endl;
    }
#endif
};

class Error_could_not_gen_socket
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_gen_socket()
    {
      cerr << "Error_could_not_gen_socket" << endl;
    }
#endif
};

class Error_address_in_use
{
#ifdef _DEBUG_EXEC_
 public:
  Error_address_in_use()
    {
      cerr << "Error_address_in_use" << endl;
    }
#endif
};

class Error_could_not_bind_socket
{
 public:
  Error_could_not_bind_socket( int en)
    {
#ifdef _DEBUG_EXEC_
      cerr << "Error_could_not_bind_socket" << endl;
      cerr << "Fehlernummer : " << en << endl;
#endif
    }
};

class Error_could_not_set_to_listen
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_set_to_listen()
    {
      cerr << "Error_could_not_set_to_listen" << endl;
    }
#endif
};

class Error_could_not_convert_ipaddr
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_convert_ipaddr()
    {
      cerr << "Error_could_not_convert_ipaddr" << endl;
    }
#endif
};

class Error_connection_timed_out
{
#ifdef _DEBUG_EXEC_
 public:
  Error_connection_timed_out()
    {
      cerr << "Error_connection_timed_out" << endl;
    }
#endif
};

class Error_connection_refused
{
#ifdef _DEBUG_EXEC_
 public:
  Error_connection_refused()
    {
      cerr << "Error_connection_refused" << endl;
    }
#endif
};

class Error_host_unreachable
{
#ifdef _DEBUG_EXEC_
 public:
  Error_host_unreachable()
    {
      cerr << "Error_host_unreachable" << endl;
    }
#endif
};

class Error_net_unreachable
{
#ifdef _DEBUG_EXEC_
 public:
  Error_net_unreachable()
    {
      cerr << "Error_net_unreachable" << endl;
    }
#endif
};

class Error_connect_failed
{
#ifdef _DEBUG_EXEC_
 public:
  Error_connect_failed()
    {
      cerr << "Error_connect_failed" << endl;
    }
#endif
};

class Error_while_reading_socket
{
#ifdef _DEBUG_EXEC_
 public:
  Error_while_reading_socket()
    {
      cerr << "Error_while_reading_socket" << endl;
    }
#endif
};

class Error_while_writing_socket
{
#ifdef _DEBUG_EXEC_
 public:
  Error_while_writing_socket()
    {
      cerr << "Error_while_writing_socket" << endl;
    }
#endif
};

class Error_socket_would_block
{
#ifdef _DEBUG_EXEC_
 public:
  Error_socket_would_block()
    {
      cerr << "Error_socket_would_block" << endl;
    }
#endif
};

class Error_while_accepting_connection
{
#ifdef _DEBUG_EXEC_
 public:
  Error_while_accepting_connection()
    {
      cerr << "Error_while_accepting_connection" << endl;
    }
#endif
};

#endif
