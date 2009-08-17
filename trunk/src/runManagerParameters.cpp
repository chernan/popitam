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

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <fstream>

#include "runManagerParameters.h"
#include "defines.h"
#include "error.h"
#include "usage.h"
#include "util.h"
#include "dbreader.h"

using namespace std;

// ********************************************************************************************** //

runManagerParameters::runManagerParameters() 
{
	XML_OUT   =  1;
	SHT_OUT   =  1;
	
	r_NORMAL  =  0;
	r_CHECK   =  0;
	r_FUN     =  0; 
	s_UNKNOWN =  0;
	s_IDSET   =  0;
	s_MIXSET  =  0;
	m_MUTMOD  =  0;
	
	strcpy(FILE_ERROR_NAME, "");
	strcpy(FORMAT, "");
	strcpy(PATH_INFILE, "");
	strcpy(AMINO_ACID_INFILE, "");
	strcpy(GPPARAMETERS_INFILE, "");
	strcpy(SCORE_FUNCTION0_INFILE, "");
	strcpy(SCORE_FUNCTION1_INFILE, "");
	strcpy(SCORE_FUNCTION2_INFILE, "");
	strcpy(PROBS_TOFTOF1_INFILE, "");
	strcpy(PROBS_QTOF1_INFILE, "");
	strcpy(PROBS_QTOF2_INFILE, "");
	strcpy(PROBS_QTOF3_INFILE, "");
	strcpy(DB1_PATH, "");
	strcpy(DB2_PATH, "");
	strcpy(OUTPUT_DIR, "");
	strcpy(GEN_OR_FILENAME_SUFF, "");
	strcpy(GEN_NOD_FILENAME_SUFF, "");
	strcpy(SCORE_NEG_OUTFILE, "");
	strcpy(SCORE_RANDOM_OUTFILE, "");
	
	strcpy(AC_FILTER, "");
	strcpy(ENZYME, "");
	strcpy(INSTRUMENT, "");
	
	TAXID_NB        = 0;
	FRAGMENT_ERROR1 = 0;
	FRAGMENT_ERROR2 = 0;
	PREC_MASS_ERROR = 0; 
	GLOBAL_CHARGE   = 0;
}

// ********************************************************************************************** //

runManagerParameters::~runManagerParameters() 
{
	FILEIN.Close();
	
	if (XML_OUT) {
		fprintf(FILEOUTXML, "</analysis>\n");
		FILEOUTXML.Close();
	}
	
	FILEOUT.Close();
	
	if (SHT_OUT) {
		FILEOUTSHORT.Close();
	}
}

// ********************************************************************************************** //

