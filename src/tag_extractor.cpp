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

#include <cctype>
#include <cstring>
#include "tag_extractor.h"
#include "defines.h"
#include "error.h"
#include "memorycheck.h"

using namespace std;

// ********************************************************************************************** //

extern memorycheck memCheck;

// ********************************************************************************************** //

tag_extractor::tag_extractor()
{
    runManParam  = NULL;
    aaParam      = NULL;
    spectrumData = NULL;
    popiGraph    = NULL;
    aPeptide     = NULL;
    suffTab      = NULL;
}

// ********************************************************************************************** //

tag_extractor::~tag_extractor()
{
    runManParam    =  NULL;
    aaParam        =  NULL;
    spectrumData   =  NULL;
    popiGraph      =  NULL;
    aPeptide       =  NULL;
    popiTags       =  NULL;
    if (suffTab   !=  NULL) {delete[] suffTab; suffTab = NULL;                                      memCheck.sufftree--;}
}

// ********************************************************************************************** //

void tag_extractor::init(runManagerParameters* rMP, aa* aaP, data* spectrumD, graphe *popiG, peptide* aPep, subseq* popiT)
{
    runManParam    = rMP;
    aaParam        = aaP;
    spectrumData   = spectrumD;
    popiGraph      = popiG;
    aPeptide       = aPep;
    popiTags       = popiT;
}

// ********************************************************************************************** //

void tag_extractor::getTheTags()
{
    //buildSuffTree(); CEH
    parseGraph();
    if (runManParam->r_CHECK) writeAllTags("TAGLIST0.txt");
    popiTags->removeSubSubSeqs(aPeptide->popiPep.dtbSeq);
    if (runManParam->r_CHECK) writeAllTags("TAGLIST1.txt");
}

// ********************************************************************************************** //

void tag_extractor::buildSuffTree()
{
    slligen tempoSuffTree;
    char str[256];

    //strcpy(aPeptide->popiPep.dtbSeq, "CTDCDYTTNK");  // POUR TESTER LE SUFFTAB ET L'EXTRACTION
    sprintf(str, "%s", aPeptide->popiPep.dtbSeq);
    tempoSuffTree.init(str);

    if (runManParam->r_CHECK) {
        char filename[256];
        sprintf(filename, "%sSUFFTREE.txt", runManParam->OUTPUT_DIR);
        File fp(filename, "a");
        fprintf(fp, "\n\n%s\n(", aPeptide->popiPep.dtbSeq);
        slligen::NOEUILLE root(1,0);
        tempoSuffTree.ecrit(fp, root);
        fp.Close();
    }

    // PASSAGE DE L'ARBRE AU TABLEAU (PLUS FACILE D'ACCES)
    sufftab::size = 5*(int)(strlen(aPeptide->popiPep.dtbSeq));
    buildSuffTab();

    tempoSuffTree.treeToTab(suffTab, aPeptide->popiPep.dtbSeq, aaParam, runManParam->FILE_ERROR_NAME);
    if (runManParam->r_CHECK) writeSuffTab();
}


// ********************************************************************************************** //


void tag_extractor::buildSuffTab()
{

    suffTab = new sufftab[sufftab::size]();                                                         memCheck.sufftree++;
    for (int i = 0; i < sufftab::size; i++) suffTab[i].init(aaParam);
}

// ********************************************************************************************** //

