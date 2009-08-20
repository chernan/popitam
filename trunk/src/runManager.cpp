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

#include "runManager.h"
#include "memorycheck.h"
#include "error.h"
#include "file.h"
#include <time.h>
#include <iostream>

using namespace	std;

// ********************************************************************************************** //
// GLOBAL VARIABLES DECLARED ELSEWHERE 

extern memorycheck memCheck;

// ********************************************************************************************** //

runManager::runManager() 
{
	runManParam      = NULL;
	aaParam          = NULL;
	ionParamTOFTOF1  = NULL;
	ionParamQTOF1    = NULL;
	ionParamQTOF2    = NULL;
	ionParamQTOF3    = NULL;
	spectrumData     = NULL;
	popiResults      = NULL;
	gp_parameters    = NULL;
	scoreFunction[0] = NULL;
	scoreFunction[1] = NULL;
	scoreFunction[2] = NULL;
	popitam          = NULL;
	allRunStats      = NULL;
}

// ********************************************************************************************** //

runManager::~runManager() 
{
	if (runManParam != NULL) {
		delete runManParam;
		runManParam = NULL;
		memCheck.runManParam--;
	}
	if (aaParam != NULL) {
		delete aaParam;
		aaParam = NULL;
		memCheck.aa--;
	}
	if (ionParamTOFTOF1 != NULL) {
		delete ionParamTOFTOF1;
		ionParamTOFTOF1 = NULL;
		memCheck.ion--;
	}
	if (ionParamQTOF1 != NULL) {
		delete ionParamQTOF1;
		ionParamQTOF1 = NULL;
		memCheck.ion--;
	}
	if (ionParamQTOF2 != NULL) {
		delete ionParamQTOF2;
		ionParamQTOF2 = NULL;
		memCheck.ion--;
	}
	if (ionParamQTOF3 != NULL) {
		delete ionParamQTOF3;
		ionParamQTOF3 = NULL;
		memCheck.ion--;
	}
	if (spectrumData != NULL) {
		delete spectrumData;
		spectrumData = NULL;
		memCheck.data--;
	}
	if (popiResults != NULL) {
		delete popiResults;
		popiResults = NULL;
		memCheck.results--;
	}
	if (allRunStats != NULL) {
		delete allRunStats;
		allRunStats = NULL;
		memCheck.allrunstatistics--;
	}
	if (gp_parameters != NULL) {
		delete gp_parameters;
		gp_parameters = NULL;
		memCheck.functionScore--;
	}
	if (scoreFunction[0] != NULL) {
		delete scoreFunction[0];
		scoreFunction[0]= NULL;
		memCheck.functionScore--;
	}
	if (scoreFunction[1] != NULL) {
		delete scoreFunction[1];
		scoreFunction[1]= NULL;
		memCheck.functionScore--;
	}
	if (scoreFunction[2] != NULL) {
		delete scoreFunction[2];
		scoreFunction[2] = NULL;
		memCheck.functionScore--;
	}
	if (popitam != NULL) {
		delete popitam;
		popitam = NULL;
		memCheck.compare--;
	}
}

// ********************************************************************************************** //

void runManager::init(int argc, char** argv) 
{
	initRunManParam(argc,	argv);

	if (runManParam->r_CHECK) {
		prepareOutDir();
	}
	
	initAaParam();
	initIonParam();
	initPopiResult();
	initAllRunStats();
	
	// MISE EN MEMOIRE DES FONCTIONS DE SCORE
	initScoreFunctions();
}

// ********************************************************************************************** //

void runManager::initRunManParam(int argc, char** argv)
{
	runManParam = new runManagerParameters();
	memCheck.runManParam++;
	runManParam->init(argc, argv);
	runManParam->display(runManParam->FILEOUT);
	
	if (runManParam->XML_OUT) {
		runManParam->displayXML(runManParam->FILEOUTXML);
	}
}

// ********************************************************************************************** //

void runManager::initAaParam() 
{
	aaParam = new aa();																			  memCheck.aa++;
	aaParam->init(runManParam);
}

// ********************************************************************************************** //

