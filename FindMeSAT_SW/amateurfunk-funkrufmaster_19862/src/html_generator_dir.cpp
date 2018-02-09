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

#include "html_generator.h"
#include "html_align.h"
#include "html_frames.h"
#include "html_forms.h"
#include "html_references.h"
#include "html_tabular.h"
#include "html_paartags.h"

#include "fwd_frontend.h"

#include "String.h"
#include "board.h"

extern config_file configuration;
extern spoolfiles spool;
extern callsign G_mycall;

const String &html_generator::addboardop( board brd )
{
  static String out;
  html_object body;

  if (usr.is_sysop())
    {
      String callst;
      String perm;
      get_postarea("call=",callst);
      get_postarea("perm=",perm);

      try
	{
	  bool fwd_flag = perm == "f";
	  if (callst == "*")
	    brd.add_sysop(callsign(),!fwd_flag,true);
	  else
	    {
	      callsign call(callst);
	      brd.add_sysop(call,!fwd_flag,false);
	    }
	  body.add(mldg.find(1189));
	}
      catch( Error_no_callsign )
	{
	  body.add(mldg.find(1190));
	}
      html_link lnk(String("/dir/")+brd.get_name(),"txt",brd.get_name());
      body.add(mldg.find(1193)+String(" "));
      body.add(lnk);
      html_object frame = standard_frame(mldg.find(1187),body);
      out = frame.write();
      return out;
    }
  else
    throw Error_html_generator_permission_violation();
}

const String &html_generator::delboardop( board brd )
{
  static String out;
  html_object body;

  if (usr.is_sysop())
    {
      String callst;
      String perm;
      get_postarea("call=",callst);

      try
	{
	  if (callst == "*")
	    brd.del_sysop(callsign(),true);
	  else
	    {
	      callsign call(callst);
	      brd.del_sysop(call,false);
	    }
	  body.add(mldg.find(1191));
	}
      catch( Error_no_callsign )
	{
	  body.add(mldg.find(1192));
	}

      html_link lnk(String("/dir/")+brd.get_name(),"txt",brd.get_name());
      body.add(mldg.find(1193)+String(" "));
      body.add(lnk);
      html_object frame = standard_frame(mldg.find(1188),body);
      out = frame.write();
      return out;
    }
  else
    throw Error_html_generator_permission_violation();
}

const String &html_generator::addboardop_form( board brd )
{
  static String out;
  
  if (usr.is_sysop())
    {
      html_form form(html_form::m_get,String("/dir/")+brd.get_name()+String("/addok"));
      form.set_target("txt");

      html_tabular tab;
      tab.set_spacing(4);

      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,"Rufzeichen");
      tab.add_cell(html_align(html_align::al_left),false,html_form_text_input("call","",10));
      tab.add_to_body();

      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,"Berechtigung");
      html_form_select sel("perm");
      sel.add_option("l","Lokal");
      sel.add_option("f","Forward");
      tab.add_cell(html_align(html_align::al_left),false,sel);
      tab.add_to_body();

      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,html_form_submit("Eintragen"));
      tab.add_cell(html_align(html_align::al_right),false,html_form_reset("Formular L&ouml;schen"));
      tab.add_to_body();

      form.add(tab);

      html_object frame = standard_frame(mldg.find(1187),form);
      out = frame.write(); 
      
      
      return out;
    }
  else 
    throw  Error_html_generator_permission_violation();
}

const String &html_generator::delboardop_form( board brd )
{
  static String out;

  if (usr.is_sysop())
    {
      vector<struct board_sysop> sysops = brd.get_sysops();

      html_form form(html_form::m_get,String("/dir/")+brd.get_name()+String("/delok"));
      form.set_target("txt");

      html_tabular tab;
      tab.set_spacing(4);

      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,"Rufzeichen");
      html_form_select sel("call");
      for (vector<struct board_sysop>::iterator it = sysops.begin();
	   it != sysops.end(); ++it )
	{
	  if (it->wildcard)
	    sel.add_option("wc","*");
	  else
	    sel.add_option(it->call.call(),it->call.call());
	}
      tab.add_cell(html_align(html_align::al_left),false,sel);
      tab.add_to_body();

      tab.new_row();
      tab.add_cell(html_align(html_align::al_left),false,html_form_submit("L&ouml;schen"));
      tab.add_cell(html_align(html_align::al_right),false,html_form_reset("Formular L&ouml;schen"));
      tab.add_to_body();

      form.add(tab);

      html_object frame = standard_frame(mldg.find(1187),form);
      out = frame.write(); 
      
      
      return out;
    }
  else 
    throw  Error_html_generator_permission_violation();
}