void tag_extractor::writeSuffTab()
{
    char filename[256];
    sprintf(filename, "%sSUFFTAB.txt", runManParam->OUTPUT_DIR);
    File fp(filename, "a");
    int n = 0;

    for (unsigned int i = 0; i < strlen(aPeptide->popiPep.dtbSeq); i++)
    {
        fprintf(fp, " %c ", aPeptide->popiPep.dtbSeq[i]);
    }
    fprintf(fp, "\n");
    for (unsigned int i = 0; i < strlen(aPeptide->popiPep.dtbSeq); i++)
    {
        fprintf(fp, " %i ", i);
    }

    fprintf(fp, "\n\n");

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "  i   state suffPos jArcPos  arcSeq        suffixSeq       succs\n");
    fprintf(fp, "----------------------------------------------------------------------");


    while (suffTab[n].longu != 1000) {

        fprintf(fp, "\n%5i   ", n);
        if (n == 0) {
            fprintf(fp, "%-5s", "R");
            fprintf(fp, "%47s", "");
            for (int i = 0; i < aaParam->aa1Nb+1; i++)
                if (suffTab[n].succ[i] != -1) fprintf(fp, "%i/%s  ", suffTab[n].succ[i], aaParam->aaTable[i].code);
        }
        else {
            if (suffTab[n].state == 0) {
                fprintf(fp, "%-5s", "N");
                fprintf(fp, "%3s   %3s      %-15s %-15s ", " -", " -", suffTab[n].subseq, "-");
                for (int i = 0; i < aaParam->aa1Nb+1; i++)
                    if (suffTab[n].succ[i] != -1) fprintf(fp, "%i/%s  ", suffTab[n].succ[i], aaParam->aaTable[i].code);
            }
            if (suffTab[n].state == 1) {
                fprintf(fp, "%-5s", "F");
                fprintf(fp, "%3i   %3i      %-15s ", suffTab[n].pos1, suffTab[n].pos2, suffTab[n].subseq);
                fprintf(fp, "%-25s ", aPeptide->popiPep.dtbSeq + suffTab[n].pos1);
            }
        }
        n++;
    }

    fprintf(fp, "\n--------------------------------------------------------------------\n\n");

    // saaParamture tab: elle permet de stocker l'arbre des suffixes sans économiser de la mémoire afin
    // de permettre un accès plus rapide aux suffixes, et de prendre en compte les double ponts.
    // chaque noeud/feuille a un indice unique qui permet de l'identifier et de l'accéder directement (indice du tableau)
    // l'indice du père de chaque noeud/feuille est mémorisé sour indPere
    // l'état permet de savoir s'il s'agit d'un noeud ou d'une feuille
    // les positions n'ont pas de significations pour les noeuds
    // Pour les feuilles, pos1 est la position du début du suffix dans la séquence mère (les noeuds précédents sont pris en compte)
    //                 et pos2 est la position de la séquence de la feuille dans la séquence mère (la séquence de la feuille
    //                         n'est pas le suffix, car il faut encore inclure les aa des noeuds précédents)
    // long est la longueur maximum du suffixe
    // subseq est la séquence de la feuille (et donc pas du suffixe entier lorsqu'il y a des noeuds précédents); c'est pourquoi
    // il faut toujours s'en tenir à pos1 et long pour connaître la séquence du suffixe
    // en enfin, succ est la liste des successeurs du noeud

    fp.Close();
}

// ********************************************************************************************** //

void tag_extractor::parseGraph()
{
    //fprintf(runManParam->FILEOUT, "\n\ntag_extractor::parseGraph parsing %s \n", aPeptide->popiPep.dtbSeq);fflush(stdout);
    ants liliRose;
    liliRose.init(runManParam, aaParam, aPeptide->popiPep.dtbSeq, aPeptide->popiPep.dtbPepMass);

    aPeptide->popiPep.dtbSeqSZ = (int)strlen(aPeptide->popiPep.dtbSeq);
    pepstr = &(aPeptide->popiPep.dtbSeq[0]);
    tab1ptr = aaParam->aaTable;
    tab2ptr = aaParam->aa2Table;
    vlst = popiGraph->vertexList;
    ovlst = popiGraph->or_vertexList;

    memset(aaFlag, 0, AASIZE);
    memset(aa2Flag, 0, AASIZE*AASIZE);
    int i, m, n;
    for ( i = 0; i < aPeptide->popiPep.dtbSeqSZ; i++ )
    {
       m = pepstr[i]-'A';
       aaFlag[m]++;
       if ( i > 0 ) aa2Flag[n][m]++;
       n = m;
    }

    for (int n = 0, vertexNb=popiGraph->get_vertexNb(); n<vertexNb; n++)
    {
        liliRose.startVertex = n;
        liliRose.iPlace      = n;
        explore(liliRose, 0, 0, 0, false);     // isuffplace, succ, moveTest
    }
    
}

// ********************************************************************************************** //


void tag_extractor::explore(ants liliRose, int isuffplace, int ilength, int vertexIndex, bool moveTest) {

    int iSuffixPlace = isuffplace;
    int iSuffixLength = ilength;

    if (moveTest ) {
        moveLili(&liliRose, iSuffixPlace, iSuffixLength, vertexIndex);
        moveTest = false;
    }

    vertex* vrtmp = &(vlst[liliRose.iPlace]);

    int succIndex, succNb;

    SUCC1* succ1 = &(vrtmp->succ1List[0]);
    succNb = vrtmp->succ1Nb;
    for (succIndex = 0; succIndex < succNb; succIndex++, succ1++) 
    {
        if ( !aaFlag[succ1->iAAi] ) continue;

        iSuffixPlace = liliRose.iSuffPlace;
        iSuffixLength = liliRose.arcPos;

        if ( iSuffixPlace+iSuffixLength<aPeptide->popiPep.dtbSeqSZ && 
             match1(vrtmp, &iSuffixPlace, &iSuffixLength, succ1->iAA)) 
        {
            moveTest = true;
            explore(liliRose, iSuffixPlace, iSuffixLength, succIndex, moveTest);
        }

    }

    SUCC2* succ2 = &(vrtmp->succ2List[0]);
    succNb = vrtmp->succ2Nb;
    for ( succIndex = 0; succIndex < succNb; succIndex++, succ2++) 
    {
        if ( !aa2Flag[succ2->iAAi][succ2->iAAj] ) continue;

        iSuffixPlace = liliRose.iSuffPlace;
        iSuffixLength = liliRose.arcPos;

        if ( iSuffixPlace+iSuffixLength < aPeptide->popiPep.dtbSeqSZ &&
             match2(vrtmp, &iSuffixPlace, &iSuffixLength, succ2->iAA) ) 
        {
                moveTest = true;
                explore(liliRose, iSuffixPlace, iSuffixLength, succIndex+vrtmp->succ1Nb, moveTest);
        }
    }

    // KEEP ?
    if (!moveTest ) {      // +1 car il manque encore le dernier vertex
        // test est false si on est au bout du tag
        if (liliRose.runnedVertexNb+1 >= popiTags->minLenght) {
            keepTag(liliRose);
        }
    }
}


