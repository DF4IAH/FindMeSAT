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
 *                                                                          *
 ****************************************************************************/

#include <sys/stat.h>

#include "html_generator.h"
#include "html_align.h"
#include "html_frames.h"
#include "html_forms.h"
#include "html_references.h"
#include "html_tabular.h"
#include "html_paartags.h"

#include "database.h"
#include "fwd_frontend.h"

extern callsign_database calls;
extern callsign G_mycall;

const String & html_generator::userinfo( const database_entry &daten, bool all_flag )
{
  static String out;
  html_object body;

  html_tabular tab;

  tab.set_border(2);
  tab.set_spacing(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1122));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_adr().adr());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1123));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_name());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1124));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_locator().str());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1125));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_geraet().get_string());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1126));
  zeit tm = daten.get_last_change();
  tm.set_darstellung(zeit::f_zeitdatum_l);
  tab.add_cell(html_align(html_align::al_left),false,tm.get_zeit_string());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1127));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_server().call());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1128));
  tab.add_cell(html_align(html_align::al_left),false,daten.get_language());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1129));
  tm = daten.get_last_login();
  if (tm != zeit(-1))
    {
      tm.set_darstellung(zeit::f_zeitdatum_l);
      tab.add_cell(html_align(html_align::al_left),false,tm.get_zeit_string());
    }
  else
    {
      tab.add_cell(html_align(html_align::al_left),true,mldg.find(1130));
    }
  tab.add_to_body();

  body.add(tab);

  if (usr.is_sysop() || samecall(daten.get_call(),usr.user_call()) )
    {
      html_link li1(String("/profile/")+daten.get_call().call(),"txt",mldg.find(1199));
      body.add(li1);
    }

  html_italic it2;
  it2.add(mldg.find(1132));
  
  html_ueberschrift u2(2);
  u2.add(it2);
  
  body.add(u2);

  vector<database_entry> daten2 = calls.get_other_pager(daten.get_call());
  body.add(user_tab(daten2,all_flag));

  html_object frame = standard_frame(mldg.find(1121)+String(" ")+daten.get_call().call(),body);
  out = frame.write();
  return out; 
  
  
}

html_tabular html_generator::user_tab( vector<database_entry> daten, bool all_flag )
{
  html_tabular tab;

  tab.set_border(2);
  tab.set_spacing(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1132));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1133));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1134));
  tab.add_cell(html_align(html_align::al_center),true,mldg.find(1135));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1136));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1137));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1138));
  tab.add_to_body();

  for (vector<database_entry>::iterator it = daten.begin(); it != daten.end(); ++it )
    {

      tab.new_row();

      callsign call = it->get_call();
      call.set_format(true);
      call.set_nossid(false);

      String cform = call.call();
      call.set_format(false);
      String cunform = call.call();

      if (all_flag)
	tab.add_cell(html_align(html_align::al_left),false,html_link(String("/user?all=1&part=")+cunform,"txt",cform));
      else
	tab.add_cell(html_align(html_align::al_left),false,html_link(String("/user?all=0&part=")+cunform,"txt",cform));
      tab.add_cell(html_align(html_align::al_left),false,it->get_adr().adr());
      tab.add_cell(html_align(html_align::al_left),false,it->get_geraet().get_string());
      tab.add_cell(html_align(html_align::al_center),false,it->get_locator().str());
      tab.add_cell(html_align(html_align::al_left),false,it->get_name());
      zeit last_change = it->get_last_change();
      last_change.set_darstellung(zeit::f_datum_s);
      call = it->get_server();  
      call.set_format(true);
      call.set_nossid(true);
      
      tab.add_cell(html_align(html_align::al_left),false,last_change.get_zeit_string());
      tab.add_cell(html_align(html_align::al_left),false,call.call());
      tab.add_to_body();
    }

  return tab;
}


