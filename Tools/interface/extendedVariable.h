/*
 * extendedVariable.h
 *
 *  Created on: May 23, 2014
 *      Author: kiesej
 */

#ifndef EXTENDEDVARIABLE_H_
#define EXTENDEDVARIABLE_H_


#include "TString.h"
#include "graphFitter.h"
#include "graph.h"


namespace ztop{
/**
 * This is a helper class to handle variables that depend on systematics
 *
 */
class extendedVariable{
public:
    explicit extendedVariable():nominal_(-100000),name_(""),operatedon_(0),operation_(op_plus),constant_(false),constval_(0),extrapolatelinear_(true){}
    extendedVariable(const TString & name):nominal_(-100000),name_(name),operatedon_(0),
    		operation_(op_plus),constant_(false),constval_(0),extrapolatelinear_(true){}
    ~extendedVariable(){
    	clear();
    }
    extendedVariable & operator =(const extendedVariable&);
    extendedVariable(const extendedVariable&);


    void setName(const TString & name){name_=name;}

    /**
     * Use to create a dummy that always returns one. it can be handy in some cases
     */
    void setToOne(bool set){constant_=set;constval_=1.;}

    void setConstant(double val){constant_=true;constval_=val;}

    void resetConstant(){constant_=false;}



    /**
     * In case of pseudo-variations, e.g. down=nominal, nominal=nominal, up=real variation
     * set extrapolation mode such that a "down" variation gets added
     * If set to false, the dependence will be constant towards "down"
     * (The same applies analogously to the "up" variation if that one is
     * a pseudo variation
     * Default is true
     */
    void setExtrapolateLinearly(bool set ){extrapolatelinear_=set;}

    /**
     * just put (non shifted) graph here
     * also adds nominal
     * performs 2nd order fit and makes
     */
    void addDependence(const graph & , size_t nompoint, const TString& sysname);
    /**
     * add dependence
     * performs 2nd order "fit"
     * errors will be arbitrary
     *
     * will have scaling in terms of sigma (low = -sigma, high=sigma), sigma=1
     *
     */
    graph addDependence(const float & low, const float& nominal, const float& high , const TString& sysname);


    size_t getNDependencies()const{return dependences_.size();}

    double getValue(const double * variations)const;
    double getValue(const float * variations)const;

    double getValue(const std::vector<float> & variations)const;
    double getValue(const std::vector<float> * variations)const;
    double getValue(const std::vector<double> & variations)const;
    double getValue(const std::vector<double> * variations)const;


    double getValue(size_t idx,float variation)const;

    const double & getNominal()const{return nominal_;}

    const std::vector<TString>& getSystNames()const{return sysnames_;}

    void clear();


    extendedVariable& operator *= (const extendedVariable&);
    extendedVariable operator * (const extendedVariable&)const;
    extendedVariable& operator /= (const extendedVariable&);
    extendedVariable operator / (const extendedVariable&)const;
    extendedVariable& operator += (const extendedVariable&);
    extendedVariable operator + (const extendedVariable&)const;
    extendedVariable& operator -= (const extendedVariable&);
    extendedVariable operator - (const extendedVariable&)const;


    extendedVariable& operator *= (const double&);
    extendedVariable operator * (const double&)const;
    extendedVariable& operator /= (const double&);
    extendedVariable operator / (const double&)const;

    /**
     * if operations were performed on the variable, this function
     * returns the total multiplication factor for a given set of variations
     * used for variateHisto1D
     */
    double getMultiplicationFactor(const double * variations)const;
    double getMultiplicationFactor(const std::vector<double> * variations)const;
    double getMultiplicationFactor(const std::vector<double> &variations)const;

    static bool debug;
    static bool debugoperations;
private:

    enum operators{op_plus,op_minus,op_multi,op_divide};

    double nominal_;
    TString name_;
    std::vector<graphFitter>  dependences_; //one for each syst


    std::vector<extendedVariable *> operatedon_;
    std::vector<operators> operation_;
    double addOperations( double,const float * )const;
    double addOperations( double,const double * )const;

    double getMultiplicationFactorRec(const double * variations)const;
    //only adds factors,
    double addMultiFactors( double,const double * )const;
//return max dependence depth
    size_t checkDepth()const;

    std::vector<TString> sysnames_; //only for later reference

    bool constant_;
    double constval_;

    bool extrapolatelinear_;

    void copyFrom(const extendedVariable&) ;

   // extendedVariable* getLast();
    void slim();
    void checkDependencies(const extendedVariable&);
    void insertOperations(const extendedVariable&) ;
};

}
#endif /* EXTENDEDVARIABLE_H_ */
