/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2003-2004 by Jens Schoon                                   *
 * Based on:  Tide  TTY client for XTide, Copyright (C) 1997  David Flater. *
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

#include "config.h"
#include "callsign.h"
#include "spoolfiles.h"
#include "gezeiten.h"
#include "globdef.h"
#include "interfaces.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "logfile.h"

extern callsign G_mycall;
extern config_file configuration;
extern spoolfiles spool;


/*
  Zunaechst kommen einige Hilfsfunktionen zum Einlesen der harmonics-Tabelle
  So liest String next_line() die naechste Zeile eine, die keine Kommentarzeile
  ist.
*/

String next_line(istream &strm )
{
  String pu("");
  while (!strm.eof() && (pu.slen() < 1 || pu[0] == '#'))
    pu.getline(strm,200);

  if (strm.eof())
    throw Error_end_of_file();
  return pu;
}


/*
  ------------------------------------------------------------------------
  Nun folgen die Methoden der Klasse local_harmonics.
  localharmonics dient dazu, die Gezeitenparameter fuer einen Ort 
  einzulesen und zu speichern
  ------------------------------------------------------------------------
*/

int local_harmonics::hhmm2sec( const String &inp )
{
  int h, m;
  char s;
  if (sscanf (strtochar(inp), "%d:%d", &h, &m) != 2)
    throw Error_syntax_fehler_in_hhmm_string();
  if (sscanf (strtochar(inp), "%c", &s) != 1)
    throw Error_syntax_fehler_in_hhmm_string();
  if (h < 0)
    m = -m;
  else if (s == '-')
    {
      m = -m;
      h = -h;
    }

  return h*3600 + m*60;
}



void local_harmonics::load( istream &strm, int num_cst )
{
  try
    {
      Ort = next_line(strm);
      Ort.kuerze();
      Ort.upcase();
      

      String line;
      line = next_line(strm);
      meridian = hhmm2sec(line);
      line.kuerze();
      
      char junk[80];
      char value[80];
      if (sscanf (strtochar(line), "%s %s", junk, value) < 2)
	Zeitzone = "UTC0";
      else
	Zeitzone = String(value);
      
      line = next_line(strm);
      line.kuerze();
      
      if (sscanf (strtochar(line), "%lf %s", &datum, value) < 2)
	Einheit = "unknown";
      else
	Einheit = String(value);
      
      for (int i = 0; i < num_cst; i++)
	{
	  double v1,v2;
	  line = next_line(strm);
	  line.kuerze();
	  if (sscanf(strtochar(line),"%s %lf %lf",junk, &v1, &v2) == 3)
	    {
	      amp.push_back(v1);
	      epoche.push_back(v2 * PI / 180);
	      if (v1 < 0.0)
		throw Error_syntax_fehler_in_harmonics_datei();
	    }
	  else
	    throw Error_syntax_fehler_in_harmonics_datei();
	}
    }
  catch( Error_syntax_fehler_in_hhmm_string )
    {
      syslog logf(configuration);
      logf.eintrag("Syntax-Fehler in Zeitangabe.",LOGMASK_PRGRMERR);
      throw Error_syntax_fehler_in_harmonics_datei();
    }
}