const String & html_generator::userinfo( vector<database_entry> daten, bool all_flag )
{
  static String out;

  html_object frame = standard_frame(mldg.find(1139),user_tab(daten,all_flag));
  out = frame.write(); 
  
  return out;
}


 
const String & html_generator::do_user( void )
{
  static String out;
  String part;
  String all;
  get_postarea("part=",part);
  get_postarea("all=",all);
  part.upcase();


  bool all_flag = ( all == "1" );
  try
    {
      callsign call(part);
      database_entry daten = calls.find(call);
      return userinfo(daten,all_flag);
    }
  catch( Error_callsign_does_not_exist )
    {
      vector<database_entry> daten = calls.get_user(part,all_flag);
      return userinfo(daten,all_flag);
    }
  catch( Error_no_callsign )
    {
      vector<database_entry> daten = calls.get_user(part,all_flag);
      return userinfo(daten,all_flag);
    }
}

const String & html_generator::do_userform( void )
{
  static String out;
  
  html_form form(html_form::m_get,"/user");
  
  html_tabular tab;
  tab.set_border(0);
  tab.set_spacing(3);
  
  tab.new_row();
  tab.add_cell(html_align(html_align::al_right),false,mldg.find(1119));
  
  html_form_check cb1("all","1","");
  tab.add_cell(html_align(html_align::al_left),false,cb1);
  tab.add_to_body();
  
  tab.new_row();
  tab.add_cell(html_align(html_align::al_right),false,mldg.find(1120));
  
  html_form_text_input ti("part","",9);
  tab.add_cell(html_align(html_align::al_left),false,ti);
  tab.add_to_body();
  
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,html_form_submit(mldg.find(1121)));
  tab.add_cell(html_align(html_align::al_left),false,"");
  tab.add_to_body();
  

  form.add(tab);
  
  html_object frame = standard_frame(mldg.find(1180),form);
  out = frame.write(); 
  
  return out;
}

html_tabular html_generator::profile_entry_tab( const html_object &o1, const html_object &o2 )
{
  html_tabular tab;
  tab.set_border(0);
  tab.set_spacing(3);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,o1);
  tab.add_to_body();
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,o2);
  tab.add_to_body();
 
  return tab;
}

const String & html_generator::profile_form( const callsign & call )
{
  static String out;

  if (usr.is_sysop() || samecall(call,usr.user_call()) )
    {
      
      html_form form(html_form::m_get,String("/setprofile/")+call.call());
      
      html_tabular tab;
      tab.set_border(0);
      tab.set_spacing(20);
      
      try
	{
	  database_entry ent = calls.find(call);
	  tab.new_row();
	  
	  tab.add_cell(html_align(html_align::al_left),false,profile_entry_tab( mldg.find(1133), ent.get_adr().adr() ) );
	  
	  html_tabular servertab;
	  servertab.set_border(0);
	  servertab.set_spacing(3);
	  
	  servertab.new_row();
	  servertab.add_cell(html_align(html_align::al_left),false,mldg.find(1137));
	  servertab.add_cell(html_align(html_align::al_left),false,ent.get_last_change().get_zeit_string());
	  servertab.add_to_body();
	  servertab.new_row();
	  servertab.add_cell(html_align(html_align::al_left),false,mldg.find(1138));
	  servertab.add_cell(html_align(html_align::al_left),false,ent.get_server().call());
	  servertab.add_to_body();
	  
	  tab.add_cell(html_align(html_align::al_left),false,servertab);
	  tab.add_to_body();
	  
	  tab.new_row();
	  html_form_select pgtyp("typ");
	  pgtyp.add_option("NO DEFINED","NO DEFINED");
	  pgtyp.add_option("UNKNOWN0k5","UNKNOWN0k5");
	  pgtyp.add_option("UNKNOWN1k2","UNKNOWN1k2");
	  pgtyp.add_option("UNKNOWN2k4","UNKNOWN2k4");
	  pgtyp.add_option("SKYPER","SKYPER");
	  pgtyp.add_option("QUIX","QUIX");
	  pgtyp.add_option("TELMI","TELMI");
	  pgtyp.add_option("CITYRUF","CITYRUF");
	  pgtyp.add_option("PRIMO","PRIMO");
	  pgtyp.add_option("SCALL","SCALL");
	  pgtyp.select_option(ent.get_geraet().get_string());
	  
	  tab.add_cell(html_align(html_align::al_left),false,profile_entry_tab( mldg.find(1134), pgtyp ) );
	  
	  html_form_text_input locfrm("loc",ent.get_locator().str(),6);
	  tab.add_cell(html_align(html_align::al_left),false,profile_entry_tab( mldg.find(1135), locfrm ) );
	  tab.add_to_body();
	  
	  tab.new_row();
	  html_form_text_input namefrm("name",ent.get_name(),20);
	  tab.add_cell(html_align(html_align::al_left),false,profile_entry_tab( mldg.find(1136), namefrm ) );
	  html_form_text_input langfrm("lang",ent.get_language(),3);
	  tab.add_cell(html_align(html_align::al_left),false,profile_entry_tab( mldg.find(1128), langfrm ) );
	  tab.add_to_body();

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,html_form_submit(mldg.find(1197)));
	  tab.add_cell(html_align(html_align::al_right),false,html_form_reset(mldg.find(1177)));
	  tab.add_to_body();

	  form.add(tab);
	  
	  html_object frame = standard_frame(mldg.find(1194)+String(' ')+call.call(),form);
	  out = frame.write(); 
	}
      catch( Error_callsign_does_not_exist )
	{
	  html_object frame = standard_frame(mldg.find(1194)+String(' ')+call.call(),mldg.find(1195));
	  out = frame.write(); 
	}
    }
  else
    {
      html_object frame = standard_frame(mldg.find(1194)+String(' ')+call.call(),mldg.find(1196));
      out = frame.write(); 
    }

  return out;
}