void runManager::initIonParam()
{
	ionParamTOFTOF1 = new ion();
	ionParamTOFTOF1->init(runManParam->FILE_ERROR_NAME, runManParam->PROBS_TOFTOF1_INFILE, aaParam);
	memCheck.ion++;
	ionParamQTOF1 = new ion();
	ionParamQTOF1->init(runManParam->FILE_ERROR_NAME, runManParam->PROBS_QTOF1_INFILE, aaParam);
	memCheck.ion++;  
	ionParamQTOF2 = new ion();
	ionParamQTOF2->init(runManParam->FILE_ERROR_NAME, runManParam->PROBS_QTOF2_INFILE, aaParam);
	memCheck.ion++;
	ionParamQTOF3 = new ion();
	ionParamQTOF3->init(runManParam->FILE_ERROR_NAME, runManParam->PROBS_QTOF3_INFILE, aaParam);
	memCheck.ion++;
	
	if (runManParam->r_CHECK) {
		char filename[256] = "";
		File fp;
		sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "ION_TOFTOF1_sorted.txt");
		fp.Open(filename, "w");
		ionParamTOFTOF1->writeSorted(fp);
		fp.Close();
		
		sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "ION_QTOF1_sorted.txt");
		fp.Open(filename, "w");
		ionParamQTOF1->writeSorted(fp);
		fp.Close();
		
		sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "ION_QTOF2_sorted.txt");
		fp.Open(filename, "w");
		ionParamQTOF2->writeSorted(fp);
		fp.Close();
		
		sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "ION_QTOF3_sorted.txt");
		fp.Open(filename, "w");
		ionParamQTOF3->writeSorted(fp);
		fp.Close();
	}
}

// ********************************************************************************************** //

void runManager::initPopiResult() 
{
	popiResults = new result;
	memCheck.results++;
	popiResults->init(runManParam);
}

// ********************************************************************************************** //

void runManager::initAllRunStats() 
{
	allRunStats = new allrunstatistics;
	memCheck.allrunstatistics++;
	allRunStats->init();
}

// ********************************************************************************************** //

void runManager::initScoreFunctions()
{
	gp_parameters = new PARAMETERS;
	memCheck.functionScore++;
	gp_parameters->init(runManParam->GPPARAMETERS_INFILE);
	
	scoreFunction[0] = new TREE;
	memCheck.functionScore++;
	scoreFunction[0]->LoadTree(gp_parameters, runManParam->FILE_ERROR_NAME, runManParam->SCORE_FUNCTION0_INFILE);
	scoreFunction[0]->UpdateTree(runManParam->FILE_ERROR_NAME);
	
	scoreFunction[1] = new TREE;
	memCheck.functionScore++;
	scoreFunction[1]->LoadTree(gp_parameters, runManParam->FILE_ERROR_NAME, runManParam->SCORE_FUNCTION1_INFILE);
	scoreFunction[1]->UpdateTree(runManParam->FILE_ERROR_NAME);
	
	scoreFunction[2] = new TREE;
	memCheck.functionScore++;
	scoreFunction[2]->LoadTree(gp_parameters, runManParam->FILE_ERROR_NAME, runManParam->SCORE_FUNCTION2_INFILE);
	scoreFunction[2]->UpdateTree(runManParam->FILE_ERROR_NAME);
}

// ********************************************************************************************** //