/*
  ------------------------------------------------------------------------
  Nun folgen die Methoden der Klasse harmonics.
  harmonics ist eine Datenstruktur fuer die gesamten dem Programm
  bekannten GEzeitenparameter. Es enthaelt auch einen Container mit allen
  local_harmonics-Objekten fuer alle Orte
  ------------------------------------------------------------------------
*/
void harmonics::load( void )
{
  char junk[80];
  syslog logf(configuration);

  try
    {
      String hfilename = configuration.find("HARMONICS");
      ifstream hfile(strtochar(hfilename));
      if (hfile)
	{
	  String line;
	  line = next_line(hfile);
	  if (sscanf (strtochar(line), "%d", &num_csts) != 1)
	    throw Error_syntax_fehler_in_harmonics_datei();

	  for (int i = 0; i < num_csts; i++ )
	    {
	      double tmp;
	      line = next_line(hfile);
	      if (sscanf (strtochar(line), "%s %lf", junk, &tmp) != 2)
		throw Error_syntax_fehler_in_harmonics_datei();
	      
	      cst_speeds.push_back(tmp * M_PI / 648000);
	    }

	  line = next_line(hfile);
	  if (sscanf (strtochar(line), "%d", &first_year) != 1)
	    throw Error_syntax_fehler_in_harmonics_datei();

	  line = next_line(hfile);
	  if (sscanf (strtochar(line), "%d", &num_epochs) != 1)
	    throw Error_syntax_fehler_in_harmonics_datei();

	  for (int i=0;i<num_csts;i++) 
	    {
	      line.getline(hfile,200);
	      vector<double> tmpvec;
	      for (int j = 0; j < num_epochs ; j++)
		{
		  double d;
		  hfile >> d;
		  tmpvec.push_back(d * M_PI / 180.0);
		}
	      line.getline(hfile,200);
	      cst_epoches.push_back(tmpvec);
	    }

	  line.getline(hfile,200);
	  line.kuerze();
	  if (line != "*END*")
	    throw Error_syntax_fehler_in_harmonics_datei();

	  line = next_line(hfile);
	  if (sscanf (strtochar(line), "%d", &num_nodes) != 1)
	    throw Error_syntax_fehler_in_harmonics_datei();

	  for (int i=0;i<num_csts;i++) 
	    {
	      line.getline(hfile,200);
	      vector<double> tmpvec;
	      for (int j = 0; j < num_nodes ; j++)
		{
		  double d;
		  hfile >> d;
		  tmpvec.push_back(d);
		}
	      line.getline(hfile,200);
	      cst_nodes.push_back(tmpvec);
	    }

	  line.getline(hfile,200);
	  line.kuerze();
	  if (line != "*END*")
	    throw Error_syntax_fehler_in_harmonics_datei();

	  while (!hfile.eof())
	    {
	      try
		{
		  local_harmonics lharm;
		  lharm.load(hfile,num_csts);

		  orte[lharm.get_ort()] = lharm;
		}
	      catch( Error_syntax_fehler_in_harmonics_datei )
		{
		  logf.eintrag("Syntaxfehler in harmonics-File in Ortsdefinition",LOGMASK_PRGRMERR);
		}
	      catch( Error_end_of_file )
		{}
	    }
	}
      else
	logf.eintrag("Kann harmonics-File nicht oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter HARMONICS nicht definiert",LOGMASK_PRGRMERR);
    }
}

void harmonics::clear( void )
{
  num_csts = 0;
  num_epochs = 0;
  num_nodes = 0;
  first_year = 0;
  cst_speeds.clear();
  cst_epoches.clear();
  cst_nodes.clear();
  orte.clear();
}

/*
  ------------------------------------------------------------------------
  Nun folgen die Methoden der gezeiten_control.
  Gezeitencontrol dient dazu die Auswahl der zu berechnenden Orte zu 
  steuern und zu konfigurieren
  ------------------------------------------------------------------------
*/

gezeiten_control::gezeiten_control( void )
{
  activ = false;
  orte.clear();
}

gezeiten_control::~gezeiten_control( void )
{
  save();
  orte.clear();
  activ = false;
  harm.clear();
}

void gezeiten_control::load( void )
{
  syslog logf(configuration);
  try
    {
      String gez_string = configuration.find("GEZEITEN");
      orte = komma_separeted(gez_string);
      vector<String>::iterator it = orte.begin();
      if (it != orte.end())
	{
	  activ = (*it == "ENABLE");
	  orte.erase(it);
	}
      else
	{
	  logf.eintrag("Parameter Gezeiten ist leer.",LOGMASK_PRGRMERR);
	  activ = false;
	}
      harm.load();
    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Fehler: Parameter GEZEITEN im Konfigurationsfile nicht definiert.",LOGMASK_PRGRMERR);
      activ = false;
    }
  catch( Error_syntax_fehler_in_harmonics_datei )
    {
      logf.eintrag("Fehler: Syntaxfehler in Harmonics-Datei",LOGMASK_PRGRMERR);
      activ = false;
    }
}


