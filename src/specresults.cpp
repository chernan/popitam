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

#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "element.h"
#include "memorycheck.h"
#include "params.h"
#include "specresults.h"
#include "graphe.h"
#include "data.h"
#include "defines.h"
#include "error.h"
#include "utils.h"
#include "subseq.h"

using namespace std;

// ********************************************************************************************** //

// GLOBAL VARIABLES DECLARED ELSEWHERE 

// ********************************************************************************************** //

extern memorycheck memCheck;

// ********************************************************************************************** //


specresults::specresults() 
{
	runManParam    = NULL;
	elementNb      = 0;
	rankNb         = 0;
	firstElement   = NULL;
	lastElement    = NULL;
	newElement     = NULL;
	currentElement = NULL;
	//  randPep        = NULL;
}


// ********************************************************************************************** //


specresults::~specresults() {

	runManParam    = NULL;
	elementNb      = 0;
	rankNb         = 0;

	newElement     = NULL;
	currentElement = NULL;

	currentElement = firstElement->following;

	while(currentElement != lastElement) {
		removeElement(currentElement);
		currentElement = firstElement->following;
	}

	// remove t�te et queue
	if (firstElement != NULL) {delete firstElement;  firstElement = NULL;                            memCheck.tagoResults--;}
	if (lastElement  != NULL) {delete lastElement;   lastElement  = NULL;                            memCheck.tagoResults--;}

	//  if (randPep      != NULL) {delete[] randPep;     randPep      = NULL;                            memCheck.tagoResults--;}
}

// ********************************************************************************************** //

void specresults::init(runManagerParameters *rMP, data* spectrumD) 
{  
	runManParam                = rMP;
	spectrumData               = spectrumD;
	elementNb                  = runManParam->RESULT_NB;
	rankNb                     = elementNb;
	currentElement             = NULL;
	firstElement               = new element();                                                       memCheck.tagoResults++;
	lastElement                = new element();                                                       memCheck.tagoResults++;
	firstElement->following    = lastElement;
	lastElement->preceding     = firstElement;
	firstElement->preceding    = NULL;
	lastElement->following     = NULL;
	currentElement = firstElement;
	for (int i = 0; i < elementNb; i++) {
		newElement = new element();                                                                      memCheck.tagoResults++;
		newElement->preceding    = NULL;
		newElement->following    = firstElement;
		firstElement->preceding  = newElement;
		firstElement             = newElement;
	}
	char str[512];
	sprintf(str, "%s%s", runManParam->OUTPUT_DIR, runManParam->SCORE_NEG_OUTFILE);
	if (runManParam->PLOT) fpSCORES_NEG.Open(str, "w");
	sprintf(str, "%s%s", runManParam->OUTPUT_DIR, runManParam->SCORE_RANDOM_OUTFILE);
	if (runManParam->PLOT) fpSCORES_RANDOM.Open(str, "w");

	ECH_SIZE_NEG    = 0;
	MEAN_NEG        = 0;
	VAR_NEG         = 0;

	//  randPep    = new peptide[runManParam->PVAL_ECHSIZE];                                                 memCheck.tagoResults++;
	//  iRandCount = 0;
	ECH_SIZE_RANDOM = 0;
	MEAN_RANDOM     = 0;
	VAR_RANDOM      = 0;
}

// ********************************************************************************************** //

void specresults::addDoublon(peptide *pep) {

	currentElement = firstElement->following;

	while (currentElement != lastElement) {

		if (currentElement->FILLED) 
		{
			if (!strcmp(pep->popiPep.dtbSeq, currentElement->Peptide->popiPep.dtbSeq))
			{
				currentElement->update(pep, runManParam->FILE_ERROR_NAME);
				return;
			}
		}
		currentElement = currentElement->following;
	}
}


/******************************************************************************/

void specresults::add_and_sort(peptide *pep, scenario *scen) {

	currentElement = firstElement->following;

	while (currentElement != lastElement) {

		if (!currentElement->FILLED) 
		{
			insert(pep, scen);
			return;
		}
		else
		{
			if (scen->Scores.finArrScore >= currentElement->Scenario->Scores.finArrScore)
			{
				// s'il s'agit d'un peptide identique, ne garde que l'offset correspondant
				if (!strcmp(pep->popiPep.dtbSeq, currentElement->Peptide->popiPep.dtbSeq))
				{
					//		addDoublon(pep, currentElement);
					return;
				}
				else
				{
					insert(pep,scen);
					return;
				}
			}
			else
			{
				currentElement = currentElement->following;
			}
		}
	}
}


