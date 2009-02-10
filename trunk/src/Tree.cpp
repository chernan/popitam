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

#include <cstdio>
#include <iostream>


#include "Tree.h"
#include <cstring>

using namespace std;

//-----------------------------------------------------------------------------------------------
 // Qmemory, developpe par D. Hernandez
//---------------------------------------
// L'arbre des mots contient des mots complets (en feuille ou noeuds) qui engendrent une allocation Qmemory
// et des embranchements qui ne sont pas des mots, auquel cas Qmemory est null.
// Si un mot complet est dans un noeud, on cree un noeud amont (sans embranchement) avec toutes les infos Qmemory

// Pour popitam j'utilise les fonctions search et insert(ou je garde uniqt le score)

// FONCTIONS CONSTRUCTEURS DESTRUCTEURS
Qmemory::Qmemory(double s) : score(s),
                             hit(1)
{
}

Qmemory::~Qmemory() {
}


TreeWord::TreeWord() {
   root = new Node();
}

TreeWord::~TreeWord() {
   destroy(root);
}
// FONCTIONS MEMBRES
void TreeWord::destroy(Node* n) {
  if (n->dejaVu)
    delete n->dejaVu;
  if (n->frere)
    destroy(n->frere);
  if (n->fils)
    destroy(n->fils);
  delete n;
}

void TreeWord::initialise() {
// pour repartir sur un arbre neuf
  destroy(root);
  root = new Node();
}

//**Count all the leaves, that is to say all the nodes that have dejavu!=null
unsigned int TreeWord::countWord(Node * n) {
  unsigned int count = 0;
  if (n->dejaVu) count ++;
  if (n->frere)
    count += countWord(n->frere);
  if (n->fils)
    count += countWord(n->fils);
  return count;
}

double TreeWord::getRedondance() {
  unsigned int nWord=countWord();
  unsigned int nHit=countHit(root);
  return 1-(double)nWord/nHit;
}

unsigned int TreeWord::countHit(Node* n) {
  unsigned int hit=0;
  if (n->dejaVu) hit+=n->dejaVu->getHit();
  if (n->frere)
    hit+=countHit(n->frere);
  if (n->fils)
    hit+=countHit(n->fils);
  return hit;
}

Qmemory* TreeWord::search(char* mot) {                                     //utlise par Popitam
//recherche si le mot est deja existant
// Si OUI, retourne ptr Qmemory, si NON retourne 0
  Node * theNode = search(mot, root);
  if (theNode) {
    theNode->dejaVu->oneMore();
    return theNode->dejaVu;
  }
  else
    return 0x0;
}

Node* TreeWord::search(char* mot, Node* n) {
  bool descend = 1;
  bool cote = 0;
  int i;
  int l = n->larc;

  for ( i=1; i<=l; i++) {
    if ( *mot != n->str[i-1] ) {
      if ( i > 1) {
        descend = cote = 0;
        i=l+1;
        n = 0x0;
      }
      else {
        descend = false;
        cote = true;
        i = n->larc+1;//on sort de la boucle
      }
    }
    else {
      mot++;
    }
  }
   
  if (*mot == '\0') {
    descend = cote = 0;
  }
 
  /*FILS*/  
  if (descend) 
    if(n->fils) {
      n=search(mot,n->fils);
    }
 
  /*FRERE*/
  if(cote) 
    if (n->frere) {
      n=search(mot, n->frere);
    }
    else
      n=0x0;
 
  return n;
  
}  

void TreeWord::findUnused(char** unusedAC, int &posInTab) {
	int posInString=0;
	return findUnusedAfter(root, unusedAC, posInTab, posInString);
}

void TreeWord::findUnusedAfter(Node* n, char** unusedAC, int &posInTab, int posInString) {
	strcpy(unusedAC[posInTab]+posInString, n->getStr());
        if(n->dejaVu) { //on a une 'feuille'
		if( n->dejaVu->getScore()==0.0) {
                	strncpy(unusedAC[posInTab+1], unusedAC[posInTab], posInString);
                	//copier n->getStr() dans unusedAC[posInTab]+posInString
                	//pas necessaire car unusedAC[posInTab] contient deja l'AC (cf 1ere ligne de la fonction)
                	posInTab++;
		}
        }
 
	if(n->fils) { //attention, on n'a pas forcément une feuille
		findUnusedAfter(n->fils, unusedAC, posInTab, posInString+(int)strlen(n->getStr()) );
	}
	if(n->frere) { //attention! une feuille peut avoir un fils (quand il y a dans l'arbre des chaines de longueur différentes) et peut aussi avoir un frere
		findUnusedAfter(n->frere, unusedAC, posInTab, posInString );
	}
	return;
}

