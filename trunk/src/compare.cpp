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

#include "defines.h"
#include "error.h"
#include "compare.h"
#include "memorycheck.h"
#include "tag_extractor.h"
#include "tag_processor.h"
#include "dbreader.h"
#include "MATerror.h"

using namespace	std;

// ********************************************************************************************** //

//const	char *Path::m_pszFileName =	"Popitam:-)";

// ********************************************************************************************** //

extern memorycheck memCheck;

// ********************************************************************************************** //

Compare::Compare() 
{
	m_digest.SetCompare(this);


  runManParam			= NULL;
  allRunStats			= NULL;
  scoreF				= NULL;
  aaParam				= NULL;
  ionParam				= NULL;
  popiResults			= NULL;

  specStats				= NULL;
  spectrumData			= NULL;
  popiGraph				= NULL;
  popiTags				= NULL;
  aPeptide				= NULL;
  funGen				= NULL;
  specResults			= NULL;
  treeWord				= NULL;
  ACtree				= NULL;

  randomEchNb			= 0;
}

// ********************************************************************************************** //


Compare::~Compare() 
{
  runManParam			= NULL;
  allRunStats			= NULL;
  scoreF				= NULL;
  aaParam				= NULL;
  ionParam				= NULL;
  popiResults			= NULL;
  
  if (specStats	   != NULL)	{delete	specStats;	  specStats	   = NULL;							  memCheck.spectrumstatistics--;}
  if (popiGraph	   != NULL)	{delete	popiGraph;	  popiGraph	   = NULL;							  memCheck.graph--;}
  if (popiTags	   != NULL)	{delete	popiTags;	  popiTags	   = NULL;							  memCheck.subseq--;}
  if (aPeptide	   != NULL)	{delete	aPeptide;	  aPeptide	   = NULL;							  memCheck.peptide--;}
  if (specResults  != NULL)	{delete	specResults;  specResults  = NULL;							  memCheck.results--;}
  if (funGen	   != NULL)	{delete	funGen;		  funGen	   = NULL;							  memCheck.fun--;}
  if (treeWord	   != NULL)	{delete	treeWord;	  treeWord	   = NULL;							  memCheck.treeword--;}
  if (ACtree	   != NULL)	{delete	ACtree;		  ACtree	   = NULL;							  memCheck.treeword--;}
}

// ********************************************************************************************** //

void Compare::init_POP(runManagerParameters* rMP, TREE*	F, aa* aaP,	ion* TOFTOF1, ion* QTOF1, ion* QTOF2,ion* QTOF3, 
			   data* specD,	result*	popiR, allrunstatistics	*allRunS)
{
  runManParam	= rMP;
  scoreF		= F;
  aaParam		= aaP;
  spectrumData	= specD;
  popiResults	= popiR;
  allRunStats	= allRunS;
  randomEchNb	= 0;

  if (runManParam->r_FUN)  
	{
	  funGen = new fun();		memCheck.fun++;
	  funGen->init(runManParam,	spectrumData->specID);
	}

  specResults =	new specresults();		memCheck.results++;
  specResults->init(runManParam, spectrumData);
  
  specStats = new spectrumstatistics();		memCheck.spectrumstatistics++;
  specStats->init();

  treeWord = new TreeWord();                    memCheck.treeword++;
  treeWord->insert(" ", 0.0);

  char str[256];
  sprintf(str, "%s%i", runManParam->INSTRUMENT,	spectrumData->get_charge());		   
  // DIRIGE LE POINTEUR VERS LE BON IONPARAM
  
  if (!strcmp(str, "TOFTOF1")) ionParam	= TOFTOF1;
  if (!strcmp(str, "QTOF1"))   ionParam	= QTOF1;
  if (!strcmp(str, "QTOF2"))   ionParam	= QTOF2;
  if (!strcmp(str, "QTOF3"))   ionParam	= QTOF3;

  if (!ionParam) ionParam = QTOF3; // évite la fatal error due à une charge non supportée
  /*
    {
      char text[512];
      sprintf(text, "%s%s%s (check tag=%s)", "No ion probability file for spectrum ", specD->title, "; please remove spectrum.", str);
      fatal_error(runManParam->FILE_ERROR_NAME, DATAS, text);
    }
  */
  buildGraph();

}


// ********************************************************************************************** //

