/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Jens Schoon                                        *
 *                                                                          *
 * Based on tle2amsat.c Copyright (C) 1999      by Hamish Moffatt VK3SB     *
 * Based on get_data.c  Copyright (C) 1991-2000 by J. Naylor HB9DRD/G4KLX   *
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


#include "sat.h"
#include "callsign.h"
#include "globdef.h"
#include "autoconf.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "interfaces.h"
#include "logfile.h"

extern config_file configuration;
extern callsign G_mycall;

#ifdef COMPILE_SLAVES
extern spoolfiles spool;
#endif

// Konstruktor
sat::sat( void )
{
  my_QTH_Height = 200.0;
  for (int i=0; i<11; i++) 
    Satelliten[i].Sat_Name[0]='\0';
}

// Destruktor
sat::~sat( void )
{
}

void sat::load( void )
{
  ds = get_default_destin();
  try
    {
      strcpy(kepler_file, strtochar(configuration.find("KEPLER")));
      try
	{
	  locator loc;
	  breite b;
	  laenge l;
	  loc=locator(strtochar(configuration.find("LOCATOR")));
	  l=loc.get_laenge();
	  b=loc.get_breite();
	  my_Latitude= b.rad();
	  my_Longitude= l.rad();
	  my_Longitude=RAD(360-DEG(my_Longitude));
	  open_sat_file();
	  open_kepler_file();
	}
      catch( Error_parameter_not_defined )
	{
	  syslog logf(configuration);
	  logf.eintrag("Parameter LOCATOR nicht definiert.",LOGMASK_PRGRMERR);
	}
    }
  catch( Error_parameter_not_defined )
    {
      syslog logf(configuration);
      logf.eintrag("Parameter KEPLER nicht definiert.",LOGMASK_PRGRMERR);
    }
}
// Wird via Crontab aufgerufen
bool sat::process(void)
{
    int Nr=0;
    zeit jetzt=zeit();
    struct stat attribut;
    syslog logf(configuration);
    if (!(Name[0]=="DISABLED"))
    {    
	if (stat(kepler_file, &attribut)==-1)
        {
	  logf.eintrag("Fehler einlesen der Attribute von kepler.txt",LOGMASK_PRGRMERR);
	}
	else
	{
    	    if(zeit(attribut.st_mtime)!=KeplerDate)
	    {
        	KeplerDate=zeit(attribut.st_mtime);
		open_kepler_file();
		logf.eintrag("kepler.txt neu eingelesen",LOGMASK_PRGRMMDG);
	    }

    	    for(Nr=1; Nr<=10; Nr++)
	    {
    	    // Ist der letzte Untergang schon vorbei und Sattelit ueberhaupt im Kepler-File gewesen?
    		if (Satelliten[Nr].Zeit<=jetzt && Name[Nr]!="" && Satelliten[Nr].Sat_Name==Name[Nr])
    		{
    		    berechnen(Nr);
    		}
    		else
		{
		}
	    }
	}
    }    
    return true;
}


int sat::kepler(String &Parameter)
{
    char old_kepler_file[80];
    strcpy(old_kepler_file,kepler_file);	//erstmal den alten pfad sichern
    strcpy(kepler_file,strtochar(Parameter));	//neuen Pfad versuchen
    if(open_kepler_file())		
    {
	configuration.set("KEPLER", kepler_file);	//Zumindest laesst sich die Datei oeffnen
	configuration.save();				//also speichern wir den Pfad ab
	return 0;					//alles ok
    }
    else
    {
	strcpy(kepler_file,old_kepler_file);		// Da lief was schief beim oeffnen.
	return 1;				// nicht ok
    }
    return 0;
}