/* retourne un pointeur sur le noeud dans lequel s'est arreté le parsing
   *mot contient la tableau de char qui n'a pas matché
   *Position contient la derniere position qui a matché dans le noeud retourné */
Node* TreeWord::parse(char** mot,int* position) {
  return parse(mot, root, position);
}

Node* TreeWord::parse(char** mot, Node* n, int* position) {
  bool descend = 1;
  bool cote = 0;
 // int i;
 // int l = n->larc;
  *position = 0;
  char* p= *mot;
  char* s= n->str;

  if( *s != '\0') {
    if( *p != *s ) {
      descend = false;
      cote = true;
    }
    else {
      p++;
      s++;
      
      while (*s != '\0') {
        if( *p != *s ) {
          descend = cote = 0;
          break;
        }
        else {
          p++;
          s++;
        }
      }
    }
  }
  
  *position = (int)(p-(*mot));
  if (*p == '\0') {
    descend = cote = 0;
  }
  
  /*FILS*/  
  if (descend) 
    if(n->fils) {
      (*mot)=p;
      n=parse(mot,n->fils, position);
    }
  /*FRERE*/
  if (cote) 
    if (n->frere) {     
      n=parse(mot, n->frere, position);
    }
  
  return n;
}


Node* TreeWord::split(Node* n,int position) {
  //  int i;
  //  int count = 0;
  Node * tmp= new Node();
  tmp->setFils(n->getFils());
  n->setFils(tmp);
  tmp->setStr(n->str,position+1,n->larc-position+1);
  tmp->setDejaVu(n->getDejaVu());
  n->str[position]='\0';
  n->larc = (int)strlen(n->str);
  n->setDejaVu(NULL);

  return tmp;
}

Node::Node() : fils(NULL),
               frere(NULL),
               dejaVu(NULL),
               larc(0) {
  str[0]='\0';
}

Node::~Node() {}

void Node::setStr(char* source, int offset, int longueur) {
  source = source+(offset-1);
  memcpy(str,source,longueur);
  str[longueur]='\0';
  larc = (int)strlen(str);
}

//***********************************
//usage POPITAM, pour optiPeptide

Node* TreeWord::insert(char* mot,
                       double fit)
                      {
  int position;
  Node * theNode;
  theNode=parse(&mot, &position);
  return insert(mot,fit,theNode,position);
 
}

Node* TreeWord::insert(char* mot, 
                       double fit, 
                       Node* theNode, 
                       int position)
{

  int l = theNode->larc;
  int lStr = (int)strlen(mot);

  if (position == l && position == lStr) {//match a la fin de theNode
    if (theNode->dejaVu) {
      theNode->dejaVu->oneMore();
      return NULL;
    }
    else {
      theNode->setDejaVu(new Qmemory(fit));
      return theNode;
    }    
  }
  else if (position == lStr) {//position == lStr && position != l
    theNode=split(theNode,position);
    theNode->setDejaVu(new Qmemory(fit));
    return theNode;
  }

  else if (position == l) {//position != lStr && position == l
    theNode->setFils(new Node());
    theNode = theNode->getFils();
    theNode->setStr(mot,position+1,lStr);
    theNode->setDejaVu(new Qmemory(fit));
    return theNode;
  }
  else if (position == 0) {//position != lStr && position != l
    theNode->setFrere(new Node);
    theNode = theNode->getFrere();
    theNode->setDejaVu(new Qmemory(fit));
    theNode->setStr(mot,1,(int)strlen(mot));
    return theNode;
    }
  else {//position != lStr && position != l && position != 0
    theNode=split(theNode,position);
    theNode->setFrere(new Node);
    theNode = theNode->getFrere();
    theNode->setDejaVu(new Qmemory(fit));
    theNode->setStr(mot,position+1,(int)strlen(mot));
    return theNode;
  }
}
//Fin POPITAM