void Compare::buildGraph()
{
  popiGraph		= new graphe();																		  memCheck.graph++;
  popiGraph->init(runManParam, aaParam,	ionParam, spectrumData);
  popiGraph->display(runManParam->FILEOUT);
  popiGraph->displayXML(runManParam->FILEOUTXML);
}

// ********************************************************************************************** //

void Compare::init_DIG()
{  
	int i;
  
  //************************ DATABASES ************************
  if (strcmp(runManParam->DB1_PATH, "NO"))
		m_db.Open(runManParam->DB1_PATH);
  if (strcmp(runManParam->DB2_PATH, "NO"))
		m_db.Open(runManParam->DB2_PATH);
		
  if ((!strcmp(runManParam->DB1_PATH, "NO")) && (!strcmp(runManParam->DB2_PATH, "NO")))
  	fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function Compare::SetParam(), no db selected");
	
	DBFileReader *pDBFileReader;
	for(i=0; i<m_db.GetNbFile(); i++){
		pDBFileReader = m_db.GetFile(i);

        // statistics
        specStats->protNbInDatabases += pDBFileReader->GetNbEntries();
        
        //check taxonomy and dbs
//        if ((runManParam->TAXID_NB == 0) && pDBFileReader->IsTaxonomyDefined())
//        {
//        	fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "please specify a taxonomy with database");
//        }
//        if ((runManParam->TAXID_NB > 0) && !pDBFileReader->IsTaxonomyDefined())
//        {
//        	fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "used database does not accept taxonomy");
//        }
		//limit search parameters
		pDBFileReader->SetMwRange(MIN_PROT_MASS, MAX_PROT_MASS);
		if (runManParam->TAXID_NB != 0)
			pDBFileReader->SetTaxId(runManParam->TAXID, runManParam->TAXID_NB);
		else
		    pDBFileReader->SetTaxId(NULL, 0);
		}

	//AC list
	if (strlen(runManParam->AC_FILTER) > 4) 
	{
	  char strAC[AC_FILTER_LENTGH];
	  strcpy(strAC,	runManParam->AC_FILTER);

	  for( char *pszAC=strtok(strAC, " "); pszAC; pszAC=strtok(NULL, " "))
	  {
  		for(i=0; i<m_db.GetNbFile(); i++)
			m_db.GetFile(i)->AddAC(pszAC);
	  }
	}

  
	//************************ DIGESTION ************************
	IOParam param;
	param.SetEnzyme(runManParam->ENZYME);
	param.m_iMissedClevage	= runManParam->MISSED;
	param.m_eResolution			= MASS_MONOISOTOPIC;
	param.m_eIonMode				= ION_MODE_M;
	param.m_bPTM						= false;
	
	//param.m_aModif.Add( new IOParamModif("CAM", "CAM", "C", "C2H3ON", 0, 0, 0.3));
  
    m_digest.Load(&param);
  
  
  //  if (tagoData->get_parentMassM() <	MIN_ERR+2) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG,	"Sorry,	parentMass is small; please	look in	compare::SetParam for borning the parentMass");
  //  m_digest.SetPeptideRange(600,	50000);
  
  // PEPTIDE RANGE: ABSOLUTE MIN = 666 (~6aa); ABSOLUTE MAX = 3333 (~30aa);
  double MIN = 666;
  double MAX = 3333;

  // c'est l'erreur sur la masse parente qui détermine le range
	if (!runManParam->m_MUTMOD){

		if ((spectrumData->get_parentMassM() - runManParam->PREC_MASS_ERROR) > 666)  
			MIN = spectrumData->get_parentMassM() - runManParam->PREC_MASS_ERROR;
    
    if ((spectrumData->get_parentMassM() + runManParam->PREC_MASS_ERROR) < 3333) 
			MAX = spectrumData->get_parentMassM() + runManParam->PREC_MASS_ERROR;
    
  }else{

		if ((spectrumData->get_parentMassM() - runManParam->UP_LIMIT_RANGE_PM)  > 666)  
			MIN = spectrumData->get_parentMassM() - runManParam->UP_LIMIT_RANGE_PM;

		if ((spectrumData->get_parentMassM() - runManParam->LOW_LIMIT_RANGE_PM) < 3333) 
			MAX = spectrumData->get_parentMassM() - runManParam->LOW_LIMIT_RANGE_PM;	
  }

	m_digest.Limit(MIN, MAX);

}



// *****************************************************************************



