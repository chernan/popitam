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
	Created on         : 02.01.2007
	Last modification  : 02.01.2007
	Comments           : 

***********************************************************/



#include "ioparam.h"
#include "ioversion.h"
#include "tagfile.h"
#include "util.h"
#include "MATerror.h"
#include "atomicmass.h"


const char *pszIonMode[NB_ION_MODE] = { "[M]", "[M+H]", "[M-H]"  };
const char *pszModifMode[2]					= {"Fixed", "Variable"};

//variable de classe
const int IOParam::iVersion = 3;


/******************************************************

		Constructor

*******************************************************/
IOParam::IOParam(void) :
	m_pszEnzyme(NULL)
{
}

/******************************************************

		Destructor

*******************************************************/
IOParam::~IOParam(void)
{
	if(m_pszEnzyme){
		delete m_pszEnzyme;
		m_pszEnzyme = NULL;
	}
  
	m_aDB.DeletePtrs();
	m_aModif.DeletePtrs();
	m_aAC.DeletePtrs();
}



/***********************************************************

   SetEnzyme

***********************************************************/
void IOParam::SetEnzyme(const char *psz)
{
	if(m_pszEnzyme){
		delete m_pszEnzyme;
		m_pszEnzyme = NULL;
	}

	m_pszEnzyme = Strdup(psz);
}

/***********************************************************

   Read

***********************************************************/
void IOParam::Read(const char *pszFileName)
{
	TagFile file;
	file.Open(pszFileName, "r");

	//version
	IOVersion version;
	version.Read(file);

	if(version.GetId() != IOParam::iVersion){

		if(version.GetId() == 1){

			ReadVersion1(file);
			file.Close();

			//convert to curent version
			Write(pszFileName);

		}else if(version.GetId() == 2){
		
			ReadVersion2(file);
			file.Close();

			//convert to curent version
			Write(pszFileName);

		}else
			ThrowError(	"IOParam::Read", "File \"%s\" version %d, is not supported", pszFileName, version.GetId());

	}else{
		Read(file);
		file.Close();
	}
}



/***********************************************************

   ReadVersion1

 ***********************************************************/
