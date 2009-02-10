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
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib> // system()
#include <cctype>  // tolower

#include "defines.h"
#include "error.h"
#include "utils.h"
#include "ion.h"
#include "data.h"
#include "graphe.h"
#include "vertex.h"
#include "memorycheck.h"
#include "compare.h"

using namespace std;

// ********************************************************************************************** //

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck memCheck;

// ********************************************************************************************** //

struct TEMPOVERTEX {
    int     iV;
    double  pr;
    double  in;
};

// ********************************************************************************************** //

// COMPARISON ROUTINE FOR QSORT()

int compar_vertices_mean_bMass(const void *a, const void *b) {
    if (((vertex*)a)->mean_bMass < ((vertex*)b)->mean_bMass) return -1;
    if (((vertex*)a)->mean_bMass > ((vertex*)b)->mean_bMass) return 1;
    return 0;
}

int compar_tempoVertex_by_p(const void *a, const void *b) {
    if (((TEMPOVERTEX*)a)->pr > ((TEMPOVERTEX*)b)->pr) return -1;
    if (((TEMPOVERTEX*)a)->pr < ((TEMPOVERTEX*)b)->pr) return 1;
    return 0;
}

int compar_tempoVertex_by_int(const void *a, const void *b) {
    if (((TEMPOVERTEX*)a)->in > ((TEMPOVERTEX*)b)->in) return -1;
    if (((TEMPOVERTEX*)a)->in < ((TEMPOVERTEX*)b)->in) return 1;
    return 0;
}


// ********************************************************************************************** //


graphe::graphe()
{

    runManParam      = NULL;
    aaParam          = NULL;
    ionParam         = NULL;

    spectrumData     = NULL;

    or_vertexList = NULL;
    vertexList    = NULL;

    orVertexNb    = 0;
    vertexNb      = 0;
    wantedNodeNb  = 0;
    minNodeNb     = 0;

    IedgeNb       = 0;
    IIedgeNb      = 0;
    or_IedgeNb    = 0;
    or_IIedgeNb   = 0;
    intSom        = 0;
}

// ********************************************************************************************** //

void graphe::init(runManagerParameters* rMP, aa* aaP, ion* ionP, data* spectrumD) {

    runManParam        = rMP;
    aaParam            = aaP;
    ionParam           = ionP;
    spectrumData       = spectrumD;

    wantedNodeNb       = spectrumData->get_binsize() * runManParam->COVBIN + 2;

    IedgeNb            = 0;
    IIedgeNb           = 0;
    or_IedgeNb         = 0;
    or_IedgeNb         = 0;

    or_vertexList      = NULL;
    vertexList         = NULL;

    buildGraph();

    //  connect(0); //PAS BESOIN
    connect(1);

    if (runManParam->r_CHECK)
    {
        char filename[256] = "";
        sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "GRAPH0.txt");
        File fp;
        fp.Open(filename, "w");
        write(fp, 0);
        fp.Close();

        sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "GRAPH1.txt");
        fp.Open(filename, "w");
        write(fp, 1);
        fp.Close();

        sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "GRAPHDOT.dot");
        fp.Open(filename, "w");
        write_dotGraphSimple(fp);
        fp.Close();
    }
}

// ********************************************************************************************** //

graphe::~graphe() {
    if (vertexList != NULL)     delete[] vertexList;    vertexList = NULL;                                            memCheck.graph--;
    if (or_vertexList  != NULL) delete[] or_vertexList; or_vertexList = NULL;                                         memCheck.graph--;
}

// ********************************************************************************************** //