// ********************************************************************************************** //

void Compare::Run(void) 
{
	while( (m_pEntry = m_db.GetNextEntry()) )
	{	
		specStats->protNbInRange++;
		m_digest.Run(m_pEntry->GetSQ(), m_pEntry->GetPtm() );  // rdv dans FindPeptide
	}
  specStats->display(runManParam->FILEOUT);	 
  specStats->displayXML(runManParam->FILEOUTXML);
}



/***********************************************************

   FindPeptide

***********************************************************/
void Compare::FindPeptide(PeptideDigest *pPeptideDigest)
{
	//peptide SQ

	int iPeptideLen = pPeptideDigest->GetEnd() - pPeptideDigest->GetStart() + 1;
	if( (iPeptideLen + 1) >	MAX_LENGHT)	
		ThrowError("Compare::FindPeptide", "Please increase MAX_LENGHT");

	strncpy(m_szPeptideSeq, m_pEntry->GetSQ() + pPeptideDigest->GetStart(), iPeptideLen);
	m_szPeptideSeq[iPeptideLen] = '\0';
/*
	if (!strcmp(m_szPeptideSeq, "LCYVALDFEQEMATAASSSSLEK"))
	{
		cout<<"hello"<<endl;
	}
*/
	//Create new Peptide
	aPeptide = new peptide();			  memCheck.peptide++;

	aPeptide->init(	runManParam,	
									aaParam,
									(float)pPeptideDigest->GetMass(), 
				   					m_szPeptideSeq, 
				   					pPeptideDigest->GetStart(), 
				   					pPeptideDigest->GetEnd(), 
				   					&m_db, 
									FALSE);	



	char str[MAX_LENGHT+1];
	sprintf(str, "%s#", m_szPeptideSeq);

	if(treeWord->search(str)) {
		specResults->addDoublon(aPeptide);
	}
	else {

		treeWord->insert(str, 0.0); //0.0 is unused (fit is a double, for each node-Qmemory)
	/*     
		if (!strcmp(m_szPeptideSeq, spectrumData->get_seqAsInDtb())) 
			printf("hello");
	*/
		// STATS
		allRunStats->trueNeg++;
		specStats->pepCandidate++;
		if (!strcmp(m_szPeptideSeq, spectrumData->get_seqAsInDtb())) 
			specStats->IN_DTB	= true;

		// PROCESS LE PEPTIDE
		processAPeptide();

		// COLLECT LES DONNEES POUR PEPTIDE RANDOMS
		// ON	REPROCESSE LE PEPTIDE APRES	L'AVOIR	RANDOMIZE, ET ON RECOLTE LES SCORES	OBTENUS	DANS fp_ScoresRandom;
		if (runManParam->PVAL_ECHSIZE)	//0	ne randomise pas, sinon	fait le	pour chaque	peptide	theorique présenté
		{
	  		//	  strcpy(pThis->specResults->randPep[pThis->specResults->iRandCount].popiPep.dtbSeq, pThis->aPeptide->popiPep.dtbSeq);
	  		aPeptide->randomize();
	  		processAPeptide();
		}
	}
  
	if(aPeptide){
		delete aPeptide;	
		aPeptide	= NULL;					  memCheck.peptide--;
	}

}



// *****************************************************************************


void Compare::GetProteinInfo( ReloadDBEntry *pReloadDBEntry,	
				  char *pszAC, char	*pszID,	char *pszDE,
				  int *piChainStart, int *piChainEnd, int *piEntryEnd) 
{
 	DBEntry *pEntry = m_db.GetEntry(pReloadDBEntry);	
 
	strcpy(pszAC, pEntry->GetAC());
	strcpy(pszID, pEntry->GetID());
	strcpy(pszDE, pEntry->GetDE());

	*piChainStart = pEntry->GetChildStart();
	*piChainEnd		= pEntry->GetChildEnd();
	*piEntryEnd		= pEntry->GetChildEnd();
}


// ********************************************************************************************** //

