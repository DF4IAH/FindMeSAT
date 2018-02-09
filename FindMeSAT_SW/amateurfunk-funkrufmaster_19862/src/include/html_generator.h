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
 *  		                                			    *
 ****************************************************************************/

#ifndef __HTML_GENERATOR_H__
#define __HTML_GENERATOR_H__

#include <vector>

#include "globdef.h"
#include "lang_support.h"
#include "user.h"
#include "config.h"
#include "String.h"
#include "gruppe.h"
#include "board.h"
#include "bake.h"
#include "passwd.h"
#include "slaves.h"

#include "html_objects.h"
#include "html_tabular.h"
#include "html_forms.h"

using namespace std;

enum mime_type { mime_none, mime_jpg, mime_gif, mime_bin, mime_wav, 
		 mime_txt, mime_html };

class Error_html_generator_error
{
  html_object error_object;

 public:
  Error_html_generator_error( const html_object &o)
    {
      error_object = o;
#ifdef _DEBUG_EXEC_
      cerr << "Error_html_generator_error" << endl;
      cerr << error_object.write() << endl;
#endif
    }
  html_object get_object( void )
    {
      return error_object;
    }
};

class Error_html_generator_permission_violation
{

 public:
  Error_html_generator_permission_violation(  void )
    {
#ifdef _DEBUG_EXEC_
      cerr << "Error_html_generator_permission_violation" << endl;
#endif
    }
};

class Error_html_generator_command_unknown
{

 public:
  Error_html_generator_command_unknown(  void )
    {
#ifdef _DEBUG_EXEC_
      cerr << "Error_html_generator_command_unknown" << endl;
#endif
    }
};

class html_generator
{
 protected:
  user_meldungen mldg;
  user usr;
  gruppen grps;
  String headline;
  String content;
  uint32_t sock_adr;
  consolpw cpw;
  bool axhtp_flag;

  bool bg_image;
  String bg_url;
  String bgcolor;

 protected:
  vector<String> blank_separated( String   );
  void get_content( const String &);
  void get_postarea( const String &, String&  );
  html_object standard_frame( const String &, const html_object &, const String & = "" );

  const String & frame( void );
  const String& homepage( const String & );
  const String &login( void );
  const String &info( void );
  const String &aktuell( void );
  const String &exec( const String &, t_baken & );
  const String &help( const String & );
  const String &do_help( const String &);
  const String &logins( void );

  const String &addboardop( board  );
  const String &delboardop( board  );
  const String &addboardop_form( board  );
  const String &delboardop_form( board  );
  const String &directory( void );

  html_object directory_head( board  );
  html_object directory_ops( board  );
  html_object directory_entry_head( board, int  );
  void split_entry_line( const String &, String &, String &, 
			 String &, String & );
  html_object directory_entry_static( board , int  );
  html_object directory_entry_dynamic( board , int  );
  html_object directory_change_forms( board  );
  const String &directory( board , bool );
  const String &change_board( board  );
  const String &do_directory( const String &url );
  const String &groups( void );
  const String &groups( gruppe  );
  const String & do_groups( const String url );
  const String &destinations( void );
  const String &destinations( const destin & );
  const String &forward( void );
  const String &forward( const callsign & );
  const String & do_forward( const String & );
  const String & logfiles( logfile &, int , const String &,  char , zeit  );
  const String & do_log( void );
  const String & do_logform( void );
  void slave_simple( const slave & , html_tabular & );
  html_tabular slave_extended( const slave & );
  html_tabular slavetab( vector<slave>  );
  const String & do_slaves( const String &  );
  const String & do_sendok( t_baken & baken );
  html_form_select destination_form( void );
  const String & sendform( void );
  const String & do_passwd( void );
  const String & do_setpw( void );
  const String &do_passwdform( void );
  const String &do_sysopform( void );
  const String & do_syspw( void );
  const String &do_logoff( void );
  html_tabular user_tab( vector<database_entry> , bool  );
  const String & userinfo( const database_entry &, bool  );
  const String & userinfo( vector<database_entry>, bool  );
  const String & do_user( void );
  const String & do_userform( void );
  const String & status_phrase( int );
  html_tabular profile_entry_tab( const html_object& , const html_object&  );
  const String & profile_form( const callsign &  );
  const String & do_profile( const String &  );
  const String & setprofile( const callsign &  );
  const String & do_setprofile( const String &  );

  int do_file( const String &, String &, mime_type & , zeit& );

 public:
  html_generator( config_file &);

  void set_user( const user& );
  void set_head( const String & );
  void set_cpw ( const consolpw & );

  int do_url( const String&, String&, mime_type& , int& ,zeit&, t_baken&);
  void do_error_msg(int ,String & ,mime_type&, int&, zeit& );

};


#endif // __HTML_GENERATOR_H__