/******************************************************************************/


void specresults::insert(peptide *pep, scenario *scen)
{
	newElement = new element();                                                                   memCheck.tagoResults++;
	elementNb++;


	newElement->fillElement(pep, scen);

	// always add before currentElement
	currentElement->preceding->following = newElement;
	newElement->preceding = currentElement->preceding;
	currentElement->preceding = newElement;
	newElement->following = currentElement;
	currentElement = newElement;
	removeElement(lastElement->preceding);
	return;
}

// ********************************************************************************************** //

void specresults::removeElement(element* el){
	// remove the element el 
	el->preceding->following = el->following;
	el->following->preceding = el->preceding;
	delete el;                                                                                        memCheck.tagoResults--;
	el = NULL;
	elementNb--;
}

/******************************************************************************/

void specresults::putRanks() 
{
	currentElement = firstElement->following;
	if (!currentElement->FILLED) return; //aucune prot�ine candidate n'a �t� trouv�e
	currentElement->rank = 1;

	while ((currentElement->following != lastElement) && (currentElement->following->FILLED))
	{
		if (isDoubleEqual(currentElement->following->Scenario->Scores.finArrScore, currentElement->Scenario->Scores.finArrScore))
			currentElement->following->rank = currentElement->rank;
		else
			currentElement->following->rank = currentElement->rank+1;
		currentElement = currentElement->following;
	}
}


// ********************************************************************************************** //


int specresults::giveRankOfCorrectPeptide() {
	// DONNE LE RANG DU PEPTIDE CORRECT S'IL SE TROUVE DANS LA RESULT_LIST  
	// SI SON SCORE EST EXAEQUO AVEC LE PREMIER, CONSIDERE QU'IL EST IDENTIFIE (IL S'AGIT D'UN I/L)

	if (!firstElement->following->FILLED) return -1;

	currentElement = firstElement->following;

	while ((currentElement != lastElement) && (currentElement->FILLED == true)) {
		if (!strcmp(currentElement->Peptide->popiPep.dtbSeq, spectrumData->get_seqAsInDtb())) {
			return currentElement->rank;
		}
		currentElement = currentElement->following;
	}

	return -1;         // PAS DANS LA LISTE
}

// ********************************************************************************************** //

