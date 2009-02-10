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

// Construction d'apres S.Kurtz "reducing the space requirement of suffix trees"
// version "SLLI"

// ********************************************************************************************** //

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "gnb.h"
#include "error.h"
#include "utils.h"
#include "defines.h"
#include "sufftab.h"
#include <cctype>

using namespace std;

// ********************************************************************************************** //

// methodes de la classe sequence

seq::seq() {tab.push_back('~');taille=0;}; /*sequence vide*/

seq::~seq() {tab.~vector();}            

seq::seq(char nomfich [LGMAX+1])              
{char c;
 ifstream entree (nomfich,ios::in);
 
 if (!entree) {cout << " ouverture impossible \n"; exit(1) ;};
 tab.push_back(c); /*case 0 inutilisee*/               
 while (entree>>c) {tab.push_back(c);};
 tab.push_back('~'); /*marqueur de fin*/
 taille=(GRANDENT)tab.size()-2;                            
 
 entree.close();
 cout << " OK ";
 
}

void seq::facteur(){
}        


//void seq::aff(){
//     cout <<"\n";
//    for (int i=1; i<taille+1; i++) {cout<<i <<":"  <<tab[i] <<" - ";};
//              }


//******************************************************************************  
// methodes de slligen


/*elague les feuilles '~' */
void slligen::elague() {

  //NOEUILLE root(1,0);
  NOEUILLE fils = tbranch[1].firstchild;
  NOEUILLE next;

  while (fils.feuille && vecseq[fils.num]=='~') {
    tbranch[1].firstchild.num = tfeuilles[fils.num].num;
    fils = tfeuilles[fils.num];
  }

  do { //pour tous les fils
   
    if (fils.feuille) 
      next = tfeuilles[fils.num];
    else
      next = tbranch[fils.num].branchbrother;

    if (next.feuille) {
      if (vecseq[next.num] == '~') {
	if (fils.feuille) {
	  tfeuilles[fils.num] = tfeuilles[next.num];
	  next = tfeuilles[fils.num];
	}
	else {
	  tbranch[fils.num].branchbrother = next;
	  next = tbranch[fils.num].branchbrother;
	}
      }
      else
	fils = next;
    }
    else {
      fils = next;
     
    }
  } while (next.num != 0);
}
	/**********************************************************************/
/*retourne l'indice de la sequence concernee par la position p*/
/*ou 0 si p > que la taille de vecseq*/
int slligen::sequenceIndice(GRANDENT p) {

  int i = 1;
  if ( p >= vecseq.size()) return 0;
  while (!(p>= tabseq[i].position && p <= tabseq[i].position + tabseq[i].longueur) ) {
    i++;
      }
  return i;
}
/**********************************************************************/

/*retourne le ième car de la chaine associee à l'arc pere-->ind */                         
inline char slligen::valarc(const NOEUILLE ind,const NOEUILLE pere,const GRANDENT ieme) {
  if (ind.feuille) 
    {return vecseq[ind.num+tbranch[pere.num].depth+ieme-1];}
  else 
    {return
       vecseq[tbranch[ind.num].headposition+tbranch[pere.num].depth+ieme-1];
    }                     
}
/**********************************************************************/
/*retourne le frère suivant de ind */             
inline slligen::NOEUILLE slligen::freresuiv(NOEUILLE ind)
{if (ind.feuille) {return (tfeuilles[ind.num]);}
 else {return (tbranch[ind.num].branchbrother);};
}
/**********************************************************************/

/*retourne de fils de ind*/
inline slligen::NOEUILLE slligen::fils(NOEUILLE ind) {

  if (ind.feuille) {
    NOEUILLE rien(0,0); return rien;
  }
  else
    return tbranch[ind.num].firstchild;
}

/**********************************************************************/

void slligen::treeToTab(sufftab* suffTab, char* dtbseq, aa* aaParam, char* fileErrorName) {
  int n = 1; 
  NOEUILLE root(1,0);
  suffTab[0].state = 0;
  tbranch[1].suffixlink = 0;
 
  tTT(root, tbranch[root.num].firstchild, &n, suffTab, dtbseq, aaParam, fileErrorName);

  if (n >= sufftab::size) fatal_error(fileErrorName, MEMORY, "in function slligen::treeToTab()");
  suffTab[n].longu = 1000; //marque la fin 
  // je peux mettre n-1 pour enlever la dernière séquence (vide)  
  
}

/**********************************************************************/

/*parcours des arcs en dessous de pere*/