void gezeiten_control::save( void )
{
  String gezeiten_string;
  if (activ)
    gezeiten_string = "ENABLE";
  else
    gezeiten_string = "DISABLE";

  for (vector<String>::iterator it = orte.begin(); it != orte.end(); ++it)
    {
      gezeiten_string.append(',');
      gezeiten_string.append(*it);
    }
  configuration.set("GEZEITEN", gezeiten_string);   // und Liste speichern
  configuration.save();
}


void  gezeiten_control::status( bool stat )
{
  activ = stat;
}


int gezeiten_control::add(String &Parameter)
{
  Parameter.kuerze();
  
  // Nachsehen, ob der Ort schon in der Liste steht
  for (t_orte_it it = orte.begin(); it != orte.end(); ++it)
    if (Parameter == *it)    // Ja, er steht drin.
      return 1;
  
  // Nachsehen, ob noch platz ist (max 10 Orte)
  int i = 0;
  for (t_orte_it it = orte.begin(); it != orte.end(); ++it)
    i++;
  
  if (i < 10)
    {
      orte.push_back(Parameter);
      save();
      return 0;
    }
  
  else
    return 2;
}


bool gezeiten_control::del(String &Parameter )
{
  Parameter.kuerze();
  for (t_orte_it it = orte.begin(); it != orte.end(); ++it )
    {
      if (Parameter == *it)
	{
	  orte.erase(it);
	  save();
	  return true;
	}
    }
  return false;
}


bool gezeiten_control::set_harmonics(String &Parameter)
{
  Parameter.kuerze();
  ifstream test(strtochar(Parameter));
  if (test)
    {
      configuration.set("HARMONICS",Parameter);
      configuration.save();
      try
	{
	  harm.clear();
	  harm.load();
	  return true;
	}
      catch( Error_syntax_fehler_in_harmonics_datei )
	{
	  syslog logf(configuration);
	  logf.eintrag("Syntaxfehler in der Harmonics-Datei",LOGMASK_PRGRMERR);
	  activ = false;
	  return false;
	}
    }
  else
    return false;
}


void gezeiten_control::show( ostream &ostr, user_meldungen mldg, char cr)
{
  ostr << mldg.find(578) << cr;
  ostr << mldg.find(579) << cr;
  ostr << mldg.find(580) << " ";
  if (activ)
    ostr << "ENABLED" << cr;
  else
    ostr << "DISABLED" << cr;

  ostr << mldg.find(581) << " ";
  for (t_orte_it it = orte.begin(); it != orte.end() ; ++it )
    ostr << *it << ' ';
  ostr << cr;
}


/*
  ------------------------------------------------------------------------
  Nun folgen die Methoden der Klasse gezeiten.
   ------------------------------------------------------------------------
*/

// Die Methoden der Klasse gezeiten benoetigt die globale Deklaration der
// Klasse gezeiten_control, um darueber an die Informationen aus der
// Konfiguration und der harmonics-Datei zu kommen.

extern gezeiten_control tiden;


gezeiten::gezeiten(void)
{
  init();
  harmon.clear();
  harmon = tiden.get_harmonics();
}

gezeiten::~gezeiten(void)
{
  harmon.clear();
}

void gezeiten::init(void)
{
  next_ht = zeit(0);
  next_ht_adj = zeit(0); 
  epoch = zeit(0);
  year = 0; 
  amplitude = 0.0; 
  absmax = 0.0; 
  absmin = 0.0; 
  htleveloff = 0.0; 
  ltleveloff = 0.0;
  fakedatum = 0.0; 
  fakeamplitude = 0.0;
  httimeoff = 0; 
  lttimeoff = 0; 
  llevelmult = 0;
  hlevelmult = 0;
  work.clear();
}

 

void gezeiten::process( void )
{
  if (tiden.status())
    {
      typedef vector<String> t_orte;
      typedef vector<String>::iterator t_orte_it;
      syslog logf(configuration);
      
      init();
      t_orte orte = tiden.get_orte();
      int i = 1;
      for (t_orte_it it = orte.begin(); it != orte.end() ; ++it )
	{
	  try
	    {  
	      list_tides (i,*it);
	    }
	  catch( Error_ort_not_defined )
	    {
	      logf.eintrag("Ort "+*it+" nicht in harmonics-Datei",LOGMASK_PRGRMMDG);
	    }
	  catch( Error_harmonics_index_out_of_range )
	    {
	      logf.eintrag("Indexueberschreitung in harmonics-Daten",LOGMASK_PRGRMERR);
	    }
	  catch( Error_while_calculating_tides )
	    {
	      logf.eintrag("Fehler bei der Tidenberechnung",LOGMASK_PRGRMERR);
	    }
	  i++;
	}
    }
}


