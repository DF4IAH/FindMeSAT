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

#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sys/times.h>

#include "String.h"
#include "zeit.h"
#include "thread.h"

using namespace std;


class system_info
{
 protected:
  long int tot_mem,used_mem,free_mem,shared_mem;
  long int tot_swap,used_swap,free_swap;
  long int pmem_size, pmem_lck,pmem_rss,pmem_data,pmem_stack,pmem_exec,pmem_lib;
  static vector<struct thread_info> tlist;
  static vector<struct thread_info>::iterator tlist_it;


  void get_mem( void );
  long int get_mem_size( String );
  void get_process_status( void );

 public:
  String get_compile_options( void );
  String get_cpu_typ( void );
  double get_cpu_speed( void );
  double get_bogomips( void );
  
  inline int get_total_memory( void )
    {
      get_mem();
      return tot_mem;
    }
  inline int get_used_memory( void )
    {
      get_mem();
      return used_mem;
    }
  inline int get_free_memory( void )
    {
      get_mem();
      return free_mem;
    }
  inline int get_shared_memory( void )
    {
      get_mem();
      return shared_mem;
    }
  inline int get_total_swap( void )
    {
      get_mem();
      return tot_swap;
    }
  inline int get_used_swap( void )
    {
      get_mem();
      return used_swap;
    }
  inline long int get_pmem_size( void )
    {
      get_process_status();
      return pmem_size;
    }
  inline long int get_pmem_lck( void )
    {
      get_process_status();
      return pmem_lck;
    }
  inline long int get_pmem_rss( void )
    {
      get_process_status();
      return pmem_rss;
    }
  inline long int get_pmem_data( void )
    {
      get_process_status();
      return pmem_data;
    }
  inline long int get_pmem_stack( void )
    {
      get_process_status();
      return pmem_stack;
    }
  inline long int get_pmem_exec( void )
    {
      get_process_status();
      return pmem_exec;
    }
  inline long int get_pmem_lib( void )
    {
      get_process_status();
      return pmem_lib;
    }
  int get_descriptor_anz( void );
  void Print_descriptoren( ostream &, char );
  void clear_tlist( void );
  void set_tlist( struct thread_info );
  bool get_tlist( bool, struct thread_info& );
};



void PrintMem(ostream&, long int );
String MemtoS(long int );

#endif