const String &html_generator::directory( void )
{
  static String out;
  boards brds;

  vector<String> names = brds.get_board_names();
  html_tabular main;
  html_tabular sub1;
  html_tabular sub2;

  main.set_border(2);
  main.set_caption(mldg.find(1029));

  sub1.set_border(0);
  sub2.set_border(0);

  sub1.new_row();
  sub1.add_cell( html_align(html_align::al_right),true,mldg.find(1030));
  sub1.add_cell( html_align(html_align::al_right),true,mldg.find(1031));
  sub1.add_cell( html_align(html_align::al_center),true,mldg.find(1032));
  sub1.add_to_body();

  sub2.new_row();
  sub2.add_cell( html_align(html_align::al_right),true,mldg.find(1030));
  sub2.add_cell( html_align(html_align::al_right),true,mldg.find(1031));
  sub2.add_cell( html_align(html_align::al_center),true,mldg.find(1032));
  sub2.add_to_body();

  int cnt = 0;
  for (vector<String>::iterator it = names.begin(); it != names.end(); ++it )
    {
      board brd(*it,configuration);
      if (cnt == 0)
	{
	  sub1.new_row();
	  sub1.add_cell( html_align(html_align::al_right),false,itoS(brd.get_brd_id()));
	  sub1.add_cell( html_align(html_align::al_right),false,html_link(String("/dir/")+brd.get_name(),String("txt"),brd.get_name()));
	  zeit newest=brd.get_newest_time();
	  if (newest>9)
	    {
	      delta_t last_msg(zeit() - newest);
	      sub1.add_cell( html_align(html_align::al_center),false,String("(")+last_msg.get_string()+String(")"));
	    }
	  else
	    sub1.add_cell(html_align(html_align::al_center),false,String("( --:--  )"));
	  sub1.add_to_body();
	  cnt++;
	}
      else
	{
	  sub2.new_row();
	  sub2.add_cell( html_align(html_align::al_right),false,itoS(brd.get_brd_id()));
	  sub2.add_cell( html_align(html_align::al_right),false,html_link(String("/dir/")+brd.get_name(),"txt",brd.get_name()));
	  zeit newest=brd.get_newest_time();
	  if (newest>9)
	    {
	      delta_t last_msg(zeit() - newest);
	      sub2.add_cell( html_align(html_align::al_center),false,String("(")+last_msg.get_string()+String(")"));
	    }
	  else
	    sub2.add_cell(html_align(html_align::al_center),false,String("( --:--  )"));
	  sub2.add_to_body();
	  cnt = 0;
	}
    }
  main.new_row();
  main.set_rowalign(html_valign(html_valign::val_top));
  main.add_cell(html_align(html_align::al_center),false,sub1);
  main.add_cell(html_align(html_align::al_center),false,sub2);
  main.add_to_body();

  html_object frame = standard_frame(mldg.find(1028),main);
  out = frame.write(); 


  return out;	
}

