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

#define	USE_POPITAMFORM
#define	USE_CGI
#define	USE_HTML
#define	USE_REQUEST
#define	USE_MSG
#define	USE_UTIL
#define	USE_TIME
#include "use.h"


const char *PopitamForm::pszRequest = "request";
const char *PopitamForm::pszCgi = "form.cgi";
const int BUFF_LENGTH = 256;


/******************************************************
 	Constructor
	    
	FileField m_dataFile;
	AreaField m_dataCopy;
	ItemList  m_formatList;
    
	ItemList  m_database;
	ItemList  m_taxonomy;
	Taxonomy m_taxonomyObj;
	AreaField m_acList;
    
	ItemList  m_fixedModifs;
	ItemList  m_variableModifs;

	ItemList  m_instrument;
	DoubleField m_fragmentError1;
    
	ItemList  m_enzyme;
    
	FileField m_scoreFile;
	AreaField m_scoreCopy;

	RadioGroup m_popitamRunMode;
	DoubleField m_parentMError;
	ItemList m_gapMax;
	DoubleField m_maxAddPM;
	DoubleField m_maxLossPM;
	DoubleField m_maxAddModif;
	DoubleField m_maxLossModif;

    
	IntField m_nbDisplay;

*******************************************************/
PopitamForm::PopitamForm(const char*pszName) :
	m_dataFile("dataFile"),
	m_dataCopy("dataFileCopy"),
	m_formatList("formatList"),
	m_database("database"),
//	  m_taxonomy("taxonomy1"),
	m_taxonomyObj(),
	m_acList("acList"),
	m_fixedModifs("fixedmodifs"),
	m_variableModifs("variablemodifs"),
	m_instrument("instrument"),
	m_fragmentError1("fragmentError1"),
	m_enzyme("enzyme"),
	m_missedcleav("missedcleav"),
	m_scoreFile("scoreFile"),
	m_scoreCopy("scoreFileCopy"),
	m_popitamRunMode("popitamRunMode"),
	m_parentMError("parentMError"),
	m_gapMax("gapMax"),
	m_maxAddPM("maxAddPM"),
	m_maxLossPM("maxLossPM"),
	m_maxAddModif("maxAddModif"),
	m_maxLossModif("maxLossModif"),
	m_nbDisplay("nbDisplay"),
	unusedAC(NULL),
	unusedACnb(0)
	{
	pszFormName=Strdup(pszName);
	}


/******************************************************

	 Destructor

*******************************************************/
PopitamForm::~PopitamForm(void)
{
}



/***********************************************************

   DefaultDisplay

***********************************************************/
void PopitamForm::DoDisplay(File &file, const char *pszMsg)
{
	WriteHtmlContentType();
	WriteForm(file, pszMsg);
}

/***********************************************************

   Load

***********************************************************/
void PopitamForm::Load(void)
{
	//popitam form
	m_dataFile.Load(NO_LIMIT, 1, 0);
	m_dataCopy.Load(NO_LIMIT, 1, 0);
	m_formatList.Load(3, 1, 1);//nb, min, max
	m_formatList.Add("mgf");
	m_formatList.Add("dta");
	m_formatList.Add("mzdata");
    
	m_database.Load(2, 1, 2);//nb, min, max
	m_database.Add("UniProtKB/Swiss-Prot");
	m_database.Add("UniProtKB/TrEMBL");
/*	  m_taxonomy.Load(64, 1, 1);//nb, min, max
	m_taxonomy.Add(1,  "All");
*/
	m_taxonomyObj.Load();
    
	m_acList.Load(NO_LIMIT,	1, 0);

	m_fixedModifs.Load(1, 0, 1);//nb, min, max
	m_fixedModifs.Add("not available for the moment");
	m_variableModifs.Load(1, 0, 1);//nb, min, max
	m_variableModifs.Add("not available for the moment");
    
	m_instrument.Load(2, 1,	1);//nb, min, max
	m_instrument.Add("QTOF");
	m_instrument.Add("TOFTOF");
	m_fragmentError1.Load(FLOOR|CEILING, 0.01, 1.0);

	m_enzyme.Load(1, 1, 1);//nb, min, max
	m_enzyme.Add("Trypsin");
	m_missedcleav.Load(2, 1, 1);//nb, min, max
	m_missedcleav.Add("0");
	m_missedcleav.Add("1");

	m_scoreFile.Load(NO_LIMIT, 1, 0);
	m_scoreCopy.Load(NO_LIMIT, 1, 0);

//	m_popitamRunMode.Load(2, 1, 1);
//	m_popitamRunMode.Add("normal");
//	m_popitamRunMode.Add("open search");
//	m_parentMError.Load(FLOOR|CEILING, 0, 2.5);
	m_gapMax.Load(2, 1, 1);//nb, min, max
	m_gapMax.Add("1");
	m_gapMax.Add("2");
	m_maxAddPM.Load(NO_LIMIT, 0, 0);
	m_maxLossPM.Load(NO_LIMIT, 0, 0);
	m_maxAddModif.Load(NO_LIMIT, 0, 0);
	m_maxLossModif.Load(NO_LIMIT, 0, 0);
    
	m_nbDisplay.Load(FLOOR|CEILING,	1, 20);

	//test form
/*	m_intField.Load(FLOOR|CEILING|EMPTY, 10, 20);
	m_fileField.Load(NO_LIMIT, 1, 0);
*/
    
}

/***********************************************************

   SetDefault

***********************************************************/
void PopitamForm::SetDefault(void)
{
	//popitam form
	//m_dataFile
	m_dataCopy.Set("");
	m_formatList.Reset();
	m_formatList.Select(0, true);
    
	m_database.Reset();
	m_database.Select(0, true);	//Swiss-Prot only, as Homo sapiens is selected in taxonomy

//	  m_taxonomy.Select(7, true);
	m_taxonomyObj.Reset();
	m_taxonomyObj.GetItem(1)->Select(true); //9606 Homo sapiens - 158879 Staphylococcus aureus (strain N315)
	m_acList.Set("");
    
	m_fixedModifs.Select(0,	true);
	m_variableModifs.Select(0, true);
    
	m_instrument.Reset();
	m_instrument.Select(0, true);
	m_fragmentError1.SetDouble(0.2);

	m_enzyme.Select(0, true);
	m_missedcleav.Reset();
	m_missedcleav.Select(0,	true);
    
	//m_scoreFile
	m_scoreCopy.Set("");

//	m_popitamRunMode.Select(1);
//	m_parentMError.SetDouble(1);
	m_gapMax.Reset();
	m_gapMax.Select(0, true);
	m_maxAddPM.SetDouble(100);
	m_maxLossPM.SetDouble(0);
	m_maxAddModif.SetDouble(50);
	m_maxLossModif.SetDouble(0);
    
	m_nbDisplay.SetInt(5);

}

/***********************************************************

   SetExample

   TEST_0MOD.dta
   TEST_0MOD.mgf
   TEST_1MOD.mgf
   TEST_2MOD.mgf

***********************************************************/

