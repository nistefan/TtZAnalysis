/*
 * ttbarXsecFitter.cc
 *
 *  Created on: Oct 9, 2014
 *      Author: kiesej
 */


#include "../interface/ttbarXsecFitter.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/Tools/interface/containerStack.h"
#include "TtZAnalysis/Tools/interface/containerStackVector.h"
#include "TtZAnalysis/Tools/interface/fileReader.h"
#include "Math/Functor.h"
#include "TtZAnalysis/Tools/interface/plotterControlPlot.h"
#include "TtZAnalysis/Tools/interface/texTabler.h"
#include "limits.h"

namespace ztop{

bool ttbarXsecFitter::debug=false;
TRandom3 * ttbarXsecFitter::random_=0;

void ttbarXsecFitter::readInput(const std::string & configfilename){
	if(debug)
		std::cout << "ttbarXsecFitter::readInput" <<std::endl;
	fitsucc_=false;

	fileReader fr;
	fr.setComment("#");
	fr.setDelimiter(",");
	//8 TeV
	fr.setStartMarker("[ datasets ]");
	fr.setEndMarker("[ end - datasets ]");
	fr.readFile(configfilename);
	//count number of datasets
	//size_t ndatasets=0;
	std::vector<std::string> datasetmarkers=fr.getMarkerValues("dataset");
	for(size_t i=0;i<datasetmarkers.size();i++){
		std::cout << datasetmarkers.at(i) <<std::endl;
		fileReader frd;
		frd.setComment("#");
		frd.setDelimiter(",");
		//8 TeV
		frd.setStartMarker("[ " +datasetmarkers.at(i) +  " ]");
		frd.setEndMarker("[ end " +datasetmarkers.at(i) + " ]");
		frd.readFile(configfilename);

		double lumi = frd.getValue<double>("lumi");
		double lumiunc = frd.getValue<double>("lumiunc");
		double energy = frd.getValue<double>("energy");
		double topmass = frd.getValue<double>("topmass");
		TString name = datasetmarkers.at(i);
		name.ReplaceAll(" ","");
		name.ReplaceAll("dataset-","");
		double xsec=getTtbarXsec(topmass,energy);

		datasets_.push_back(dataset(lumi,lumiunc,xsec,name));
		//DEBUG
		std::cout << "ttbarXsecFitter::readInput: created dataset: "
				<< lumi << ", "<< lumiunc <<", " << xsec << ", "<<name <<std::endl;
	}
	if(datasets_.size()<1)
		throw std::logic_error("ttbarXsecFitter::readInput: no dataset description found");


	for(size_t i=0;i<datasets_.size();i++){
		datasets_.at(i).readStacks(configfilename,replaceininfiles_,removesyst_);
	}
	for(size_t i=0;i<datasets_.size();i++){
		for(size_t j=0;j<datasets_.size();j++){
			if(i==j)
				continue;
			datasets_.at(i).equalizeIndices(datasets_.at(j));
		}
	}
	//now idices are fixed
	for(size_t i=0;i<datasets_.size();i++){
		datasets_.at(i).createXsecIdx();

	}
	createContinuousDependencies();
	//fileReader::debug=true;
	///////////// READ PRIORS ////////////

	if(!nopriors_){
		fr.setStartMarker("[ priors ]");
		fr.setEndMarker("[ end - priors ]");
		fr.setDelimiter("=");
		fr.readFile(configfilename);
		for(size_t i=0;i<fr.nLines();i++){
			if(fr.nEntries(i)<2)
				continue;
			TString sysname= fr.getData<TString>(i,0);
			TString priorstr=fr.getData<TString>(i,1);
			if(priorstr=="box")
				setPrior(sysname, prior_box);
			if(priorstr=="boxright")
				setPrior(sysname, prior_narrowboxright);
			if(priorstr=="boxleft")
				setPrior(sysname, prior_narrowboxleft);
			if(priorstr=="float")
				setPrior(sysname, prior_float);
			if(priorstr=="gauss")
				setPrior(sysname, prior_gauss);
			if(priorstr=="fixed")
				setPrior(sysname, prior_parameterfixed);
		}
	}
	if(!removesyst_){
		fr.setStartMarker("[ full extrapolation ]");
		fr.setEndMarker("[ end - full extrapolation ]");
		fr.readFile(configfilename);
		for(size_t i=0;i<fr.nLines();i++){
			if(fr.nEntries(i)!=1)
				continue;
			addFullExtrapolError(fr.getData<TString>(i,0));
		}
	}

	if(debug){
		std::cout << "ttbarXsecFitter::readInput: done. " <<std::endl;
	}
}
void ttbarXsecFitter::createPseudoDataFromMC(container1D::pseudodatamodes mode){
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).createPseudoDataFromMC(mode);
}

void ttbarXsecFitter::dataset::createPseudoDataFromMC(container1D::pseudodatamodes mode){
	if(!random_){ //first call
		random_ = new TRandom3();
		datacontsorig_nbjets_=dataconts_nbjets_; //safe originals
		backgroundcontsorig_nbjets_=backgroundconts_nbjets_; //safe originals
		signalcontsorig_nbjets_=signalconts_nbjets_; //safe originals

		//there are no syst anymore (only dummies)
		/*for(size_t i=0;i<priors_.size();i++){
			if(i!=xsecidx7_ && i != xsecidx8_)
				setPrior(parameternames_.at(i),prior_parameterfixed);
		}
		 */
		//minimize output
		std::cout << "\nEntering pseudo-experiments mode for dataset " << name_ <<" \n" <<std::endl;
		if(!debug)simpleFitter::printlevel=-1;
		//debug=false;
	}
	for(size_t i=0;i<dataconts_nbjets_.size();i++){
		container1D tmpmc=backgroundcontsorig_nbjets_.at(i) + signalcontsorig_nbjets_.at(i);
		dataconts_nbjets_.at(i)=tmpmc.createPseudoExperiment(random_,&datacontsorig_nbjets_.at(i),mode);
		backgroundconts_nbjets_.at(i)=backgroundcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode);
		signalconts_nbjets_.at(i)=signalcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode);
		signalpsmigconts_nbjets_.at(i)=signalpsmigcontsorig_nbjets_.at(i).createPseudoExperiment(random_,0,mode);
	}
	signalshape_nbjet_.clear();
	data_nbjet_.clear();
	background_nbjet_.clear();
}

void ttbarXsecFitter::createContinuousDependencies(){
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).createContinuousDependencies(useMConly_);

	ndependencies_=datasets_.at(0).signalshape(0).getNDependencies();
	if(debug)
		std::cout << "ttbarXsecFitter::createContinuousDependencies: created dep: "
		<< ndependencies_<<std::endl;
	fittedparas_.resize(ndependencies_,0);
	priors_.resize(ndependencies_,prior_gauss);
	parameternames_=datasets_.at(0).signalshape(0).getSystNames();

	if(debug)
		for(size_t i=0;i<parameternames_.size();i++){
			std::cout << "\t"<< parameternames_.at(i) <<std::endl;
		}
}
void ttbarXsecFitter::dataset::createContinuousDependencies(bool useMConly){
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createContinuousDependencies" <<std::endl;

	size_t bjetcount=0;
	//bool eighttev=true;
	signalshape_nbjet_.clear();
	data_nbjet_.clear();
	background_nbjet_.clear();
	for(size_t it=0;it<signalconts_nbjets_.size();it++){

		signalshape_nbjet_.push_back(createLeptonJetAcceptance(signalconts_nbjets_,signalpsmigconts_nbjets_,signalvisgenconts_nbjets_, bjetcount));
		container1D temp=dataconts_nbjets_.at(it);
		if(useMConly){
			temp=signalconts_nbjets_.at(it) + backgroundconts_nbjets_.at(it);
			temp.setAllErrorsZero(false);
		}

		variateContainer1D tmpvarc;
		tmpvarc.import(temp);
		data_nbjet_.push_back(tmpvarc);

		tmpvarc.import(backgroundconts_nbjets_.at(it));
		background_nbjet_.push_back(tmpvarc);

		bjetcount++;
	}
}