/*
void slligen::parcourNodes(const NOEUILLE pere) {
  int f = 0;
  int n = 0;
  if(!pere.feuille) {// si c'est un noeud interne
    parcourFeuilles(pere, &f, pere, pere, &n);  // le deuxième pere ne changera pas tout au long des fonctions
    // récursives; il est nécessaire pour ensuite trouver les longueurs des arcs 
    // pour l'instant, le troisième pere ne sert à rien, mais bientôt il contiendra le fils
    parcourNodes(tbranch[pere.num].firstchild);
  }
  
  if(pere.feuille) { // si c'est une feuille
    if (tfeuilles[pere.num].num) // si elle a un frère
      parcourNodes(tfeuilles[pere.num]);
  } 
  else 
    if (tbranch[pere.num].branchbrother.num) 
      parcourNodes(tbranch[pere.num].branchbrother);

}

*/
/**********************************************************************/

/*parcours des arcs en dessous de pere*/
void slligen::parcourArc(const NOEUILLE pere) {
  
  if (pere.feuille)
    cout << "rien\n";
  else {

    NOEUILLE fils = tbranch[pere.num].firstchild;
  parcour (pere,fils);
  }
}

/**********************************************************************/

// parcours tous les arcs en dessous de pere
void slligen::completeParcour() {
  NOEUILLE root(1,0);
  parcour(root, tbranch[root.num].firstchild);
}

/**********************************************************************/

/*parcours récursivement tous les arc en dessous de pere->fils*/
void slligen::parcour(NOEUILLE pere,NOEUILLE fils) {

  int l,i;

  
  l = longueurarc2(fils,pere);

  for (i=1; i<=l; i++)
  cout << valarc(fils,pere,i);
  
  if (fils.feuille) cout << " : " << fils.num;
  cout << endl;

  /*FILS*/  
  if (fils.feuille == 0) { //si le fils n'est pas une feuille 
   //   pere = fils;
   //   fils = tbranch[fils.num].firstchild;
    
  parcour(fils,tbranch[fils.num].firstchild);
  }
  
  /*FRERE*/
   if (fils.feuille) {//si le fils est une feuille
    if(tfeuilles[fils.num].num) {//s'il a un frere

  //    fils = tfeuilles[fils.num];//passage au frere suivant
      parcour(pere,tfeuilles[fils.num]);
    }
  }
  else { //le fils est un noeud interne

  // fils = tbranch[fils.num].branchbrother;//passage au frere suivant
    if (tbranch[fils.num].branchbrother.num)
      parcour(pere,tbranch[fils.num].branchbrother);
  }
 
}


/**********************************************************************/

/*parcours récursivement tous les arc en dessous de pere->fils et place les infos dans tab*/

void slligen::tTT(NOEUILLE pere,NOEUILLE fils, int*n, sufftab* suffTab, char* dtbseq, aa* aaP, char* fileErrorName) {
  
    suffTab[*n].state = fils.feuille;
    suffTab[*n].longu = longueurarc2(fils,pere)+ tbranch[pere.num].depth;  
    
    //    suffTab[*n].indPere = *k;  
    
    if (fils.feuille) {
      suffTab[*n].pos1 = fils.num-1;
      suffTab[*n].pos2 = suffTab[(*n)].pos1 + tbranch[pere.num].depth;
    }
    else {
      suffTab[*n].pos1 = tbranch[fils.num].headposition-1;
      suffTab[*n].pos2 = suffTab[*n].pos1 + tbranch[pere.num].depth;
    }   
    
                            //for (int i = 0; i < suffTab[*n].longu; i++) {suffTab[*n].subseq[i] = dtbseq[suffTab[*n].pos1+i];};
    for (int i = 0; i < int(suffTab[*n].longu - tbranch[pere.num].depth); i++) {
      suffTab[*n].subseq[i] = dtbseq[suffTab[*n].pos2 + i];
    }
    suffTab[*n].subseq[int(suffTab[*n].longu - tbranch[pere.num].depth)] = '\0';
    
    char toto  = valarc(fils, pere, 1); // retourne le premier aa de l'arc fils pere
    int  indice;
    if (toto != '~') {
      indice = aaP->findIndice(toto);
      if ((indice == -1) | (indice > aaP->aa1Nb)) fatal_error(fileErrorName, DEBUG, "in function slligen::tTT");
    }
    else indice = aaP->aa1Nb;  // IL Y A UNE PLACE RESERVEE POUR ~ DANS LES ACIDES AMINES

    //    if (!((suffTab[*n].pos1 == suffTab[*n].pos2) && (suffTab[*n].pos1 == (int)strlen(dtbseq)))) 
      suffTab[tbranch[pere.num].suffixlink].succ[indice] = *n;
      // ECRIRE OU PAS LE SUCCESSEUR DE LA SEQUENCE VIDE
    
      (*n)++; if ((*n) > sufftab::size) fatal_error(fileErrorName, MEMORY, "in function slligen::tTT()");

  /*FILS*/  
  if (!fils.feuille) { //si le fils n'est pas une feuille
    tbranch[fils.num].suffixlink = (*n)-1;
    tTT(fils,tbranch[fils.num].firstchild, n, suffTab, dtbseq, aaP, fileErrorName);    
  }
  
  /*FRERE*/
  if (fils.feuille) {//si le fils est une feuille
    
    // *k = suffTab[*n].indPere;
    
    if(tfeuilles[fils.num].num) {//s'il a un frere
      tTT(pere,tfeuilles[fils.num], n, suffTab, dtbseq, aaP, fileErrorName);
    }
  }
  else { //le fils est un noeud interne
    if (tbranch[fils.num].branchbrother.num) {
      //    tbranch[pere.num].suffixlink = (*n)-1;
      tTT(pere,tbranch[fils.num].branchbrother, n, suffTab, dtbseq, aaP, fileErrorName);
    }
  }
}