// ********************************************************************************************** //

void tag_extractor::moveLili(ants* liliRose, int isuffplace, int length, int succIndex) {

    vertex* vrtmp = &(vlst[liliRose->iPlace]);

    liliRose->iSuffPlace = isuffplace;
    liliRose->arcPos     = length;

    char  code[3];
    if (succIndex < vrtmp->succ1Nb)                      // si c'est un 1 aa pont
        strcpy(code, aaParam->aaTable[vrtmp->getIAA(succIndex)].code);
    else {
        strcpy(code, aaParam->aa2Table[vrtmp->getIAA(succIndex)].code);
        code[0] = tolower(code[0]);
        code[1] = tolower(code[1]);
    }
    strcat(liliRose->parsedSequence, code);
    liliRose->parsedSequenceLenght += ((int)strlen(code));

    liliRose->runnedVertex[liliRose->runnedVertexNb].iVertex = vrtmp->or_indice;
    liliRose->runnedVertex[liliRose->runnedVertexNb].iEdge   = succIndex;
    liliRose->runnedVertexNb++;

    liliRose->iPlace = vrtmp->getSucc(succIndex);
//    vrtmp = &(vlst[liliRose->iPlace]);
//    for (int m = 0, mergedNb = vrtmp->mergedNb; m < mergedNb; m++) {
//        if (vrtmp->iPeak[m] != -1)
//            liliRose->tabooTab[vrtmp->iPeak[m]] = false; 	NB: this line causes a bug in popitam as the tabooTab has a length fixed to 200 
//															and some spectra have more than this number of peaks!!
//    }
}

//---------------------------------------------------------------