void PopitamForm::SetExample0MODDta(void)
{
	//popitam form
	m_dataCopy.Set("");
	//assigne le contenu des data au champ correspondant
	int	BIG_BUFF_LENGTH	= 50*BUFF_LENGTH;
	char szBuffer[BUFF_LENGTH];
	char* szData = new char[BIG_BUFF_LENGTH];
	strcpy(szData, "");
	char* szTemp;
	File dataFile;
	if(dataFile.OpenIfExist(StrdupFormat("%sTEST_0MOD.dta", path()->pszScores), "r") ) {
		int currentLength=0;
		while( fgets(szBuffer, BUFF_LENGTH,	dataFile)!=NULL	) {
			currentLength+=(int)strlen(szBuffer);
			if(currentLength>BIG_BUFF_LENGTH) {
				BIG_BUFF_LENGTH*=2;
				szTemp = new char[BIG_BUFF_LENGTH];
				strcpy(szTemp, szData);
				delete[] szData;
				szData = szTemp;
			}
			strcat(szData, szBuffer);
		}
		dataFile.Close();
		m_dataCopy.Set(szData);
	}
	delete[] szData;

	m_formatList.Reset();
	m_formatList.Select(1, true); //dta
    
	m_database.Reset();
	m_database.Select(0, true);	//SP
	m_taxonomyObj.Reset();
	m_taxonomyObj.GetItem(9606)->Select(true); //human
	m_acList.Set(""); //no
    
	m_fixedModifs.Select(0, true); //default
	m_variableModifs.Select(0, true); //default
    
	m_instrument.Reset();
	m_instrument.Select(0, true); //qtof
	m_fragmentError1.SetDouble(0.2); //0.2

	m_enzyme.Select(0, true); //trypsin
	m_missedcleav.Reset();
	m_missedcleav.Select(1, true); //1 missed
    
	//m_scoreFile
        BIG_BUFF_LENGTH = 50*BUFF_LENGTH;
        szData = new char[BIG_BUFF_LENGTH];
        strcpy(szData, "");
        if(dataFile.OpenIfExist(StrdupFormat("%sfunScore0.dot", path()->pszScores), "r") ) {
                int currentLength=0;
                while( fgets(szBuffer, BUFF_LENGTH, dataFile)!=NULL ) {
                        currentLength+=(int)strlen(szBuffer);
                        if(currentLength>BIG_BUFF_LENGTH) {
                                BIG_BUFF_LENGTH*=2;
                                szTemp = new char[BIG_BUFF_LENGTH];
                                strcpy(szTemp, szData);
                                delete[] szData;
                                szData = szTemp;
                        }
                        strcat(szData, szBuffer);
                }
                dataFile.Close();
                m_scoreCopy.Set(szData);
        }
	//was : m_scoreCopy.Set(""); //default

	m_popitamRunMode.Select(0); //normal
	m_parentMError.SetDouble(2); //2
	m_gapMax.Reset();
	m_gapMax.Select(0, true); //default
	m_maxAddPM.SetDouble(100); //default
	m_maxLossPM.SetDouble(0); //default
	m_maxAddModif.SetDouble(50); //default
	m_maxLossModif.SetDouble(0); //default
    
	m_nbDisplay.SetInt(5); //default

}
void PopitamForm::SetExample0MODMgf(void)
{
	//popitam form
	m_dataCopy.Set("");
	//assigne le contenu des data au champ correspondant
	const int	BUFF_LENGTH	= 256;
	int	BIG_BUFF_LENGTH	= 50*BUFF_LENGTH;
	char szBuffer[BUFF_LENGTH];
	char* szData = new char[BIG_BUFF_LENGTH];
	strcpy(szData, "");
	char* szTemp;
	File dataFile;
	if(dataFile.OpenIfExist(StrdupFormat("%sTEST_0MOD.mgf", path()->pszScores), "r") ) {
		int	currentLength=0;
		while( fgets(szBuffer, BUFF_LENGTH,	dataFile)!=NULL	) {
			currentLength+=(int)strlen(szBuffer);
			if(currentLength>BIG_BUFF_LENGTH) {
				BIG_BUFF_LENGTH*=2;
				szTemp = new char[BIG_BUFF_LENGTH];
				strcpy(szTemp, szData);
				delete[] szData;
				szData = szTemp;
			}
			strcat(szData, szBuffer);
		}
		dataFile.Close();
		m_dataCopy.Set(szData);
	}
	delete[] szData;
	m_formatList.Reset();
	m_formatList.Select(0, true); //mgf
    
	m_database.Reset();
	m_database.Select(0, true);	//SP
	m_taxonomyObj.Reset();
	m_taxonomyObj.GetItem(9606)->Select(true); //human
	m_acList.Set(""); //no
    
	m_fixedModifs.Select(0, true); //default
	m_variableModifs.Select(0, true); //default
    
	m_instrument.Reset();
	m_instrument.Select(0, true); //qtof
	m_fragmentError1.SetDouble(0.2); //0.2

	m_enzyme.Select(0, true); //trypsin
	m_missedcleav.Reset();
	m_missedcleav.Select(1,	true); //1 missed
    
	//m_scoreFile
        BIG_BUFF_LENGTH = 50*BUFF_LENGTH;
        szData = new char[BIG_BUFF_LENGTH];
        strcpy(szData, "");
        if(dataFile.OpenIfExist(StrdupFormat("%sfunScore0.dot", path()->pszScores), "r") ) {
                int currentLength=0;
                while( fgets(szBuffer, BUFF_LENGTH, dataFile)!=NULL ) {
                        currentLength+=(int)strlen(szBuffer);
                        if(currentLength>BIG_BUFF_LENGTH) {
                                BIG_BUFF_LENGTH*=2;
                                szTemp = new char[BIG_BUFF_LENGTH];
                                strcpy(szTemp, szData);
                                delete[] szData;
                                szData = szTemp;
                        }
                        strcat(szData, szBuffer);
                }
                dataFile.Close();
                m_scoreCopy.Set(szData);
        }
	//was : m_scoreCopy.Set(""); //default

	m_popitamRunMode.Select(0); //open
	m_parentMError.SetDouble(1); //default
	m_gapMax.Reset();
	m_gapMax.Select(0, true); //default
	m_maxAddPM.SetDouble(100); //default
	m_maxLossPM.SetDouble(0); //default
	m_maxAddModif.SetDouble(50); //default
	m_maxLossModif.SetDouble(0); //default
    
	m_nbDisplay.SetInt(5); //default

}
void PopitamForm::SetExample1MODMgf(void)
{
	//popitam form
	m_dataCopy.Set("");
	//assigne le contenu des data au champ correspondant
	int	BIG_BUFF_LENGTH	= 50*BUFF_LENGTH;
	char szBuffer[BUFF_LENGTH];
	char* szData = new char[BIG_BUFF_LENGTH];
	strcpy(szData, "");
	char* szTemp;
	File dataFile;
	if(dataFile.OpenIfExist(StrdupFormat("%sTEST_1MOD.mgf", path()->pszScores), "r") ) {
		int	currentLength=0;
		while( fgets(szBuffer, BUFF_LENGTH,	dataFile)!=NULL ) {
			currentLength+=(int)strlen(szBuffer);
			if(currentLength>BIG_BUFF_LENGTH) {
				BIG_BUFF_LENGTH*=2;
				szTemp = new char[BIG_BUFF_LENGTH];
				strcpy(szTemp, szData);
				delete[] szData;
				szData = szTemp;
			}
			strcat(szData, szBuffer);
		}
		dataFile.Close();
		m_dataCopy.Set(szData);
	}
	delete[] szData;
	m_formatList.Reset();
	m_formatList.Select(0, true); //mgf
    
	m_database.Reset();
	m_database.Select(0, true);	//SP
	m_taxonomyObj.Reset();
	m_taxonomyObj.GetItem(1)->Select(true); //human
	m_acList.Set("P11940 P63261 P18621 P11216 P63261"); //P11940 P63261 P18621 P11216 P63261
    
	m_fixedModifs.Select(0,	true); //default
	m_variableModifs.Select(0, true); //default
    
	m_instrument.Reset();
	m_instrument.Select(0, true); //qtof
	m_fragmentError1.SetDouble(0.2); //0.2

	m_enzyme.Select(0, true); //trypsin
	m_missedcleav.Reset();
	m_missedcleav.Select(1,	true); //1 missed
    
	//m_scoreFile
        BIG_BUFF_LENGTH = 50*BUFF_LENGTH;
        szData = new char[BIG_BUFF_LENGTH];
        strcpy(szData, "");
        if(dataFile.OpenIfExist(StrdupFormat("%sfunScore1.dot", path()->pszScores), "r") ) {
                int currentLength=0;
                while( fgets(szBuffer, BUFF_LENGTH, dataFile)!=NULL ) {
                        currentLength+=(int)strlen(szBuffer);
                        if(currentLength>BIG_BUFF_LENGTH) {
                                BIG_BUFF_LENGTH*=2;
                                szTemp = new char[BIG_BUFF_LENGTH];
                                strcpy(szTemp, szData);
                                delete[] szData;
                                szData = szTemp;
                        }
                        strcat(szData, szBuffer);
                }
                dataFile.Close();
                m_scoreCopy.Set(szData);
        }
 	//was : m_scoreCopy.Set(""); //default

//	m_popitamRunMode.Select(1);	//open
//	m_parentMError.SetDouble(1); //default
	m_gapMax.Reset();
	m_gapMax.Select(0, true); //1
	m_maxAddPM.SetDouble(200); //200
	m_maxLossPM.SetDouble(0); //default
	m_maxAddModif.SetDouble(200); //200
	m_maxLossModif.SetDouble(0); //default
    
	m_nbDisplay.SetInt(5); //default

}
void PopitamForm::SetExample2MODMgf(void)
{
	//popitam form
	m_dataCopy.Set("");
	//assigne le contenu des data au champ correspondant
	int	BIG_BUFF_LENGTH	= 70*BUFF_LENGTH;
	char szBuffer[BUFF_LENGTH];
	char* szData = new char[BIG_BUFF_LENGTH];
	strcpy(szData, "");
	char* szTemp;
	File dataFile;
	if(dataFile.OpenIfExist(StrdupFormat("%sTEST_2MOD.mgf", path()->pszScores), "r") ) {
		int	currentLength=0;
		while( fgets(szBuffer, BUFF_LENGTH,	dataFile)!=NULL ) {
			currentLength+=(int)strlen(szBuffer);
			if(currentLength>BIG_BUFF_LENGTH) {
				BIG_BUFF_LENGTH*=2;
				szTemp = new char[BIG_BUFF_LENGTH];
				strcpy(szTemp, szData);
				delete[] szData;
				szData = szTemp;
			}
			strcat(szData, szBuffer);
		}
		dataFile.Close();
		m_dataCopy.Set(szData);
	}
	delete[] szData;
	m_formatList.Reset();
	m_formatList.Select(0, true); //mgf
    
	m_database.Reset();
	m_database.Select(0, true);	//SP
	m_taxonomyObj.Reset();
	m_taxonomyObj.GetItem(1)->Select(true); //human
	m_acList.Set("Q9GZL7");	//Q9GZL7
    
	m_fixedModifs.Select(0,	true); //default
	m_variableModifs.Select(0, true); //default
    
	m_instrument.Reset();
	m_instrument.Select(0, true); //qtof
	m_fragmentError1.SetDouble(0.2); //0.2

	m_enzyme.Select(0, true); //trypsin
	m_missedcleav.Reset();
	m_missedcleav.Select(1,	true); //1 missed
    
	//m_scoreFile
        BIG_BUFF_LENGTH = 50*BUFF_LENGTH;
        szData = new char[BIG_BUFF_LENGTH];
        strcpy(szData, "");
        if(dataFile.OpenIfExist(StrdupFormat("%sfunScore2.dot", path()->pszScores), "r") ) {
                int currentLength=0;
                while( fgets(szBuffer, BUFF_LENGTH, dataFile)!=NULL ) {
                        currentLength+=(int)strlen(szBuffer);
                        if(currentLength>BIG_BUFF_LENGTH) {
                                BIG_BUFF_LENGTH*=2;
                                szTemp = new char[BIG_BUFF_LENGTH];
                                strcpy(szTemp, szData);
                                delete[] szData;
                                szData = szTemp;
                        }
                        strcat(szData, szBuffer);
                }
                dataFile.Close();
                m_scoreCopy.Set(szData);
        }
 	//was : m_scoreCopy.Set(""); //default

//	m_popitamRunMode.Select(1);	//open
//	m_parentMError.SetDouble(1); //default
	m_gapMax.Reset();
	m_gapMax.Select(1, true); //2
	m_maxAddPM.SetDouble(200); //200
	m_maxLossPM.SetDouble(0); //default
	m_maxAddModif.SetDouble(200); //200
	m_maxLossModif.SetDouble(0); //default
    
	m_nbDisplay.SetInt(5); //default

}

