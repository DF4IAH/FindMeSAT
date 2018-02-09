/****************************************************************************
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2001,2002 by Holger Flemming                               *
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
 *  		                                			    *
 ****************************************************************************/

#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdlib.h>
#include "container_errors.h"

template<class Key>
class table
{

 protected:

  Key *array;
  bool *filled;
  int siz;
  int anz;
  int length;

 public:

  friend class iterator
    {
    private:
      int index;
      table<Key> *tab;

    public:
      inline iterator(int i = 0)
	{
	  index = i;
	  tab = NULL;
	}

      inline iterator( table<Key> &m, int i = 0 ) // Konstruktor
      {
	index = i;
	tab = &m;
      }

      inline int get_index( void )
	{
	  return index;
	}

      inline table<Key>* get_tab( void )
	{
	  return tab;
	}

      iterator operator++ ( void ) // preinkrement
	{
	  index ++;
	  while (index < tab->siz && !tab->filled[index])
	    index++;
	  if (index >= tab->siz)
	    index = -1;
	  return *this;
	}

      iterator operator-- ( void ) // Predekrement
	{
	  index --;
	  while (index >= 0 && !tab->filled[index])
	    index--;
	  if (index < 0)
	    index = -1;
	  return *this;
	}

      Key & operator*( void ) // Dereferenzierungsoperator *
	{
	  if (tab == NULL) throw Error_no_set_Specified_in_iterator();
	  if ((index < 0 ) || (index >= tab->siz)) throw Error_index_out_of_range();
	  if (!tab->filled[index]) throw Error_index_out_of_range();
	  return tab->array[index];
	}

      Key* operator->( void ) // Dereferenzierungsoperator ->
	{
	  if (tab == NULL) throw Error_no_set_Specified_in_iterator();
	  if ((index < 0 ) || (index >= tab->siz)) throw Error_index_out_of_range();
	  if (!tab-> filled[index]) throw Error_index_out_of_range();
	  return &(tab->array[index]);
	}

      friend bool operator== ( const iterator &i1, const iterator &i2)
      {
	return (i1.index == i2.index) && (i1.tab == i2.tab);
      }
      friend bool operator!= ( const iterator &i1, const iterator &i2)
      {
	return (i1.index != i2.index) || (i1.tab != i2.tab);
      }
    };

 public:
  inline table( void ) // Defaultkonstruktor
    {
      array = NULL;
      filled = NULL;
      siz = 0;
      anz = 0;
      length = 0;
    }

  inline ~table( void ) // Destruktor
    {
      delete [] array;
      delete [] filled;
    }

  // Wegen der dynamischen Komponenten jetzt auch Copy-Konstruktor und = - Operator

  table( const table<Key>  &m) 
    {
      siz = m.length;
      anz = m.anz;
      length = m.length;
      array = new Key [siz];
      filled = new bool [siz];

      for (int i = 0; i < length; i++ )
	{
	  if (m.filled[i])
	    array[i] = m.array[i];
	  filled[i] = m.filled[i];
	}
    }

  const table<Key>& operator= ( const table<Key> &m )
    {
      if (this == &m)
	return *this;
      else
	{
	  anz = m.anz;
	  length = m.length;
	  if (siz < length)
	    {
	      delete [] array;
	      delete [] filled;
	      siz = length;
	      array = new Key [siz];
	      filled = new bool [siz];
	    }
	  for (int i = 0;i < length;i++)
	    {
	      if (m.filled[i])
		array[i] = m.array[i];
	      filled[i] = m.filled[i];
	    }
	  return *this;
	}
    }

  iterator add( const Key& k )
    {
      if (anz + 1 > siz)
	{
	  Key *tmpK;
	  bool *tmpb;
	  siz += 10;
	  tmpK = new Key [siz];
	  tmpb = new bool [siz];
	  for (int i = 0; i < siz; i++)
	    if (i < length)
	      {
		if (filled[i])
		  tmpK[i] = array[i];
		tmpb[i] = filled[i];
	      }
	    else
	      tmpb[i] = false;
 
	  delete [] array;
	  delete [] filled;
	  array = tmpK;
	  filled = tmpb;
	}
      for (int i = 0;i < siz;i++)
	if (!filled[i])
	  {
	    array[i] = k;
	    filled[i] = true;
	    if (i >= length)
	      length = i+1;
	    anz++;
	    return iterator(*this,i);
	  }
      return iterator(*this,-1);
    }

  void erase ( iterator & i) // element aus Array entfernen.
    {
      if (i.get_tab() != this) 
	throw Error_iterator_points_to_different_container();
      int ind = i.get_index();

      if ( (ind >= 0) && (ind < length)) // Löschung erfolgt nur, wenn Index innerhalb des 
	filled[ind] = false;           // Arrays liegt.
      // Objekt als gelöscht markieren.
      if (ind == length -1 ) length--;
      anz--;
    }

  iterator begin ( void )
    {
      int ind = 0;
      while (ind < siz && !filled[ind])
	ind++;
      if (ind >= siz)
	ind = -1;
      return iterator( *this,ind );

    }

  // Iterator auf erstes "Element" nach dem Ende der Liste.

  iterator end( void )
    {
      return iterator ( *this, -1  );
    }

  void clear ( void )
    {
      delete [] array;
      array = NULL;
      delete [] filled;
      filled = NULL;
      siz = 0;
      length = 0;
      anz = 0;
    }
  
  int size( void )
    {
      return anz;
    }

  void push_back( const Key& k )
    {
      if (length + 1 > siz)
	{
	  Key *tmpK;
	  bool *tmpb;
	  siz += 10;
	  tmpK = new Key [siz];
	  tmpb = new bool [siz];
	  for (int i = 0; i < siz; i++)
	    if (i < length)
	      {
		if (filled[i])
		  tmpK[i] = array[i];
		tmpb[i] = filled[i];
	      }
	    else
	      tmpb[i] = false;
 
	  delete [] array;
	  delete [] filled;
	  array = tmpK;
	  filled = tmpb;
	}
      array[length] = k;
      filled[length++] = true;
      anz++;
    }

  void pop_back( Key &k )
    {
      k = array[--length];
      filled[length] = false;
      anz--;
    }
};

#endif
