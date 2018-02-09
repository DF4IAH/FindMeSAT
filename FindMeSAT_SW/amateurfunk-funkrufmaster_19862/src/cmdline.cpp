/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000-2004 by Holger Flemming                               *
 *                                                                          *
 * Thist Program is free software; yopu can redistribute ist and/or modify  *
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

#include "cmdline.h"


#include <stdlib.h>

// Konstruktor aus Komandozeichenparametern
// int argc und char *argv[]

cmdline::cmdline( int c, char **v)
{
  argc = c;
  argv = v;
}

bool cmdline::exist( int i )
{
  return ( i < argc) ; // Ueberpruefen ob der Prameter Nr. i noch existiert
}

// Liefert einfach den Parameter i als char* zurueck.
char *cmdline::parameter( int i )
{
  if (exist(i))
    return argv[i];
  else
    return NULL;
}

char *cmdline::option( char p )
{
  char *help = NULL;
  for (int i = 1;i<argc;i++) // alle Parameter werden durchlaufen
    {
      char* par = argv[i];   // Zeiger auf Parameterstring holen
      if (*(par++) == '-')   // Zeichen = '-' ?
	if (*(par++) == p )  // entspricht naechstes Zeichen dem gewuenschten
	  {
	    if (*par == '=') par++; // naechstes Zeichen = '=' ?
	    help = par;             // Dann Zeiger auf den REst uebergeben
	  }
    }
  return help;
}

// Ueberpruefen ob der in p genannte Switch in der Kommandozeile vorhanden ist
bool cmdline::pswitch( char p ) 
{
  bool help = false;
  for (int i = 1;(i<argc) && !help;i++)  // Alle Parameter durchgehen
    {
      char *par = argv[i];    // Zeiger auf Parameterstring holen
      if (*(par++) == '-')    // erstes zeichen gleich '-' ?
	if (*(par++) == p )   // Gewuenschtes zeichen ?
	  help = true;        // Ja, dann ist switch vorhanden
    }
  return help;
}
