/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002-2004 by Holger Flemming                               *
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

/*
  Modelle und Gleichungen zur Berechnung der Erd- und Mondbahn aus

  dem de.sci.astronomie-NRFAQ
    2.3 Wie kan man Sonnenauf- und Untergaenge berechnen?
        von Hartmut Rick

  http://dsa-faq.berlios.de

  und 

  Manfred Maday, DC9ZP, Satelliten, Sonne, Mond

  http://manfred.maday.bei.t-online.de/page6_7.htm

*/

#include "astronom.h"
#include "autoconf.h"
#include "config.h"
#include "callsign.h"

#ifdef COMPILE_SLAVES
#include "spoolfiles.h"
#endif

extern callsign G_mycall;
extern config_file configuration;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

/*


Geplante Aenderung:
===================

Festlegung der Astrodaten nach Slots:

Slot 0 
naechsten drei Auf- bzw. Untergaenge der Sonne, dazu Datums- und
Positionsangabe
                           11111111112
                  12345678901234567890

                  Sonne ##.##.  ######
                  Aufgang     ##:##:##
                  Untergang   ##:##:##
                  Aufgang     ##:##:##

Slot 1
Fuer den heutigen und den naechsten Tag die Tageslaenge, sowie zeit der
groessten Elevation mit Elevation

                           11111111112
                  12345678901234567890

                  ##.##. Tgsl. ##:## #
                  Max. Elv. ##   ##:## 
                  ##.##. Tgsl. ##:## #
                  Max. Elv. ##   ##:## 

Slot 2
naechsten drei Auf- bzw. Untergaenge des Mondes, dazu Datums- und
Positionsangabe
                           11111111112
                  12345678901234567890

                  Mond  ##.##.  ######
                  Aufgang     ##:##:##
                  Untergang   ##:##:##
                  Aufgang     ##:##:##

Slot 3
Aktuelle Mondphase in %, sowie naechste Voll- und Neumond

                           11111111112
                  12345678901234567890

                  Zunehmend        ##%
                  NM     ##.##.  ##:##
                  VM     ##.##.  ##:##

*/

double astro_daten::periode( double phi )
{
  double d;

  double x = phi / ( 2 * PI );
  double x2 = modf(x,&d);

  return 2 * PI * x2;
}

astro_daten::astro_daten( void ) : logf(configuration)
{

  neigung_erdach = 0.40910518;
  t_umlauf_sonne = 31556926.;
  exentr         = 0.0167;
  gamma          = 1.7976891;
  bezug          = zeit(962730000);

  const double b1 = 0.751213;
  const double a1 = 0.036601102;
  const double b2 = 0.822513;
  const double a2 = 0.0362916457;
  const double b3 = 0.995766;
  const double a3 = 0.00273777852;
  const double b4 = 0.974271;
  const double a4 = 0.0338631922;
  const double b5 = 0.0312525;
  const double a5 = 0.0367481957;

  const double epoche = 25567.5;

  double d;
  
  // Umrechnung der Mondbahnkonstanten auf Epoche 1.1.1970 und 
  // Zeiteinheit Sekunden:

  ma1 = a1 / 86400.;
  ma2 = a2 / 86400.;
  ma3 = a3 / 86400.;
  ma4 = a4 / 86400.;
  ma5 = a5 / 86400.;
  
  mb1 = modf(b1 + a1 * epoche,&d);
  mb2 = modf(b2 + a2 * epoche,&d);
  mb3 = modf(b3 + a3 * epoche,&d);
  mb4 = modf(b4 + a4 * epoche,&d);
  mb5 = modf(b5 + a5 * epoche,&d);
  
  
  try
    {
      loc = locator(configuration.find("LOCATOR"));
      activ = configuration.find("ASTRODATEN") == String("JA");

    }
  catch( Error_parameter_not_defined )
    {
      logf.eintrag("Parameter LOCATOR oder ASTRODATEN nicht definiert",LOGMASK_PRGRMERR);
    }
}