void runManager::run()
{
	if (runManParam->XML_OUT) {
		fprintf(runManParam->FILEOUTXML, "<spectrumMatchList>\n");
	}
	
	spectrumData = new data();
	memCheck.data++;
	spectrumData->init(runManParam);
	
	// DIGESTION: static initialization
	IOParam param;
	param.SetEnzyme(runManParam->ENZYME);
	param.m_iMissedClevage	= runManParam->MISSED;
	param.m_eResolution = MASS_MONOISOTOPIC;
	param.m_eIonMode = ION_MODE_M;
	param.m_bPTM = false;
	Digest digest;
	digest.Load(&param);
	
	while (spectrumData->load()) {
		spectrumData->display(runManParam->FILEOUT);
		
		if (runManParam->XML_OUT) {
			spectrumData->displayXML(runManParam->FILEOUTXML);
		}
		
		unsigned long int iniTime = clock();
		
		popitam = new Compare();
		memCheck.popitam++;
		
		// CONSTRUIT LE GRAPHE, DIRIGE LES POINTEURS
		//gfs
		unsigned long int t0 = clock();
		//eof gfs
		popitam->init_POP(runManParam, scoreFunction[runManParam->m_MUTMOD], aaParam,
				   ionParamTOFTOF1, ionParamQTOF1, ionParamQTOF2, ionParamQTOF3,
				   spectrumData, popiResults, allRunStats);
		//gfs
		double dt0 = ((double)(clock()-t0)/(CLOCKS_PER_SEC/1000));
		unsigned long int t1 = clock();
		//eof gfs
		popitam->init_DIG(); // PREPARE LA DIGESTION
		
		
		// DIGESTION: dynamic initialization
		digest.SetCompare(popitam);
		// PEPTIDE RANGE: ABSOLUTE MIN = 666 (~6aa); ABSOLUTE MAX = 3333 (~30aa);
		double MIN = 666;
		double MAX = 3333;
		
		// c'est l'erreur sur la masse parente qui determine le range
		if (!runManParam->m_MUTMOD) {
			if ((spectrumData->get_parentMassM() - runManParam->PREC_MASS_ERROR) > 666) {  
				MIN = spectrumData->get_parentMassM() - runManParam->PREC_MASS_ERROR;
			}
			if ((spectrumData->get_parentMassM() + runManParam->PREC_MASS_ERROR) < 3333) { 
				MAX = spectrumData->get_parentMassM() + runManParam->PREC_MASS_ERROR;
			}
		}
		else {
			if ((spectrumData->get_parentMassM() - runManParam->UP_LIMIT_RANGE_PM)  > 666) {  
				MIN = spectrumData->get_parentMassM() - runManParam->UP_LIMIT_RANGE_PM;
			}
			if ((spectrumData->get_parentMassM() - runManParam->LOW_LIMIT_RANGE_PM) < 3333) { 
				MAX = spectrumData->get_parentMassM() - runManParam->LOW_LIMIT_RANGE_PM;
			}
		}
		
		digest.Limit(MIN, MAX);
		// ---------------------------

		//gfs
		double dt1 = ((double)(clock()-t1)/(CLOCKS_PER_SEC/1000));
		unsigned long int t2 = clock();
		//eof gfs
		popitam->Run(&digest);      // IDENTIFIE LE SPECTRE
		//gfs
		double dt2 = ((double)(clock()-t2)/(CLOCKS_PER_SEC/1000));
		//cout << "init_POP = " << dt0 << endl;
		//cout << "init_DIG = " << dt1 << endl;
		//cout << "Run = " << dt2 << endl;
		//eof gfs
		
		popitam->EndRun(spectrumData->specID);
		
		fprintf(runManParam->FILEOUT,	"\nProcessing time was:	%f\n", (double)(clock()-iniTime)/CLOCKS_PER_SEC);
		
		//popitam->DisplayUnusedAC(runManParam->FILEOUT);
		fprintf(runManParam->FILEOUT,	"\n\nNEXT______________________________________________________________\n\n");
		
		if (popitam != NULL) {
			delete popitam;
			popitam = NULL;
			memCheck.popitam--;
		}
	}

	if (runManParam->XML_OUT) {
		fprintf(runManParam->FILEOUTXML, "</spectrumMatchList>\n");
		fprintf(runManParam->FILEOUTXML, "<spectrumList>\n");
	}

	if (spectrumData != NULL) {
		delete spectrumData;
		spectrumData = NULL;
		memCheck.data--;
	}

	if (runManParam->XML_OUT) {
		// je reload les spectres pour le fichier output xml (les spectres doivent appara�tre � la fin)
		runManParam->FILEIN.Close();
		runManParam->FILEIN.Open(runManParam->FILEINNAME, "r");
		spectrumData = new data();
		memCheck.data++;
		spectrumData->init(runManParam);
		
		while(spectrumData->load()) {
			spectrumData->writeSpectrumListXML(runManParam->FILEOUTXML);
		}
		
		if (spectrumData != NULL) {
			delete spectrumData;
			spectrumData = NULL;
			memCheck.data--;
		}
		
		fprintf(runManParam->FILEOUTXML, "</spectrumList>\n");
	}
}

// ********************************************************************************************** //

