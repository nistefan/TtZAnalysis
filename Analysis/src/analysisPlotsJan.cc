/*
 * analysisPlotsJan.cc
 *
 *  Created on: Feb 12, 2014
 *      Author: kiesej
 */


#include "../interface/analysisPlotsJan.h"
#include "../interface/AnalysisUtils.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/DataFormats/src/classes.h"

namespace ztop{

void analysisPlotsJan::bookPlots(){
    if(!use()) return;
    using namespace std;
    using namespace ztop;

    vector<float> genmlb_bins;
    genmlb_bins  << 0 << 50 << 90 << 120 << 140 << 160 << 230  << 350;
    vector<float> ivangen_mlbbins;
    ivangen_mlbbins << 0 << 70 << 116 << 150 <<400;
    vector<float> mlb_bins=ztop::subdivide<float>(genmlb_bins,5);
    vector<float> ivan_mlbbins=subdivide<float>(ivangen_mlbbins,5);

    Mlb=addPlot(genmlb_bins,mlb_bins,"m_lb unfold","M_{lb} [GeV]", "N/GeV");

    mlbcombthresh_=165;

    mlb=addPlot(genmlb_bins,mlb_bins,"m_lb pair comb unfold","M_{lb} [GeV]", "N/GeV");

    mlbmin=addPlot(genmlb_bins,mlb_bins,"m_lb pair min unfold","M_{lb} [GeV]", "N/GeV");

    mlbivansbins=addPlot(ivangen_mlbbins,ivan_mlbbins,"m_lb ivansbins unfold","M_{lb} [GeV]", "N/GeV");

    vector<float> inclbins; inclbins << 0.5 << 1.5; //vis PS, fullPS

    total=addPlot(inclbins,inclbins,"total","bin","N_{evt}");
    total->setBinByBin(true); //independent bins



}


void analysisPlotsJan::fillPlotsGen(){
    if(!use()) return;
    if(!event()) return;
    using namespace std;
    using namespace ztop;

    total->fillGen(1.,puweight());

    //calculate mlbs based on ME leptons
    if(event()->genvisleptons3 && event()->genvisbjetsfromtop){
        if(event()->genvisleptons3->size()>1 && event()->genvisbjetsfromtop->size()>0 ){
            NTLorentzVector<float> bjetp4=event()->genvisbjetsfromtop->at(0)->p4();
            NTLorentzVector<float> llepp4=event()->genvisleptons3->at(0)->p4();
            NTLorentzVector<float> slepp4=event()->genvisleptons3->at(1)->p4();

            float fMlb=(bjetp4+llepp4).m();
            float fm2lb=(bjetp4+slepp4).m();
            float fmlb=fMlb;
            float fmlbmin=fMlb;
            if(fMlb>mlbcombthresh_ && fm2lb<mlbcombthresh_)  fmlb=fm2lb;
            if(fMlb>fm2lb) fmlbmin=fm2lb;

            Mlb->fillGen(fMlb,puweight());
            mlb->fillGen(fmlb,puweight());
            mlbmin->fillGen(fmlbmin,puweight());

            mlbivansbins->fillGen(fmlb,puweight());
        }
    }


}

void analysisPlotsJan::fillPlotsReco(){
    if(!use()) return;
    if(!event()) return;
    using namespace std;
    using namespace ztop;

    total->fillReco(1,puweight());

    if(event()->leadinglep && event()->secleadinglep && event()->selectedbjets ){
        if(event()->selectedbjets->size()>0 ){
            NTLorentzVector<float> bjetp4=event()->selectedbjets->at(0)->p4();
            NTLorentzVector<float> llepp4=event()->leadinglep->p4();
            NTLorentzVector<float> slepp4=event()->secleadinglep->p4();

            float fMlb=(bjetp4+llepp4).m();
            float fm2lb=(bjetp4+slepp4).m();
            float fmlb=fMlb;
            float fmlbmin=fMlb;
            if(fMlb>mlbcombthresh_ && fm2lb<mlbcombthresh_)  fmlb=fm2lb;
            if(fMlb>fm2lb) fmlbmin=fm2lb;

            Mlb->fillReco(fMlb,puweight());
            mlb->fillReco(fmlb,puweight());
            mlbmin->fillReco(fmlbmin,puweight());

            mlbivansbins->fillReco(fmlb,puweight());
        }
    }


}

}//ns