/* List several days of high and low tides to standard output.  This
got ugly because of the need to make things come out in the right order
with the enhanced mark code. */
void gezeiten::list_tides (int slot, const String& ort)
{
  int event_type;
  String msg("Gezeitenberechnung: ");

  next_ht = zeit();
  msg=msg+ort;

  while(msg.slen()<40) 
    msg.append(" ");

  for (int i=0;i<2;i++) 
  {
    event_type = update_high_tide (ort);

    next_ht_adj.set_darstellung(zeit::f_zeitdatum);
    next_ht_adj.set_utc();

    if (event_type & 1) 
      msg=msg + "NW:   " + next_ht_adj.get_zeit_string() + " z";
    if (event_type & 2) 
      msg=msg + "HW:   " + next_ht_adj.get_zeit_string() + " z";
  }
  spool_msg(msg,slot);
}

void gezeiten::spool_msg( const String& msg , int slot)
{
  syslog logf(configuration);
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht abschicken
      board brd(RUB_GEZEITEN,configuration);
      int board_id = brd.get_brd_id();
      destin ds = get_default_destin();
      brd.set_msg(msg,slot,ds);

      // Nachricht ins Spoolverzeichnis schreiben
      spool.spool_bul(G_mycall,zeit(),board_id,slot,msg,false,ds,128);
    }
  // Moegliche Fehler-Exceptions abfangen
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Gezeiten Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Gezeiten Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
}


/* Determine next big event and set up text string for write_high_tide.
   Returns are same as next_big_event. */
int gezeiten::update_high_tide (const String &ort)
{
  loc_harmon = harmon.get_ort(ort);

  int event_type = next_big_event (next_ht);

  if (event_type & 2)
    next_ht_adj = next_ht + httimeoff;
  else if (event_type & 1)
    next_ht_adj = next_ht + lttimeoff;
  else
    next_ht_adj = next_ht;
    
  return event_type;
}


/* Next high tide, low tide, transition of the mark level, or some
   combination.
       Bit      Meaning
        0       low tide
        1       high tide
        2       falling transition
        3       rising transition
*/

int gezeiten::next_big_event (zeit &tm)
{

  static zeit last_tm   = TIDE_BAD_TIME;
  static zeit cache_hilo;
  static bool  is_high;
  zeit t_hilo;
  zeit t_mark;
  int   is_rising;
  int   stat = 0;

  // Find next high/low tide
  if (tm == last_tm)           // If we have a cached hi/lo tide, use it
    {
      t_hilo = cache_hilo;
    }      
  else
    {
      // Find time of next high or low tide
      t_hilo = next_high_or_low_tide(tm, is_high);
      if (!(t_hilo > tm)) return 0;

      //
      // Brauchen wir das mit dem Mark-crossing ueberhaupt ???
      //

      if ((t_mark = find_mark_crossing(tm, t_hilo, is_rising)) != TIDE_BAD_TIME)
        {
          if (!(t_mark >= tm && t_mark <= t_hilo)) 
	    return 0;
          cache_hilo = t_hilo;  // Save time of next hi/lo
          last_tm = tm = t_mark;
          stat = is_rising ? 0x08 : 0x04;
          if (t_mark < t_hilo)
	    return stat;
        }
    }
  
  last_tm = TIDE_BAD_TIME;              // tag cache as invalid
  tm = t_hilo;
  return stat | (is_high ? 0x02 : 0x01);
}


zeit gezeiten::next_high_or_low_tide (zeit t, bool &hiflag)
{
  bool    rising;
  zeit thilo = next_zero(t, rising, dt_tide_max(2), dt_tide_max(3));
  hiflag = !rising;
  return thilo;
}


