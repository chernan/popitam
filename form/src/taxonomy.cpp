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

	Company			   : Swiss Intitute	of Bioinformatics
	Author			   : Marc Tuloup
	Contact			   : Marc.Tuloup@isb-sib.ch
	Created	on		   : 28.11.2003
	Last modification  : 15.09.2004
	Comments		   : 

***********************************************************/

#define	USE_TAXONOMY
#define	USE_FILE
#define	USE_UTIL
#define	USE_MENU
#define	USE_MSG
#include "use.h"



const char *Taxonomy::pszTaxonTag	= "taxonomy";
const char *Taxonomy::pszTaxonomyTxt   = "taxonomy.txt";
const char *Taxonomy::pszFindTaxId = "findTaxid.txt";

/******************************************************

		Constructor

*******************************************************/
Taxonomy::Taxonomy(void) :
	m_iNbTaxId(0),
	m_ptstTaxId(NULL),
	m_bTaxIdOk(false)
{
//	  m_pTaxId = new TextField("onlyTaxid");
}


/******************************************************

		Destructor

*******************************************************/
Taxonomy::~Taxonomy(void)
{
	if(m_ptstTaxId){
		free(m_ptstTaxId);
		m_ptstTaxId	= NULL;
	}   
}

/***********************************************************

	Reset

***********************************************************/

void Taxonomy::Reset(void)
{
	m_root.Reset();
}


/**********************************************************

		Load

rempli l'arbre des Taxons.
**********************************************************/

void Taxonomy::Load(void)
{
	//taxon
	TagFile	file;
	char buffer[256];
	strcpy(buffer, path()->pszTaxonomy);
	strcat(buffer, pszTaxonomyTxt);
	file.Open(buffer, "r");

	m_root.Read(&file);

	file.Close();

	//taxid
//	  m_pTaxId->Load(FLOOR|CEILING|EMPTY, 0, 100);
}


/***********************************************************

	ReadCgi

***********************************************************/

void Taxonomy::ReadCgi(Cgi *pCgi)
{
	int			iNbItem;
	int			iId;
	const char	*psz;
	Taxon		*pTaxon;

	    
	iNbItem	= pCgi->GetNbElt(pszTaxonTag);


	for(int	i=0; i<iNbItem;	i++){
		psz	= pCgi->GetStringPtr(pszTaxonTag, i);   


		if(!( (sscanf(psz, "%d", &iId) == 1) &&	(psz = strchr(psz, '|')) ))
			ThrowError("Taxonomy::ReadCgi",	"Taxon not formated	like : \"int | string\"");

		psz++;

		pTaxon = m_root.Find(iId);	    

		if(!pTaxon)
			ThrowError("Taxonomy::ReadCgi",	"No	Taxon id %d	string \"%s\"",	iId, psz);
	    
		pTaxon->Select(true);
	}


	//only
//	  m_pTaxId->ReadCgi(pCgi);
}



/***********************************************************

	CheckCgi

***********************************************************/

void Taxonomy::CheckCgi(void)
{
	//recupere les taxId de	la string Only
	DynamicArray<TS_TaxId> taxIdArray;
	taxIdArray.Alloc(5);

//	  TS_TaxId *pstTaxId;

	m_bTaxIdOk = false;

/*	  if( !m_pTaxId->IsWhite() ){
    
		char	*pszTaxId	= Strdup(m_pTaxId->GetString());
		char	*psz		= strtok(pszTaxId, "; ");
		int		iId;


		do{
			if(	sscanf(psz,	"%d", &iId)	!= 1)
				ThrowMsg(*PROTEIN_TAB,*	"Specific taxId	list is	bad	formated");
	    
			pstTaxId			= taxIdArray.Add();
			pstTaxId->iTaxId	= iId;
			pstTaxId->pTaxon	= GetOwnerTaxon(iId);

		}while(	(psz = strtok(NULL,	"; ")) && *psz);

		free(pszTaxId);


		m_iNbTaxId	= taxIdArray.GetNbElt();
		m_ptstTaxId	= taxIdArray.GetWithAjust();

	}else{
*/		  m_iNbTaxId = 0;
//	  }

	m_bTaxIdOk = true;


	if(	!m_root.GetNbSelectedTaxon() &&	!m_iNbTaxId)
		ThrowMsg(/*PROTEIN_TAB,*/ "No taxon	and	no specific	taxid selected");

}

/***********************************************************

	GetOwnerTaxon

***********************************************************/

Taxon *Taxonomy::GetOwnerTaxon(int iId)
{
	File file;
	char buffer[256];
	strcpy(buffer, path()->pszTaxonomy);
	strcat(buffer, pszFindTaxId);
	file.Open(buffer, "r");

	int	iTaxId,	iTaxIdOwner;

	while(fscanf(file, "%d %d",	&iTaxId, &iTaxIdOwner) == 2){

		if(iTaxId == iId){
			file.Close();

			Taxon *pTaxon =	m_root.Find(iTaxIdOwner);

			if(!pTaxon)
				ThrowError("Taxonomy::GetOwnerTaxon", "Unable to find taxon	id=\"%d\" in tree",	iId);

			return pTaxon;
		}
	}

	file.Close();

	ThrowMsg(/*PROTEIN_TAB,*/ "Unknown specific	TaxId :	%d", iId);

	return 0;
}


/***********************************************************

	ReadFile

***********************************************************/

