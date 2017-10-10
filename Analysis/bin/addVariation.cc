/*
 * addVariation.cc
 *
 *  Created on: Dec 17, 2014
 *      Author: kiesej
 */


/*
 * Simple program to add a systematic variation to a file containing
 * the nominal points (other systematics allowed)
 * Don't use this to add large bunches of systematics, for this purpose use
 * mergeSyst with the appropriate naming conventions.
 * This program is only meant for small checks.
 *
 * To execute: addVariation <var_name> <nominal file> <up_file> <down_file> (optional -o <output>)
 */

#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include <vector>
#include "TString.h"
#include "TtZAnalysis/Tools/interface/histoStackVector.h"
#include <string>

invokeApplication(){
	using namespace ztop;
	parser->setAdditionalDesciption(
			"* Simple program to add a systematic variation to a file containing  \
* the nominal points (other systematics allowed)  \
* Don\'t use this to add large bunches of systematics, for this purpose use  \
* mergeSyst with the appropriate naming conventions.  \
* This program is only meant for small checks.  \
*   \
* To execute: addVariation <var_name> <nominal file> <up_file> <down_file>");
	TString output=parser->getOpt<TString>("o","","specify output file name, otherwise nominal will be overwritten.");

	std::vector<TString> input=parser->getRest<TString>();

	if(input.size()!=4 || !fileExists(input.at(1)) || !fileExists(input.at(2)) || !fileExists(input.at(3))){
		parser->coutHelp(); //exits
	}
	if(output=="")
		output=input.at(1);

	histoStackVector* csvnom=new histoStackVector();
	histoStackVector* csvup=new histoStackVector();
	histoStackVector* csvdown=new histoStackVector();

	//try to load variations first (prob smaller)
	try{
            csvup->readFromFile((std::string)input.at(2));
	}catch(...){
		std::cout << "could not read up variation file"<<std::endl;
		delete csvnom; delete csvup; delete csvdown;
		return -1;
	}
	try{
            csvnom->readFromFile((std::string)input.at(1));
	}catch(...){
		std::cout << "could not read nominal file"<<std::endl;
		delete csvnom; delete csvup; delete csvdown;
		return -1;
	}

	csvnom->addMCErrorStackVector(input.at(0)+"_up",*csvup);
	delete csvup;

	try{
            csvdown->readFromFile((std::string)input.at(3));
	}catch(...){
		std::cout << "could not read down variation file"<<std::endl;
		delete csvnom; delete csvdown;
		return -1;
	}

	csvnom->addMCErrorStackVector(input.at(0)+"_down",*csvdown);
	delete csvdown;

	csvnom->writeToFile((std::string)output);
	delete csvnom;
	return 0;
}