/* next_zero(time_t t, double (*f)(), double max_fp, double max_fpp)
 *   Find the next zero of the function f which occurs after time t.
 *   The arguments max_fp and max_fpp give the maximum possible magnitudes
 *   that the first and second derivative of f can achieve.
 *
 *   Algorithm:  Our goal here is to bracket the next zero of f ---
 *     then we can use find_zero() to quickly refine the root.
 *     So, we will step forward in time until the sign of f changes,
 *     at which point we know we have bracketed a root.
 *     The trick is to use large steps in are search, which making
 *     sure the steps are not so large that we inadvertently
 *     step over more than one root.
 *
 *     The big trick, is that since the tides (and derivatives of
 *     the tides) are all just harmonic series', it is easy to place
 *     absolute bounds on their values.
 */
zeit gezeiten::next_zero (zeit t, bool &risingflag, double max_fp, double max_fpp)
{
  zeit t_left = t;
  zeit t_right;
  int step, step1, step2;

  double f_left, df_left, f_right;
  double scale = 1.0;
  // If we start at a zero, step forward until we're past it.
  while ((f_left = f_hiorlo(t_left,0)) == 0.0)
      t_left += TIDE_TIME_PREC;
  if ( !(risingflag = f_left < 0) )
    {
      scale = -1.0;
      f_left = -f_left;
    }
  while (1)
    {
      /* Minimum time to next zero: */
      step1 = (int)(fabs(f_left) / max_fp);

      /* Minimum time to next turning point: */
      df_left = scale * f_hiorlo(t_left,1);
      step2 = (int)(fabs(df_left) / max_fpp);
      if (df_left < 0.0)
        {
          /* Derivative is in the wrong direction. */
          step = step1 + step2;
        }
      else
        {
          step = step1 > step2 ? step1 : step2;
        }

      if (step < TIDE_TIME_STEP)
          step = TIDE_TIME_STEP; /* No rediculously small steps... */

      t_right = t_left + step;
      /*
       * If we hit upon an exact zero, step right until we're off
       * the zero.  If the sign has changed, we are bracketing a desired
       * root, if the sign hasn't changed, then the zero was at
       * an inflection point (i.e. a double-zero to within TIDE_TIME_PREC)
       * and we want to ignore it.
       */
      while ((f_right = scale * f_hiorlo(t_right, 0)) == 0.0)
          t_right += TIDE_TIME_PREC;
      if (f_right > 0.0)
          return find_zero(t_left, t_right); // Found a bracket

      t_left = t_right, f_left = f_right;
    }
}


/* find_zero (time_t t1, time_t t2, double (*f)(time_t t, int deriv))
 *   Find a zero of the function f, which is bracketed by t1 and t2.
 *   Returns a value which is either an exact zero of f, or slightly
 *   past the zero of f.
 */
zeit gezeiten::find_zero (zeit t1, zeit t2)
{
  zeit tl = t1;
  zeit tr = t2;
  double fl = f_hiorlo(tl,0);
  double fr = f_hiorlo(tr,0);
  double scale = 1.0;
  int    dt;
  zeit t = 0;
  double fp = 0.0;
  double ft = 1.0;              /* Forces first step to be bisection */
  double f_thresh = 0.0;

  if (fl == 0.0 || fr == 0.0)
    throw Error_while_calculating_tides();
  if ( tl > tr)
    throw Error_while_calculating_tides();

  if (fl > 0)
    {
      scale = -1.0;
      fl = -fl;
      fr = -fr;
    }

  if (fl == 0.0 || fr == 0.0)
    throw Error_while_calculating_tides();

  while (tr - tl > TIDE_TIME_PREC)
    {
      if (fabs(ft) > f_thresh   /* not decreasing fast enough */
          || (ft > 0 ?          /* newton step would go outside bracket */
              (fp <= ft / (t - tl)) :
              (fp <= -ft / (tr - t))))
        {
          dt = 0;                       /* Force bisection */
        }
      else
        {
          /* Attempt a newton step */
          dt = (int)floor(-ft/fp + 0.5);

          /* Since our goal specifically is to reduce our bracket size
             as quickly as possible (rather than getting as close to
             the zero as possible) we should ensure that we don't take
             steps which are too small.  (We'd much rather step over
             the root than take a series of steps which approach the
             root rapidly but from only one side.) */
          if (abs(dt) < TIDE_TIME_PREC)
              dt = ft < 0 ? TIDE_TIME_PREC : -TIDE_TIME_PREC;

          if ((t += dt) >= tr || t <= tl)
              dt = 0;           /* Force bisection if outside bracket */
          f_thresh = fabs(ft) / 2.0;
        }

      if (dt == 0)
        {
          /* Newton step failed, do bisection */
          t = tl + (tr - tl) / 2;
          f_thresh = fr > -fl ? fr : -fl;
        }

      if ((ft = scale * f_hiorlo(t,0)) == 0)
          return t;             /* Exact zero */
      else if (ft > 0.0)
          tr = t, fr = ft;
      else
          tl = t, fl = ft;

      fp = scale * f_hiorlo(t,1);
    }
  return tr;
}


