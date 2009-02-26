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
	Created on         : 19.01.2006
  Last modification  : 27.03.2008
	Comments           : 
	
***********************************************************/

#include "dbentry.h"
#include "dbfile.h"

#include <string.h>
#include <limits.h>


/******************************************************

		ENCODE

*******************************************************/

//15 caracteres les plus frequents dans les sequences sont encodes sur 4 bits
const char DBEntry::decodeTable1[15] = {'L', 'A', 'S', 'G', 'V', 'E', 'K', 'I', 'T', 'D', 'R', 'P', 'N', 'Q', 'F'};

//8 caracteres les moins frequents dans les sequences sont encodes sur 8 bits
const char DBEntry::decodeTable2[8] = {'Y', 'M', 'H', 'C', 'W', 'X', 'B', 'Z'};



/******************************************************

		DECODE

*******************************************************/

//false = encode sur 4 bits
//true = encode sur 8 bits
const bool DBEntry::encodeTable1[26] = {	true,		//A
																					false,	//B
																					false,	//C
																					true,		//D
																					true,		//E
																					true,		//F
																					true,		//G
																					false,	//H
																					true,		//I
																					false,	//J		inutilise
																					true,		//K
																					true,		//L
																					false,	//M
																					true,		//N
																					false,	//O		inutilise
																					true,		//P
																					true,		//Q
																					true,		//R
																					true,		//S
																					true,		//T
																					false,	//U		inutilise
																					true,		//V
																					false,	//W
																					false,	//X
																					false,	//Y
																					false		//Z
																			};	


//index dans decodeTable1 ou decodeTable2 
const unsigned char DBEntry::encodeTable2[26] = {	1,	//A
																									6,	//B
																									3,	//C
																									9,	//D
																									5,	//E
																									14, //F
																									3, 	//G
																									2, 	//H
																									7, 	//I
																									5, 	//J		inutilise	pointe par defaut sur X
																									6, 	//K
																									0, 	//L
																									1, 	//M
																									12, //N
																									5,	//O		inutilise	pointe par defaut sur X
																									11, //P
																									13, //Q
																									10, //R
																									2, 	//S
																									8, 	//T
																									5, 	//U 	inutilise	pointe par defaut sur X
																									4, 	//V
																									4, 	//W
																									5, 	//X
																									0, 	//Y
																									7		//Z
																								};







/******************************************************

		Constructor

*******************************************************/
DBEntry::DBEntry(void) 
{	
}


/******************************************************

		Destructor

*******************************************************/
DBEntry::~DBEntry(void)
{
}


/***********************************************************

	Read

***********************************************************/

bool DBEntry::Read(FILE *pFile, unsigned int *puiOffset, DBFile *pDBFile)
{
	ResetForRead();

	m_uiOffset	= *puiOffset;					
	m_iNbByte		= 0;

	//header
	if(fread(&m_stEntryHeader, sizeof(TS_EntryHeader), 1, pFile) != 1)
		return false;

	m_iNbByte = sizeof(TS_EntryHeader);

	//AC
	m_iNbByte += m_aAC.Read(pFile, m_stEntryHeader.usAC_Size);
	m_aAC.Add('\0');

	//ID
	m_iNbByte += m_aID.Read(pFile, m_stEntryHeader.usID_Size);
	m_aID.Add('\0');

	//DE
	m_iNbByte += m_aDE.Read(pFile, m_stEntryHeader.usDE_Size);
	m_aDE.Add('\0');


	//Keyword
	m_iNbByte += m_aKW.Read(pFile, m_stEntryHeader.usKW_Size) * sizeof(unsigned short);

	//PTM
	m_iNbByte += m_aPtmRef.Read(pFile,	m_stEntryHeader.usPTM_Size) * sizeof(TS_PtmRef);

	//Sequence
	ReadSQ(pFile);
	

	*puiOffset += m_iNbByte;


	//Transform PtmRef info into Ptm info
	m_aPTM.SetNbElt(m_aPtmRef.GetNbElt());

	for(int i=0; i<m_aPtmRef.GetNbElt(); i++){
		m_aPTM[i].iPos	= m_aPtmRef[i].iPos;
		m_aPTM[i].pPtm	= pDBFile->GetPtm(m_aPtmRef[i].iId);
	}
	

	return true;
}



/***********************************************************

	Write

***********************************************************/

