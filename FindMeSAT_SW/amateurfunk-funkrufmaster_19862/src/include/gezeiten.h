/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003 by Jens Schoon                                        *
 *                                                                          *
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
 * Jens Schoon, DH6BB             email : dh6bb@darc.de                     *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 * List of other authors:                                                   *
 * Holger Flemming, DH4DAI        email : dh4dai@amsat.org                  *
 *                                PR    : dh4dai@db0wts.#nrw.deu.eu         *
 ****************************************************************************/


#ifndef __GEZEITEN_H__
#define __GEZEITEN_H__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include "logfile.h"
#include "lang_support.h"

#include <vector>

using namespace std;

#ifndef PI
#define PI 3.1415926535897932384626433832795029
#endif

#define linelen 	3000
#define MAXARGLEN 	80
#define HOURSECONDS 	3600
#define TIDE_TIME_PREC  (15)
#define TIDE_BLEND_TIME (3600)
#define TIDE_TIME_STEP  (TIDE_TIME_PREC)
#define TIDE_MAX_DERIV  (2) 
#define TIDE_BAD_TIME   ( zeit(-1) )

class Error_syntax_fehler_in_hhmm_string
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_syntax_fehler_in_hhmm_string()
    {
      cerr << "Error_syntax_fehler_in_hhmm_string" << endl;
    }
#endif
};

class Error_syntax_fehler_in_harmonics_datei
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_syntax_fehler_in_harmonics_datei()
    {
      cerr << "Error_syntax_fehler_in_harmonics_datei" << endl;
    }
#endif
};

class Error_end_of_file
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_end_of_file()
    {
      cerr << "Error_end_of_file" << endl;
    }
#endif
};

class Error_harmonics_index_out_of_range
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_harmonics_index_out_of_range()
    {
      cerr << "Error_harmonics_index_out_of_range" << endl;
    }
#endif
};

class Error_ort_not_defined
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_ort_not_defined()
    {
      cerr << "Error_ort_not_defined" << endl;
    }
#endif
};

class Error_while_calculating_tides
{
#ifdef _DEBUG_EXEC_
 public:
  inline Error_while_calculating_tides()
    {
      cerr << "Error_while_calculating_tides" << endl;
    }
#endif
};

class local_harmonics
{
 protected:
  String Ort;
  int meridian;
  String Zeitzone;

  double datum;
  String Einheit;

  vector<double> amp;
  vector<double> epoche;

 protected:
  int hhmm2sec( const String & );

 public:
  void load( istream& , int );

  inline String get_ort( void )
    {
      return Ort;
    }
  inline int get_meridian( void )
    {
      return meridian;
    }
  inline String get_zeitzone( void )
    {
      return Zeitzone;
    }
  inline double get_datum( void )
    {
      return datum;
    }
  inline String get_einheit( void )
    {
      return Einheit;
    }
  inline double local_amp( int i)
    {
      return amp[i];
    }
  inline double local_epoche( int i)
    {
      return epoche[i];
    }
};

class harmonics
{
  int num_csts;
  int num_epochs;
  int num_nodes;
  int first_year;
  vector<double> cst_speeds;
  vector<vector<double> > cst_epoches;
  vector<vector<double> > cst_nodes;
  map<String,local_harmonics> orte;

 public:
  void load( void );
  void clear( void );
  inline int get_cst_anz( void )
    {
      return num_csts;
    }
  inline int get_epochs_anz( void )
    {
      return num_epochs;
    }
  inline int get_node_anz( void )
    {
      return num_nodes;
    }
  inline int get_first_year( void )
    {
      return first_year;
    }
  inline double get_speed( int i )
    {
      if (i < num_csts)
	return cst_speeds[i];
      else
	throw Error_harmonics_index_out_of_range();
    }
  inline double get_epoche( int i, int j )
    {
      if (i < num_csts )
	if (j < num_epochs )
	  return cst_epoches[i][j];
	else
	  throw Error_harmonics_index_out_of_range();
      else
	throw Error_harmonics_index_out_of_range();
    }
  inline double get_node( int i, int j )
    {
      if (i < num_csts )
	if (j < num_nodes )
	  return cst_nodes[i][j];
	else
	  throw Error_harmonics_index_out_of_range();
      else
	throw Error_harmonics_index_out_of_range();
    }
  inline local_harmonics& get_ort( const String & ortname )
    {
      map<String,local_harmonics>::iterator it = orte.find(ortname);
      if ( it != orte.end() )
	return it->second;
      else
	throw Error_ort_not_defined();
    }
};

class gezeiten_control
{
  typedef vector<String> t_orte;
  typedef vector<String>::iterator t_orte_it;
  t_orte orte;
  bool activ;

  harmonics harm;

 public:
  gezeiten_control( void );
  ~gezeiten_control( void );
  void save(void);
  void load(void);
  void status( bool );
  inline bool status( void )
    {
      return activ;
    } 
  int add(String &);
  bool del(String &);
  bool set_harmonics(String &);
  void show( ostream &, user_meldungen , char = '\n' );
  inline harmonics& get_harmonics( void )
    {
      return harm;
    }
  inline vector<String>& get_orte( void )
    {
      return orte;
    }
};


class gezeiten
{
 protected:

  harmonics harmon;
  local_harmonics loc_harmon;

  zeit next_ht;
  zeit next_ht_adj;
  zeit epoch;

  int year;

  double amplitude;
  double absmax;
  double absmin;
  double htleveloff;
  double ltleveloff;
  double fakedatum;
  double fakeamplitude;

  vector<double> work;

  int httimeoff;
  int lttimeoff;
  int hlevelmult;
  int llevelmult;

 protected:
  void init(void);
  void list_tides (int, const String&);
  void spool_msg(const String &, int);
  int update_high_tide (const String &ort);
  int next_big_event (zeit&);
  zeit next_high_or_low_tide ( zeit, bool &);
  zeit next_zero (zeit, bool &, double, double);
  zeit find_zero (zeit, zeit);
  double dt_tide_max (int);
  double blend_weight (double, int);
  double blend_tide (zeit, int, int, double);
  double _time2dt_tide (zeit, int);
  double time2dt_tide (zeit, int);
  double f_hiorlo (zeit, int);
  void figure_multipliers (void);
  void figure_amplitude (void);
  void happy_new_year (int);
  void set_epoch (int, int, int);
  int yearofzeit( zeit );
  zeit find_mark_crossing (zeit, zeit, int &);


 public:
  gezeiten();
  ~gezeiten();
  void process(void);
};

#endif // __GEZEITEN_H__
