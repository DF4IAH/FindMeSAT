/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2000 by Holger Flemming                                    *
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

#ifndef __CMDLINE__
#define __CMDLINE__


// Die Klasse cmdline dient dazu, Parameter aus der Kommandozeile auszuwerten.
// Es besteht die Moeglichkeit, einen bestimmten Parameter nach der Position in 
// der Kommandozeile auszuliefern oder nach bestimmten Parametern der Form -<o>=<value>
// zu suchen, wobei <o> ein Buchstabe ist, der die gesuchte Option kennzeichnet. 
//
// Eine weitere Moeglichkeit besteht darin, nach switches der Form -<s> zu suchen.
// Hier wird lediglich geprueft ob der angegebene Switch vorhanden ist oder nicht.
//  
class cmdline
{
 private:
  int argc;
  char **argv;

 public:
  cmdline(int , char ** );
  bool exist( int );
  char *parameter( int );
  char *option( char );
  bool pswitch( char );
};


#endif