void IOParam::ReadVersion1(TagFile &file)
{
	int i, iNbItem, iValue;

	file.ReadStartObject("Param");

	//PROTEIN
	file.ReadStartObject("Protein");

		//DB
		file.ReadStartObject("DBFileList");
		file.ReadMember("NbItem", &iNbItem);	
		m_aDB.EnsureSize(iNbItem);
		IOParamDB *pIOParamDB;
		for(i=0; i<iNbItem; i++){
			pIOParamDB = new IOParamDB;
			m_aDB.Add(pIOParamDB);
			pIOParamDB->ReadVersion1And2(file);
		}
		file.ReadEndObject();	//DBFileList

		//Taxonomy
		file.ReadStartObject("TaxId");
		file.ReadMember("NbItem", &iNbItem);
		m_aTaxId.EnsureSize(iNbItem);

		int iTaxId;
		file.ReadArrayStart();
		for(i=0; i<iNbItem; i++){
			if(fscanf(file, "%d", &iTaxId) != 1)
				ThrowError("IOParam::Read", "Unable to read TaxId");
			m_aTaxId.Add(iTaxId);
		}
		file.ReadArrayEnd();
		file.ReadEndObject();	//TaxId

		//Filters
		file.ReadStartObject("Filters");
		file.ReadMember("MwMin", 					&m_iMwMin);
		file.ReadMember("MwMax", 					&m_iMwMax);
		file.ReadMember("PiMin", 					&m_dPiMin);
		file.ReadMember("PiMax", 					&m_dPiMax);
		file.ReadMember("UseVarsplices",	&m_bVarsplices);
		file.ReadMember("UseFragments",		&m_bFragments);

			file.ReadStartObject("Keyword");
			file.ReadMember("Mode",		&iValue);
			m_eKwMode = (TE_KwMode)iValue;
			file.ReadMember("NbItem", &iNbItem);
			m_aKW.EnsureSize(iNbItem);

			int iKW;
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%d", &iKW) != 1)
					ThrowError("IOParam::Read", "Unable to read Keyword");
				m_aKW.Add(iKW);
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//Keyword

			file.ReadStartObject("AC");
			file.ReadMember("NbItem",	&iNbItem);
			m_aAC.EnsureSize(iNbItem);

			char szAC[128];
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%s", szAC) != 1)
					ThrowError("IOParam::Read", "Unable to read AC");
				m_aAC.Add(Strdup(szAC));
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//AC

		file.ReadEndObject();	//Filters

	file.ReadEndObject();	//Protein


	//PEPTIDE
	file.ReadStartObject("Peptide");
	file.ReadMember("Enzyme",					&m_pszEnzyme);
	file.ReadMember("MissedCleavage",	&m_iMissedClevage);
	file.ReadMember("Resolution",			&iValue);
	m_eResolution = (TE_MassMode)iValue;
	file.ReadMember("IonMode",				&iValue);
	m_eIonMode = (TE_IonMode)iValue;
	file.ReadMember("Ptm",						&m_bPTM);

	file.ReadStartObject("ModifList");
	file.ReadMember("NbItem", &iNbItem);
	m_aModif.EnsureSize(iNbItem);
	IOParamModif *pModif;
	for(i=0; i<iNbItem; i++){
		pModif = new IOParamModif;
		m_aModif.Add(pModif);
		pModif->Read(file);
	}

	file.ReadEndObject();	//ModifList

	file.ReadEndObject();	//Peptide


	//THRESHOLDS
	file.ReadStartObject("Thresholds");
	file.ReadMember("Shift",		&m_dShift);
	file.ReadMember("Slope",		&m_iSlope);
	file.ReadMember("Error",		&m_iError);
	file.ReadMember("HitsMin",	&m_iHitsMin);
	file.ReadEndObject();	//Thresholds


	//SCORING
	bool bRandom;
	file.ReadStartObject("Scoring");
	file.ReadMember("Random", &bRandom);
	file.ReadEndObject();	//Scoring

	//OUTPUT
	file.ReadStartObject("Output");
	file.ReadMember("nbMaxProtein", &m_iDisplayMax);
	file.ReadEndObject();	//Output

	file.ReadEndObject();	//Param
}



/***********************************************************

   ReadVersion2

 ***********************************************************/
