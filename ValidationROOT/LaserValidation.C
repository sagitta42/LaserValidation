#include <iostream>
#include <fstream>
#include <string>
#include "TH1.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"
#include "TLegend.h"
#include "Laser.C"

/* 26.07.2018
 * LaserValidation procedure
 * Updates the historical file laser_history.csv with the info from the new laser run
 */

using namespace std;
void DisplayLaser();

const string histfname = "laser_history_c19.csv"; // test
//    histfname = "/home/production/laser_history.csv"; // official

// ------------------ main class for plotting ----------------- //

class Plots{
	public:
		Plots();
		~Plots(){};
		void FillHistos();
		void Style();
		void DrawHistos();
		void Range();

	private:
		TH1F* histo[6];
		TCanvas* c[3];
		TLegend* leg[2];

		// we want to remember the last run to set the axis limit
		int run;
		
};


// ------------------ main update function ----------------- //

void UpdateLaserRuns(int runnum){
	// open the historical file and check if this run already exists
	ifstream f(histfname.c_str());
	int run;
	string line;
	// get rid of the header
	getline(f, line);

	//check
	while(f >> run){
		if(run == runnum){
			cout << "Run number " << runnum << " is already present in the historical file." << endl;
			return;
		}
		// remove the rest of the line
		getline(f, line);
	}
	f.close();

	// if not present, update the file
	ofstream out(histfname.c_str(), fstream::app);

	// class defined in Laser.C with all the calculations
	Laser* las  = new Laser(runnum);    

	// general information about the run
	out << las->GetRun() << " "
		<< las->GetNLive() << " "
    << las->GetTT8() << " "
		<< las->GetNentries() << " "
		<< las->GetMaxY() << " "
		<< las->GetMaxX();
	
	// amplitude, mean and sigma from the Gaus fit
	double* par = las->GausFit();
  for(int i = 0; i < 3; i++){out << " " << par[i];}

	// amplitude, mean, left and right sigma from the asymmetrical Gaus fit
  par = las->SkewGausFit();
  for(int i = 0; i < 4; i++){out << " " << par[i];}

	// number of "bad channels" (that are split)
	int badcount = las->BadChannels();
  out << " " << badcount << endl;

	cout << "Updated the file with run " << runnum << endl;

	// show the plots
//	DisplayLaser();
	
}

// ------------------ display function ----------------- //

void DisplayLaser(){
	Plots* p = new Plots();
	p->FillHistos();
	p->Style();
	p->DrawHistos();
	p->Range();
}



// ------------------ constructor ----------------- //

Plots::Plots(){


	int r1 = 5000; int r2 = 60000; int rbins = r2 - r1;

	histo[0] = new TH1F("hgaus", "Gaus sigma", rbins, r1, r2);
	histo[1] = new TH1F("hskewLeft", "Skew Gaus left sigma", rbins, r1, r2);
	histo[2] = new TH1F("hskewRight", "Skew Gaus right sigma", rbins, r1, r2);
	histo[3] = new TH1F("hintTotal", "Intensity (total entries)", rbins, r1, r2);
	histo[4] = new TH1F("hintPeak", "Intensity (peak)", rbins, r1, r2);
	histo[5] = new TH1F("hbad", "Bad channels", rbins, r1, r2);
}

// ------------------ drawing ----------------- //

void Plots::FillHistos(){
	cout << "Reading info..." << endl;

	// read the file
	ifstream f(histfname.c_str());
	int nlive, tt8, nentries, badch;
	double maxX, maxY, gausAmp, gausMean, gausSigma, skewAmp, skewMean, skewLeft, skewRight;
	// get rid of the header
	string header; getline(f, header); 

	while (f >> run >> nlive >> tt8 >> nentries >> maxY >> maxX >> gausAmp >> gausMean >> gausSigma >> skewAmp >> skewMean >> skewLeft >> skewRight >> badch){
		histo[0]->Fill(run, gausSigma);
		histo[1]->Fill(run, skewLeft);
		histo[2]->Fill(run, skewRight);
		histo[3]->Fill(run, nentries*1. / nlive / tt8);
		histo[4]->Fill(run, gausAmp * gausSigma * TMath::Sqrt(2*TMath::Pi()) / nlive / tt8);
		histo[5]->Fill(run, badch);
	}

	f.close();
}

// ----------------------------------- //

void Plots::DrawHistos(){
	cout << "Plotting..." << endl;

	int width = 800; int height = 600;

	// canvas 1: sigmas
	c[0] = new TCanvas("C1", "Sigma", width, height);
	cout << c[0]->GetName() << ": " << c[0]->GetTitle() << endl;

	histo[2]->SetMarkerColor(kBlue); // right sigma first because it's the largest (to define the y axis limit)
	histo[2]->Draw("P");
	histo[0]->SetMarkerColor(kBlack);
	histo[0]->Draw("same, P");
	histo[1]->SetMarkerColor(kRed);
	histo[1]->Draw("same, P");

	leg[0]->Draw();
	c[0]->Modified(); c[0]->Update();

	// canvas 2: intensity
	c[1] = new TCanvas("C2", "Intensity", width, height);
	cout << c[1]->GetName() << ": " << c[1]->GetTitle() << endl;

	histo[3]->SetMarkerColor(kBlack);
	histo[3]->Draw("P");
	histo[4]->SetMarkerColor(kBlue);
	histo[4]->Draw("same, P");
	
	leg[1]->Draw();

	// canvas 3: bad channels
	c[2] = new TCanvas("C3", "Bad channels",width, height);
	cout << c[2]->GetName() << ": " << c[2]->GetTitle() << endl;
	
	histo[5]->Draw("P");

}

// ----------------------------------- //

void Plots::Style(){
	// styles
	for(int i = 0; i < 6; i++){
		histo[i]->SetMarkerStyle(20);
		histo[i]->SetMarkerSize(0.7);
		histo[i]->SetStats(0);
		histo[i]->SetXTitle("Run number");
	}

	// legends
	leg[0] = new TLegend(.15, .12, .45, .22);
	for(int i = 0; i < 3; i++){	leg[0]->AddEntry(histo[i], histo[i]->GetTitle());	}

	leg[1] = new TLegend(.15, .12, .45, .22);
	for(int i = 3; i < 5; i++){	leg[1]->AddEntry(histo[i], histo[i]->GetTitle());	}
	
	// labels and titles
	histo[2]->SetTitle("Peak sigma");
	histo[2]->SetYTitle("Time [ns]");
	histo[3]->SetTitle("Intensity");
	histo[3]->SetYTitle("Entries / # live PMTs / # TT8 events");
	histo[3]->SetTitleOffset(1.1, "Y");
	histo[5]->SetYTitle("Number of channels");

}

// ----------------------------------- //

void Plots::Range(){
	for(int i = 0; i < 6; i++){ histo[i]->GetXaxis()->SetRangeUser(24000., run+100.); }
	for(int i = 0; i < 3; i++){ c[i]->Modified(); c[i]->Update(); }
}