size_t ttbarXsecFitter::nDatasets()const{
	return datasets_.size();
}
const TString& ttbarXsecFitter::datasetName(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::datasetName: index out of range");
	return datasets_.at(datasetidx).getName();
}
void ttbarXsecFitter::setPrior(const TString& sysname, priors prior){
	if(debug)
		std::cout << "ttbarXsecFitter::setPrior" <<std::endl;

	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::setPrior: first read in plots");
	}
	std::vector<TString> systs=datasets_.at(0).getSystNames();
	size_t idx=std::find(systs.begin(),systs.end(),sysname) - systs.begin();
	if(idx == systs.size())
		throw std::out_of_range("ttbarXsecFitter::setPrior: requested variation not found");

	priors_.at(idx) = prior;
}
void ttbarXsecFitter::addFullExtrapolError(const TString& sysname){
	if(debug)
		std::cout << "ttbarXsecFitter::addFullError" <<std::endl;

	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::addFullExtrapolError: first read in plots");
	}
	std::vector<TString> systs=datasets_.at(0).getSystNames();
	size_t idx=std::find(systs.begin(),systs.end(),sysname) - systs.begin();
	if(idx == systs.size()){
		std::string errstr=(std::string)"ttbarXsecFitter::addFullError: requested variation not found: "+sysname.Data();
		throw std::out_of_range(errstr);
	}
	if(std::find(addfullerrors_.begin(),addfullerrors_.end(),idx) == addfullerrors_.end()){//new
		addfullerrors_.push_back(idx);
	}
}

containerStack ttbarXsecFitter::produceStack(bool fittedvalues,size_t bjetcat,size_t datasetidx,double& chi2)const{
	if(debug)
		std::cout << "ttbarXsecFitter::produceStack" <<std::endl;
	checkSizes();
	if(bjetcat > 2)
		throw std::logic_error("ttbarXsecFitter::produceStack: allowed b-jet categories: 0,1,2. 0 includes 3+");

	if(datasetidx >= datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::produceStack: dataset index out of range");

	size_t nbjet=bjetcat;

	containerStack out;
	container1D data,signal,background;
	std::vector<double> fittedparas;
	if(fittedvalues)
		fittedparas=fittedparas_;
	else
		fittedparas.resize(fittedparas_.size(),0);


	if(debug)
		std::cout << "ttbarXsecFitter::produceStack: data: "<< datasets_.at(datasetidx).data(nbjet).getNDependencies() << std::endl;
	data=datasets_.at(datasetidx).data(nbjet).exportContainer( fittedparas );

	if(debug)
		std::cout << "ttbarXsecFitter::produceStack: background" << std::endl;
	background=datasets_.at(datasetidx).background(nbjet).exportContainer( fittedparas );

	if(debug)
		std::cout << "ttbarXsecFitter::produceStack: normalization" << std::endl;
	double multi = datasets_.at(datasetidx).lumi() *
			(fittedparas[datasets_.at(datasetidx).xsecIdx()] + datasets_.at(datasetidx).xsecOffset());

	if(norm_nbjet_global_)
		multi*=datasets_.at(datasetidx).normalization(nbjet).getValue(fittedparas);

	if(debug)
		std::cout << "ttbarXsecFitter::produceStack: normalization (2)" << std::endl;
	multi/=datasets_.at(datasetidx).signalshape(nbjet).getIntegral(&fittedparas[0]);

	if(debug)
		std::cout << "ttbarXsecFitter::produceStack: signalshape" << std::endl;
	container1D exportc=datasets_.at(datasetidx).signalshape(nbjet).exportContainer( fittedparas );
	signal=  exportc * multi ;


	out.push_back(data,"data",1,1,99);
	out.push_back(background,"background",601,1,2);
	out.push_back(signal,"signal",633,1,1);

	out.setName(out.getName() + "_" + datasets_.at(datasetidx).getName());

	chi2=out.chi2();

	return out;
}


int ttbarXsecFitter::fit(std::vector<float>& xsecs, std::vector<float>& errup ,std::vector<float>& errdown){
	if(debug)
		std::cout << "ttbarXsecFitter::fit" <<std::endl;
	if(datasets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::fit: First read in datasets");
	}
	if(!datasets_.at(0).readyForFit())
		createContinuousDependencies();
	checkSizes();
	//no except: sizes ok
	//	fittedparas_.at(xsecidx8_)=-8.; //other starting value closer to measured value TEST

	fitter_.setParameterNames(parameternames_);
	fitter_.setRequireFitFunction(false);
	std::vector<double> stepwidths;
	stepwidths.resize(fittedparas_.size(),0.1);
	for(size_t i=0;i<datasets_.size();i++){
		stepwidths.at(datasets_.at(i).xsecIdx())=1;
	}


	fitter_.setParameters(fittedparas_,stepwidths);
	//set fixed paras
	for(size_t i=0;i<priors_.size();i++){
		if(priors_.at(i) == prior_parameterfixed)
			fitter_.setParameterFixed(i,true);
		else
			fitter_.setParameterFixed(i,false);
	}
	fitter_.setMinimizer(simpleFitter::mm_minuit2);
	if(fittedparas_.size() != ndependencies_)
		throw std::logic_error("ttbarXsecFitter::fit: fittedparas.size() != ndep");
	functor_ = ROOT::Math::Functor(this,&ttbarXsecFitter::toBeMinimized,ndependencies_);//ROOT::Math::Functor f(this,&ttbarXsecFitter::toBeMinimized,ndependencies_);
	fitter_.setMinFunction(&functor_);

	//fitter_.setParameterFixed(lumiidx7_);
	//if(includes7TeV())

	//pre fit to get rough estimate
	fitter_.setStrategy(0);
	fitter_.setTolerance(1);
	fitter_.fit();
	fitter_.feedErrorsToSteps();
	if(!silent_)
		std::cout << "First rough fit done" <<std::endl;
	fitter_.setStrategy(2);
	fitter_.setTolerance(0.01);
	if(!nominos_){
		for(size_t i=0;i<datasets_.size();i++)
			fitter_.setAsMinosParameter(datasets_.at(i).xsecIdx());
	}
	fitter_.fit();
	//feed back


	if(!silent_)
		for(size_t i=0;i<datasets_.size();i++){
			size_t xsecidx=datasets_.at(i).xsecIdx();
			double xsecoff=datasets_.at(i).xsecOffset();
			std::cout << "fitted xsecs: "<< datasets_.at(i).getName()<< ": "<<fitter_.getParameters()->at(xsecidx)+xsecoff <<
					"+" << fitter_.getParameterErrUp()->at(xsecidx) *100/(fitter_.getParameters()->at(xsecidx)+xsecoff) <<
					"-" << fitter_.getParameterErrDown()->at(xsecidx) *100/(fitter_.getParameters()->at(xsecidx)+xsecoff) <<
					std::endl;
		}
	if(fitter_.wasSuccess()){
		fitsucc_=true;
		if(parameterwriteback_){
			fittedparas_ = *fitter_.getParameters();
		}
		xsecs.clear();
		errup.clear();
		errdown.clear();
		for(size_t i=0;i<datasets_.size();i++){
			size_t xsecidx=datasets_.at(i).xsecIdx();
			double xsecoff=datasets_.at(i).xsecOffset();
			xsecs.push_back(fitter_.getParameters()->at(xsecidx)+xsecoff);
			errup.push_back(fitter_.getParameterErrUp()->at(xsecidx));
			errdown.push_back(fitter_.getParameterErrDown()->at(xsecidx));
		}
		if(!nominos_ && !fitter_.minosWasSuccess()){
			throw std::runtime_error("ttbarXsecFitter::fit(): Minos error failed!");
		}

		return 0;
	}
	//else
	if(!silent_)
		std::cout << "ttbarXsecFitter::fit(): Fit failed" <<std::endl;
	throw std::runtime_error("ttbarXsecFitter::fit(): Fit failed");
	return -1;
}
int ttbarXsecFitter::fit(){
	std::vector<float> dummy;
	return fit(dummy,dummy,dummy);
}

container1D ttbarXsecFitter::getCb (bool fittedvalues,size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getCb: dataset index out of range");

	//if(eighttev) return container_c_b_.at(0); else return container_c_b_.at(1);
	variateContainer1D temp=datasets_.at(datasetidx).container_c_b();

	if(fittedvalues)
		return temp.exportContainer(fittedparas_);
	else
		return temp.exportContainer();

}
container1D ttbarXsecFitter::getEps(bool fittedvalues, size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getEps: dataset index out of range");

	//{if(eighttev) return container_eps_b_.at(0); else return container_eps_b_.at(1);}
	variateContainer1D temp=datasets_.at(datasetidx).container_eps_b();

	if(fittedvalues)
		return temp.exportContainer(fittedparas_);
	else
		return temp.exportContainer();

}

container2D  ttbarXsecFitter::getCorrelations()const{
	container2D out;
	fitter_.fillCorrelationCoefficients(&out);
	return out;
}


double ttbarXsecFitter::getXsec(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	return fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdx())+datasets_.at(datasetidx).xsecOffset();
}

