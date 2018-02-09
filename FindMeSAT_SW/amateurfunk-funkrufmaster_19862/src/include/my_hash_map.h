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

#ifndef __MY_HASH__MAP_H__
#define __MY_HASH__MAP_H__

#define MAX_HASH_TAB 4096
#define HASH_TAB_MASK 0xfff

#include "container_errors.h"
#include "hashtable.h"

using namespace std;

// Die Klasse my_hash_map stellt eine gegenueber der array_map verbesserte assoziative 
// Container-Klasse dar.
// Die Daten werden in verketteten Listen gespeichert, auf die Ein Zugriff mittels
// eines hash-Algorithmus erfolgt.
// Eine Derartige Klasse ist auch in der STL enthalten. Der Zugriff darauf bereitete
// jedoch zahlreiche Probleme, sodass eine eigene Klasse implementiert wurde.

template<class Key>
class eql_to
{
 public:
  bool operator()( const Key k1, const Key k2) const
    {
      return k1 == k2;
    }
};

template<class Key, class Val, class HashFunc = hash<Key>, class EQLKey = eql_to<Key> >
class my_hash_map
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
    
    // Um eine Verkette Liste zu erstllen, muessen die Paare zusammen mit einem Zeiger auf
    // das jeweils naechste Listenelement abgespeichert werden.
    
    class list_element
    {
    public:
      list_element *next;
      pair valpair;
    };
    
    protected:
    list_element* hash_tab[MAX_HASH_TAB];
    int length;
    HashFunc Hash;
    EQLKey eql;
    
    public:
    friend class iterator
    {
    private:
      my_hash_map<Key,Val,HashFunc,EQLKey> *map;
      int tab_index;
      my_hash_map<Key,Val,HashFunc,EQLKey>::list_element *pointer;
      
    public:
      inline iterator( void )  // Default konstruktor
	{
	  map = NULL;
	  tab_index = 0;
	  pointer = NULL;
	}
      
      // Und noch ein Konstruktor
      inline iterator( my_hash_map<Key,Val,HashFunc,EQLKey> &m, int i = 0, my_hash_map<Key,Val,HashFunc,EQLKey>::list_element *pt = NULL )
	{
	  map = &m;
	  tab_index = i;
	  pointer = pt;
	}
      
      inline int get_index( void )
	{
	  return tab_index;
	}
      
      inline my_hash_map<Key,Val,HashFunc,EQLKey>::list_element *get_pointer ( void )
	{
	  return pointer;
	}
      
      inline my_hash_map<Key,Val,HashFunc,EQLKey> *get_map ( void )
	{
	  return map;
	}
      
      my_hash_map<Key,Val,HashFunc,EQLKey>::pair & operator* ( void ) // Dereferenzierungsoperator
	{
	  if (map == NULL) throw Error_no_map_Specified_in_iterator();
	  if (pointer == NULL) throw Error_index_out_of_range();
	  
	  return pointer->valpair;
	}
      
      iterator operator++ ( void ) // preinkrement
	{
	  if ((map != NULL) && (pointer != NULL))
	    {
	      pointer = pointer->next;
	      if (pointer == NULL)
		{
		  tab_index++;
		  while ((tab_index < MAX_HASH_TAB) && (map->hash_tab[tab_index] == NULL))
		    tab_index++;
		  if (tab_index < MAX_HASH_TAB)
		    pointer = map->hash_tab[tab_index];
		  else
		    pointer = NULL;
		}
	    }
	  return *this;
	}
      
      // Predekrement-Operator gibt es hier nicht.
      
      my_hash_map<Key,Val,HashFunc,EQLKey>::pair* operator->( void ) // Dereferenzierungsoperator ->
	{
	  if (map == NULL) throw Error_no_map_Specified_in_iterator();
	  if (pointer == NULL) throw Error_index_out_of_range();
	  
	  return &(pointer->valpair);
	}
      
      friend bool operator== ( const iterator &i1, const iterator &i2 )
	{
	  return (i1.map == i2.map) && (i1.tab_index == i2.tab_index) && (i1.pointer == i2.pointer);
	}
      
      friend bool operator!= ( const iterator &i1, const iterator &i2 )
	{
	  return (i1.map != i2.map) || (i1.tab_index != i2.tab_index) || (i1.pointer != i2.pointer);
	}
      
    };
    
    inline my_hash_map( void ) // Defaultkonstruktor
    {
      for (int i = 0 ; i < MAX_HASH_TAB;i++)
	hash_tab[i] = NULL;
      length = 0;
    }
    
    ~my_hash_map( void ) // Destruktor
    {
      for (int i = 0; i < MAX_HASH_TAB; i++ )
      if (hash_tab[i] != NULL )
      {
	list_element *lauf,*lauf2;
	lauf = hash_tab[i];
	while (lauf != NULL)
	{
	  lauf2 = lauf;
	  lauf = lauf->next;
	  delete lauf2;
	}
      }
    }
    
    // Wegen der dynamischen Komponenten jetzt auch Copy-Konstruktor und = -Operator
    //
    my_hash_map( const my_hash_map<Key,Val,HashFunc,EQLKey> &m)
    {
      for (int i = 0; i < MAX_HASH_TAB;i++)    // Alle Eintraege der Hash-Tabelle durchgehen
      {
	if (m.hash_tab[i] == NULL)           // Wenn in der Quell-Tabelle kein Zeiger
	hash_tab[i] = NULL;                  // eingetragen ist, dann auch in neuer
	else                                 // Tabelle NULL
	{ 
	  hash_tab[i] = new list_element;  // Sonst erstes Element der Liste anlegen
	  list_element *lauf,*nlauf;       // Laufzeiger deklarieren
	  
	  lauf = m.hash_tab[i];            // Laufzeiger auf quell-Objekte
	  nlauf = hash_tab[i];             // Laufzeiger auf Zielobjekte
	  
	  while ( lauf != NULL )           // Bis zum Ende der Liste
	    {
	      *nlauf = *lauf;              // Element kopieren
	      if (lauf->next != NULL )     // Zeiger auf weiteres Element?
		nlauf->next = new list_element; // Neues Element in Zielliste erzeugen
	      else
		nlauf->next = NULL;        // Auf Nummer Sicher gehen...
	      lauf = lauf->next;           // Laufzeiger auf jeweils naechstes Element
	      nlauf = nlauf->next;
	    }
	}
      }
      length = m.length;                       // Laenge kopieren
    }
    
    const my_hash_map<Key,Val,HashFunc,EQLKey>& operator= ( const my_hash_map<Key,Val,HashFunc,EQLKey> &m )
    {
      if ( this == &m )   // Wenn Ziel und Quelle uebereinstimmen muss nichts gemacht werden.
      return *this;
      else
      {                                         // Zunaechst alten Inhalt des Ziels loeschen
	for (int i = 0; i < MAX_HASH_TAB; i++ ) // Alle elemente der Hash-Tabelle durchgehen
	  if (hash_tab[i] != NULL )             // Wenn keine Liste anhaengt, dann braucht
	    {                                   // auch nichts geloescht zu werden.
	      list_element *lauf,*lauf2;        // Laufzeiger deklarieren
	      lauf2 = hash_tab[i];              // Laufzeiger auf Anfang der Liste
	      while (lauf2 != NULL)             // Liste bis zum Ende durchlaufen
		{
		  lauf = lauf2;                 // Zeiger auf das Element sichern
		  lauf2 = lauf->next;           // Zeiger auf das naechste Element
		  delete lauf;                  // und Element loeschen
		}
	    }
	
	// Und nun wird die quell-map zur Zielmap kopiert.
	// Der Code entspricht dem des Copy-Konstruktors
	
	for (int i = 0; i < MAX_HASH_TAB;i++)    // Alle Eintraege der Hash-Tabelle durchgehen
	  {
	    if (m.hash_tab[i] == NULL)           // Wenn in der Quell-Tabelle kein Zeiger
	      hash_tab[i] = NULL;                  // eingetragen ist, dann auch in neuer
	    else                                 // Tabelle NULL
	      {
		hash_tab[i] = new list_element;  // Sonst erstes Element der Liste anlegen
		list_element *lauf,*nlauf;       // Laufzeiger deklarieren
		
		lauf = m.hash_tab[i];            // Laufzeiger auf quell-Objekte
		nlauf = hash_tab[i];             // Laufzeiger auf Zielobjekte
		
		while ( lauf != NULL )           // Bis zum Ende der Liste
		  {
		    *nlauf = *lauf;              // Element kopieren
		    if (lauf->next != NULL )     // Zeiger auf weiteres Element?
		      nlauf->next = new list_element; // Neues Element in Zielliste erzeugen
		    else
		      nlauf->next = NULL;        // Auf Nummer Sicher gehen...
		    lauf = lauf->next;           // Laufzeiger auf jeweils naechstes Element
		    nlauf = nlauf->next;
		  }
	      }
	  }
	length = m.length;                       // Laenge kopieren
	return *this;
      }
    }
    
    
    // Hinzufuegen eines Elementes
    iterator add ( const Key &k, const Val &v )
    {
      // Zunaechst den Hash-Wert des Schluesses berechnen
      
      int hash_val = Hash(k) & HASH_TAB_MASK;
      
      list_element *pt = new list_element;  // Zeiger auf neues Element und Element selber erzeugen
      
      pt->next = NULL;                      // Next-Pointer auf NULL setzen
      pt->valpair.first = k;                // Schluesses im element setzen
      pt->valpair.second = v;               // Wert im Element setzen
      
      if (hash_tab[hash_val] == NULL)       // Bereits eine Liste vorhanden?
      hash_tab[hash_val] = pt;            // Nein, dann erstes Element der Liste
      else                                  // Sonst vorhandene Liste bis zum Ende durchlaufen
      {
	list_element *lauf;               // Lauf-Zeiger erzeugen
	lauf = hash_tab[hash_val];        // und auf Anfang der Liste setzen
	
	while ( lauf->next != NULL )      // Liste bis zum ende durchlaufen
	  lauf = lauf->next;              // 
	
	lauf->next = pt;                  // Neues Element anhaengen
      }
      length++;                             // Map ist jetzt um ein Element groesser
      return iterator(*this,hash_val,pt);   // Iterator als Rueckgabewert erzeugen
    }
    
    void erase( iterator &i )
    {
      if ( i.get_map() == this )           // Loeschen nur, wenn der Iterator auch auf
      {                                  // diese Map zeigt
	int t_index = i.get_index();     // Hash-Index des Iterators holen
	list_element *pt = i.get_pointer(); // Pointer auf LIstenelement holen
	
	if (pt != NULL)
	{
	  if ( hash_tab[t_index] == pt )   // Zu loeschendes Element gleich am Anfang der
	    {                              // Liste ?
	      hash_tab[t_index] = pt->next;// Element aus LIste heraus nehmen
	      delete pt;                   // Elemente wirklich loeschen
	      length--;                    // Liste ist um ein Element kleiner geworden
	    }
	  else
	    {
	      list_element *lauf;          // Laufzeiger definieren
	      
	      lauf = hash_tab[t_index];    // Laufzeiger auf Anfang der Liste
	      while ( ( lauf != NULL ) && ( lauf->next != pt )) // Liste durchlaufen
		lauf = lauf->next;
	      
	      if (lauf != NULL )
		{
		  lauf->next = pt->next;   // Element aus Liste herausnehmen
		  delete pt;               // Element loeschen
		  length--;                // Liste ist um ein Element kleiner geworden
		}
	    }
	}
      }
    }
    
    
    iterator find( const Key &k )
    {
      int hash_func = Hash(k) & HASH_TAB_MASK;      // Hash-WErt berechnen
      
      if ( hash_tab[hash_func] != NULL )// entsprechende Liste vorhanden?
      { 
	// Laufzeiger deklarieren und initialisieren
	list_element *lauf = hash_tab[hash_func]; 
	
	// Liste bis zum Ende oder bis zum gesuchten Element durchlaufen
	while ( ( lauf != NULL ) && ( !eql(lauf->valpair.first,k) ) )
	  lauf = lauf->next;
	
	if ( lauf != NULL )
	  return iterator(*this,hash_func,lauf);   // element gefunden, Iterator erzeugen
	else
	  return iterator(*this,MAX_HASH_TAB,NULL);// element nicht gefunden
      }
      else
      return iterator(*this,MAX_HASH_TAB,NULL);
    }
    
    // Iterator auf das erste Element des Arrays erzeugen
    
    iterator begin( void )
    {
      int i = 0;
      while ( ( i < MAX_HASH_TAB ) && ( hash_tab[i] == NULL ) )
      i++;
      if ( i < MAX_HASH_TAB )
      return iterator(*this,i,hash_tab[i]);
      else
      return iterator(*this,MAX_HASH_TAB,NULL);
    }
    
    // End Iterator erzeugen
    
    iterator end( void )
    {
      return iterator(*this,MAX_HASH_TAB,NULL);
    }
    
    // Der []-Operator
    
    Val & operator[] ( const Key &k )
    {
      iterator it = find(k);
      
      if ( it != end() )
      return it->second;
      else
      {
	it = add(k,Val());
	return it->second;
      }
    }
    
    // Map loeschen
    
    void clear ( void ) 
    {
      for (int i = 0; i < MAX_HASH_TAB; i++ )
      if (hash_tab[i] != NULL )
      {
	list_element *lauf,*lauf2;
	lauf2 = hash_tab[i];
	hash_tab[i] = NULL;
	while (lauf2 != NULL)
	{
	  lauf = lauf2;
	  lauf2 = lauf->next;
	  delete lauf;
	}
      }
      length = 0;
    }
    
    
    int size ( void )
    { 
      return length;
    }
    
    void test ( void )
    {
      for ( int i = 0 ; i < MAX_HASH_TAB ; i++ )
      {
	cerr << setw(5) << i << "->";
	if ( hash_tab[i] == NULL )
	cerr << "NULL" << endl;
	else
	{
	  list_element *lauf = hash_tab[i];
	  while ( lauf != NULL )
	    {
	      cerr << lauf->valpair.first << "->";
	      lauf = lauf->next;
	    }
	  cerr << "NULL" << endl;
	}
      }
    }
  };

#endif