/***********************************************************

   ReadCgi

***********************************************************/
int PopitamForm::ReadCgi(void)
{
	Cgi cgi;
    
	//stocke le nbre de bytes des parametres pour initialiser le generateur de nbre aleatoire
	int iNbByte = cgi.Load();
	srand(iNbByte);

	int m_iRequest = cgi.GetInt(PopitamForm::pszRequest);

	//popitam form
	m_dataFile.ReadCgi(&cgi);
	m_dataCopy.ReadCgi(&cgi);
	m_formatList.ReadCgi(&cgi);
    
	m_database.ReadCgi(&cgi);
//	  m_taxonomy.ReadCgi(&cgi);
	m_taxonomyObj.ReadCgi(&cgi);
	m_acList.ReadCgi(&cgi);
    
//	  m_fixedModifs.ReadCgi(&cgi);
//	  m_variableModifs.ReadCgi(&cgi);
    
	m_instrument.ReadCgi(&cgi);
	m_fragmentError1.ReadCgi(&cgi);

	m_enzyme.ReadCgi(&cgi);
	m_missedcleav.ReadCgi(&cgi);
    
	m_scoreFile.ReadCgi(&cgi);
	m_scoreCopy.ReadCgi(&cgi);

//	m_popitamRunMode.ReadCgi(&cgi);
//	m_parentMError.ReadCgi(&cgi);
	m_gapMax.ReadCgi(&cgi);
	m_maxAddPM.ReadCgi(&cgi);
	m_maxLossPM.ReadCgi(&cgi);
	m_maxAddModif.ReadCgi(&cgi);
	m_maxLossModif.ReadCgi(&cgi);
    
	m_nbDisplay.ReadCgi(&cgi);

	m_display.ReadCgi(&cgi);
    
	return m_iRequest;
}

/***********************************************************

   CheckCgi

***********************************************************/
void PopitamForm::CheckCgi(void)
{
                if(strlen(m_dataFile.GetString())) { //copy file content in data text area
                        m_dataCopy.Set(m_dataFile.GetString());
                }
                if( strlen(m_scoreFile.GetString()) ) {
                        m_scoreCopy.Set(m_scoreFile.GetString());
                }
 

	if(!strlen(m_dataFile.GetString()) && !strlen(m_dataCopy.GetString()) ) {
 		ThrowMsg("Please choose	a data file or fill in the text area.");
	}

	if( strlen(m_dataFile.GetString()) ) {
		if(!strcmp(GetDataFormat(), "mgf")){
			CheckMGF(Strdup(GetDataFile()));
		}
		else if(!strcmp(GetDataFormat(), "dta")	) {
			CheckDTA(Strdup(GetDataFile()));
		}
		else if(!strcmp(GetDataFormat(), "mzdata")	) {
			CheckMZDATA(Strdup(GetDataFile()));
		}
	}
	else if( strlen(m_dataCopy.GetString())	) {
		if(!strcmp(GetDataFormat(), "mgf") ) {
			CheckMGF(Strdup(GetDataCopy()));
		}
		else if(!strcmp(GetDataFormat(), "dta")	) {
			CheckDTA(Strdup(GetDataCopy()));
		}
		else if(!strcmp(GetDataFormat(), "mzdata")	) {
			CheckMZDATA(Strdup(GetDataCopy()));
		}
	}
	    
	if(!m_database.GetNbSelectedItem())	{
 		ThrowMsg("Please select	database(s) in list.");
	}
	    
/*	if(!m_taxonomyObj.GetNbSelectedItem()) {
		ThrowMsg("Please select	taxon in list.");
	}*/
/*	else {
		Taxon* ptSelTaxon = m_taxonomyObj.GetFirstSelectedItem();
		int nbSPEntries = m_database.GetItem(0)->IsSelected()?ptSelTaxon->GetNbSPEntries():0;
		int nbTREntries = m_database.GetItem(1)->IsSelected()?ptSelTaxon->GetNbTREntries():0;
		int maxLimit = 35000;
		if(nbSPEntries+nbTREntries > maxLimit && !strlen(m_acList.GetString()) ) {
			ThrowMsg("Please select taxons or database, so that protein number is <%d (current total is %d).", maxLimit, nbSPEntries+nbTREntries);
		}
	}
*/
	if(!strlen(m_acList.GetString()) && GetMutMode() ) {
		ThrowMsg("Please fill in from 1 to 2000 AC when running open-modification search.");//Q7A5V8
	}

    if(strlen(m_acList.GetString())>14000 ) {// = 2000 *(6 char AC + 1 espace)
		char* list = Strdup(m_acList.GetString());
		int tokensNb=0;
		char* token = NULL;
		for( token=strtok(list, " \n\r\t"); token; token=strtok(NULL, " \n\r\t") ) {
			tokensNb++;
		}
		if(tokensNb >2000) {
                	ThrowMsg("Please limit your request to 2000 AC. (%d)", tokensNb);
		}
    }


	if( strlen(m_scoreFile.GetString()) ) {
		CheckScoring(Strdup(GetScoreFile()));
	}
	else if( strlen(m_scoreCopy.GetString()) ) {
		CheckScoring(Strdup(GetScoreCopy()));
	}
	    
	m_display.CheckCgi();
}

/***********************************************************

   CheckMGF
   
***********************************************************/

void PopitamForm::CheckScoring(char* scoringFunction)
{
	char* ptrFun = scoringFunction;
	const char* numChars = "0123456789";
	bool valid=false;
    
	unsigned int lineNb=1;

	for(ptrFun = strtok(scoringFunction, "\n"); ptrFun!=NULL && ptrFun[0]=='#'; ptrFun=strtok(NULL, "\n"), lineNb++ ) {
	}
    
	if(!strncmp(ptrFun, "digraph g {", 11)==0) {
		ThrowMsg("Scoring : no _digraph g {_ line at the very beginning of the file (# comments are optional).");
	}
    
	for(ptrFun=strtok(NULL, "\n"); ptrFun!=NULL; ptrFun=strtok(NULL, "\n") ) {
		lineNb++;
	    
		if(strncmp(ptrFun, "node [", 6)==0) {
			continue;
		}
		if(strncmp(ptrFun, "label = \"", 9)==0) {
			continue;
		}
		if(strncmp(ptrFun, "node", 4)==0 && strspn(ptrFun+4, numChars)>0) {
			break;
		}
	}

	if(ptrFun==NULL) {
		ThrowMsg("Scoring : missing node description inside digraph.");
	}

	int nbNumChar;
	for( ; ptrFun!=NULL; ptrFun=strtok(NULL, "\n"),	lineNb++ ) {
	    
		if( ptrFun[0]=='}' ) { //end of digraph
			valid=true;
			break;
		}

		if(strncmp(ptrFun, "node", 4)==0) {
			nbNumChar = (int)strspn(ptrFun+4, numChars);
			if(strncmp(ptrFun+4+nbNumChar, "[label = \"", 10)!=0) {
				ThrowMsg("Scoring line nb %d: must be NODELABEL[label =	SUBSCORE_OR_OPERATOR];", lineNb);
			}
	    
			ptrFun = strtok(NULL, "\n");
			lineNb++;
			if(ptrFun!=NULL) {
				if(strncmp(ptrFun, "\"node", 5)==0)	{
					nbNumChar = (int)strspn(ptrFun+5, numChars);
					if(nbNumChar==0	|| strncmp(ptrFun+5+nbNumChar, "\"->\"node", 8)!=0 || strspn(ptrFun+5+nbNumChar+8, numChars)==0	) {
						ThrowMsg("Scoring line nb %d: link description must be _NODELABEL->NODELABEL;_", lineNb);
					}
				}
				else {
					if( ptrFun[0]=='}' ) { //end of digraph
						valid=true;
						break;
					}
					else {
						ThrowMsg("Scoring line nb %d: missing link in digraph description.", lineNb);
					}
				}
			}
			else {
				ThrowMsg("Scoring line nb %d: missing link in digraph description.", lineNb);
			}
		}
		else {
			ThrowMsg("Scoring line nb %d: invalid.", lineNb);
		}
	}
    
	if(!valid) {
		if(ptrFun==NULL) {
			ThrowMsg("Scoring : missing closing } in digraph description.");
		}
		ThrowMsg("Invalid Scoring function format.");
	}

}

/***********************************************************

   CheckMGF
   
***********************************************************/

void PopitamForm::CheckMGF(char* data)
{
	char* ptrData =	data;
	const char*	numChars = "-0123456789.eE";
	const char*	spaceChars = " \t";
	const char*	spaceAndRetChars = " \t\r\n";
    
	bool valid = false;
	char buffer[128];

	unsigned int nbNumChars1=0, nbNumChars2=0, nbSpace1=0, nbSpace2=0;
	unsigned int lineLength=0;//, dataLength=0, readLength=0;
	unsigned int lineNb=0;
	unsigned int spectraNb=0;

	for(ptrData = strtok(data, "\n"); ptrData!=NULL; ptrData = strtok(NULL, "\n") ) {
		lineNb++;
	    
		if(strncmp(ptrData, "BEGIN IONS", 10)==0) {//begin ion data
			spectraNb++;

			while(strncmp(ptrData, "END IONS", 8)!=0) {
				ptrData	= strtok(NULL, "\n");
				lineNb++;
			    
				if(ptrData==NULL) { //BEGIN IONS without END IONS
					valid =	false;
					ThrowMsg("Last BEGIN IONS without END IONS");
				}

				lineLength = (unsigned int)strlen(ptrData);
			    
				if(strncmp(ptrData, "END IONS", 8)==0) { //end of ion data
					valid=true;
					break;
//					  ThrowMsg("Line nb %d is %d long.%s found END IONS.", lineNb, lineLength, ptrData);
				}
				if(strncmp(ptrData, "BEGIN IONS", 10)==0) {
					ThrowMsg("Line nb %d: precedent	BEGIN IONS without coresponding	END IONS", lineNb);
				}
				if(strchr(ptrData, '=')!=NULL || ptrData[0]=='#') {
					continue; //found mgf parameters or comment line
				}
				if(strcspn(ptrData, numChars)==0) {//cherche la premiere occurence d'un numChar
					nbNumChars1 = (unsigned int)strspn(ptrData, numChars); //lit le nombre de numChars
					if(nbNumChars1<lineLength) {
						nbSpace1 = (unsigned int)strspn(ptrData+nbNumChars1, spaceChars); //lit le nombre de spaceChars
						if(nbNumChars1+nbSpace1<lineLength) {
							nbNumChars2 = (unsigned int)strspn(ptrData+nbNumChars1+nbSpace1, numChars); //lit le nombre de numChars
//							nbSpace2 = (unsigned int)strspn(ptrData+nbNumChars1+nbSpace1+nbNumChars2, spaceAndRetChars); //lit le nombre de spaceAndReturnChars
//							if(nbNumChars1+nbSpace1+nbNumChars2+nbSpace2 < lineLength) {
//								strncpy(buffer,	ptrData, nbNumChars1+nbSpace1+nbNumChars2);
//								ThrowMsg("Line nb %d: does not contain only two numbers.", lineNb);
//							}
						}
						else {
							ThrowMsg("Line nb %d: contains only one number.", lineNb);
						}
					}
					else {
						ThrowMsg("Line nb %d: contains only one number.", lineNb);
					}
				}
				else {
					ThrowMsg("Line nb %d : Invalid, not parameter or comment, nor values, nor END IONS.", lineNb);
				}
			}
		    
		}
	}
	if(!valid) {
		ThrowMsg("Invalid MGF format.");
	}
    
}

