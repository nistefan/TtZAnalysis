/*
 * plotterControlPlot.h
 *
 *  Created on: Jan 7, 2014
 *      Author: kiesej
 */

#ifndef PLOTTERCONTROLPLOT_H_
#define PLOTTERCONTROLPLOT_H_

#include "plotterBase.h"

class TLegend;
namespace ztop{
class plot;

class containerStack;

/**
 * plotter for control plots. stack+ratio plot
 * here also canvas options, text boxes, legends are being defined
 */
class plotterControlPlot: public plotterBase{
public:

    plotterControlPlot(): plotterBase(), divideat_(0), stackp_(0),templegp_(0),tempdataentry_(0),invertplots_(false){}
    ~plotterControlPlot(){plotterControlPlot::cleanMem();}

    void readStyleFromFile(const std::string& );
    /*
     * expects entries:
     * [plotterControlPlot] defines divideat
     * [textBoxes - boxes]
     * [containerStyle - DataUpper]
     * [containerStyle - MCUpper]
     * [containerStyle - DataRatio]
     * [containerStyle - MCRatio]
     * [plotStyle - Upper]
     * [plotStyle - Ratio]
     */

    void setStack(const containerStack *s){stackp_=s;}

    void clear();
    void clearPlots();
    void cleanMem();

protected:
    void preparePad();
    void drawPlots();
    // void drawTextBoxes();
    void drawLegends();


private:
    plotStyle upperstyle_;
    plotStyle ratiostyle_;
    containerStyle datastyleupper_;
    containerStyle datastyleratio_;
    containerStyle mcstyleupper_;
    containerStyle mcstyleratio_;



    float divideat_;

    const containerStack * stackp_;
//taken care of by addObject
    TLegend * templegp_;
    size_t tempdataentry_;
//must be taken care of
    std::vector<plot *> tempplots_;

    bool invertplots_;

    void drawControlPlot();
    void drawRatioPlot();

};


}


#endif /* PLOTTERCONTROLPLOT_H_ */