double gezeiten::f_hiorlo (zeit t, int deriv)
{
  return time2dt_tide(t, deriv + 1);
}


/* blend_weight (double x, int deriv)
 *
 * Returns the value nth derivative of the "blending function" w(x):
 *   w(x) =  0,     for x <= -1
 *   w(x) =  1/2 + (15/16) x - (5/8) x^3 + (3/16) x^5,
 *                  for  -1 < x < 1
 *   w(x) =  1,     for x >= 1
 * This function has the following desirable properties:
 *    w(x) is exactly either 0 or 1 for |x| > 1
 *    w(x), as well as its first two derivatives are continuous for all x.
 */


double gezeiten::blend_weight (double x, int deriv)
{
  double x2 = x * x;
 
   if (x2 >= 1.0)
     return deriv == 0 && x > 0.0 ? 1.0 : 0.0;

   switch (deriv) 
     {
       case 0  : return ((3.0 * x2 -10.0) * x2 + 15.0) * x / 16.0 + 0.5;
       case 1  : return ((x2 - 2.0) * x2 + 1.0) * (15.0/16.0);
       case 2  : return (x2 - 1.0) * x * (15.0/4.0);
       default : return 0;      
     }
   //  assert(0);
}

// This function does the actual "blending" of the tide
// and its derivatives.
double gezeiten::blend_tide (zeit t, int deriv, int first_year, double blend)
{
  vector<double> fl(TIDE_MAX_DERIV + 1);
  vector<double> fr(TIDE_MAX_DERIV + 1);
  bool lr_flag = false;
  vector<double> w(TIDE_MAX_DERIV + 1);
  double        fact = 1.0;
  double        f;
  int           n;

  if (deriv < 0 || deriv > TIDE_MAX_DERIV)
    throw Error_while_calculating_tides();

// If we are already happy_new_year()ed into one of the two years
// of interest, compute that years tide values first.
  if (year == harmon.get_first_year() + 1)
    lr_flag = true;
  else if (year != harmon.get_first_year())
      happy_new_year(harmon.get_first_year());
  for (n = 0; n <= deriv; n++)
    if (lr_flag)
      fr[n] = _time2dt_tide(t, n);
    else
      fl[n] = _time2dt_tide(t, n);

// Compute tide values for the other year of interest,
// and the needed values of w(x) and its derivatives.
  if (!lr_flag)
    {
      happy_new_year(harmon.get_first_year() + 1);
      lr_flag = true;
    }
  else
    {
      happy_new_year(harmon.get_first_year());
      lr_flag = false;
    }
  for (n = 0; n <= deriv; n++)
    {
      if (lr_flag)
	fr[n] = _time2dt_tide(t, n);
      else
	fl[n] = _time2dt_tide(t, n);
      w[n] = blend_weight(blend, n);
    }

  // Do the blending.
  f = fl[deriv];
  for (n = 0; n <= deriv; n++)
    {
      f += fact * w[n] * (fr[deriv-n] - fl[deriv-n]);
      fact *= (double)(deriv - n)/(n+1) * (1.0/TIDE_BLEND_TIME);
    }
  return f;
}


/* time2dt_tide(time_t t, int n)
 *
 *   Calculate nth time derivative the normalized tide.
 *
 * Notes: This function does not check for changes in year.
 *  This is important to our algorithm, since for times near
 *  new years, we interpolate between the tides calculated
 *  using one years coefficients, and the next years coefficients.
 *
 *  Except for this detail, time2dt_tide(t,0) should return a value
 *  identical to time2tide(t).
 */