void runManagerParameters::init(int argc, char** argv) 
{ 
	if (!(argc >= 9 && argc <= 11)) {
		usage();
	}
	
	initArguments(argc, argv);
	
	char* filenameIN  = argv[5]+3;
	char* fileformat  = argv[6]+3;
	char* filenameOUT = argv[8]+3;
	char* filenameERR = argv[7]+3;
	char* filenamePARAMS = argv[4]+3;
	
	initIN_OUT(filenameIN, fileformat, filenameOUT);
	
	strcpy(FILE_ERROR_NAME, filenameERR); 
	checkArguments(argc, argv);
	displayArguments(FILEOUT);
	
	// Open xml output file (if requested)
	if (XML_OUT) {
		char filenameXML[256];
		strcpy(filenameXML, filenameOUT);
		strcat(filenameXML, ".xml");
		FILEOUTXML.Open(filenameXML, "w");
		fprintf(FILEOUTXML, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(FILEOUTXML, "<analysis>\n");
		fprintf(FILEOUTXML, "<version>%s</version>\n", POP_VERSION);
	}
	
	// Open sht output file (if requested)
	if (SHT_OUT) { 
		char filenameShort[256];
		strcpy(filenameShort, filenameOUT);
		strcat(filenameShort, ".sht");
		FILEOUTSHORT.Open(filenameShort, "w");
	}
	
	File fp;
	fp.Open(filenamePARAMS, "r");
	loadParameters(fp, argv);
	fp.Close();
}

// ********************************************************************************************** //

void runManagerParameters::initIN_OUT(char* filenameIN, char* fileformat, char* filenameOUT)
{
	if (!strcmp(filenameOUT, "stdout")) {
		// OUVERTURE FICHIER OUTPUT
		FILEOUT.Open(stdout);
	}
	else {
		FILEOUT.Open(filenameOUT, "w");
	}

	char* usedFileName = NULL;
	
	if (!strcmp(fileformat, "mzdata")) {
		strcpy(FORMAT, "mgf");
		usedFileName = convertSpectra(filenameIN);
	}
	else {
		usedFileName = filenameIN;
	}

	strcpy(FILEINNAME, usedFileName);
	
	// OUVERTURE FICHIER MSMSDATA
	FILEIN.Open(usedFileName, "r");
	if (!strcmp(fileformat, "mgf")) {
		strcpy(FORMAT, "mgf");
	}
	if (!strcmp(fileformat, "dta")) {
		strcpy(FORMAT, "dta");
	}
	if (!strcmp(fileformat, "pop")) {
		strcpy(FORMAT, "pop");
	}
}

// ********************************************************************************************** //

char* runManagerParameters::convertSpectra(char* filename) 
{
	char* newname = StrdupFormat("%s.mgf", filename);
	//perl -I /home/vital-it/phenyx/local/perl5/lib/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ia64-linux-thread-multi -I /home/vital-it/phenyx/swisspit/perl/phenyxPerl/lib/ -I /home/vital-it/phenyx/swisspit/perl/InSilicoSpectro/lib -I /home/vital-it/phenyx/swisspit/perl -I /home/vital-it/phenyx/swisspit/perl/Phenyx-Utils/lib "./convertSpectra.pl" /home/vital-it/aquandt/PipelineProgs/
	fprintf(FILEOUT, "perl -I /home/vital-it/phenyx/local/perl5/lib/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ia64-linux-thread-multi -I /home/vital-it/phenyx/swisspit/perl/phenyxPerl/lib/ -I /home/vital-it/phenyx/swisspit/perl/InSilicoSpectro/lib -I /home/vital-it/phenyx/swisspit/perl -I /home/vital-it/phenyx/swisspit/perl/Phenyx-Utils/lib \"/home/vital-it/phenyx/insilicospectro/InSilicoSpectro/scripts/convertSpectra.pl\" --in=mzdata:%s --out=mgf:%s --defaultcharge=2+", filename, newname);
	SystemCmd("perl -I /home/vital-it/phenyx/local/perl5/lib/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ -I /home/vital-it/phenyx/local/perl5/lib/perl5/site_perl/5.8.5/ia64-linux-thread-multi -I /home/vital-it/phenyx/swisspit/perl/phenyxPerl/lib/ -I /home/vital-it/phenyx/swisspit/perl/InSilicoSpectro/lib -I /home/vital-it/phenyx/swisspit/perl -I /home/vital-it/phenyx/swisspit/perl/Phenyx-Utils/lib \"/home/vital-it/phenyx/insilicospectro/InSilicoSpectro/scripts/convertSpectra.pl\" --in=mzdata:%s --out=mgf:%s --defaultcharge=2+", filename, newname);
	return newname;	
}

// ********************************************************************************************** //

void runManagerParameters::initArguments(int argc, char** argv) 
{
	if (!strcmp(argv[1]+3, "NORMAL")) {
		r_NORMAL = 1;
	}
	// RECUPERE LES VARIABLES DE RUN
	if (!strcmp(argv[1]+3, "CHECK")) {
		r_CHECK    = 1;
	}
	if (!strcmp(argv[1]+3, "FUN")) {
		r_FUN = 1;
	}
	if (!strcmp(argv[2]+3, "UNKNOWN")) {
		s_UNKNOWN  = 1;
	}
	if (!strcmp(argv[2]+3, "IDSET")) {
		s_IDSET = 1;
	}
	if (!strcmp(argv[2]+3, "MIXSET")) {
		s_MIXSET = 1;
	}
	if (!strcmp(argv[3]+3, "0")) {
		m_MUTMOD = 0;
	}
	if (!strcmp(argv[3]+3, "1")) {
		m_MUTMOD = 1;
	}
	if (!strcmp(argv[3]+3, "2")) {
		m_MUTMOD = 2;
	}
	if (!strcmp(argv[3]+3, "3")) {
		m_MUTMOD = 3;
	}
	if (s_MIXSET == 1) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "MIXSET is not yet implemented; please don't use it");
	}
}

