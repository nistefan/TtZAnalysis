/*
 * plotter2D.cc
 *
 *  Created on: Oct 27, 2014
 *      Author: kiesej
 */



#include "../interface/plotter2D.h"
#include "TColor.h"
#include "../interface/fileReader.h"

namespace ztop{


plotter2D::plotter2D(): plotterBase(),dividebybinarea_(false),zaxismin_(1),zaxismax_(-1){
	// not here, defaults set by constr : readStyleFromFileInCMSSW("/src/TtZAnalysis/Tools/styles/plot2D_default.txt");
}
plotter2D::~plotter2D(){}



void plotter2D::setPlot(const histo2D* c, bool dividebybinarea){
	plot_=*c;
	if(dividebybinarea)
		dividebybinarea_=true;
	else
		dividebybinarea_=false;
}
void plotter2D::readStylePriv(const std::string&file, bool requireall){

	fileReader fr;
	fr.setComment("$");
	fr.readFile(file);
	fr.setRequireValues(requireall);

	zaxismin_ = fr.getValue<float>("zaxis.min",zaxismin_);//,zaxismin_);
	zaxismax_ = fr.getValue<float>("zaxis.max",zaxismax_);//,zaxismax_);


	rootDrawOpt_ = fr.getValue<TString>("rootDrawOpt",rootDrawOpt_);

	palette_= fr.getValue<TString>("palette",palette_);
}

void plotter2D::preparePad(){
	TVirtualPad* c=getPad();
	c->Clear();
	c->SetLogy(0);
	c->SetBottomMargin(0.15);
	c->SetLeftMargin(0.15);
	c->SetRightMargin(0.2);
}
void plotter2D::drawPlots(){

	if(plot_.isDummy())
		throw std::logic_error("plotter2D::draw: No plot defined");

	const Int_t NRGBs = 5;
	const Int_t NCont = 255;
	if(palette_=="bluetored"){

		Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
		Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
		Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
		Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };

		TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
		gStyle->SetNumberContours(NCont);
	}
	if(palette_=="bluewhitered"){

		Double_t stops[NRGBs] = { 0.00, 0.34, 0.5, 0.84, 1.00 };
		Double_t red[NRGBs]   = { 0.00, 0.00, 1., 1.00, 0.51 };
		Double_t green[NRGBs] = { 0.00, 0.81, 1., 0.20, 0.00 };
		Double_t blue[NRGBs]  = { 0.51, 1.00, 1., 0.00, 0.00 };

		TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
		gStyle->SetNumberContours(NCont);
	}
	else{
		Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
		Double_t red[NRGBs]   = { 1, 0.00, 0.87, 1.00, 0.51 };
		Double_t green[NRGBs] = { 1, 0.81, 1.00, 0.20, 0.00 };
		Double_t blue[NRGBs]  = { 1, 1.00, 0.12, 0.00, 0.00 };

		TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
		gStyle->SetNumberContours(NCont);
	}
	//applt style
	// STC!!!



	TH2D* h=addObject(plot_.getTH2D(plot_.getName(),dividebybinarea_,false));


	h->GetZaxis()->SetTitleSize(0.07);

	h->GetZaxis()->SetLabelSize(0.06);
	h->GetYaxis()->SetTitleSize(0.07);
	h->GetYaxis()->SetLabelSize(0.06);
	h->GetXaxis()->SetTitleSize(0.07);
	h->GetXaxis()->SetLabelSize(0.06);
	h->GetXaxis()->SetTitleOffset(0.95);
	float max=plot_.getMax();
	float min=plot_.getMin();
	max=floor(max)+1;
	min=floor(min)-1;

	if(zaxismin_ > zaxismax_)
		h->GetZaxis()->SetRangeUser(min,max);
	else
		h->GetZaxis()->SetRangeUser(zaxismin_,zaxismax_);

	/*
	h->GetZaxis()->SetTitle(plot_.getZAxisName()); */
	h->Draw("colz");
	gPad->RedrawAxis();

}
// void drawTextBoxes();
void plotter2D::drawLegends(){

}




}//ns



