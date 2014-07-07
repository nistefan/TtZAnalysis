/*
 * containerStyle.h
 *
 *  Created on: Sep 30, 2013
 *      Author: kiesej
 */

#ifndef CANVASSTYLE_H_
#define CANVASSTYLE_H_
/**
 * for slow migration to histoplotter class using these styles
 * in general set style form template like "ratiodata, ratioMC, controlplotdata, controlplotmc..." and then adapt
 */
#include <vector>
#include "TString.h"
#include <iostream>
#include "tObjectList.h"

class TVirtualPad;
class TH1;
class TGraphAsymmErrors;

namespace ztop{
class containerStyle;
class plot;
//helper classes
class axisStyle{
    friend class containerStyle;

public:
    axisStyle();
    ~axisStyle();
    axisStyle(float titlesize,float labelsize,float titleoffset, float labeloffset, float ticklength,int ndivisions,float Max, float Min):
        titleSize(titlesize),labelSize(labelsize),titleOffset(titleoffset),labelOffset(labeloffset),tickLength(ticklength),ndiv(ndivisions),max(Max),min(Min),log(false),forcezero(false),name("USEDEF"){}

    bool applyAxisRange() const; //if min>max false (default)

    float titleSize;
    float labelSize;
    float titleOffset;
    float labelOffset;
    float tickLength;
    int ndiv;

    float max,min; //if min > max, don't apply
    bool log;

    bool forcezero;

    TString name;

};
class containerStyle;
class textBox{
    friend class containerStyle;
public:
    textBox();
    textBox(float,float,const TString &,float textsize=0.05,int font=62,int align=11);
    ~textBox();
    void setText(const TString &);
    void setTextSize(float);
    void setCoords(float,float);
    void setFont(int);
    void setAlign(int);
    const TString & getText() const;
    const float & getTextSize() const;
    const float & getX() const;
    const float &  getY() const;
    const int& getFont()const;
    const int& getAlign()const;

private:
    float x_,y_;
    TString text_;
    float textsize_;
    int align_,font_;

};

class textBoxes: tObjectList{
public:
    textBoxes(){}
    ~textBoxes(){}

    textBox & at(size_t idx){return boxes_.at(idx);}
    const textBox & at(size_t idx)const{return boxes_.at(idx);}

    void clear(){boxes_.clear();}
    void add(float x,float y,const TString & text,float textsize=0.05,int font=62,int align=11){
        boxes_.push_back(textBox(x,y,text,textsize,font,align));
    }
    size_t size(){return boxes_.size();}
    void readFromFileInCMSSW(const std::string & filename, const std::string & markername);
    void readFromFile(const std::string & filename, const std::string& markername);

    void drawToPad(TVirtualPad * p,bool NDC=true);

    static bool debug;
private:
    std::vector<textBox> boxes_;

};

class containerStyle{
public:


    containerStyle();
    ~containerStyle();
    containerStyle(float markersize, int markerstyle,int markercolor, float linesize, int linestyle, int linecolor,
            int fillstyle, int fillcolor):
                markerSize(markersize),markerStyle(markerstyle),markerColor(markercolor),lineSize(linesize),lineStyle(linestyle),lineColor(linecolor),
                fillStyle(fillstyle),fillColor(fillcolor),sysFillStyle(1001),sysFillColor(1){}


    void readFromFile(const std::string & filename, const std::string& stylename, bool requireall=true);

    void multiplySymbols(float val);

    void applyContainerStyle(TH1*,bool sys)const;
    void applyContainerStyle(TGraphAsymmErrors*,bool sys)const;
    void applyContainerStyle(plot *)const;

    float markerSize;
    int markerStyle;
    int markerColor;

    float lineSize;
    int lineStyle;
    int lineColor;

    int fillStyle;
    int fillColor;

    int sysFillStyle;
    int sysFillColor;

    TString drawStyle,rootDrawOpt,sysRootDrawOpt;

};



}//namespace

#endif /* CONTAINERSTYLE_H_ */