// ********************************************************************************************** //

void runManagerParameters::checkArguments(int argc, char** argv) 
{ 
	if (strncmp(argv[1], "-r", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for runMode");
	}
	if (strncmp(argv[2], "-s", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for soluceMode");
	}
	if (strncmp(argv[3], "-m", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for mutmodMode");
	}
	if (strncmp(argv[4], "-p", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for parameter file");
	}
	if (strncmp(argv[5], "-d", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for MS/MS data input file");
	}
	if (strncmp(argv[6], "-f", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for fileformat");
	}
	if (strncmp(argv[7], "-e", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for output file");
	}
	if (strncmp(argv[8], "-o", 2)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for output file");
	}
	if (r_NORMAL  + r_CHECK + r_FUN != 1) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for runMode");
	}
	if (s_UNKNOWN + s_IDSET + s_MIXSET != 1) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for soluceMode");
	}
	if ((m_MUTMOD < 0) || (m_MUTMOD > 3)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "please check your argument for soluceMode");
	}
	if ((r_FUN) && (s_UNKNOWN)) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "Popitam cannot be used FUN mode with unidentified spectra");
	}
	if (MAX_MUT_NB < m_MUTMOD) {
		fatal_error(FILE_ERROR_NAME, PARAMETER, "To much MUTMOD; please increase MAX_MUT_NB in defines.h");
	}
	
	// Parse -noxml and/or -nosht optional arguments
	const char noxml[] = "-noxml";
	const char nosht[] = "-nosht";
	const int len1 = strlen(noxml);
	const int len2 = strlen(nosht);
	const char* argvA = argv[9];
	const char* argvB = argv[10];
	if (argc == 10) {
		if (strncmp(argvA, noxml, len1) == 0) {
			XML_OUT = 0;
		}
		else if (strncmp(argvA, nosht, len2) == 0) {
			SHT_OUT = 0;
		}
		else {
			fatal_error(FILE_ERROR_NAME, PARAMETER, "invalid argument! (only -noxml or -nosht are valid)");
		}
	}
	else if (argc == 11) {
		if ((strncmp(argvA, noxml, len1) == 0 && strncmp(argvB, nosht, len2) == 0) || ((strncmp(argvA, nosht, len2) == 0 && strncmp(argvB, noxml, len1) == 0))) {
			XML_OUT = 0;
			SHT_OUT = 0;
		}
		else {
			fatal_error(FILE_ERROR_NAME, PARAMETER, "invalid argument! (only -noxml or -nosht are valid)");
		}
	}
}

// ********************************************************************************************** //

void runManagerParameters::displayArguments(File &fp)
{
	fprintf(fp, "Popitam %s is now running; run parameters are ", POP_VERSION);
	
	if (r_NORMAL) {
		fprintf(fp, "%s", "NORMAL and ");
	}
	if (r_CHECK) {
		fprintf(fp, "%s", "CHECK and ");
	}
	if (r_FUN) {
		fprintf(fp, "%s", "FUN and ");
	}
	if (s_UNKNOWN) {
		fprintf(fp, "%s", "UNKNOWN ");
	}
	if (s_IDSET) {
		fprintf(fp, "%s", "IDSET ");
	}
	if (s_MIXSET) {
		fprintf(fp, "%s", "MIXSET ");
	}
	
	fprintf(fp, "(%i allowed gaps)\n", m_MUTMOD);
	
	if (XML_OUT) {
		fprintf(fp, "%s", "XML output is enabled\n");
	}
	else {
		fprintf(fp, "%s", "XML output is disabled\n");
	}
	
	if (SHT_OUT) {
		fprintf(fp, "%s", "SHT output is enabled\n");
	}
	else {
		fprintf(fp, "%s", "SHT output is disabled\n");
	}
	
	fprintf(fp, "\n________________________________________________________________________________\n\n");  
}

// ********************************************************************************************** //

void runManagerParameters::loadParameters(File &fp, char** argv) 
{
	char car;
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(PATH_INFILE, 255, fp);
			PATH_INFILE[strlen(PATH_INFILE)-1] = '\0';
			break;
		}
	}
	
	if ( strncmp(PATH_INFILE, "default", 7) == 0 ) {
		strcpy(PATH_INFILE, DEF_DATA_FOLDER);
		strcat(PATH_INFILE, "path_lin.txt");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(AMINO_ACID_INFILE, 255, fp);
			AMINO_ACID_INFILE[strlen(AMINO_ACID_INFILE)-1] = '\0';
			break;
		}
	}
	
	if ( strncmp(AMINO_ACID_INFILE, "default", 7) == 0 ) {
		strcpy(AMINO_ACID_INFILE, DEF_DATA_FOLDER);
		strcat(AMINO_ACID_INFILE, "aa20.txt");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(GPPARAMETERS_INFILE, 255, fp);
			GPPARAMETERS_INFILE[strlen(GPPARAMETERS_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(GPPARAMETERS_INFILE, "default", 7) == 0 ) {
		strcpy(GPPARAMETERS_INFILE, DEF_DATA_FOLDER);
		strcat(GPPARAMETERS_INFILE, "functionLoadParam.txt");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(SCORE_FUNCTION0_INFILE, 255, fp);
			SCORE_FUNCTION0_INFILE[strlen(SCORE_FUNCTION0_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(SCORE_FUNCTION0_INFILE, "default", 7) == 0 ) {
		strcpy(SCORE_FUNCTION0_INFILE, DEF_DATA_FOLDER);
		strcat(SCORE_FUNCTION0_INFILE, "funScore0.dot");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(SCORE_FUNCTION1_INFILE, 255, fp);
			SCORE_FUNCTION1_INFILE[strlen(SCORE_FUNCTION1_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(SCORE_FUNCTION1_INFILE, "default", 7) == 0 ) {
		strcpy(SCORE_FUNCTION1_INFILE, DEF_DATA_FOLDER);
		strcat(SCORE_FUNCTION1_INFILE, "funScore1.dot");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(SCORE_FUNCTION2_INFILE, 255, fp);
			SCORE_FUNCTION2_INFILE[strlen(SCORE_FUNCTION2_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(SCORE_FUNCTION2_INFILE, "default", 7) == 0 ) {
		strcpy(SCORE_FUNCTION2_INFILE, DEF_DATA_FOLDER);
		strcat(SCORE_FUNCTION2_INFILE, "funScore2.dot");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(PROBS_TOFTOF1_INFILE, 255, fp);
			PROBS_TOFTOF1_INFILE[strlen(PROBS_TOFTOF1_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(PROBS_TOFTOF1_INFILE, "default", 7) == 0 ) {
		strcpy(PROBS_TOFTOF1_INFILE, DEF_DATA_FOLDER);
		strcat(PROBS_TOFTOF1_INFILE, "TOFTOF_1.prob");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(PROBS_QTOF1_INFILE, 255, fp);
			PROBS_QTOF1_INFILE[strlen(PROBS_QTOF1_INFILE)-1] = '\0'; break;
		}
	}
	if ( strncmp(PROBS_QTOF1_INFILE, "default", 7) == 0 ) {
		strcpy(PROBS_QTOF1_INFILE, DEF_DATA_FOLDER);
		strcat(PROBS_QTOF1_INFILE, "QTOF_1.prob");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(PROBS_QTOF2_INFILE, 255, fp);
			PROBS_QTOF2_INFILE[strlen(PROBS_QTOF2_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(PROBS_QTOF2_INFILE, "default", 7) == 0 ) {
		strcpy(PROBS_QTOF2_INFILE, DEF_DATA_FOLDER);
		strcat(PROBS_QTOF2_INFILE, "QTOF_2.prob");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(PROBS_QTOF3_INFILE, 255, fp);
			PROBS_QTOF3_INFILE[strlen(PROBS_QTOF3_INFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(PROBS_QTOF3_INFILE, "default", 7) == 0 ) {
		strcpy(PROBS_QTOF3_INFILE, DEF_DATA_FOLDER);
		strcat(PROBS_QTOF3_INFILE, "QTOF_3.prob");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(DB1_PATH, 255, fp);
			DB1_PATH[strlen(DB1_PATH)-1] = '\0';
			break;
		}
	}
	if ( strncmp(DB1_PATH, "default", 7) == 0 ) {
		strcpy(DB1_PATH, DEF_DB_PATH);
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(DB2_PATH, 255, fp);
			DB2_PATH[strlen(DB2_PATH)-1] = '\0';
			break;
		}
	}
	if ( strncmp(DB2_PATH, "default", 7) == 0 ) {
		strcpy(DB2_PATH, DEF_DB_PATH);
	}
	
	// la taille de strTaxIds doit etre compatible avec MAX_TAXID_NB dans defines.h
	char strTaxIds[1024];
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(strTaxIds, 1023, fp);
			strTaxIds[strlen(strTaxIds)-1] = '\0';
			break;
		}
	}
	if (!strncmp(strTaxIds, "NO", 2)) {
		TAXID_NB = 0;
	}
	else {
		char* currentId;
		TAXID_NB = 0;
		currentId = strtok(strTaxIds, " ");
		while (currentId != NULL) {
			TAXID[TAXID_NB] = atoi(currentId);
			TAXID_NB++;
			currentId = strtok(NULL, " ");
		}
	}
	
	while (fscanf(fp, "%c", &car)) { 
		if (car == ':') {
			fgets(AC_FILTER, AC_FILTER_LENTGH, fp); 
			AC_FILTER[strlen(AC_FILTER)-1] = '\0';
			break;
		}
	}
	if (strlen(AC_FILTER) >= AC_FILTER_LENTGH-3) {
		fatal_error(FILE_ERROR_NAME, MEMORY, "Please, increase AC_FILTER_LENTGH in defines.h");
	} 
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(ENZYME, 255, fp);
			ENZYME[strlen(ENZYME)-1] = '\0';
			break;
		}
	}
	
	if ( strncmp(ENZYME, "default", 7) == 0 ) {
		strcpy(ENZYME, "Trypsin");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(OUTPUT_DIR, 255, fp);
			OUTPUT_DIR[strlen(OUTPUT_DIR)-1] = '\0'; break;
		}
	}
	if ( strncmp(OUTPUT_DIR, "default", 7) == 0 ) {
		strcpy(OUTPUT_DIR, "../out/");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(GEN_OR_FILENAME_SUFF, 255, fp);
			GEN_OR_FILENAME_SUFF[strlen(GEN_OR_FILENAME_SUFF)-1] = '\0';
			break;
		}
	}
	if ( strncmp(GEN_OR_FILENAME_SUFF, "default", 7) == 0 ) {
		strcpy(GEN_OR_FILENAME_SUFF, "/PAT/LS/OR_SPEC");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(GEN_NOD_FILENAME_SUFF, 255, fp);
			GEN_NOD_FILENAME_SUFF[strlen(GEN_NOD_FILENAME_SUFF)-1] = '\0';
			break;
		}
	}
	if ( strncmp(GEN_NOD_FILENAME_SUFF, "default", 7) == 0 ) {
		strcpy(GEN_NOD_FILENAME_SUFF, "/PAT/LS/NOD_SPEC");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(SCORE_NEG_OUTFILE, 255, fp);
			SCORE_NEG_OUTFILE[strlen(SCORE_NEG_OUTFILE)-1] = '\0';
			break;
		}
	}
	if ( strncmp(SCORE_NEG_OUTFILE, "default", 7) == 0 ) {
		strcpy(SCORE_NEG_OUTFILE, "SCORE_NEG.txt");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(SCORE_RANDOM_OUTFILE, 255, fp);
			SCORE_RANDOM_OUTFILE[strlen(SCORE_RANDOM_OUTFILE)-1] = '\0'; break;
		}
	}
	if ( strncmp(SCORE_RANDOM_OUTFILE, "default", 7) == 0 ) {
		strcpy(SCORE_RANDOM_OUTFILE, "SCORE_RANDOM.txt");
	}
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%f", &FRAGMENT_ERROR1);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%f", &FRAGMENT_ERROR2);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%f", &PREC_MASS_ERROR);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fgets(INSTRUMENT, 255, fp);
			INSTRUMENT[strlen(INSTRUMENT)-1] = '\0';
			break;
		}
	}
	
	// ************************************************************************* DIGESTION PARAMETERS //
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &MISSED);
			break;
		}
	}
	
	// ****************************************************************** POPITAM SPECIFIC PARAMETERS //
	
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%f", &PEAK_INT_SEUIL);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &BIN_NB);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &COVBIN);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &EDGES_TYPE);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &MIN_TAG_LENTGH);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &RESULT_NB);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &MIN_PEP_PER_PROT);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			//IL FAUDRA CHANGER EN UP_LIMIT_RANGE_PM DANS POPPARAM
			fscanf(fp, "%f", &UP_LIMIT_RANGE_PM);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			//IL FAUDRA CHANGER EN LOW_LIMIT_RANGE_PM DANS POPPARAM
			fscanf(fp, "%f", &LOW_LIMIT_RANGE_PM);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			//IL FAUDRA CHANGER EN UP_LIMIT_RANGE_MOD DANS POPPARAM
			fscanf(fp, "%f", &UP_LIMIT_RANGE_MOD);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			//IL FAUDRA CHANGER EN LOW_LIMIT_RANGE_MOD DANS POPPARAM
			fscanf(fp, "%f", &LOW_LIMIT_RANGE_MOD);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%f", &MIN_COV_ARR);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &PLOT);
			break;
		}
	}
	while (fscanf(fp, "%c", &car)) {
		if (car == ':') {
			fscanf(fp, "%i", &PVAL_ECHSIZE);
			break;
		}
	}
	
	// TODO check modif; was argv[5]+3
	File fpTemp(FILEIN.GetFileName(), "r");
	findSpectrumNb(fpTemp);
	fpTemp.Close();
	
	if ((r_CHECK) && (SPECTRUM_NB > 1)) {
		fatal_error(FILE_ERROR_NAME, MODE, "in check mode, the number of spectra must be 1");
	}
	
	//PAH: NO NEED WITH NEW DIGEST VERSION
	/*
	* // reads the version of database
	char pth[256];
	sprintf(pth, "%s%s", DEF_DATA_FOLDER, "db_release.txt");
	ifstream fin; fin.open(pth);
	if (!fin) {}
	char line[256];
	fin.getline(line, 255);
	// REMOVE SPECIAL CHARACTERS
	int lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
	while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
	lenght--;
	}
	}
	line[lenght] = '\0';
	strcpy(SP_release, line);
	fin.getline(line, 255);
	// REMOVE SPECIAL CHARACTERS
	lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
	while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
	lenght--;
	}
	}
	line[lenght] = '\0';
	strcpy(TR_release, line);
	fin.close(); fin.clear();
	*/
}

