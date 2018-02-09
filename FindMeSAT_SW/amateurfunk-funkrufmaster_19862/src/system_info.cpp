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
 * Jens Schoon, DH6BB		  email : dh6bb@darc.de			    *
 *                                PR    : dh6bb@db0whv.#nds.deu.eu          *
 *                                                                          *
 ****************************************************************************/

#include "system_info.h"
#include "autoconf.h"

#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

vector<struct thread_info> system_info::tlist;
vector<struct thread_info>::iterator system_info::tlist_it;


long cpu_messung::sys_clk_tck;

String system_info::get_compile_options( void )
{
  String tmp;

#ifdef COMPILE_DEBUG
  tmp.append("Debugging ");
#endif
#ifdef COMPILE_AX25
  tmp.append("AX25 ");
#endif
#ifdef COMPILE_TCP
  tmp.append("TCP ");
#endif
#ifdef COMPILE_CLUSTER
  tmp.append("DX-Cluster-Spion ");
#endif
#ifdef COMPILE_WX
  tmp.append("WX-Auslese ");
#endif
#ifdef COMPILE_SAT
  tmp.append("SAT ");
#endif
#ifdef COMPILE_ASTRO
  tmp.append("Astro ");
#endif
#ifdef COMPILE_TIDE
  tmp.append("Tiden ");
#endif
#ifdef COMPILE_SLAVES
  tmp.append("Slaves ");
#endif

  return tmp;
}

String system_info::get_cpu_typ( void )
{
  ifstream cpuinfo("/proc/cpuinfo");
  String line;
  String name("");
  while (cpuinfo)
    {
      line.getline(cpuinfo,200);
      if (line.slen() > 10)
	{
	  if (line.copy(0,10) == String("model name"))
	    {
	      unsigned int ind = 0;
	      while (ind < line.slen() && line[ind] != ':')
		ind++;
	      ind += 2;
	      int len = line.slen() - ind;
	      if (len > 0)
		name = line.copy(ind,len);
	    }
	}
    }
  return name;
}

double system_info::get_cpu_speed( void )
{
  ifstream cpuinfo("/proc/cpuinfo");
  String line;
  String name("");
  while (cpuinfo)
    {
      line.getline(cpuinfo,200);
      if (line.slen() > 7)
	if (line.copy(0,7) == String("cpu MHz"))
	  {
	    unsigned int ind = 0;
	    while (ind < line.slen() && line[ind] != ':')
	      ind++;
	    
	    ind += 2;
	    int len = line.slen() - ind;
	    if (len > 0)
	      name = line.copy(ind,len);
	  }
    }
  double f = atof(strtochar(name));
  return f;
}

double system_info::get_bogomips( void )
{
  ifstream cpuinfo("/proc/cpuinfo");
  String line;
  String name("");
  while (cpuinfo)
    {
      line.getline(cpuinfo,200);
      if (line.slen() > 8)
	if (line.copy(0,8) == String("bogomips"))
	  {
	    unsigned int ind = 0;
	    while (ind < line.slen() && line[ind] != ':')
	      ind++;
	    
	    ind += 2;
	    int len = line.slen() - ind;
	    if (len > 0)
	      name = line.copy(ind,len);
	  }
    }
  double f = atof(strtochar(name));
  return f;
}

void system_info::get_mem( void  )
{
  ifstream memstat("/proc/meminfo");
  String line;
  shared_mem = 0;
  while (memstat)
  {
    line.getline(memstat,200);
    if (line.slen()>8)
    {
        if (line.copy(0,9) == String("MemTotal:"))
    	    tot_mem=get_mem_size(line);
	if (line.copy(0,8) == String("MemFree:"))
	    free_mem=get_mem_size(line);
        if (line.copy(0,12) == String("SharedTotal:"))
	    shared_mem=get_mem_size(line);
	if (line.copy(0,10) == String("SwapTotal:"))
	    tot_swap=get_mem_size(line);
        if (line.copy(0,9) == String("SwapFree:"))
    	    free_swap=get_mem_size(line);
    }
  }
  used_mem = tot_mem - free_mem;
  used_swap = tot_swap - free_swap; // hier used-swap = free swap
}

long int system_info::get_mem_size( String zeile  )
{
    long int size;
    unsigned int i=0, j=0;
    while (zeile[i]!=' ' && zeile.slen()>i) i++;
    while (zeile[i]==' ' && zeile.slen()>i) i++;
    j=i;
    while (zeile[j]!=' ' && zeile.slen()>j) j++;
    size=(zeile.copy(i, j-i+1)).Stoi();
    return (size*1024); // in kB
}


void system_info::clear_tlist( void )
{
  tlist.clear();
  tlist_it = tlist.end();
}