void DBEntry::Write(FILE *pFile, unsigned int *puiOffset)
{
	//Transform Ptm info into PtmRef
	m_aPtmRef.SetNbElt(m_aPTM.GetNbElt());

	for(int i=0; i<m_aPTM.GetNbElt(); i++){
		m_aPtmRef[i].iPos	= m_aPTM[i].iPos;
		m_aPtmRef[i].iId	= m_aPTM[i].pPtm->GetId();
	}



	assert(m_aAC.GetNbElt() -1 <= USHRT_MAX);
	assert(m_aID.GetNbElt() -1 <= USHRT_MAX);
	assert(m_aDE.GetNbElt() -1 <= USHRT_MAX);
	assert(m_aSQ.GetNbElt() -1 <= USHRT_MAX);
	assert(m_aKW.GetNbElt() <= USHRT_MAX);
	assert(m_aPtmRef.GetNbElt() <= USHRT_MAX);

	m_uiOffset	= *puiOffset;				
	m_iNbByte		= 0;

	m_stEntryHeader.usAC_Size		= m_aAC.GetNbElt() -1;	//sans le null de fin de string
	m_stEntryHeader.usID_Size		= m_aID.GetNbElt() -1;	//sans le null de fin de string
	m_stEntryHeader.usDE_Size		= m_aDE.GetNbElt() -1;	//sans le null de fin de string
	m_stEntryHeader.usSQ_Size		= m_aSQ.GetNbElt() -1;	//sans le null de fin de string
	m_stEntryHeader.usKW_Size		= m_aKW.GetNbElt();
	m_stEntryHeader.usPTM_Size	= m_aPtmRef.GetNbElt();

	//header
	fwrite(&m_stEntryHeader, sizeof(TS_EntryHeader), 1, pFile);
	m_iNbByte = sizeof(TS_EntryHeader);

	//AC
	m_iNbByte += m_aAC.Write(pFile, 0, m_stEntryHeader.usAC_Size);

	//ID
	m_iNbByte += m_aID.Write(pFile, 0, m_stEntryHeader.usID_Size);

	//DE
	m_iNbByte += m_aDE.Write(pFile, 0, m_stEntryHeader.usDE_Size);

	//Keyword
	m_aKW.Sort(DBEntry::SortKeyword);
	m_iNbByte += m_aKW.Write(pFile) * sizeof(unsigned short);

	
	

	//PTM
	m_aPtmRef.Sort(DBEntry::SortPtmByPos);
	m_iNbByte += m_aPtmRef.Write(pFile) * sizeof(TS_PtmRef);

	
	//Sequence
	WriteSQ(pFile);
	
	

	*puiOffset += m_iNbByte;
}


/***********************************************************

   ResetForRead

***********************************************************/
void DBEntry::ResetForRead(void)
{
	m_aAC.Reset();
	m_aID.Reset();
	m_aDE.Reset();
	m_aKW.Reset();
	m_aPtmRef.Reset();
	m_aSQ.Reset();
}

/***********************************************************

   ResetForWrite

***********************************************************/
void DBEntry::ResetForWrite(void)
{
	memset(&m_stEntryHeader, 0, sizeof(TS_EntryHeader));

	m_aAC.Reset();
	m_aID.Reset();
	m_aDE.Reset();
	m_aKW.Reset();
	m_aPtmRef.Reset();
	m_aSQ.Reset();
}

/***********************************************************

   ReadSQ

***********************************************************/
void DBEntry::ReadSQ(FILE *pFile)
{
	m_aSQ.Reset();
	m_aSQ.EnsureSize(m_stEntryHeader.usSQ_Size + 2);	//on peut inserer un caractere de plus + le null de fin
	char *psz = m_aSQ;

	unsigned char ucByte, ucHightBits, ucLowBits;

	bool bEchap			= false;
	int iNbAARead		= 0;
	int iEnd				= m_stEntryHeader.usSQ_Size;

	//cette boucle peut lire 2 AA on peut donc a la fin de la sequence ajouter un AA de trop 
	//(poid faible du dernier octet qui reste ne doit pas forcement etre decode)
	while(iNbAARead < iEnd){
		
		fread(&ucByte, 1, 1, pFile);
		m_iNbByte++;

		ucHightBits	= ucByte >> 4;	//4 bits de poids fort de l'octet
		ucLowBits		= ucByte & 0xF;	//4 bits de poids faible de l'octet

		if(bEchap){		
			*psz++ = decodeTable2[ucHightBits];
			iNbAARead++;
			bEchap = false;
		}else{
			if(ucHightBits == 0xF){
				bEchap = true;
			}else{
				*psz++ = decodeTable1[ucHightBits];
				iNbAARead++;
			}
		}

		if(bEchap){
			*psz++ = decodeTable2[ucLowBits];
			iNbAARead++;
			bEchap = false;
		}else{

			if(ucLowBits == 0xF){
				bEchap = true;	
			}else{
				*psz++ = decodeTable1[ucLowBits];
				iNbAARead++;
			}
		}
	}

	//on place la fin au bon endroit meme si un caractere de trop
	m_aSQ.SetNbElt(m_stEntryHeader.usSQ_Size);

	//null de fin de string
	m_aSQ.Add('\0');
}