// ********************************************************************************************** //

void runManagerParameters::findSpectrumNb(File& fp)
{
	char line[256];
	SPECTRUM_NB = 0;
	
	if ((!strcmp(FORMAT, "mgf")) | (!strcmp(FORMAT, "pop"))) {
		while (fgets(line, 255, fp)) {
			if (!strncmp(line, "BEGIN ION", 9)) SPECTRUM_NB++;
		}
	}
	else {
		if (!strcmp(FORMAT, "dta")) {
			// il faut d'abord regarder s'il y a des blancs avant le premier spectre
			fgets(line, 255, fp);
			if (isspace(line[0])) {
				while (isspace(line[0])) {
					fgets(line, 255, fp);
				}
			}

			// puis on compte de la maniere suivante:
			// on repere la premiere ligne blanche, et on verifie que la suivante est pleine
			bool GOT_BLANK = false;
			SPECTRUM_NB = 1; 	  
		
			while (fgets(line, 255, fp)) {
				if (isspace(line[0])) {
					GOT_BLANK = true;
				}
				if (isdigit(line[0]) && (GOT_BLANK)) {
					SPECTRUM_NB++;
					GOT_BLANK = false;
				}
			}
		}
		else {
			fatal_error(FILE_ERROR_NAME, PARAMETER, "There is a problem with the FORMAT argument");
		}
	}
}

