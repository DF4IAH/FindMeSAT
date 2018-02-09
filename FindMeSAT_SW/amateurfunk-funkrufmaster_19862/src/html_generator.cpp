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

#include "callsign.h"
#include "texte.h"
#include "user_interface.h"
#include "system_info.h"
#include "makros.h"
#include "database.h"
#include "fwd_autorouter.h"
#include "fwd_router.h"
#include "slaves.h"

extern callsign G_mycall;
extern config_file configuration;
extern callsign_database calls;
extern autorouter a_router;
extern fwd_router router;
extern slave_control slaves;





html_generator::html_generator( config_file &cfg ) : mldg(cfg), grps(cfg), cpw(false)
{
  axhtp_flag = false;
  try
    {
      String bg = cfg.find("HTML_BACKGROUND");
      String bgc = bg;
      if (bg.slen() == 6)
	{
	  bg_image = false;
	  bgc.upcase();

	  for (unsigned int i = 0; i < 6; i++ )
	    if ((bg[i] < '0' || bg[i] > '9') && (bg[i] < 'A' || bg[i] > 'F'))
	      bg_image = true;

	}
      else
	bg_image = true;

      if (bg_image)
	bg_url = bg;
      else
	bgcolor = bgc;
    }
  catch( Error_parameter_not_defined )
    {
      bgcolor = "FFFFFF";
    }
}


void html_generator::set_user( const user &u  )
{
  usr = u;
  mldg.load(usr.language);
}

void html_generator::set_head( const String &hd )
{
  headline = hd;
}

void html_generator::set_cpw ( const consolpw &c )
{
  cpw = c;
}

vector<String> html_generator::blank_separated( String  input )
{
  vector<String> tmp;


  input.kuerze();
  unsigned int length = input.slen();
  unsigned int begin = 0;

  unsigned int i = 0;
  while (i < length )
    {
      if (input[i] == ' ' )
	{
	  String part = input.copy(begin,i-begin);
	  begin = i + 1;
	  while (begin < length && input[begin] == ' ')
	    begin++;
	  i = begin + 1;
	  tmp.push_back(part);
	}
      else
	i++;
    }
  // Jetzt nochmal separat den letzten Teil abtrennen
  if (begin < length)
    {
      String part = input.copy(begin,length-begin);
      tmp.push_back(part);
    }
  else
    {
      // Wenn letztes Zeichen ein Komma ist, dann wird begin == length
      // In diesem Fall leeren String als letzte Komponente anhängen
      tmp.push_back(String());
    }
  return tmp;
}

void html_generator::get_content( const String &url_str )
{
  int index = url_str.pos('?');

  if (index >= 0)
    {
      content = url_str.copy(index+1,url_str.slen() - index - 1);
    }
}

void html_generator::get_postarea( const String &tag, String &res )
{
  res = String("");
  if (content.slen() > 0)
    {
      int ind = content.pos(tag);
      unsigned int index;
      if (ind >= 0)
	{
	  index = (unsigned int) ind;
	  index += tag.slen();
	  while (index < content.slen() && content[index] != '&')
	    {
	      if (content[index] == '+')
		res.append(' ');
	      else if (content[index] == '%' && index < content.slen()-3)
		{
		  char hex[3];
		  unsigned val;
		  hex[0] = content[index+1];
		  hex[1] = content[index+2];
		  hex[2] = (char) 0;
		  sscanf(hex,"%2X",&val);
		  index += 2;
		  if (val == 0x0d)
		    {
		      index++;
		      continue;
		    }
		  res.append(String((char) val));
		}
	      else
		res.append(String(content[index]));
	      index++;
	    }
	}
    }
}


html_object html_generator::standard_frame( const String &ueberschr, const html_object &obj, const String &col )
{
  html_body body;

  if (col.slen() > 0)
    body.set_bgcolor(col);
  else if (bg_image)
    body.set_bg_url(bg_url);
  else
    body.set_bgcolor(bgcolor);

  if (ueberschr.slen() > 0)
    {
      html_italic it;
      it.add(ueberschr);
      
      html_ueberschrift u1(1);
      u1.add(it);
      
      body.add(u1);
    }
  body.add(obj);
  html_title title(headline);
  html_head hd;
  
  hd.add(title);
  
  html base;
  base.add(hd);
  base.add(body);

  return base;
}

const String & html_generator::frame( void )
{
  static String out;

  html_frames fr(true);
  fr.new_frame(200,true,sc_no,"/command","cmd");
  fr.new_frame(-1,true,sc_auto,"/login","txt");

  html_body body;
  if (bg_image)
    body.set_bg_url(bg_url);
  else
    body.set_bgcolor(bgcolor);
  body.add(mldg.find(1086));

  html_noframe nf;
  nf.add(body);

  html_title title(headline);
  html_head hd;

  hd.add(title);
  hd.add(fr);

  html base;
  base.add(hd);
  base.add(nf);

  out = base.write(); 

  return out;
}


const String& html_generator::homepage( const String &cmd )
{
  static String out;
  String tar = " target=\"txt\"";

  html_object body;

  body.add(String("<B><font size=+2> FunkrufMaster - WEB-Interface Version "));
  body.add(String(VERSION)+"</font>\n");
  body.add(String("<font size=0> GPL &copy; by DH4DAI and DH6BB </font> <p>\n"));
  body.add(String("<font size=+2> <font color=\"#ff0000\">"));
  body.add(usr.user_call().call()+" de "+G_mycall.call());
  body.add(String("</font></font>\n"));

  html_form input_form(html_form::m_get,"/cmd");
  input_form.set_target("txt");
  input_form.add(mldg.find(1000));

  html_form_text_input inp("cmd",cmd,40);
  input_form.add(inp);

  html_form_submit sub(mldg.find(1001));
  input_form.add(sub);
  body.add(input_form);

  html_link l1("/","_parent",mldg.find(1002));
  body.add(l1);
  body.add("|");
  html_link l2("/aktuell","txt",mldg.find(1003));
  body.add(l2);
  body.add("|");
  html_link l3("/send","txt",mldg.find(1004));
  body.add(l3);
  body.add("|");
  html_link l4("/logins","txt",mldg.find(1005));
  body.add(l4);
  body.add("|");
  html_link l5("/dir","txt",mldg.find(1006));
  body.add(l5);
  body.add("|");
  html_link l6("/grp","txt",mldg.find(1007));
  body.add(l6);
  body.add("|");
  html_link l7("/dest","txt",mldg.find(1008));
  body.add(l7);
  body.add("|");
  html_link l8("/fwd","txt",mldg.find(1009));
  body.add(l8);
  body.add("|");
  html_link l9("/help","txt",mldg.find(1010));
  body.add(l9);
  body.add("|");
  html_link l10("/info","txt",mldg.find(1011));
  body.add(l10);
  body.add("|");
  html_link l11("/log","txt",mldg.find(1012));
  body.add(l11);
  body.add("|");
  html_link l12("/slave","txt",mldg.find(1013));
  body.add(l12);
  body.add("|");
  html_link l13("/cmd?cmd=v+*","txt",mldg.find(1014));
  body.add(l13);
  body.add("|");
  html_link l14("/userf","txt",mldg.find(1015));
  body.add(l14);
  body.add("|");
  html_link l14b("/profile","txt",mldg.find(1198));
  body.add(l14b);
  body.add("|");
  html_link l15("/passwd","txt",mldg.find(1016));
  body.add(l15);
  body.add("|");
  if (usr.is_sysop())
    {
      html_link l16("/logoff","txt",mldg.find(1017));
      body.add(l16);
    }
  else
    {
      html_link l17("/sysop","txt",mldg.find(1018));
      body.add(l17);
    }

  body.add(String("<hr>\n"));

  html_object frame = standard_frame("",body);
  out = frame.write();

  return out;
}