double ttbarXsecFitter::getXsecOffset(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsecOffset: dataset index out of range");
	return datasets_.at(datasetidx).xsecOffset();
}

double ttbarXsecFitter::getVisXsec(size_t datasetidx)const{
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getXsec: dataset index out of range");
	double visxsec=getXsec(datasetidx);//fitter_.getParameters()->at(datasets_.at(datasetidx).xsecIdx())+datasets_.at(datasetidx).xsecOffset();
	visxsec*=datasets_.at(datasetidx).fullToVisCorrFact().getValue(fitter_.getParameters());
	return visxsec;
}

double ttbarXsecFitter::getParaError(size_t idx)const{
	if(idx>=fitter_.getParameters()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaError");
	return fitter_.getParameterErr(idx);
}
TString ttbarXsecFitter::getParaName(size_t idx)const{
	if(idx>=fitter_.getParameterNames()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaName");
	return fitter_.getParameterNames()->at(idx);
}
void ttbarXsecFitter::getParaErrorContributionToXsec(int idxin, size_t datasetidx,double&up,double&down,bool& anticorr){
	if(idxin>=(int)fitter_.getParameters()->size())
		throw std::out_of_range("ttbarXsecFitter::getParaErrorContributionToXsec: index out of range");
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::getParaErrorContributionToXsec: dataset index out of range");

	size_t xsecidx=datasets_.at(datasetidx).xsecIdx() ;
	double xsecoffset=datasets_.at(datasetidx).xsecOffset() ;
	double xsec=fitter_.getParameter(xsecidx)+xsecoffset;
	if(!nosystbd_){
		if(idxin>=0){

			size_t idx=idxin;
			anticorr=fitter_.getCorrelationCoefficient(xsecidx,idx)<0;

			fitter_.getParameterErrorContribution(idx,xsecidx,up,down);
			up/=xsec;
			down/=xsec;
			return;
		}
		else{ //stat
			fitter_.getStatErrorContribution(xsecidx,up,down);
			up/=xsec;
			down/=xsec;
			return;
		}
	}
	else{
		up=1;
		down=1;
		return;
	}
}
void ttbarXsecFitter::createSystematicsBreakdowns(){
	for(size_t i=0;i<datasets_.size();i++)
		createSystematicsBreakdown(i);
}

void ttbarXsecFitter::createSystematicsBreakdown(size_t datasetidx){
	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::createSystematicsBreakdown: dataset index out of range");
	size_t xsecidx=datasets_.at(datasetidx).xsecIdx();
	std::cout << "creating systematics breakdown for " << datasets_.at(datasetidx).getName() <<std::endl;

	formatter fmt;
	TString cmsswbase=getenv("CMSSW_BASE");
	fmt.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());

	for(size_t i=0;i<getNParameters();i++){
		displayStatusBar(i,getNParameters());
		bool isxsec=false;
		for(size_t j=0;j<datasets_.size();j++)
			if(datasets_.at(j).xsecIdx() == i){
				isxsec=true;
				break;
			}
		if(isxsec) continue;
		TString name=fmt.translateName( getParaName(i));
		double errup,errdown;
		bool anitcorr;
		getParaErrorContributionToXsec(i,datasetidx,errup,errdown,anitcorr);
		dataset::systematic_unc tmpunc;
		tmpunc.name=name;
		tmpunc.errdown=errdown;
		tmpunc.errup=errup;
		tmpunc.constr= fitter_.getParameterErr(i);
		tmpunc.pull= fitter_.getParameters()->at(i);
		datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
		datasets_.at(datasetidx).postFitSystematicsVis().push_back(tmpunc);

	}


	std::cout << "\nadding extrapolation uncertainties..." <<std::endl;

	std::vector<double> fulladdtoerrup2,fulladdtoerrdown2,visaddtoerrup2,visaddtoerrdown2;
	dataset::systematic_unc tmpunc;
	for(size_t i=0;i<addfullerrors_.size();i++){
		displayStatusBar(i,addfullerrors_.size());
		bool anticorr=false;
		size_t idx=addfullerrors_.at(i);
		TString name=fmt.translateName( getParaName(idx) );
		name+=" (extr)";
		float exup=getExtrapolationError(datasetidx,idx,true,true);
		float exdown=getExtrapolationError(datasetidx,idx,false,true);
		double errup,errdown;
		bool anitcorr;
		getParaErrorContributionToXsec(idx,datasetidx,errup,errdown,anitcorr);
		if(anticorr){
			fulladdtoerrup2.push_back(-errdown*errdown);
			fulladdtoerrdown2.push_back(-errup*errup);
			visaddtoerrup2.push_back(-errdown*errdown);
			visaddtoerrdown2.push_back(-errup*errup);
		}
		else{
			fulladdtoerrdown2.push_back(-errdown*errdown);
			fulladdtoerrup2.push_back(-errup*errup);
			visaddtoerrup2.push_back(-errdown*errdown);
			visaddtoerrdown2.push_back(-errup*errup);
		}

		///TAKE INTO ACCOUNT THE CONSTRAINTS HERE!! THE PARA ERRORS ARE NOT ASYMM

		double totup=exup+errup;
		double totdown=exdown+errdown;

		bool corranti;
		double tmp=getMaxVar(true,totup,totdown,corranti);
		fulladdtoerrup2.push_back(tmp*tmp);
		tmp=getMaxVar(false,totup,totdown,corranti);
		fulladdtoerrdown2.push_back(tmp*tmp);


		tmpunc.name=name;
		tmpunc.errdown=exup;
		tmpunc.errup=exdown;
		tmpunc.pull=999;
		tmpunc.constr=999;

		datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
		// datasets_.at(datasetidx).postFitSystematicsVis().push_back(tmpunc);
		getMaxVar(true,exup,exdown,corranti);

		if(anticorr){
			fulladdtoerrup2.push_back(-errdown*errdown);
			fulladdtoerrdown2.push_back(-errup*errup);
			visaddtoerrup2.push_back(-errdown*errdown);
			visaddtoerrdown2.push_back(-errup*errup);
		}
		else{
			fulladdtoerrdown2.push_back(-errdown*errdown);
			fulladdtoerrup2.push_back(-errup*errup);
			visaddtoerrdown2.push_back(-errdown*errdown);
			visaddtoerrup2.push_back(-errup*errup);
		}


		exup=getExtrapolationError(datasetidx,idx,true,false);
		exdown=getExtrapolationError(datasetidx,idx,false,false);

		totup=exup+errup;
		totdown=exdown+errdown;

		tmp=getMaxVar(true,totup,totdown,corranti);
		visaddtoerrup2.push_back(tmp*tmp);
		tmp=getMaxVar(false,totup,totdown,corranti);
		visaddtoerrdown2.push_back(tmp*tmp);

		tmpunc.errdown=exup;
		tmpunc.errup=exdown;

		datasets_.at(datasetidx).postFitSystematicsVis().push_back(tmpunc);
	}

	//statistics
	double errupstat,errdownstat;
	bool anticorr=false;//dummy
	getParaErrorContributionToXsec(-1,datasetidx,errupstat,errdownstat,anticorr);

	tmpunc.name="Stat";
	tmpunc.errup=errupstat;
	tmpunc.errdown=errdownstat;
	tmpunc.pull=999;
	tmpunc.constr=999;

	datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);
	datasets_.at(datasetidx).postFitSystematicsVis().push_back(tmpunc);

	///finally: cross section error
	double xsecoffset=datasets_.at(datasetidx).xsecOffset();
	double fullpsxsec=(fitter_.getParameter(xsecidx) + xsecoffset);
	double fullrelxsecerrup=fitter_.getParameterErrUp()->at(xsecidx)/fullpsxsec;
	double fullrelxsecerrdown=fitter_.getParameterErrDown()->at(xsecidx)/fullpsxsec;

	//double extrCorr= datasets_.at(datasetidx).eps_emu_fulltovis().getValue(fittedparas_);
	//double vispsxsec=extrCorr * fullpsxsec;
	double vispsrelxsecerrup=fullrelxsecerrup; //no extrapolation yet
	double vispsrelxsecerrdown=fullrelxsecerrdown; //no extrapolation yet

	for(size_t i=0;i<fulladdtoerrup2.size();i++)
		fullrelxsecerrup=sqrt(fullrelxsecerrup*fullrelxsecerrup+fulladdtoerrup2.at(i));
	for(size_t i=0;i<fulladdtoerrdown2.size();i++)
		fullrelxsecerrdown=sqrt(fullrelxsecerrdown*fullrelxsecerrdown+fulladdtoerrdown2.at(i));

	tmpunc.name="Total";
	tmpunc.errup=fullrelxsecerrup;
	tmpunc.errdown=fullrelxsecerrdown;

	datasets_.at(datasetidx).postFitSystematicsFull().push_back(tmpunc);


	for(size_t i=0;i<visaddtoerrup2.size();i++)
		vispsrelxsecerrup=sqrt(vispsrelxsecerrup*vispsrelxsecerrup+visaddtoerrup2.at(i));
	for(size_t i=0;i<visaddtoerrdown2.size();i++)
		vispsrelxsecerrdown=sqrt(vispsrelxsecerrdown*vispsrelxsecerrdown+visaddtoerrdown2.at(i));

	tmpunc.errup=vispsrelxsecerrup;
	tmpunc.errdown=vispsrelxsecerrdown;
	datasets_.at(datasetidx).postFitSystematicsVis().push_back(tmpunc);

}

