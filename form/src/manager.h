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
	Created	on		   : 24.02.2005
	Last modification  : 24.02.2005
	Comments		   : 

***********************************************************/



#ifndef	__MANAGER_H__
#define	__MANAGER_H__



/***********************************************************

Class Manager

***********************************************************/

class Manager
{

private	:

	PopitamForm		inputForm;
	int				m_iRequest;
	bool			isBatchMode;
	static const bool DEBUG;

public :

	Manager(void);
	~Manager(void);

	void Run(void);

	void BatchMode(const char *pszParamFileName, const char	*pszDataFileName, const	char *pszScoreFileName);
    
private	:

	void WriteParamFile(TagFile	*pFile,	const char *pszScoreFileName);
	void WriteOutput(const char	* outputData, File &file, bool isStdout);
	void WriteErrorPage(File &output, const char* pszErrorFilePath, bool isStdout, int jobId);
	void WriteErrorMessage(File &output, const char* pszErrorFilePath, int jobId);
	void WriteHeader(File &output, bool isStdout);
	void WriteFooter(File &output, bool isStdout);

	void RequestSubmit(void);
	void Submit(const char *pszDataFileName, const char	*pszScoreFileName, File	&file, bool	isStdout);

};


#endif