void IOParam::ReadVersion2(TagFile &file)
{
	int i, iNbItem, iValue;

	file.ReadStartObject("Param");

	//PROTEIN
	file.ReadStartObject("Protein");

		//DB
		file.ReadStartObject("DBFileList");
		file.ReadMember("NbItem", &iNbItem);	
		m_aDB.EnsureSize(iNbItem);
		IOParamDB *pIOParamDB;
		for(i=0; i<iNbItem; i++){
			pIOParamDB = new IOParamDB;
			m_aDB.Add(pIOParamDB);
			pIOParamDB->ReadVersion1And2(file);
		}
		file.ReadEndObject();	//DBFileList

		//Taxonomy
		file.ReadStartObject("TaxId");
		file.ReadMember("NbItem", &iNbItem);
		m_aTaxId.EnsureSize(iNbItem);

		int iTaxId;
		file.ReadArrayStart();
		for(i=0; i<iNbItem; i++){
			if(fscanf(file, "%d", &iTaxId) != 1)
				ThrowError("IOParam::Read", "Unable to read TaxId");
			m_aTaxId.Add(iTaxId);
		}
		file.ReadArrayEnd();
		file.ReadEndObject();	//TaxId

		//Filters
		file.ReadStartObject("Filters");
		file.ReadMember("MwMin", 					&m_iMwMin);
		file.ReadMember("MwMax", 					&m_iMwMax);
		file.ReadMember("PiMin", 					&m_dPiMin);
		file.ReadMember("PiMax", 					&m_dPiMax);
		file.ReadMember("UseVarsplices",	&m_bVarsplices);
		file.ReadMember("UseFragments",		&m_bFragments);

			file.ReadStartObject("Keyword");
			file.ReadMember("Mode",		&iValue);
			m_eKwMode = (TE_KwMode)iValue;
			file.ReadMember("NbItem", &iNbItem);
			m_aKW.EnsureSize(iNbItem);

			int iKW;
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%d", &iKW) != 1)
					ThrowError("IOParam::Read", "Unable to read Keyword");
				m_aKW.Add(iKW);
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//Keyword

			file.ReadStartObject("AC");
			file.ReadMember("NbItem",	&iNbItem);
			m_aAC.EnsureSize(iNbItem);

			char szAC[128];
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%s", szAC) != 1)
					ThrowError("IOParam::Read", "Unable to read AC");
				m_aAC.Add(Strdup(szAC));
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//AC

		file.ReadEndObject();	//Filters

	file.ReadEndObject();	//Protein


	//PEPTIDE
	file.ReadStartObject("Peptide");
	file.ReadMember("Enzyme",					&m_pszEnzyme);
	file.ReadMember("MissedCleavage",	&m_iMissedClevage);
	file.ReadMember("Resolution",			&iValue);
	m_eResolution = (TE_MassMode)iValue;
	file.ReadMember("IonMode",				&iValue);
	m_eIonMode = (TE_IonMode)iValue;
	file.ReadMember("Ptm",						&m_bPTM);

	file.ReadStartObject("ModifList");
	file.ReadMember("NbItem", &iNbItem);
	m_aModif.EnsureSize(iNbItem);
	IOParamModif *pModif;
	for(i=0; i<iNbItem; i++){
		pModif = new IOParamModif;
		m_aModif.Add(pModif);
		pModif->Read(file);
	}

	file.ReadEndObject();	//ModifList

	file.ReadEndObject();	//Peptide


	//THRESHOLDS
	file.ReadStartObject("Thresholds");
	file.ReadMember("Shift",		&m_dShift);
	file.ReadMember("Slope",		&m_iSlope);
	file.ReadMember("Error",		&m_iError);
	file.ReadMember("HitsMin",	&m_iHitsMin);
	file.ReadEndObject();	//Thresholds

	//OUTPUT
	file.ReadStartObject("Output");
	file.ReadMember("nbMaxProtein", &m_iDisplayMax);
	file.ReadEndObject();	//Output

	file.ReadEndObject();	//Param
}

/***********************************************************

   Read

 ***********************************************************/