texTabler ttbarXsecFitter::makeSystBreakDownTable(size_t datasetidx,bool inclusive){

	/*
	 * Split this function in a part where the systematic contributions are
	 * calculated and saved per dataset (for extrapol unc minos errors up/down)
	 * and then a function to print them to a table
	 */


	if(datasetidx>=datasets_.size())
		throw std::out_of_range("ttbarXsecFitter::makeSystBreakdown: dataset index out of range");


	if((inclusive && datasets_.at(datasetidx).postFitSystematicsFull().size()<1) ||
			(!inclusive && datasets_.at(datasetidx).postFitSystematicsVis().size()<1))
		createSystematicsBreakdown(datasetidx);

	formatter fmt;
	texTabler table(" l | c | c | c ");
	table << "Name" << "Pull" << "Constr / $\\sigma$" << "Contribution [\\%]";
	table << texLine();
	std::vector<dataset::systematic_unc> * unc=&datasets_.at(datasetidx).postFitSystematicsVis();
	if(inclusive)
		unc=&datasets_.at(datasetidx).postFitSystematicsFull();

	for(size_t i=0;i<unc->size();i++){
		dataset::systematic_unc * sys=&unc->at(i);

		bool anticorr=false;
		getMaxVar(true,sys->errup,sys->errdown,anticorr);
		TString errstr;

		if(fabs(sys->errdown) == fabs(sys->errup)){
			if(anticorr)
				errstr="$\\mp{"+fmt.toTString(fmt.round(100*sys->errdown,0.1))+"}$";
			else
				errstr="$\\pm{"+fmt.toTString(fmt.round(100*sys->errup,0.1))+ "}$";
		}
		else{
			if(anticorr)
				errstr="$\\mp^{"+fmt.toTString(fmt.round(100*fabs(sys->errdown),0.1))+"}_{"+fmt.toTString(fmt.round(100*fabs(sys->errup),0.1))+ "}$";
			else
				errstr="$\\pm{"+fmt.toTString(fmt.round(100*fabs(sys->errup),0.1))+ "}_{"+fmt.toTString(fmt.round(100*fabs(sys->errdown),0.1))+"}$";;
		}
		if(sys->name == "Total")
			table <<texLine(1);
		if(sys->pull < 900)
			table << sys->name << fmt.round( sys->pull, 0.1)<<
			fmt.round( sys->constr, 0.1)	<< errstr;
		else
			table << sys->name << " "<< " "	<< errstr;
	}
	table <<texLine(1);
	float fullxsec=fitter_.getParameter(datasets_.at(datasetidx).xsecIdx()) + datasets_.at(datasetidx).xsecOffset();
	if(inclusive){
		table <<fmt.translateName( getParaName(datasets_.at(datasetidx).xsecIdx()) )<<
				" " << " "
				<< fmt.toTString(fmt.round(fullxsec,0.1))+" pb";
	}
	else{
		table <<fmt.translateName( getParaName(datasets_.at(datasetidx).xsecIdx()) )+ " vis"<<
				" " << " "
				<< fmt.toTString(fmt.round(getVisXsec(datasetidx),0.01))+" pb";
	}
	table << texLine();
	return table;


	/*
	size_t xsecidx=datasets_.at(datasetidx).xsecIdx();
	formatter fmt;

	TString cmsswbase=getenv("CMSSW_BASE");
	fmt.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());
	texTabler table(" l | c | c | c ");
	table << "Name" << "Pull" << "Constr / $\\sigma$" << "Contribution [\\%]";
	table << texLine();
	std::cout << "Evaluating systematic uncertainties for " << datasets_.at(datasetidx).getName() <<std::endl;


	for(size_t i=0;i<getNParameters();i++){
		displayStatusBar(i,getNParameters());
		bool isxsec=false;
		for(size_t j=0;j<datasets_.size();j++)
			if(datasets_.at(j).xsecIdx() == i){
				isxsec=true;
				break;
			}
		if(isxsec) continue;
		TString name=fmt.translateName( getParaName(i));
		double errup,errdown;
		bool anitcorr;
		getParaErrorContributionToXsec(i,datasetidx,errup,errdown,anitcorr);
		TString errstr;
		if(anitcorr)
			errstr="$\\mp{"+fmt.toTString(fmt.round(100*errdown,0.1))+"}$";
		else
			errstr="$\\pm{"+fmt.toTString(fmt.round(100*errup,0.1))+ "}$";


		if(std::find(addfullerrors_.begin(),addfullerrors_.end(),i)!=addfullerrors_.end())
			name+=" (vis)";


		table << name << fmt.round( fitter_.getParameters()->at(i), 0.1)<<
				fmt.round( fitter_.getParameterErr(i), 0.1)	<< errstr;

	}
	table <<texLine(1);
	double xsecoffset=datasets_.at(datasetidx).xsecOffset();
	//double xsec=fitter_.getParameter(xsecidx) + xsecoffset;
	///add extrapolation in addition:
	std::vector<double> addtoerrup2,addtoerrdown2;

	std::cout << "\nAdding extrapolation uncertainties for " << datasets_.at(datasetidx).getName() <<std::endl;

	for(size_t i=0;i<addfullerrors_.size();i++){
		displayStatusBar(i,addfullerrors_.size());
		bool anticorr=false;
		size_t idx=addfullerrors_.at(i);
		TString name=fmt.translateName( getParaName(idx) );
		name+=" (extr)";
		//	extendedVariable * var=&datasets_.at(datasetidx).eps_emu();


		float exup=getExtrapolationError(datasetidx,idx,true,true);
		float exdown=getExtrapolationError(datasetidx,idx,false,true);
		double errup,errdown;
		bool anitcorr;
		getParaErrorContributionToXsec(idx,datasetidx,errup,errdown,anitcorr);
		if(anticorr){
			errup=-fabs(errup);
			errdown=fabs(errdown);
			addtoerrup2.push_back(-errdown*errdown);
			addtoerrdown2.push_back(-errup*errup);
		}
		else{
			errup=fabs(errup);
			errdown=-fabs(errdown);
			addtoerrdown2.push_back(-errdown*errdown);
			addtoerrup2.push_back(-errup*errup);
		}

		///TAKE INTO ACCOUNT THE CONSTRAINTS HERE!! THE PARA ERRORS ARE NOT ASYMM

		double totup=exup+errup;
		double totdown=exdown+errdown;

		bool corranti;
		double tmp=getMaxVar(true,totup,totdown,corranti);
		addtoerrup2.push_back(tmp*tmp);
		tmp=getMaxVar(false,totup,totdown,corranti);
		addtoerrdown2.push_back(tmp*tmp);

		getMaxVar(true,exup,exdown,corranti);


		TString errstr;


		if(corranti)
			errstr="$\\mp^{"+fmt.toTString(fmt.round(100*fabs(exup),0.1))
			+"}_{"+fmt.toTString(fmt.round(100*fabs(exdown),0.1)) +"}$";
		else
			errstr="$\\pm^{"+fmt.toTString(fmt.round(100*fabs(exup),0.1))
			+"}_{"+fmt.toTString(fmt.round(100*fabs(exdown),0.1)) +"}$";

		table << name << "-" <<
				"-"	<< errstr;
	}
	std::cout << std::endl;
	//add stat
	double errupstat,errdownstat;
	bool anticorr=false;//dummy
	getParaErrorContributionToXsec(-1,datasetidx,errupstat,errdownstat,anticorr);
	TString errstrstat="$\\pm{"+fmt.toTString(fmt.round(100*errdownstat,0.1))+ "}$";
	table << texLine();
	table << "Stat" << " " << " " << errstrstat;

	//add total
	table << texLine();

	double fullrelxsecerrup=fitter_.getParameterErrUp()->at(xsecidx)/(fitter_.getParameter(xsecidx) + xsecoffset);

	//////change this and take into account correlations
	for(size_t i=0;i<addtoerrup2.size();i++)
		fullrelxsecerrup=sqrt(fullrelxsecerrup*fullrelxsecerrup+addtoerrup2.at(i));
	double fullrelxsecerrdown=fitter_.getParameterErrDown()->at(xsecidx)/(fitter_.getParameter(xsecidx) + xsecoffset);
	for(size_t i=0;i<addtoerrdown2.size();i++)
		fullrelxsecerrdown=sqrt(fullrelxsecerrdown*fullrelxsecerrdown+addtoerrdown2.at(i));

	TString errstrxsec="$\\pm^{"+ fmt.toTString(fmt.round(100*fullrelxsecerrup,0.1)) +
			"}_{"+ fmt.toTString(fmt.round(100*fullrelxsecerrdown,0.1))+"}$";
	table << "Total" << " " << " "
			<< errstrxsec;
	table << texLine(2);
	table <<fmt.translateName( getParaName(xsecidx) ) << " " << " " << fmt.toTString(fmt.round((fitter_.getParameter(xsecidx) + xsecoffset),0.1))+" pb";
	table << texLine();
	return table;
	 */

}

