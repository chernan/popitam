/*
 * This file is part of the Popitam software
 * Copyright (C) 2009 Swiss Institute of Bioinformatics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifndef TREE_H_
#define TREE_H_
#define STRMAX 50    //pour classes David
#include "Sequence.h"

//-----------------------------------------------------------------------------
//********** classes instanciables arbres de mots- D. Hernandez
//-----------------------------------------------------------------------------
// tree structure to store the all explored word

class Node;

//classe allegee pour popitam: pas besoin de pointeur a memoriser
class Qmemory{

	double score;
	unsigned int hit;

public:

	Qmemory(double); //amenage pour popitam
	~Qmemory();      //amenage popitam
	inline void oneMore() {hit++;}
	inline unsigned int getHit() {return hit;}
	inline double getScore() {return score;}
	inline void setScore(double value) { score=value; }
};

//classe representant l'arbre des mots deja vus
class TreeWord {

  Node * root;
 
 public:
   TreeWord();
  ~TreeWord();

  void destroy(Node*);
  void initialise();
  
  Node* insert(char*, double);                    //popi

  Qmemory* search(char *);
  void findUnused(char**, int&);
  void findUnusedAfter(Node*, char**, int&, int);

  inline unsigned int countWord() {return countWord(root);}
  inline unsigned int countHit() {return countHit(root);}
  double getRedondance();

 private:
  unsigned int countWord(Node*);
  unsigned int countHit(Node*);
  Node* search(char*, Node*);
  Node* parse(char**, int*);
  Node* parse(char**, Node*, int*);
  Node* split(Node*,int);
  Node* insert(char*, double, Node*, int);        //popi

};

class Node  {

 public:
  Node    *fils;
  Node    *frere;
  Qmemory *dejaVu;

 public:
 int larc;
 char str[STRMAX];
  Node();
  ~Node();

  inline void setFrere( Node* node) {     frere = node;}
  inline void setFils( Node* node) {     fils = node;}
  inline void setDejaVu( Qmemory* m) {     dejaVu = m;}

  inline Node* getFrere() {return frere;}
  inline Node* getFils() {return fils;}
  inline Qmemory* getDejaVu() {return dejaVu;}

  
  void setStr(char*, int, int);
  inline char* getStr() {return str;}
};

#endif