void IOParam::Read(TagFile &file)
{
	int i, iNbItem, iValue;

	file.ReadStartObject("Param");

	//PROTEIN
	file.ReadStartObject("Protein");

		//DB
		file.ReadStartObject("DBFileList");
		file.ReadMember("NbItem", &iNbItem);	
		m_aDB.EnsureSize(iNbItem);
		IOParamDB *pIOParamDB;
		for(i=0; i<iNbItem; i++){
			pIOParamDB = new IOParamDB;
			m_aDB.Add(pIOParamDB);
			pIOParamDB->Read(file);
		}
		file.ReadEndObject();	//DBFileList

		//Taxonomy
		file.ReadStartObject("TaxId");
		file.ReadMember("NbItem", &iNbItem);
		m_aTaxId.EnsureSize(iNbItem);

		int iTaxId;
		file.ReadArrayStart();
		for(i=0; i<iNbItem; i++){
			if(fscanf(file, "%d", &iTaxId) != 1)
				ThrowError("IOParam::Read", "Unable to read TaxId");
			m_aTaxId.Add(iTaxId);
		}
		file.ReadArrayEnd();
		file.ReadEndObject();	//TaxId

		//Filters
		file.ReadStartObject("Filters");
		file.ReadMember("MwMin", 					&m_iMwMin);
		file.ReadMember("MwMax", 					&m_iMwMax);
		file.ReadMember("PiMin", 					&m_dPiMin);
		file.ReadMember("PiMax", 					&m_dPiMax);
		file.ReadMember("UseVarsplices",	&m_bVarsplices);
		file.ReadMember("UseFragments",		&m_bFragments);

			file.ReadStartObject("Keyword");
			file.ReadMember("Mode",		&iValue);
			m_eKwMode = (TE_KwMode)iValue;
			file.ReadMember("NbItem", &iNbItem);
			m_aKW.EnsureSize(iNbItem);

			int iKW;
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%d", &iKW) != 1)
					ThrowError("IOParam::Read", "Unable to read Keyword");
				m_aKW.Add(iKW);
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//Keyword

			file.ReadStartObject("AC");
			file.ReadMember("NbItem",	&iNbItem);
			m_aAC.EnsureSize(iNbItem);

			char szAC[128];
			file.ReadArrayStart();
			for(i=0; i<iNbItem; i++){
				if(fscanf(file, "%s", szAC) != 1)
					ThrowError("IOParam::Read", "Unable to read AC");
				m_aAC.Add(Strdup(szAC));
			}
			file.ReadArrayEnd();
			file.ReadEndObject();	//AC

		file.ReadEndObject();	//Filters

	file.ReadEndObject();	//Protein


	//PEPTIDE
	file.ReadStartObject("Peptide");
	file.ReadMember("Enzyme",					&m_pszEnzyme);
	file.ReadMember("MissedCleavage",	&m_iMissedClevage);
	file.ReadMember("Resolution",			&iValue);
	m_eResolution = (TE_MassMode)iValue;
	file.ReadMember("IonMode",				&iValue);
	m_eIonMode = (TE_IonMode)iValue;
	file.ReadMember("Ptm",						&m_bPTM);

	file.ReadStartObject("ModifList");
	file.ReadMember("NbItem", &iNbItem);
	m_aModif.EnsureSize(iNbItem);
	IOParamModif *pModif;
	for(i=0; i<iNbItem; i++){
		pModif = new IOParamModif;
		m_aModif.Add(pModif);
		pModif->Read(file);
	}

	file.ReadEndObject();	//ModifList

	file.ReadEndObject();	//Peptide


	//THRESHOLDS
	file.ReadStartObject("Thresholds");
	file.ReadMember("Shift",		&m_dShift);
	file.ReadMember("Slope",		&m_iSlope);
	file.ReadMember("Error",		&m_iError);
	file.ReadMember("HitsMin",	&m_iHitsMin);
	file.ReadEndObject();	//Thresholds

	//OUTPUT
	file.ReadStartObject("Output");
	file.ReadMember("nbMaxProtein", &m_iDisplayMax);
	file.ReadEndObject();	//Output

	file.ReadEndObject();	//Param
}