// ********************************************************************************************** //

void runManagerParameters::display(File &fp)
{
	fprintf(fp, "%-45s: %s\n", "File processed",               FILEIN.GetFileName());
	fprintf(fp, "%-45s: %i\n", "Initial number of spectra",    SPECTRUM_NB); 
	fprintf(fp, "%-45s: %s\n", "Instrument",                   INSTRUMENT);
	fprintf(fp, "%-45s: %i\n", "Min covbin",                   MIN_COVBIN);
	fprintf(fp, "%-45s: %i\n", "Wanted number of coverage",    COVBIN);
	fprintf(fp, "%-45s: %i\n", "Minimal tag lentgh",           MIN_TAG_LENTGH);
	fprintf(fp, "%-45s: %f\n", "Minimal coverage for a valuable arrangement", MIN_COV_ARR);
	fprintf(fp, "\n%-45s \n",  "Graph parameters ");
	
	if (EDGES_TYPE == 0) {
		fprintf(fp, "%-45s: %s\n",    " - edges_type : ",     "1 aa");
	}
	else {
		fprintf(fp, "%-45s: %s\n",    " - edges_type : ",     "1,2 aa");
	}
	
	fprintf(fp, "%-45s: %.2f\n",  " - peptide error : ",   PREC_MASS_ERROR);
	fprintf(fp, "%-45s: %.2f\n",  " - fragment error1 : ", FRAGMENT_ERROR1);
	fprintf(fp, "%-45s: %.2f\n",  " - fragment error2 : ", FRAGMENT_ERROR2);

	// ecritures des donn�es sur les dbs
	DBReader m_db;
	DBFileReader *pDBFileReader;
	
	fprintf(fp, "\n%-45s \n", "DBs");
	if ((strcmp(DB1_PATH, "NO")) && (strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB1_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "%-45s: %s\n", pDBFileReader->GetLabel(), pDBFileReader->GetRelease());
		m_db.Open(DB2_PATH);
		pDBFileReader = m_db.GetFile(1);
		fprintf(fp, "%-45s: %s\n", pDBFileReader->GetLabel(), pDBFileReader->GetRelease());
		m_db.CloseAll();
	}
	
	if ((strcmp(DB1_PATH, "NO")) && (!strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB1_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "%-45s: %s\n", pDBFileReader->GetLabel(), pDBFileReader->GetRelease());
		m_db.CloseAll();
	}
	
	if ((!strcmp(DB1_PATH, "NO")) && (strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB2_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "%-45s: %s\n", pDBFileReader->GetLabel(), pDBFileReader->GetRelease());
		m_db.CloseAll();
	}
	
	fprintf(fp, "%-45s: ", "TAXONOMY");
	if (TAXID_NB == 0) {
		fprintf(fp, "NO\n");
	}
	else { 
		for (int i = 0; i < TAXID_NB; i++) {
			fprintf(fp, "%i ", TAXID[i]);
		}
		fprintf(fp, "\n");
	}  
	
	fprintf(fp, "%-45s: ", "AC FILTER");
	if (!(strncmp(AC_FILTER, "NO", 2)) && (strlen(AC_FILTER)<4)) {
		fprintf(fp, "%s\n", "NO");
	}
	else {
		if (strlen(AC_FILTER) < 50) {
			fprintf(fp, "%s\n", AC_FILTER);
		}
		else {
			for (int c = 0; c < 50; c++) {
				fprintf(fp, "%c", AC_FILTER[c]);
			}
			fprintf(fp, "...  \n");
		}
	}
	
	fprintf(fp, "\n%-45s: %s\n", "Enzyme", ENZYME);
	fprintf(fp, "%-45s: %i\n", "Missed cleavage number", MISSED);
	
	if (m_MUTMOD) {
		fprintf(fp, "\n%-45s \n", "Modification/Mutation parameters");
		//      fprintf(fp, "%-45s: %6i\n",    "Searching for ", max mutation hypotheses",            MAX_MUT_NB);
		fprintf(fp, "%-45s: %6.2f\n",  "up limit of PM range", UP_LIMIT_RANGE_PM);
		fprintf(fp, "%-45s: %6.2f\n",  "low limit of PM range", LOW_LIMIT_RANGE_PM);
		fprintf(fp, "%-45s: %6.2f\n",  "up limit of MOD range", UP_LIMIT_RANGE_MOD);
		fprintf(fp, "%-45s: %6.2f\n",  "low limit of MOD range",   LOW_LIMIT_RANGE_MOD);
	}
	
	fprintf(fp, "\n\n________________________________________________________________________________\n\n");
}