const String &html_generator::login( void )
{
  static String out;
  makros mak(usr);

  htctext ct(configuration,mak,usr.language);
  html_object frame = standard_frame(G_mycall.call()+" Login",ct.get());

  out = frame.write(); 

  return out;
}

const String &html_generator::info( void )
{
  static String out;
  makros mak(usr);

  htitext it(configuration,mak,usr.language);
  html_object frame = standard_frame(mldg.find(1140),it.get());
  out = frame.write(); 

  return out;
}

const String &html_generator::aktuell( void )
{
  static String out;
  makros mak(usr);

  htatext at(configuration,mak,usr.language);
  html_object frame = standard_frame(mldg.find(1141),at.get());
  out = frame.write(); 

  return out;
}

const String &html_generator::help( const String &cmd )
{
  static String out;
  makros mak(usr);

  htext ht(configuration,mak,usr.language);
  vector<struct h_eintrag> ent = ht.suche(cmd,usr);

  html_tabular tab;

  tab.set_border(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1143));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1144));
  tab.add_to_body();

  for (vector<struct h_eintrag>::iterator i = ent.begin(); i != ent.end(); ++i)
    {
      tab.new_row();
      html_link lnk(String("/help/")+i->cmd,"txt",i->cmd);
      tab.add_cell(html_align(html_align::al_left),true,lnk);

      html_object cell;
      for ( vector<String>::iterator j = i->htxt.begin(); j != i->htxt.end(); ++j)
	cell.add(line_text2html(*j)+String("<br>"));

      tab.add_cell(html_align(html_align::al_left),false,cell);
      tab.set_rowalign(html_valign(html_valign::val_top));
      tab.add_to_body();
    }
  html_object frame = standard_frame(mldg.find(1142)+' '+cmd,tab);

  out = frame.write(); 

  return out;
}


const String &html_generator::do_help( const String &url )
{
  if (url.in("/help/"))
    {
      unsigned int p = url.pos("/help/");
      if (url.slen() > p + 6)
	{
	  String cmd = url.copy(p+6,url.slen()-p-6);
	  return help(cmd);
	}
      else
	return help("");
    }
  else
    return help("");
}

const String &html_generator::exec( const String &cmd, t_baken &baken )
{
  String dummy;
  String intf_output;
  static String out;

  html_object body;

  body.add(String("<hr>\n"));
  body.add(String("<p><font size=+2>")+mldg.find(1019)+" "+cmd);
  body.add(String("</font></p>\n"));
  body.add(String("<pre>\n"));
  user_interface ui(dummy,usr,false,baken);
  ui.process(cmd+'\n',intf_output);
  body.add(intf_output);
  body.add(String("</pre>\n"));

  html_object frame = standard_frame(G_mycall.call()+":",body);
  out = frame.write(); 
  ui.set_io('H',0,0);

  return out;
}

const String &html_generator::logins( void )
{
  static String out;

  html_tabular tab;
  tab.set_border(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,String("PID"));
  tab.add_cell(html_align(html_align::al_center),true,String(""));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1020));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1026));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1021));
  tab.add_cell(html_align(html_align::al_right),true,mldg.find(1022));
  tab.add_cell(html_align(html_align::al_right),true,mldg.find(1023));
  tab.add_cell(html_align(html_align::al_right),true,mldg.find(1024));
  tab.add_cell(html_align(html_align::al_right),true,mldg.find(1025));
  tab.add_cell(html_align(html_align::al_right),true,mldg.find(1026));
  tab.add_to_head();

  system_info sysinf;
  struct thread_info ln;
  bool first_flag = true;
  while (sysinf.get_tlist(first_flag,ln))
    {
      first_flag = false;
      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,itoS(ln.pid));
      tab.add_cell(html_align(html_align::al_center),false,ln.direction);
      tab.add_cell(html_align(html_align::al_center),false,ln.state);
      tab.add_cell(html_align(html_align::al_left),false,ln.typ);
      tab.add_cell(html_align(html_align::al_left),false,ln.call.call());
      tab.add_cell(html_align(html_align::al_right),false,ln.login_zeit.get_string());
      tab.add_cell(html_align(html_align::al_right),false,ln.user_zeit.get_string());
      tab.add_cell(html_align(html_align::al_right),false,ln.sys_zeit.get_string());
      tab.add_cell(html_align(html_align::al_right),false,MemtoS(ln.in_bytes));
      tab.add_cell(html_align(html_align::al_right),false,MemtoS(ln.out_bytes));
      tab.add_to_body();
    }

  html_object frame = standard_frame(mldg.find(1183),tab);
  out = frame.write(); 

  return out;
};


const String &html_generator::groups( void )
{
  static String out;

  html_tabular main;
  main.set_border(2);

  main.new_row();
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1046));
  main.add_cell(html_align(html_align::al_right),true,mldg.find(1047));
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1048));
  main.add_to_body();

  vector<struct gruppe_info> infos = grps.get_infos();
  for (vector<struct gruppe_info>::iterator it = infos.begin(); it != infos.end(); ++it )
    {
      main.new_row();
      main.add_cell(html_align(html_align::al_left),false,html_link(String("/grp/")+it->name,"txt",it->name));
      main.add_cell(html_align(html_align::al_right),false,itoS(it->anz));
      main.add_cell(html_align(html_align::al_left),false,it->info);
      main.add_to_body();
    }

  html_object frame = standard_frame(mldg.find(1049),main);
  out = frame.write(); 

  return out;
}


const String &html_generator::groups( gruppe grp )
{
  static String out;

  html_tabular main;
  main.set_border(2);

  main.new_row();
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1051));
  main.add_to_body();
  main.new_row();
  main.add_cell(html_align(html_align::al_left),false,grp.get_info());
  main.add_to_body();
  main.new_row();
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1052));
  main.add_to_body();
  main.new_row();


  html_tabular member_spalten;
  member_spalten.set_border(1);
  member_spalten.new_row();

  for (int col = 0 ; col < 2; col++ )
    {
      html_tabular spalte;
      spalte.set_border(0);
      spalte.new_row();
      spalte.add_cell(html_align(html_align::al_right),true,mldg.find(1053));
      spalte.add_cell(html_align(html_align::al_left),true,mldg.find(1054));
      spalte.add_to_body();

      callsign call;
      String name;
      
      int pos = 0;
      try
	{
	  call = grp.get_first();
	  
	  do
	    {
	      if (pos++ == 2)
		pos = 1;
	      if (pos -1 == col)
		{
		  spalte.new_row();
		  call.set_format(true);
		  spalte.add_cell(html_align(html_align::al_right),false,html_link(String("/user?all=1&part=")+call.call(),String("txt"),call.call()));
		  try
		    {
		      spalte.add_cell(html_align(html_align::al_left),false,calls.find(call).get_name());
		    }
		  catch( Error_callsign_does_not_exist )
		    {
		      spalte.add_cell(html_align(html_align::al_left),false,"Call not in database");
		    }
		  spalte.add_to_body();
		}
	      call = grp.get_next();
	    }
	  while( true );
	}
      catch( Error_no_more_call_in_group )
	{
	}
      member_spalten.add_cell(html_align(html_align::al_left),false,spalte);
    }
  member_spalten.add_to_body();
  main.add_cell(html_align(html_align::al_left),false,member_spalten);
  main.add_to_body();

  html_object frame = standard_frame(mldg.find(1050)+String(" ")+grp.get_name(),main);
  out = frame.write(); 

  return out;
}

