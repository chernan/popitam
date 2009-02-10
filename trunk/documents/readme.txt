
!!!! Please consider this distribution of Popitam as an experimental work about peptide
     characterization from tandem mass spectrometry data. 
     If you encounter any problem, or if you have questions, do not hesitate to ask for 
     help at the following email adresses:
     patricia.hernandez@isb-sib.ch
     tools@expasy.org
!!!!

//*********************************************************************************//
  CONTENT OF THIS FILE
//*********************************************************************************//
1) Overview of files and folders included in this distribution
2) Testing the local installation of Popitam
3) Building databases from fasta files
4) Using Popitam *without* the rescoring of Phenyx
5) Using Popitam *with* the rescoring of Phenyx   
//*********************************************************************************//

//*********************************************************************************//
  INSTRUCTIONS FOR USING POPITAM (ONLY LINUX!!!):
//*********************************************************************************//

//*********************************************************************************//
  1) Overview of files and folders included in this distribution
//*********************************************************************************//

This distribution of Popitam includes the following files/folders:

// -------------------------------------------------------------------------------

popitamDist:

- resources                         this directory contains some files used by Popitam
- dbs                               this directory contains protein databases and programs to create them
- rescoring                         this directory contains files used for rescoring Popitam's scenario using Phenyx's scoring functions 
- auto_popitam_rescored2pidres.pl   a script for running a small pipeline (Popitam + Phenyx rescorer)
- com.txt                           some examples of commands to run Popitam
- popitam                           the Popitam program 
- popParam.txt                      a default parameter file
- testPopitam.pl                    a perl script that runs Popitam on the test input spectra and
                                    compares the obtained outputs with the example outputs (located in popitamDist/test)

// -------------------------------------------------------------------------------
                               
popitamDist/test:                   some example input spectra and output files

// -------------------------------------------------------------------------------

popitamDist/dbs:

- createDB                  program for database indexing (only linux)
- demoDB_BASE_FORWARDandDECOY.fasta         a small demo database in fasta format 
- demoDB_BASE_FORWARDandDECOY.bin           the same database after indexing 
- demoDB_PHENHEADER.fasta                   a demo database with phenyx-type headers
- demoDB_CAMHEADER.fasta                    a demo database with a very simple header        

Please note that these are demo databases. The DECOY sequences are not shuffled. Consequently, 
the DECOY peptides are always grouped with the "non decoy" ones in the results.

//*********************************************************************************//
  2) Testing the local installation of Popitam
//*********************************************************************************//

First, you may want to test your Popitam installation. Just execute the script testPopitam.pl
in a shell.
In short, the script compares example outputs with obtained ones using a diff command. 
Please note that the test runs two times Popitam with specific example spectra against 
the demoDB database. If everything goes well, the last line of the test output will 
be: TEST SUCCESSFUL. In some cases, the test is not successful because of tiny differences 
in the scores... This is an issue we haven't fixed, but these differences doesn't really 
affect the interpretation of the output.

//*********************************************************************************//
  3) Building databases from fasta files
//*********************************************************************************//

You can create new indexed databases for any fasta file with a supported header type.
You currently have the choice between three types of headers: BASE, PHENHEADER and CAMHEADER.
Please run createDB without arguments to display its usage. 

//*********************************************************************************//
  4) Using Popitam *without* the rescoring of Phenyx:
//*********************************************************************************//

For a first run, you may try to run Popitam with example inputs and predefined commands 
(please look in file com.txt for some examples of commands).

For example, you can type:

popitam -r=NORMAL -s=UNKNOWN -m=1 -p=popParam.txt -d=test/TEST_1MOD.mgf -f=mgf -e=error.txt -o=outputOfPopitam.txt

The arguments are:
-r=NORMAL                do not change this one 
-s=UNKNOWN               do not change this one
-m=1                     you have the choice between 1 or 2; the number indicates the number 
                         of mass shifts (modification events) Popitam is looking for.