const String & html_generator::do_profile( const String & url )
{
  if (url.in("/profile/"))
    {
      unsigned int p = url.pos("/profile/");
      if (url.slen() > p + 9)
	{
	  String cname = url.copy(p+9,url.slen()-p-9);
	  try
	    {
	      callsign pcall(cname);
	      return profile_form(pcall);
	    }
	      catch( Error_no_callsign )
		{
		  throw Error_html_generator_error("Error : No Callsign");
		}
	}
      else
	return profile_form(usr.user_call());
    }
  else
    return profile_form(usr.user_call());
}

const String & html_generator::setprofile( const callsign & call )
{
  static String out;

  if (usr.is_sysop() || samecall(call,usr.user_call()) )
    {
      String typ;
      String loc;
      String name;
      String lang;
      get_postarea("typ=",typ);
      get_postarea("loc=",loc);
      get_postarea("name=",name);
      get_postarea("lang=",lang);

      try
	{
	  database_entry ent = calls.find(call);
	  ent.set_geraet(pager_typ(typ),G_mycall);
	  ent.set_loc(locator(loc),G_mycall);
	  ent.set_name(name,G_mycall);
	  ent.set_language(lang,G_mycall);

	  calls.change(call,ent);

	  fwd_api fwd;
	  fwd.fwd_usr('C',call);

	  usr.name = name;
	  usr.loc = locator(loc);
	  usr.language = lang;
	  return profile_form(call);
	}
      catch( Error_callsign_does_not_exist )
	{
	  throw Error_html_generator_error(String("Error : Callsign not in Database "));
	}
    }
  else
    {
      html_object frame = standard_frame(mldg.find(1194)+String(' ')+call.call(),mldg.find(1196));
      out = frame.write(); 
      return out;
    }
}

const String & html_generator::do_setprofile( const String & url )
{

  if (url.in("/setprofile/"))
    {
      unsigned int p = url.pos("/setprofile/");
      if (url.slen() > p + 12)
	{
	  unsigned int q = p + 12;
	  // Nach naechstem Fragezeichen suchen
	  while (q < url.slen() && url[q] != '?')
	    q++;

	  String cname;
	  if (q < url.slen())
	    cname = url.copy(p+12,q-p-12);
	  else
	    cname = url.copy(p+12,url.slen()-p-12);
	  try
	    {
	      callsign pcall(cname);
	      return setprofile(pcall);
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_html_generator_error(String("Error : No Callsign ")+cname);
	    }
	}
      else
	throw Error_html_generator_error(String("Error : No Callsign "));
    }
  else
    throw Error_html_generator_error(String("Error : No Callsign "));
}
