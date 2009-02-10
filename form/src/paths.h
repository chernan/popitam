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

#ifndef	__PATH_H__
#define	__PATH_H__



#pragma	pack(1)

typedef	struct{
	char *pszIOTag;
	char *pszTaxoTag;
	char *pszTaxonomyU;
	char *pszAtomicMass;
	char *pszPtm;
	char *pszSwissprot;
	char *pszTrembl;
	char *pszIndex1Ext;
	char *pszIndex2Ext;
	char *pszServer; //nom du serveur, i.e http://master.expasy.org
	char *pszRelativeServer; //sur expasy, liens doivent etre en relatif i.e /cgi-bin/popitam/form.cgi mais	ne marche pas sur lin-071
	char *pszHTMLdocs; //link to html documentation
	char *pszCgiFullPath; //utilise	pour lancer	la commande	d'execution	en mode	batch
	char *pszPrgmFullPath;
	char *pszTmpDir;
	char *pszSendMail;
	char *pszLibPath;
	char *pszAA;
	char *pszAAProp;
	char *pszProb;
	char *pszScores;
	char *pszDB;
	char *pszTaxonomy;
	char *pszExpFilesDir;
}TS_Path;

#pragma	pack()



/******************************************************
//pszServer<-debut des chemins du formulaire cgi
attention, si on ajoute	un chemin ici, il faut aussi l'ajouter a la	lecture	dans le	path.h de tagopop,
sinon il plante	car	il verifie aussi qu'il n'y a pas de	chemins	en trop	(a modifier)

Class Paths	:


*******************************************************/


class Paths	: public PszList
{

public :

	static TS_Path		m_stPsz;

	static const char	*m_pszFileName;

public :

	static inline void Load()
	{
	PszList::Load(m_pszFileName, (char **)(&m_stPsz), sizeof(TS_Path) /	sizeof(char	*));
	}

	static inline void Free(void)
	{ PszList::Free( (char **)(&m_stPsz), sizeof(TS_Path) /	sizeof(char	*)); }
};


inline TS_Path *path(void)
{ return &(Paths::m_stPsz);	}




#endif