/**********************************************************************/

/* parcours toutes les feuilles en dessous de pere et stocke les positions dans un tableau */
/*
void slligen::parcourFeuilles(NOEUILLE courant, int* f, NOEUILLE papa, NOEUILLE fils, int*n) {
  int pos, l, indice1, indice2;
  NOEUILLE next;
  if (courant.feuille) {              // si c'est une feuille
    if (courant.num!= 0) {            // si elle existe
     
      pos = courant.num + tbranch[papa.num].depth;
      l = longueurarc2(fils, papa);
      
      if (pos <= seqLenght) {  // NECESSAIRE
	indice1 = liliRose.i1_dtbSequence[pos-1];
	//tbranch[papa.num].iAA1dep[indice1].num     = papa.num;
	//tbranch[papa.num].iAA1dep[indice1].feuille = papa.feuille;
	//tbranch[papa.num].iAA1arr[indice1].num     = fils.num;
	//tbranch[papa.num].iAA1arr[indice1].feuille = fils.feuille;
	//tbranch[papa.num].l1[indice1]              = l;
      }
      
      
      if (pos < seqLenght) {
	indice2 = liliRose.i2_dtbSequence[pos-1];
	
	//tbranch[papa.num].iAA2dep[indice2].num     = papa.num;
	//tbranch[papa.num].iAA2dep[indice2].feuille = papa.feuille;
	//tbranch[papa.num].iAA2arr[indice2].num     = fils.num;
	//tbranch[papa.num].iAA2arr[indice2].feuille = fils.feuille;
	//tbranch[papa.num].l2[indice2]              = l;
      }
      (*f)++;
    }
  }
  else {                              //c'est un noeud interne
    next = tbranch[courant.num].firstchild;
    // on le fait seulement si next est un noeud interne et qu'il se trouve à un seul niveau en dessous de papa
    // puisque le but c'est de pouvoir retrouver la longueur de l'arc entre papa et le noeud en dessous
    // on ne veut pas sauter des noeuds dans l'arbre
    if (*n == 0) {
      // si on n'est pas encore descendu dans l'arbre (on veut que le fils soit juste à un niveau
      // en dessous du père
      fils = tbranch[courant.num].firstchild; 
    }
  (*n)++;

    // le fils = firstchild ne peut se faire forcément qu'une seule fois. Par contre, le fils = freresuiv() peut
    // se faire plusieurs fois, mais seulement à un seul niveau au dessous du père
    do {
      parcourFeuilles(next, f, papa, fils, n); 
        if (*n == 1)  fils = freresuiv(next);
      next = freresuiv(next);
    } while (next.num);
    (*n)--;
  }
}
*/
/**********************************************************************/

/* parcours toutes les feuilles en dessous de pere */

void slligen::parcourFeuilles(NOEUILLE pere) {

  NOEUILLE next;
  if (pere.feuille) {//si c'est une feuille
    if (pere.num!= 0) {//si elle existe 
      //    cout << pere.num << " : ";
    }
  }
  else {//c'est un noeud interne
    next = tbranch[pere.num].firstchild;
    do {
      parcourFeuilles(next);
      next = freresuiv(next);
    } while (next.num);
  }

}

/**********************************************************************/

bool slligen::occ(char * mot) {
  NOEUILLE root(1,0);
  NOEUILLE fils = tbranch[root.num].firstchild;
  if (strlen(mot) != 0) {
    //   cout << "recherche de : " << mot << endl;
    return cherche2(root,fils,mot,0,0);  // remplacer par cherche() si on veut les occurences
  }
  else {
    return false;
  }
}

/**********************************************************************/

// retourne false si l'occurence n'est pas présente (ne cherche pas à parcourir
// tout l'arbre et retourner toutes les occurences)