texTabler ttbarXsecFitter::makeCorrTable() const{
	TString format=" l ";
	container2D corr=getCorrelations();
	for(size_t i=1;i<corr.getBinsX().size()-1;i++)
		format+=" | c ";
	texTabler tab(format);

	formatter Formatter;
	TString cmsswbase=getenv("CMSSW_BASE");
	Formatter.readInNameTranslateFile((cmsswbase+"/src/TtZAnalysis/Analysis/configs/general/SystNames.txt").Data());
	for(size_t j=0;j<corr.getBinsY().size()-1;j++){
		for(size_t i=0;i<corr.getBinsX().size()-1;i++){
			//names
			if(i && !j)
				tab << "";//Formatter.translateName(parameternames_.at(i-1));
			else if(j && !i)
				tab << Formatter.translateName(parameternames_.at(j-1));
			else if (!j && !i)
				tab << "";
			else if(i<=j){
				float content=corr.getBinContent(i,j);
				if(fabs(content)<0.3)
					tab << Formatter.round(corr.getBinContent(i,j),0.01);
				else
					tab <<  "\\textbf{" +Formatter.toTString(Formatter.round(corr.getBinContent(i,j),0.01)) +"}";
			}
			else
				tab << "";
		}
		tab<<texLine(); //row done
	}

	return tab;
}