const String & html_generator::do_groups( const String url )
{
  try
    {
      gruppen grps(configuration);
      
      if (url.in("/grp/"))
	{
	  unsigned int p = url.pos("/grp/");
	  if (url.slen() > p + 5)
	    {
	      String bname = url.copy(p+5,url.slen()-p-5);
	      try
		{
		  gruppe grp = grps.find(bname);
		  return groups(grp);
		}
	      catch( Error_no_group_name )
		{
		  throw Error_html_generator_error("Error : No Groupname");
		}
	      catch( Error_group_does_not_exist )
		{
		  throw Error_html_generator_error("Error : Group does not exist");
		}
	    }
	  else
	    return groups();
	}
      else
	{
	  return groups();
	}
    }
  catch( Error_could_not_open_groups )
    {
      throw Error_html_generator_error("Error: Could not open Groups");
    }
}

const String &html_generator::destinations( void )
{
  static String out;

  html_tabular main;
  main.set_border(2);

  main.new_row();
  main.add_cell(html_align(html_align::al_right),true,mldg.find(1056));
  main.add_cell(html_align(html_align::al_right),true,mldg.find(1057));
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1058));

  vector<callsign> nbs = a_router.get_nachbarcalls();
  for (vector<callsign>::iterator it = nbs.begin(); it != nbs.end(); ++it )
    main.add_cell(html_align(html_align::al_left),true,it->call());
  
  main.add_to_body();

  vector<struct destin_info> infos = a_router.get_infos();
  for (vector<struct destin_info>::iterator it2 = infos.begin(); it2 != infos.end(); ++it2 )
    {
      main.new_row();
      main.add_cell(html_align(html_align::al_right),false,it2->zielgebiet.get_string());
      delta_t dt;
      dt = delta_t(it2->min_delay);
      main.add_cell(html_align(html_align::al_right),false,dt.get_string());
      main.add_cell(html_align(html_align::al_right),false,it2->min_delay_call.call());
      for (vector<double>::iterator it3 = it2->delay.begin(); it3 != it2->delay.end(); ++it3 )
	{
	  if (*it3 == -1)
	    main.add_cell(html_align(html_align::al_right),false,"-----");
	  else
	    {
	      dt = delta_t(*it3);
	      main.add_cell(html_align(html_align::al_right),false,dt.get_string());
	    }
	}
      main.add_to_body();
    }

  html_object frame = standard_frame(mldg.find(1055),main);
  out = frame.write(); 
  return out;
}

const String &html_generator::destinations( const destin &ds )
{
  static String out;

  html_tabular main;
  main.set_border(2);

  main.new_row();
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1056));
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1057));
  main.add_cell(html_align(html_align::al_left),true,mldg.find(1058));

  vector<callsign> nbs = a_router.get_nachbarcalls();
  for (vector<callsign>::iterator it = nbs.begin(); it != nbs.end(); ++it )
    main.add_cell(html_align(html_align::al_left),true,it->call());
  
  main.add_to_body();

  vector<struct destin_info> infos = a_router.get_infos(ds);
  for (vector<struct destin_info>::iterator it2 = infos.begin(); it2 != infos.end(); ++it2 )
    {
      main.new_row();
      main.add_cell(html_align(html_align::al_right),false,it2->zielgebiet.get_string());
      delta_t dt;
      dt = delta_t(it2->min_delay);
      main.add_cell(html_align(html_align::al_right),false,dt.get_string());
      main.add_cell(html_align(html_align::al_right),false,it2->min_delay_call.call());
      for (vector<double>::iterator it3 = it2->delay.begin(); it3 != it2->delay.end(); ++it3 )
	{
	  dt = delta_t(*it3);
	  main.add_cell(html_align(html_align::al_right),false,dt.get_string());
	}
      main.add_to_body();
    }

  html_object frame = standard_frame(mldg.find(1055),main);
  out = frame.write(); 

  return out;
}


const String &html_generator::forward( void )
{
  static String out;
  bool akt;

  vector<struct neighbor_info> infos = router.get_infos(akt);

  String ueberschr;
  html_tabular main;

  if (akt)
    {
      ueberschr = mldg.find(1059);
      main.set_border(2);
      main.new_row();
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1060));
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1061));
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1062));
      main.add_cell(html_align(html_align::al_center),true,mldg.find(1063));
      main.add_cell(html_align(html_align::al_center),true,mldg.find(1064));
      main.add_cell(html_align(html_align::al_center),true,mldg.find(1065));
      main.add_cell(html_align(html_align::al_center),true,mldg.find(1200));
      main.add_cell(html_align(html_align::al_center),true,mldg.find(1201));
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1066));
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1067));
      main.add_cell(html_align(html_align::al_left),true,mldg.find(1068));
      main.add_to_body();
      for ( vector<struct neighbor_info>::iterator it = infos.begin() ; it != infos.end(); ++it )
	{
	  main.new_row();
	  it->call.set_format(false);
	  it->call.set_nossid(true);
	  main.add_cell(html_align(html_align::al_left),false,html_link(String("fwd/")+it->call.call(),"txt",it->call.call()));

	  String trm;
	  switch(it->stat)
	    {
	      case st_getrennt    : trm = mldg.find(1069);
	                            break;
	      case st_aufbau      : trm = mldg.find(1070);
    	                            break;
              case st_aktiv       : trm = mldg.find(1071);
	                            break;
              case st_gescheitert : trm = " --- ";
	                            trm.append(String('(')+itoS(it->sch_cnt)+String(')'));
				    break;
	    }
	  main.add_cell(html_align(html_align::al_left),false,trm);
	  delta_t dt(zeit() - it->last_change);
	  main.add_cell(html_align(html_align::al_right),false,dt.get_string());
	  main.add_cell(html_align(html_align::al_right),false,itoS(it->n_pers));
	  main.add_cell(html_align(html_align::al_right),false,itoS(it->n_bul));
	  main.add_cell(html_align(html_align::al_right),false,itoS(it->n_sonst));
          main.add_cell(html_align(html_align::al_right),false,itoS(it->unack));
	  dt = delta_t(it->t_w);
	  main.add_cell(html_align(html_align::al_right),false,dt.get_string());
	  
	  if (it->mean_rtt != -1)
	    {
	      dt = delta_t(it->mean_rtt);
	      trm = dt.get_string();
	    }
	  else
	    trm = "-----";
	  main.add_cell(html_align(html_align::al_right),false,trm);

	  if (it->mean_offset != -1)
	    {
	      dt = delta_t(it->mean_offset);
	      trm = dt.get_string();
	    }
	  else
	    trm = "-----";
	  main.add_cell(html_align(html_align::al_right),false,trm);
	  main.add_cell(html_align(html_align::al_left),false,it->options.get_string());
	  main.add_to_body();
	}
    }
  else
    ueberschr = mldg.find(1072);
  
  html_object frame = standard_frame(ueberschr,main);
  out = frame.write(); 

  return out;
}
      