/***********************************************************

   CheckDTA

***********************************************************/

void PopitamForm::CheckDTA(char* data)
{
	char* ptrData =	data;
	const char*	numChars = "-0123456789.eE";
	const char*	spaceChars = " \t";
	const char*	spaceAndRetChars = " \t\r\n";
    
	unsigned int lineLength=0, dataLength=0, readLength=0;
	unsigned int nbNumChars1=0,	nbNumChars2=0, nbSpace1=0, nbSpace2=0;
	unsigned int lineNb=1;
	unsigned int spectraNb=1;
	char buffer[128];

	dataLength = (unsigned int)strlen(ptrData);
    
	for(ptrData = strtok(data, "\n"); ptrData!=NULL; ptrData = strtok(NULL, "\n") ) { //commentaire line1
		lineLength = (unsigned int)strlen(ptrData);
		readLength+=(lineLength+1);
		lineNb++;

		for( ; readLength<dataLength; lineNb++,	readLength+=(lineLength+1))	//data
		{
			ptrData	= strtok(NULL, "\n");
			if(ptrData==NULL) break; //end of data

			lineLength = (unsigned int)strlen(ptrData);
			if (lineLength == strspn(ptrData, spaceAndRetChars)) {
				lineNb++;
				spectraNb++;
//				if(spectraNb>5)	{
//					ThrowMsg("Please limit your request to 5 spectra.");
//				}
				break; //just one new line; new set of data
			}
			if(strcspn(ptrData, numChars)==0) { //cherche la premiere occurence d'un numChar
				nbNumChars1 = (unsigned int)strspn(ptrData, numChars); //lit le nombre de numChars
				if(nbNumChars1<lineLength) {
					nbSpace1 = (unsigned int)strspn(ptrData+nbNumChars1, spaceChars); //lit le nombre de spaceChars
					if(nbNumChars1+nbSpace1<lineLength)	{
						nbNumChars2 = (unsigned int)strspn(ptrData+nbNumChars1+nbSpace1, numChars); //lit le nombre de numChars
						nbSpace2 = (unsigned int)strspn(ptrData+nbNumChars1+nbSpace1+nbNumChars2, spaceAndRetChars); //lit le nombre de spaceAndReturnChars
						if(nbNumChars1+nbSpace1+nbNumChars2+nbSpace2 < lineLength) {
							strncpy(buffer,	ptrData, nbNumChars1+nbSpace1+nbNumChars2);
							ThrowMsg("Line %d does not contain only	two numbers.", lineNb);
						}
					}
					else {
						ThrowMsg("Line %d contains only one number.", lineNb);
					}
				}
				else {
					ThrowMsg("Line %d contains only one number.", lineNb);
				}
			}
			else {
				if(strcspn(ptrData, spaceAndRetChars)!=0) {//ligne commence par une nouvelle ligne
//					  ThrowMsg("Line nb %d is %d long and %d retsp %s. May be a new line.", lineNb, lineLength, strspn(ptrData, spaceAndRetChars), ptrData);
					lineNb++;
				}
				else {
					ThrowMsg("Line nb %d does not start with a number.", lineNb, lineLength, strspn(ptrData, spaceAndRetChars), ptrData);
				}
			}
		}
	    
		if(ptrData==NULL) break; //end of data
	}
    
//	  ThrowMsg("All	DTA data ok %d dataLength %d readline %d", readLength, dataLength, lineNb);
}


/***********************************************************

   CheckDTA

***********************************************************/

void PopitamForm::CheckMZDATA(char* data)
{
	char* ptrData =	data;
	ptrData = strtok(data, "\n"); 
	if( ptrData!=NULL && !strncmp( ptrData, "<?xml", 5) ) {
		ptrData = strtok(NULL, "\n");
		if ( ptrData!=NULL && !strncmp( ptrData, "<mzData", 7) )  {
			return;	
		}
	}
	ThrowMsg("Bad MZData format in xml file : second line must begin by '<mzdata'.");

}
		




/***********************************************************

1.05" accessionNumber="psi-ms:100" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">

   WriteExpasyFile

char *pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_foot"); _head _script _search

File expasyHeader;
if( expasyHeader.OpenIfExist(pszExpFileName, "r") ) {
	popitamForm.WriteExpasyFile(file, expasyHeader);
}

***********************************************************/
void PopitamForm::WriteExpasyFile(File &file, File &expasyFile)
{
	char szBuffer[BUFF_LENGTH];
    
	while( fgets(szBuffer, BUFF_LENGTH, expasyFile)!=NULL )	{
		fprintf(file, "%s", szBuffer);
	}
	fprintf(file, "\n");

}


/***********************************************************

   WriteResubmit

***********************************************************/
void PopitamForm::WriteResubmit(File &file, bool isByMail)
{
	if(isByMail) { //case where Resubmit button is displayed on a page sent by mail to the user, path to the form.cgi must contain the name of the server to be found
		fprintf(file, "<form ENCTYPE=\"multipart/form-data\" method=POST name=\"%s\" action=\"%s/cgi-bin/popitam/%s\">\n", pszFormName, path()->pszServer, pszCgi);
	}
	else { //page where button is displayed directly on screen, path is only relative
		fprintf(file, "<form ENCTYPE=\"multipart/form-data\" method=POST name=\"%s\" action=\"%scgi-bin/popitam/%s\">\n", pszFormName, path()->pszRelativeServer, pszCgi);
	}

	fprintf(file, "<input type=hidden name=%s value=%d>\n",	PopitamForm::pszRequest, REQUEST_FORM);
    
	//popitam form
	m_dataFile.Hide(file);
	if( strlen(m_dataFile.GetFileName()) ) {
		m_dataCopy.HideTxt(file, m_dataFile.GetString());
	}
	else {
		m_dataCopy.Hide(file);
	}
	m_formatList.Hide(file);
    
	m_database.Hide(file);
//	  m_taxonomy.Hide(file);
	m_taxonomyObj.Hide(file);
	m_acList.Hide(file);
    
	m_fixedModifs.Hide(file);
	m_variableModifs.Hide(file);
    
	m_instrument.Hide(file);
	m_fragmentError1.Hide(file);

	m_enzyme.Hide(file);
	m_missedcleav.Hide(file);

	m_scoreFile.Hide(file);
	if( strlen(m_scoreFile.GetFileName()) ) {
		m_scoreCopy.HideTxt(file, m_scoreFile.GetString());
	}
	else {
		m_scoreCopy.Hide(file);
	}

//	m_popitamRunMode.Hide(file);
//	m_parentMError.Hide(file);
	m_gapMax.Hide(file);
	m_maxAddPM.Hide(file);
	m_maxLossPM.Hide(file);
	m_maxAddModif.Hide(file);
	m_maxLossModif.Hide(file);

	m_nbDisplay.Hide(file);
	m_display.Hide(file);

	fprintf(file, "<input type=submit value=\"Resubmit\">");

	fprintf(file, "</form>\n");
}

/***********************************************************

   ReadFile
//	  m_dataFile.ReadFile(pFile);
//	  m_dataCopy.ReadFile(pFile);
//	  m_taxonomy.ReadFile(pFile);

***********************************************************/
void PopitamForm::ReadFile(TagFile *pFile)
{
	//popitam form
	m_formatList.ReadFile(pFile);
    
	m_database.ReadFile(pFile);
	m_taxonomyObj.ReadFile(pFile);
	m_acList.ReadFile(pFile);
    
	m_fixedModifs.ReadFile(pFile);
	m_variableModifs.ReadFile(pFile);
    
	m_instrument.ReadFile(pFile);
	m_fragmentError1.ReadFile(pFile);

	m_enzyme.ReadFile(pFile);
	m_missedcleav.ReadFile(pFile);

//	m_popitamRunMode.ReadFile(pFile);
//	m_parentMError.ReadFile(pFile);
	m_gapMax.ReadFile(pFile);
	m_maxAddPM.ReadFile(pFile);
	m_maxLossPM.ReadFile(pFile);
	m_maxAddModif.ReadFile(pFile);
	m_maxLossModif.ReadFile(pFile);
    
	m_nbDisplay.ReadFile(pFile);
	m_display.ReadFile(pFile);

}

/***********************************************************

   WriteFile

***********************************************************/
void PopitamForm::WriteFile(TagFile *pFile)
{
	//popitam form
//	  m_dataFile.WriteFile(pFile);
//	  m_dataCopy.WriteFile(pFile);
	m_formatList.WriteFile(pFile);
    
	m_database.WriteFile(pFile);
//	  m_taxonomy.WriteFile(pFile);
	m_taxonomyObj.WriteFile(pFile);
	m_acList.WriteFile(pFile);
    
	m_fixedModifs.WriteFile(pFile);
	m_variableModifs.WriteFile(pFile);
    
	m_instrument.WriteFile(pFile);
	m_fragmentError1.WriteFile(pFile);
    
	m_enzyme.WriteFile(pFile);
	m_missedcleav.WriteFile(pFile);
    
//	  m_scoreFile.WriteFile(pFile);
//	  m_scoreCopy.WriteFile(pFile);

//	m_popitamRunMode.WriteFile(pFile);
//	m_parentMError.WriteFile(pFile);
	m_gapMax.WriteFile(pFile);
	m_maxAddPM.WriteFile(pFile);
	m_maxLossPM.WriteFile(pFile);
	m_maxAddModif.WriteFile(pFile);
	m_maxLossModif.WriteFile(pFile);

	m_nbDisplay.WriteFile(pFile);
	m_display.WriteFile(pFile);
}

