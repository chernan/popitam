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
  Last modification  : 19.01.2006
  Comments           : 

***********************************************************/

#ifndef __DBENTRY_H__
#define __DBENTRY_H__

#include "dynamicarray.h"
#include "file.h"
#include "dbptm.h"

#pragma pack(1)

typedef struct{
	int		iTaxId;			//taxid de NEWT
	int		iMw;			//mass arrondie a l'entier (somme des masses AVERAGE des AA)
	unsigned short	usPi;			//Pi : (2 chiffres apres la virgule * 100) exemple 456 pour 4.56
	unsigned char	ucType;			//fragment, chain, varsplice...
	unsigned char	ucChildId;		//nieme entree demergee depuis une entree Swiss-Prot
	unsigned short	usChildStart;		//position start sur la sequence de l'entree Swiss-Prot
	unsigned short	usChildEnd;		//position end sur la sequence de l'entree Swiss-Prot
	unsigned short	usKW_Size;		//nbre de keyword
	unsigned short	usPTM_Size;		//nbre de PTM
	unsigned short	usSQ_Size;		//taille de la sequence (sans le null de fin de string)
	unsigned short	usDE_Size;		//nbre de caracteres de description (sans le null de fin de string)
	unsigned short	usAC_Size;		//nbre de caracteres de AC (sans le null de fin de string)	
	unsigned short	usID_Size;		//nbre de caracteres de ID (sans le null de fin de string)	
}TS_EntryHeader;


typedef struct{
	int		iPos;			//position sur la sequence
	int		iId;			//id 
}TS_PtmRef;

#pragma pack()

//champ de bytes
enum{
	TYPE_UNDEFINED	= 0,
	TYPE_FRAGMENT	= 1,
	TYPE_CHAIN	= 2,
	TYPE_VARSPLICE	= 4
};

typedef struct {
	int		iPos;			//position sur la sequence
	DBPtm		*pPtm;			//Ptm
}TS_Ptm;

class DBFile;

/******************************************************

Class DBEntry

*******************************************************/

class DBEntry
{
public :

	DBEntry(void);
	~DBEntry(void);

	bool Read(FILE *pFile, unsigned int *puiOffset, DBFile *pDBFile);
	void Write(FILE *pFile, unsigned int *puiOffset);
	
	/******************************************************
	GET
	*******************************************************/

	inline int GetTaxId(void) const
	{ return m_stEntryHeader.iTaxId; }

	inline int GetMw(void) const
	{ return m_stEntryHeader.iMw; }

	inline double GetPi(void) const
	{ return m_stEntryHeader.usPi / 100.; }

	inline unsigned char GetType(void) const
	{ return m_stEntryHeader.ucType; }

	inline bool IsVarsplice(void) const
	{ return (m_stEntryHeader.ucType & TYPE_VARSPLICE)? true : false; }

	inline bool IsFragment(void) const
	{ return (m_stEntryHeader.ucType & TYPE_FRAGMENT)? true : false; }

	inline bool IsChain(void) const
	{ return (m_stEntryHeader.ucType & TYPE_CHAIN)? true : false; }

	inline int GetChildId(void) const
	{ return m_stEntryHeader.ucChildId; }

	inline int GetChildStart(void) const
	{ return m_stEntryHeader.usChildStart; }

	inline int GetChildEnd(void) const
	{ return m_stEntryHeader.usChildEnd; }

	inline unsigned int GetOffset(void) const
	{ return m_uiOffset; }

	inline int GetNbKeyword(void)
	{ return m_aKW.GetNbElt(); }

	inline unsigned short GetKeyword(int i) 
	{ return m_aKW[i]; }

	inline DynamicArray<TS_Ptm> &GetPtm(void) 
	{ return m_aPTM; }


	inline const char *GetSQ(void)
	{ return m_aSQ; }

	inline const char *GetAC(void)
	{ return m_aAC; }

	inline const char *GetID(void)
	{ return m_aID; }

	inline const char *GetDE(void)
	{ return m_aDE; }


	/******************************************************
	SET
	*******************************************************/

	// Copy values from the entry passed in argument.
	// NOTE: keywords are not copied!
	void copy(DBEntry* entry);
	
protected :
	void Set(	const char *pszAC, const char *pszID, const char *pszDE, const char *pszSQ, 
						int iTaxId, int iMw, double dPi, unsigned char ucType, int iChildId, int iChildStart, int iChildEnd,
						DynamicArray<unsigned short> &aKW, DynamicArray<TS_Ptm> &aPtm);
	inline void SetTaxId(int iTaxId)
	{ m_stEntryHeader.iTaxId = iTaxId; }

	inline void SetMw(int iMw)
	{ m_stEntryHeader.iMw = iMw; }

	inline void SetPi(double d) 
	{ m_stEntryHeader.usPi = (unsigned short)(d * 100); }

	inline void SetType(unsigned char ucType) 
	{ m_stEntryHeader.ucType = ucType; }

	inline void SetChildId(unsigned char ucChildId) 
	{ m_stEntryHeader.ucChildId = ucChildId; }

	inline void SetChildStart(unsigned short usChildStart) 
	{ m_stEntryHeader.usChildStart = usChildStart; }

	inline void SetChildEnd(unsigned short usChildEnd) 
	{ m_stEntryHeader.usChildEnd = usChildEnd; }
	
	inline void SetKeyword(DynamicArray<unsigned short> &aKW) 
	{ m_aKW.Copy(aKW); }

	inline void SetPtm(DynamicArray<TS_Ptm> &aPtm) 
	{ m_aPTM.Copy(aPtm); }

	void SetSQ(const char *pszSQ);
	void SetAC(const char *pszAC);
	void SetID(const char *pszID);
	void SetDE(const char *pszDE);

	void Print(void);

	void ResetForWrite(void);

	inline unsigned short GetU16Pi(void) const
	{ return m_stEntryHeader.usPi; }
	
private :
	TS_EntryHeader			m_stEntryHeader;
	DynamicArray<char>		m_aAC;
	DynamicArray<char>		m_aID;
	DynamicArray<char>		m_aDE;
	DynamicArray<unsigned short>	m_aKW;
	DynamicArray<TS_PtmRef>		m_aPtmRef;
	DynamicArray<char>		m_aSQ;
	DynamicArray<TS_Ptm>		m_aPTM;

	//offset dans le fichier en cours de lecture
	unsigned int			m_uiOffset;
	int				m_iNbByte;

	//variables de classe
	static const char		decodeTable1[15];
	static const char		decodeTable2[8];
	static const bool		encodeTable1[26];
	static const unsigned char	encodeTable2[26];
	
	
	void ResetForRead(void);
	void DecodeUniref(void);
	void ReadSQ(FILE *pFile);
	void WriteSQ(FILE *pFile);
	
	static inline int SortPtmByPos(const void *p1, const void *p2)
	{ return ((TS_PtmRef *)p1)->iPos - ((TS_PtmRef *)p2)->iPos; }

	static inline int SortKeyword(const void *p1, const void *p2)
	{	return (int)( (*(unsigned short *)p1) - (*(unsigned short *)p2) ); }
};

#endif