const String &html_generator::forward( const callsign &call )
{
  static String out;
  bool akt;
  vector<struct neighbor_info> infos = router.get_infos(akt,call);

  String ueberschr;
  html_tabular main;

  if (akt)
    {
      ueberschr = mldg.find(1073) + String(' ')+ call.call();
      vector<struct neighbor_info>::iterator it = infos.begin();
      if (it != infos.end())
	{
	  main.set_border(2);
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1074));
	  main.add_cell(html_align(html_align::al_left),false,it->typ);
	  main.add_to_body();
	  
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1075));
	  main.add_cell(html_align(html_align::al_left),false,it->address);
	  main.add_to_body();
	  
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1076));
	  
	  String trm;
	  switch(it->stat)
	    {
	      case st_getrennt    : trm = mldg.find(1077);
	                            break;
	      case st_aufbau      : trm = mldg.find(1078);
	                            break;
              case st_aktiv       : trm = mldg.find(1079);
	                            break;
              case st_gescheitert : trm = mldg.find(1080);
		                    trm.append(String(" (")+itoS(it->sch_cnt)+String(')'));
                                    break;
	    }
	  main.add_cell(html_align(html_align::al_left),false,trm);
	  main.add_to_body();
	  
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1081));
	  delta_t dt(zeit() - it->last_change);
	  main.add_cell(html_align(html_align::al_left),false,dt.get_string());
	  main.add_to_body();
	  
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1082));
	  
	  html_tabular thrd_tab;
	  thrd_tab.set_spacing(3);
	  thrd_tab.set_border(0);
	  thrd_tab.new_row();
	  
	  for (vector<int>::iterator it2 = it->akt_thrds.begin();
	       it2 != it->akt_thrds.end(); ++it2 )
	    thrd_tab.add_cell(html_align(html_align::al_center),false,itoS(*it2));
	  
	  thrd_tab.add_to_body();
	  main.add_cell(html_align(html_align::al_left),false,thrd_tab.write());
	  main.add_to_body();
	  
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1202));
	  {
	    html_tabular spool_tab;
	    spool_tab.set_border(1);
	    spool_tab.set_spacing(2);
	    spool_tab.new_row();
	    spool_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1063));
	    spool_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1064));
	    spool_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1065));
	    spool_tab.add_to_body();
	    spool_tab.new_row();
	    spool_tab.add_cell(html_align(html_align::al_right),false,itoS(it->n_pers));
	    spool_tab.add_cell(html_align(html_align::al_right),false,itoS(it->n_bul));
	    spool_tab.add_cell(html_align(html_align::al_right),false,itoS(it->n_sonst));
	    spool_tab.add_to_body();
	    main.add_cell(html_align(html_align::al_left),false,spool_tab);
	  }
	  main.add_to_body();
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1203));
	  {
	    html_tabular para_tab;
	    para_tab.set_border(1);
	    para_tab.set_spacing(2);
	    para_tab.new_row();
	    para_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1200));
	    para_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1201));
	    para_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1204));
	    para_tab.add_cell(html_align(html_align::al_center),true,mldg.find(1205));
	    para_tab.add_to_body();
	    para_tab.new_row();
	    para_tab.add_cell(html_align(html_align::al_right),false,itoS(it->unack));
	    dt = delta_t(it->t_w);
	    para_tab.add_cell(html_align(html_align::al_right),false,dt.get_string());
	    para_tab.add_cell(html_align(html_align::al_right),false,itoS(it->n_max));
	    para_tab.add_cell(html_align(html_align::al_right),false,itoS(it->fehler_zaehler));
	    para_tab.add_to_body();
	    main.add_cell(html_align(html_align::al_left),false,para_tab);
	  }
	  main.add_to_body();
	  main.new_row();
	  main.add_cell(html_align(html_align::al_left),true,mldg.find(1083));
	  if (it->mean_rtt != -1)
	    {
	      dt = delta_t(it->mean_rtt);
	      trm = dt.get_string();
	    }
	  else
	    trm = "-----";
	  main.add_cell(html_align(html_align::al_left),false,trm);
	  main.add_to_body();
	  
	  main.new_row(),
	    main.add_cell(html_align(html_align::al_left),true,"");
	  
	  html_tabular rtt_tab;
	  rtt_tab.set_border(0);
	  rtt_tab.set_spacing(3);
	  rtt_tab.new_row();
	  vector<double>::iterator itd;
	  int i = 0;
	  for (itd = it->rtt.begin(); itd != it->rtt.end(); ++itd)
	    {
	      if (*itd != -1)
		{
		  dt = delta_t(*itd);
		  trm = dt.get_string();
		}
	      else
		trm = "-----";
	      rtt_tab.add_cell(html_align(html_align::al_right),false,trm);
	      
	      if (++i == 8)
		{
		  rtt_tab.add_to_body();
		  rtt_tab.new_row();
		  i = 0;
		}
	    }
	  main.add_cell(html_align(html_align::al_left),false,rtt_tab.write());
	  main.add_to_body();
	  
	  main.new_row(),
	    main.add_cell(html_align(html_align::al_left),true,mldg.find(1084));
	  if (it->mean_offset != -9999)
	    {
	      dt = delta_t(it->mean_offset);
	      trm = dt.get_string();
	    }
	  else
	    trm = "-----";
	  main.add_cell(html_align(html_align::al_left),false,trm);
	  main.add_to_body();
	  
	  main.new_row(),
	    main.add_cell(html_align(html_align::al_left),true,"");
	  
	  html_tabular off_tab;
	  off_tab.set_border(0);
	  off_tab.set_spacing(3);
	  off_tab.new_row();
	  i = 0;
	  for (itd = it->offset.begin(); itd != it->offset.end(); ++itd)
	    {
	      if (*itd != -9999)
		{
		  dt = delta_t(*itd);
		  trm = dt.get_string();
		}
	      else
		trm = "-----";
	      off_tab.add_cell(html_align(html_align::al_right),false,trm);
	      
	      if (++i == 8)
		{
		  off_tab.add_to_body();
		  off_tab.new_row();
		  i = 0;
		}
	    }
	  main.add_cell(html_align(html_align::al_left),false,off_tab.write());
	  main.add_to_body();
	}
      else
	ueberschr = mldg.find(1085);
    }
  else
    ueberschr = mldg.find(1072);
  
  html_object frame = standard_frame(ueberschr,main);
  out = frame.write(); 

  return out;
}
 


const String & html_generator::do_forward( const String &url )
{
  if (url.in("/fwd/"))
    {
      unsigned int p = url.pos("/fwd/");
      if (url.slen() > p + 5)
	{
	  String nachbar = url.copy(p+5,url.slen()-p-5);
	  try
	    {
	      callsign nb(nachbar);
	      return forward(nb);
	    }		
	  catch( Error_no_callsign )
	    {
	      throw Error_html_generator_error("Error : No Callsign ");
	    }
	}
      return forward();
    }
  else
    {
      return forward();
    }
}

const String & html_generator::logfiles( logfile &logf, int anz, const String &part,  char time, zeit bis )
{
  static String out;
  int cnt;
  vector<String> eintraege = logf.get_lines(cnt,anz,part,time,bis);
  html_tabular tab;
  tab.set_border(2);
  tab.set_spacing(1);
  for (vector<String>::iterator it = eintraege.begin(); it != eintraege.end(); ++it )
    {
      tab.new_row();
      vector<String> elemente = blank_separated(*it);
      for (vector<String>::iterator it2 = elemente.begin(); it2 != elemente.end(); ++it2 )
	tab.add_cell(html_align(html_align::al_left),false,*it2);
      tab.add_to_body();
    }

  html_object frame = standard_frame(mldg.find(1179),tab);
  out = frame.write(); 

  return out;
}  

