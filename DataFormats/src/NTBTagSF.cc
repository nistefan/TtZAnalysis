#include "../interface/NTBTagSF.h"
#include "TFile.h"
#include "TTree.h"


namespace ztop{


float NTBTagSF::getNTEventWeight(const std::vector<ztop::NTJet *> & jets){
    if(!isMC_ || userandom_|| makesEff()) return 1.;
    resetCounter();
    for(size_t i=0;i<jets.size();i++){
        ztop::NTJet *jet=jets.at(i);
        countJet(jet->pt(),fabs(jet->eta()),jet->genPartonFlavour());
    }
    return getEventSF();
}

/**
 * changes the discriminator values of the jets using random technique.
 * After this step do not expect the discriminator distrubtions in data MC to match!
 */

void NTBTagSF::changeNTJetTags( std::vector<ztop::NTJet *> *jets)const{
    if(!isMC_ || !userandom_ || makesEff()) return;
    for(size_t i=0;i<jets->size();i++){
        ztop::NTJet * jet=jets->at(i);
        if(std::abs(jet->genPartonFlavour()) == 0) continue;
        bool shouldbetagged=false;
        shouldbetagged= jetIsTagged(jet->pt(),std::abs(jet->eta()),jet->genPartonFlavour(),jet->btag(),
                std::abs<int>(1.e6 * sin(jet->phi())));

        // throw std::logic_error(" NTBTagSF::changeNTJetTags wrong type (undefined");

        if(!shouldbetagged){
            if(jet->btag()>getWPDiscrValue())
                jet->setBtag(-0.1);
        }
        else if(jet->btag()<getWPDiscrValue()){
            jet->setBtag(1.1);
        }
    }
}
void NTBTagSF::changeNTJetTags( std::vector<ztop::NTJet *> &jets)const{
    NTBTagSF::changeNTJetTags(&jets);
}


void NTBTagSF::setWorkingPoint(const TString& wpstring){
    std::cout << "NTBTagSF::setWorkingPoint: " << wpstring << std::endl;
    if(wpstring=="csvl"){
        bTagBase::setWorkingPoint(workingPoints::csvl_wp);}
    else if(wpstring=="csvm"){
        bTagBase::setWorkingPoint(workingPoints::csvm_wp);}
    else if(wpstring=="csvt"){
        bTagBase::setWorkingPoint(workingPoints::csvt_wp);}
    else{
        throw std::logic_error(("NTBTagSF::setWorkingPoint: doesn't exist"));
    }
}


void  NTBTagSF::setSystematic(const TString &sys=""){
    std::cout << "NTBTagSF::setSystematic: " <<sys<<std::endl;
    if(sys=="heavy up")
        bTagBase::setSystematic(bTagBase::heavyup);
    else if(sys=="heavy down")
        bTagBase::setSystematic(bTagBase::heavydown);
    else if(sys=="light up")
        bTagBase::setSystematic(bTagBase::lightup);
    else if(sys=="light down")
        bTagBase::setSystematic(bTagBase::lightdown);
    else if(sys=="nominal"||sys=="no"||sys=="")
        bTagBase::setSystematic(bTagBase::nominal);
    else
        throw std::logic_error(("NTBTagSF::setSystematic doesn't exist"));

}


NTBTagSF  NTBTagSF::operator + (NTBTagSF  second){
    std::map<std::string,std::vector<TH2D> >::iterator sampleit,sampleit2;//=histos_.find(samplename);
    for(sampleit=histos_.begin();sampleit!=histos_.end();++sampleit){
        std::string samplename1=sampleit->first;
        for(sampleit2=second.histos_.begin();sampleit2!=second.histos_.end();++sampleit2){
            std::string samplename2=sampleit2->first;
            if(samplename1 == samplename2){
                std::cout << "NTBTagSF::operator +: adding efficiencies with same samplename not supported! \ncheck unique naming\nwill just use old one" << std::endl;
                return *this;
            }
        }
    }
    //none of the samples are the same
    second.histos_.insert(histos_.begin(),histos_.end()); // samplename vec<hist>
    second.effhistos_.insert(effhistos_.begin(),effhistos_.end()); // samplename vec<hist>
    second.medianMap_.insert(medianMap_.begin(),medianMap_.end()); // median map
    return second;
}



/**
 * recreates file!
 */
void NTBTagSF::writeToTFile(TString filename){
    AutoLibraryLoader::enable();
    TH1::AddDirectory(false);
    TFile * f = new TFile(filename,"RECREATE");
    bool madenew=false;
    TTree * t=0;
    if(f->Get("NTBTagSFs")){
        t = (TTree*) f->Get("NTBTagSFs");
    }
    else{
        madenew=true;
        t = new TTree("NTBTagSFs","NTBTagSFs");
    }
    if(t->GetBranch("NTBTagSFs")){ //branch does  exist
        NTBTagSF * bt=this;
        t->SetBranchAddress("NTBTagSFs", &bt);
    }
    else{
        t->Branch("NTBTagSFs",this);
        std::cout << "NTBTagSF::writeToTFile: added branch" << std::endl;
    }
    setMakeEff(false);
    cleanptr();
    t->Fill();
    t->Write("",TObject::kOverwrite);
    if(madenew)
        delete t;
    f->Close();
    delete f;
}




void NTBTagSF::readFromTFile(TString filename){
    AutoLibraryLoader::enable();
    TH1::AddDirectory(false);
    TFile * f = new TFile(filename);
    TTree * t = (TTree*) f->Get("NTBTagSFs");
    NTBTagSF * bt=0;
    int branchret=t->SetBranchAddress("NTBTagSFs", &bt) ;
    if(branchret <0){
        std::cout << "NTBTagSF::readFromTFile: reading branch not successful (return " << branchret << ")" <<std::endl;
        throw std::runtime_error("NTBTagSF::readFromTFile: branch not found");
    }
    int entries=t->GetEntries();
    if(entries>1)
        std::cout << "NTBTagSF::readFromTFile: warning more than one NTBTagSF Class found. chose first entry" << std::endl;
    for(float n=0;n<t->GetEntries();n++){
        t->GetEntry(n);
        if(bt){
            *this=*bt;
            setMakeEff(false);
            break;
        }
    }
    f->Close();
    delete f;

}

}//namespace