void Compare::processAPeptide()
{
  popiTags = new subseq();											 memCheck.subseq++;
  popiTags->init(runManParam, aaParam, spectrumData, ionParam);
   
  tag_extractor* tagExtractor =	new tag_extractor();								 memCheck.tagextractor++;
  tagExtractor->init(runManParam, aaParam, spectrumData, popiGraph, aPeptide, popiTags);
  tagExtractor->getTheTags();

  if (popiTags->subSeqNb > 0)
	{
	  tag_processor* tagProcessor =	new tag_processor();										 memCheck.tagprocessor++; 
	  tagProcessor->init(runManParam, specStats, aaParam, ionParam,	spectrumData, 
			 popiGraph, aPeptide, popiTags, funGen, scoreF, specResults);	 
  
	  tagProcessor->processTags();		   // CONSTRUIT	ET SCORE LES SCENARIOS,	GARDE LE MEILLEUR

	  if (tagProcessor != NULL)	  {delete tagProcessor;	tagProcessor = NULL;					 memCheck.tagprocessor--;}
	}

  if (tagExtractor != NULL)	{delete	tagExtractor; tagExtractor = NULL;							 memCheck.tagextractor--;}
  if (popiTags	   != NULL)	{delete	popiTags;	  popiTags	   = NULL;							 memCheck.subseq--;}
  
}

// ********************************************************************************************** //

void Compare::fillDtbInfos()
{
	specResults->currentElement = specResults->firstElement->following;

	int exemplairesNB =0;
	while ((specResults->currentElement != specResults->lastElement) && (specResults->currentElement->FILLED))
	{
		exemplairesNB = specResults->currentElement->Peptide->exemplairesNb;
		for (int i = 0; i < exemplairesNB; i++)
		{
			GetProteinInfo(&(specResults->currentElement->Peptide->myProt[i]->m_reloadDBEntry), 
			 specResults->currentElement->Peptide->myProt[i]->AC,
			 specResults->currentElement->Peptide->myProt[i]->ID,
			 specResults->currentElement->Peptide->myProt[i]->DE, 
			 &specResults->currentElement->Peptide->iChainStart[i], 
			 &specResults->currentElement->Peptide->iChainEnd[i],
			 &specResults->currentElement->Peptide->iEntryEnd[i]);
		}
		specResults->currentElement = specResults->currentElement->following;
	}
}

// ********************************************************************************************** //

void Compare::computeIDSETstats()
{
  int rank	=		 specResults->giveRankOfCorrectPeptide();
  
  if (rank != -1)	 popiResults->tabRank[rank-1]++;

  specStats->MANQUE	= true;
  if ((specStats->IN_DTB) && (rank == 1))  specStats->MANQUE = FALSE;

  if (!specStats->IN_DTB)				   allRunStats->notInDtb++;
  if ((specStats->IN_DTB) && (rank == 1)) {allRunStats->truePos++;	allRunStats->trueNeg--;} 
  if ((specStats->IN_DTB) && (rank != 1)) {allRunStats->falsePos++;	allRunStats->falseNeg++;}
}

// ********************************************************************************************** //

void Compare::displayIDSETstats(File &fp)
{
  if ((specStats->IN_DTB) && (!specStats->MANQUE)) fprintf(fp, "\nI	GOT	IT!\n");
  else {	  
	if		(!specStats->IN_DTB)  fprintf(fp, "\nI MISSED IT, BUT IT WAS NOT IN	DTB\n");
	else if	( specStats->MANQUE)  fprintf(fp, "\nI REALLY MISSED IT\n");
  }
  fprintf(fp, "SPECTRUM	SEQUENCE WAS %s\n",	spectrumData->get_seqSpec());
  fprintf(fp, "DATABASE	SEQUENCE WAS %s\n",	spectrumData->get_seqAsInDtb());
}

// ********************************************************************************************** //


void Compare::EndRun(int ID)
{
  fillDtbInfos();
  specResults->putRanks();
  specResults->computePValuesNEG();
  specResults->computePValuesRANDOM();
  //  specResults->writeSimple(runManParam->FILEOUT);
  specResults->write(runManParam->FILEOUT);
  specResults->writeXML(runManParam->FILEOUTXML);
  specResults->writeShort(runManParam->FILEOUTSHORT);

  if (runManParam->PLOT)	   {specResults->createPopScoresNEG(ID);
								specResults->createPopScoresRANDOM(ID);}

  if (runManParam->s_IDSET)	   {computeIDSETstats();
								displayIDSETstats(runManParam->FILEOUT);}  
    
  if (runManParam->r_NORMAL)	popiResults->addToProtList(specResults,	ID);

  if (runManParam->r_FUN)		{funGen->endRun(ID);}


	m_db.CloseAll();
}

 //	********************************************************************************************** //