const String & html_generator::do_log( void )
{
  String logtyp;
  String time;
  String tag;
  String monat;
  String jahr;
  String anz;
  String part;
  get_postarea("logtyp=",logtyp);
  get_postarea("time=",time);
  get_postarea("tag=",tag);
  get_postarea("monat=",monat);
  get_postarea("jahr=",jahr);
  get_postarea("anz=",anz);
  get_postarea("part=",part);

  logfile *logf;
  if (usr.is_sysop())
    {
      if (logtyp == "system")
	logf = new syslog(configuration);
      else if (logtyp == "ip")
	logf = new iplog(configuration);
      else if (logtyp == "http")
	logf = new httplog(configuration);
      else if (logtyp == "forward")
	logf = new fwdlog(configuration);
      else if (logtyp == "rpc")
	logf = new rpclog(configuration);
      else if (logtyp == "spool")
	logf = new spoollog(configuration);
      else
	logf = new userlog(configuration);
    }
  else
    logf = new userlog(configuration);

  int i_anz = anz.Stoi();
  char t;
  if (time.slen() > 0)
    t = time[0];
  else
    t = 'h';

  zeit bis = get_datum(tag.Stoi(),monat.Stoi(),jahr.Stoi());

  return logfiles(*logf,i_anz,part,t,bis);
  delete logf;
}

const String & html_generator::do_logform( void )
{
  static String out;

  html_form form(html_form::m_get,"/logout");
  form.set_target("txt");
  {
    html_tabular main_tab;
    main_tab.set_border(0);
    main_tab.set_spacing(4);
    {
      html_tabular logtyp;
      logtyp.set_border(0);
      logtyp.set_spacing(4);
      logtyp.new_row();
      logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","user","Userlog"));
      if (usr.is_sysop())
	{
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","system","Systemlog"));
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","forward","Forwardlog"));
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","ip","IP-log"));
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","http","HTTP-log"));
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","rpc","RPC-log"));
	  logtyp.add_cell(html_align(html_align::al_left),false,html_form_radio("logtyp","spool","Spoollog"));
	}
      logtyp.add_to_body();
      
      main_tab.new_row();
      main_tab.add_cell(html_align(html_align::al_left),false,logtyp);
      main_tab.add_to_body();
    }

    {
      html_tabular zeitraum;
      zeitraum.set_border(0);
      zeitraum.set_spacing(4);
      zeitraum.new_row();
      zeitraum.add_cell(html_align(html_align::al_left),false,html_form_radio("time","h",mldg.find(1168)));
      zeitraum.add_cell(html_align(html_align::al_left),false,html_form_radio("time","w",mldg.find(1169)));
      zeitraum.add_cell(html_align(html_align::al_left),false,html_form_radio("time","m",mldg.find(1170)));
      zeitraum.add_cell(html_align(html_align::al_left),false,html_form_radio("time","q",mldg.find(1171)));
      zeitraum.add_cell(html_align(html_align::al_left),false,html_form_radio("time","y",mldg.find(1172)));
      zeitraum.add_cell(html_align(html_align::al_left),false,"vor dem");
      
      zeit jetzt;
      int tag = jetzt.get_tag();
      int monat = jetzt.get_monat();
      int jahr = jetzt.get_jahr();

      html_form_select day("tag");
      for (int i = 1; i < 32; i++)
	day.add_option(itoS(i),itoS(i));
      day.select_option(itoS(tag));
      
      html_form_select month("monat");
      for (int i = 1; i < 13; i++)
	month.add_option(itoS(i),itoS(i));
      month.select_option(itoS(monat));

      html_form_select year("jahr");
      for (int i = 2000; i < 2009; i++)
	year.add_option(itoS(i),itoS(i));
      year.select_option(itoS(jahr));
      
      zeitraum.add_cell(html_align(html_align::al_left),false,day);
      zeitraum.add_cell(html_align(html_align::al_left),false,month);
      zeitraum.add_cell(html_align(html_align::al_left),false,year);
      zeitraum.add_to_body();
      
      
      main_tab.new_row();
      main_tab.add_cell(html_align(html_align::al_left),false,zeitraum);
      main_tab.add_to_body();
    }

    {
      html_tabular suche;
      suche.set_border(0);
      suche.set_spacing(4);
      
      suche.new_row();
      suche.add_cell(html_align(html_align::al_left),false,mldg.find(1174));
      suche.add_cell(html_align(html_align::al_left),false,html_form_text_input("anz","15",3));
      suche.add_cell(html_align(html_align::al_left),false,mldg.find(1175));
      suche.add_cell(html_align(html_align::al_left),false,html_form_text_input("part","",40));
      suche.add_to_body();
      
      main_tab.new_row();
      main_tab.add_cell(html_align(html_align::al_left),false,suche);
      main_tab.add_to_body();
    }

    {
      html_tabular sub;
      sub.set_border(0);
      sub.set_spacing(4);

      sub.new_row();
      sub.add_cell(html_align(html_align::al_left),false,html_form_submit(mldg.find(1176)));
      sub.add_cell(html_align(html_align::al_right),false,html_form_reset(mldg.find(1177)));
      sub.add_to_body();

      main_tab.new_row();
      main_tab.add_cell(html_align(html_align::al_left),false,sub);
      main_tab.add_to_body();
    }

    form.add(main_tab);

  }

  html_object frame = standard_frame(mldg.find(1178),form);
  out = frame.write(); 

  return out;
}  

void html_generator::slave_simple( const slave & sl, html_tabular &tab )
{
  tab.new_row();
  callsign call = sl.get_call();
  call.set_format(false);
  call.set_nossid(true);
  tab.add_cell(html_align(html_align::al_left),false,html_link(String("/slave/")+call.call(),"txt",call.call()));

  String stat_string;
  switch (sl.status())
    {
      case slave::sts_disabled            : stat_string = mldg.find(1145);
                                            break;
      case slave::sts_spool               : stat_string = mldg.find(1146);
                                            break;
      case slave::sts_getrennt            : stat_string = mldg.find(1147);
                                            break;
      case slave::sts_aufbau              : stat_string = mldg.find(1148);
                                            break;
      case slave::sts_aktiv               : stat_string = mldg.find(1149);
	                                    break;
      case slave::sts_trennen_uz_disablen :
      case slave::sts_trennen_uz_spoolen  : stat_string = mldg.find(1150);
                                            break;
      case slave::sts_disablen            :
      case slave::sts_spoolen             : stat_string = mldg.find(1151);
                                            break;
      case slave::sts_gescheitert         : stat_string = mldg.find(1152);
    }
  tab.add_cell(html_align(html_align::al_left),false,stat_string);
  delta_t dt(zeit() - sl.last_change());
  tab.add_cell(html_align(html_align::al_right),false,dt.get_string());
  String mode_string;
  switch( sl.get_mode() )
    {
      case slave::m_passiv : mode_string = mldg.find(781);
                             break;
      case slave::m_activ  : mode_string = mldg.find(782);
                             break;
    }
  tab.add_cell(html_align(html_align::al_center),false,stat_string);
  tab.add_cell(html_align(html_align::al_center),false,sl.get_slots());
  tab.add_cell(html_align(html_align::al_left),false,sl.get_version());
  tab.add_to_body();
}