void Taxonomy::ReadFile(TagFile	*pFile)
{
	int			iNbItem;
	int			iTaxonId;
	char		*psz;
	int			iTaxId;
	Taxon		*pTaxon;
	TS_TaxId	*pstTaxId;


	DynamicArray<TS_TaxId> taxIdArray;
	taxIdArray.Alloc(5);


	Reset();


	pFile->ReadStartObject("Taxonomy");

	pFile->ReadMember("iNbItem", &iNbItem);


	for(int	i=0; i<iNbItem;	i++){

		pFile->ReadStartObject("Item");
		pFile->ReadMember("id",		 &iTaxonId);
		pFile->ReadMember("string",	 &psz);
		pFile->ReadMember("only",	 &iTaxId);
		pFile->ReadEndObject();


		if(iTaxId == 0){

			if(!(pTaxon	= m_root.Find(iTaxonId)))
				ThrowError("Taxonomy::ReadFile", "No taxon with	id=\"%d\"",	iTaxonId);

			if(strcmp(psz, pTaxon->GetString())	)
				ThrowError("Taxonomy::ReadFile", "Taxon	id=\"%d\" should be	string=\"%s\" and not \"%s\"",
				iTaxonId, pTaxon->GetString(), psz);

			pTaxon->Select(true);

		}else{

			pstTaxId			= taxIdArray.Add();
			pstTaxId->iTaxId	= iTaxId;
			pstTaxId->pTaxon	= GetOwnerTaxon(iTaxId);
		}

		free(psz);
	}

	pFile->ReadEndObject();

	m_iNbTaxId	= taxIdArray.GetNbElt();
	m_ptstTaxId	= taxIdArray.GetWithAjust();

	m_bTaxIdOk = true;
}

/***********************************************************

	WriteFile

***********************************************************/

void Taxonomy::WriteFile(TagFile *pFile)
{
	pFile->WriteStartObject("Taxonomy");


	int	iNbTaxon = m_root.GetNbSelectedTaxon();


	//somme	des	taxons et taxid
	int	iNbItem	= iNbTaxon + m_iNbTaxId;


	//taxon
	pFile->WriteMember("iNbItem", iNbItem);
	m_root.WriteFile(pFile);


	//taxid
	for(int	i=0; i<m_iNbTaxId; i++){
		pFile->WriteStartObject("Item");
		pFile->WriteMember("id",	 m_ptstTaxId[i].pTaxon->GetId());
		pFile->WriteMember("string", m_ptstTaxId[i].pTaxon->GetString());
		pFile->WriteMember("only",	 m_ptstTaxId[i].iTaxId);
		pFile->WriteEndObject();
	}

    
	pFile->WriteEndObject();	// iotag()->pszTaxonList
}


/***********************************************************

	WriteContaminantParam

***********************************************************/

void Taxonomy::WriteContaminantParam(TagFile *pFile)
{
	pFile->WriteStartObject("Taxonomy");


	Taxon *pTaxon =	m_root.Find("Contaminant");

	if(!pTaxon)
		ThrowError("Taxonomy::WriteContaminantParam", "Taxon Contaminant does not exist");

    
	pFile->WriteMember("iNbItem", (int)1);

	pTaxon->WriteFileAlone(pFile);

	pFile->WriteEndObject();
}


/***********************************************************

	WriteTaxon

***********************************************************/

void Taxonomy::WriteTaxon(File &file)
{
	fprintf(file, "<select name=\"%s\" size=7>\n", pszTaxonTag);

	Taxon::iDeepInTree = 0;
//	m_root.Find(1)->WriteChildren(file);	//m_root ne	correspond pas a All (taxId	= 1)
	m_root.Find(1)->Write(file);	//m_root ne	correspond pas a All (taxId	= 1)
	fprintf(file, "</select>\n");
}


/***********************************************************

	WriteTaxId

***********************************************************/

/*void Taxonomy::WriteTaxId(File &file)
{
	int	iSize =	34;

	if(m_bTaxIdOk){

		fprintf(file, "<input name=\"%s\" size=%d value=\"", m_pTaxId->GetName(), iSize);
	    
		for(int	i=0; i<m_iNbTaxId; i++)
			fprintf(file, "%d; ", m_ptstTaxId[i].iTaxId);
	    
		fprintf(file, "\">\n");

	}else{
		m_pTaxId->Write(file, iSize);
	}
}
*/



/***********************************************************

	Hide

***********************************************************/

void Taxonomy::Hide(File &file)
{
	//taxon
	m_root.Hide(file, pszTaxonTag);
    
	//taxid
/*	  if(m_bTaxIdOk){

		fprintf(file, "<input type=hidden name=\"%s\" value=\"", m_pTaxId->GetName());

		for(int	i=0; i<m_iNbTaxId; i++)
			fprintf(file, "%d; ", m_ptstTaxId[i].iTaxId);

		fprintf(file, "\">\n");

	}else{
		m_pTaxId->Hide(file);
	}*/
}


/***********************************************************

	WriteCheckLimit

***********************************************************/

void Taxonomy::WriteCheckLimit(File	&file)
{
//	  m_pTaxId->WriteCheckLimit(file, "TaxID(s)");
}

/***********************************************************

	WriteHtmlInputSummary

***********************************************************/

void Taxonomy::WriteHtmlInputSummary(File &file, bool bLight)
{
	fprintf(file, "<tr><td class=title>Taxon(s)</td><td	class=left>");
	m_root.WriteHtmlInputSummary(file);
	fprintf(file, "</td></tr>");
//	  fprintf(file,	"</td></tr><tr><td class=title>Specific	TaxID(s)</td><td class=left>");

/*	  for(int i=0; i<m_iNbTaxId; i++)
		fprintf(file, "%d; ", m_ptstTaxId[i].iTaxId);

	fprintf(file, "</td></tr>");*/
	(bLight)? fprintf(file,	"\n") :	fprintf(file, "\\\n");
}