/***********************************************************

   Write

***********************************************************/
void IOParam::Write(const char *pszFileName)
{
	int i;
	TagFile file;
	file.Open(pszFileName, "w");

	//version
	IOVersion version(IOParam::iVersion);
	version.Write(file);

	file.WriteStartObject("Param");

	//PROTEIN
	file.WriteStartObject("Protein");

		//DB
		file.WriteStartObject("DBFileList");
		file.WriteMember("NbItem", m_aDB.GetNbElt());
		for(i=0; i<m_aDB.GetNbElt(); i++)
			m_aDB[i]->Write(file);	
		file.WriteEndObject();	//DBFileList

		//Taxonomy
		file.WriteStartObject("TaxId");
		file.WriteMember("NbItem", m_aTaxId.GetNbElt());
		file.WriteArrayStart();
		for(i=0; i<m_aTaxId.GetNbElt(); i++)
			fprintf(file, "%d ", m_aTaxId[i]);
		file.WriteArrayEnd();
		file.WriteEndObject();	//TaxId

		//Filters
		file.WriteStartObject("Filters");
		file.WriteMember("MwMin", m_iMwMin);
		file.WriteMember("MwMax", m_iMwMax);
		file.WriteMember("PiMin", m_dPiMin);
		file.WriteMember("PiMax", m_dPiMax);
		file.WriteMember("UseVarsplices", m_bVarsplices);
		file.WriteMember("UseFragments",	m_bFragments);

			file.WriteStartObject("Keyword");
			file.WriteMember("Mode", (int)m_eKwMode);
			file.WriteMember("NbItem", m_aKW.GetNbElt());
			file.WriteArrayStart();
			for(i=0; i<m_aKW.GetNbElt(); i++)
				fprintf(file, "%d ", m_aKW[i]);
			file.WriteArrayEnd();
			file.WriteEndObject();	//Keyword

			file.WriteStartObject("AC");
			file.WriteMember("NbItem", m_aAC.GetNbElt());
			file.WriteArrayStart();
			for(i=0; i<m_aAC.GetNbElt(); i++)
				fprintf(file, "%s ", m_aAC[i]);
			file.WriteArrayEnd();
			file.WriteEndObject();	//AC

		file.WriteEndObject();	//Filters

	file.WriteEndObject();	//Protein


	//PEPTIDE
	file.WriteStartObject("Peptide");
	file.WriteMember("Enzyme",					m_pszEnzyme);
	file.WriteMember("MissedCleavage",	m_iMissedClevage);
	file.WriteMember("Resolution",			(int)m_eResolution);
	file.WriteMember("IonMode",					(int)m_eIonMode);
	file.WriteMember("Ptm",							m_bPTM);

	file.WriteStartObject("ModifList");
	file.WriteMember("NbItem", m_aModif.GetNbElt());
	for(i=0; i< m_aModif.GetNbElt(); i++)
		m_aModif[i]->Write(file);
	file.WriteEndObject();	//ModifList

	file.WriteEndObject();	//Peptide


	//THRESHOLDS
	file.WriteStartObject("Thresholds");
	file.WriteMember("Shift",		m_dShift);
	file.WriteMember("Slope",		m_iSlope);
	file.WriteMember("Error",		m_iError);
	file.WriteMember("HitsMin", m_iHitsMin);
	file.WriteEndObject();	//Thresholds


	//OUTPUT
	file.WriteStartObject("Output");
	file.WriteMember("nbMaxProtein", m_iDisplayMax);
	file.WriteEndObject();	//Output

	file.WriteEndObject();	//Param


	file.Close();
}

/***********************************************************

   Copy

***********************************************************/
void IOParam::Copy(IOParam *pIOParam)
{
	CopyArray(pIOParam->m_aDB, m_aDB);
	CopyArray(pIOParam->m_aModif, m_aModif);
	
	m_aTaxId.Copy(pIOParam->m_aTaxId);

	m_aKW.Copy(pIOParam->m_aKW);
	m_eKwMode = pIOParam->m_eKwMode;

	m_aAC.DeletePtrs();
	m_aAC.EnsureSize(pIOParam->m_aAC.GetNbElt());
	for(int i=0; i<pIOParam->m_aAC.GetNbElt(); i++)
		m_aAC.Add(Strdup(pIOParam->m_aAC[i]));

	SetEnzyme(pIOParam->m_pszEnzyme);

	m_iMwMin					= pIOParam->m_iMwMin;
	m_iMwMax					= pIOParam->m_iMwMax;
	m_dPiMin					= pIOParam->m_dPiMin;
	m_dPiMax					= pIOParam->m_dPiMax;
	m_bVarsplices			= pIOParam->m_bVarsplices;
	m_bFragments			= pIOParam->m_bFragments;
	m_iMissedClevage	= pIOParam->m_iMissedClevage;
	m_eResolution			= pIOParam->m_eResolution;
	m_eIonMode				= pIOParam->m_eIonMode;
	m_bPTM						= pIOParam->m_bPTM;
	m_dShift					= pIOParam->m_dShift;
	m_iSlope					= pIOParam->m_iSlope;
	m_iError					= pIOParam->m_iError;
	m_iHitsMin				= pIOParam->m_iHitsMin;
	m_iDisplayMax			= pIOParam->m_iDisplayMax;
}







/******************************************************

		Constructor

*******************************************************/
IOParamDB::IOParamDB(void) :
	m_pszLabel(NULL),
	m_pszPath(NULL),
	m_uiTime(0)
{
}



/******************************************************

		Constructor

*******************************************************/
IOParamDB::IOParamDB(const char *pszLabel, const char *pszPath, unsigned int	uiTime) :
	m_pszLabel(Strdup(pszLabel)),
	m_pszPath(Strdup(pszPath)),
	m_uiTime(uiTime)
{
}




