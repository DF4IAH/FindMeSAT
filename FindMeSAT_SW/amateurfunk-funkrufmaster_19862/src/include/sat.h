/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Jens Schoon                                        *
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

#ifndef __SAT_H__
#define __SAT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include "String.h"
#include "board.h"
#include "destin.h"
#include "spoolfiles.h"
#include "zeit.h"
#include "locator.h"
#include "logfile.h"

using namespace std;

#define	TLE_NAME	0
#define	TLE_LINE1	1
#define	TLE_LINE2	2

#ifndef PI
#define PI		3.141592654
#endif
#ifndef R_ERDE
#define R_ERDE 		6378.16
#endif
#ifndef F_ERDE
#define F_ERDE 		(1.0/298.25)
#endif

class sat
{
 protected:
    String Name[11];
    char   kepler_file[80];
    double my_Latitude;
    double my_Longitude;
    double my_QTH_Height;
    zeit   KeplerDate;

    bool   open_sat_file(void);
    bool   open_kepler_file(void);
    void   berechnen(int);
    double Conv_Digits(char *, int);
    int    Assign_Sat_Name(char *);
    int    Assign_Line1(char *);
    int    Assign_Line2(char *);
    void   Insert_Sat(void);
    void   do_rubrik(String,int);
    void   Calculate_Initial_Data(int);
    void   Calculate_Times(long, int);
    void   Calculate_Satellite_Position(int);
    int    Year_Conversion(int);
    double RAD(double);
    double DEG(double);
    bool   sat_in_kepler(String);
    void   save_SatFileString(void);

    char   Sat_Name[21];
    int    Sat_Epoch_Year;
    double Sat_Epoch_Day;
    double Sat_Inclination;
    double Sat_Mean_Anomoly;
    double Sat_Mean_Motion;
    double Sat_Eccentricity;
    double Sat_Arg_Of_Perigee;
    double Sat_RAAN;
    long   Sat_Epoch_Orbit;
    double Sat_Drag;
    int    Sat_Aflag;
    double Sat_Alon;
    double Sat_Alat;
    int    Sat_MA_Day[10];
    char   Sat_Mode[10][5];
    long   Sat_Catalogue_No;
    
    double Track_Day_Time;
    double Elapsed_Time;
    struct tm Current_Time;
    double Azimuth;
    double Elevation;
    double Range;
    double Half_Angle;
    double SSP_Lat;
    double SSP_Long;
    double Orbit_No;
    double SMA;
    double Height;
    double Arg_Of_Perigee;
    double Mean_Anomoly;
    double RAAN;
    int    MA;
    double Squint;

    double Precession;
    double Station_Earth_X;
    double Station_Earth_Y;
    double Station_Earth_Z;
    double Sidereal_Epoch;
    double Sidereal_Constant;

    char cr;
    char ende;

    destin ds;


struct satellit
{
    zeit   Zeit;
    char   Sat_Name[21];
    int    Sat_Epoch_Year;
    double Sat_Epoch_Day;
    double Sat_Inclination;
    double Sat_Mean_Anomoly;
    double Sat_Mean_Motion;
    double Sat_Eccentricity;
    double Sat_Arg_Of_Perigee;
    double Sat_RAAN;
    long   Sat_Epoch_Orbit;
    double Sat_Drag;
    int    Sat_Aflag;
    double Sat_Alon;
    double Sat_Alat;
    int    Sat_MA_Day[10];
    char   Sat_Mode[10][5];
    long   Sat_Catalogue_No;
};

struct satellit Satelliten[11];


 public:
  sat();
  ~sat();
  String get_sat_list(void);
  String get_status(void);
  void load( void );
  bool process(void);
  void show( ostream&, char );
  int add(String &);
  int del(String &);
  int slot(String &);
  int kepler(String &);
};

class Satellit
{
};

#endif