// ********************************************************************************************** //

void runManagerParameters::displayXML(File &fp)
{
	fprintf(fp, "<inputParameters>\n");	
	fprintf(fp, "   <inputFile>%s</inputFile>\n", FILEIN.GetFileName());
	fprintf(fp, "   <initNbSpectra>%i</initNbSpectra>\n", SPECTRUM_NB); 
	fprintf(fp, "   <instrument>%s</instrument>\n", INSTRUMENT);
	fprintf(fp, "   <minCovBin>%i</minCovBin>\n", MIN_COVBIN);
	fprintf(fp, "   <wantedCovBin>%i</wantedCovBin>\n", COVBIN);
	fprintf(fp, "   <minTagLength>%i</minTagLength>\n", MIN_TAG_LENTGH);
	fprintf(fp, "   <minCovArr>%f</minCovArr>\n", MIN_COV_ARR);  
	fprintf(fp, "   <edgeType>%i</edgeType>\n", EDGES_TYPE);
	fprintf(fp, "   <precTolerance>%f</precTolerance>\n", PREC_MASS_ERROR);
	fprintf(fp, "   <fragmError1>%f</fragmError1>\n", FRAGMENT_ERROR1);
	fprintf(fp, "   <fragmError2>%f</fragmError2>\n", FRAGMENT_ERROR2);

	// ecritures des donn�es sur les dbs
	DBReader	m_db;
	DBFileReader *pDBFileReader;
	
	if ((strcmp(DB1_PATH, "NO")) && (strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB1_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "   <db1path>%s</db1path>\n", pDBFileReader->GetLabel());
		fprintf(fp, "   <db1release>%s</db1release>\n", pDBFileReader->GetRelease());
		m_db.Open(DB2_PATH);
		pDBFileReader = m_db.GetFile(1);
		fprintf(fp, "   <db2path>%s</db2path>\n", pDBFileReader->GetLabel());
		fprintf(fp, "   <db2release>%s</db2release>\n", pDBFileReader->GetRelease());
		m_db.CloseAll();
	}

	if ((strcmp(DB1_PATH, "NO")) && (!strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB1_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "   <db1path>%s</db1path>\n", pDBFileReader->GetLabel());
		fprintf(fp, "   <db1release>%s</db1release>\n", pDBFileReader->GetRelease());
		fprintf(fp, "   <db2path></db2path>\n");
		fprintf(fp, "   <db2release></db2release>\n");
		m_db.CloseAll();
	}
	
	if ((!strcmp(DB1_PATH, "NO")) && (strcmp(DB2_PATH, "NO"))) {
		m_db.Open(DB2_PATH);
		pDBFileReader = m_db.GetFile(0);
		fprintf(fp, "   <db1path></db1path>\n");
		fprintf(fp, "   <db1release></db1release>\n");
		fprintf(fp, "   <db2path>%s</db2path>\n", pDBFileReader->GetLabel());
		fprintf(fp, "   <db2release>%s</db2release>\n", pDBFileReader->GetRelease());
		m_db.CloseAll();
	}
	
	if (TAXID_NB == 0) {
		fprintf(fp, "   <taxId>%i</taxId>\n", 1);
	}
	else { 
		for (int i = 0; i < TAXID_NB; i++) {
			fprintf(fp, "   <taxId>%i</taxId>\n", TAXID[i]);
		}
		fprintf(fp, "\n");
	}  
	
	
	if (strlen(AC_FILTER) > 4) {
		fprintf(fp, "   <acFilter>%s</acFilter>\n", AC_FILTER);
	}
	fprintf(fp, "   <enzyme>%s</enzyme>\n", ENZYME);
	fprintf(fp, "   <missedCleavageNb>%i</missedCleavageNb>\n", MISSED);
	
	if (m_MUTMOD) {
		fprintf(fp, "   <upLimitPM>%f</upLimitPM>\n", UP_LIMIT_RANGE_PM);
		fprintf(fp, "   <lowLimitPM>%f</lowLimitPM>\n", LOW_LIMIT_RANGE_PM);
		fprintf(fp, "   <upLimitMod>%f</upLimitMod>\n", UP_LIMIT_RANGE_MOD);
		fprintf(fp, "   <lowLimitMod>%f</lowLimitMod>\n", LOW_LIMIT_RANGE_MOD);
	}
	
	fprintf(fp, "</inputParameters>\n");	
}