double gezeiten::_time2dt_tide (zeit t, int deriv)
{
  double dt_tide = 0.0;
  int a, b;
  double term, tempd;

  tempd = M_PI / 2.0 * deriv;
  for (a=0;a<harmon.get_cst_anz();a++)
    {
      term = work[a] *
          cos(tempd +
              harmon.get_speed(a) * 
	      ((long)(t - epoch) + loc_harmon.get_meridian()) +
              harmon.get_epoche(a,year-harmon.get_first_year()) - 
	      loc_harmon.local_epoche(a));
      for (b = deriv; b > 0; b--)
          term *= harmon.get_speed(a);
      dt_tide += term;
    }
  return dt_tide;
}

double gezeiten::time2dt_tide (zeit t, int deriv)
{
  static zeit next_epoch      = TIDE_BAD_TIME; //next years newyears 
  static zeit this_epoch      = TIDE_BAD_TIME; // this years newyears 
  static int    this_year     = -1;
  int           new_year      = yearofzeit(t);

  // Make sure our values of next_epoch and epoch are up to date. 
  if (new_year != this_year)
    {
      if (new_year + 1 < harmon.get_first_year() + harmon.get_epochs_anz())
        {
          set_epoch(new_year + 1, harmon.get_epochs_anz(), harmon.get_first_year());
          next_epoch = epoch;
        }
      else
          next_epoch = TIDE_BAD_TIME;

      happy_new_year(this_year = new_year);
      this_epoch = epoch;
    }

  if (t < this_epoch)
    throw Error_while_calculating_tides();

  if (next_epoch != TIDE_BAD_TIME && t >= next_epoch)
    throw Error_while_calculating_tides();

// If we're close to either the previous or the next
// new years we must blend the two years tides.
  if (t - this_epoch <= TIDE_BLEND_TIME && this_year > harmon.get_first_year())
      return blend_tide(t, deriv, this_year - 1,
                        (double)(t - this_epoch)/TIDE_BLEND_TIME);
  else if (next_epoch - t <= TIDE_BLEND_TIME
           && this_year + 1 < harmon.get_first_year() + harmon.get_epochs_anz())
      return blend_tide(t, deriv, this_year,
                        -(double)(next_epoch - t)/TIDE_BLEND_TIME);

  // Else, we're far enough from newyears to ignore the blending.
  if (this_year != year)
    happy_new_year(this_year);
  return _time2dt_tide(t, deriv);
}


/* dt_tide_max (int n)
 *   Returns the maximum that the absolute value of the nth derivative
 * of the tide can ever attain.
 */
double gezeiten::dt_tide_max (int deriv)
{
  static double maxdt[TIDE_MAX_DERIV+2]; 
  double        max     = 0.0;
  int           myyear, wasyear;
  int           a;

  // Initialise maxdt

  for (int i = 0; i < TIDE_MAX_DERIV+2; i++)
    maxdt[i] = 0.;

  /* We need to be able to calculate max tide derivatives for one
   * derivative higher than we actually need to know the tides.
   */

  if (deriv < 0 || deriv > TIDE_MAX_DERIV+1)
    throw Error_while_calculating_tides();

  if (maxdt[deriv] <= 0.0)
    {
      /* Actually doing a happy_new_year on 1970 is unsafe because
         the mktime in tm2gmt will, on rare occasions, fail because the
         uncorrected time_t is before the beginning of the Unix epoch.
         I've kludged this to include 1970 without crashing mktime.
         -- DWF
         tm2gmt has since been redone, but this "workaround" doesn't
         harm anything, so I'll leave it in. -- DWF */

      int first_yr = harmon.get_first_year();
      wasyear = year;
      if (!wasyear)
        wasyear = first_yr+1;
      for (myyear = first_yr; myyear < first_yr + harmon.get_epochs_anz(); myyear++)
        {
          /* happy_new_year(myyear);    Crash.  Burn. */
          year = myyear;
          figure_multipliers ();

          max = 0.0;
          for (a=0;a<harmon.get_cst_anz();a++)
              max += work[a] * pow(harmon.get_speed(a), (double) deriv);
          if (max > maxdt[deriv])
              maxdt[deriv] = max;
        }
      maxdt[deriv] *= 1.1;      /* Add a little safety margin... */
      happy_new_year (wasyear);   /* Clean up the mess */
    }
  return maxdt[deriv];
}