void specresults::write(File &fp) {

	fprintf(fp, "SAMPLE SIZES for statistics: %i peptides neg\n", ECH_SIZE_NEG);
	fprintf(fp, "\n");
	fprintf(fp, "%9s | "  ,  "#");
	fprintf(fp, "%-27s | "  , "score (deltaS ; pValue)");
	fprintf(fp, "%-11s | "  ,  "mass");
	fprintf(fp, "%-25s | " ,  "access");
	fprintf(fp, "%-20s | ",  "id");
	fprintf(fp, "%-20s \n",  "dtbSeq / scenarios (shifts)");
	fprintf(fp, "%9s | %27s | %11s | %25s | %20s | %20s \n", "---------","---------------------------","-----------","-------------------------","--------------------","--------------------");

	//double nextScore;

	currentElement = firstElement->following;
	while ((currentElement != lastElement) && (currentElement->FILLED)) {

		fprintf(fp, "%2i. (%3i) | ",                     currentElement->rank, currentElement->Peptide->exemplairesNb);

		//    if (currentElement->Peptide->ISRANDOM) fprintf(fp, "%9s", "SHUFFLED");



		if (currentElement->following->Scenario != NULL) 
		{
			fprintf(fp, "%10.3f (%4.2f ;",               currentElement->Scenario->Scores.finArrScore, 
				currentElement->following->Scenario->Scores.finArrScore/currentElement->Scenario->Scores.finArrScore);
		}
		else
		{
			fprintf(fp, "%10.3f (1000 ;",               currentElement->Scenario->Scores.finArrScore);
		}

		if (currentElement->ECH_SIZE_NEG  < 20) fprintf(fp, "    1000) | ");
		else                                    fprintf(fp, " %7.1e) | ", currentElement->PVALUE_NEG);

		fprintf(fp, "%11.5f | ",         currentElement->Peptide->popiPep.dtbPepMass);
		fprintf(fp, "%-25s | ",         currentElement->Peptide->getProtein(0)->GetAC());
		char tempoText[ID_LENGHT];
		if (strlen(currentElement->Peptide->getProtein(0)->GetID()) > 19) 
		 {
		    strcpy(tempoText, currentElement->Peptide->getProtein(0)->GetID());
		 	tempoText[16] = '\0';
			strcat(tempoText, "...");
		 }
		else {
			strcpy(tempoText, currentElement->Peptide->getProtein(0)->GetID());
		}
		fprintf(fp, "%-20s | ",         tempoText);
		fprintf(fp, "%s",                      currentElement->Peptide->popiPep.dtbSeq);
		// fprintf(fp, " \t %f",                currentElement->ZSCORE_RANDOM);
		fprintf(fp, "\n");

		// l�, j'inscrit le scenario, et en m�me temps, j'affiche les autres AC et ID s'ils existent
		int i = 1;                                                           
		
		if (i < currentElement->Peptide->exemplairesNb) 
		{
			if (strlen(currentElement->Peptide->getProtein(i)->GetID()) > 19) 
			{
				strcpy(tempoText, currentElement->Peptide->getProtein(i)->GetID());
				tempoText[16] = '\0';
				strcat(tempoText, "...");
			} 
			fprintf(fp, "%9s | %27s | %11s | %-25s | %-20s | ", "", "", "", currentElement->Peptide->getProtein(i)->GetAC(), tempoText);
			i++;
		}
		else 
		{
			fprintf(fp, "%9s | %27s | %11s | %25s | %20s | ", "", "", "", "", "");
		}

		fprintf(fp, "%-18s", currentElement->Scenario->scenarioSeq);
		for (int w = 0; w < currentElement->Scenario->shiftNb; w++) 
		{
			if (fabs(currentElement->Scenario->shift[w]) > runManParam->FRAGMENT_ERROR2) 
				fprintf(fp, " %7.2f ", currentElement->Scenario->shift[w]);
		}

		fprintf(fp, "\n");

		// s'il reste des exemplaires � �crire, fait le maintenant
		if (i < currentElement->Peptide->exemplairesNb) {
			while (i < currentElement->Peptide->exemplairesNb) 
			{	
				if (strlen(currentElement->Peptide->getProtein(i)->GetID()) > 19) 
				{
				strcpy(tempoText, currentElement->Peptide->getProtein(i)->GetID());
				tempoText[16] = '\0';
				strcat(tempoText, "...");
				}
				fprintf(fp, "%9s | %27s | %11s | %-25s | %-20s | %20s \n", "", "", "", currentElement->Peptide->getProtein(i)->GetAC(), tempoText, "");
				i++;
			}
		}

		fprintf(fp, "%9s | %27s | %11s | %25s | %20s | %20s \n", "---------","---------------------------","-----------","-------------------------","--------------------","--------------------");

		currentElement = currentElement->following;
	}
}



// **********************************************************************************************  //

