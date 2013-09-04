#!/bin/sh



export CPLUS_INCLUDE_PATH=$CMSSW_BASE/src
ROOTFLAGS=`root-config --cflags`
ROOTLIBS=`root-config --libs`

CMSLIBS=$CMSSW_BASE/lib/$SCRAM_ARCH/
#declare -a libs
libs=("TopAnalysisZTopUtils" 
"TtZAnalysisDataFormats" 
"TtZAnalysisTools"
"TtZAnalysisAnalysis"
"FWCoreFWLite"
#"DataFormatsStdDictionaries"
#"CondFormatsJetMETObjects"
);

#echo $libs

#exit

scram b -j12

libdir=$CMSSW_BASE/src/TtZAnalysis/Analysis/lib

mkdir -p $libdir
linklibs=""

for (( i=0;i<${#libs[@]};i++)); do
linklibs="$linklibs -l${libs[${i}]}"
cp ${CMSLIBS}lib${libs[${i}]}.so $libdir
done

if [ $1 ];
then

echo compiling analyse.C
infile=analyse.C

g++ $ROOTFLAGS -fopenmp -I$CPLUS_INCLUDE_PATH -c -o $infile.o $infile
g++ -o analyse.exe -fopenmp -Wall $ROOTLIBS -Llib$linklibs -l${CMSSW_BASE}/src/TtZAnalysis/Tools/TUnfold/libunfold.so $infile.o

infile=mergeSyst.cc

echo compiling mergeSyst.cc

g++ $ROOTFLAGS -fopenmp -I$CPLUS_INCLUDE_PATH -c -o $infile.o $infile
g++ -o mergeSyst.exe -fopenmp -Wall $ROOTLIBS -Llib$linklibs $infile.o

else
	echo "only compiled analyzer code, NOT steering program (analyse.exe)"
fi