int sat::add(String &Parameter)
{
    String SatFileString=Name[0]+',';
    bool full=true;
    Parameter.kuerze();
    for (int i=1; i<=10; i++) 	// Nachsehen, ob der Satellit schon in der Liste steht
    {
        if(Name[i]==Parameter)
    	    return 1;		// Ja, er steht drin.
    }
    if (sat_in_kepler(Parameter))
    {
        for (int i=1; i<=10; i++) 	// Nachsehen, ob noch platz ist (max 10 Sats)
	{
    	    if(Name[i]=="")	// Ja, wir haben noch platz
    	    {
    		full=false;
    		Name[i]=Parameter;		// Satellit eintragen
		Satelliten[i].Zeit=0;	// und letzter Untergang auf 0 setzen
		SatFileString.append(Name[i]);	// und an die Liste anhaengen
		break;
	    }
	    SatFileString.append(Name[i]);	// Liste aufbauen
	    SatFileString.append(",");
	}
	SatFileString.kuerze();
	if (full)
	    return 2;	// Liste ist voll
	else
	{
	    open_kepler_file();				// Alles neu einlesen
	    configuration.set("SAT", SatFileString);	// uns natuerlich auch speichern
	    configuration.save();
	    return 0;
	}
	return 0;
    }
    else
	return 3;	// Sat nicht im Kepler-File
}

int sat::del(String &Parameter )
{
    String SatFileString=Name[0]+',';
    bool geloescht=false;
    Parameter.kuerze();
    for (int i=1; i<=10; i++) 
    {
        if(Name[i]==Parameter)	// Steht der Sat in der Liste
        {
    	    Name[i]="";	// Ja, also loeschen
	    geloescht=true;
	}
	else if (Name[i]!="")		// Noch nicht gefunden
	{
	    SatFileString.append(Name[i]);	// Liste aufbauen
	    SatFileString.append(",");
	}
    }
    SatFileString.kuerze();    
    if(!geloescht)
	return 1;
    else
    {
	if(SatFileString.slen()>0 && SatFileString[SatFileString.slen()-1]==',')	// Letztes ","
	  {
	    String tmp = SatFileString.copy(0,SatFileString.slen()-1); // in der Liste loeschen
	    SatFileString = tmp;
	  }
	SatFileString.kuerze();    
	configuration.set("SAT", SatFileString);			// und Liste speichern
	configuration.save();
	open_sat_file();	// Liste neu Einlesen
	return 0;
    }
    return 0;
}

int sat::slot(String &Parameter)
{
    Parameter.kuerze();
    if (Parameter=="LOCK")
    {
	Name[0]="LOCK";
	save_SatFileString();
	return 1;
    }
    if (Parameter=="ROTATE")
    {
	Name[0]="ROTATE";
	save_SatFileString();
	return 2;
    }
    if (Parameter=="DISABLE")
    {
	Name[0]="DISABLE";
	save_SatFileString();
	return 3;
    }
    return 0;
}


void sat::save_SatFileString()
{
    String SatFileString=Name[0]+',';
    for (int i=1; i<=10; i++) 
    {
	if (Name[i]!="")		// Noch nicht gefunden
	{
	    SatFileString.append(Name[i]);	// Liste aufbauen
	    SatFileString.append(",");
	}
	else
	{
	SatFileString.kuerze();    
	if(SatFileString.slen()>0 && SatFileString[SatFileString.slen()-1]==',')	// Letztes ","
	  {
	    String tmp = SatFileString.copy(0,SatFileString.slen()-1);	    
	    SatFileString = tmp;		// in der Liste loeschen
	  }
	SatFileString.kuerze();    
	configuration.set("SAT", SatFileString);			// und Liste speichern
	configuration.save();
	break;
	}
    }
}
String sat::get_status()
{
    return(Name[0]);
}

String sat::get_sat_list()
{
    int i=1;
    String SatListe="";
    while (Name[i]!="")
    {
	SatListe.append(Name[i]+"  ");
	i++;
    }
    return SatListe;
}