html_object html_generator::directory_head( board brd )
{
  html_object obj;

  html_tabular head1;
  head1.set_border(1);
  head1.new_row();
  head1.add_cell(html_align(html_align::al_center),true,mldg.find(1035));
  head1.add_cell(html_align(html_align::al_center),true,mldg.find(1036));
  head1.add_cell(html_align(html_align::al_center),true,mldg.find(1037));
  head1.add_to_body();
  
  head1.new_row();
  head1.add_cell(html_align(html_align::al_center),false,itoS(brd.get_brd_id()));
  head1.add_cell(html_align(html_align::al_center),false,itoS(brd.get_deflt()));
  
  if (brd.get_tx_intv() == 0)
    {
      head1.add_cell(html_align(html_align::al_center),false,String("keine"));
    }	
  else
    {
      html_tabular intvtab;
      intvtab.set_border(0);
      intvtab.new_row();
      intvtab.add_cell(html_align(html_align::al_center),true,mldg.find(1038));
      intvtab.add_cell(html_align(html_align::al_center),true,mldg.find(1039));
      intvtab.add_to_body();
      
      intvtab.new_row();
      delta_t dt(brd.get_tx_intv());
      zeit last_tx = brd.get_last_tx();
      last_tx.set_darstellung(zeit::f_zeit_s);
      intvtab.add_cell(html_align(html_align::al_center),false,dt.get_string());
      intvtab.add_cell(html_align(html_align::al_center),false,last_tx.get_zeit_string());
      intvtab.add_to_body();
      head1.add_cell(html_align(html_align::al_center),false,intvtab.write());
    }
  head1.add_to_body();
  obj.add(head1);

  return obj;
}

html_object html_generator::directory_ops( board brd )
{
  html_object obj;

  html_tabular tab;
  tab.set_border(0);
  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,mldg.find(1040));
  tab.add_to_body();
  tab.new_row();
  String ln;
  bool flag = false;
  vector<struct board_sysop> sysops = brd.get_sysops();
  for (vector<struct board_sysop>::iterator it = sysops.begin() ; it != sysops.end() ; ++it)
    {
      if (flag)
	ln.append(", ");
      else
	flag = true;
      
      if (it->wildcard)
	ln.append("*");
      else 
	{
	  html_link l(String("/user?all=1&part=")+it->call.call(),String("txt"),it->call.call());
	  ln.append(l.write());
	}
      if (it->local)
	ln.append("(l)");
      else
	ln.append("(f)");
      
      flag = true;
    }
  tab.add_cell(html_align(html_align::al_left),false,ln);
  tab.add_to_body();

  obj.add(tab);
  return obj;
}

html_object html_generator::directory_entry_head( board brd, int index )
{
  html_object obj;

  html_tabular tab1;
  tab1.set_border(0);

  tab1.new_row();
  tab1.add_cell(html_align(html_align::al_left),true,mldg.find(1041));
  tab1.add_cell(html_align(html_align::al_left),false,itoS(index));
  tab1.add_to_body();

  tab1.new_row();
  tab1.add_cell(html_align(html_align::al_left),true,mldg.find(1042));
  zeit t = brd.get_time(index);
  t.set_darstellung(zeit::f_zeitdatum_s);
  tab1.add_cell(html_align(html_align::al_left),false,t.get_zeit_string());
  tab1.add_to_body();

  tab1.new_row();
  tab1.add_cell(html_align(html_align::al_left),true,mldg.find(1043));
  t = t + brd.get_lifetime(index) * 86400;
  t.set_darstellung(zeit::f_zeitdatum_s);
  tab1.add_cell(html_align(html_align::al_left),false,t.get_zeit_string());
  tab1.add_to_body();

  tab1.new_row();
  tab1.add_cell(html_align(html_align::al_left),true,mldg.find(1044));
  try
    {
      tab1.add_cell(html_align(html_align::al_left),false,brd.get_destin(index).get_string());
    }
  catch( Error_destin_checksum_error )
    {
      tab1.add_cell(html_align(html_align::al_left),true,mldg.find(1045));
    }
  tab1.add_to_body();
  obj.add(tab1);

  return obj;
}

void html_generator::split_entry_line( const String &line, String &l1, 
				       String &l2, String &l3, String &l4 )
{
  int l = line.slen();
  if (l > 20)
    {
      l1 = line.copy(0,20);
      if (l > 40)
	{
	  l2 = line.copy(20,20);
	  if (l > 60)
	    {
	      l3 = line.copy(40,20);
	      l4 = line.copy(60,l-60);
	    }
	  else
	    {
	      l3 = line.copy(40,l-40);
	      l4 = "";
	    }
	}
      else
	{
	  l2 = line.copy(20,l-20);
	  l3 = "";
	  l4 = "";
	}
    }
  else
    {
      l1 = line;
      l2 = "";
      l3 = "";
      l4 = "";
    }
}

