#ifndef __MID_H__
#define __MID_H__

#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <list>

#include "String.h"
#include "config.h"
#include "zeit.h"

using namespace std;

class Error_could_not_gen_new_mid
{
#ifdef _DEBUG_EXEC_
 public:
  Error_could_not_gen_new_mid()
    {
      cerr << "Error_could_not_gen_new_mid" << endl;
    }
#endif
};

typedef String Mid;

class mid
{
 private:
  struct ltstr
    {
      bool operator()(const String &s1, const String &s2) const
      {
	return s1 < s2;
      }
    };
  typedef list<Mid> t_mids_unsorted;
  typedef set<Mid,ltstr> t_mids_sorted;


  t_mids_unsorted mids_unsorted;
  t_mids_sorted mids_sorted;
 
  String midlistfile;

  unsigned int max_size;

  zeit last_save;
  int entries_without_save;

 public:
  mid( const String&, int = 8096 );
  mid( void );
  ~mid( void );

  void load( void );
  void save( void );
  void clear( void );
  bool check_mid( Mid & );
  Mid get_newmid( void );

};
#endif