// Wird im Konstruktor aufgerufen
bool sat::open_sat_file()
{
  String Sats;
  unsigned int len = 0;           // Laenge des Strings ermitteln
  unsigned int index = 0;         // Indexzaehler
  unsigned int beg = index;
  int Nr = 0;
  bool ende = false;

    try
    {
      Sats=configuration.find("SAT");
      len = Sats.slen();           // Laenge des Strings ermitteln      

      for (int i=1; i<=10; i++) 
      {
        Name[i]=""; 
        Satelliten[i].Zeit=zeit(0);
      }

//      while (index < len)  // In Zusammenhang mit dem break unten, macht das
	                   // while doch keine Sinn, oder ???
//      {
          while (index < len && Sats[index] != ',') // ',' suchen
    	    index++;
          Name[0] = Sats.copy(beg,index-beg);	//Status rauskopieren
	    beg=++index; 
//	    break;          // <-- Dieses Break ist gemeint...
//      }

      while (index < len && !ende)
      {
          while (index < len && Sats[index] != ',') // ',' suchen
    	    index++;
          Name[Nr+1] = Sats.copy(beg,index-beg);
    	    Nr++;
	    beg=++index;
          if (Nr>=10) ende = true;                      // Flagvariable definieren
      }

      return true;
    }
    catch( Error_parameter_not_defined )
    {
      syslog logf(configuration);
      logf.eintrag("Parameter SAT nicht definiert.",LOGMASK_PRGRMERR);
    }
    return false;
}

//******* Start Einlesen 2Line ***********
int sat::Assign_Sat_Name(char *Buffer)
{
	int  i;

	if (strlen(Buffer) < 2)
		return 0;

	for (i = strlen(Buffer) - 1; Buffer[i] == ' ' && i >= 0; i--);
    		Buffer[i + 1] = '\0';

	if (strlen(Buffer) > 17 || strlen(Buffer) < 2)
		return 0;

	strcpy(Sat_Name, Buffer);
    		return 1;
}

int sat::Assign_Line1(char *Buffer)
{
	if (Buffer[0] != '1' || strlen(Buffer) < 69)  return 0;

	Sat_Catalogue_No = (long)Conv_Digits(Buffer + 2, 5);
	Sat_Epoch_Year = (int)Conv_Digits(Buffer + 18, 2);

	if (Sat_Epoch_Year > 56)  Sat_Epoch_Year += 1900;
	else 		          Sat_Epoch_Year += 2000;

	Sat_Epoch_Day = Conv_Digits(Buffer + 20, 12);
	Sat_Drag = Conv_Digits(Buffer + 33, 10);
	return 1;
}

int sat::Assign_Line2(char *Buffer)
{
	char Temp[15];
	if (Buffer[0] != '2' || strlen(Buffer) < 69)
		return false;

	Sat_Inclination = Conv_Digits(Buffer + 8, 8);
	Sat_RAAN = Conv_Digits(Buffer + 17, 8);
	Temp[0] = '0'; Temp[1] = '.';
	strncpy(Temp + 2, Buffer + 26, 7);
	Sat_Eccentricity = Conv_Digits(Temp, 9);
	Sat_Arg_Of_Perigee = Conv_Digits(Buffer + 34, 8);
	Sat_Mean_Anomoly = Conv_Digits(Buffer + 43, 8);
	Sat_Mean_Motion = Conv_Digits(Buffer + 52, 11);
	Sat_Epoch_Orbit = (long)Conv_Digits(Buffer + 63, 5);
	return true;
}
//******* Ende Einlesen 2Line ***********


void sat::Insert_Sat()
{
    int Nr=0;
    for(Nr=1; Nr<=10; Nr++)
    {
	if (Name[Nr]==Sat_Name) 
	{
	    Satelliten[Nr].Zeit='\0';
	    strcpy(Satelliten[Nr].Sat_Name,Sat_Name);
	    Satelliten[Nr].Sat_Epoch_Year=Sat_Epoch_Year;
	    Satelliten[Nr].Sat_Epoch_Day=Sat_Epoch_Day;
	    Satelliten[Nr].Sat_Inclination=Sat_Inclination;
	    Satelliten[Nr].Sat_Mean_Anomoly=Sat_Mean_Anomoly;
	    Satelliten[Nr].Sat_Mean_Motion=Sat_Mean_Motion;
	    Satelliten[Nr].Sat_Eccentricity=Sat_Eccentricity;
	    Satelliten[Nr].Sat_Arg_Of_Perigee=Sat_Arg_Of_Perigee;
	    Satelliten[Nr].Sat_RAAN=Sat_RAAN;
	    Satelliten[Nr].Sat_Epoch_Orbit=Sat_Epoch_Orbit;
	    Satelliten[Nr].Sat_Drag=Sat_Drag;
	    Satelliten[Nr].Sat_Aflag=Sat_Aflag;
	    Satelliten[Nr].Sat_Alon=Sat_Alon;
	    Satelliten[Nr].Sat_Alat=Sat_Alat;
	    memcpy(Satelliten[Nr].Sat_MA_Day,Sat_MA_Day,sizeof(Sat_MA_Day));
	    memcpy(Satelliten[Nr].Sat_Mode,Sat_Mode,sizeof(Sat_Mode));
	    Satelliten[Nr].Sat_Catalogue_No=Sat_Catalogue_No;
	}
    }
}