// Figure out normalized multipliers for constituents for a particular
// year.  Save amplitude for drawing unit lines.
void gezeiten::figure_multipliers ()
{
  int a;

  figure_amplitude();
  if (year < harmon.get_first_year() || 
      year >= harmon.get_first_year() + harmon.get_node_anz()) 
    {
      throw Error_while_calculating_tides();  
    }

  int cst_anz = harmon.get_cst_anz();

  work = vector<double>(cst_anz,0.);

  for (a = 0; a < harmon.get_cst_anz(); a++)
    work[a] = loc_harmon.local_amp(a) * 
      harmon.get_node(a,year-harmon.get_first_year()) / amplitude;
}


// Figure out max amplitude over all the years in the node factors table.
// This function by Geoffrey T. Dairiki
void gezeiten::figure_amplitude ()
{
  int i, a;

  if (amplitude == 0.0)
    {
      int anz = harmon.get_node_anz();
      for (i = 0; i < anz; i++)
	{
	  double year_amp = 0.0;

	  int anz_cst = harmon.get_cst_anz();
	  for (a=0; a < anz_cst ; a++)
	    year_amp += loc_harmon.local_amp(a) * harmon.get_node(a,i);

	  if (year_amp > amplitude)
	    amplitude = year_amp;
	}
      
      // Figure other, related global values (DWF)
      absmax = loc_harmon.get_datum() + amplitude;
      absmin = loc_harmon.get_datum() - amplitude;
      if (htleveloff) 
      {
	if (hlevelmult)
	  absmax *= htleveloff;
	else
	  absmax += htleveloff;
      }
      if (ltleveloff) 
      {
	if (llevelmult)
	  absmin *= ltleveloff;
	else
	  absmin += ltleveloff;
      }
      fakedatum = (absmax + absmin) / 2.0;
      fakeamplitude = (absmax - absmin) / 2.0;

      if (fabs (fakedatum) >= 100.0 || fakeamplitude >= 100.0)
      {
      }	
    }
}


// Re-initialize for a different year
void gezeiten::happy_new_year (int new_year)
{
  year = new_year;
  figure_multipliers ();
  set_epoch (year, harmon.get_epochs_anz(), harmon.get_first_year());
}


// Calculate zeit() of the epoch.
void gezeiten::set_epoch (int year, int num_epochs, int first_year)
{
    if (year < first_year || year >= first_year + num_epochs) 
      {
	throw Error_while_calculating_tides();    
      }
    int years = year - 1970;      // Anzahl der jahre seit 1970 mal 
    int secs = years * 31536000;  // Anzahl der Sekunden pro Jahr

    // Nun Anzahl der Schalttage berechnen

    int stage = (year / 4) - 492;  // Bis 1970 gab es schon 492 Schalttage
    stage -= ( (year / 100) - 19 ); // 
    stage += ( (year / 400) - 4 );

    secs += stage * 86400;

    epoch = zeit(secs);

}

int gezeiten::yearofzeit( zeit t )
{
  int secs = t.get_systime();
  int years = secs / 31536000;    // Erste annaeherung
  // Jetzt darauf basierend die Anzahl der Schalttage berechnen
  int stage = (year / 4) - 492;  // Bis 1970 gab es schon 492 Schalttage
  stage -= ( (year / 100) - 19 ); // 
  stage += ( (year / 400) - 4 );
  // Und die entsprechende Sekundenzahl abziehen
  secs -= stage * 86400;
  years = secs / 31536000; // Nun richtige Anzahl berechnen
  return years + 1970;
}


// Das mark-Crossing wird erstmal nicht weiter beachtet, deshalb hier eine 
// Dummy-Funktion. Spaeter kann man das immer noch einbauen.
zeit gezeiten::find_mark_crossing (zeit t1, zeit t2, int &tag)
{
  tag = 0;
  return TIDE_BAD_TIME;
}