/***********************************************************

   WriteFormHead

***********************************************************/
void PopitamForm::WriteFormHead(File &file)
{
	fprintf(file, "<script type=\"text/javascript\" LANGUAGE=JAVASCRIPT>\n"
		  "<!--\n");

	fprintf(file, "function	CheckAndSend()\n"
		"{\n");

	WriteFormCheckLimit(file);

	fprintf(file, "	document.myform.submit();\n"
		 "}\n\n");

	fprintf(file, "function	FormRequest(requestId)\n"
		"{\n"
		" document.myform.request.value=requestId;\n"
		" document.myform.submit();\n"
		"}\n\n");

	fprintf(file, "function	CheckDouble(obj, limit,	min, max, stringError)\n"
		"{\n"
		" if((limit & 4) && !obj.value)\n"
		"  return true\n"
		" var re = /^-?\\d+\\.?\\d*e?-?\\d*$/\n"
		" if(!re.exec(obj.value) || !CheckLimit(obj.value, limit, min, max)){\n"
		"  alert(stringError)\n"
		"  obj.select()\n"
		"  return false\n"
		" }\n"
		" return true \n"
		"}\n\n");

	fprintf(file, "function	CheckInt(obj, limit, min, max, stringError)\n"
					"{\n"
					" if((limit & 4) && !obj.value)\n"
					"  return true\n"
					" var re = /^-?\\d+$/\n"
					" if(!re.exec(obj.value) || !CheckLimit(obj.value, limit, min, max)){\n"
					"  alert(stringError)\n"
					"  obj.select()\n"
					"  return false\n"
					" }\n"
					" return true\n"  
					"}\n\n");


	fprintf(file, "function	CheckString(obj, limit,	min, max, stringError)\n"
					"{\n"
					" if((limit	& 4) &&	!obj.value)\n"
					"  return true\n"
					" if(!CheckLimit(obj.value.length, limit, min, max)){\n"
					"  alert(stringError)\n"
					"  obj.select()\n"
					"  return false\n"
					" }\n"
					" return true \n"
					"}\n\n");

	fprintf(file, "function	CheckLimit(value, limit, min, max)\n"
					"{\n"
					" if((limit & 1) && (value < min))\n"
					"  return false\n"
					" if((limit & 2) && (value > max))\n"
					"  return false\n"
					" return true\n"
					"}\n\n");


	char *pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_script.txt"); // _script
	File expasyFile;
	if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
		WriteExpasyFile(file, expasyFile);
	}

	fprintf(file, "-->\n</script>\n\n");

}

/***********************************************************

   WriteFormCheckLimit

***********************************************************/
void PopitamForm::WriteFormCheckLimit(File &file)
{
	//popitam form
	m_dataFile.WriteCheckLimit(file, "Data file");
	//m_dataFile
	//m_formatList
    
	//m_database
	//m_taxonomy
	//m_taxonomyObj
	m_acList.WriteCheckLimit(file, "AC list");
    
	//m_modifications
	//m_instrument
	m_fragmentError1.WriteCheckLimit(file, "Fragment error");
	//m_enzyme
	//m_missedcleav

	m_scoreFile.WriteCheckLimit(file, "Scoring function file");
	//m_scoreFile

	//m_popitamRunMode
//	m_parentMError.WriteCheckLimit(file, "Precursor mass error");
	m_maxAddPM.WriteCheckLimit(file, "Precursor mass range. Maximal mass add");
	m_maxLossPM.WriteCheckLimit(file, "Precursor mass range. Maximal mass loss");
	m_maxAddModif.WriteCheckLimit(file, "Gap mass range. Maximal gap add");
	m_maxLossModif.WriteCheckLimit(file, "Gap mass range. Maximal gap loss");
    
	m_nbDisplay.WriteCheckLimit(file, "Results number");

	m_display.WriteCheckLimit(file);
}

/***********************************************************

   WriteLogo

Display	SIB	and	PIG	logos in a table
***********************************************************/

void PopitamForm::WriteLogo(File &file)
{
//	  fprintf(file,	"<div style=\"text-align: right;\">\n");
//	  fprintf(file,	"<img src=\"./image/pig_logo.gif\" alt=\"PIG\" width=\"175px\" height=\"100px\" align=\"right\">\n");
//	  fprintf(file,	"</div><br><br>\n");
	fprintf(file, "<table border=\"0\" width=\"100&#37;\"> <tbody>\n");
	fprintf(file, "<tr> <td align=\"left\">\n");
	fprintf(file, "<a href=\"http://www.isb-sib.ch\">\n");
	fprintf(file, "<img src=\"%simages/expasy_logos/siblogo.jpg\" alt=\"SIB\" width=\"175px\" height=\"100px\">\n", path()->pszRelativeServer);
	fprintf(file, "</a></td> <td align=\"right\">\n");
	fprintf(file, "<a href=\"http://www.expasy.org/people/pig/\">\n");
	fprintf(file, "<img src=\"%simages/expasy_logos/pig.gif\" alt=\"PIG\" width=\"175px\" height=\"100px\">\n", path()->pszRelativeServer);
	fprintf(file, "</a></td> </tr>\n");
	fprintf(file, "</tbody>	</table> \n");
}

//style
void PopitamForm::WriteStyle(File &file)
{

	fprintf(file, "<link rel=\"shortcut icon\" href=\"/favicon.ico\" type=\"image/x-icon\">\n");
	fprintf(file, "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n");
	fprintf(file, "<link rel=\"stylesheet\" href=\"/base.css\" type=\"text/css\">\n");
	fprintf(file, "<STYLE TYPE=\"text/css\">\n"
					"h1 {text-align: left;}\n"
					"body { color: #000000; background-color: #FFFFFF; }\n"
					"a:link	{ color:#000099; text-decoration: underline; }\n"
					"a:visited { color:#990099; text-decoration: underline; }\n"
					"a:active { color:#000000; text-decoration: underline; }\n"
					"A.blacklink:link { color: WindowText; text-decoration: none}\n"
					"A.blacklink:visited { color: WindowText; text-decoration: none}\n"
					"A.blacklink:hover { text-decoration: underline}\n"
					"table.popitableoutputform { background-color: #FFFFEA; }\n"
					"table.spectrum { background-color: #CCCCFF; border-width: 1px; width: 100%%; }\n"
					"</STYLE>\n" );
}

