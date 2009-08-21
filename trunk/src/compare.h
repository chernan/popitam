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

#ifndef __COMPARE_H__
#define __COMPARE_H__

// ********************************************************************************************** //

#include "defines.h"
#include "runManagerParameters.h"
#include "aa.h"
#include "ion.h"
#include "data.h"
#include "peptide.h"
#include "graphe.h"
#include "spectrumstatistics.h"
#include "allrunstatistics.h"
#include "ants.h"
#include "subseq.h"
#include "gnb.h"
#include "sufftab.h"
#include "gptree.h"
#include "specresults.h"
#include "result.h"
#include "fun.h"
#include "Tree.h"
#include "dbreader.h"
#include "digest.h"

// ********************************************************************************************** //

class Compare {

public :
	runManagerParameters   *runManParam;
	allrunstatistics       *allRunStats;
	TREE                   *scoreF;
	aa                     *aaParam;
	ion                    *ionParam;
	result                 *popiResults;
	spectrumstatistics     *specStats;
	data                   *spectrumData;
	graphe                 *popiGraph;
	subseq                 *popiTags;
	fun                    *funGen;
	specresults            *specResults;
	TreeWord               *treeWord;
	TreeWord               *ACtree;
	int                     randomEchNb;

	Compare(void);
	~Compare(void);
	
	void init_DIG();
	void Run(Digest* digest);
	void EndRun(int);
	void init_POP(runManagerParameters*, TREE*, aa*, ion*, ion*, ion*, ion*, data*, result*, allrunstatistics*);
	void FindPeptide(PeptideDigest *pPeptideDigest);
	void test();
	
private :
	DBEntry *currentProtein;	
	DBReader m_db;
	char m_szPeptideSeq[MAX_LENGHT];
	unsigned long m_ulOffset2;
	int m_iDataBaseId;
	int m_iTaxonId;

	void loadSpectrum(int);
	void buildGraph();
	void processAPeptide(peptide *pep);
	/* void fillDtbInfos(); */
	void computeIDSETstats();
	void displayIDSETstats(File&);
	void DisplayUnusedAC(File&);
	/*
	void GetProteinInfo(ReloadDBEntry *pReloadDBEntry, char *pszAC, char	*pszID,	char *pszDE,
	                    int *piChainStart, int *piChainEnd, int *piEntryEnd);
	*/
};

#endif