/******************************************************

		Destructor

*******************************************************/
IOParamDB::~IOParamDB(void)
{
	Reset();
}


/***********************************************************

   Reset

***********************************************************/
void IOParamDB::Reset(void)
{
	if(m_pszLabel){
		delete [] m_pszLabel;
		m_pszLabel = NULL;
	}

	if(m_pszPath){
		delete m_pszPath;
		m_pszPath = NULL;
	}

	m_uiTime = 0;
}

/***********************************************************

   Copy

***********************************************************/
void IOParamDB::Copy(IOParamDB *pIOParamDB)
{
	Reset();

	m_pszLabel	= Strdup(pIOParamDB->m_pszLabel);
	m_pszPath		= Strdup(pIOParamDB->m_pszPath);
	m_uiTime		= pIOParamDB->m_uiTime;
}


/***********************************************************

   ReadVersion1And2

***********************************************************/
void IOParamDB::ReadVersion1And2(TagFile &file)
{
	file.ReadStartObject("DBFile");
	file.ReadMember("Label",	&m_pszLabel);
	file.ReadMember("Path",		&m_pszPath);
	file.ReadEndObject();	//DBFile
}

/***********************************************************

   Read

***********************************************************/
void IOParamDB::Read(TagFile &file)
{
	file.ReadStartObject("DBFile");
	file.ReadMember("Label",	&m_pszLabel);
	file.ReadMember("Time",		&m_uiTime);
	file.ReadMember("Path",		&m_pszPath);
	file.ReadEndObject();	//DBFile
}

/***********************************************************

   Write

***********************************************************/
void IOParamDB::Write(TagFile &file)
{
	file.WriteStartObject("DBFile");
	file.WriteMember("Label", m_pszLabel);
	file.WriteMember("Time",	m_uiTime);
	file.WriteMember("Path",	m_pszPath);
	file.WriteEndObject();	//DBFile
}












/******************************************************

		Constructor

*******************************************************/
IOParamModif::IOParamModif(void) :
	m_pszName(NULL),
	m_pszLabel(NULL),
	m_pszLocus(NULL),
	m_pszFormula(NULL),
	m_bMode(MODIF_FIXED),
	m_iTolerance(0),
	m_dScoring(0)
{
}



/******************************************************

		Constructor

*******************************************************/
IOParamModif::IOParamModif(const char *pszName, const char *pszLabel, const char *pszLocus, const char *pszFormula,	bool bMode, int iTolerance, double dScoring) :
	m_bMode(bMode),
	m_iTolerance(iTolerance),
	m_dScoring(dScoring)
{
	m_pszName			= Strdup(pszName);
	m_pszLabel		= Strdup(pszLabel);
	m_pszLocus		= Strdup(pszLocus);
	m_pszFormula	= Strdup(pszFormula);
}




/******************************************************

		Destructor

*******************************************************/
IOParamModif::~IOParamModif(void)
{
	if(m_pszName){
		delete [] m_pszName;
		m_pszName = NULL;
	}

	if(m_pszLabel){
		delete [] m_pszLabel;
		m_pszLabel = NULL;
	}

	if(m_pszLocus){
		delete m_pszLocus;
		m_pszLocus = NULL;
	}
  
	if(m_pszFormula){
		delete [] m_pszFormula;
		m_pszFormula = NULL;
	}
}

/***********************************************************

   Copy

***********************************************************/
void IOParamModif::Copy(IOParamModif *pModif)
{
	m_pszName			= Strdup(pModif->m_pszName);
	m_pszLabel		= Strdup(pModif->m_pszLabel);
	m_pszLocus		= Strdup(pModif->m_pszLocus);
	m_pszFormula	= Strdup(pModif->m_pszFormula);
	m_bMode				= pModif->m_bMode;
	m_iTolerance	= pModif->m_iTolerance;
	m_dScoring		= pModif->m_dScoring;
}






/***********************************************************

   SetName

***********************************************************/
void IOParamModif::SetName(const char *pszName)
{
	if(strlen(pszName) == 0)
		ThrowError("IOParamModif::SetName", "IOParamModif Name should not be empty");

	if(m_pszName){
		delete [] m_pszName;
		m_pszName = NULL;
	}

	m_pszName	= Strdup(pszName);
}