void tag_extractor::keepTag(ants liliRose) {

    popiTags->initSubSeqTab(popiTags->subSeqNb, spectrumData->get_peakNb(), popiGraph->orVertexNb);


    // RECUPERE LE INFOS SUR LES PICS ET HYPOTHESES IONIQUES
    int i_orVertex;
    int peakIndice;

    SUBSEQ* sstptr = &(popiTags->subSeqTab[popiTags->subSeqNb]);

    for (int r = 0; r < liliRose.runnedVertexNb; r++) {
        i_orVertex = liliRose.runnedVertex[r].iVertex;  // iVertex est l'indice dans or_vertexList
        sstptr->or_nL[i_orVertex] = true;
        sstptr->or_iNodesParsed[sstptr->iNodeNb] = i_orVertex;
        sstptr->iNodeNb++;
        if (sstptr->iNodeNb >= MAX_LENGHT) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function tag_extractor::keepTag()");
        for (int m = 0; m < ovlst[i_orVertex].mergedNb; m++) {
            peakIndice = ovlst[i_orVertex].iPeak[m];
            if (peakIndice != -1) sstptr->or_pL[peakIndice]++;
        }
    }

    // RECUPERE LES DERNIERES INFOS
    i_orVertex = vlst[liliRose.iPlace].or_indice;  // attention ici, car la fourmi garde l'indice du noeud de
    // la vertexList et il faut mémoriser l'indice correspondant dans la or_vertexList

    sstptr->or_nL[i_orVertex] = true;
    sstptr->or_iNodesParsed[sstptr->iNodeNb] = i_orVertex;
    sstptr->iNodeNb++;
    if (sstptr->iNodeNb >= MAX_LENGHT) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function tag_extractor::keepTag()");

    for (int m = 0; m < ovlst[i_orVertex].mergedNb; m++) {
        peakIndice = ovlst[i_orVertex].iPeak[m];
        if (peakIndice != -1) sstptr->or_pL[peakIndice]++;
    }

    // RECUPERE LA MASSE BEGIN ET LA MASSE END
    sstptr->massBegin       = ovlst[liliRose.runnedVertex[0].iVertex].mean_bMass;  // RECUPERE LA MASSE END
    sstptr->massEnd         = ovlst[vlst[liliRose.iPlace].or_indice].mean_bMass;
    sstptr->lenght          = liliRose.parsedSequenceLenght;
    strcpy(sstptr->parsedSeq, liliRose.parsedSequence);

    //Store current tag occurence
    sstptr->posBegin = liliRose.iSuffPlace;//suffTab[liliRose.iSuffPlace].pos1;
    popiTags->subSeqNb++;
    if (popiTags->subSeqNb >= MAX_SUBSEQ_NB) {
        fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "please increase MAX_SUBSEQ_NB in defines.h or reduce COVBIN in popParam.txt or increase MIN_TAG_LENGHT in subseq.cc"); exit(1);
    }

    //search other occurences of the same tag in the dtb sequence
    char* tagSeq = new char[liliRose.parsedSequenceLenght+1];                           memCheck.tagextractor++;
    memset( tagSeq, '\0', liliRose.parsedSequenceLenght+1);
    strncpy(tagSeq, aPeptide->popiPep.dtbSeq + liliRose.iSuffPlace, liliRose.parsedSequenceLenght);

    char* currentPos = strstr(aPeptide->popiPep.dtbSeq, tagSeq); //cannot use liliRose.parsedSequence as it may contain lower case characters
    while( (currentPos=strstr(currentPos+1, tagSeq)) ) { //current tag will always be the first occurence in the sequence, so duplicated tags may come next

        popiTags->initSubSeqTab(popiTags->subSeqNb, spectrumData->get_peakNb(), popiGraph->orVertexNb);
        popiTags->copySubSeq(popiTags->subSeqNb, popiTags->subSeqNb-1, spectrumData->get_peakNb(), popiGraph->orVertexNb);
        popiTags->subSeqTab[popiTags->subSeqNb].posBegin = (int)(strlen(aPeptide->popiPep.dtbSeq) - strlen(currentPos));
        popiTags->subSeqNb++;
        if (popiTags->subSeqNb >= MAX_SUBSEQ_NB) {
            fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "please increase MAX_SUBSEQ_NB in defines.h or reduce COVBIN in popParam.txt or increase MIN_TAG_LENGHT in subseq.cc"); exit(1);
        }
    }
    delete tagSeq; tagSeq = NULL;                                                         memCheck.tagextractor--;
    // QUAND LE TABOOTAB EST-IL REMIS SUR TRUE??? --> LORSQUE QU'ON REMONTE DANS LA RECURSIVITE
}


bool tag_extractor::match1(vertex* vrtmp, int* iSuffPlace, int* iLength, int iAAtmp) {

    bool rtn = false;

    static char tag[100];   // LL: double-check if 100 is enough

    const char tabtmp = tab1ptr[tab1ptr[iAAtmp].indice].code[0];

    if(pepstr[*iSuffPlace+*iLength]==tabtmp) {
        (*iLength)++;
        rtn = true;
    }
    else {
        memcpy(tag, pepstr+(*iSuffPlace), *iLength);
        tag[*iLength] = tabtmp;
        tag[*iLength+1] = '\0';

        char* pos;
        if( (pos=strstr(pepstr+(*iSuffPlace), tag)) ) {
            (*iSuffPlace) = (int)(pos-pepstr);
            (*iLength)++;
            rtn = true;
        }
    }
    return rtn;
}


bool tag_extractor::match2(vertex* vrtmp, int* iSuffPlace, int* iLength, int iAAtmp) {

    bool rtn = false;

    static char tag[100];   // LL: double-check if 100 is enough

    // récupère les indices des acides aminés du double pont du spectrum graph
    const char tabtmp1 = tab1ptr[tab2ptr[iAAtmp].indice1].code[0];
    const char tabtmp2 = tab1ptr[tab2ptr[iAAtmp].indice2].code[0];
    if( (pepstr+*iSuffPlace)[*iLength]==tabtmp1 &&
            ((*iLength) +2)<((int)strlen(pepstr + *iSuffPlace)) &&
            ((pepstr+*iSuffPlace)[(*iLength) +1])==tabtmp2) {
        (*iLength)+=2;
        rtn = true;
    }
    else
    {
        memcpy(tag, pepstr+(*iSuffPlace), *iLength);
        tag[*iLength] = tabtmp1;
        tag[*iLength+1] = tabtmp2;
        tag[*iLength+2] = '\0';

        char* pos=NULL;
        if( (pos=strstr(pepstr+(*iSuffPlace), tag)) ) {
            (*iSuffPlace) = (int)(pos-pepstr);
            (*iLength)+=2;
            rtn = true;
        }
    }

    return rtn;
}

// ********************************************************************************************** //

void tag_extractor::writeAllTags(char* name)
{
    char filename[256];
    sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, name);
    File fp(filename, "a");

    popiTags->write(aPeptide->popiPep.dtbSeq, fp, spectrumData->get_peakNb());
}

// ********************************************************************************************** //