-p=popParam.txt          this is the parameter file used by Popitam (you can change the path of the 
                         database, the used taxonomy, and some other parameters in this file)
-d=test/TEST_1MOD.mgf    this is the input file (MSMS spectra to be analyzed)
-f=mgf                   this is the format of the input file (you have the choice between mgf and merged
                         dta)
-e=error.txt             if Popitam quits with an error, the error type should be written in this file
-o=out.txt               output filename; popitam will produce a human-readable text file (out.txt) 
                         and an xml file (out.txt.xml).

//*********************************************************************************//
  5) Using Popitam *with* the rescoring of Phenyx
//*********************************************************************************//

The script auto_popitam_rescored2pidres.pl allows to: 
- run Popitam (if asked)
- parse a Popitam output (XML) and extract its scenarii into a temp file
- rescore the scenarii and integrate the results into a pidres (or display on stdout)

Some help can be obtained with the command:
./auto_popitam_rescored2pidres.pl -h

Two important arguments are:
--alone (or -a): when running Popitam independently of a Phenyx's installation
--debug (or -d): with this option, information about the progress of the script is
                 displayed and temporary files are not erased

Example of commands:

Rescoring a Popitam output (xml):
./auto_popitam_rescored2pidres.pl --in=outputOfPopitam.txt.xml --out=outputOfPopitam.txt.pidres --scoring=rescoring/02_pm-rescoring/qtof.scoring.xml --alone --debug
--> 3 files are created:
      outputOfPopitam.txt.xml.scen:     input files used by the phenyx rescorer. Don't look at it as it is not interesting.
      outputOfPopitam.txt.xml.resc      rescored scenarios (raw text)
      outputOfPopitam.txt.xml.pidres    rescored scenarios (pidres format)

Running Popitam and rescoring:
./auto_popitam_rescored2pidres.pl --pop=popParam.txt --in=test/TEST_1MOD.mgf --mod=1 --scoring=rescoring/02_pm-rescoring/qtof.scoring.xml --out=outputOfPopitam.txt.pidres --alone --debug
--> 8 files are created:
      TEST_1MOD.mgf.filter.xml           spectrum file after filtering
      TEST_1MOD.mgf.1filter              spectrum file after filtering (format xml)
      TEST_1MOD.mgf.popout               Popitam original output (raw text) 
      TEST_1MOD.mgf.popout.sht           Popitam original output (short view)
      TEST_1MOD.mgf.popout.xml           Popitam original output (xml)
      TEST_1MOD.mgf_popout.xml.scen      input files used by the phenyx rescorer
      TEST_1MOD.mgf.popout.resc          rescored scenarios (raw text)
      outputOfPopitam.txt.pidres         rescored scenarios (pidres format)

Running Popitam with a pre-filtering of the spectra (for example, remove all spectra with less than 5 peaks) and rescoring:
./auto_popitam_rescored2pidres.pl --pop=popParam.txt --in=test/TEST_1MOD.mgf --fpeaks=5 --mod=1 --scoring=rescoring/02_pm-rescoring/qtof.scoring.xml --out=outputOfPopitam.txt.pidres --alone --debug
--> only file outputOfPopitam.txt.xml.pidres is created. This file contains the rescored scenarios in pidres format.

Running Popitam, rescoring, and importing the result into Phenyx (don't use the option --alone !)
./auto_popitam_rescored2pidres.pl --pop=popParam.txt --in=test/TEST_1MOD.mgf --mod=1 --scoring=rescoring/02_pm-rescoring/qtof.scoring.xml --out=outputOfPopitam.txt.pidres --user=yourUsername
--> you should be able to visualise the pidres file with the interface of Phenyx

You may need to add some paths in @INC (in case where some libraries such as PhenyxPerl are not found).
Then try to modify the environment variable PERL5LIB by adding the lacking paths.
You may also want to install the modules with cpan (type "cpan" and "install " +the module name).