html_tabular html_generator::slave_extended( const slave &sl )
{
  html_tabular tab;

  tab.set_border(2);
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1155));
  callsign call = sl.get_call();
  call.set_format(false);
  call.set_nossid(false);
  tab.add_cell(html_align(html_align::al_left),false,call.call());
  tab.add_to_body();

  String stat_string;
  switch (sl.status())
    {
      case slave::sts_disabled            : stat_string = mldg.find(1145);
                                            break;
      case slave::sts_spool               : stat_string = mldg.find(1146);
                                            break;
      case slave::sts_getrennt            : stat_string = mldg.find(1147);
                                            break;
      case slave::sts_aufbau              : stat_string = mldg.find(1148);
                                            break;
      case slave::sts_aktiv               : stat_string = mldg.find(1149);
	                                    break;
      case slave::sts_trennen_uz_disablen :
      case slave::sts_trennen_uz_spoolen  : stat_string = mldg.find(1150);
                                            break;
      case slave::sts_disablen            :
      case slave::sts_spoolen             : stat_string = mldg.find(1151);
                                            break;
      case slave::sts_gescheitert         : stat_string = mldg.find(1152);
    }
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1156));
  tab.add_cell(html_align(html_align::al_left),false,stat_string);
  tab.add_to_body();

  delta_t dt(zeit() - sl.last_change());
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1157));
  tab.add_cell(html_align(html_align::al_left),false,dt.get_string());
  tab.add_to_body();

  String mode_string;
  switch( sl.get_mode() )
    {
      case slave::m_passiv : mode_string = mldg.find(781);
                             break;
      case slave::m_activ  : mode_string = mldg.find(782);
                             break;
    }
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1158));
  tab.add_cell(html_align(html_align::al_left),false,mode_string);
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1159));
  tab.add_cell(html_align(html_align::al_left),false,sl.get_bake());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1160));
  tab.add_cell(html_align(html_align::al_left),false,sl.get_slots());
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1181));
  tab.add_cell(html_align(html_align::al_left),false,sl.get_version());
  tab.add_to_body();
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1182));
  dt = delta_t(sl.get_diff());
  tab.add_cell(html_align(html_align::al_left),false,dt.get_string());
  tab.add_to_body();
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1161));
  tab.add_cell(html_align(html_align::al_left),false,sl.get_connect_pfad().get_string());
  tab.add_to_body();

  String pw_string;
  if (sl.get_passwd().slen() > 0)
    {
      pw_string =  mldg.find(1163) + String(" (");
      pw_string.append(itoS(sl.get_passwd().slen())+String(")"));
    }
  else
    pw_string = mldg.find(1164);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1162));
  tab.add_cell(html_align(html_align::al_left),false,pw_string);
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1165));
  tab.add_cell(html_align(html_align::al_left),false,sl.get_destinations().get_string());
  tab.add_to_body();

  return tab;
}

html_tabular html_generator::slavetab( vector<slave> sl )
{
  html_tabular tab;

  tab.set_border(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1155));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1156));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1157));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1158));
  tab.add_cell(html_align(html_align::al_left),true,mldg.find(1160));
  tab.add_to_body();

  for (vector<slave>::iterator it = sl.begin(); it != sl.end(); ++it )
    slave_simple(*it,tab);

  return tab;
}

const String & html_generator::do_slaves( const String & url )
{
  static String out;
  String ueberschr;

  html_object body;

  if (url.in("/slave/"))
    {
      unsigned int p = url.pos("/slave/");
      if (url.slen() > p + 7)
	{
	  String slavecall = url.copy(p+7,url.slen()-p-7);
	  try
	    {
	      callsign sc(slavecall);
	      ueberschr = mldg.find(1166)+String(" ")+slavecall;

	      vector<slave> sl = slaves.get_slave(sc);
	      vector<slave>::iterator it = sl.begin();
	      if (it != sl.end())
		body.add(slave_extended(*it));
	      else
		{
		  html_italic it2;
		  it2.add(mldg.find(1166)+String(" ")+slavecall);
		  it2.add(String(" ")+mldg.find(1167));
		  html_ueberschrift u2(2);
		  u2.add(it2);
		  body.add(u2);

		}
	    }
	  catch( Error_no_callsign )
	    {
	      throw Error_html_generator_error("Error : No Callsign ");
	    }
	}
    }
  else
    {
      ueberschr = mldg.find(1166);
      vector<slave> sl = slaves.get_slave();
      body.add(slavetab(sl));
    }
  html_object frame = standard_frame(ueberschr,body);
  out = frame.write(); 

  return out;
}




const String & html_generator::do_sendok( t_baken & baken )
{
  String to;
  String typ;
  String msg;
  String destin;
  
  get_postarea("an=",to);
  get_postarea("typ=",typ);
  get_postarea("destin=",destin);
  get_postarea("msg=",msg);

  String cmd;
  if (typ == "1")
    {
      cmd = "page "+to + destin+" "+msg;
      return exec(cmd,baken);
    }
  else if (typ == "2")
    {
      //      cmd = "gp "+to + destin+" "+msg;
      // DH6BB 10.07.04: Solange GP noch keine Destination versteht machen wirs ohne.
      cmd = "gp "+to + " "+msg;
      return exec(cmd,baken);
    }
  else if (typ == "3")
    {
      cmd = "ru "+to + destin+" "+msg;
      return exec(cmd,baken);
    }
  else
    {
      throw Error_html_generator_error("Bitte w&auml;hlen Sie einen Funkruf-Typ aus ");
  }
}

html_form_select html_generator::destination_form( void )
{
  html_form_select fdest("destin");
  fdest.add_option("@dl.eu","@DL.EU");
  fdest.add_option("@a.dl.eu","@A.DL.EU");
  fdest.add_option("@b.dl.eu","@B.DL.EU");
  fdest.add_option("@c.dl.eu","@C.DL.EU");
  fdest.add_option("@d.dl.eu","@D.DL.EU");
  fdest.add_option("@e.dl.eu","@E.DL.EU");
  fdest.add_option("@f.dl.eu","@F.DL.EU");
  fdest.add_option("@g.dl.eu","@G.DL.EU");
  fdest.add_option("@h.dl.eu","@H.DL.EU");
  fdest.add_option("@i.dl.eu","@I.DL.EU");
  fdest.add_option("@k.dl.eu","@K.DL.EU");
  fdest.add_option("@l.dl.eu","@L.DL.EU");
  fdest.add_option("@m.dl.eu","@M.DL.EU");
  fdest.add_option("@n.dl.eu","@N.DL.EU");
  fdest.add_option("@o.dl.eu","@O.DL.EU");
  fdest.add_option("@p.dl.eu","@P.DL.EU");
  fdest.add_option("@q.dl.eu","@Q.DL.EU");
  fdest.add_option("@r.dl.eu","@R.DL.EU");
  fdest.add_option("@s.dl.eu","@S.DL.EU");
  fdest.add_option("@t.dl.eu","@T.DL.EU");
  fdest.add_option("@u.dl.eu","@U.DL.EU");
  fdest.add_option("@v.dl.eu","@V.DL.EU");
  fdest.add_option("@w.dl.eu","@W.DL.EU");
  fdest.add_option("@x.dl.eu","@X.DL.EU");
  fdest.add_option("@y.dl.eu","@Y.DL.EU");

  return fdest;
}