bool slligen::cherche2(NOEUILLE pere,NOEUILLE fils, char *mot, int p, bool fini) {
  GRANDENT l,i;
    char x;
    bool descend = true;
    bool cote = false;

    if (!fini) {
    l = longueurarc2(fils,pere);
    
    for (i=1; i<=l; i++) {
      x = valarc(fils,pere,i);
      
      if (toupper(x) != toupper(mot[p])){//si l'arc ne correspond plus
	if(i>1) {//la recherche est finie ->aucune occurence
	  fini = true;
	  descend=cote=false;
	  i=l;//sort de la boucle for
	  p--;//sinon bug
	  p++; return false; 
	}
	else {//on va voir a l'arc frere
	  descend = false;
	  cote = true;
	  i=l;//on quite la boucle for
	  p--;	
	}
      }
      p++;
      
      if (unsigned(p)>=strlen(mot)) {
	fini = true;
	descend = false;
	cote = false;
	i=l;
	return true;
	//parcourFeuilles(fils);
      }
    }
    
     /*FILS*/  
    if (descend) {
      if (fils.feuille == 0) { //si le fils n'est pas une feuille 
	//   pere = fils;
	//   fils = tbranch[fils.num].firstchild;
	
	return cherche2(fils,tbranch[fils.num].firstchild,mot,p,fini);
      }
      return false;
    }
    if(cote) {
      /*FRERE*/
      if (fils.feuille) {//si le fils est une feuille
	if(tfeuilles[fils.num].num) {//s'il a un frere
	  return cherche2(pere,tfeuilles[fils.num],mot,p,fini);
	}
	return false;
      }
      else { //le fils est un noeud interne
	if (tbranch[fils.num].branchbrother.num) {//s'il a un frere
	  return cherche2(pere,tbranch[fils.num].branchbrother,mot,p,fini);
	}
	return false;
      }
    }
  }

    // ne devrait pas arriver là
    return false;
}


/**********************************************************************/

/*retourne la feuille ou noeud dont mot est le plus grand prefixe
  ou null s'il n'existe pas*/
void slligen::cherche(NOEUILLE pere,NOEUILLE fils, char * mot, int p, bool fini) {
  GRANDENT l,i;
    char x;
    bool descend = true;
    bool cote = false;

    if (!fini) {
    l = longueurarc2(fils,pere);
    
    for (i=1; i<=l; i++) {
      x = valarc(fils,pere,i);
      
      if (toupper(x) != toupper(mot[p])){//si l'arc ne correspond plus
	if(i>1) {//la recherche est finie ->aucune occurence
	  fini = true;
	  descend=cote=false;
	  i=l;//sort de la boucle for
	  p--;//sinon bug
	}
	else {//on va voir a l'arc frere
	  descend = false;
	  cote = true;
	  i=l;//on quite la boucle for
	  p--;	
	}
      }
      p++;
      
      if (unsigned(p)>=strlen(mot)) {
	fini = true;
	descend = false;
	cote = false;
	i=l;
	parcourFeuilles(fils);
      }
    }
    
    /*FILS*/  
    if (descend) {
      if (fils.feuille == 0) { //si le fils n'est pas une feuille 
	//   pere = fils;
	//   fils = tbranch[fils.num].firstchild;
	
	cherche(fils,tbranch[fils.num].firstchild,mot,p,fini);
      }
    }
    if(cote) {
      /*FRERE*/
      if (fils.feuille) {//si le fils est une feuille
	if(tfeuilles[fils.num].num) {//s'il a un frere
	  
	  //    fils = tfeuilles[fils.num];//passage au frere suivant
	  cherche(pere,tfeuilles[fils.num],mot,p,fini);
	}
      }
      else { //le fils est un noeud interne
	
	// fils = tbranch[fils.num].branchbrother;//passage au frere suivant
	if (tbranch[fils.num].branchbrother.num)
	  cherche(pere,tbranch[fils.num].branchbrother,mot,p,fini);
      }
    }
  }
}

/**********************************************************************/


/*retourne la longueur de l'arc*/                   
inline GRANDENT slligen::longueurarc(const NOEUILLE fils,const NOEUILLE pere)    
{  if (fils.feuille)  {
 return 1000000000;//sinon, la construction ne fonctionne pas
//  int seq = sequenceIndice(fils.num);
//  return tabseq[seq].longueur - fils.num - tbranch[pere.num].depth;
}  //infini
 else                                                        
   {return (tbranch[fils.num].depth-tbranch[pere.num].depth);}
}

/******************************************************/

/*idem mais renvoie aussi longueur arc pere->feuille*/                   

inline GRANDENT slligen::longueurarc2(const NOEUILLE fils,const NOEUILLE pere)  {

  if (fils.feuille)  {
    
    int seq = sequenceIndice(fils.num);
    GRANDENT p = tabseq[seq].position;
    GRANDENT l = tabseq[seq].longueur;
    
    return l - (fils.num - p) - tbranch[pere.num].depth;
  }  
  
  else {
    return (tbranch[fils.num].depth-tbranch[pere.num].depth);
  }
}

/**********************************************************/

/* ecrit dans 'fout' la structure de l'arbre parenthesée */

