/*
 * wReweighterInterface.cc
 *
 *  Created on: 30 May 2016
 *      Author: jkiesele
 */

#include "../interface/wReweighterInterface.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"

namespace ztop{
wReweighterInterface::wReweighterInterface(
		tTreeHandler* t,
		bool enable,
		const TString& size_branch,
		const TString& weightsbranch):wNTBaseInterface(t,enable){

	if(size_branch.Length()){
		addBranch<int>(size_branch);
		addBranch<float*>(weightsbranch,128);
	}


	//create the default one


}
void wReweighterInterface::addWeightIndex(const size_t index){
	if(!enabled())
		return;
	indicies_.push_back(index);
	reweighter_.resize(indicies_.size());
	scalers_.resize(indicies_.size(),1);
}

void wReweighterInterface::reWeight(float & old){
	if(associatedBranches().size() <1 || !enabled())
		return ;
	const size_t brsize = *getBranchContent<int>(0);
	if(brsize> associatedBranches().at(1)->getBufMax()){
		std::string err="wReweighterInterface::reWeight: buffer too small to store all weights: ";
		err+= toString(brsize);
		err+="/";
		err+=toString(associatedBranches().at(1)->getBufMax());
		throw std::out_of_range(err);

	}
	for(size_t i=0;i<indicies_.size();i++){
		size_t idx=indicies_.at(i);
		if(idx>=brsize)
			throw std::out_of_range("wReweighterInterface::reWeight ");
		float weight=(* getBranchContent<float*>(1))[idx];
		if(scalers_.at(i)==1.)
			scalers_.at(i)=fabs(weight);
		weight/=scalers_.at(i);
		//makes the weight of order 1 on average, assuming the weights are approx the same
		//better numeric precision for the overall sum and reweighting factor.
		reweighter_.at(i).setNewWeight(weight);
		reweighter_.at(i).reWeight(old);
	}
}

double wReweighterInterface::getRenormalization()const{
	if(associatedBranches().size() <1 || !enabled())
		return 1.;
	double out=1;
	for(size_t i=0;i<reweighter_.size();i++)
		out*=reweighter_.at(i).getRenormalization();
	return out;
}

}
