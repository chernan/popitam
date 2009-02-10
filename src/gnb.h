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

#ifndef GNB_H_
#define GNB_H_

#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <cstdio>


#include "sufftab.h"
#include "defines.h"
#include "aa.h"

using namespace std;



#define              MAX_TAILLE       256
typedef unsigned int GRANDENT;
const int            LGMAX           = 20;
const                GRANDENT racine =  1;
const                GRANDENT RIEN   =  0;

struct intervalle {
  int deb;
  int fin;
  bool sensindirect;
};

//*********************************************************************************

//définition de la classe seq qui contient la séquence à analyser 
class seq  {
 public:
  GRANDENT taille;
  vector<char> tab;
  seq();
  seq(char [LGMAX+1]);
  ~seq();
  void aff();
  void facteur();
};

//*********************************************************************************

// définition de la classe slli généralisé
class slligen  {

  public:
    // sous classes
    //représente une occurrence dans vecseq
  class occurrence {
  public:
    GRANDENT debut;
    GRANDENT fin;
    occurrence(GRANDENT deb,GRANDENT fin){debut=deb; fin=fin;};
    occurrence(){debut=0; fin=0;};
    inline void change(GRANDENT d,GRANDENT f) {if (d<=f) {debut=d,fin=f;}
                                               else {debut=0;fin=0;}}
    GRANDENT longueur()                       {if (debut<=fin) {return(fin-debut+1);}
                                               else {return (0);}}
  };
  
  //adresse dans un des 2 tableaux avec un bool pour préciser lequel (noeud ou feuille) 
  // A CHANGER->vecteurs de bool       
  class NOEUILLE {
  public:
    GRANDENT num;
    bool feuille;
    NOEUILLE(){num=RIEN;};
    NOEUILLE(GRANDENT i, bool j){num = i; feuille = j;};
  };               
 
  //PUBLIC ET PRIVATE A CLARIFIER 
  // elts de tbranch (noeuds)
  class ELTBRANCH {
  public:public:
    NOEUILLE firstchild;
    NOEUILLE branchbrother;
    GRANDENT depth;
    GRANDENT headposition;
    GRANDENT suffixlink;
    ELTBRANCH(){firstchild.num=RIEN; branchbrother.num=RIEN; depth=RIEN; headposition=RIEN; suffixlink=1;}       
    inline void ini() {firstchild.num=RIEN; branchbrother.num=RIEN; depth=RIEN; headposition=RIEN; suffixlink=1;}
  };
  
  //noeud virtuel = noeud réel + indice + fils + fils précédent
  class noeudvirt {
  public:
    GRANDENT num;
    GRANDENT ind;
    NOEUILLE fils;
    NOEUILLE filsprec;
    noeudvirt() {num=0; ind=0; fils.num=0; filsprec.num=0;};
    noeudvirt(GRANDENT n,GRANDENT i,NOEUILLE f,NOEUILLE p) {num=n; ind=i; fils=f; filsprec=p;};
    inline void change(GRANDENT n,GRANDENT i,NOEUILLE f) {num=n; ind=i; fils=f;};
    inline void change(GRANDENT n,GRANDENT i,NOEUILLE f,NOEUILLE p) {num=n; ind=i; fils=f; filsprec=p;};
    inline GRANDENT longchaine(){return (ind);};
    inline bool reel(){return (ind==0);};
   };
 
//pour stocker les occurences et les longueurs des sequences
 
  class TABSEQ { 
  public:
    GRANDENT position;
    GRANDENT longueur;
    TABSEQ(){};
    TABSEQ(GRANDENT p, GRANDENT l){position = p; longueur = l;}
    inline void setPosition(GRANDENT p){position = p;}
    inline void setLongueur(GRANDENT l){longueur = l;}
    inline GRANDENT getPosition(){return position;}
    inline GRANDENT getLongueur(){return longueur;}
  };
  
 
 // feuilles et noeuds                     
 vector<NOEUILLE> tfeuilles; 
 vector<ELTBRANCH> tbranch;
 vector<TABSEQ> tabseq;
 
 // FONCTIONS DE SLLIGEN
 void construction_arbre();             
 void insertleaf(const noeudvirt ,const GRANDENT ,const GRANDENT );
 void insertbranch(noeudvirt & ,const GRANDENT,const GRANDENT ,const GRANDENT);
 void scanprefix (noeudvirt &,GRANDENT &);
 void rescan (noeudvirt &,const GRANDENT ,const occurrence );                  
 char valarc(const NOEUILLE ,const NOEUILLE ,const GRANDENT );  
 NOEUILLE freresuiv(NOEUILLE ); 

 /*retourne la longueur de l'arc pere->fils ou 1000000000 si
   le fils est une feuille.
   je ne sais pas pourquoi mais ça parait nescessaire pour la construction
   de l'arbre*/
 GRANDENT longueurarc(const NOEUILLE ,const NOEUILLE );

/*c'est pourqois j'ai réecris une autre fonction qui renvoie toujours
  la longueur de l'arc, meme si le fils est une feuille.*/
 GRANDENT longueurarc2(const NOEUILLE ,const NOEUILLE );  
             
 void virtoreel(noeudvirt ,GRANDENT & ,occurrence & );
 //int sequenceassociee(NOEUILLE );

 void treeToTab(sufftab*, char*, aa*, char*);
 void tTT(NOEUILLE, NOEUILLE, int*, sufftab*, char*, aa*, char*);
 void parcourNodes(const NOEUILLE);

 void parcourArc(const NOEUILLE);
 void completeParcour();
 void parcour(NOEUILLE, NOEUILLE);
 int  sequenceIndice(GRANDENT p);
 void ecrit(FILE* fpTree, NOEUILLE pere);
 void parcourFeuilles(int, int*, int*, NOEUILLE pere, int* f, NOEUILLE papa, NOEUILLE fils,int* n);
 void parcourFeuilles(NOEUILLE pere);
 void cherche(NOEUILLE pere,NOEUILLE fils, char * mot, int p, bool fini);
 bool cherche2(NOEUILLE pere, NOEUILLE fils, char* mot, int p, bool fini);
 bool occ(char*);
 void elague();
 NOEUILLE fils(NOEUILLE);
 // vecteur avec toutes les séquences 
 vector<char> vecseq;
 
 
 
 int sequence_courante;
 
 
 GRANDENT tailleADN;
 
 
 slligen();
 void init(char*);
 ~slligen(){};
 
 void aff();
 GRANDENT nbfeuilles(const NOEUILLE, vector<GRANDENT> & );
 GRANDENT facteur_present(seq ,vector<GRANDENT> &);
 
 
 
 bool scan(noeudvirt &  ,const char );
 
 
 
};

//slligen * pointarb;

//vector<GRANDENT>  candi ;

#endif