/***********************************************************

   SetLabel

***********************************************************/
void IOParamModif::SetLabel(const char *pszLabel)
{
	if(strlen(pszLabel) == 0)
		ThrowError("IOParamModif::SetLabel", "IOParamModif Label should not be empty");

	if(strlen(pszLabel) > 8)
		ThrowError("IOParamModif::SetLabel", "IOParamModif Label should not have more than 8 characters");

	if(m_pszLabel){
		delete [] m_pszLabel;
		m_pszLabel = NULL;
	}

	m_pszLabel	= Strdup(pszLabel);
}

/***********************************************************

   SetLocus

***********************************************************/
void IOParamModif::SetLocus(const char *pszLocus)
{
	int iLen = (int)strlen(pszLocus);

	if(iLen == 0)
		ThrowError("IOParamModif::SetLocus", "IOParamModif Locus should not be empty");

	if(iLen > 21)
		ThrowError("IOParamModif::SetLocus", "IOParamModif Locus should not have more than 21 characters");

	for(int i=0; i<iLen; i++){
		if(!strchr("ACDEFGHIKLMNPQRSTVWXY$", pszLocus[i]))
			ThrowError("IOParamModif::SetLocus", "IOParamModif Locus bad format : use one letter amino acid code and \"$\" for peptide ends");
	}
	
	if(m_pszLocus){
		delete [] m_pszLocus;
		m_pszLocus = NULL;
	}

	m_pszLocus	= Strdup(pszLocus);
}

/***********************************************************

   SetFormula

***********************************************************/
void IOParamModif::SetFormula(const char *pszFormula)
{
	try{
		AtomicMass atomicMass(MASS_MONOISOTOPIC);
		atomicMass.GetMass(pszFormula);
	}catch(Error *pError){
		pError->Stack("IOParamModif::SetFormula", "IOParamModif Formula is not valid");
	}

	if(m_pszFormula){
		delete [] m_pszFormula;
		m_pszFormula = NULL;
	}

	m_pszFormula	= Strdup(pszFormula);
}


/***********************************************************

   SetTolerance

***********************************************************/
void IOParamModif::SetTolerance(int iTolerance)
{
	if(iTolerance < 0 || iTolerance > 10)
		ThrowError("IOParamModif::SetTolerance", "IOParamModif Tolerance should be an integer >= 0 and <= 10");

	m_iTolerance = iTolerance;
}
/***********************************************************

   SetScoring

***********************************************************/
void IOParamModif::SetScoring(double dScoring)
{
	if(dScoring < 0 || dScoring > 1)
		ThrowError("IOParamModif::SetScoring", "IOParamModif Scoring should be a floating value between 0 and 1");

	m_dScoring = dScoring;
}

/***********************************************************

   Read

***********************************************************/
void IOParamModif::Read(TagFile &file)
{
	file.ReadStartObject("Modif");
	file.ReadMember("Name",				&m_pszName);
	file.ReadMember("Label",			&m_pszLabel);
	file.ReadMember("Locus",			&m_pszLocus);
	file.ReadMember("Formula",		&m_pszFormula);
	file.ReadMember("Mode",				&m_bMode);
	file.ReadMember("Tolerance",	&m_iTolerance);
	file.ReadMember("Scoring",		&m_dScoring);
	file.ReadEndObject();	//Modif
}

/***********************************************************

   Write

***********************************************************/
void IOParamModif::Write(TagFile &file)
{
	file.WriteStartObject("Modif");
	file.WriteMember("Name",				m_pszName);
	file.WriteMember("Label",				m_pszLabel);
	file.WriteMember("Locus",				m_pszLocus);
	file.WriteMember("Formula",			m_pszFormula);
	file.WriteMember("Mode",				m_bMode);
	file.WriteMember("Tolerance",		m_iTolerance);
	file.WriteMember("Scoring",			m_dScoring);
	file.WriteEndObject();	//Modif
}