void specresults::writeXML(File &fp) {

	fprintf(fp, "         <sampleSize>%i</sampleSize>\n", ECH_SIZE_NEG);
	fprintf(fp, "      </dbSearch>\n");
	fprintf(fp, "      <matchList>\n");
	
	currentElement = firstElement->following;
	while ((currentElement != lastElement) && (currentElement->FILLED)) 
	{
        fprintf(fp, "        <match>\n");
        fprintf(fp, "          <rank>%i</rank>\n", currentElement->rank);
        fprintf(fp, "          <score>%f</score>\n", currentElement->Scenario->Scores.finArrScore);
        if (currentElement->following->Scenario != NULL) 
		{
			fprintf(fp, "          <deltaS>%f</deltaS>\n", currentElement->following->Scenario->Scores.finArrScore/currentElement->Scenario->Scores.finArrScore);
		}
		else
		{
			fprintf(fp, "          <deltaS>1000</deltaS>\n");
		}

		if (currentElement->ECH_SIZE_NEG  < 20) fprintf(fp, "          <pValue>1000</pValue>\n");
		else                                    fprintf(fp, "          <pValue>%f</pValue>\n", currentElement->PVALUE_NEG);

		fprintf(fp, "          <peptide>\n");
		fprintf(fp, "            <mass>%f</mass>\n", currentElement->Peptide->popiPep.dtbPepMass);
		fprintf(fp, "            <dbSequence>%s</dbSequence>\n", currentElement->Peptide->popiPep.dtbSeq);
		fprintf(fp, "            <scenario>%s</scenario>\n", currentElement->Scenario->scenarioSeq);
		
		for (int w = 0; w < currentElement->Scenario->shiftNb; w++) 
		{
			if (fabs(currentElement->Scenario->shift[w]) > runManParam->FRAGMENT_ERROR2) 
			{
				fprintf(fp, "            <shift>%f</shift>\n", currentElement->Scenario->shift[w]);
			}
		}
		fprintf(fp, "          </peptide>\n");
		fprintf(fp, "          <dbRefList>\n");
		fprintf(fp, "            <dbRef>\n");
		for (int e = 0; e < currentElement->Peptide->exemplairesNb; e++)
		{fprintf(fp, "               <ac>%s</ac>\n", currentElement->Peptide->getProtein(e)->GetAC());
		 fprintf(fp, "               <id>%s</id>\n", currentElement->Peptide->getProtein(e)->GetID());
		 fprintf(fp, "               <de>%s</de>\n", currentElement->Peptide->getProtein(e)->GetDE());}
		fprintf(fp, "            </dbRef>\n");
		fprintf(fp, "          </dbRefList>\n");
		fprintf(fp, "        </match>\n");
		
		currentElement = currentElement->following;
	}
		
	fprintf(fp, "      </matchList>\n");
    fprintf(fp, "    </spectrumMatch>\n");
}



// **********************************************************************************************  //

void specresults::writeShort(File &fp) {
   
	currentElement = firstElement->following;
	
	while ((currentElement != lastElement) && (currentElement->FILLED)) 
	{
	    fprintf(fp, "%i ", spectrumData->getSpecID());
        fprintf(fp, "%i ", currentElement->rank);
		fprintf(fp, "%s ", spectrumData->title);
        fprintf(fp, "%f ", currentElement->Scenario->Scores.finArrScore);
		fprintf(fp, "%s ", currentElement->Peptide->popiPep.dtbSeq);
		fprintf(fp, "%s ", currentElement->Scenario->scenarioSeq);
		
		for (int w = 0; w < currentElement->Scenario->shiftNb; w++) 
		{
			if (fabs(currentElement->Scenario->shift[w]) > runManParam->FRAGMENT_ERROR2) 
			{
				fprintf(fp, "%f ", currentElement->Scenario->shift[w]);
			}
		}
		fprintf(fp, "\n");
		currentElement = currentElement->following;
	}
}

// **********************************************************************************************  //

void specresults::writeSimple(File &fp) {

	fprintf(fp, "\n");
	fprintf(fp, "%9s | "  ,  "#");
	fprintf(fp, "%18s | "  , "score      ");
	fprintf(fp, "%7s | "  ,  "mass  ");
	fprintf(fp, "%6s | " ,  "access");
	fprintf(fp, "%15s | ",  "id      ");
	fprintf(fp, "%-20s \n",  "dtbSeq / scenario (shifts)");
	fprintf(fp, "%9s | %18s | %7s | %6s | %15s | %20s \n", "---------","------------------","-------","------","---------------","--------------------");


	currentElement = firstElement->following;
	while ((currentElement != lastElement) && (currentElement->FILLED)) {

		fprintf(fp, "%2i. (%3i) | ",                  currentElement->rank, currentElement->Peptide->exemplairesNb);

		fprintf(fp, "%18.3f | ", currentElement->Scenario->Scores.finArrScore);


		fprintf(fp, "%7.2f | ",                currentElement->Peptide->popiPep.dtbPepMass);
		fprintf(fp, "%-6s | %-15s | ",         currentElement->Peptide->getProtein(0)->GetAC(), currentElement->Peptide->getProtein(0)->GetID());
		fprintf(fp, " %s",                     currentElement->Peptide->popiPep.dtbSeq);
		// fprintf(fp, " \t %f",                currentElement->ZSCORE_RANDOM);
		fprintf(fp, "\n");

		// l�, j'inscrit le scenario, et en m�me temps, j'affiche les autres AC et ID s'ils existent
		int i = 1;                                                           
		if (i < currentElement->Peptide->exemplairesNb) { 
			fprintf(fp, "%9s | %18s | %7s | %-6s | %-15s | ", "", "", "", currentElement->Peptide->getProtein(i)->GetAC(), currentElement->Peptide->getProtein(i)->GetID());
			i++;
		}
		else {
			fprintf(fp, "%9s | %18s | %7s | %6s | %15s | ", "", "", "", "", "");
		}

		fprintf(fp, "%-28s", currentElement->Scenario->scenarioSeq);
		for (int w = 0; w < currentElement->Scenario->shiftNb; w++) 
		{
			if (fabs(currentElement->Scenario->shift[w]) > runManParam->FRAGMENT_ERROR2) 
				fprintf(fp, " %7.2f ", currentElement->Scenario->shift[w]);
		}

		fprintf(fp, "\n");

		// s'il reste des exemplaires � �crire, fait le maintenant
		if (i < currentElement->Peptide->exemplairesNb) {
			while (i < currentElement->Peptide->exemplairesNb) {	
				fprintf(fp, "%9s | %18s | %7s | %-6s | %-15s | %20s \n", "", "", "", currentElement->Peptide->getProtein(i)->GetAC(), currentElement->Peptide->getProtein(i)->GetID(), "");
				i++;
			}
			//fprintf(fp, "\n");
		}

		fprintf(fp, "%9s | %18s | %7s | %6s | %15s | %20s \n", "---------","------------------","-------","------","---------------","--------------------");

		currentElement = currentElement->following;
	}
}