void sat::berechnen(int Nr)
{
    zeit aAufgang;
    zeit aUntergang;
    time_t Track_Time;
    String SatText;
    time_t time_now=time(NULL);
    time_t Aufgang;
    time_t Untergang;
    int max_Elevation=0;
    Calculate_Initial_Data(Nr);
    Track_Time=time_now;
    Elevation=-1.0;
    while (Elevation <=5.0)	//wir werten nur bei Elevation >=5 Grad aus.
    {
	Calculate_Times(Track_Time, Nr);
	Calculate_Satellite_Position(Nr);
        Aufgang=Track_Time;
	Track_Time+=60;	// Eine Minute weiter
    }
    Untergang=Aufgang; //Sicher ist sicher....
    while (Elevation >=0.0)
    {
	Calculate_Times(Track_Time, Nr);
	Calculate_Satellite_Position(Nr);
        Untergang=Track_Time;
	aUntergang=Untergang;
	Track_Time+=60;
	if (Elevation > max_Elevation) max_Elevation = int(Elevation);
    }
    aAufgang = Aufgang;
    aUntergang = Untergang;
    Satelliten[Nr].Zeit=aUntergang;
    delta_t Ueberflugsdauer(Untergang - Aufgang);


    aAufgang.set_darstellung (zeit::f_zeitdatum);
    aUntergang.set_darstellung (zeit::f_zeitdatum);

    SatText=Name[Nr];
    if (SatText.slen() <8) do {SatText.append(" ");} while (SatText.slen() <8);
    SatText.append("DUR:");
    SatText.append(Ueberflugsdauer.get_string());
    SatText.append("AOS: ");
    SatText.append(aAufgang.get_zeit_string());
    SatText.append("UTC");
    if (SatText.slen() <(40)) do {SatText.append(" ");} while (SatText.slen() <(40));
    SatText.append("LOS: ");
    SatText.append(aUntergang.get_zeit_string());
    SatText.append("UTC");
    if (SatText.slen() <(60)) do {SatText.append(" ");} while (SatText.slen() <(60));
    SatText.append("Max. Elevation:  "); 
    SatText.append(itoS(max_Elevation)); 
    SatText.append("*");
    
    do_rubrik(SatText,Nr);
}


bool sat::open_kepler_file()
{
	char Buffer[101];
	char *s;
	int  State;
	State = TLE_NAME;
	FILE *fp;
	if ((fp = fopen(kepler_file, "r")) != NULL)
        {
    	    while (fgets(Buffer, 100, fp) != NULL)
            {
		if ((s = strchr(Buffer, '\n')) != NULL) *s = '\0';
		if ((s = strchr(Buffer, '\r')) != NULL) *s = '\0';

		switch (State)
                {
			case TLE_NAME:
				if (!Assign_Sat_Name(Buffer))
                                {
					State = TLE_NAME;
					break;
				}
				State = TLE_LINE1;
				break;
			case TLE_LINE1:
				if (!Assign_Line1(Buffer))
                                {
					State = TLE_NAME;
					break;
				}
				State = TLE_LINE2;
				break;
			case TLE_LINE2:
				if (!Assign_Line2(Buffer))
                                {
					State = TLE_NAME;
					break;
				}
				Insert_Sat();	
				State = TLE_NAME;
				break;
			default:
				break;
		}
    	    }
    	    fclose(fp);
	    return true;
        }
	return false;
}