/***********************************************************

   WriteSQ

***********************************************************/
void DBEntry::WriteSQ(FILE *pFile)
{
	unsigned char ucByte;

	int iChar;
	char cEchap = 0xF;
	bool bWriteHightBits = true;

	for(int i=0; i<m_stEntryHeader.usSQ_Size; i++){
		iChar = m_aSQ[i] - 'A';

		if(encodeTable1[iChar]){

			//encodage sur 4 bits
			if(bWriteHightBits){
				ucByte = encodeTable2[iChar] << 4;
				bWriteHightBits = false;
			}else{
				ucByte += encodeTable2[iChar];
				fwrite(&ucByte, 1, 1, pFile);			//ecrit un octet dans le fichier
				m_iNbByte++;
				bWriteHightBits = true;
			}

		}else{
			
			//encodage sur 8 bits
			if(bWriteHightBits){
				ucByte = cEchap << 4;
				ucByte += encodeTable2[iChar];
				fwrite(&ucByte, 1, 1, pFile);			//ecrit un octet dans le fichier
				m_iNbByte++;
			}else{
				ucByte += cEchap;
				fwrite(&ucByte, 1, 1, pFile);			//ecrit un octet dans le fichier
				m_iNbByte++;
				ucByte = encodeTable2[iChar] << 4;
			}
		}
	}

	//dernier octet
	if(!bWriteHightBits){
		fwrite(&ucByte, 1, 1, pFile);	
		m_iNbByte++;
	}
}


/***********************************************************

   Print

***********************************************************/
void DBEntry::Print(void)
{
	printf("AC=%s ", &m_aAC[0]);
	printf("ID=%s ", &m_aID[0]);
	printf("taxid=%d ", m_stEntryHeader.iTaxId);
	printf("mw=%d ", m_stEntryHeader.iMw);
	printf("pi=%.2f\n", GetPi());

	printf("DE=%s\n", &m_aDE[0]);


	printf("childID=%d\n", m_stEntryHeader.ucChildId);
	
	printf("type=");
	if(IsChain())
		printf("Chain ");
	if(IsFragment())
		printf("Fragment ");
	if(IsVarsplice())
		printf("Varsplice ");
	printf("\n");

	printf("childStart=%d childEnd=%d\n", m_stEntryHeader.usChildStart, m_stEntryHeader.usChildEnd);


	int i;
	printf("Kw=");
	for(i=0; i<m_aKW.GetNbElt(); i++)
		printf("%d ", m_aKW[i]);
	printf("\n");

	printf("Ptm=");
	for(i=0; i<m_aPTM.GetNbElt(); i++)
		printf("(%s %.2f %d) ", m_aPTM[i].pPtm->GetLabel(), m_aPTM[i].pPtm->GetMass(), m_aPTM[i].iPos);
	printf("\n");

	printf("SQ=%s\n\n\n", &m_aSQ[0]);
}



/***********************************************************

   SetSQ

 ***********************************************************/
void DBEntry::SetSQ(const char *pszSQ)
{
	m_aSQ.Reset();
	m_aSQ.Add((char *)pszSQ, (int)strlen(pszSQ)+1);
}


/***********************************************************

   SetAC

 ***********************************************************/
void DBEntry::SetAC(const char *pszAC)
{
	m_aAC.Reset();
	m_aAC.Add((char *)pszAC, (int)strlen(pszAC)+1);
}


/***********************************************************

   SetID

 ***********************************************************/
void DBEntry::SetID(const char *pszID)
{
	m_aID.Reset();
	m_aID.Add((char *)pszID, (int)strlen(pszID)+1);
}


/***********************************************************

   SetDE

 ***********************************************************/
void DBEntry::SetDE(const char *pszDE)
{
	m_aDE.Reset();

	int iLen = (int)strlen(pszDE);

	if(iLen > 60){
		m_aDE.Add((char *)pszDE, 57);
		m_aDE.Add('.');
		m_aDE.Add('.');
		m_aDE.Add('.');
		m_aDE.Add('\0');
	}else{
		m_aDE.Add((char *)pszDE, iLen+1);
	}
}

void DBEntry::Set(	const char *pszAC, const char *pszID, const char *pszDE, const char *pszSQ, 
										int iTaxId, int iMw, double dPi, unsigned char ucType, int iChildId, int iChildStart, int iChildEnd,
										DynamicArray<unsigned short> &aKW, DynamicArray<TS_Ptm> &aPtm)
{

	ResetForWrite();

	SetAC(pszAC);
	SetID(pszID);
	SetDE(pszDE);
	SetSQ(pszSQ);
	SetTaxId(iTaxId);
	SetMw(iMw);
	SetPi(dPi);
	SetType(ucType);
	SetChildId(iChildId);
	SetChildStart(iChildStart);
	SetChildEnd(iChildEnd);
	SetKeyword(aKW);
	SetPtm(aPtm);
}