// **********************************************************************************************  //

void specresults::computePValuesNEG() 
{ 
	currentElement = firstElement->following;

	// IL FAUT RETIRER LE SCORE LE PLUS ELEVE DE LA DISTRIBUTION, CAR SA VALEUR PEUT ETRE >> QUE LES AUTRES
	// ET CELA PEUT PERTURBER SIGNIFICATIVEMENT LA DISTRIBUTION
	// JE LE FAIS DE MANIERE PERMANENTE POUR LE PREMIER ELEMENT
	// PUIS POUR LES AUTRES, J'ENLEVE TOUR A TOUR LEUR SCORE, MAIS SEULEMENT POUR LE CALCUL DE LEUR PVALUE

	float curr_MEAN_NEG = 0;
	float curr_VAR_NEG  = 0;

	while (currentElement != lastElement) 
	{ 
		if (currentElement->FILLED) {

			// RETIRE LE MEILLEUR SCORE DES STATS
			if (currentElement == firstElement->following)
			{   // si c'est le premier element
				MEAN_NEG      = MEAN_NEG     -  (float)currentElement->Scenario->Scores.finArrScore;
				VAR_NEG       = VAR_NEG      - (float)(currentElement->Scenario->Scores.finArrScore * currentElement->Scenario->Scores.finArrScore);
				ECH_SIZE_NEG  = ECH_SIZE_NEG -  1;
				curr_MEAN_NEG = MEAN_NEG;
				curr_VAR_NEG  = VAR_NEG;
			}
			else
			{
				curr_MEAN_NEG          = MEAN_NEG - (float)currentElement->Scenario->Scores.finArrScore;
				curr_VAR_NEG           = VAR_NEG  - (float)(currentElement->Scenario->Scores.finArrScore * currentElement->Scenario->Scores.finArrScore);
			}

			// TERMINE LE CALCUL DE MEAN ET VAR

			if ((ECH_SIZE_NEG > 1) & (curr_VAR_NEG > 0))
			{
				curr_MEAN_NEG       =   curr_MEAN_NEG     /  (float)(ECH_SIZE_NEG-1);
				curr_VAR_NEG        =   curr_VAR_NEG      /  (float)(ECH_SIZE_NEG-1);
				curr_VAR_NEG        =   curr_VAR_NEG      -  (curr_MEAN_NEG * curr_MEAN_NEG);
				
				double temp1, temp2, temp3;
				temp3 = sqrt(fabs(curr_VAR_NEG));
				if (temp3 < 0.00001) temp3 = 0.00001; // pr�vient la division par 0 lorsque tous les scores sont tr�s similaires
				currentElement->ZSCORE_NEG  =  ((float)currentElement->Scenario->Scores.finArrScore - curr_MEAN_NEG)/temp3; 
				if (currentElement->ZSCORE_NEG > 0) temp1 =     normal3(currentElement->ZSCORE_NEG, runManParam->FILE_ERROR_NAME)+0.5; 
				// normal3 repr�sente toute la partie GAUCHE par rapport au meilleur score
				else                                temp1 =  1-(normal3(currentElement->ZSCORE_NEG, runManParam->FILE_ERROR_NAME)+0.5);
				// normal3 repr�sente toute la partie DROITE par rapport au meilleur score, donc 1-() 

				temp2 = pow(temp1, ECH_SIZE_NEG-1); // ATTENTION, SI JE FAIS (DOUBLE)ECH_SIZE, RESULTAT ERRONE
				// temp2 repr�sente la probabilit� que tous les peptides scor�s de la dtb soient < bestScore
				currentElement->PVALUE_NEG = 1-temp2;
				//  PVALUE     =   1-(pow(normal3(ZSCORE_NEG, runManParam->FILE_ERROR_NAME)+0.5,(double)ECH_SIZE_NEG)); // on ne consid�re que ceux qui ont �t� scor�s; on pourrait
				//  PVALUE EST LA PROBABILITE QUE, DANS UNE DTB DE ECH_SIZE SEQUENCES, AUCUNE N'AIT UN SCORE SIMILAIRE OU SUP A BESTSCORE
				currentElement->ECH_SIZE_NEG = ECH_SIZE_NEG-1;
			}
			else
			{
				currentElement->ZSCORE_NEG   = 0;
				currentElement->PVALUE_NEG   = 0;
				currentElement->ECH_SIZE_NEG = 0;
			}
		}
		currentElement = currentElement->following;
	}
}

