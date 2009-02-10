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

//********************************************************************//
//              INCLUDE                                               //
//********************************************************************//
#include <iostream>                                                 // flux io.
#include <cstdlib>                                                   // rand(), exit().
#include <cstdio>                                                    // MAX.
#include <cmath>                                                     // pow()
#include <cstring>                                                   // strcmp(), strcpy()
#include <fstream>                                                  // ifstream, ofstream.

#include "defines.h"
#include "utils.h"
#include "error.h"
#include "gpparameters.h"                                               // PARAMETERS.
#include "gptreenode.h"                                                 // NODE.
#include "gptree.h"                                                     // TREE.


using namespace std;



//********************************************************************//
//              CONSTRUCTOR                                           //
//********************************************************************//

TREE::TREE()
{ 
	Root        = NULL;
	NbNode      = 0;
	NbTerminal  = 0;
	NbFunction  = 0;
	NbCoeff     = 0;
	Depth       = 0;
	State       = false;
}


//********************************************************************//
//              DESTRUCTOR                                            //
//********************************************************************//


TREE::~TREE()
{
	if (Root != NULL) {DeleteSubTree(Root);}
}


//********************************************************************//
//              DESTRUCTOR                                            //
//********************************************************************//

void TREE::DeleteSubTree(NODE *racine) 
{
	if (racine!=NULL)
	{
		for (int i=0; i<racine->getArgvNb(); i++)
		{
			DeleteSubTree(racine->getChild(i));
			racine->setChild(NULL,i);
		}
		if (racine->getFather() != NULL)
		{
			racine->setFather(NULL);
		}
		delete racine;
		racine = NULL;
	}
}


//********************************************************************//
//              CHARGEMENT D'UN ARBRE A PARTIR D'UN DOTFILE           //
//********************************************************************//

void TREE::LoadTree(PARAMETERS* gp_parameters, char* errorFileName, char* filename)
{
	ifstream dotfile;
	dotfile.open(filename);
	if (!dotfile) fatal_error(errorFileName, FILES, "In function TREE::LoadTree() a)");

	int   NodeArgv;
	int   NodeValue;                               
	float CoeffValue;                             
	bool  NodeType;                               
	LoadNode(gp_parameters, dotfile, NodeArgv, NodeType, NodeValue, CoeffValue);
	Root = new NODE(NodeType,NodeValue,NodeArgv);
	if (NodeType == 0 && NodeValue == 4)
		Root->setCoeff(CoeffValue);
	LoadTree(gp_parameters, Root,dotfile);
	dotfile.close(); dotfile.clear();
	State=true;
}


void TREE::LoadTree(PARAMETERS* gp_parameters, NODE *root,ifstream &FIN)
{
	int   NodeArgv;                 
	int   NodeValue;
	float CoeffValue;
	bool  NodeType;
	if (root->getType() == 1) 
	{  
		for (int i=0; i<root->getArgvNb(); i++)
		{
			LoadNode(gp_parameters, FIN, NodeArgv, NodeType, NodeValue, CoeffValue);
			NODE *child = new NODE(NodeType,NodeValue,NodeArgv);
			if (child->getType()==0 && child->getValue()==4) 
				child->setCoeff(CoeffValue);
			child->setFather(root);
			root->setChild(child,i); 
			LoadTree(gp_parameters, root->getChild(i),FIN);
			child = NULL;
		}
	}
}

void TREE::LoadNode(PARAMETERS* gp_parameters, ifstream &FIN, int &NodeArgv, bool &NodeType,int &NodeValue,float &CoeffValue)
{
	int  i;
	char line[256];
	char tag[256];
	while (true)
	{
		FIN.getline(line, 255);
		strncpy(tag, line, 255);
		if (!strncmp(tag, "}", 1))
			break;
		if (!strncmp(tag, "node", 4)) 
		{
			if (strncmp(tag, "node ", 5)) 
			{
				i=0;
				while(tag[i]!='"')
					i++;
				char *label     = new char[30];
				int  j          = 0;
				int  StartCoeff = 0;
				i++;
				while(tag[i]!='"')
				{
					if (tag[i] == '=')
						StartCoeff = j+1;
					label[j]=tag[i];
					i++;
					j++;
				}
				label[j]='\0';
				if (StartCoeff != 0)
				{
					NodeArgv   = 0;
					NodeType   = 0;
					NodeValue  = 4;
					char *Coeff = new char[30];
					for (unsigned int k=StartCoeff; k<strlen(label); k++)
						Coeff[k-StartCoeff] = label[k];
					CoeffValue = (float)atof(Coeff);
					if (Coeff != NULL)
					{
						delete Coeff;
						Coeff = NULL;
					}
					return;
				}
				else 
				{
					for (int IDfunction=0; IDfunction<gp_parameters->FunctionNumber; IDfunction++)
					{
						if (!strncmp(label,gp_parameters->TConvertFunction[IDfunction],strlen(label)))
						{ 
							NodeArgv   = gp_parameters->FunctionArity[IDfunction];
							NodeType   = 1;
							NodeValue  = IDfunction;
							CoeffValue = 0;
							return;
						}
					}
					for (int IDterminal=0; IDterminal<gp_parameters->TerminalNumber; IDterminal++)
					{
						if (!strncmp(label,gp_parameters->TConvertTerminal[IDterminal],strlen(label)))
						{ 
							NodeArgv   = 0;
							NodeType   = 0;
							NodeValue  = IDterminal;
							CoeffValue = 0;
							return;
						}
					} 
				}
				if (label != NULL) 
				{
					delete label;
					label = NULL;
				}
			}
		}
	}
}