void slligen::ecrit(FILE* fpTree, NOEUILLE pere) {
    
  if(pere.feuille) {// c'est une feuille
    fprintf(fpTree, "F%i", pere.num);
    if (tfeuilles[pere.num].num)//s'il a un frere
      fprintf(fpTree, ",");
  }
  else {//c'est un noeud interne
    fprintf(fpTree, "N%ih%id%i(", pere.num, tbranch[pere.num].headposition, tbranch[pere.num].depth);
    tbranch[pere.num].suffixlink =1000;
    // suffixlink n'est utilisé que pour la construction de l'arbre.
    // ici je l'utilise pour marquer la fin des noeuds;
    ecrit(fpTree,tbranch[pere.num].firstchild);
  }
  
  if(pere.feuille) {
    if (tfeuilles[pere.num].num)
      ecrit(fpTree,tfeuilles[pere.num]);
    else
      fprintf(fpTree, ")");
  }
  else {
    if (tbranch[pere.num].branchbrother.num) 
      ecrit(fpTree,tbranch[pere.num].branchbrother);
    else
      fprintf(fpTree, ")");
  }
}
/**********************************************************************/
/*décompose un noeud virtuel*/
inline void slligen::virtoreel(noeudvirt n,GRANDENT & a,occurrence & o) {
  a = n.num;
  if(n.fils.feuille) {
    o.debut=n.fils.num+tbranch[n.num].depth;
  }
  else {
    o.debut=tbranch[n.fils.num].headposition+tbranch[n.num].depth;
  }
  o.fin=o.debut+n.ind-1;
}              

/**********************************************************************/

/* insère la feuille-suffixe suffinser  avec tail=i à l'emplacement e */  

inline void slligen::insertleaf(const noeudvirt  e,const GRANDENT  i,const GRANDENT suffinser)
{NOEUILLE temp=tbranch[e.num].firstchild; /*temp:=fils courant*/              
 NOEUILLE empl;empl.num=e.num; empl.feuille=false; /*empl:=emplacement e*/
 NOEUILLE nouvfeuille;
 if /* ordre lexicographique ok on ajoute à gauche */
   (valarc(temp,empl,1) > vecseq[i])
   {tbranch[empl.num].firstchild.num=suffinser;
   tbranch[empl.num].firstchild.feuille=true;
   nouvfeuille.num=temp.num; /* frere droit */
   nouvfeuille.feuille=temp.feuille;}
 else                 
   {while(((freresuiv(temp).num) != RIEN) && (valarc(freresuiv(temp),empl,1)< vecseq[i]))
     /* tt que on n'a pas trouve le bon emplacement ds la liste des freres */    
     {temp=freresuiv(temp);}
   /* on a trouve */ 
   /*on ajoute entre temp et son frere*/
   
   /*on actualise le lien frere de la feuille inseree*/                         
   nouvfeuille.num=freresuiv(temp).num;
   nouvfeuille.feuille=freresuiv(temp).feuille;
   
   if (temp.feuille) /*frere precedent de la feuille a inserer est 1 feuille*/
     {tfeuilles[temp.num].num=suffinser;
     tfeuilles[temp.num].feuille=true;}
   else     /* si noeud */
     {tbranch[temp.num].branchbrother.num=suffinser;                                                                         tbranch[temp.num].branchbrother.feuille=true;};
   
   
   }
 tfeuilles.push_back(nouvfeuille); 
 
}
/**********************************************************************/
// insertion de noeud            
inline void slligen::insertbranch(noeudvirt & empvirt,const GRANDENT  t,const GRANDENT suffinser,const GRANDENT tbder) 
  /*on insère la feuille suffinser avec tailptr==t en empvirt, tbder=no du noeud cree*/
{NOEUILLE temppere;
 temppere.num=empvirt.num;temppere.feuille=false;
 NOEUILLE tempfils=empvirt.fils;
 
 NOEUILLE tempfilspred=empvirt.filsprec;                
 NOEUILLE tempfeuille;
 tempfeuille.num=suffinser;tempfeuille.feuille=true;
 ELTBRANCH nouvnoeud; nouvnoeud.ini();
 NOEUILLE adnouvnoeud;
 NOEUILLE nouvfeuille; 
 adnouvnoeud.num=tbder; adnouvnoeud.feuille=false;
 
 /*init communes à tous les cas */
 if (tempfilspred.feuille)
   {tfeuilles[tempfilspred.num]=adnouvnoeud;}
 else {tbranch[tempfilspred.num].branchbrother=adnouvnoeud;};
 nouvnoeud.branchbrother=freresuiv(tempfils);
 nouvnoeud.depth=tbranch[empvirt.num].depth+(empvirt.ind);
 nouvnoeud.headposition=suffinser;// A VERIFIER
 if (empvirt.num==racine && empvirt.ind==1)
   {nouvnoeud.suffixlink=racine;};
 
 if (tbranch[temppere.num].firstchild.num==tempfils.num
     && tbranch[temppere.num].firstchild.feuille==tempfils.feuille)
   /*on est sur le premier fils*/
   {tbranch[temppere.num].firstchild=adnouvnoeud;};
 
 if (vecseq[t] < valarc(tempfils,temppere,empvirt.ind+1))
   /*on insère à gauche*/
   {nouvnoeud.firstchild=tempfeuille;
   nouvfeuille=tempfils;
   
   if (tempfils.feuille)
     {tfeuilles[tempfils.num].num=RIEN;}
   else {tbranch[tempfils.num].branchbrother.num=RIEN;};
   }
 
 else   /* on insère à droite */
   {nouvnoeud.firstchild=tempfils;
   nouvfeuille.num=RIEN;
   if (tempfils.feuille)
     {tfeuilles[tempfils.num]=tempfeuille;}
   else {tbranch[tempfils.num].branchbrother=tempfeuille;};
   }
 tbranch.push_back(nouvnoeud); /*le nouveau noeud est insere*/  
 tfeuilles.push_back(nouvfeuille); /*la nouvelle feuille est inseree*/ 
 empvirt.fils=adnouvnoeud; /*headloc est mis à jour*/                
}
/**********************************************************************/