// ********************************************************************************************** //

void specresults::computePValuesRANDOM() 
{ 

	MEAN_RANDOM       =   MEAN_RANDOM     /  (float)ECH_SIZE_RANDOM;
	VAR_RANDOM        =   VAR_RANDOM      /  (float)ECH_SIZE_RANDOM;
	VAR_RANDOM        =   VAR_RANDOM      -  (MEAN_RANDOM * MEAN_RANDOM);

	currentElement = firstElement->following;

	while (currentElement != lastElement) 
	{ 
		if (currentElement->FILLED) {

			// TERMINE LE CALCUL DE MEAN ET VAR

			if ((ECH_SIZE_RANDOM > 0) & (VAR_RANDOM > 0))
			{

				currentElement->ZSCORE_RANDOM  =  ((float)currentElement->Scenario->Scores.finArrScore - MEAN_RANDOM)/sqrt(fabs(VAR_RANDOM));  

				double temp1, temp2;
				if (currentElement->ZSCORE_RANDOM > 0)    temp1 =    normal3(currentElement->ZSCORE_RANDOM, runManParam->FILE_ERROR_NAME)+0.5; 
				// normal3 repr�sente toute la partie GAUCHE par rapport au meilleur score
				else                                      temp1 = 1-(normal3(currentElement->ZSCORE_RANDOM, runManParam->FILE_ERROR_NAME)+0.5);
				// normal3 repr�sente toute la partie DROITE par rapport au meilleur score, donc 1-() 

				temp2 = pow(temp1, ECH_SIZE_RANDOM); // ATTENTION, SI JE FAIS (DOUBLE)ECH_SIZE, RESULTAT ERRONE

				// temp2 repr�sente la probabilit� que tous les peptides scor�s de la dtb soient < bestScore
				currentElement->PVALUE_RANDOM = 1-temp2;
				//  PVALUE     =   1-(pow(normal3(ZSCORE_RANDOM, runManParam->FILE_ERROR_NAME)+0.5,(double)ECH_SIZE_RANDOM)); // on ne consid�re que ceux qui ont �t� scor�s; on pourrait
				//  PVALUE EST LA PROBABILITE QUE, DANS UNE DTB DE ECH_SIZE SEQUENCES, AUCUNE N'AIT UN SCORE SIMILAIRE OU SUP A BESTSCORE
				currentElement->ECH_SIZE_RANDOM = ECH_SIZE_RANDOM;
			}
			else
			{
				currentElement->ZSCORE_RANDOM   = 0;
				currentElement->PVALUE_RANDOM   = 0;
				currentElement->ECH_SIZE_RANDOM = 0;
			}
		}
		currentElement = currentElement->following;
	}
}

// ********************************************************************************************** //

