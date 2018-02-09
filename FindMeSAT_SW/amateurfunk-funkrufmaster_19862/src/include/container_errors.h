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

#ifndef __CONTAINER_ERRORS_H__
#define __CONTAINER_ERRORS_H__

#include <iostream>

using namespace std;

class Error_no_set_Specified_in_iterator
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_set_Specified_in_iterator()
    {
      cerr << "Error_no_set_Specified_in_iterator" << endl;
    }
#endif
};

class Error_index_out_of_range
{
#ifdef _DEBUG_EXEC_
 public:
  Error_index_out_of_range()
    {
      cerr << "Error_index_out_of_range" << endl;
    }
#endif
};

class Error_no_map_Specified_in_iterator
{
#ifdef _DEBUG_EXEC_
 public:
  Error_no_map_Specified_in_iterator()
    {
      cerr << "Error_no_map_Specified_in_iterator" << endl;
    }
#endif
};

class Error_iterator_points_to_different_container
{
#ifdef _DEBUG_EXEC_
 public:
  Error_iterator_points_to_different_container()
    {
      cerr << "Error_iterator_points_to_different_container" << endl;
    }
#endif
};

#endif
