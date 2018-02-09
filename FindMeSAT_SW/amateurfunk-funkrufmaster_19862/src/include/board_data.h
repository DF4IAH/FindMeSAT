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

#ifndef __BOARD_DATA_H__
#define __BOARD_DATA_H__

#include "globdef.h"
#include "String.h"


#define BID_BAKE       95




struct board_data_entry
{
  String name;
  int board_id;
  int def_lifetime;
  int def_tx_intv;
};


struct board_data_entry board_daten[100] =
{
//  Name            Nr  DefLt  TX_Repeat
  { "Wichtig"      , 1 , 365 , 43200 },
  { "Info"         , 2 , 365 , 43200 },
  { "Aktuell"      , 3 ,  30 , 43200 },
  { RUB_DXKWCW     , 4 ,   0 ,     0 },
  { RUB_DXKW       , 5 ,   0 ,     0 },
  { RUB_DX50       , 6 ,   0 ,     0 },
  { RUB_DXVUSHF    , 7 ,   0 ,     0 },
  { RUB_DXWX       , 8 ,   0 ,     0 },
  { "Lokales"      , 9 ,  60 , 43200 },
  { "Relais-10m"   ,10 , 365 ,  3600 },
  { "Relais-2m"    ,11 , 365 ,  3600 },
  { "Relais-70cm"  ,12 , 365 ,  3600 },
  { "Relais-23cm"  ,13 , 365 ,  3600 },
  { "Digipeater"   ,14 ,   7 ,  3600 },
  { "Termine"      ,15 ,  30 , 86400 },
  { "ATV"          ,16 , 365 , 86400 },
  { "SSTV"         ,17 , 365 , 86400 },
  { "PSK31"        ,18 , 365 , 86400 },
  { RUB_WSJT       ,19 , 365 , 86400 },
  { "Distrikte"    ,20 ,  60 , 86400 },
  { "Distrikt-A"   ,21 ,  60 , 86400 },
  { "Distrikt-B"   ,22 ,  60 , 86400 },
  { "Distrikt-C"   ,23 ,  60 , 86400 },
  { "Distrikt-D"   ,24 ,  60 , 86400 },
  { "Distrikt-E"   ,25 ,  60 , 86400 },
  { "Distrikt-F"   ,26 ,  60 , 86400 },
  { "Distrikt-G"   ,27 ,  60 , 86400 },
  { "Distrikt-H"   ,28 ,  60 , 86400 },
  { "Distrikt-I"   ,29 ,  60 , 86400 },
  { "Distrikt-K"   ,30 ,  60 , 86400 },
  { "Distrikt-L"   ,31 ,  60 , 86400 },
  { "Distrikt-M"   ,32 ,  60 , 86400 },
  { "Distrikt-N"   ,33 ,  60 , 86400 },
  { "Distrikt-O"   ,34 ,  60 , 86400 },
  { "Distrikt-P"   ,35 ,  60 , 86400 },
  { "Distrikt-Q"   ,36 ,  60 , 86400 },
  { "Distrikt-R"   ,37 ,  60 , 86400 },
  { "Distrikt-S"   ,38 ,  60 , 86400 },
  { "Distrikt-T"   ,39 ,  60 , 86400 },
  { "Distrikt-U"   ,40 ,  60 , 86400 },
  { "Distrikt-V"   ,41 ,  60 , 86400 },
  { "Distrikt-W"   ,42 ,  60 , 86400 },
  { "Distrikt-X"   ,43 ,  60 , 86400 },
  { "Distrikt-Y"   ,44 ,  60 , 86400 },
  { "Wetter"       ,45 ,   7 ,     0 },
  { "RegTP"        ,46 , 365 , 86400 },
  { "DARC"         ,47 , 365 , 86400 },
  { "VFDB"         ,48 , 365 , 86400 },
  { "Funkruf"      ,49 ,  30 ,  3600 },
  { "Amsat"        ,50 , 365 , 86400 },
  { "Satelliten"   ,51 ,   7 ,     0 },
  { "EMV-EMVU"     ,52 , 365 , 86400 },
  { "Contest"      ,53 ,  30 , 86400 },
  { "EME"          ,54 , 365 , 86400 },
  { "ARDF"         ,55 ,   7 , 86400 },
  { RUB_ASTRO      ,56 ,   7 ,     0 },
  { "APRS"         ,57 ,   7 ,     0 },
  { RUB_GEZEITEN   ,58 ,   7 ,     0 },
  { "Baken"        ,59 , 365 ,  3600 },
  { RUB_DIGI_STAT  ,91 ,   7 ,     0 },
  { RUB_LINK_STAT  ,92 ,   7 ,     0 },
  { RUB_STATISTIK  ,93 ,   7 ,     0 },
  { RUB_STATUSBAKE ,94 ,   7 ,     0 },
  { RUB_BAKE       ,BID_BAKE, 365, 0 },
  { "Ende"         ,0  , 365 ,     0 }
};



#endif // __BOARD_DATA_H__