const String & html_generator::sendform( void )
{
  static String out;

  html_form form(html_form::m_get,"/sendok");

  html_tabular tab;
  tab.set_border(0);

  tab.new_row();

  html_form_select ftyp("typ");
  ftyp.add_option("0",mldg.find(1087));
  ftyp.add_option("1",mldg.find(1088));
  ftyp.add_option("2",mldg.find(1089));
  ftyp.add_option("3",mldg.find(1090));

  tab.add_cell(html_align(html_align::al_left),false,ftyp);
  tab.add_cell(html_align(html_align::al_left),false,"");
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1091));

  html_form_text_input fadr("an","",10);

  html_object tabcell;
  tabcell.add(fadr);
  tabcell.add(destination_form());
  tab.add_cell(html_align(html_align::al_left),false,tabcell);
  tab.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1092));
  html_form_text_input fmsg("msg","",80);
  tab.add_cell(html_align(html_align::al_left),false,fmsg);
  tab.add_to_body();

  tab.new_row();
  html_form_submit sub(mldg.find(1093));
  html_form_reset res(mldg.find(1094));
  tab.add_cell(html_align(html_align::al_left),false,sub);
  tab.add_cell(html_align(html_align::al_right),false,res);
  tab.add_to_body();

  form.add(tab);
  html_object frame = standard_frame(mldg.find(1184),form);
  out = frame.write(); 

  return out;
}


const String & html_generator::do_passwd( void )
{

  static String out;
  String ueberschr;
  html_object body;

  String old;
  String new1;
  String new2;
  
  get_postarea("old=",old);
  get_postarea("pw1=",new1);
  get_postarea("pw2=",new2);


  if (new1 == new2)
    {
      if (cpw.change_password(usr.user_call(),old,new1))
	{
	  ueberschr = mldg.find(1095);
	}
      else
	{
	  ueberschr = mldg.find(1096);
	  html_absatz p2;
	  p2.add(mldg.find(1097));

	  body.add(p2);
	}
    }
  else
    {
      ueberschr = mldg.find(1096);

      html_absatz p2;
      p2.add(mldg.find(1098));

      body.add(p2);
    }
  html_object frame = standard_frame(ueberschr,body);
  out = frame.write();
  return out; 

}

const String & html_generator::do_setpw( void )
{
  static String out;

  String callst;
  String new1;
  String new2;
  get_postarea("call=",callst);
  get_postarea("pw1=",new1);
  get_postarea("pw2=",new2);

  html_object body;

  if (usr.is_sysop())
    {
      try
	{
	  callsign call(callst);
	  if (new1 == new2)
	    {
	      if (cpw.set_password(call,new1))
		{
		  html_italic it;
		  it.add(mldg.find(1095));
		  body.add(it);
		}
	      else
		{
		  html_absatz p1;
		  html_italic it;
		  it.add(mldg.find(1096));
		  p1.add(it);
		  
		  body.add(p1);
		}
	    }
	  else
	    {
	      html_absatz p1;
	      html_italic it;
	      it.add(mldg.find(1096));
	      p1.add(it);
	      
	      html_absatz p2;
	      p2.add(mldg.find(1098));
	      
	      body.add(p1);
	      body.add(p2);
	    }
	}
      catch( Error_no_callsign )
	{
	  html_absatz p1;
	  html_italic it;
	  it.add(mldg.find(1096));
	  p1.add(it);
	  
	  html_absatz p2;
	  p2.add(mldg.find(1099));
	      
	  body.add(p1);
	  body.add(p2);
	}
    }
  else
    {
      html_absatz p1;
      html_italic it;
      it.add(mldg.find(1096));
      p1.add(it);
      
      html_absatz p2;
      p2.add(mldg.find(1100));
      
      body.add(p1);
      body.add(p2);
    }
  html_object frame = standard_frame("",body);
  out = frame.write();
  return out; 

}

const String &html_generator::do_passwdform( void )
{
  static String out;

  html_object body;
  String ueberschr;

  if (cpw.ax25_connection())
    {
      ueberschr = mldg.find(1101);
      body.add(mldg.find(1102));
    }
  else
    {
      if (usr.is_sysop())
	{
	  ueberschr = mldg.find(1103);
	  
	  html_form form(html_form::m_get,"/setpw");

	  html_tabular tab;
	  tab.set_border(0);

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1104));
	  html_form_text_input c("call","",12);
	  tab.add_cell(html_align(html_align::al_left),false,c);
	  tab.add_to_body();

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1105));
	  html_form_password_input pw1("pw1","",12);
	  tab.add_cell(html_align(html_align::al_left),false,pw1);
	  tab.add_to_body();

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1106));
	  html_form_password_input pw2("pw2","",12);
	  tab.add_cell(html_align(html_align::al_left),false,pw2);
	  tab.add_to_body();

	  tab.new_row();
	  html_form_submit sub(mldg.find(1107));
	  html_form_reset res(mldg.find(1094));
	  tab.add_cell(html_align(html_align::al_left),false,sub);
	  tab.add_cell(html_align(html_align::al_right),false,res);
	  tab.add_to_body();
	  form.add(tab);
	  body.add(form);
	}
      else
	{
	  ueberschr = mldg.find(1108);
	  
	  html_form form(html_form::m_get,"/changepw");

	  html_tabular tab;
	  tab.set_border(0);

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1109));
	  html_form_password_input c("old","",6);
	  tab.add_cell(html_align(html_align::al_left),false,c);
	  tab.add_to_body();

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1105));
	  html_form_password_input pw1("pw1","",12);
	  tab.add_cell(html_align(html_align::al_left),false,pw1);
	  tab.add_to_body();

	  tab.new_row();
	  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1106));
	  html_form_password_input pw2("pw2","",12);
	  tab.add_cell(html_align(html_align::al_left),false,pw2);
	  tab.add_to_body();

	  tab.new_row();
	  html_form_submit sub(mldg.find(1107));
	  html_form_reset res(mldg.find(1094));
	  tab.add_cell(html_align(html_align::al_left),false,sub);
	  tab.add_cell(html_align(html_align::al_right),false,res);
	  tab.add_to_body();
	  form.add(tab);
	  body.add(form);
	}
    }
  html_object frame = standard_frame(ueberschr,body);
  out = frame.write();
  return out; 
  
  
}


const String &html_generator::do_sysopform( void )
{
  static String out;
  String zahlen;

  html_object body;

  if (!usr.auth1(configuration,zahlen))
    {
      body.add(mldg.find(1111));
    }
  else
    {

      html_absatz p1;
      p1.add(mldg.find(1112));

      html_absatz p2;
      p2.add(mldg.find(1113));

      body.add(p1);
      body.add(p2);
      body.add("<hr>");

      html_absatz p3;
      p3.add(mldg.find(1114));

      p3.add(zahlen);
      body.add(p3);


      html_form form(html_form::m_get,"/syspw");
      html_form_text_input pw("pw","",80);
      form.add(pw);

      html_form_submit sub(mldg.find(1115));
      form.add(sub);

      body.add(form);
    }
  html_object frame = standard_frame(mldg.find(1110),body);
  out = frame.write();  
  return out; 
  
  
}

const String & html_generator::do_syspw( void )
{

  static String out;
  String antw;
  get_postarea("pw=",antw);
  html_object body;
  String ueberschr = mldg.find(1185);

  if ( usr.auth2(configuration,antw) )
    {
      if (usr.is_sysop() )
	{
	  ueberschr = mldg.find(1116);
	  html_absatz p1;
	  p1.add(mldg.find(1117));
	  
	  body.add(p1);
	  html_absatz p2;
	  p2.add(html_link("/","_parent","ok"));
	  body.add(p2);
	}
      else
	{
	  ueberschr = mldg.find(1118);
	}
    }
  else
	  ueberschr = mldg.find(1118);

  html_object frame = standard_frame(ueberschr,body);
  out = frame.write(); 
  
  return out;
}
     