void astro_daten::spool_msg( const String& msg , int slot)
{
#ifdef COMPILE_SLAVES
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht 
      // abschicken
      board brd(RUB_ASTRO,configuration);
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
      logf.eintrag("Astrodaten Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Astrodaten Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
#endif
}

double astro_daten::deklination( double laenge, double breite, double ekl )
{
  double tm = cos(breite) * sin(laenge) * sin(ekl) 
    + sin(breite) * cos(ekl);

  return asin(tm);
}

double astro_daten::rektaszension( double laenge, double breite, double dekl, double ekl )
{
  double a2 = cos(breite) * cos(laenge) / cos(dekl);
  double a1 = (cos(breite) * sin(laenge) * cos(ekl) - sin(breite) * sin(ekl))/
    cos(dekl);

  return atan2(a1,a2);
}

void astro_daten::sonne_pos(zeit t, double &laenge, double &breite )
{

  double x = 2 * PI / t_umlauf_sonne * (t - bezug);
  laenge = periode(gamma + x - 2 * exentr * sin( x ));
  breite = 0.;
}

void astro_daten::mond_pos( zeit t, double &laenge, double &breite )
{
  double dummy;
  double c1 = modf(ma1 * (double) t.get_systime() + mb1,&dummy) * 2 * PI;
  double c2 = modf(ma2 * (double) t.get_systime() + mb2,&dummy) * 2 * PI;
  double c3 = modf(ma3 * (double) t.get_systime() + mb3,&dummy) * 2 * PI;
  double c4 = modf(ma4 * (double) t.get_systime() + mb4,&dummy) * 2 * PI;
  double c5 = modf(ma5 * (double) t.get_systime() + mb5,&dummy) * 2 * PI;
  
  // Mondlaenge auf der Ekliptik berechnen
  laenge  = c1 +0.0114842 * sin(2 * c4) + 0.1097637 * sin(c2);
  laenge -= 0.02223544 * sin( c2 - 2 * c4) - 0.0032986 * sin(c3);
  laenge += 0.003735 * sin( 2 * c2) - 0.0019697 * sin( 2 * c5);
  laenge -= 0.0010298 * sin(2 * c2 - 2 * c4) - 0.0009948 * sin( c2 + c3 - 2 * c4);
  
  double c6 = c5 + 0.011507 *  sin( 2 * c4) + 
    0.108739 * sin(c2) - 0.0222 * sin(c2 - 2 * c4);

  breite = 0.08978 * sin(c6) - 0.0025482 * sin(c5 - 2 * c4 );
  laenge = periode(laenge);
  breite = periode(breite);
}

zeit astro_daten::aufgang( int &st , zeit tages_anfang, double rek, double dekl, double hoehe)
{

  double x = 2 * PI / t_umlauf_sonne * (tages_anfang - bezug);

  double b = loc.get_breite().rad();
  double l = loc.get_laenge().rad();
  double costau = ( sin (hoehe) - sin(dekl) * sin(b) ) / ( cos(dekl) * cos(b) );
  if (costau > 1)
    st = +1;
  else if (costau < -1)
    st = -1;
  else 
    {
      st = 0;
      double tau = -acos( costau );
      int u = (int) (86400. / (2 * PI) * (tau + rek - gamma - x - l ) + 43200 );
      u = u % 86400;
      return tages_anfang + u;
    }
  return zeit();
}

zeit astro_daten::untergang( int &st , zeit tages_anfang, double rek, double dekl, double hoehe)
{

  double x = 2 * PI / t_umlauf_sonne * (tages_anfang - bezug);

  double b = loc.get_breite().rad();
  double l = loc.get_laenge().rad();
  double costau = ( sin (hoehe) - sin(dekl) * sin(b) ) / ( cos(dekl) * cos(b) );
  if (costau > 1)
    st = +1;
  else if (costau < -1)
    st = -1;
  else 
    {
      st = 0;
      double tau = acos( costau );
      int u = (int) (86400. / (2 * PI) * (tau + rek - gamma - x - l ) + 43200 );
      u = u % 86400;
      return tages_anfang + u;
    }
  return zeit();
}
 
void astro_daten::sonne( zeit tn, int slot )
{
  const double hoehe          = -0.014660744;

  zeit t = tn + 86400;
  zeit tages_anfang = t - (t.get_systime() % 86400);
  t = tages_anfang;

  zeit t_auf = t;
  zeit t_unter = t;

  int st;

  for (int cnt = 0; cnt < 5; cnt++)
    {
      double laenge, breite;
      double rek_auf, rek_unter;
      double dekl_auf, dekl_unter;
      sonne_pos(t_auf,laenge,breite);
      dekl_auf = deklination(laenge,breite,neigung_erdach);
      rek_auf = rektaszension(laenge,breite,dekl_auf,neigung_erdach);
      sonne_pos(t_unter,laenge,breite);
      dekl_unter = deklination(laenge,breite,neigung_erdach);
      rek_unter = rektaszension(laenge,breite,dekl_unter,neigung_erdach);

      t_auf = aufgang(st,tages_anfang,rek_auf,dekl_auf,hoehe);
      if (st != 0)
	break;
      t_unter = untergang(st,tages_anfang,rek_unter,dekl_unter,hoehe);
      if (st != 0)
	break;
    }
  // Erzeugung des Funkrufes:
  // 12345678901234567890
  // Sonne ##.##.  ######
  // Aufgang   :  ##:## z
  // Untergang :  ##:## z
  // Tagesl.   :  ##:## #

  String msg = "Sonne ";
  zeit jetzt = tn;
  jetzt.set_darstellung(zeit::f_datum);
  msg = msg + jetzt.get_zeit_string() + "  ";
  msg = msg + loc.str();
  if (st < 0)
    msg = msg + "staendig sichtbar";
  else if (st > 0)
    msg = msg + "nie sichtbar";
  else
    {
      t_auf.set_darstellung(zeit::f_zeit_s);
      t_unter.set_darstellung(zeit::f_zeit_s);
      msg = msg + "Aufgang   :  "+t_auf.get_zeit_string()+" z";
      msg = msg + "Untergang :  "+t_unter.get_zeit_string()+" z";
      delta_t dt(t_unter-t_auf);
      msg = msg + "Tagesl.   : "+dt.get_string();
    }
  spool_msg(msg,slot);
}

void astro_daten::mond( zeit tn, int slot )
{
  const double hoehe          = -0.014660744;


  zeit t = tn + 86400;
  zeit tages_anfang = t - (t.get_systime() % 86400);
  t = tages_anfang;

  zeit t_auf = t;
  zeit t_unter = t;

  int st;


  for (int cnt = 0; cnt < 5; cnt++)
    {
      double laenge, breite;
      double rek_auf, rek_unter;
      double dekl_auf, dekl_unter;
      mond_pos(t_auf,laenge,breite);
      dekl_auf = deklination(laenge,breite,neigung_erdach);
      rek_auf = rektaszension(laenge,breite,dekl_auf,neigung_erdach);
      mond_pos(t_unter,laenge,breite);
      dekl_unter = deklination(laenge,breite,neigung_erdach);
      rek_unter = rektaszension(laenge,breite,dekl_unter,neigung_erdach);

      t_auf = aufgang(st,tages_anfang,rek_auf,dekl_auf,hoehe);
      if (st != 0)
	break;
      t_unter = untergang(st,tages_anfang,rek_unter,dekl_unter,hoehe);
      if (st != 0)
	break;
    }

  double l_sonne,l_mond,breite;

  mond_pos(tn,l_mond,breite);
  sonne_pos(tn,l_sonne,breite);

  double ldiff = (l_mond - l_sonne) / 2;
  double si = sin(ldiff);
  int ph = (int) (si * si * 100 );

  // Erzeugung des Funkrufes:
  // 12345678901234567890
  // Mond  ##.##.  ######
  // Aufgang   :  ##:## z
  // Untergang :  ##:## z
  // Zunehmend        ##%


  String msg = "Mond  ";
  zeit jetzt = tn;
  jetzt.set_darstellung(zeit::f_datum);
  msg = msg + jetzt.get_zeit_string() + "  ";
  msg = msg + loc.str();
  if (st < 0)
    msg = msg + "staendig sichtbar   ";
  else if (st > 0)
    msg = msg + "nie sichtbar        ";
  else
    {
      t_auf.set_darstellung(zeit::f_zeit_s);
      t_unter.set_darstellung(zeit::f_zeit_s);
      msg = msg + "Aufgang   :  " + t_auf.get_zeit_string() + " z";
      msg = msg + "Untergang :  " + t_unter.get_zeit_string() + " z";
      if ((-PI < ldiff && ldiff < -(PI/2)) || (0 < ldiff && ldiff < PI/2))
	msg = msg + "Zunehmend        ";
      else
	msg = msg + "Abnehmend        ";
      msg = msg + itoS(ph,2,false) + "%";
    }
  spool_msg(msg,slot);
}



void astro_daten::process( void )
{
  if (activ)
    {
      sonne(zeit(),1);
      sonne(zeit()+86400,3);
      sonne(zeit()+2*86400,5);

      mond(zeit(),2);
      mond(zeit()+86400,4);
      mond(zeit()+2*86400,6);
    }
}