//********************************************************************//
//              MISE A JOUR DES PARAMETRES DE L'ARBRE                 //
//********************************************************************//

void TREE::UpdateTree(char* errorFileName)
{
	int proflocale  = 0;
	NbNode          = 0;
	Depth           = 0;
	NbTerminal      = 0;
	NbFunction      = 0;
	NbCoeff         = 0;
	if (Root != NULL) 
	{
		UpdateTree(Root, proflocale);
	}
	else 
	{
		fatal_error(errorFileName, DEBUG, "in function TREE::UpdateTree() b)"); 
	}
	NbNode = NbTerminal + NbFunction;
}


void TREE::UpdateTree(NODE *racine, int proflocal)
{
	if (racine!=NULL)
	{
		if (racine->getType() == 0) 
		{ 
			NbTerminal++; 
			if (racine->getValue() == 4)
				NbCoeff++;
		}     
		else
		{ 
			NbFunction++;
			for (int i=0; i<racine->getArgvNb(); i++)
				UpdateTree(racine->getChild(i), proflocal+1);
		}            
		if (proflocal>Depth) 
			Depth = proflocal;
	}
}


//********************************************************************//
//              PARSETREE                                             //
//********************************************************************//
double TREE::ParseTree(char* errorFileName, NODE *root, score c, bool* NAN_, bool* INF_)
{  
	double result = 0;
	if (root->getType() == 0)
	{ 
		switch(root->getValue())
		{
		case 0:   return c.lakScore1_a;
		case 1:   return c.modScore1_b;
		case 2:   return c.covScore1_c;
		case 3:   return c.covScore2_d;
		case 4:   return root->getCoeff();
		case 5:   return c.covScore3_e;
		case 6:   return c.intScore1_f;
		case 7:   return c.perScore1_g;
		case 8:   return c.famScore1_h;
		case 9:   return c.errScore1_i;
		case 10:  return c.redScore1_j;
		case 11:  return c.serScore1_k;
		case 12:  return c.serScore2_l;

		default : 
			fatal_error(errorFileName, DEBUG, "In function  TREE::ParseTree()"); 
		} 
	}
	else 
	{
		switch(root->getValue())
		{
		case 0:
			{
				result = (ParseTree(errorFileName, root->getChild(0),c, NAN_, INF_) + ParseTree(errorFileName, root->getChild(1),c, NAN_, INF_));
				// test NAN_ et INF_ sur result
				if (!isDoubleEqual(result,   result)) {(*NAN_) = true; return 0;}
				if ( isDoubleEqual(result-1, result)) {(*INF_) = true; return 0;}
				return result;
			}
		case 1:
			{
				result = (ParseTree(errorFileName, root->getChild(0),c, NAN_, INF_) - ParseTree(errorFileName, root->getChild(1),c, NAN_, INF_)); 
				// test NAN_ et INF_ sur result
				if (!isDoubleEqual(result,   result)) {(*NAN_) = true; return 0;}
				if ( isDoubleEqual(result-1, result)) {(*INF_) = true; return 0;}
				return result;
			}
		case 2: 
			{
				result = (ParseTree(errorFileName, root->getChild(0),c, NAN_, INF_) * ParseTree(errorFileName, root->getChild(1),c, NAN_, INF_));
				// test NAN_ et INF_ sur result
				if (!isDoubleEqual(result,   result)) {(*NAN_) = true; return 0;}
				if ( isDoubleEqual(result-1, result)) {(*INF_) = true; return 0;}
				return result;
			}
		case 3:	  
			{
				result = (ParseTree(errorFileName, root->getChild(0),c, NAN_, INF_) / ParseTree(errorFileName, root->getChild(1),c, NAN_, INF_));
				// test NAN_ et INF_ sur result
				if (!isDoubleEqual(result,   result)) {(*NAN_) = true; return 0;}
				if ( isDoubleEqual(result-1, result)) {(*INF_) = true; return 0;}
				return result;
			}
		case 4:
			{
				if ( ParseTree(errorFileName, root->getChild(0),c, NAN_, INF_) < ParseTree(errorFileName, root->getChild(1),c, NAN_, INF_) )
					return ParseTree(errorFileName, root->getChild(2),c, NAN_, INF_);
				else 
					return ParseTree(errorFileName, root->getChild(3),c, NAN_, INF_);
			}
		default : 
			fatal_error(errorFileName, DEBUG, "In function TREE::ParseTree() c)");
		}
	}
	fatal_error(errorFileName, DEBUG, "In function  TREE::ParseTree()"); 
	return 0;
}


