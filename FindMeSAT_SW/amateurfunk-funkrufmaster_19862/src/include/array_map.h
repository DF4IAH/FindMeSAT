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

#ifndef __ARRAY_MAP_H__
#define __ARRAY_MAP_H__

#include "container_errors.h"

// Die Klasse array_map stellt eine einfache assoziative Container-Klasse dar.
//
// Die Daten werden intern in einem dynamisch verwarlteten Array gespeichert,
// Die Suche nach einzelnen Elementen erfolgt sehr ineffizient.
// Die Klasse ist nur uebergangsweise zur Benutzung vorgesehen, bis eine
// effizientere Klasse unter Einsatz eines Hash-Algorithmus existiert.
//
//
// Bei den Dereferenzierungoperatoren kann es zu den Ausnahmen
// Error_no_map_Specified_in_iterator und 
// Error_index_out_of_range
// kommen.



template<class Key, class Val>
class array_map
{

  // Daten werden paarweise in der pair-Klasse gespeichert. Das Paar besteht aus dem
  // Key und dem Wert.

 public:
  class pair
    {
     public:
      Key first;
      Val second;
    };

 protected:

  struct pair *array;
  int ar_size;
  int length;

 public:

  // Die Iterator-Klasse

  friend class iterator
    {
     private:
      int index;
      array_map<Key,Val> *map;

     public:
      inline iterator( int i = 0 ) // Default-Konstruktor
	{
	  index = i;
	  map = NULL;
	}

      inline iterator( array_map<Key,Val> &m, int i = 0 ) // Konstruktor
      {
	index = i;
	map = &m;
      }

      inline int get_index( void )
	{
	  return index;
	}

      struct pair & operator*( void ) // Dereferenzierungsoperator *
	{
	  if (map == NULL) throw Error_no_map_Specified_in_iterator();
	  if ((index < 0 ) || (index > map->ar_size)) throw Error_index_out_of_range();

	  return map->array[index];
	}

      iterator operator++ ( void ) // preinkrement
	{
	  index ++;
	  return *this;
	}
      iterator operator-- ( void ) // Predekrement
	{
	  index --;
	  return *this;
	}
      struct array_map<Key,Val>::pair* operator->( void ) // Dereferenzierungsoperator ->
	{
	  if (map == NULL) throw Error_no_map_Specified_in_iterator();
	  if ((index < 0 ) || (index > map->ar_size)) throw Error_index_out_of_range();

	  return &(map->array[index]);
	}

      friend bool operator== ( const iterator &i1, const iterator &i2)
      {
	return (i1.index == i2.index) && (i1.map == i2.map);
      }
      friend bool operator!= ( const iterator &i1, const iterator &i2)
      {
	return (i1.index != i2.index) || (i1.map != i2.map);
      }
    };

  

 public:
  inline array_map( void ) // Defaultkonstruktor
    {
      array = NULL;
      ar_size = 0;
      length = 0;
    }

  inline ~array_map( void ) // Destruktor
    {
      if (array != NULL)
	delete [] array;
    }

  // Wegen der dynamischen Komponenten jetzt auch Copy-Konstruktor und = - Operator

  array_map( const array_map<Key,Val>  &m) 
    {
      ar_size = m.ar_size;
      length = m.length;
      array = new struct pair [ar_size];

      for (int i = 0; i < length; i++ )
	array[i] = m.array[i];

    }

  const array_map<Key,Val>& operator= ( const array_map<Key,Val> &m )
    {
      if (this == &m)
	return *this;
      else
	{
	  length = m.length;
	  if (ar_size < length)
	    {
	      delete [] array;
	      ar_size = length;
	      array = new struct pair [ar_size];
	    }
	  for (int i = 0;i < length;i++)
	    array[i] = m.array[i];
	  return *this;
	}
    }

  // HInzufügen eines Elementes erfolgt am Ende des Arrays.

  iterator add( const Key& k, const Val& v)
    {
      struct pair p;
      p.first = k;
      p.second = v;

      if (length + 1 > ar_size) // Überprüfen ob der Platz ausreicht, ggf. Array vergrößern
	{
	  struct pair *tmp;
	  ar_size += 10;
	  tmp = new struct pair [ar_size];

	  for (int i = 0; i < length; i++ )
	    tmp[i] = array[i];

	  delete [] array;
	  array = tmp;
	}

      array[length++] = p;
      return iterator(*this,length-1);
    }

  void erase ( iterator & i) // element aus Array entfernen.
    {
      int ind = i.get_index();

      if ( (ind >= 0) && (ind < length)) // Löschung erfolgt nur, wenn Index innerhalb des 
	{                                // Arrays liegt.
	  for (int j = ind ; j < length-1 ; j++ ) // Restlichen Elemente um eine Position nach
	    array[j] = array[j+1];                // vorne kopieren
	  length--;
	}
    }

  // Ein bestimmtes Element anhand des Schlüssels Key suchen.

  iterator find ( const Key &k )
    {
      int i = 0;
      bool found = false;

      while (i < length && !found )
	found = k == array[i++].first;

      if (found)
	return iterator(*this,--i);
      else 
	return iterator( *this, length );
    }

  // Iterator auf Anfang der Liste liefern.

  iterator begin ( void )
    {
      return iterator( *this,0 );
    }

  // Iterator auf erstes "Element" nach dem Ende der Liste.

  iterator end( void )
    {
      return iterator ( *this, length  );
    }
  
  Val & operator[] ( const Key &k )
    {
      int i = 0;
      bool found = false;
      
      while ( i < length && !found )
	found = k == array[i++].first;
      
      if (found)
	return array[--i].second;
      else
	{
	  add(k,Val());
	  return array[length-1].second;
	}
    }
  
  // Array-Map löschen.
  
  void clear ( void )
    {
      if (array != NULL )
	{
	  delete [] array;
	  array = NULL;
	}
      ar_size = 0;
      length = 0;
    }

  int size( void )
    {
      return length;
    }

  void push_back( const Key& k, const Val& v)
    {
      struct pair p;
      p.first = k;
      p.second = v;

      if (length + 1 > ar_size) // Überprüfen ob der Platz ausreicht, ggf. Array vergrößern
	{
	  struct pair *tmp;
	  ar_size += 10;
	  tmp = new struct pair [ar_size];

	  for (int i = 0; i < length; i++ )
	    tmp[i] = array[i];

	  delete [] array;
	  array = tmp;
	}

      array[length++] = p;
    }

  void pop_back( Key &k, Val &v )
    {
      struct pair p;
      p = array[--length];
      k = p.first;
      v = p.second;
    }

};


#endif
