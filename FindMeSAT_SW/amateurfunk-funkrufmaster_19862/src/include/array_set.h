#ifndef __ARRAY_SET_H__
#define __ARRAY_SET_H__

#include <stdlib.h>
#include "container_errors.h"

// Die Klasse array_map stellt eine einfache assoziative Container-Klasse dar.
//
// Die Daten werden intern in einem dynamisch verwarlteten Array gespeichert,
// Die Suche nach einzelnen Elementen erfolgt sehr ineffizient.
// Die Klasse ist nur uebergangsweise zur Benutzung vorgesehen, bis eine
// effizientere Klasse unter Einsatz eines Hash-Algorithmus existiert.
//

template<class Key>
class array_set
{

  // Daten werden paarweise in der pair-Klasse gespeichert. Das Paar besteht aus dem
  // Key und dem Wert.

 protected:

  Key *array;
  int siz;
  int length;

 public:

  // Die Iterator-Klasse

  friend class iterator
    {
     private:
      int index;
      array_set<Key> *set;

     public:
      inline iterator( int i = 0 ) // Default-Konstruktor
	{
	  index = i;
	  set = NULL;
	}

      inline iterator( array_set<Key> &m, int i = 0 ) // Konstruktor
      {
	index = i;
	set = &m;
      }

      inline int get_index( void )
	{
	  return index;
	}

      Key & operator*( void ) // Dereferenzierungsoperator *
	{
	  if (set == NULL) throw Error_no_set_Specified_in_iterator();
	  if ((index < 0 ) || (index > set->siz)) throw Error_index_out_of_range();

	  return set->array[index];
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
      Key* operator->( void ) // Dereferenzierungsoperator ->
	{
	  if (set == NULL) throw Error_no_set_Specified_in_iterator();
	  if ((index < 0 ) || (index > set->siz)) throw Error_index_out_of_range();

	  return &(set->array[index]);
	}

      friend bool operator== ( const iterator &i1, const iterator &i2)
      {
	return (i1.index == i2.index) && (i1.set == i2.set);
      }
      friend bool operator!= ( const iterator &i1, const iterator &i2)
      {
	return (i1.index != i2.index) || (i1.set != i2.set);
      }
    };

  

 public:
  inline array_set( void ) // Defaultkonstruktor
    {
      array = NULL;
      siz = 0;
      length = 0;
    }

  inline ~array_set( void ) // Destruktor
    {
      if (array != NULL)
	delete [] array;
    }

  // Wegen der dynamischen Komponenten jetzt auch Copy-Konstruktor und = - Operator

  array_set( const array_set<Key>  &m) 
    {
      siz = m.siz;
      length = m.length;
      array = new Key [siz];

      for (int i = 0; i < length; i++ )
	array[i] = m.array[i];

    }

  const array_set<Key>& operator= ( const array_set<Key> &m )
    {
      if (this == &m)
	return *this;
      else
	{
	  length = m.length;
	  if (siz < length)
	    {
	      delete [] array;
	      siz = length;
	      array = new Key [siz];
	    }
	  for (int i = 0;i < length;i++)
	    array[i] = m.array[i];
	  return *this;
	}
    }

  // HInzufügen eines Elementes erfolgt am Ende des Arrays.

  iterator add( const Key& k )
    {

      if (length + 1 > siz) // Überprüfen ob der Platz ausreicht, ggf. Array vergrößern
	{
	  Key *tmp;
	  siz += 10;
	  tmp = new Key [siz];

	  for (int i = 0; i < length; i++ )
	    tmp[i] = array[i];

	  delete [] array;
	  array = tmp;
	}

      array[length++] = k;
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
	found = k == array[i++];

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


  // Array-Map löschen.

  void clear ( void )
    {
      if (array != NULL )
	{
	  delete [] array;
	  array = NULL;
	}
      siz = 0;
      length = 0;
    }
  
  int size( void )
    {
      return length;
    }

  void push_back(const Key &k )
    {
      if (length + 1 > siz) // Überprüfen ob der Platz ausreicht, ggf. Array vergrößern
	{
	  Key *tmp;
	  siz += 10;
	  tmp = new Key [siz];

	  for (int i = 0; i < length; i++ )
	    tmp[i] = array[i];

	  delete [] array;
	  array = tmp;
	}

      array[length++] = k;
    }

  void pop_back( Key &k )
    {
      k = array[--length];
    }

};


#endif