/* ajoute le plus long prefixe de S(t) contenu dans l'arbre  à h(reel), sa longueur à t   =slowfind */             
inline void slligen::scanprefix(noeudvirt & h,GRANDENT & t) {
  NOEUILLE tempfils=tbranch[h.num].firstchild;
  NOEUILLE temppere; 
  temppere.num=h.num;
  temppere.feuille=false;
  GRANDENT indtemp=1;
  GRANDENT longarc=longueurarc(tempfils,temppere);
  bool pasfini=true;
  while (pasfini) {

    h.filsprec.num=RIEN;              
    /* tt que on n'a pas trouve un bon emplacement ds la liste des freres */ 
    while ( (( freresuiv(tempfils).num) != RIEN) 
	 && (valarc(tempfils,temppere,1) != vecseq[t]) ) {
      
      h.filsprec=tempfils;
      tempfils=freresuiv(tempfils);
    };
   
   if (valarc(tempfils,temppere,1)!=vecseq[t]) {
     pasfini=false;
     h.change(temppere.num,0,tempfils);
   }
   else {
     longarc=longueurarc(tempfils,temppere);
     
     /* tt que l'arc courant correspond, on le parcourt*/
     while ( valarc(tempfils,temppere,indtemp) == vecseq[t] &&
	     indtemp<longarc &&
	     vecseq[t] != '~') {
       t++;
       indtemp++;
     }
     
     if /* on peut passer à l'arc suivant */
       (indtemp==longarc && valarc(tempfils,temppere,indtemp) == vecseq[t] && !tempfils.feuille) {
       
       temppere=tempfils;
       tempfils=tbranch[temppere.num].firstchild;  
       indtemp=1;
       t++;
       longarc=longueurarc(tempfils,temppere);  
     }
     else {
       pasfini = false;
       indtemp--;
       h.change(temppere.num,indtemp,tempfils);
     } /* pas beau du tout */
   }
  }    
}
/**********************************************************************/
/*fastfind , le prefixe t à scanner est dejà dans l'arbre, on part de nv*/             
inline void slligen::rescan (noeudvirt & h,const GRANDENT  nv,const occurrence  t) {

  NOEUILLE temp=tbranch[nv].firstchild;
  NOEUILLE htemp;
  htemp.num=nv;
  htemp.feuille=false;
  NOEUILLE freregtemp;
  freregtemp.num=0;
  occurrence tempocc=t;
 
  bool pasfini=true;
  if(tempocc.longueur()<=0) { 
    h.change(nv,0,temp,freregtemp);
  }
  else {
    while (pasfini) {

      /* tt que on n'a pas trouve un bon emplacement ds la liste des freres */  
      while(((freresuiv(temp).num) != RIEN)
	    && (valarc(temp,htemp,1) != vecseq[tempocc.debut])) {
	
	freregtemp=temp;
	temp=freresuiv(temp);
	//cout<<endl<<temp.num<<temp.feuille<<endl;
      };
      if (valarc(temp,htemp,1)!=vecseq[tempocc.debut]) {
	pasfini=false;
	h.change(htemp.num,0,temp,freregtemp);
      }
      else {
	if (longueurarc(temp,htemp)<tempocc.longueur() && !temp.feuille) {
	  tempocc.debut=tempocc.debut+longueurarc(temp,htemp);
	  htemp=temp;
	  temp=tbranch[htemp.num].firstchild;
	  freregtemp.num=0;
	}
	else {
	  pasfini=false;
	  if (longueurarc(temp,htemp)==tempocc.longueur()) {
	    htemp.num=0;
	    h.change(temp.num,0,htemp,freregtemp);
	  }
	  else {
	    h.change(htemp.num,tempocc.longueur(),temp,freregtemp);
	  }
	}
      }
    }
  }  
}
/**********************************************************************/
void slligen::construction_arbre() {
  
  TABSEQ sequActuelle(0,0);
  tabseq.push_back(sequActuelle);//commence a 1
  GRANDENT debut=1;
  GRANDENT fin=1;

  GRANDENT noeud_temp=RIEN;
  occurrence ch;
  NOEUILLE temp;
  temp.num=RIEN;
  noeudvirt headloc(racine,0,temp,temp) ; /* endroit ou inserer */
  
  GRANDENT i=1; /* suffixe courant à inserer */
  GRANDENT tailptr=1; /* reste à inserer */                   
  GRANDENT tbder=racine; /* dernière case remplie de tbranch */
  GRANDENT n= (GRANDENT)vecseq.size()-1; /*taille de la sequence */
  GRANDENT indn=1; /*num dernier noeud seq precedente */
 
  /*initialisation de l'arbre*/
  ELTBRANCH init;
  init.ini();
  tbranch.push_back(init); /*noeud 0 inutile*/     
  tfeuilles.push_back(temp);/*feuille o inutile*/
  init.headposition=1;
  init.depth=0;
  init.suffixlink=RIEN;
  init.branchbrother.num=RIEN;     
  init.firstchild.num=i;
  init.firstchild.feuille=true;
 
  tbranch.push_back(init);  /*arbre de depart:1 feuille et 1 noeud (racine) */
  tfeuilles.push_back(temp);
  sequence_courante=0;

 while ( i<n ) {
   i++; 
   if (vecseq[i]=='~') {// cas fin  sequence ->seq suiv
  
     /*stockage des debuts et longueurs
       des sequences dans tabseq*/
     fin = i;
     sequActuelle.setLongueur(fin-debut);
     sequActuelle.setPosition(debut);
     debut = fin+1;
     tabseq.push_back(sequActuelle);
     /*******************************/


     sequence_courante++;
     indn=tbder;                     
     headloc.num=racine;
     headloc.ind=0;
     
    /*i++; inserer les stops? */
    
   }
  
   tailptr=i;/*on passe au suffixe suivant */
   
   if (headloc.num == racine && headloc.reel()) {
     // cout<<endl<<"scan1";
     scanprefix(headloc,tailptr);
   }                                      
   else { 
     if (headloc.reel()) {/* noeud reel */
       headloc.change(tbranch[headloc.num].suffixlink,0,temp);
       tailptr=tailptr+tbranch[headloc.num].depth; 
       // cout<<endl<<"scan2";
       scanprefix(headloc,tailptr);
     }
   
     else {/*noeud virtuel */
       virtoreel(headloc,noeud_temp,ch);             
       /*headloc decompose*/
       if (noeud_temp == racine) {
	 ch.debut++; /*on se passe du premier car */
	 //cout<<endl<<"rescan1";
	 rescan(headloc,racine,ch);
       }
       else {
	 noeud_temp=tbranch[headloc.num].suffixlink;
	 //cout<<endl<<"rescan2";
	 rescan(headloc,noeud_temp,ch);
       };
       if (headloc.reel()) {
       // ATTENTION liens suff entre sequences !?!?!?!?!
	 if(tbder>indn) {
	   tbranch[tbder].suffixlink=headloc.num;
	 };  /*maj lien suffixe */
       
	 tailptr=tailptr+tbranch[headloc.num].depth;
	 //cout<<endl<<"scan3";
	 scanprefix(headloc,tailptr);
       }
       else {
	 tailptr=tailptr+tbranch[headloc.num].depth+headloc.ind;
	 if(tbder>indn) {
	   tbranch[tbder].suffixlink=tbder+1;
	 }
       } /*maj lien suffixe*/
     }
   }
   
   if (headloc.reel()) {
     //cout<<endl<<"insertleaf"<<headloc.num;
     insertleaf(headloc,tailptr,i);
   }
   else {
     tbder++;                   
     // cout<<endl<<"insertbranch"<<headloc.num<<" " <<tailptr<<" "<<i<< " "<<tbder;
     insertbranch(headloc,tailptr,i,tbder);
   }
   //gadget
//   if ((n/20) != 0) {
 //    if (i % (n/20)==0) {
  //     cout <<(i*100)/n +1<<" % "<<endl;
  //   }      
//   }
 }  


 
 //cout<<endl<<"taille de tbranch:"<<tbranch.size();
 //cout<<endl<<"taille de tfeuilles:"<<tfeuilles.size();
 //cout<<endl<<"taille de vecseq:"<<vecseq.size();
 //cout<<endl<<"nombre de sous-sequences: "<<sequence_courante;
}            