const String & html_generator::do_logoff( void )
{
  static String out;
  
  html_object body;

  if (usr.is_sysop())
    {
      usr.logoff();
      body.add(mldg.find(1186)+String(" "));
      body.add(html_link("/","_parent","ok"));
    }
  html_object frame = standard_frame("",body);
  out = frame.write(); 
  
  return out;
}



int html_generator::do_file( const String &url, String &out, mime_type &m_typ, zeit &date )
{
  if (url.in(".."))
    {
      return 403;
    }
  try
    {
      String htmldir = configuration.find("HTMLDIR");
      String fname;
      if (url.slen() > 0 && url[0] == '/')
	fname = htmldir + url.copy(1,url.slen()-1);
      else
	fname = htmldir + url;

      int l = fname.slen();

      if (l - 4 >= 0 && fname.copy(l-4,4) == ".jpg")
	m_typ = mime_jpg;
      else if (l - 5 >= 0 && fname.copy(l-5,5) == ".jpeg")
	m_typ = mime_jpg;
      else if (l - 4 >= 0 && fname.copy(l-4,4) == ".gif")
	m_typ = mime_gif;
      else if (l - 4 >= 0 && fname.copy(l-4,4) == ".wav")
	m_typ = mime_wav;
      else if (l - 4 >= 0 && fname.copy(l-4,4) == ".txt")
	m_typ = mime_txt;
      else if (l - 4 >= 0 && fname.copy(l-4,4) == ".htm")
	m_typ = mime_html;
      else if (l - 5 >= 0 && fname.copy(l-5,5) == ".html")
	m_typ = mime_html;

      //cerr << "Filename : " << fname << endl;

      ifstream ifile( strtochar(fname) );
      if (ifile)
	{
	  while (!ifile.eof())
	    {
	      int anz = 1024;
	      char buf[1025];
	      ifile.read(buf,anz);
	      anz = ifile.gcount();
	      out.append(String(buf,anz));
	    }
	  struct stat stat_struct;

	  stat(strtochar(fname), &stat_struct);
	  date = stat_struct.st_mtime;

	  return 200;
	}
      else
	return 404;
    }
  catch( Error_parameter_not_defined )
    {
      return 404;
    }

  return 404;
}

int html_generator::do_url( const String &url, String &out, mime_type &m_typ, 
			    int &clength  , zeit &date, t_baken &baken)
{
  String cmd;
  int status = 200;
  date = zeit();

  m_typ = mime_html;

  get_content(url);

  try
    {
      get_postarea("cmd=",cmd);
      if (cmd.slen() != 0)
	headline= cmd;
      else
	headline = G_mycall.str() + String(", FunkrufMaster V") + String(VERSION);
      
      if (url == "/")
	out = frame();
      else if (url.in("/command"))
	out = homepage(cmd);
      else if (url.in("/logins"))
	out = logins();
      else if (url.in("/login"))
	out = login();
      else if (url.in("/info"))
	out = info();
      else if (url.in("/aktuell"))
	out = aktuell();
      else if (url.in("/help"))
	out = do_help(url);
      else if (url.in("/cmd"))
	out = exec(cmd,baken);
      else if (url.in("/sendok"))
	out = do_sendok(baken);
      else if (url.in("/send"))
	out = sendform();
      else if (url.in("/passwd"))
	out = do_passwdform();
      else if (url.in("/changepw"))
	out = do_passwd();
      else if (url.in("/setpw"))
	out =do_setpw();
      else if (url.in("/sysop"))
	out = do_sysopform();
      else if (url.in("/syspw"))
	out = do_syspw();
      else if (url.in("/logoff"))
	out = do_logoff();
      else if (url.in("/dir"))
	out = do_directory(url);
      else if (url.in("/grp"))
	out = do_groups(url);
      else if (url.in("/dest"))
	out = destinations();
      else if (url.in("/fwd"))
	out = do_forward(url);
      else if (url.in("/slave"))
	out = do_slaves(url);
      else if (url.in("/userf"))
	out = do_userform();
      else if (url.in("/user"))
	out = do_user();
      else if (url.in("/profile"))
	out = do_profile(url);
      else if (url.in("/setprofile"))
	out = do_setprofile(url);
      else if (url.in("/logout"))
	out = do_log();
      else if (url.in("/log"))
	out = do_logform();
      else 
	{
	  out = "";
	  status = do_file(url,out,m_typ,date);
	}
    }
  catch( Error_html_generator_error gerror)
    {
      html_object eobj = gerror.get_object();
      html_body body;
      body.set_bgcolor("FFFFFF");

      html_italic it;
      it.add(eobj);
  
      html_ueberschrift u1(1);
      u1.add(it);
  
      body.add(u1);

      html_fett fett;
      fett.add(G_mycall.str()+", FunkrufMaster V"+ VERSION);

      body.add(fett);

      html_title title(String("FunkrufMaster V ")+VERSION+" HTML-Error");
      html_head hd;
  
      hd.add(title);
  
      html base;
      base.add(hd);
      base.add(body);
      
      out = base.write();
      m_typ = mime_html;
      status = 200;
    }
  catch( Error_html_generator_permission_violation )
    {
      status = 403;
    }
  catch( Error_html_generator_command_unknown )
    {
      status = 404;
    }
  clength = out.slen();
  return status;
}


const String & html_generator::status_phrase( int status )
{
  static String out;

  if (status == 200)
    out = "OK";
  else if (status == 201)
    out = "Created";
  else if (status == 202)
    out = "Accepted";
  else if (status == 204)
    out = "No Content";
  else if (status == 301)
    out = "Moved Permanently";
  else if (status == 302)
    out = "Moved Temporarily";
  else if (status == 304)
    out = "Not Modified";
  else if (status == 400)
    out = "Bad Request";
  else if (status == 401)
    out = "Unauthorized";
  else if (status == 403)
    out = "Forbidden";
  else if (status == 404)
    out = "Not Found";
  else if (status == 500)
    out = "internal Server Error";
  else if (status == 501)
    out = "Not Implemented";
  else if (status == 502)
    out = "Bad Gateway";
  else if (status == 503)
    out = "Service Unavailable";
  else if (status == 0)
    out = "Unknown";
  else
    out = "";

  return out;
}

void html_generator::do_error_msg(int status,String & tx_content,
				  mime_type &m_typ, int &clength, zeit &date)
{
  html_body body;
  body.set_bgcolor("FFFFFF");

  html_italic it;
  it.add(status_phrase(status)+String(" ")+itoS(status));
  
  html_ueberschrift u1(1);
  u1.add(it);
  
  body.add(u1);

  html_fett fett;
  fett.add(G_mycall.str()+", FunkrufMaster V"+ VERSION+String(" - "));

  html_italic it2;
  it2.add("httpd");
  fett.add(it2);

  body.add(fett);

  html_title title(String("FunkrufMaster V ")+VERSION+" HTTP-Error");
  html_head hd;
  
  hd.add(title);
  
  html base;
  base.add(hd);
  base.add(body);
  
  tx_content = base.write();
  m_typ = mime_html;
  clength = tx_content.slen();
  date = zeit();
}