// can not be const due to root limitations
// do size checks before!
double ttbarXsecFitter::toBeMinimized(const double * variations){
	static size_t ncalls=0;
	static double lastout=10000;
	ncalls++;
	if(variations[0]!=variations[0]){
		std::cout << "Exception in call "<<ncalls<<std::endl;
		throw std::runtime_error("ttbarXsecFitter::toBeMinimized: at least one nan input from minuit");
	}
	double out=0;
	for(size_t ndataset=0;ndataset<datasets_.size();ndataset++){
		dataset * set=&datasets_.at(ndataset);

		for(size_t nbjet=0;nbjet<dataset::nBjetCat();nbjet++){
			if(exclude0bjetbin_){
				if(nbjet==0) continue;
			}
			//make sure its normalized
			//the influence usually is <0.001%, but still more correct that way
			double shapeintegral=set->signalshape(nbjet).getIntegral(variations);
			double bjetcat_norm= set->normalization(nbjet).getValue(variations);

			for(size_t bin=0;bin<set->signalshape(nbjet).getNBins();bin++){
				//std::cout << bin << "/"<< signalshape_nbjet_.at(nbjet).getNBins()<<" " << nbjet << " " << std::endl;
				double norm    = set->lumi() * (variations[set->xsecIdx()] +set->xsecOffset()) ;
				double signal=norm* set->signalshape(nbjet).getBin(bin)->getValue(variations) / shapeintegral;
				if(norm_nbjet_global_)
					signal*=bjetcat_norm;


				double nbackground = set->background(nbjet).getBin(bin)->getValue(variations);
				double backgroundstat=set->background(nbjet).getBinErr(bin);

				double data = set->data(nbjet).getBin(bin)->getValue(variations);
				//this is obsolete, strictly speaking, but safer in case somethin is normalized or weighted events are used
				double datastat = set->data(nbjet).getBinErr(bin);

				//this  might happen for some variations, fix it to physics values
				if(nbackground<0) nbackground=0;
				if(signal<0)signal=0;

				double predicted = signal+nbackground;

				//debug



				if(lhmode_ == lhm_poissondatastat)   {
					double roundpred=predicted;//format_.round(predicted,1);
					if(roundpred<0)roundpred=0;//safety
					if(data<0)continue; //safety
					out+=-2*logPoisson(data, roundpred); //unstable...
				}

				else if(lhmode_ ==lhm_chi2datastat)   {
					if(datastat<=0) continue;
					out+= (data-predicted)*(data-predicted) / (datastat*datastat); //chi2 approach
				}
				else if(lhmode_ ==lhm_chi2datamcstat)	{
					if((datastat*datastat + backgroundstat*backgroundstat)<=0) continue;
					out+= (data-predicted)*(data-predicted) / (datastat*datastat + backgroundstat*backgroundstat); //chi2 approach with bg errors
				}

				/* debug if a call fails			if(ncalls==284 && bin == 32){
				ZTOP_COUTVAR(bin);
				ZTOP_COUTVAR(nbjet);
				ZTOP_COUTVAR(bjetcat_norm);
				ZTOP_COUTVAR(data);
				ZTOP_COUTVAR(datastat);
				ZTOP_COUTVAR(nbackground);
				ZTOP_COUTVAR(backgroundstat);
				ZTOP_COUTVAR(signal);
				ZTOP_COUTVAR(predicted);
				ZTOP_COUTVAR(out);
				for(size_t i=0;i<parameternames_.size();i++)
					std::cout << parameternames_.at(i) << ":\t"<<variations[i]<<std::endl;
			}
				 */
			}
		}
	}
	if(out==std::numeric_limits<double>::infinity()){
		out=lastout*10; //this is only supposed to happen for extreme variations during fit process->check pulls
	}
	lastout=out;
	//constraints
	for(size_t sys=0;sys<ndependencies_;sys++){
		bool isxsec=false;
		for(size_t i=0;i<datasets_.size();i++)
			if(sys == datasets_.at(i).xsecIdx()){
				isxsec=true;
				break;
			}
		if(isxsec) continue;


		//put else here if lumi gets treated differently
		if(priors_[sys] == prior_gauss){
			out+= simpleFitter::nuisanceGaus(variations[sys]);}
		else if(priors_[sys] == prior_box){
			out+= simpleFitter::nuisanceBox(variations[sys]);
		}
		else if(priors_[sys] == prior_float){
			//don't do anything
		}
		else if(priors_[sys] == prior_narrowboxleft){
			double var=variations[sys];
			if(var>0) var+=1;
			out+= simpleFitter::nuisanceBox(var);
		}
		else if(priors_[sys] == prior_narrowboxright){
			double var=variations[sys];
			if(var<0) var-=1;
			out+= simpleFitter::nuisanceBox(var);
		}
		//the fitter will handle this case, delta should be zero in all cases
		//	else if(priors_[sys] == prior_parameterfixed){
		//		double deltafixed=variations[sys]-fitter_.getParameter(sys);
		//		out+=deltafixed*deltafixed*1e9; //make it a bit smooth
		//	}

	}
	if(out!=out){
		throw std::runtime_error("ttbarXsecFitter::toBeMinimized NAN");
	}
	if(out<DBL_EPSILON && out !=0){
		out=DBL_EPSILON;
		std::cout << "restricted out!" <<std::endl;
	}

	return out;
}
void ttbarXsecFitter::checkSizes()const{
	if(debug)
		std::cout << "ttbarXsecFitter::checkSizes" <<std::endl;
	for(size_t i=0;i<datasets_.size();i++)
		datasets_.at(i).checkSizes();
	if(debug)
		std::cout << "ttbarXsecFitter::checkSizes: done" <<std::endl;
}