/***********************************************************

   WriteForm

PAS DE CARACTERES SPECIAUX TEL QUE ' OU " DANS LES MESSAGE UTILISATEUR MSG
A CAUSE	DU JAVASCRIPT

***********************************************************/
void PopitamForm::WriteForm(File &file,	const char *pszMsg)
{   
	fprintf(file, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
	fprintf(file, "<html lang=\"en-US\">\n<head>\n<title>ExPASy - Popitam</title>\n");
	WriteFormHead(file);
	WriteStyle(file);
	fprintf(file, "</head>\n");

	if(pszMsg)
		fprintf(file, "<body onLoad=\"alert('%s')\" >\n", pszMsg);
	else
		fprintf(file, "<body>\n");

	//Expasy
	char *pszExpFileName = StrdupFormat("%s%s",	path()->pszExpFilesDir,	"expasy_head.txt");	//_head	_script	_search	_foot
	File expasyFile;
	if(	expasyFile.OpenIfExist(pszExpFileName, "r")	) {
		WriteExpasyFile(file, expasyFile);
	}
	pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_search.txt");	// _script
	if(	expasyFile.OpenIfExist(pszExpFileName, "r")	) {
		WriteExpasyFile(file, expasyFile);
	}
    	
	
	//Header
	//WriteLogo(file);
    
	fprintf(file, "<h1>\n");
	fprintf(file, "<img src=\"%simages/expasy_logos/expasy.gif\" title=\"\" alt=\"\" width=\"100px\" height=\"125px\"> Popitam\n", path()->pszRelativeServer);
	fprintf(file, "</h1> Version of 06/15/2006 (<a href=\"%scgi-bin/popitam/help.pl?about.html\" target =\"_new\">release notes</a>)\n<br>\n", path()->pszRelativeServer);
    
	fprintf(file, "<h2>What	is Popitam ?</h2>");

	fprintf(file, "<p>Popitam is an	ongoing	work about protein characterization using tandem mass spectrometry data. A short description of the algorithm can be found <a href=\"%simage/posterPopitam.pdf\">here</a> (pdf document, about 2Mb)</p>\n", path()->pszHTMLdocs);

	fprintf(file, "<p>Popitam is a new MS/MS method designed to characterize peptides with mutations or unexpected post-translational modifications. Popitam performs an \"open-modification search\": it takes into account any type and number of differences between an MS/MS spectrum and theoretical peptides from UniProtKB/Swiss-Prot and/or UniProtKB/TrEMBL.</p>\n");

	fprintf(file, "<p><a href=\"%scgi-bin/popitam/help.pl\">More about Popitam</a></p>\n", path()->pszRelativeServer);
    
	// **************************************************************************
	//Table	form
	// **************************************************************************
	fprintf(file, "<a name=\"form\"></a>\n");
	fprintf(file, "<form ENCTYPE=\"multipart/form-data\" method=POST name=\"myform\" action=\"%scgi-bin/popitam/%s\">\n", path()->pszRelativeServer, PopitamForm::pszCgi);
	fprintf(file, "<input type=hidden name=%s value=%d>\n",	PopitamForm::pszRequest, REQUEST_SUBMIT);
    
	// **********
	//Examples
	// **********
	fprintf(file, "<table border=\"1\" cellpadding=\"10\" width=\"100&#37;\" bgcolor=\"#CCCCFF\" >\n<tbody>\n");
	fprintf(file, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\">\n");
//	fprintf(file, "<input type=button name=\"bExample1\" value=\"Load example 1\" onClick='javascript:FormRequest(2)'>\n");
//	fprintf(file, "</td>\n<td>\n");
//	fprintf(file, "<input type=button name=\"bExample2\" value=\"Load example 2\" onClick='javascript:FormRequest(3)'>\n");
//	fprintf(file, "</td>\n<td>\n");
	fprintf(file, "<input type=button name=\"bExample3\" value=\"Load example 1\" onClick='javascript:FormRequest(4)'>\n");
	fprintf(file, "</td>\n<td align=\"center\">\n");
	fprintf(file, "<input type=button name=\"bExample4\" value=\"Load example 2\" onClick='javascript:FormRequest(5)'>\n");
	fprintf(file, "</td>\n<td align=\"center\">\n");
	fprintf(file, "<input type=button name=\"bDefault\" value=\"Reset form\" onClick='javascript:FormRequest(6)'>\n");
	fprintf(file, "</td>\n<td align=\"center\">\n<a href=\"%scgi-bin/popitam/help.pl?help.html+examples\" target =\"_new\">About the examples</a></td>\n</tr>\n</tbody>\n</table>\n", path()->pszRelativeServer);
    

	fprintf(file, "<table border=\"1\" cellpadding=\"10\" width=\"100&#37;\" bgcolor=\"#CCCCFF\" >\n<tbody>\n");

	//First	row	: parameters' table
	fprintf(file, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\">\n");
	WriteFormParamMode(file);
//	fprintf(file, "</td>\n</tr>\n");

	//Second row : scoring function
//	fprintf(file, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\">\n");// cellpadding=\"10\" 
	WriteFormScoringMode(file);
//	fprintf(file, "</td>\n</tr>\n");

	//Third	row	: search mode
//	fprintf(file, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\">\n");// cellpadding=\"10\" 
	WriteFormRunningMode(file);
//	fprintf(file, "</td>\n</tr>\n");

	//Fourth row : output mode
//	fprintf(file, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\">\n");// cellpadding=\"10\" 
	WriteFormOutputMode(file);
	fprintf(file, "</td>\n</tr>\n");
	
	//
	fprintf(file, "</tbody>\n</table>\n");
	fprintf(file, "</form>\n");
	
	
	// **************************************************************************

	pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_foot.txt"); // _script _search 
	if(	expasyFile.OpenIfExist(pszExpFileName, "r")	) {
		WriteExpasyFile(file, expasyFile);
	}
    
	fprintf(file, "</body></html>\n");
}


//***************************************
//		Global parameters
//		data file, taxonomy...
//***************************************

void PopitamForm::WriteFormParamMode(File &file)
{
	char buffer[128];
	fprintf(file, "<table border=\"0\" cellpadding=\"10\" width=\"100&#37;\"><tbody>\n");

	//*************************************
	fprintf(file, "<tr><td bgcolor=\"#FFFFCC\" colspan=\"2\" align=\"center\"><b>\n");
	fprintf(file, "Input data parameters<br>\n");
//	fprintf(file, "<b>Caution : </b><br>\n");
//	fprintf(file, "<p>Popitam is <b>very gluttony</b>... In order to preserve computing capacities of our server, very strong filtering must be used on the database sequences. We fervently hope to correct this annoying issue, but until this, we kindly ask the user to use the AC list filter (maximum 2000 ACs).</p>\n");
	fprintf(file, "</b></td></tr>\n");
	//************************************* bgcolor=\"#FF9900\" orange
	fprintf(file, "<tr><td colspan=\"2\" align=\"center\">\n");

	fprintf(file, "<table border=\"0\"><tbody>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+data");
	m_dataFile.WriteLink(file, 50, "Data file :", buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	m_dataCopy.WriteLink(file, 7, 30, "Data file content :", buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+dataformat");
	m_formatList.WriteLink(file, 1, "Data file format : ", buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "</tbody></table>\n");
    
	fprintf(file, "</td></tr>\n");
	//*************************************
	fprintf(file, "<tr><td bgcolor=\"#FFFFCC\" colspan=\"2\" align=\"center\">\n");
	fprintf(file, "<b>Other general parameters</b>\n");
	fprintf(file, "</td></tr>\n");

	fprintf(file, "<tr><td>\n");

	fprintf(file, "<table border=\"0\"><tbody>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+database");
	m_database.WriteLink(file, 2, "Database	: ", buffer);
		const int	BUFF_LENGTH	= 256;
		char szBuffer[BUFF_LENGTH];
		char *pszDbreleaseFileName = StrdupFormat("%sdb_release.txt", path()->pszTaxonomy);
		File dbreleaseFile;
		dbreleaseFile.Open(pszDbreleaseFileName, "r");
		if(	fgets(szBuffer, BUFF_LENGTH, dbreleaseFile)!=NULL ) {
			char* releasePos = strstr(szBuffer, "Release");
			if(releasePos!=NULL) {
				char* pPos = strrchr(releasePos, ':');
				if(pPos!=NULL) {
					char szBuffer2[BUFF_LENGTH];
					int newLength = (int) (strlen(releasePos)-strlen(pPos));
					if(newLength>0)	{
						strncpy(szBuffer2, releasePos, newLength);
						szBuffer2[newLength]='\0';
						fprintf(file, "<td>%s<br>",	szBuffer2);
					}
					else { fprintf(file, "<td>Release unknown<br>"); }
				}
				else { fprintf(file, "<td>Release unknown<br>"); }
			}
			else { fprintf(file, "<td>Release unknown<br>"); }
		}
		else { fprintf(file, "<td>Release unknown<br>"); }
		if(	fgets(szBuffer,	BUFF_LENGTH, dbreleaseFile)!=NULL )	{
			char* releasePos = strstr(szBuffer,	"Release");
			if(releasePos!=NULL) {
				char* pPos = strrchr(releasePos, ':');
				if(pPos!=NULL) {
					char szBuffer2[BUFF_LENGTH];
					int	newLength =	(int) (strlen(releasePos)-strlen(pPos));
					if(newLength>0)	{
						strncpy(szBuffer2, releasePos, newLength);
						szBuffer2[newLength]='\0';
						fprintf(file, "%s</td>\n", szBuffer2);
					}
					else { fprintf(file, "Release unknown</td>\n");	}
				}
				else { fprintf(file, "Release unknown</td>\n");	}
			}
			else { fprintf(file, "Release unknown</td>\n");	}
		}
		else {
			fprintf(file, "Release unknown</td>\n");
		}
	fprintf(file, "</tr>\n");
/*	fprintf(file, "<tr><td>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+taxonomy");
	HRef(file, "Taxonomy : ", buffer);
	fprintf(file, "</td><td	colspan=\"2\">\n");
	m_taxonomyObj.WriteTaxon(file);
	fprintf(file, "</td></tr>\n");
*/	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+aclist");
	fprintf(file, "<td>");
	HRef(file, "AC list	(< 2000) : ", buffer);
	fprintf(file, "<br><b>Required parameter.</b>");
	fprintf(file, "</td><td	colspan=\"2\">");
	m_acList.Write(file,  7, 45);
	fprintf(file, "</td>\n");
	fprintf(file, "</tr>\n");
	fprintf(file, "</tbody></table>\n");
    
	fprintf(file, "</td><td>\n");
    
	fprintf(file, "<table border=\"0\"><tbody>\n");
	fprintf(file, "<tr><td colspan=\"2\">\n");
	fprintf(file, "Expected	post-translational modifications \n");
	fprintf(file, "</td></tr>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+exmodifs");
	m_fixedModifs.WriteLink(file, 8, "fixed	:",	buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	m_variableModifs.WriteLink(file, 7,	"variable :", buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "</tbody></table>\n");
	//fprintf(file, "</td></tr>\n");
    
	fprintf(file, "</td></tr>\n");
	//*************************************
	fprintf(file, "<tr><td valign=\"top\">\n");
    
	fprintf(file, "<table border=\"0\"><tbody>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+instrument");
	m_instrument.WriteLink(file, 1,	"Instrument	:",	buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+fragmenterr");
	m_fragmentError1.WriteLink(file, 10, "Fragment error :", buffer);
	fprintf(file, "<td>Da</td>\n");
	fprintf(file, "</tr>\n");
	fprintf(file, "</tbody></table>\n");
    
	fprintf(file, "</td><td>\n");

	fprintf(file, "<table border=\"0\"><tbody>\n");
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+enzyme");
	m_enzyme.WriteLink(file, 1,	"Enzyme	:",	buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	fprintf(file, "<td>Allow up	to </td><td>");
	m_missedcleav.Write(file, 1);
	fprintf(file, "</td><td>");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+missedcleav");
	HRef(file, "missed cleavage", buffer);
	fprintf(file, "</td></tr>\n");
	fprintf(file, "</tbody></table>\n");

	fprintf(file, "</td></tr>\n");
	//*************************************
	fprintf(file, "</tbody></table>\n");

}

//***************************************
//		Scoring	mode
//		which function
//***************************************

void PopitamForm::WriteFormScoringMode(File	&file)
{
	char buffer[128];
    // bgcolor=\"#FF9900\" orange
	fprintf(file, "<table border=\"0\" rules=\"none\" cellpadding=\"7\" width=\"100&#37;\"><tbody>\n");
	fprintf(file, "<tr><td bgcolor=\"#FFFFCC\" colspan=\"2\" align=\"center\"><b>Scoring function (facultative parameter)</b><br>If nothing is specified, popitam will use its default scoring functions.<br></td></tr>\n");
	fprintf(file, "</tbody></table>\n");

	fprintf(file, "<table border=\"0\" rules=\"none\" cellpadding=\"7\"><tbody>\n");
    
	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+scoring");
	m_scoreFile.WriteLink(file, 50, "Scoring file :", buffer);
	fprintf(file, "</tr>\n");
	fprintf(file, "<tr>\n");
	m_scoreCopy.WriteLink(file, 7, 30, "Scoring file content :", buffer);
	fprintf(file, "</tr>\n");
    
	fprintf(file, "</tbody></table>\n");
}

//***************************************
//		Running	mode
//		normal,	mutmod...
//***************************************

void PopitamForm::WriteFormRunningMode(File	&file)
{
	char buffer[128];
	fprintf(file, "<table border=\"0\" rules=\"none\" cellpadding=\"7\" width=\"100&#37;\"><tbody>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+runmode");
	fprintf(file, "<tr>\n<td bgcolor=\"#FFFFCC\" nowrap align=\"center\"><b><a href=\"%s\" target=\"_blank\">Open-modification search parameters</a></b></td>\n</tr>\n", buffer);
//	m_popitamRunMode.WriteLink(file, 1, "Search mode", buffer);
	fprintf(file, "</tbody></table>\n");

	fprintf(file, "<table border=\"0\" rules=\"none\" cellpadding=\"7\"><tbody>\n");

	fprintf(file, "<tr><td>\n");
	fprintf(file, "<table border=\"1\" rules=\"rows\" cellpadding=\"10\"><tbody>\n");
/*	  fprintf(file,	"<tr><td>\n");
	  m_popitamRunMode.GetItem(0)->Write(file, m_popitamRunMode.GetName());
	  fprintf(file,	"</td>\n");
	  fprintf(file,	"<td>\n");
	  fprintf(file,	"<table	border=\"0\" rules=\"none\" cellpadding=\"3\"><tbody>\n");
	  fprintf(file,	"<tr>\n");
	  strcpy(buffer, path()->pszRelativeServer);
	  strcat(buffer, "cgi-bin/popitam/help.pl?help.html+parenterr");
	  m_parentMError.WriteLink(file, 10, "Precursor	mass error :", buffer);
	  fprintf(file,	"<td>Da</td>\n");
	  fprintf(file,	"</tr>\n");
	  fprintf(file,	"</tbody></table>\n");
	  fprintf(file,	"</td></tr>\n");
*/	  
	  fprintf(file,	"<tr>\n");
//	  fprintf(file,	"<td>\n");
//	  m_popitamRunMode.GetItem(1)->Write(file, m_popitamRunMode.GetName());
//	  fprintf(file,	"</td>\n");
	  fprintf(file,	"<td>\n");
	  fprintf(file,	"<table	border=\"0\" rules=\"none\" cellpadding=\"3\"><tbody>\n");
	  fprintf(file,"<tr>\n");
	  strcpy(buffer, path()->pszRelativeServer);
	  strcat(buffer, "cgi-bin/popitam/help.pl?help.html+modGapNb");
	  m_gapMax.WriteLink(file, 1, "modGap number :", buffer);
	  fprintf(file,	"</tr>\n<tr>\n<td>");
	  strcpy(buffer, path()->pszRelativeServer);
	  strcat(buffer, "cgi-bin/popitam/help.pl?help.html+parenterr");
	  HRef(file, "Precursor	mass range", buffer);
	  fprintf(file,	"</td>\n<td>");
	  m_maxLossPM.Write(file, 10);
	  fprintf(file,	"</td>\n<td>to</td>\n<td>");
	  m_maxAddPM.Write(file, 10);
	  fprintf(file,	"</td>\n<td>Da</td>\n</tr>\n<tr><td>");
	  strcpy(buffer, path()->pszRelativeServer);
	  strcat(buffer, "cgi-bin/popitam/help.pl?help.html+gapmass");
	  HRef(file, "modGap mass range", buffer);
	  fprintf(file,	"</td>\n<td>");
	  m_maxLossModif.Write(file, 10);
	  fprintf(file,	"</td>\n<td>to</td>\n<td>");
	  m_maxAddModif.Write(file,	10);
	  fprintf(file,	"</td>\n<td>Da</td></tr>\n");
	  fprintf(file,	"</tbody></table>\n");
	  fprintf(file,	"</td></tr>\n");
	fprintf(file, "</tbody>\n</table>\n");
	fprintf(file, "</td>\n</tr>\n");
    
	fprintf(file, "</tbody>\n</table>\n");
}

//***************************************
//		Output infos in	html file
//		nb of results, email...
//***************************************

void PopitamForm::WriteFormOutputMode(File &file)
{
	fprintf(file, "<table border=\"0\" rules=\"none\" cellpadding=\"7\" width=\"100&#37;\"><tbody>\n");
	fprintf(file, "<tr><td bgcolor=\"#FFFFCC\" colspan=\"2\" align=\"center\"><b>Output parameters</b></td></tr>\n");
	fprintf(file, "</tbody></table>\n");

	fprintf(file, "<table border=\"0\" cellpadding=\"7\">\n<tbody>\n");

	fprintf(file, "<tr>\n");
	char buffer[128];
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+toppeptides");
	m_nbDisplay.WriteLink(file, 10, "Number of displayed peptides :", buffer);
	fprintf(file, "</tr>\n");

	fprintf(file, "<tr>\n");
	strcpy(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+mail");
	m_display.WriteLink(file, buffer);
	fprintf(file, "</tr>\n");

	fprintf(file, "</tbody>\n</table>\n");
	fprintf(file, "<table border=\"0\" cellpadding=\"7\">\n<tbody>\n");

	fprintf(file, "<tr><td align=\"center\">Please note that submissions are limited in time to 5 minutes.<br>For longer jobs, please specify your e-mail adress in the coresponding field, so that results can be sent.</td></tr>\n");
	fprintf(file, "<tr><td align=\"center\">\n");
	fprintf(file, "<input type=button value=\"Submit\" onClick='CheckAndSend()'>\n");//<input type=\"reset\" value=\"Reset\">&nbsp;&nbsp;
	fprintf(file, "</td>\n</tr>\n");
	
	fprintf(file, "</tbody>\n</table>\n");

}

//***************************************
//		Parameters infos in	output display
//	    
//***************************************

void PopitamForm::WriteOutputParam(File	&file, bool	isStdout)
{
	char* server = "";
	if(!isStdout) server = path()->pszServer;
    
	char buffer[128];
	fprintf(file, "<h2>Parameters</h2><br>\n");

	//rappel popitam form
	fprintf(file, "<table class=\"popitableoutputform\" border=\"1\" cellpadding=\"5\">\n");//cellpadding=10

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+data");
	HRefU(file, "Data file name", buffer);
	if( !strlen(GetDataFileName()) ) {
		fprintf(file, "</td>\n<td>Pasted data</td>\n</tr>\n");
	}
	else {
		fprintf(file, "</td>\n<td>%s</td>\n</tr>\n", GetDataFileName());
	}

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+database");
	HRefU(file, "Database", buffer);
	fprintf(file, "</td><td>");
		char szBuffer[BUFF_LENGTH];
		char *pszDbreleaseFileName = StrdupFormat("%sdb_release.txt", path()->pszTaxonomy);
		char* SPRelease	= NULL;
		char* TRRelease	= NULL;
		File dbreleaseFile;
		dbreleaseFile.Open(pszDbreleaseFileName, "r");
		if( fgets(szBuffer, BUFF_LENGTH, dbreleaseFile)!=NULL )	{
			char* releasePos = strstr(szBuffer, "Release");
			if(releasePos!=NULL) {
				char* pPos = strrchr(releasePos, ':');
				if(pPos!=NULL) {
					int newLength = (int)(strlen(releasePos)-strlen(pPos));
					if(newLength>0)	{
						SPRelease = new char[newLength+1];
						strncpy(SPRelease, releasePos, newLength);
						SPRelease[newLength]='\0';
					}
					else { SPRelease = "Release unknown"; }
				}
				else { SPRelease = "Release unknown"; }
			}
			else { SPRelease = "Release unknown"; }
		}
		else { SPRelease = "Release unknown"; }
		if( fgets(szBuffer, BUFF_LENGTH, dbreleaseFile)!=NULL ) {
			char* releasePos = strstr(szBuffer, "Release");
			if(releasePos!=NULL) {
				char* pPos = strrchr(releasePos, ':');
				if(pPos!=NULL) {
					int newLength = (int)(strlen(releasePos)-strlen(pPos));
					if(newLength>0)	{
						TRRelease = new char[newLength];
						strncpy(TRRelease, releasePos, newLength);
						TRRelease[newLength]='\0';
					}
					else { TRRelease = "Release unknown"; }
				}
				else { TRRelease = "Release unknown"; }
			}
			else { TRRelease = "Release unknown"; }
		}
		else {
			TRRelease = "Release unknown";
		}
	if(m_database.GetItem(0)->IsSelected())	{ //Swiss-Prot
		fprintf(file, "%s ", m_database.GetItem(0)->GetString());
		if(SPRelease!=NULL)	{
			fprintf(file, "%s<br>",	SPRelease);
		}
		else {
			fprintf(file, "<br>");
		}
	}
	if(m_database.GetItem(1)->IsSelected())	{
		fprintf(file, "%s ", m_database.GetItem(1)->GetString());
		if(TRRelease!=NULL)	{
			fprintf(file, "%s", TRRelease);
		}
	}
	fprintf(file, "</td></tr>\n");

/*
	for(Item *pItem=m_database.GetFirstSelectedItem(); pItem; pItem=m_database.GetNextSelectedItem()) {
		fprintf(file, "%s<br>",	pItem->GetString());
		if(pItem->GetId()==0 &&	SPRelease!=NULL) { //Swiss-Prot
			fprintf(file, "%s",	SPRelease);
		}
		else if(pItem->GetId()==1) { //TrEMBL
			fprintf(file, "%s",	TRRelease);
		}
	}

*/

/*	  fprintf(file,	"<tr><td>");
	HRefU(file, "Taxonomy", "help.html#taxonomy");
	fprintf(file, "</td><td>");
	fprintf(file, "%s<br>",	GetTaxonomyStr());
	fprintf(file, "</td></tr>\n");
*///	  m_taxonomyObj.WriteHtmlInputSummary(file, true);

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+aclist");
	HRefU(file, "AC list", buffer);
	int length = (int)strlen(m_acList.GetString());
	if(length == 0)	{
		fprintf(file, "</td><td>No AC list given.</td></tr>\n");
	}
	else {
		char* tmpPtr = NULL;
		char* tempText = Strdup(m_acList.GetString());
		//rub out unused ACs from the AC text list (tempText)
		for(int i=0; i<unusedACnb; i++) {
			while( (tmpPtr=strstr(tempText, unusedAC[i])) ){ 
				strncpy(tmpPtr, "      ", 6);
			}
		}

		//parse tempText to 
		char* ptr = NULL;
		int usedAC =0;
		fprintf(file, "</td><td>");
		for( ptr=strtok(tempText, " \t\n\r"); ptr; ptr=strtok( NULL, " \t\n\r") ) {
			usedAC++;
			if(usedAC >10) break; //then some '...' will be added at the end of the line
			fprintf(file, "%s ", ptr);
		}
		if(usedAC==0) {
			fprintf(file, "NONE OF THE GIVEN ACs COULD BE USED !! (check taxonomy selection...) ");
		}
		if(usedAC>10) {
                        fprintf(file, "...");
		}

		fprintf(file, "</td></tr>\n");
	}

	if(unusedACnb!=0) {
		fprintf(file, "<tr><td>Unused AC</td><td>");
		for(int i=0; i<unusedACnb; i++) {
			fprintf(file, "%s ", unusedAC[i]);
		}
		fprintf(file, "</td></tr>");
	}

/*	  fprintf(file,	"<tr><td>");
	HRefU(file,	"Modifications", "help.html#modification");
	fprintf(file, "</td><td>");
	for(Item *pItem=m_modifications.GetFirstSelectedItem();	pItem; pItem=m_modifications.GetNextSelectedItem())
		fprintf(file, "%s<br>",	pItem->GetString());
	fprintf(file, "</td></tr>\n");
*/
	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+instrument");
	HRefU(file,	"Instrument", buffer);
	fprintf(file, "</td><td	>%s</td></tr>\n", GetInstrument());
    
	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+fragmenterr");
	HRefU(file,	"Fragment error", buffer);
	fprintf(file, "</td><td>%s</td></tr>\n", GetFormattedFragmentError1());

//	fprintf(file, "<tr><td>");
//	strcpy(buffer, server);
//	strcat(buffer, path()->pszRelativeServer);
//	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+runmode");
//	HRefU(file,	"Search mode",	buffer);
//	fprintf(file, "</td><td>%s<br></td></tr>\n", GetRunMode());

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+modGapNb");
	HRefU(file, "ModGap number", buffer);
        if(GetMutMode()) {
		fprintf(file, "</td><td>%d<br></td></tr>\n", GetGapMax());
	}
	else {
		fprintf(file, "</td><td>0<br></td></tr>\n");
	}

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+scoring");
	HRefU(file, "Scoring file name", buffer);
	if(	strlen(GetScoreFileName()) ) {
		fprintf(file, "</td>\n<td>%s</td>\n</tr>\n", GetScoreFileName());
	}
	else if( strlen(GetScoreCopy())	) {
		fprintf(file, "</td>\n<td>Pasted scoring function</td>\n</tr>\n");
	}
	else {
		fprintf(file, "</td>\n<td>Default</td>\n</tr>\n");
	}

//	if(!GetMutMode()) {
/*		  fprintf(file,	"<tr><td>");
		HRefU(file,	"Precursor mass	error",	"help.html#parenterr");
		fprintf(file, "</td><td>%s</td></tr>\n", FormatDouble(szBuffer,	m_parentMError.Get()));
		//attention	a FormatDouble,	pas	deux fois dans le meme printf avec le meme buffer !!!
*///	  }
//	else {
/*		  fprintf(file,	"<tr><td>");
		HRefU(file,	"Max add Precursor mass", "help.html#maxAddPM");
		fprintf(file, "</td><td>%s</td></tr>\n", FormatDouble(szBuffer,	m_maxAddPM.Get()));
*//*		fprintf(file, "<tr><td>");
		HRefU(file,	"Max loss Precursor	mass", "help.html#fragmenterr");
		fprintf(file, "</td><td>%s</td></tr>\n", FormatDouble(szBuffer,	m_maxLossPM.Get()));
*//*		fprintf(file, "<tr><td>");
		HRefU(file,	"Max add Modification",	"help.html#maxAddModif");
		fprintf(file, "</td><td>%s</td></tr>\n", FormatDouble(szBuffer,	m_maxAddModif.Get()));
*//*		fprintf(file, "<tr><td>");
		HRefU(file,	"Max loss Modification", "help.html#maxLossModif");
		fprintf(file, "</td><td>%s</td></tr>\n", FormatDouble(szBuffer,	m_maxLossModif.Get()));
*/
//	}

	fprintf(file, "<tr><td>");
	strcpy(buffer, server);
	strcat(buffer, path()->pszRelativeServer);
	strcat(buffer, "cgi-bin/popitam/help.pl?help.html+toppeptides");
	HRefU(file, "Number of displayed peptides", buffer);
	fprintf(file, "</td><td>%d</td></tr>\n", GetDisplayNb());
    
//	  m_display.WriteOutput(file);

	fprintf(file, "\n</table>\n<br>\n");

}


/***********************************************************

   Accessors

***********************************************************/
bool PopitamForm::IsMailUsed(void) {
	return m_display.IsMailUsed();
}

const char*	PopitamForm::GetMail(void) {
	return m_display.GetMail();
}

int	PopitamForm::GetDisplayNb(void)	{
	return m_nbDisplay.Get();
}

const char*	PopitamForm::GetDataFile(void) {
	return m_dataFile.GetString();
}
const char*	PopitamForm::GetDataFileName(void) {
	return m_dataFile.GetFileName();
}

const char*	PopitamForm::GetDataCopy(void) {
	return m_dataCopy.GetString();
}

void PopitamForm::SetDataCopy(const	char* pszData) {
	m_dataCopy.Set(pszData);
}

const char*	PopitamForm::GetDataFormat(void) {
	return m_formatList.GetFirstSelectedItem()->GetString();
}

/**
 * Renvoie la DB sélectionnée dans un format utilisable	pour le	popparam
 * = SWISSPROT ou TREMBL ou	SPROTREMBL pour	les	deux
 * Part	du principe	qu'il n'y a	que	deux éléments sélectionnables,
 *	le premier "UniProtKB/Swiss-Prot" et le	deuxieme "UniProtKB/TrEMBL".
 */
const char*	PopitamForm::GetDatabase(void) {
	if(m_database.GetNbSelectedItem()==2) {
		return "SPROTREMBL";
	}
	else {//SWISSPROT or TREMBL
		if(m_database.GetFirstSelectedItem()->GetId()==0) {
			return "SWISSPROT";
		}
		else {
			return "TREMBL";
		}
	}
}

const char*	PopitamForm::GetScoreFile(void)	{
	return m_scoreFile.GetString();
}

const char*	PopitamForm::GetScoreFileName(void)	{
	return m_scoreFile.GetFileName();
}

const char*	PopitamForm::GetScoreCopy(void)	{
	return m_scoreCopy.GetString();
}

void PopitamForm::SetScoreCopy(const char* pszData)	{
	m_scoreCopy.Set(pszData);
}

int	PopitamForm::GetMutMode(void) {
/*	Radio* item	= m_popitamRunMode.GetSelectedItem();
	if(item!=NULL){
		return (!item->GetId())?
		0:
		m_gapMax.GetFirstSelectedItem()->GetId()+1;
	}
	return -1;
	*/
	return m_gapMax.GetFirstSelectedItem()->GetId()+1;
	
}

/**
 * 2 run modes only.
 */
const char*	PopitamForm::GetRunMode(void) {
/*	int	id = m_popitamRunMode.GetSelectedItem()->GetId();
	if(id==0) return "Normal";
	else return	"Open search";*/
	return	"Open search";
//was	 return	m_popitamRunMode.GetSelectedItem()->GetString();
}

/*void PopitamForm::SetRunMode(int runMode) {
	Radio* item	= m_popitamRunMode.GetSelectedItem();
	if(item!=NULL){
		item->Select(false);
	}
	m_popitamRunMode.Select(runMode);
}*/

const char*	PopitamForm::GetTaxonomyStr(void) {
	return m_taxonomyObj.GetFirstSelectedItem()->GetString();
//	  const	char* taxoString = m_taxonomy.GetFirstSelectedItem()->GetString();
//	  while(*taxoString=='.' &&	*taxoString!='\0') taxoString++;
//	  return taxoString;
}

int	PopitamForm::GetTaxonomyId(void) {
	return m_taxonomyObj.GetFirstSelectedItem()->GetId();
}

const char*	PopitamForm::GetACFilter(void) {
	return strlen(m_acList.GetString())>0?
		m_acList.GetString():
		"NO";
}

const char*	PopitamForm::GetInstrument(void) {
	return m_instrument.GetFirstSelectedItem()->GetString();
}

double PopitamForm::GetFragmentError1(void)	{
	return m_fragmentError1.Get();
}

const char*	PopitamForm::GetFormattedFragmentError1(void) {
	char szBuffer[128];
	return FormatDouble(szBuffer, m_fragmentError1.Get());
	//attention	a FormatDouble,	pas	deux fois dans le meme printf avec le meme buffer !!!
}


double PopitamForm::GetFragmentError2(void)	{
	double dFragErr1 = m_fragmentError1.Get();
	return (dFragErr1<0.75)?dFragErr1*2.0:1.5;
}

int	PopitamForm::GetMissedCleavage(void) {
	return m_missedcleav.GetFirstSelectedItem()->GetId();
}

int	PopitamForm::GetGapMax(void) {
	return m_gapMax.GetFirstSelectedItem()->GetId()+1;
}

void PopitamForm::SetGapMax(int	nbGap) {
	if(nbGap>0)	{
		Item* pItem	= m_gapMax.GetFirstSelectedItem();
		if(pItem !=NULL) pItem->Select(false);
		m_gapMax.Select(nbGap-1, true);
	}
}

double PopitamForm::GetMaxAddPM(void) {
	return m_maxAddPM.Get();
}
double PopitamForm::GetMaxLossPM(void) {
	return m_maxLossPM.Get();
}
double PopitamForm::GetMaxAddModif(void) {
	return m_maxAddModif.Get();
}
double PopitamForm::GetMaxLossModif(void) {
	return m_maxLossModif.Get();
}

