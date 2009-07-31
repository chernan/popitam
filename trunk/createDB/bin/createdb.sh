#!/bin/bash

app=`basename $0`

if [ $# != 2 ]; then
	echo "Usage: $app <inputdb> <outputdb>"
	echo "Where: inputdb is a fasta database complying to the swissprot/trembl format."
	echo "       outputdb is the output database file to produce."
	exit 1
fi

inputdb=$1
outputdb=$2
release=`date +%Y%m%d`

# Check whether we are dealing with an old swissprot db (prior to 2009).
# If it is the new format, convert it to the old format since createDB can only work with those.
prefix=`cat $inputdb | line`
prefix=${prefix:0:4}
if [ $prefix == ">sp|" ] || [ $prefix == ">tr|" ]; then
	tempdb=temp-$$.db.tmp
	echo "converting new swissprot db to old format..."
	sed 's/sp|//g;s/tr|//g' < $inputdb > $tempdb
	echo "done."
	./createDB $tempdb $outputdb swissprot ../nodes.txt  NULL $release BASE
	/bin/rm $tempdb
else
	./createDB $inputdb $outputdb swissprot ../nodes.txt  NULL $release BASE
fi