//the whole function is obsolete once the class works as expected!
void ttbarXsecFitter::dataset::checkSizes()const{

	if(debug)
		std::cout << "ttbarXsecFitter::dataset::checkSizes: "<<name_ <<std::endl;
	if(signalshape_nbjet_.size() < 1){
		throw std::logic_error("ttbarXsecFitter::checkSizes: no input");
	}
	if(signalshape_nbjet_.size() != background_nbjet_.size())
		throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != background_nbjet_.size()");
	if(signalshape_nbjet_.size() != data_nbjet_.size())
		throw std::logic_error("ttbarXsecFitter::checkSizes: leptonacceptance_nbjet_.size() != data_nbjet.size()");


	for(size_t i=0;i<signalshape_nbjet_.size() ;i++){
		if(signalshape_nbjet_.at(i).getBins() != background_nbjet_.at(i).getBins())
			throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != background_nbjet_.at(i).getBins()");
		if(signalshape_nbjet_.at(i).getBins() != data_nbjet_.at(i).getBins())
			throw std::logic_error("leptonacceptance_nbjet_.at(i).getBins() != data_nbjet.at(i).getBins()");
	}
	if(signalshape_nbjet_.at(0).getNDependencies() != background_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
	if(signalshape_nbjet_.at(0).getNDependencies() != data_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");
	if(signalshape_nbjet_.at(0).getNDependencies() != normalization_nbjet_.at(0).getNDependencies())
		throw  std::logic_error("ttbarXsecFitter::checkSizes: ndep broken");

	if(debug)
		std::cout << "ttbarXsecFitter::dataset::checkSizes: "<<name_ << " done" <<std::endl;

}//ttbarXsecFitter::checkSizes()


double ttbarXsecFitter::getExtrapolationError( size_t datasetidx, size_t paraidx, bool up, bool fullps){
	/*std::vector<double> paracopy=fittedparas_;
		float nom=var->getValue(paracopy);
		if(i==0)
			std::cout << "---->> Epsilon emu (%): " << nom*100 << std::endl;
		paracopy.at(idx)=1;
		float up=var->getValue(paracopy);

		paracopy.at(idx)=-1;
		float down=var->getValue(para */
	if(paraidx >= parameternames_.size()){
		throw std::out_of_range("ttbarXsecFitter::getExtrapolationError: parameter index.");
	}
	double nominal=1,construp=1,constrdown=1,fullup=1,fulldown=1;
	extendedVariable * varanticorr=0, * varcorr=0;
	extendedVariable varone;
	varone.setToOne(true);
	varcorr=&varone;
	varanticorr=&datasets_.at(datasetidx).eps_emu();
	if(!fullps)
		varcorr=&datasets_.at(datasetidx).fullToVisCorrFact();

	std::vector<double> paracopy=fittedparas_;
	nominal=varcorr->getValue(paracopy) / varanticorr->getValue(paracopy);       ///<--------
	paracopy.at(paraidx) += fitter_.getParameterErrUp()->at(paraidx);
	if(paracopy.at(paraidx) >1)
		paracopy.at(paraidx)=1;
	else if(paracopy.at(paraidx)<-1)
		paracopy.at(paraidx)=-1;
	construp=varcorr->getValue(paracopy) / varanticorr->getValue(paracopy);          ///<--------
	paracopy=fittedparas_;  //fabs: make sure sign is right
	paracopy.at(paraidx) -= (fitter_.getParameterErrDown()->at(paraidx));
	if(paracopy.at(paraidx) >1)
		paracopy.at(paraidx)=1;
	else if(paracopy.at(paraidx)<-1)
		paracopy.at(paraidx)=-1;
	constrdown=varcorr->getValue(paracopy) / varanticorr->getValue(paracopy);             ///<--------
	paracopy.at(paraidx) = 1;
	fullup=varcorr->getValue(paracopy) / varanticorr->getValue(paracopy);          ///<--------
	paracopy.at(paraidx) = -1;
	fulldown=varcorr->getValue(paracopy) / varanticorr->getValue(paracopy);                ///<--------


	double xsecup  = ( fullup/nominal  ) - (construp/nominal);
	double xsecdown= (fulldown/nominal ) - (constrdown/nominal);

	if(up)
		return xsecup;
	else
		return xsecdown;

}

variateContainer1D ttbarXsecFitter::dataset::createLeptonJetAcceptance(const std::vector<container1D>& signals,
		const std::vector<container1D>& signalpsmig,
		const std::vector<container1D>& signalvisPSgen,
		size_t bjetcategory)
{
	// this one can use container1D operators!
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createLeptonJetAcceptance: "<< bjetcategory <<", "<< name_<<std::endl;

	if(signals.size()<1)
		throw std::logic_error("ttbarXsecFitter::createLeptonJetAcceptance  stack vector empty.");

	size_t bjetbin=bjetcategory;
	size_t minbjetbin=0;
	size_t maxbjetbin=3;


	float gennorm = 1/(lumi_*xsecoff_);

	//unused	size_t bjet0bin=minbjetbin;
	size_t bjet1bin=minbjetbin+1;
	size_t bjet2bin=minbjetbin+2;

	container1D signal=signals.at(bjetbin);//.getSignalContainer();
	//if(norm_nbjet_global)
	signal=signal.getIntegralBin();
	container1D signalintegral=signals.at(minbjetbin); //stack->at(minbjetbin).getSignalContainer();
	//if(norm_nbjet_global)
	signalintegral=signalintegral.getIntegralBin();
	for(size_t i=minbjetbin+1;i<maxbjetbin;i++){
		container1D tmpsig=signals.at(i); //stack->at(i).getSignalContainer();
		//if(norm_nbjet_global)
		tmpsig=tmpsig.getIntegralBin();
		signalintegral += tmpsig;
	}

	container1D psmigint;
	for(size_t i=minbjetbin;i<maxbjetbin;i++){
		container1D tmp=signalpsmig.at(i);
		tmp=tmp.getIntegralBin();
		psmigint+=tmp;
	}
	container1D visgenint;
	for(size_t i=minbjetbin;i<maxbjetbin;i++){
		container1D tmp=signalvisPSgen.at(i);
		tmp=tmp.getIntegralBin();
		visgenint+=tmp;
	}

	//need signal integral per bin -> same distributions for fixed add jet multi...
	bool tmpmultiplyStatCorrelated = histoContent::multiplyStatCorrelated;
	bool tmpaddStatCorrelated = histoContent::addStatCorrelated;
	bool tmpdivideStatCorrelated = histoContent::divideStatCorrelated;
	histoContent::multiplyStatCorrelated = true;
	histoContent::addStatCorrelated = true;
	histoContent::divideStatCorrelated = true;


	container1D one=signal.createOne();
	container1D AccBRRecodil = signalintegral * gennorm; //this includes lumi unc but cancels the xsec and lumi abs value!


	container1D onebjetsignal = signals.at(bjet1bin);//stack->at(bjet1bin).getSignalContainer();
	//if(norm_nbjet_global)
	onebjetsignal=onebjetsignal.getIntegralBin();
	container1D twobjetsignal = signals.at(bjet2bin);//stack->at(bjet2bin).getSignalContainer();
	//if(norm_nbjet_global)
	twobjetsignal=twobjetsignal.getIntegralBin();

	container1D correction_b =  ((signalintegral * twobjetsignal) * 4.)
																																																																						                																																																																																																																																																														/ ( (onebjetsignal + (twobjetsignal * 2.)) * (onebjetsignal + (twobjetsignal * 2.)));

	correction_b.removeStatFromAll();

	container_c_b_.import(correction_b);

	container1D eps_b = twobjetsignal / (correction_b * signalintegral);
	eps_b.sqrt();
	eps_b.removeStatFromAll();

	container_eps_b_.import(eps_b);

	container1D lepjetcont1bjet=     (eps_b * 2.  - (correction_b * (eps_b * eps_b)) * 2.);
	lepjetcont1bjet.removeStatFromAll();
	container1D lepjetcont2bjets=    (correction_b * eps_b) * eps_b ;
	lepjetcont2bjets.removeStatFromAll();
	container1D lepjetcont0bjet=   AccBRRecodil* (one - lepjetcont1bjet - lepjetcont2bjets);
	lepjetcont0bjet.removeStatFromAll();
	lepjetcont1bjet *= AccBRRecodil;
	lepjetcont1bjet.removeStatFromAll();
	lepjetcont2bjets*= AccBRRecodil;
	lepjetcont2bjets.removeStatFromAll();

	//epsemu (extrapolation)
	variateContainer1D tmp;
	tmp.import(AccBRRecodil);

	eps_emu_=*tmp.getBin(1);
	container1D branchingratiotovis= visgenint * (gennorm /((double)totalvisgencontsread_));
	// The generated part was filled per event for each of the containers
	//visible = full * BRtovis

	//	container1D fullDivVis =   ((signalintegral- psmigint)/visgenint) / AccBRRecodil  ;//gennorm

	///whats the best?

	tmp.import(branchingratiotovis);
	fulltoviscorrfact_=*tmp.getBin(1);

	//the stat uncertainties will be screwed up, but they are not used anyway
	bool tmpvarcdebug=variateContainer1D::debug;
	//	if(debug)
	//		variateContainer1D::debug=true;
	variateContainer1D out;
	//if(norm_nbjet_global){
	// disentangle shape and global norm here
	container1D normedsignal = signals.at(bjetbin);//stack->at(bjetbin).getSignalContainer(); //for shape
	normedsignal.normalize(true,true); //normalize to 1 incl syst
	out.import(normedsignal);


	if(normalization_nbjet_.size() <= bjetbin)
		normalization_nbjet_.resize(bjetbin+1);

	variateContainer1D tmpvarc;
	if(bjetcategory == 0){
		tmpvarc.import(lepjetcont0bjet);
	}
	else if(bjetcategory == 1){
		tmpvarc.import(lepjetcont1bjet);
	}
	else if(bjetcategory == 2){
		tmpvarc.import(lepjetcont2bjets);
	}
	normalization_nbjet_.at(bjetbin) = *tmpvarc.getBin(1);


	histoContent::multiplyStatCorrelated=tmpmultiplyStatCorrelated;
	histoContent::addStatCorrelated=tmpaddStatCorrelated;
	histoContent::divideStatCorrelated=tmpdivideStatCorrelated;

	variateContainer1D::debug=tmpvarcdebug;
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::createLeptonJetAcceptance: done " << std::endl;
	return out;
}



const size_t & ttbarXsecFitter::dataset::xsecIdx()const{
	if(xsecidx_==9999){
		throw std::logic_error("ttbarXsecFitter::datasets::xsecIdx: first create index");
	}
	return xsecidx_;
}
void ttbarXsecFitter::dataset::createXsecIdx(){
	//search for it
	if(signalconts_nbjets_.size()<1){
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: first read-in stacks");
	}
	std::vector<TString> sysnames=signalconts_nbjets_.at(0).getSystNameList();
	size_t idx=std::find(sysnames.begin(),sysnames.end(),"Xsec ("+name_+")")-sysnames.begin();
	if(idx == sysnames.size())
		throw std::logic_error("ttbarXsecFitter::datasets::createXsecIdx: index not found");
	xsecidx_=idx;
}

void  ttbarXsecFitter::dataset::readStacks(const std::string configfilename,const std::pair<TString,TString>&replaceininfiles,bool removesyst){
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::readStacks" <<std::endl;

	fileReader fr;
	fr.setComment("#");
	fr.setDelimiter("|");
	//8 TeV
	fr.setStartMarker(("[ "+name_+ " ]").Data());
	fr.setEndMarker(("[ end - "+name_+ " ]").Data());
	fr.readFile(configfilename);
	std::vector<std::vector<containerStack> >  out;

	std::cout << "reading and preparing " << name_ << " stacks..." <<std::endl;
	size_t bjetcount=0,jetcount=0,oldbjetcount=0;
	TString oldfilename="";
	containerStackVector csv;
	std::vector<containerStack> tmpstacks;
	for(size_t line=0;line<fr.nLines();line++){
		if(!debug)
			displayStatusBar(line,fr.nLines());
		if(fr.nEntries(line) < 1) continue;
		if(fr.getData<TString>(line,0) == "bjet++"){
			bjetcount++;
			jetcount=0;
			continue;
		}
		if(fr.nEntries(line) < 2 || ! fr.nEntries(line) % 2){
			std::cout << "ttbarXsecFitter::readInput: Line format wrong:\n" << fr.getReJoinedLine(line) << std::endl;
		}
		if(oldbjetcount!=bjetcount){
			out.push_back(tmpstacks);
			oldbjetcount=bjetcount;
			tmpstacks.clear();
		}
		for(size_t entry=0;entry<fr.nEntries(line);entry++){
			TString filename=fr.getData<TString>(line,entry);
			if(replaceininfiles.first.Length()>0)
				filename.ReplaceAll(replaceininfiles.first,replaceininfiles.second);
			entry++;
			TString plotname=fr.getData<TString>(line,entry);
			bool newcsv=oldfilename!=filename;
			oldfilename=filename;
			if(newcsv)
				csv.loadFromTFile(filename); //new file

			//csv.listStacks();
			containerStack tmpstack;
			try{
				tmpstack=csv.getStack(plotname);
			}
			catch(std::exception &ex){
				csv.listStacks();
				ex.what();
				throw std::runtime_error("stack not found");
			}
			if(!tmpstack.is1DUnfold())
				throw std::runtime_error("ttbarXsecFitter::readStacks: Stack is has no unfolding information!");
			if(tmpstack.getContribution1DUnfold("data").isDummy())
				throw std::runtime_error("ttbarXsecFitter::readStacks: Stack has no data entry!");
			if(tmpstack.getSignalIdxs().size() <1)
				throw std::runtime_error("ttbarXsecFitter::readStacks: No signal defined!");

			addUncertainties(&tmpstack,bjetcount,removesyst);
			/*	if(newcsv) //not necessary if same csv where everything is ordered in the same manner
				for(size_t i=0;i<out.size();i++){
					tmpstack.equalizeSystematicsIdxs(out.at(i));
				}
			 */
			tmpstacks.push_back(tmpstack);
			//stack=stack.append(tmpstack);
			//	std::cout << "ttbarXsecFitter::readStacks: added "
			//			<< " stack(s) for nbjets: " << bjetcount << " njets: " << jetcount <<std::endl;
		}
		jetcount++;
	}
	if(out.size()>0)
		out.push_back(tmpstacks);//last push back
	if(out.size()<1)
		std::cout << "ttbarXsecFitter::readStacks: no plots for " << name_ << " TeV added." <<std::endl;
	else if(out.size() != 3)
		throw std::runtime_error("ttbarXsecFitter::readStacks: tried to add less or more than (allowed) 3 nbjets categories");

	signalconts_nbjets_.resize(out.size());
	signalvisgenconts_nbjets_.resize(out.size());
	signalpsmigconts_nbjets_.resize(out.size());
	backgroundconts_nbjets_.resize(out.size());
	dataconts_nbjets_.resize(out.size());
	for(size_t nbjet=0;nbjet<out.size();nbjet++){
		container1D sign,signvisps,signpsmig,backgr,data;
		for(size_t j=0;j<out.at(nbjet).size();j++){
			sign          =sign.append(out.at(nbjet).at(j).getSignalContainer1DUnfold().getRecoContainer());
			signvisps     =signvisps.append(out.at(nbjet).at(j).getSignalContainer1DUnfold().getGenContainer());
			totalvisgencontsread_++;
			signpsmig=signpsmig.append(out.at(nbjet).at(j).getSignalContainer1DUnfold().getBackground());
			backgr      =backgr.append(out.at(nbjet).at(j).getBackgroundContainer());
			data          =data.append(out.at(nbjet).at(j).getDataContainer());
		}
		signalconts_nbjets_.at(nbjet) = sign;
		signalvisgenconts_nbjets_.at(nbjet)=signvisps;
		signalpsmigconts_nbjets_.at(nbjet)=signpsmig;
		backgroundconts_nbjets_.at(nbjet) = backgr;
		dataconts_nbjets_.at(nbjet)=data;
	}
	if(debug){
		std::cout << " ttbarXsecFitter::dataset::readStacks: done" <<std::endl;
	}

}

/*
 * Assumes consistent ordering within same dataset - is the case if input is read normally
 */
void ttbarXsecFitter::dataset::equalizeIndices(dataset & rhs){

	container1D::equalizeSystematics(signalconts_nbjets_,rhs.signalconts_nbjets_);
	container1D::equalizeSystematics(signalvisgenconts_nbjets_,rhs.signalvisgenconts_nbjets_);
	container1D::equalizeSystematics(signalpsmigconts_nbjets_,rhs.signalpsmigconts_nbjets_);
	container1D::equalizeSystematics(dataconts_nbjets_,rhs.dataconts_nbjets_);
	container1D::equalizeSystematics(backgroundconts_nbjets_,rhs.backgroundconts_nbjets_);


}

void ttbarXsecFitter::dataset::addUncertainties(containerStack * stack,size_t nbjets,bool removesyst)const{ //can be more sophisticated
	if(debug)
		std::cout << "ttbarXsecFitter::dataset::addUncertainties: " <<name_ << ", "<< nbjets <<std::endl;

	std::vector<TString> excludefromglobalvar;
	excludefromglobalvar.push_back("DY#rightarrow#tau#tau");
	excludefromglobalvar.push_back("DY#rightarrowll");
	//excludefromglobalvar.push_back("Z#rightarrowll");

	stack->mergeLegends(excludefromglobalvar,"DY",432,false);

	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: merged DY" <<std::endl;

	try{
		stack->mergeVariationsFromFileInCMSSW("/src/TtZAnalysis/Analysis/configs/fitTtBarXsec/combine_syst.txt");
	}
	catch(...){}
	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: merged variations" <<std::endl;

	stack->addRelErrorToBackgrounds(0.3,false,"BG","DY"); //more sophisticated approaches can be chosen here - like DY float etc


	float dy0bjetserr=0,dy1bjetserr=0,dy2bjetserr=0;
	if(nbjets==0)
		dy0bjetserr=0.3;
	else if(nbjets==1)
		dy1bjetserr=0.3;
	else if(nbjets==2)
		dy2bjetserr=0.3;

	stack->addRelErrorToContribution(dy0bjetserr,"DY","BG_0_bjets");
	stack->addRelErrorToContribution(dy1bjetserr,"DY","BG_1_bjets");
	stack->addRelErrorToContribution(dy2bjetserr,"DY","BG_2_bjets");


	float addlumiunc=0;
	addlumiunc=unclumi_/100;
	if(removesyst){
		addlumiunc=0;
		stack->removeAllSystematics();
	}

	stack->addGlobalRelMCError("Lumi ("+name_ +")",addlumiunc);

	///fake uncertainty
	stack->addGlobalRelMCError("Xsec ("+name_+")",0);
	if(debug)
		std::cout << "ttbarXsecFitter::addUncertainties: done" <<std::endl;



}

}