void graphe::buildGraph()
{

    build_vertice();
    qsort(vertexList, vertexNb, sizeof(vertex), compar_vertices_mean_bMass);
    // CONTROL QUE LA SEQUENCE VIDE EST BIEN LE PREMIER NOEUD (= NOEUD ARTIFICIEL)
    // ET RECIPROQUEMENT POUR LA SEQUENCE PLEINE
    if (vertexList[0].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that takes the place of the first node of the graph; this is not good");
    if (vertexList[vertexNb-1].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that take the place of the last node of the graph; this is not good");


    merge();
    qsort(or_vertexList, orVertexNb, sizeof(vertex), compar_vertices_mean_bMass);

    // CONTROL QUE LA SEQUENCE VIDE EST BIEN LE PREMIER NOEUD (= NOEUD ARTIFICIEL)
    // ET RECIPROQUEMENT POUR LA SEQUENCE PLEINE
    if (or_vertexList[0].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that takes the place of the first node of the graph; this is not good");
    if (or_vertexList[vertexNb-1].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that take the place of the last node of the graph; this is not good");

    // ATTENTION, maintenant les noeud sont dans or_vertexList!!!

    // parcours le graphe et pick les noeuds à plus grande prob. ionique jusqu'à
    // ce que wantedNodeNb soit atteint
    pickNodes();
    // ATTENTION, maintenant les noeuds sont à nouveau dans vertexList

    // il faut refaire un qsort par les mean_bMasses, car le merging
    // a pu en déplacer certaines
    qsort(vertexList, vertexNb, sizeof(vertex), compar_vertices_mean_bMass);
    if (vertexList[0].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that takes the place of the first node of the graph; this is not good");
    if (vertexList[vertexNb-1].iHypo[0] != -1)
        fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "The spectrum has an peak that take the place of the last node of the graph; this is not good");

    // calcul la somme de toutes les intensités des pics inclus dans le small graphe (cette
    // valeur sera utilisée lors du scoring (sous-score des intensités)
    intSom = 0;
    // (je ne prends pas en compte les pseudonoeuds)
    for (int i = 1; i < vertexNb-1; i++) {
        for (int m = 0; m < vertexList[i].mergedNb; m++) {
            intSom += vertexList[i].peakInt[m];
        }
    }

}

// ********************************************************************************************** //

void graphe::build_vertice() {
    vertexList = new vertex[spectrumData->get_peakNb()*ionParam->get_totIonNb()+2]();               memCheck.graph++;

    // ------------- THE FIRST VERTEX ------------------

    vertexList[0].mergedNb    =  1;
    vertexList[0].mean_bMass  = aaParam->H_mass;
    vertexList[0].bMass[0]    = aaParam->H_mass;
    vertexList[0].iHypo[0]    = -1;
    vertexList[0].iPeak[0]    = -1;
    vertexList[0].peakMass[0] = -1;
    vertexList[0].peakInt[0]  = -1;
    vertexList[0].peakBin[0]  = -1;

    // ------------- ALL POSSIBLE OTHER VERTICES ------------------

    int       c    =   1;  // compteur indice des noeuds
    double    PM   =   spectrumData->get_parentMassM();

    // peaks
    double    peakMass, peakIntensity;
    int       peakBin;
    bool      CHARGE1, CHARGE2, CHARGE3;

    // ions
    double    ionDelta;
    int       ionCharge;
    char      term;

    for (int j = 0; j < ionParam->get_totIonNb(); j++) {                                              // pour tous les ions

        ionDelta  = ionParam->get_delta(j);
        ionCharge = ionParam->get_charge(j);
        term      = ionParam->get_term(j);

        for (int i = 0; i < spectrumData->get_peakNb(); i++) {

            peakMass       = spectrumData->get_peak_i_mass(i);
            peakIntensity  = spectrumData->get_peak_i_intensity(i);
            peakBin        = spectrumData->get_peak_i_noBin(i);
            CHARGE1        = spectrumData->get_peak_i_CHARGE1(i);
            CHARGE2        = spectrumData->get_peak_i_CHARGE2(i);
            CHARGE3        = spectrumData->get_peak_i_CHARGE3(i);

            if ((ionCharge == 2) && (CHARGE2 == false)) continue;
            if ((ionCharge == 3) && (CHARGE3 == false)) continue;
            if (get_bMass(PM, peakMass, (double)ionCharge, ionDelta, term) > PM - aaParam->H_mass - aaParam->O_mass) continue;
            if (get_bMass(PM, peakMass, (double)ionCharge, ionDelta, term) < aaParam->H_mass)                        continue;

            vertexList[c].mergedNb    = 1;
            vertexList[c].mean_bMass  = get_bMass(PM, peakMass, (double)ionCharge, ionDelta, term);
            vertexList[c].bMass[0]    = get_bMass(PM, peakMass, (double)ionCharge, ionDelta, term);
            vertexList[c].iPeak[0]    = i;
            vertexList[c].peakMass[0] = peakMass;
            vertexList[c].peakInt[0]  = peakIntensity;
            vertexList[c].peakBin[0]  = peakBin;
            vertexList[c].iHypo[0]    = j;

            c++;
        }
    }

    // ------------- THE LAST VERTEX ------------------

    vertexList[c].mergedNb    = 1;
    vertexList[c].mean_bMass  = PM - aaParam->H_mass - aaParam->O_mass;
    vertexList[c].bMass[0]    = PM - aaParam->H_mass - aaParam->O_mass;
    vertexList[c].iHypo[0]    = -1;
    vertexList[c].iPeak[0]    = -1;
    vertexList[c].peakMass[0] = -1;
    vertexList[c].peakInt[0]  = -1;
    vertexList[c].peakBin[0]  = -1;

    vertexNb = c+1;
    orVertexNb = vertexNb;
}

// ********************************************************************************************** //

void graphe::merge() {
    // greedy
    int i,j;

    while (true) {
        if (!findClosestNodes(&i, &j)) break;
        if ((i >= vertexNb) | (j >= vertexNb)) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function graphe.:merge();");
        mergeNodes(i, j);
    }

    // traite le cas du premier et dernier noeuds
    // normalement, le premier noeud ne devrait pas avoir d'autres noeuds très près
    // retire les noeuds qui ont été mergés
    cleanGraph();
    // place les indices dans orVertexList (ils seront utilisés par la fonction
    // dichomSearch
    for (int i = 0; i< orVertexNb; i++) or_vertexList[i].or_indice = i;
    // ATTENTION, maintenant les noeuds sont dans or_vertexList !!!!!

}

// ********************************************************************************************** //

bool graphe::findClosestNodes(int*a, int*b) {

    // les noeuds sont déjà triés selon leur masse

    double min = 1000;
    double breakMin=0.0;
    double setError=0.0;

    for (int i = 0; i < vertexNb-1; i++) {
        if (isDoubleEqual(vertexList[i].mean_bMass, 1000000.0)) continue; // signifie qu'il a déjà été mergé
        for (int j = i+1; j < vertexNb; j++) {
            if (isDoubleEqual(vertexList[j].mean_bMass, 1000000.0)) continue;  // signifie qu'il a déjà été mergé
            breakMin = 1000;

            // ATTENTION, il faut comparer toutes les bMasses entre elles et non les mean_bMass!!!
            for (int m = 0; m < vertexList[j].mergedNb; m++) {
                for (int n = 0; n < vertexList[i].mergedNb; n++) {
                    if (fabs(vertexList[j].bMass[m] - vertexList[i].bMass[n]) < min) {
                        // je fais la valeur absolue, même si les noeuds sont triés, car je ne suis pas certaine
                        // que les bMasses soient également triés après que le processus de merging ait commencé

                        // on ne va updater le minimum que si on est en dessous de l'erreur
                        // maximale autorisée pour le merging; et cette erreur dépend des
                        // terminaux des hypothèses ioniques utilisées pour bMass[m] et bMass[n]
                        // si l'un des noeuds à merger est un terminal, autorise l'erreur la plus grande (à cause des
                        // problèmes de calibration)
                        if ((i == 0) | (j == vertexNb-1)) setError = runManParam->FRAGMENT_ERROR2;
                        else {
                            if (ionParam->get_term(vertexList[j].iHypo[n]) == ionParam->get_term(vertexList[j].iHypo[m])) setError = runManParam->FRAGMENT_ERROR1;
                            else                                                                                          setError = runManParam->FRAGMENT_ERROR2;
                        }
                        if (vertexList[j].bMass[m] - vertexList[i].bMass[n] < setError) {
                            min = fabs(vertexList[j].bMass[m] - vertexList[i].bMass[n]);
                            (*a) = i;
                            (*b) = j;
                        }
                        if (vertexList[j].bMass[m] - vertexList[i].bMass[n] < breakMin) {
                            breakMin = fabs(vertexList[j].bMass[m] - vertexList[i].bMass[n]);
                        }
                    }
                }
            }
            // BREAK?

            if (breakMin > runManParam->FRAGMENT_ERROR2) break;
        }
    }

    // min représente la distance minimale
    if (min < setError) return true;
    else                return false;
}

//---------------------------------------------------------------

void graphe::mergeNodes(int i, int j) {
    // met j dans i
    for (int m = 0; m < vertexList[j].mergedNb; m++) {
        vertexList[i].mean_bMass += vertexList[j].bMass[m];
        vertexList[i].bMass[vertexList[i].mergedNb]     = vertexList[j].bMass[m];
        vertexList[i].iHypo[vertexList[i].mergedNb]     = vertexList[j].iHypo[m];
        vertexList[i].iPeak[vertexList[i].mergedNb]     = vertexList[j].iPeak[m];
        vertexList[i].peakMass[vertexList[i].mergedNb]  = vertexList[j].peakMass[m];
        vertexList[i].peakInt[vertexList[i].mergedNb]   = vertexList[j].peakInt[m];
        vertexList[i].peakBin[vertexList[i].mergedNb]   = vertexList[j].peakBin[m];

        vertexList[i].mergedNb++;
        if (vertexList[i].mergedNb >= MAX_MERGED){
            fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "Please increase MAX_MERGED in defines.h");
        }

    }
    // recalcule la mean_bMass pour le noeud i
    vertexList[i].mean_bMass = 0;
    for (int m = 0; m < vertexList[i].mergedNb; m++) {
        vertexList[i].mean_bMass += vertexList[i].bMass[m];
    }
    vertexList[i].mean_bMass /= (double)vertexList[i].mergedNb;

    // et met le noeud j hors service
    vertexList[j].mean_bMass = 1000000;
}

// ********************************************************************************************** //

void graphe::cleanGraph() {
    // compte le nombre de noeuds valides
    int c = 0;
    for (int i = 0; i < vertexNb; i++) if (!isDoubleEqual(vertexList[i].mean_bMass, 1000000.0)) c++;
    orVertexNb    = c;
    or_vertexList = new vertex[c]();                                                                memCheck.graph++;

    // copie les noeuds valides dans or_vertexList
    c = 0;
    for (int i = 0; i < vertexNb; i++) {
        if (isDoubleEqual(vertexList[i].mean_bMass,  1000000.0)) continue;
        or_vertexList[c].mergedNb   = vertexList[i].mergedNb;
        or_vertexList[c].mean_bMass = vertexList[i].mean_bMass;

        for (int m = 0; m < vertexList[i].mergedNb; m++) {
            or_vertexList[c].bMass[m]     = vertexList[i].bMass[m];
            or_vertexList[c].iHypo[m]     = vertexList[i].iHypo[m];
            or_vertexList[c].iPeak[m]     = vertexList[i].iPeak[m];
            or_vertexList[c].peakMass[m]  = vertexList[i].peakMass[m];
            or_vertexList[c].peakInt[m]   = vertexList[i].peakInt[m];
            or_vertexList[c].peakBin[m]   = vertexList[i].peakBin[m];
        }
        c++;
    }

    vertexNb = c;
    delete[] vertexList; vertexList = NULL;                                                         memCheck.graph--;

    // clean aussi le premier et dernier noeud
    or_vertexList[0].mergedNb    = 1;
    or_vertexList[0].bMass[0]    = or_vertexList[0].mean_bMass;
    or_vertexList[0].iHypo[0]    = -1;
    or_vertexList[0].iPeak[0]    = -1;
    or_vertexList[0].peakMass[0] = -1;
    or_vertexList[0].peakInt[0]  = -1;
    or_vertexList[0].peakBin[0]  = -1;

    or_vertexList[vertexNb-1].mergedNb    = 1;
    or_vertexList[vertexNb-1].bMass[0]    = or_vertexList[vertexNb-1].mean_bMass;
    or_vertexList[vertexNb-1].iHypo[0]    = -1;
    or_vertexList[vertexNb-1].iPeak[0]    = -1;
    or_vertexList[vertexNb-1].peakMass[0] = -1;
    or_vertexList[vertexNb-1].peakInt[0]  = -1;
    or_vertexList[vertexNb-1].peakBin[0]  = -1;
}

// ********************************************************************************************** //

void graphe::pickNodes() {

    // commence par trier les graphe selon les probs maximum
    TEMPOVERTEX *tempoVertex;
    tempoVertex = new TEMPOVERTEX[vertexNb];                                                        memCheck.graph++;
    double maxP = -1000;
    double maxInt;  //pas besoin d'initialiser, car je prends automatiquement l'int de maxP
    tempoVertex[0].iV    = 0;
    tempoVertex[0].pr    = 100;
    tempoVertex[0].in    = 100;

    for (int i = 1; i < vertexNb-1; i++) {
        maxP = -1000;
        for (int m = 0; m < or_vertexList[i].mergedNb; m++) {
            if (ionParam->get_prob(or_vertexList[i].iHypo[m], or_vertexList[i].peakBin[m]) > maxP) {
                maxP = ionParam->get_prob(or_vertexList[i].iHypo[m], or_vertexList[i].peakBin[m]);
                maxInt = or_vertexList[i].peakInt[m];
            }
        }
        tempoVertex[i].iV = i;
        tempoVertex[i].pr = maxP;
        tempoVertex[i].in = maxInt;
    }

    tempoVertex[vertexNb-1].iV    = vertexNb-1;
    tempoVertex[vertexNb-1].pr    = 100;
    tempoVertex[vertexNb-1].in    = 100;

    // il faut trier selon la probabilité, puis pour chaque probabilité, selon l'intensité du pic
    qsort(tempoVertex, vertexNb, sizeof(TEMPOVERTEX), compar_tempoVertex_by_p);
    // trie encore une fois selon l'intensité (par classe de prob)
    int          n = 1;
    int          c = 1;
    double       prob = tempoVertex[0].pr;
    TEMPOVERTEX* p    = &tempoVertex[0];
    while (n < vertexNb) {

        if(!isDoubleEqual(tempoVertex[n].pr, prob)) {

            // trie
            p =  &tempoVertex[n-c];
            qsort(p, c, sizeof(TEMPOVERTEX), compar_tempoVertex_by_int);
            prob = tempoVertex[n].pr;
            c = 0;
        }
        else  {
            n++;
            c++;
        }

    }


    // CONSTITUTION DE LA LISTE DE NOEUDS FINALE
    vertexList = new vertex[wantedNodeNb];                                                          memCheck.graph++;

    c = 0;
    while (c < wantedNodeNb) {
        if (c == vertexNb) break;

        // pas besoin de copier les successeurs, car ils ne sont pas encore remplis
        vertexList[c].or_indice     = tempoVertex[c].iV;
        vertexList[c].mergedNb      = or_vertexList[tempoVertex[c].iV].mergedNb;
        vertexList[c].mean_bMass    =  or_vertexList[tempoVertex[c].iV].mean_bMass;
        for (int m = 0; m < or_vertexList[tempoVertex[c].iV].mergedNb; m++) {

            vertexList[c].bMass[m]    = or_vertexList[tempoVertex[c].iV].bMass[m];
            vertexList[c].iHypo[m]    = or_vertexList[tempoVertex[c].iV].iHypo[m];
            vertexList[c].iPeak[m]    = or_vertexList[tempoVertex[c].iV].iPeak[m];
            vertexList[c].peakMass[m] = or_vertexList[tempoVertex[c].iV].peakMass[m];
            vertexList[c].peakInt[m]  = or_vertexList[tempoVertex[c].iV].peakInt[m];
            vertexList[c].peakBin[m]  = or_vertexList[tempoVertex[c].iV].peakBin[m];
        }
        c++;
    }

    if (c < wantedNodeNb) {
        //  cout<<"WARNING !!! ---------- node number is less than wantedNodeNb (which is "<<wantedNodeNb<<" for this graph)"<<endl<<endl;
        vertexNb = c;
    }
    else {
        vertexNb = wantedNodeNb;
    }

    delete[] tempoVertex; tempoVertex = NULL;                                                       memCheck.graph--;
    // je garde or_vertexList à disposition pour les fonctions allonge_left et allonge_right
}

// ********************************************************************************************** //

void graphe::connect(int choice) {
    connectSucc1(choice);

    if (runManParam->EDGES_TYPE == 1) {
        connectSucc2(choice);
    }
}

// ********************************************************************************************** //

void graphe::connectSucc1(int choice) {

    vertex* vL=NULL;
    int     vN=0;
    int     countEdges  = 0;


    if (choice == 0) {vN = orVertexNb; vL = or_vertexList;}
    if (choice == 1) {vN = vertexNb;   vL = vertexList;}

    int   i,j,k;                 // i = v1; j = v2; k = aa;
    int   aaNb   = aaParam->aa1Nb;
    double diff=0.0;
    float setError=0.0;
    aa*   crotteAlors;
    crotteAlors = aaParam;

    for (i = 0; i < vN; i++)  {


        for (j = i+1; j < vN; j++) {

            // je suis entrain de comparer un noeud avec son suivant (dont la mean_bMass doit être plus grande)
            if (vL[j].mean_bMass < vL[i].mean_bMass) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function graph::connectSucc1");


            // il faut maintenant comparer toutes les bMasses des des noeuds pour savoir si on peut
            // les relier ensembles

            // LL: what's the purpose of these two loops?  only the last diff-value matters???
            for (int m = 0; m < vL[j].mergedNb; m++) {


                for (int n = 0; n < vL[i].mergedNb; n++) {


                    // comment sont les terminaux?
                    if ((vL[j].iHypo[m] == -1) | (vL[i].iHypo[n] == -1))
                        setError = runManParam->FRAGMENT_ERROR2;
                    else {
                        if (ionParam->get_term(vL[j].iHypo[m]) == ionParam->get_term(vL[i].iHypo[n]))
                            setError = runManParam->FRAGMENT_ERROR1;
                        else setError = runManParam->FRAGMENT_ERROR2;
                    }

                    diff = abs(vL[j].bMass[m] - vL[i].bMass[n]);
                }
            }



            if (diff - runManParam->FRAGMENT_ERROR2 > crotteAlors->aaTable[aaNb-1].mass) {
                break;
            }


            // recherche si un acide aminé peut correspondre à diff
            for (k = 0; k < aaNb; k++) {


                if(fabs(diff - crotteAlors->aaTable[k].mass) < setError) {
                    if (vL[i].succ1Nb >= MAX_SUCC1) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function graphe::connectSucc1(); please increase MAX_SUCC1 in defines.h");
                    vL[i].succ1List[vL[i].succ1Nb].iVertexSucc = j;
                    vL[i].succ1List[vL[i].succ1Nb].iAA = crotteAlors->aaTable[k].indice;
                    vL[i].succ1List[vL[i].succ1Nb].iAAi = crotteAlors->aaTable[crotteAlors->aaTable[k].indice].code[0] - 'A';
                    vL[i].succ1Nb ++;
                    countEdges ++;

                }

                if (crotteAlors->aaTable[k].mass > diff + runManParam->FRAGMENT_ERROR2) break;
            }
        }
    }
    if (choice == 0) or_IedgeNb = countEdges;
    if (choice == 1) IedgeNb    = countEdges;

}

// ********************************************************************************************** //

void graphe::connectSucc2(int choice) {

    vertex* vL=NULL;
    int     vN=0;
    int     countEdges  = 0;

    if (choice == 0) {vN = orVertexNb; vL = or_vertexList;}
    if (choice == 1) {vN = vertexNb;   vL = vertexList;}

    int   i,j,k;                 // i = v1; j = v2; k = aa;
    int   aaNb   = aaParam->aa2Nb;
    double diff = 0.0;
    float setError = 0.0;
    aa*   crotteAlors;
    crotteAlors = aaParam;

    for (i = 0; i < vN; i++)  {
        for (j = i+1; j < vN; j++) {

            // je suis entrain de comparer un noeud avec son suivant (dont la mean_bMass doit être plus grande)
            if (vL[j].mean_bMass < vL[i].mean_bMass) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function graph::connectSucc1");

            // il faut maintenant comparer toutes les bMasses des des noeuds pour savoir si on peut
            // les relier ensembles
            for (int m = 0; m < vL[j].mergedNb; m++) {
                for (int n = 0; n < vL[i].mergedNb; n++) {

                    // comment sont les terminaux?
                    if ((vL[j].iHypo[m] == -1) | (vL[i].iHypo[n] == -1))
                        setError = runManParam->FRAGMENT_ERROR2;
                    else {
                        if (ionParam->get_term(vL[j].iHypo[m]) == ionParam->get_term(vL[i].iHypo[n]))
                            setError = runManParam->FRAGMENT_ERROR1;
                        else setError = runManParam->FRAGMENT_ERROR2;
                    }

                    diff = abs(vL[j].bMass[m] - vL[i].bMass[n]);
                }
            }
            if (diff - runManParam->FRAGMENT_ERROR2 > aaParam->aa2Table[aaNb-1].mass) break;

            // recherche si un acide aminé peut correspondre à diff
            for (k = 0; k < aaNb; k++) {

                if(fabs(diff - crotteAlors->aa2Table[k].mass) < setError) {
                    if (!verifyEdges(vL, i, j, k)) {
                        if (vL[i].succ2Nb >= MAX_SUCC2) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function graphe::connectSucc2(); please increase MAX_SUCC2 in defines.h");
                        vL[i].succ2List[vL[i].succ2Nb].iVertexSucc = j;
                        vL[i].succ2List[vL[i].succ2Nb].iAA = crotteAlors->aa2Table[k].indice;
                        vL[i].succ2List[vL[i].succ2Nb].iAAi = crotteAlors->aa2Table[crotteAlors->aa2Table[k].indice].code[0] - 'A';
                        vL[i].succ2List[vL[i].succ2Nb].iAAj = crotteAlors->aa2Table[crotteAlors->aa2Table[k].indice].code[1] - 'A';
                        vL[i].succ2Nb++;
                        countEdges++;
                    }
                }

                if (crotteAlors->aa2Table[k].mass > diff + runManParam->FRAGMENT_ERROR2) break;

            }
        }
    }
    if (choice == 0) or_IIedgeNb = countEdges;
    if (choice == 1) IIedgeNb    = countEdges;
}


// ********************************************************************************************** //


bool graphe::verifyEdges(vertex* vL, int ii, int jj, int kk) {
    // dès qu'on trouve deux successeurs partant de ii et menant à jj, retourne TRUE, ce qui signifie :
    // deux edges existent déjà entre ii et jj, donc ne les relie pas avec un 2aa_edge;

    int iEl1 = aaParam->aa2Table[kk].indice1;
    int iEl2 = aaParam->aa2Table[kk].indice2;
    int succ_node1 = 0;
    int edge1      = 0;
    int edge2      = 0;

    for (int u = 0, nbSucc = vL[ii].succ1Nb ; u < nbSucc ; u++) {                           // pour tout successeur de ii
        succ_node1 = vL[ii].succ1List[u].iVertexSucc;

        for (int v = 0, nbSuccSucc = vL[succ_node1].succ1Nb; v < nbSuccSucc; v++) {                 // pour tout successeur de successeur de ii
            edge1 = vL[ii].succ1List[u].iAA;
            edge2 = vL[succ_node1].succ1List[v].iAA;

            if ((iEl1 == edge1) && (iEl2 == edge2))
                return TRUE;
        }
    }
    return FALSE;
}

// ********************************************************************************************** //

void graphe::write(File &fp, int choice) {

    vertex* vL = NULL;
    int     vN = 0;

    if (choice == 0) {vN = orVertexNb; vL = or_vertexList;}
    if (choice == 1) {vN = vertexNb;   vL = vertexList;}

    fprintf(fp, "GRAPH\n--------------------------------------\n");
    fprintf(fp, "BinNb                    : %i\n", runManParam->BIN_NB);
    fprintf(fp, "CovBin                   : %i\n", runManParam->COVBIN);
    fprintf(fp, "Small graph vertexNb     : %i\n", vertexNb);
    fprintf(fp, "Small graph simpleEdgeNb : %i\n", IedgeNb);
    fprintf(fp, "Small graph doubleEdgeNb : %i\n", IIedgeNb);
    fprintf(fp, "Big graph vertexNb       : %i\n", orVertexNb);
    fprintf(fp, "Big graph simpleEdgeNb   : %i\n", or_IedgeNb);
    fprintf(fp, "Big graph doubleEdgeNb   : %i\n", or_IIedgeNb);
    fprintf(fp, "Threshold1           : %f\n", runManParam->FRAGMENT_ERROR1);
    fprintf(fp, "Threshold2           : %f\n", runManParam->FRAGMENT_ERROR2);
    fprintf(fp, "\n--------------------------------------\n");

    fprintf(fp, "a) OR_INDICE\nb)MEAN_BMASS\nc)BMASS_MERGED;IONIC HYPO;OCC PROB;IPEAK;PEAK_MASS;BIN;\nd)SUCC1 and SUCC2\n\n");
    for (int i = 0; i < vN; i++) {
        fprintf(fp, "%-5i", i);
        fprintf(fp, "(%i)\n", vL[i].or_indice);

        fprintf(fp, "MEAN_BMASS = %.3f %.3f\n\n", vL[i].mean_bMass, spectrumData->peakList[vL[i].iPeak[0]].intensity);

        for (int m = 0; m < vL[i].mergedNb; m++) {
            fprintf(fp, "MERGED: ");
            fprintf(fp, "%10.3f %5.3f ", vL[i].bMass[m], spectrumData->peakList[vL[i].iPeak[m]].intensity);


            if (vL[i].iHypo[m] == -1)    fprintf(fp, "%-4s ", "/");
            else  {
                fprintf(fp, "%-4s  ",ionParam->get_name(vL[i].iHypo[m]));
                fprintf(fp, "%5.2f  ", ionParam->get_prob(vL[i].iHypo[m], vL[i].peakBin[m]));
            }

            // iPeak
            if (vL[i].iPeak[m] == -1)    fprintf(fp, "%-3s  ", "/");
            else                                 fprintf(fp, "%3i  ",   vL[i].iPeak[m]);

            // peakMass
            if (isDoubleEqual(vL[i].peakMass[m], -1)) fprintf(fp, "%-10s  ", "/");
            else                                 fprintf(fp, "%10.2f  ",vL[i].peakMass[m]);

            // peakBin
            if (vL[i].peakBin[m] == -1)  fprintf(fp, "%-3s\n", "/");
            else                                 fprintf(fp, "%3i\n",  vL[i].peakBin[m]);
        }
        fprintf(fp, "\n");

        fprintf(fp, "SUCC1NB   = %4i    ",vL[i].succ1Nb);
        for (int j = 0, nSucc1 = vL[i].succ1Nb ; j < nSucc1 ; j++) {
            fprintf(fp, "%2i/", vL[i].succ1List[j].iVertexSucc);
            fprintf(fp, "%s   ", aaParam->aaTable[vL[i].succ1List[j].iAA].code);
            if ((j+1)%10 == 0) fprintf(fp, "\n                   ");
        }
        fprintf(fp, "\n");
        fprintf(fp, "SUCC2NB   = %4i    ",vL[i].succ2Nb);
        for (int j = 0, nbSucc2 = vL[i].succ2Nb; j < nbSucc2; j++) {
            fprintf(fp, "%2i/", vL[i].succ2List[j].iVertexSucc);
            fprintf(fp, "%s  ", aaParam->aa2Table[vL[i].succ2List[j].iAA].code);
            if ((j+1)%10 == 0) fprintf(fp, "\n                    ");
        }
        fprintf(fp, "\n----------------------------------------------------\n");
    }
}

// ********************************************************************************************** //

void graphe::write_dotGraphSimple(File &fp) {


    // UNIQUEMENT SIMPLE EDGES

    // ECRITURE EN-TETE
    fprintf(fp, "digraph seqgraph { \n");
    // page A4 portrait
    // fprintf(fp,"size=\"7.5,15\";\n");
    // fprintf(fp,"orientation=\"portrait\";\n");
    // page A4 landscape
    fprintf(fp,"size=\"15,7.5\";\n");
    fprintf(fp,"orientation=\"landscape\";\n");

    fprintf(fp,"node [shape = ellipse];\n");
    fprintf(fp,"\n\n");

    // ECRITURE GRAPH
    for (int i = 0; i < vertexNb; i++) {
        for (int j = 0, nbSucc1 = vertexList[i].succ1Nb ; j < nbSucc1; j++) {

            // NOEUD
            fprintf(fp, "\"%.1f\\n", vertexList[i].mean_bMass);
            fprintf(fp,"\"\t ->\t");

            // PONTS
            fprintf(fp,"\"%.1f\\n", vertexList[vertexList[i].succ1List[j].iVertexSucc].mean_bMass);
            fprintf(fp, "\"\t[ label = \"");
            fprintf(fp, "%s\\n\"]\n", aaParam->aaTable[vertexList[i].succ1List[j].iAA].code);
        }
    }

    /*
    // COULEUR DES NOEUDS SELON LA TAILLE DE LA FAMILLE
    for (int i = 0; i < vertexNb; i++) {
      if (vertexList[i].succ1Nb > 0) {
        fprintf(fp, "\"%i\\n", vertexList[i].or_indice);
        fprintf(fp, "%.2f\\n", spectrumData->get_peak_i_mass(vertexList[i].iPeak));
        fprintf(fp,"%.4f\\n", vertexList[i].bMass); 
        
        if (vertexList[i].iHypo != -1)
          fprintf(fp,"%s\\n\"",ionParam->get_name(vertexList[i].iHypo));
        else fprintf(fp, "-\\n\"");
        if (vertexList[i].familySize > 1) 
    fprintf(fp, "\t[ color = cornflowerblue, fontcolor = cornflowerblue]\n");
        else 
    fprintf(fp, "\t[ color = black, fontcolor = black]\n");
      }
    }
    */

    fprintf(fp, "}");

    //system("dot -Tps ../out/dotGraph.dot -o ../out/dotGraph.ps");
    //system("gv ../out/dotGraph.ps");
}

// ********************************************************************************************** //

void graphe::write_dotGraphDouble(File &fp) {

    // ECRITURE EN-TETE
    fprintf(fp, "digraph seqgraph { \n");
    // page A4 portrait
    // fprintf(fp,"size=\"7.5,15\";\n");
    // fprintf(fp,"orientation=\"portrait\";\n");
    // page A4 landscape
    fprintf(fp,"size=\"15,7.5\";\n");
    fprintf(fp,"orientation=\"landscape\";\n");

    fprintf(fp,"node [shape = ellipse];\n");
    fprintf(fp,"\n\n");

    // ECRITURE GRAPH
    for (int i = 0; i < vertexNb; i++) {
        for (int j = 0, nbSucc1 = vertexList[i].succ1Nb ; j < nbSucc1; j++) {

            // NOEUD
            fprintf(fp, "\"%.0f\\n", vertexList[i].mean_bMass);
            fprintf(fp,"\"\t ->\t");

            // PONTS
            fprintf(fp,"\"%.0f\\n", vertexList[vertexList[i].succ1List[j].iVertexSucc].mean_bMass);
            fprintf(fp, "\"\t[ label = \"");
            fprintf(fp, "%s\\n\"]\n", aaParam->aaTable[vertexList[i].succ1List[j].iAA].code);
        }
        for (int j = 0, nbSucc2 = vertexList[i].succ2Nb ; j < nbSucc2; j++) {

            // NOEUD
            fprintf(fp, "\"%.0f\\n", vertexList[i].mean_bMass);
            fprintf(fp,"\"\t ->\t");

            // PONTS
            fprintf(fp,"\"%.0f\\n", vertexList[vertexList[i].succ2List[j].iVertexSucc].mean_bMass);
            fprintf(fp, "\"\t[ label = \"");
            fprintf(fp, "%s\\n\"]\n", aaParam->aa2Table[vertexList[i].succ2List[j].iAA].code);
        }
    }

    fprintf(fp, "}");

    //system("dot -Tps ../out/dotGraph.dot -o ../out/dotGraph.ps");
    //system("gv ../out/dotGraph.ps");
}


// ********************************************************************************************** //

void graphe::display(File &fp)
{
    fprintf(fp,"%-25s: %7i \n","NodeNb ",  vertexNb);
    fprintf(fp,"%-25s: %7i / %-7i\n", "EdgeNb (simple/double)", IedgeNb, IIedgeNb);
}

// ********************************************************************************************** //

void graphe::displayXML(File &fp)
{
    fprintf(fp,"      <nodeNb>%i</nodeNb>\n", vertexNb);
    fprintf(fp,"      <simpleEdgeNb>%i</simpleEdgeNb>\n", IedgeNb);
    fprintf(fp,"      <doubleEdgeNb>%i</doubleEdgeNb>\n", IIedgeNb);
}

// ********************************************************************************************** //