html_object html_generator::directory_entry_static( board brd, int index )
{
  html_object obj;

  html_tabular tab;
  tab.set_border(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,directory_entry_head(brd,index));
  tab.add_to_body();

  String line = brd.get_msg(index);
  String l1,l2,l3,l4;
  split_entry_line(line,l1,l2,l3,l4);

  html_tabular tab2;
  tab2.set_border(0);

  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,String("<pre>")+l1+String("</pre>"));
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,String("<pre>")+l2+String("</pre>"));
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,String("<pre>")+l3+String("</pre>"));
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,String("<pre>")+l4+String("</pre>"));
  tab2.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,tab2,"FFFF00");
  tab.add_to_body();

  obj.add(tab);

  return obj;
}

html_object html_generator::directory_entry_dynamic( board brd, int index )
{
  html_object obj;

  html_tabular tab;
  tab.set_border(2);

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,directory_entry_head(brd,index));
  tab.add_to_body();

  String line = brd.get_msg(index);
  String l1,l2,l3,l4;
  split_entry_line(line,l1,l2,l3,l4);

  html_form_text_input li1(String("m")+itoS(index)+String("l1"),l1,20);
  html_form_text_input li2(String("m")+itoS(index)+String("l2"),l2,20);
  html_form_text_input li3(String("m")+itoS(index)+String("l3"),l3,20);
  html_form_text_input li4(String("m")+itoS(index)+String("l4"),l4,20);
  html_tabular tab2;
  tab2.set_border(0);

  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,li1);
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,li2);
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,li3);
  tab2.add_to_body();
  tab2.new_row();
  tab2.add_cell(html_align(html_align::al_left),false,li4);
  tab2.add_to_body();

  tab.new_row();
  tab.add_cell(html_align(html_align::al_left),false,tab2,"FFFF00");
  tab.add_to_body();

  obj.add(tab);

  return obj;
}

html_object html_generator::directory_change_forms( board brd )
{
  html_object obj;

  html_tabular form_tab;
  form_tab.set_border(0);

  form_tab.new_row();
  form_tab.add_cell(html_align(html_align::al_left),false,mldg.find(1208));
  form_tab.add_to_body();

  form_tab.new_row();
  html_object o1;
  o1.add(mldg.find(1209)+String(" "));
  o1.add(html_form_text_input("lt",itoS(brd.get_deflt()),3));
  o1.add(String(" ")+mldg.find(1210)+String(" "));
  o1.add(destination_form());
  form_tab.add_cell(html_align(html_align::al_left),false,o1);
  form_tab.add_to_body();

  html_tabular but_tab;
  but_tab.set_border(0);
  
  but_tab.new_row();
  but_tab.add_cell(html_align(html_align::al_left),false,html_form_submit(mldg.find(1206)));
  but_tab.add_cell(html_align(html_align::al_right),false,html_form_reset(mldg.find(1207)));
  but_tab.add_to_body();

  form_tab.new_row();
  form_tab.add_cell(html_align(html_align::al_left),false,but_tab);
  form_tab.add_to_body();

  obj.add(form_tab);
  return obj;
}
const String &html_generator::directory( board brd, bool sysop )
{
  static String out;
  html_object frame;
  board::permissions perm;

  html_tabular main;
  html_object main_frame;

  if (sysop)
    perm = board::perm_forw;
  else
    perm = brd.get_permission(usr.user_call());

  main.set_border(2);
  main.set_caption(mldg.find(1034)+String(' ')+brd.get_name());

  main.new_row();
  main.add_cell(html_align(html_align::al_left),false,directory_head(brd));
  main.add_to_body();
 
  if (sysop)
    {
      main.new_row();
      main.add_cell(html_align(html_align::al_left),false,directory_ops(brd));
      main.add_to_body();
    }
  for (int i = 1 ; i <= 10 ; i++)
    {
      main.new_row();
      if (perm == board::perm_no)
	main.add_cell(html_align(html_align::al_left),false,directory_entry_static( brd,i ));
      else
	main.add_cell(html_align(html_align::al_left),false,directory_entry_dynamic( brd,i ));
      
      main.add_to_body();
    }

  if (perm != board::perm_no)
    {
      main.new_row();
      main.add_cell(html_align(html_align::al_left),false,directory_change_forms(brd));
      main.add_to_body();

      html_form form(html_form::m_get,String("/dir/")+brd.get_name()+String("/change"));
      form.add(main);
      main_frame.add(form);
    }
  else
    main_frame.add(main);

  if (sysop)
    {
      html_tabular systab;
      systab.set_spacing(10);

      systab.new_row();
      systab.set_rowalign(html_valign(html_valign::val_top));
      systab.add_cell(html_align(html_align::al_left),false,main_frame);

      html_object liste;
      liste.add("<ul>\n");
      liste.add("<li> ");
      liste.add(html_link(String("/dir/")+brd.get_name()+"/add","txt","Board Operator hinzuf&uuml;gen"));
      liste.add("<li> ");
      liste.add(html_link(String("/dir/")+brd.get_name()+"/del","txt","Board Operator l&ouml;schen"));
      liste.add("</ul>\n");
      systab.add_cell(html_align(html_align::al_left),false,liste);
      systab.add_to_body();

      frame = standard_frame(mldg.find(1034)+String(" ")+brd.get_name(),systab);

    }
  else
    frame = standard_frame(mldg.find(1034)+String(" ")+brd.get_name(),main_frame);

  out = frame.write(); 

  return out;
}

