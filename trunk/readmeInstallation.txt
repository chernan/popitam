How to install Popitam:

1) Make a checkout of Popitam's code
http://code.google.com/p/popitam/source/checkout

2) Compile the code of popitam and the code for indexing the databases:
make all

3) Create a first database (used as test)
createDB/bin/createDB createDB/testDBs/demoDB_BASE_FORWARDandDECOY.fasta bin/dbs/demoDB_BASE_FORWARDandDECOY.bin demoDB_FORWARDandDECOY NULL NULL BASE_FORWARDandDECOY_1.0 BASE|]
--> the usage of createDB is given by typing createDB in the command line without arguments)
--> the file demoDB_BASE_FORWARDandDECOY.bin is created 
--> you can, in the same way, create any database using your own fasta files, as long as the header type is supported

4) Test your local installation of Popitam
cd bin
perl testPopitam.pl
--> the test runs two times Popitam with specific example spectra against the demoDB database. If everything goes well, the last line of the test output will be: TEST SUCCESSFUL. In some cases, the test is not successful because of tiny differences in the scores... This is an issue we haven't fixed, but these differences doesn't really affect the interpretation of the output.