void sat::do_rubrik(String sat_line, int slot)
{
#ifdef COMPILE_SLAVES
  String rub;
  rub = RUB_SAT;
  syslog logf(configuration);
  
  try
    {
      // Das entsprechende Board oeffnen und die Nachricht abschicken
      board brd(rub,configuration);
      int board = brd.get_brd_id();
      String msg = sat_line;
      if (Name[0]=="ROTATE")
         slot = brd.get_slot();
      brd.set_msg(msg,slot,ds);

      // Nachricht ins Spoolverzeichnis schreiben
      spool.spool_bul(G_mycall,zeit(),board,slot,msg,false,ds,128);
    }
  // Moegliche Fehler-Exceptions abfangen
  catch( Error_could_not_open_file )
    {
      logf.eintrag("Nicht moeglich, Datei im Spoolverzeichnis zu oeffnen",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_open_boardfile )
    {
      logf.eintrag("Satelliten Boarddatei nicht angelegt.",LOGMASK_PRGRMERR);
    }
  catch( Error_boarddirectory_not_defined )
    {
      logf.eintrag("Rubrikenverzeichnis nicht definiert.",LOGMASK_PRGRMERR);
    }
  catch( Error_could_not_create_boardfile )
    {
      logf.eintrag("Nicht moeglich, Satelliten Boarddatei zu speichern.",LOGMASK_PRGRMERR);
    }
#endif
}

void sat::Calculate_Initial_Data(int Nr)
{
	double Earth_Radius;
	double Eccentricity;
	double Mean_Motion;
	double L8;

	Eccentricity  = Satelliten[Nr].Sat_Eccentricity;
	Mean_Motion   = Satelliten[Nr].Sat_Mean_Motion;

	SMA = R_ERDE * pow(107.08816036 / (2.0 * PI * Mean_Motion), 2.0 / 3.0);
	Precession = 9.95 * pow(R_ERDE / SMA, 3.5) / pow(1.0 - pow(Eccentricity, 2.0), 2.0);
	Earth_Radius = R_ERDE * (1.0 - (F_ERDE / 2.0) * (cos(2.0 * my_Latitude) - 1.0)) + my_QTH_Height / 1000.0;
	L8 = atan((1.0 - F_ERDE) * (1.0 - F_ERDE) * sin(my_Latitude) / cos(my_Latitude));

	Station_Earth_Z = Earth_Radius * sin(L8);
	Station_Earth_X = Earth_Radius * cos(L8) * cos(my_Longitude);
	Station_Earth_Y = Earth_Radius * cos(L8) * sin(-my_Longitude);
}

void sat::Calculate_Times(time_t Track_Time, int Nr)
{
	double Epoch_Day_Time;
	struct tm *Time_Struct;
	double Temp;

	Time_Struct = gmtime(&Track_Time);
	Current_Time = *Time_Struct;

	Track_Day_Time = (double)Year_Conversion(Current_Time.tm_year + 1900) +
			 (double)(Current_Time.tm_yday + 1) +
			 (double)Current_Time.tm_hour / 24.0  +
			 (double)Current_Time.tm_min / 1440.0 +
			 (double)Current_Time.tm_sec / (60.0 * 1440.0);

	Epoch_Day_Time = (double)Year_Conversion(Satelliten[Nr].Sat_Epoch_Year) +
			 Satelliten[Nr].Sat_Epoch_Day;

	Elapsed_Time = Track_Day_Time - Epoch_Day_Time;

	Sidereal_Epoch = (double)((int)(365.25 * (double)(Current_Time.tm_year - 81)) + 366);

	Temp = ((double)Sidereal_Epoch + 29218.5) / 36525.0;
	Sidereal_Constant = (6.6460656 + Temp * (2400.051262 + Temp * 2.581e-5)) / 24.0 - (double)Current_Time.tm_year;
}

void sat::Calculate_Satellite_Position(int Nr)
{
	double Earth_X, Earth_Y, Earth_Z;
	double Orbit_X, Orbit_Y, Orbit_Z;
	double Stars_X, Stars_Y, Stars_Z;
	double Station_X, Station_Y, Station_Z;
	double X8, Y8, Z8;
	double Alat;
	double Alon;
	double C[3][3];
	double Denom;
	double Drag;
	double Drag_Term;
	double Eccentricity;
	double Epoch_A_O_P;
	double Epoch_Eccen;
	double Epoch_MA;
	double Epoch_Orbit;
	double Epoch_RAAN;
	double G7;
	double Inclination;
	double Increment;
	double Integer_Part;
	double Mean_Motion;
	double Orbital_Phase;

	Drag        = Satelliten[Nr].Sat_Drag;
	Inclination = RAD(Satelliten[Nr].Sat_Inclination);
	Epoch_Orbit = (double)Satelliten[Nr].Sat_Epoch_Orbit;
	Epoch_A_O_P = Satelliten[Nr].Sat_Arg_Of_Perigee;
	Epoch_Eccen = Satelliten[Nr].Sat_Eccentricity;
	Epoch_MA    = Satelliten[Nr].Sat_Mean_Anomoly;
	Epoch_RAAN  = Satelliten[Nr].Sat_RAAN;
	Mean_Motion = Satelliten[Nr].Sat_Mean_Motion;

	RAAN = RAD(Epoch_RAAN - (Elapsed_Time * Precession * cos(Inclination)));

	Arg_Of_Perigee = RAD(Epoch_A_O_P + (Elapsed_Time * Precession * (2.5 * pow(cos(Inclination), 2.0) - 0.5)));

	C[0][0] =  cos(Arg_Of_Perigee) * cos(RAAN) - sin(Arg_Of_Perigee) * sin(RAAN) * cos(Inclination);
	C[0][1] = -sin(Arg_Of_Perigee) * cos(RAAN) - cos(Arg_Of_Perigee) * sin(RAAN) * cos(Inclination);
	C[0][2] =  sin(RAAN) * sin(Inclination);
	C[1][0] =  cos(Arg_Of_Perigee) * sin(RAAN) + sin(Arg_Of_Perigee) * cos(RAAN) * cos(Inclination);
	C[1][1] = -sin(Arg_Of_Perigee) * sin(RAAN) + cos(Arg_Of_Perigee) * cos(RAAN) * cos(Inclination);
	C[1][2] = -cos(RAAN) * sin(Inclination);
	C[2][0] =  sin(Arg_Of_Perigee) * sin(Inclination);
	C[2][1] =  cos(Arg_Of_Perigee) * sin(Inclination);
	C[2][2] =  cos(Inclination);

	Drag_Term = (((Drag / Mean_Motion) / 3.0) * Elapsed_Time);

	Orbital_Phase = Mean_Motion * Elapsed_Time * (1.0 + 3.0 * Drag_Term) + (Epoch_MA / 360.0) + Epoch_Orbit;
	Orbital_Phase = modf(Orbital_Phase, &Orbit_No);

	MA = (int)(Orbital_Phase * 256.0);

	Mean_Anomoly = 2.0 * PI * Orbital_Phase;

	Eccentricity = Mean_Anomoly + Epoch_Eccen * (sin(Mean_Anomoly) + 0.5 * Epoch_Eccen * sin(Mean_Anomoly * 2.0));

	do {
	  Denom     = 1.0 - Epoch_Eccen * cos(Eccentricity);
 	  Increment = Eccentricity - Epoch_Eccen * sin(Eccentricity) - Mean_Anomoly;
	  Eccentricity -= Increment / Denom;
	} while (fabs(Increment) > 0.00001);

	Orbit_X = SMA * (cos(Eccentricity) - Epoch_Eccen) * (1.0 - 4.0 * Drag_Term);
	Orbit_Y = SMA * sqrt(1.0 - Epoch_Eccen * Epoch_Eccen) * sin(Eccentricity) * (1.0 - 4.0 * Drag_Term);
	Height  = SMA * Denom * (1.0 - 4.0 * Drag_Term);

	Stars_X = Orbit_X * C[0][0] + Orbit_Y * C[0][1];
	Stars_Y = Orbit_X * C[1][0] + Orbit_Y * C[1][1];
	Stars_Z = Orbit_X * C[2][0] + Orbit_Y * C[2][1];

	G7 = 2.0 * PI * modf((Track_Day_Time - Sidereal_Epoch) * 1.0027379 + Sidereal_Constant, &Integer_Part);

	Earth_X = Stars_X *  cos(G7) - Stars_Y * -sin(G7);
	Earth_Y = Stars_X * -sin(G7) + Stars_Y *  cos(G7);
	Earth_Z = Stars_Z;

	Station_X = Earth_X - Station_Earth_X;
	Station_Y = Earth_Y - Station_Earth_Y;
	Station_Z = Earth_Z - Station_Earth_Z;

	Range = sqrt(Station_X * Station_X + Station_Y * Station_Y + Station_Z * Station_Z);

	Z8 =  Station_X * cos(my_Longitude) * cos(my_Latitude) + Station_Y * sin(-my_Longitude) * cos(my_Latitude) + Station_Z * sin(my_Latitude);
	X8 = -Station_X * cos(my_Longitude) * sin(my_Latitude) - Station_Y * sin(-my_Longitude) * sin(my_Latitude) + Station_Z * cos(my_Latitude);
	Y8 =  Station_Y * cos(my_Longitude) - Station_X * sin(-my_Longitude);

	Azimuth   = DEG(atan(Y8 / X8));
	Elevation = DEG(asin(Z8 / Range));

	if (X8 < 0.0)       Azimuth += 180.0;
	if (Azimuth < 0.0)  Azimuth += 360.0;

	Half_Angle = DEG(acos(R_ERDE / Height));
	SSP_Lat    = DEG(asin(Earth_Z / Height));
	SSP_Long   = DEG(-atan(Earth_Y / Earth_X));

	if (Earth_X < 0.0)  SSP_Long += 180.0;
	if (SSP_Long < 0.0) SSP_Long += 360.0;

	if (Satelliten[Nr].Sat_Aflag) 
	{
		Alon = RAD(Satelliten[Nr].Sat_Alon);
		Alat = RAD(Satelliten[Nr].Sat_Alat);

		Orbit_X = -cos(Alat) * cos(Alon);
		Orbit_Y = -cos(Alat) * sin(Alon);
		Orbit_Z = -sin(Alat);

		Stars_X = Orbit_X * C[0][0] + Orbit_Y * C[0][1] + Orbit_Z * C[0][2];
		Stars_Y = Orbit_X * C[1][0] + Orbit_Y * C[1][1] + Orbit_Z * C[1][2];
		Stars_Z = Orbit_X * C[2][0] + Orbit_Y * C[2][1] + Orbit_Z * C[2][2];

		Earth_X = Stars_X *  cos(G7) - Stars_Y * -sin(G7);
		Earth_Y = Stars_X * -sin(G7) + Stars_Y *  cos(G7);
		Earth_Z = Stars_Z;		

		Squint = DEG(acos(-((Station_X / Range) * Earth_X +
				    (Station_Y / Range) * Earth_Y +
				    (Station_Z / Range) * Earth_Z)));
	} 
	else 
	{
		Squint = 90.0 - Elevation - DEG(acos(sin(my_Latitude) * sin(RAD(SSP_Lat)) + cos(my_Latitude) * cos(RAD(SSP_Lat)) * cos(my_Longitude - RAD(SSP_Long))));
	}

	if (Squint < 0.0) Squint += 180.0;
}

double sat::Conv_Digits(char *Buffer, int n)
{
	char Temp[20];
	strncpy(Temp, Buffer, n);
	Temp[n] = '\0';
	return atof(Temp);
}

int sat::Year_Conversion(int Year)
{
	int Day = 0;
	for (int i = 1980; i < Year; i++)
		Day += 365 + (i % 4 == 0 && (i % 100 != 0 || i % 400 == 0));
	return Day;
}

double sat::RAD(double angle)
{
	return (angle / 180.0) * PI;
}

double sat::DEG(double angle)
{
	return (angle / PI) * 180.0;
}

bool sat::sat_in_kepler(String SatName)
{
	char Buffer[101];
	FILE *fp;
	if ((fp = fopen(kepler_file, "r")) != NULL)
        {
    	    while (fgets(Buffer, 100, fp) != NULL)
            {
		if (String(Buffer).in(SatName))
                {
		    fclose(fp);
		    return true;
		}
	    }
	fclose(fp);
	}
	return false;
}
