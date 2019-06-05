#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TLeaf.h"
#include <string>
#include <iostream>

/*
 * 26.07.2018
 * Laser class that is used by HistoricalFile.C to create a historical file of all PMT laser calibration runs
 * Used by LaserValidation.C to update the historical file
 * Contains all the calculations done on the *laser_calibrations_validate_c18.root file
 */

double SkewGaus(double* x, double* params); // mathematical function defined in the bottom

class Laser{
	/* main class that gets all the info from the laser run that we're interested in */

	public:
		Laser(){};
		Laser(int runnum);
		~Laser();
		
		// getters
		int GetRun();
		int GetNLive();
		int GetTT8();
		int GetNentries();
		double GetMaxX();
		double GetMaxY();

		// calculations
		double* GausFit();
		double* SkewGausFit();
		int BadChannels();

	private:
		int run;
		char fname[200];
		TFile* f;
		TTree* t;
		TH2S* h;
		TH1D* p;
		int nentries;
		int maxbin;
		double max_x;
};


// --------------- constructor/destructor --------------- //

Laser::Laser(int runnum){
	run = runnum;
	// adding zeros if necessary
	std::string zeros;
	if(run <= 9999){zeros = "00";}
	else if(run <= 99999){zeros = "0";}
	else{zeros = "";}

	sprintf(fname, "/bxstorage/rootfiles/cycle_18/laser_run/Run%s%i_laser_calibrations_validate_c18.root", zeros.c_str(), run);
	
	f = TFile::Open(fname);
	f->GetObject("bxtree",t);
	h = (TH2S*)f->Get("barn/bx_calib_laben_time_align/bx_time_calib");
	p = h->ProjectionY();
	maxbin = p->GetMaximumBin();
	max_x = p->GetBinCenter(maxbin);
	nentries = (int) p->Integral();
//	nentries = (int) h->GetEntries(); // bug
}

// ------------------------------------------- //

Laser::~Laser(){
	delete t;
	delete h;
	delete p;
	delete f;
}

// --------------- getters --------------- //

int Laser::GetRun(){ return run; }

int Laser::GetNLive(){
	t->GetEntry(0);
	return (int) t->GetLeaf("laben.n_live_pmts")->GetValue(0);
}

int Laser::GetTT8(){ return t->GetEntries("trigger.trgtype == 8"); }
int Laser::GetNentries(){ return nentries; }
double Laser::GetMaxX(){ return max_x; }
double Laser::GetMaxY(){return p->GetBinContent(maxbin); }


// --------------- calculations --------------- //

double* Laser::GausFit(){
	/*  obtain amplitude, mean and sigma of the peak */

	TF1* fit = new TF1("g0", "gaus", 500., 1000.);
	int peak_entries =  (int) p->Integral(maxbin - 20, maxbin + 20);
	fit->SetParameters(peak_entries, max_x, 3.);
	fit->SetRange(max_x - 12., max_x + 12.);
	p->Fit("g0", "QRL0");
	double* par = new double[3];
	fit->GetParameters(par);
	return par;
}

// ------------------------------------------- //

double* Laser::SkewGausFit(){
	/* obtain amplitude, mean, left and right sigma of the peak
	 * uses function SkewGaus defined in the bottom
	 */

	p->SetAxisRange(max_x - 25, max_x + 25, "X");
	TF1* fit = new TF1("skew", SkewGaus, max_x - 25, max_x + 25, 4);
  double pmean = p->GetMean();
  double prms = p->GetRMS();
  fit->SetParameters(nentries, pmean, prms, prms);
  fit->SetParLimits(1, pmean - 3*prms, pmean + 3*prms);
  fit->SetParLimits(2, 0., 1000.);
  fit->SetParLimits(3, 0., 1000.);
  int fres = p->Fit("skew", "QRL0");
  double* par = new double[4];
  fit->GetParameters(par);

	if(fres != 0 || par[2] <= 0 || par[2] > 10 || par[3] > 10){
		std::cout << "!! WARNING: bad fit! zero returned!" << std::endl;
		for(int i = 0; i < 4; i++){par[i] = 0;}
	}

  return par;

}


// ------------------------------------------- //

int Laser::BadChannels(){
	/* return the number of "bad channels" in this run
	 * bad channels are the ones where the peak is split by a gap
	 * phenomenon found in September - November 2017 due to an issue with the clock
	 */

	// should always be 2240
  int nchannels = h->GetXaxis()->GetNbins();
  int badcount = 0;

  for(int ch = 1; ch < nchannels + 1; ch++){
		// projection for this channel
     char pname[5];
     sprintf(pname, "p%i", ch);
     TH1D* pch = h->ProjectionY(pname, ch, ch);

     // too few entries --> out
     if(pch->GetEntries() < 700){continue;}

     // if no clear peak --> out
     int pmaxbin = pch->GetMaximumBin();
     if( pch->Integral(pmaxbin - 12, pmaxbin + 12)/pch->GetEntries() < 0.2 ){continue;}

     double pmax = pch->GetMaximum();
     // floor: 3% of maximum. if drops below floor, it's either a normal end of the peak or a bad gap
     double floor = 0.03 * pmax;
     // ceiling: 8% of the peak.
     // if rises above ceiling after drops below floor, it's a split -> bad channel
 		 // note: a more complicated algorithm can be (and was) made checking different shapes of drops, but it's not necessary; this selection is precise enough, faster and more easily understandable
     double ceil = 0.08 * pmax;

     // flags to check for drops etc.
     bool peakstart = false;
     bool drop = false;
     bool bad = false;

     // only check in the viscinity of the peak to save time and ignore some fluctuations
		 // --> start from this bin:
     int bin =  pmaxbin - 25;

		 // stop checking if already found out that this channel is bad
     while(!bad && bin < pmaxbin + 25){
			 double y = pch->GetBinContent(bin);

       // if y > ceil for the first time, means we're simply entering the peak
       if(!peakstart && y > ceil){peakstart = true;}

			 // check if we dropped to floor after the peak started
       if(!drop && peakstart && y < floor){drop = true;}

       // if we dropped but didn't rise above ceil again, it's the end of the peak with normal fluctuations
			 // if we rose above ceil, it's a split --> bad channel
       if(drop && y > ceil){ bad = true; badcount++;}

       // otherwise keep going
       bin ++;
		 }
	}

  return badcount;

}

// --------------- helper function --------------- //

double SkewGaus(double* x, double* params){
	/* asymmetrical gaussian fit */

	double N = params[0];
	double mu = params[1];
	double sL = params[2];
	double sR = params[3];
	double sAvg = (sL+sR)/2;
	double sig;
	if(x[0] <= mu){sig = sL;} else{sig = sR;}
	return 2 * N / sqrt(2 * TMath::Pi()) / sAvg * TMath::Exp(-0.5 * pow((x[0]-mu)/sig, 2));
}

