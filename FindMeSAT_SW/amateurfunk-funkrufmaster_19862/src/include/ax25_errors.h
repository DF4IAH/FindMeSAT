/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2002 by Holger Flemming                                    *
 *                                                                          *
 * This Program is free software; yopu can redistribute ist and/or modify   *
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

#ifndef __AX25_ERRORS_H__
#define __AX25_ERRORS_H__

class Error_ax25_config_load_ports
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_config_load_ports()
    {
      cerr << "Error_ax25_config_load_ports" << endl;
    }
#endif
};

class Error_ax25_config_get_addr
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_config_get_addr()
    {
      cerr << "Error_ax25_config_get_addr" << endl;
    }
#endif
};

class Error_ax25_master_call
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_master_call()
    {
      cerr << "Error_ax25_master_call" << endl;
    }
#endif
};

class Error_ax25_slave_call
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_slave_call()
    {
      cerr << "Error_ax25_slave_call" << endl;
    }
#endif
};

class Error_ax25_l2digi_call
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_l2digi_call()
    {
      cerr << "Error_ax25_l2digi_call" << endl;
    }
#endif
};

class Error_ax25_path_call
{
#ifdef _DEBUG_EXEC_
 public:
  Error_ax25_path_call()
    {
      cerr << "Error_ax25_path_call" << endl;
    }
#endif
};

#endif