/**********************************************************************/
// constructeur 

//slligen::slligen(const char* nfich) {
slligen::slligen() {}

void slligen::init(char* entree) {

  //char c;
  /*cout << "fichier entre:" << nfich << endl;  
  
  ifstream entree (nfich,ios::in); 
  
  if (!entree) {
    cout << " ouverture impossible \n"; exit(1) ;
  } //à changer (pas planter)
  
  cout << "Chargement du fichier..." << endl;
  vecseq.push_back(c); //car 0 inutilise
  */
  /*
  while ( entree >> c ) {
    vecseq.push_back(c);
  }
  */

  vecseq.push_back('0');
  for (unsigned int z = 0; z < strlen(entree); z++) {
     vecseq.push_back(entree[z]);
  }


  vecseq.push_back('~'); /*marqueur de fin*/
  // vecfichseq.push_back(vecseq.size()-1);  
 // entree.close();
 //cout << "fichier entre" << endl << "Construction de l'arbre..." << endl;

  try {
    tfeuilles.reserve(vecseq.size()+1) ;
  } catch (...) {
    cout  << "\n" << " depassement tfeuilles" << endl ;
  }
  
  
 // cout << "Taille max de tbranch:";    
 // cout << tbranch.max_size() << endl;
  
  try {  
    tbranch.reserve((unsigned int)((0.8)*(vecseq.size()+1))); /* 0.62 en moyenne */
  } catch (...) { 
    cout  << "\n" << " depassement tbranch" << endl;
  }

  construction_arbre();
  
 // cout << endl << "arbre construit" << endl;
  /*cout<<'\007';   bip*/
  
  tbranch.resize(tbranch.size()); /*????????*/
 
  
}
/**********************************************************************/
// recursif ! AJOUTE dans res les indices des feuilles sous le noeuille n         
GRANDENT slligen::nbfeuilles(const NOEUILLE n,vector<GRANDENT> & indres)
{
  if (n.feuille) {//cout<<endl<<n.num<<"!"<<indres.size()<<endl;
    indres.push_back(n.num);
    return 1;}
  else {GRANDENT nb=0;
  NOEUILLE tn=tbranch[n.num].firstchild;
  NOEUILLE tnf=tn;
  while (tnf.num!=RIEN) {tn=tnf;tnf=freresuiv(tn);nb=nb+nbfeuilles(tn,indres);}
  return nb;}
}
/**********************************************************************/
GRANDENT slligen::facteur_present(seq fac_rech, vector<GRANDENT> & res)
{GRANDENT t=1;
 GRANDENT nbocc=0;
 NOEUILLE htemp; htemp.num=racine; htemp.feuille=false;
 NOEUILLE temp=tbranch[htemp.num].firstchild;
 GRANDENT indtemp=1;
 GRANDENT longarc=longueurarc(temp,htemp);
 bool pasfini=true;
 bool resb=false;
 
 while (pasfini)
   {
     while(((freresuiv(temp).num) != RIEN) && (valarc(temp,htemp,1) != fac_rech.tab[t]))
       /* tt que on n'a pas trouve un bon emplacement ds la liste des freres */    
       {temp=freresuiv(temp);};
     
     if (valarc(temp,htemp,1)!=fac_rech.tab[t])
       {pasfini=false;
       resb=false;
       temp=htemp;}
     else      
       {longarc=longueurarc(temp,htemp);
       while ( valarc(temp,htemp,indtemp) == fac_rech.tab[t] && indtemp<longarc && 
	       t< fac_rech.taille)
	 /* tt que l'arc courant correspond */ 
	 {indtemp++;t++;};
       
       if /* on peut passer à l'arc suivant */
	 (indtemp==longarc && valarc(temp,htemp,indtemp) == fac_rech.tab[t] 
	  && t<fac_rech.taille)
	 {htemp=temp; 
	 temp=tbranch[htemp.num].firstchild;  
	 indtemp=1;t++;
	 longarc=longueurarc(temp,htemp);
	 
	 }
       else {pasfini=false;
       if (t==fac_rech.taille && valarc(temp,htemp,indtemp) == fac_rech.tab[t]) 
	 {resb=true;}
       else {resb=false;};
       
       }
       }
   }//fin while(pasfini)
 
 if(!resb){return 0;} 
 else
   {nbocc=nbfeuilles(temp,res);
   // comptage du nombre de feuilles et mises dans res (ind dans fichier source !)
   cout <<endl<< "Nombre de repetitions:"<<nbocc<<"("<<(unsigned int)(res.size())<<")";
   //if (nbocc<12){;
   //          cout<<"--> ";
   //          for(list<GRANDENT>::iterator il=(res.begin());il!=(res.end());il++)
   //            {cout<<*il<<", ";}; };
   cout<<endl;          
   return nbocc;}
 
}