void runManager::prepareOutDir()
{
	// NECESSAIRE	DE NETTOYER	POUR SUFFTAB ET	SUFFTREE, CAR ILS SONT OUVERTS EN MODE "a"
	File fp;
	char str[256];
	
	sprintf(str, "rm %sSUFFTAB.txt %sSUFFTREE.txt", runManParam->OUTPUT_DIR, runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sCOMPRED_TAB.txt %sSCENARIOS.txt", runManParam->OUTPUT_DIR, runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sARRANGEMENTS.txt", runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sGRAPH*.txt", runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sGRAPHDOT.dot", runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sAMINO_ACIDS*.txt", runManParam->OUTPUT_DIR);
	system(str);
	sprintf(str, "rm %sION_*.txt", runManParam->OUTPUT_DIR);
	system(str);
	
	sprintf(str, "rm %sSCORE*	%sTIME.txt", runManParam->OUTPUT_DIR,  runManParam->OUTPUT_DIR);
	system(str);
	sprintf(str, "rm %sSOURCE_SPECTRUM*.txt", runManParam->OUTPUT_DIR);
	system(str);
	sprintf(str, "rm %sTAGLIST*.txt", runManParam->OUTPUT_DIR);
	system(str);

	if (runManParam->r_FUN) {
		sprintf(str, "rm %s*.fun", runManParam->GEN_OR_FILENAME_SUFF);
		system(str);
		sprintf(str, "rm %s*.fun", runManParam->GEN_NOD_FILENAME_SUFF);
		system(str);
	}
	
	// PLUS CLAIR, LORSQUE LES REMOVES DONNENT DES MESSAGES D'ERREUR
	fprintf(runManParam->FILEOUT,	"\n\n");
}

// ********************************************************************************************** //

void runManager::endRun()
{   
	if (runManParam->s_IDSET) {
		writePerformanceIDSET(runManParam->FILEOUT);
	}
	
	// if (runManParam->s_MIXSET) writePerformanceMIXSET(runManParam->FILEOUT);
	
	// LISTING DES PROTEINS IDENTIFIEES
	if (runManParam->r_NORMAL)  {
		// S'IL Y A AU MOINS UN ELEMENT
		if (popiResults->protList->firstElement->following->FILLED) {
			popiResults->protList->compile();
			popiResults->protList->writeSimple(runManParam->FILEOUT);
			// popiResults->protList->write(runManParam->FILEOUT);
		}
		else {
			fprintf(runManParam->FILEOUT, "Sorry, no identified proteins");
		}
	}
}

// ********************************************************************************************** //

void runManager::writePerformanceIDSET(File& fp)
{
	fprintf(fp, "\nPERFORMANCE:\n\n");
	fprintf(fp, "\nTabRank : \n\n");
	
	for (int i = 0; i < runManParam->RESULT_NB; i++) {
		fprintf(fp, "# correct identifications in rank %i: %i ", i, popiResults->tabRank[i]);
		if ((runManParam->SPECTRUM_NB - allRunStats->notInDtb) == 0) {
			fprintf(fp, "(%s)\n", "0%");
		}
		else {
			fprintf(fp, "(%.1f%s)\n", (float)popiResults->tabRank[i]/(float)(runManParam->SPECTRUM_NB - allRunStats->notInDtb)*100, "%");
		}
	}
	
	fprintf(fp, "\n\n%i sequences	were not in	the	database\n\n\n", allRunStats->notInDtb);
	fprintf(fp, "\nConfusion matrix:\n\n");
	fprintf(fp, "%12s|%12s|%12s|\n", "", "ACT_NEG	", "ACT_POS	");
	fprintf(fp, "------------|------------|------------|\n");
	fprintf(fp, "%12s| TN%8s | FN%8i |\n", "PRED_NEG ", "plenty",	allRunStats->falseNeg);
	fprintf(fp, "------------|------------|------------|\n");
	fprintf(fp, "%12s| FP%8i | TP%8i |\n", "PRED_POS ", allRunStats->falsePos, allRunStats->truePos);
	fprintf(fp, "------------|------------|------------|\n\n");
	fprintf(fp, "\nSensitivity (TPR):	%4.2f,	",float(allRunStats->truePos)/(float(allRunStats->falsePos)+float(allRunStats->truePos)));
	fprintf(fp, "with	TPR	= TP/(FP+TP), =	proportion of positive cases that were correctly identified");
	fprintf(fp, "\nPrecision (P):		%4.2f,	",float(allRunStats->truePos)/(float(allRunStats->falseNeg)+float(allRunStats->truePos)));
	fprintf(fp, "with	P	= TP/(FN+TP), =	proportion of the predicted	positive cases that	were correct");
	
	if (allRunStats->truePos == 0) {
		fprintf(fp, "\n\nPercent of True Positive cases	  =	%s", "0%");
	}
	else {
		fprintf(fp, "\n\nPercent of True Positive cases	  =	%5.1f%s",
			float(allRunStats->truePos) /(float)(runManParam->SPECTRUM_NB - allRunStats->notInDtb) * 100,"%");
	}
	if (allRunStats->falsePos == 0) {
		fprintf(fp, "\n\nPercent of False	Positive cases	   = %s", "0%");
	}
	else {
		fprintf(fp, "\nPercent	of False Positive cases	   = %5.1f%s",
			float(allRunStats->falsePos)/(float)(runManParam->SPECTRUM_NB - allRunStats->notInDtb) * 100, "%");
	}
	fprintf(fp, "\n\n%i sequences	were not in	the	database\n", allRunStats->notInDtb);
}