const String &html_generator::change_board( board brd )
{
  board::permissions perm;
  fwd_api fwd ;

  String dest;
  String ltstring;

  get_postarea("destin=",dest);
  get_postarea("lt=",ltstring);

  int lt = ltstring.Stoi();
  destin ds;
  try
    {
      ds = destin(dest);
    }
  catch( Error_no_destin )
    {
      ds = get_default_destin();
    }
  if (usr.is_sysop())
    perm = board::perm_forw;
  else
    perm = brd.get_permission(usr.user_call());
  
  for (int i = 1; i <= 10 ; i++ )
    {
      String ln = "";
      for (int j = 1 ; j <= 4; j++ )
	{
	  String lf;
	  get_postarea(String("m")+itoS(i)+String("l")+itoS(j)+String("="),lf);
	  lf.append("                     ");
	  lf = lf.copy(0,20);
	  ln.append(lf);
	}
      ln.kuerze();
      if (ln !=  brd.get_msg(i))
	{
	  brd.set_msg(ln,i,ds,lt);
#ifdef COMPILE_SLAVES
	  spool.spool_bul(G_mycall,zeit(),brd.get_brd_id(),i,ln,false,ds,128);
#endif
	  if (perm == board::perm_forw)
	    fwd.fwd_bul(usr.user_call(),brd.get_name(),i,ds,128,lt,ln);
	}
    }
  return directory(brd,usr.is_sysop());
}

const String &html_generator::do_directory( const String &url )
{
  boards brds;

  if (url.in("/dir/"))
    {
      unsigned int p = url.pos("/dir/");
      if (url.slen() > p + 5)
	{
	  unsigned int q = p +5;
	  // Nach naechstem SChraegstrich suchen
	  while (q < url.slen() && url[q] != '/')
	    q++;

	  String bname;
	  if (q < url.slen())
	    bname = url.copy(p+5,q-p-5);
	  else
	    bname = url.copy(p+5,url.slen()-p-5);
	  try
	    {
	      board brd(bname,configuration);
	      if ( q < url.slen() )
		{
		  int k = url.pos("?");
		  String action;
		  if (k > 0 && k > (int) q)
		    action = url.copy(q,k-q);
		  else
		    action = url.copy(q,url.slen()-q);
		  if (action == "/addok")
		    return addboardop(brd);
		  else if (action == "/delok")
		    return delboardop(brd);
		  else if (action == "/add")
		    return addboardop_form(brd);
		  else if (action == "/del")
		    return delboardop_form(brd);
		  else if (action == "/change")
		    return change_board(brd);
		  else
		    throw Error_html_generator_command_unknown();
		}
	      else
		return directory(brd,usr.is_sysop());
	    }
	  catch( Error_could_not_open_boardfile )
	    {
	      throw Error_html_generator_error(String("Error : No Skyper-Board ")+bname);
	      // return "";
	    }
	}
      else
	return directory();
    }
  else
    {
      return directory();
    }
}

