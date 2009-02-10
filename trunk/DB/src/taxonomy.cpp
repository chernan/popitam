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

/***********************************************************

	Company            : Swiss Intitute of Bioinformatics
	Author             : Marc Tuloup
	Contact            : Marc.Tuloup@isb-sib.ch
	Created on         : 28.11.2003
  Last modification  : 08.02.2006
	Comments           : 

***********************************************************/

#include "taxonomy.h"
#include "tagfile.h"
#include "MATerror.h"
#include "util.h"
#include "dynamicarray.h"
#include "dbfile.h"
#include "errno.h"


#define MAX_LINE_LEN		1024


/******************************************************

		Constructor

*******************************************************/
Taxonomy::Taxonomy(void) :
	m_pRoot(NULL),
	m_iHighestId(0),
	m_bDefined(false)
{
}


/******************************************************

		Destructor

*******************************************************/
Taxonomy::~Taxonomy(void)
{
	if(m_pRoot){
		delete m_pRoot;
		m_pRoot = NULL;
	}		
}


/***********************************************************

	Load

***********************************************************/

void Taxonomy::Load(const char *pszNCBIFileName)
{
	DynamicArray<Taxon *> aTaxon;	//tmp


	if(pszNCBIFileName){
		LoadNCBI(pszNCBIFileName, aTaxon);	//use the taxonomy file
		m_bDefined = true;
	}else{
		aTaxon.Add(new Taxon(1, 0));				//no taxonomy => one unique taxon root id=1 parentId=0
		m_iHighestId = 1;
		m_bDefined = false;
	}

	SetTaxId(aTaxon);
	Link(aTaxon);
}



/***********************************************************

	LoadNCBI

***********************************************************/

void Taxonomy::LoadNCBI(const char *pszFileName, DynamicArray<Taxon *> &aTaxon)
{
	File	file;
	file.Open(pszFileName, "r");


	char	szLine[MAX_LINE_LEN];
	int		iId, iParentId;

	char	*psz;

	
	while(fgets(szLine, MAX_LINE_LEN, file)){	//iId  | iParentId | ...
		

		if(	!( psz = strtok(szLine, "|"))					||
				(sscanf(psz, "%d", &iId) != 1)				||
				!(psz = strtok(NULL, "|"))					||
				(sscanf(psz, "%d", &iParentId) != 1)
			)
			ThrowError("Taxonomy::LoadAllTaxon", "NCBI nodes.dmp bad format");
		

		aTaxon.Add(new Taxon(iId, iParentId));

		//recupere iId le plus eleve
		if( iId > m_iHighestId)
			m_iHighestId = iId;

	}

	file.Close();	
}



/***********************************************************

	SetTaxId

***********************************************************/

void Taxonomy::SetTaxId(DynamicArray<Taxon *> &aTaxon)
{
	m_aTaxId.SetNbElt(m_iHighestId +1);
	m_aTaxId.ZeroMem();

	//fait le lien entre iId et pTaxon
	for(int i=0; i<aTaxon.GetNbElt(); i++)
		m_aTaxId[aTaxon[i]->GetId()] = aTaxon[i];
}



/***********************************************************

	Link

***********************************************************/

void Taxonomy::Link(DynamicArray<Taxon *> &aTaxon)
{
	Taxon *pParentTaxon;

	//root de NCBI a pour taxid 1
	m_pRoot	= m_aTaxId[1];	
	m_pRoot->m_iParentId = 0;	//root n'a pas de parent

	//accroche tous les taxons a leur parents
	for(int i=0; i<aTaxon.GetNbElt(); i++){
		if(aTaxon[i]->GetParentId() != 0){	
			if(!(pParentTaxon = m_aTaxId[aTaxon[i]->GetParentId()]) )
				ThrowError("Taxonomy::Link", "Taxid %d has an undefined parent %d", aTaxon[i]->GetId(), aTaxon[i]->GetParentId());
			pParentTaxon->AddChild(aTaxon[i]);
		}
	}
}




/***********************************************************

	GetTaxon

***********************************************************/

Taxon *Taxonomy::GetTaxon(int iTaxId)
{
	if( (iTaxId < 0) || (iTaxId > m_iHighestId) || (!m_aTaxId[iTaxId]) )
		return m_pRoot;		//le taxid n'existe pas renvoie root comme taxon unclassified
	
	return m_aTaxId[iTaxId];
}



/***********************************************************

   WriteTaxIdOffset

***********************************************************/
void Taxonomy::WriteTaxIdOffset(File &file)
{
	if(!m_bDefined)
		return;

	Taxon						*pTaxon;	
	TS_OffsetRange	stOffset;

	for(int i=0; i<=m_iHighestId; i++){

		pTaxon = m_aTaxId[i];

		if(pTaxon){
			stOffset.uiStart	= pTaxon->GetOffsetStart();
			stOffset.uiEnd		= pTaxon->GetOffsetEnd();
		}else{
			stOffset.uiStart	= 0;
			stOffset.uiEnd		= 0;
		}

		fwrite(&stOffset, sizeof(TS_OffsetRange), 1, file);
	}
}