void system_info::set_tlist( struct thread_info ent )
{
  tlist.push_back(ent);
  tlist_it = tlist.end();
}

bool system_info::get_tlist(bool beg, struct thread_info &ent )
{
  if (beg)
    tlist_it = tlist.begin();
  if (tlist_it != tlist.end())
    {
      ent = *tlist_it;
      tlist_it++;
      return true;
    }
  else
    return false;
}


void system_info::get_process_status( void )
{
  pid_t p = getpid();
  String puffer = "";
  ostringstream fname;
  String line;
  String zpuf;
  fname << "/proc/" << p << "/status" << (char) 0;
  puffer.append(fname);
  ifstream proc_stat(strtochar(puffer));
  while (proc_stat)
    {
      line.getline(proc_stat,99);
      if (line.in("VmSize:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_size = zpuf.Stoi() * 1024;
	}
      if (line.in("VmLck:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_lck = zpuf.Stoi() * 1024;
	}
      if (line.in("VmRSS:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_rss = zpuf.Stoi() * 1024;
	}
      if (line.in("VmData:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_data = zpuf.Stoi() * 1024;
	}
      if (line.in("VmStk:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_stack = zpuf.Stoi() * 1024;
	}
      if (line.in("VmExe:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_exec = zpuf.Stoi() * 1024;
	}
      if (line.in("VmLib:"))
	{
	  zpuf = line.copy(9,8);
	  pmem_lib = zpuf.Stoi() * 1024;
	}
    }
}

int system_info::get_descriptor_anz( void )
{
  pid_t p = getpid();
  String puffer = "";
  ostringstream fname;
  fname << "/proc/" << p << "/fd" << (char) 0;
  puffer.append(fname);
  DIR *fds;
  fds = opendir(strtochar(puffer));
  int cnt = 0;
  if (fds != NULL)
    {
      struct dirent *eintrag;
      while ((eintrag = readdir(fds)) != NULL)
	{
	  if ( (strcmp(eintrag->d_name,".") != 0) && (strcmp(eintrag->d_name,"..")))
	    cnt++;
	}
      closedir(fds);
    }
  return cnt;
}

void system_info::Print_descriptoren( ostream &ostr, char cr )
{
  pid_t p = getpid();
  String puffer = "";
  ostringstream fname;
  fname << "/proc/" << p << "/fd" << (char) 0;
  puffer.append(fname);
  DIR *fds;
  fds = opendir(strtochar(puffer));
  if (fds != NULL)
    {
      struct dirent *eintrag;
      while ((eintrag = readdir(fds)) != NULL)
	{
	  if ( (strcmp(eintrag->d_name,".") != 0) && (strcmp(eintrag->d_name,"..")))
	    { 
	      ostr << setw(5) << eintrag->d_name << " ==> ";
	      char compl_name[200];
	      char link_name[200];
	      strncpy(compl_name,strtochar(puffer),199);
	      strcat(compl_name,"/");
	      strcat(compl_name,eintrag->d_name);
	      int i = readlink(compl_name,link_name,sizeof(link_name));
	      if (i > 0 && i < (int) sizeof(link_name))
		{
		  link_name[i] = '\0';
		  ostr << link_name;
		}
	      ostr << cr;
	    }
	}
      closedir(fds);
    }
}


void PrintDouble4( ostream &strm, double m )
{
  int ganz;
  int frac;

  ganz = (int) m;

  if (ganz >= 100)
    strm << setw(4) << ganz;

  else if (ganz >= 10)
    {
      strm << setw(2) << ganz << ',';
      frac = (int) ((m - (double) ganz) * 10);
      strm << setw(1) << frac;
    }
  else
    {
      strm << setw(1) << ganz << ',';
      frac = (int) ((m - (double) ganz) * 100);
      strm << setw(2) << setfill('0') << frac << setfill(' ');
    }
}

void PrintMem( ostream &strm, long int m )
{
  double mem = (double) m;
  char vorsilbe = ' ';
  if (mem >= 1024.)
    {
      if (mem < 1048576.)
	{
	  mem /= 1024.;
	  vorsilbe = 'K';
	}
      else if (mem < 1073741824. )
	{
	  mem /= 1048576.;
	  vorsilbe = 'M';
	}
      else if (mem < 1.099511628e+12 )
	{
	  mem /= 1073741824.;
	  vorsilbe = 'G';
	}
      else
	{
	  mem /= 1.099511628e+12;
	  vorsilbe = 'T';
	}
      PrintDouble4(strm,mem);
      strm << ' ' << vorsilbe;
    }
  else
    strm << setw(4) << m << "  ";
}


String MemtoS( long int m )
{
  String puffer = "";
  ostringstream buf;

  PrintMem(buf,m);
  buf << ends;
  puffer.append(buf);
  return String(puffer);
}
