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
	Last modification  : 28.11.2003
	Comments           : 

***********************************************************/

#ifndef __LEXICO_H__
#define __LEXICO_H__



/***********************************************************

	Class Lexico	

***********************************************************/


class Lexico
{

private :
	
	enum{	
		ORIGIN = 0,							//provenance d'un elt par rapport Þ son parent
		LEFT,
		RIGHT
	};

	typedef struct lexico_s {				//element de l'arbre binaire
		struct lexico_s	*psLeft;
		struct lexico_s	*psRight;
		const char		*psz;
		void			*pvData;
	} TS_Lexico;


	typedef struct{							//Navigation dans un arbre : un seul pointeur dans les fct recursives
		TS_Lexico	**ppElt;
		TS_Lexico	*pElt;
		TS_Lexico	*pParent;
		const char	*psz;
		int			iFrom;
		void		*pvData;
		void		**ppvData;
		int			iStrcmpReturn;
	}TS_BrowseInTree;


private :

	//racine de l'arbre binaire
	TS_Lexico	*m_pRacine;	
	
	//pointeur sur fct free elt de l'arbre
	void		(* m_pfFreeElt)(const char *psz, void *pData);	


public :

	Lexico( void (* pfFreeElt)(const char *psz, void *pData) );
	~Lexico(void);

	bool Add(const char *psz, void *pvData);

	inline bool Supp(const char *psz)
	{ return Supp( &m_pRacine, psz); }

	bool Get(const char *psz, void **ppvData);

	void Free(void);



private :

	//fct recursives lancee par les fct pubic du meme nom
	bool Add(TS_BrowseInTree *pBrowse);
	bool Supp(TS_Lexico **ppstLexico, const char *psz);
	bool Get(TS_BrowseInTree *pBrowse);
	void Free(TS_Lexico *pstLexico);

	bool GetParent( TS_BrowseInTree *pBrowse);
	bool GetLastLeft( TS_BrowseInTree *pBrowse);



};


#endif