void specresults::createPopScoresNEG(int specID) 
{
	// cr�� le fichier montrant la distribution des scores avec R

	fpSCORES_NEG.Close();

	char com[512];
	char scriptName[512];
	File fp;

	if ((ECH_SIZE_NEG > 3) && (currentElement->FILLED))
	{
		sprintf(com, "mv %s%s %sSCORES_PLOTS_NEG_%i.txt", runManParam->OUTPUT_DIR, runManParam->SCORE_NEG_OUTFILE, runManParam->OUTPUT_DIR, specID%12); 
		system(com);

		sprintf(scriptName, "%sSCORES_NEG_RSCRIPT.txt", runManParam->OUTPUT_DIR);

		if (specID%12 == 0)  {
			fp.Open(scriptName, "w");
			fprintf(fp, "par(mfrow=c(3,4))\n"); 
		}
		else {fp.Open(scriptName, "a");}

		sprintf(com, "var_%i <- read.table(\"%sSCORES_PLOTS_NEG_%i.txt\")\n",specID%12, runManParam->OUTPUT_DIR, specID%12);
		fprintf(fp, "%s", com);

		sprintf(com, "hist(var_%i[,1], breaks = 1000, main=\"SPEC#%i, BESTSCORE=%.2f\", xlab=\"score\", ylab=\"count\")\n", specID%12, specID, firstElement->following->Scenario->Scores.finArrScore);
		//  sprintf(com, "hist(var_%i[,1], breaks = 50, main=\"SPEC#%i, MEAN=%.2f BESTZ=%.2f\", xlab=\"score\", ylab=\"count\")\n", specID%12, specID, MEAN_NEG/(float)ECH_SIZE_NEG, firstElement->following->ZSCORE_NEG);
		fprintf(fp, "%s", com);
		fp.Close();
	}

	if (specID%12 == 11) {
		sprintf(com, "R < %s --save", scriptName);
		system(com);
		sprintf(com, "mv Rplots.ps %sSCORES_PLOTS_NEG_%i_TO_%i.ps", runManParam->OUTPUT_DIR, specID-11, specID);
		system(com);
	}
}

// ********************************************************************************************** //

void specresults::createPopScoresRANDOM(int specID) 
{
	// cr�� le fichier montrant la distribution des scores avec R

	fpSCORES_RANDOM.Close();
	char com[512];
	char scriptName[512];
	File fp;
	if ((ECH_SIZE_RANDOM > 3) && (currentElement->FILLED))
	{
		sprintf(com, "mv %s%s %sSCORES_PLOTS_RANDOM_%i.txt", runManParam->OUTPUT_DIR, runManParam->SCORE_RANDOM_OUTFILE, runManParam->OUTPUT_DIR, specID%12); 
		system(com);

		sprintf(scriptName, "%sSCORES_RANDOM_RSCRIPT.txt", runManParam->OUTPUT_DIR);

		if (specID%12 == 0)  {
			fp.Open(scriptName, "w");
			fprintf(fp, "par(mfrow=c(3,4))\n"); 
		}
		else {fp.Open(scriptName, "a");}

		sprintf(com, "var_%i <- read.table(\"%sSCORES_PLOTS_RANDOM_%i.txt\")\n",specID%12, runManParam->OUTPUT_DIR, specID%12);
		fprintf(fp, "%s", com);
		sprintf(com, "hist(var_%i[,1], breaks = 1000, main=\"SPEC#%i, BESTSCORE=%.2f\", xlab=\"score\", ylab=\"count\")\n", specID%12, specID, firstElement->following->Scenario->Scores.finArrScore);
		//  sprintf(com, "hist(var_%i[,1], breaks = 50, main=\"SPEC#%i, MEAN=%.2f BESTZ=%.2f\", xlab=\"score\", ylab=\"count\")\n", specID%12, specID, MEAN_RANDOM, firstElement->following->ZSCORE_RANDOM);
		fprintf(fp, "%s", com);
		fp.Close();
	}

	if (specID%12 == 11) {
		sprintf(com, "R < %s --save", scriptName);
		system(com);
		sprintf(com, "mv Rplots.ps %sSCORES_PLOTS_RANDOM_%i_TO_%i.ps", runManParam->OUTPUT_DIR, specID-11, specID);
		system(com);
	}
}

// ********************************************************************************************** //
