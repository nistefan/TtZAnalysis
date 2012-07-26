#!/bin/sh

# emu

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MuEG/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mueg_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MuEG/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mueg_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MuEG/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mueg_runC_prompt_cff.py --no-check-certificate


#double e

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleElectron/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O ee_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleElectron/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O ee_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleElectron/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O ee_runC_prompt_cff.py --no-check-certificate

#double mu

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleMu/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mumu_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleMu/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mumu_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/DoubleMu/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O mumu_runC_prompt_cff.py --no-check-certificate



#met

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MET/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O met_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MET/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O met_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/MET/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O met_runC_prompt_cff.py --no-check-certificate


#single e

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleElectron/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlee_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleElectron/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlee_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleElectron/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlee_runC_prompt_cff.py --no-check-certificate


#single mu

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleMu/Run2012A-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlemu_runA_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleMu/Run2012B-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlemu_runB_prompt_cff.py --no-check-certificate

wget 'https://cmsweb.cern.ch/das/makepy?dataset=/SingleMu/Run2012C-PromptReco-v1/AOD&instance=cms_dbs_prod_global' -O singlemu_runC_prompt_cff.py --no-check-certificate
