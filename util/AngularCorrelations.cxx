#include "TFile.h"
#include "TH2.h"
#include "TH1.h"
#include "TGraphErrors.h"

#include "ArgParser.h"
#include "TGriffinAngles.h"
#include "TPeakFitter.h"
#include "TRWPeak.h"

/// program to read in 2D matrices from AngularCorrelationHelper
/// project and fit peaks to create angular correlation plots
/// and to create chi-square plots
int main(int argc, char** argv)
{
	// input parameters
	bool help = false;
	double projGateLow = 0.;
	double projGateHigh = -1;
	double projBgLow = 0.;
	double projBgHigh = -1.;
	double timeRandomNorm = -1.;
	std::string baseName = "AngularCorrelation";
	double peakPos = 0.;
	double peakLow = 0.;
	double peakHigh = -1.;
	std::string inputFile;

	ArgParser parser;
	parser.option("h help ?", &help, true).description("Show this help message.");
	parser.option("projection-low proj-low", &projGateLow, true).description("Low edge of projection gate.");
	parser.option("projection-high proj-high", &projGateHigh, true).description("High edge of projection gate.");
	parser.option("background-low bg-low", &projBgLow, true).description("Low edge of background gate.");
	parser.option("background-high bg-high", &projBgHigh, true).description("High edge of background gate.");
	parser.option("time-random-normalization", &timeRandomNorm, true).description("Normalization factor for subtraction of time-random matrix. If negative (default) it will be calculated automatically.");
	parser.option("base-name", &baseName, true).description("Base name of matrices.").default_value("AngularCorrelation");
	parser.option("peak-pos peak", &peakPos, true).description("Peak position.");
	parser.option("peak-low", &peakLow, true).description("Low edge of peak fit.");
	parser.option("peak-high", &peakHigh, true).description("High edge of peak fit.");
	parser.option("input", &inputFile, true).description("Input file with gamma-gamma matrices for each angle (coincident, time-random, and event mixed).");

	parser.parse(argc, argv, true);

	if(help) {
		std::cout<<parser<<std::endl;
		return 1;
	}

	// check if all necessary settings have been provided
	if(projGateLow >= projGateHigh) {
		std::cerr<<"Need a projection gate with a low edge that is smaller than the high edge, "<<projGateLow<<" >= "<<projGateHigh<<std::endl;
		return 1;
	}

	if(projBgLow >= projBgHigh) {
		std::cerr<<"Need a background gate with a low edge that is smaller than the high edge, "<<projBgLow<<" >= "<<projBgHigh<<std::endl;
		return 1;
	}

	if(peakLow >= peakHigh) {
		std::cerr<<"Need a fit range with a low edge that is smaller than the high edge, "<<peakLow<<" >= "<<peakHigh<<std::endl;
		return 1;
	}

	if(peakPos >= peakHigh || peakPos <= peakLow) {
		std::cerr<<"Need a peak within the fit range, "<<peakPos<<" not within "<<peakLow<<" - "<<peakHigh<<std::endl;
		return 1;
	}

	if(timeRandomNorm <= 0.) {
		std::cerr<<"Need a positive normalization factor for time random subtraction"<<std::endl;
		return 1;
	}

	if(inputFile.empty()) {
		std::cerr<<"Need an input file!"<<std::endl;
		return 1;
	}

	TFile input(inputFile.c_str());

	if(!input.IsOpen()) {
		std::cerr<<"Failed to open input file "<<inputFile<<std::endl;
		return 1;
	}

	auto angles = static_cast<TGriffinAngles*>(input.Get("GriffinAngles"));

	if(angles == nullptr) {
		std::cerr<<"Failed to find 'GriffinAngles' in '"<<inputFile<<"'"<<std::endl;
		return 1;
	}

	// open output file and create graphs
	// TODO: base the output file name off the input files name? i.e. take the run number(s) and subrun number(s) from the input string
	TFile output("AngularCorrelations.root", "recreate");

	auto rawAngularDistribution = new TGraphErrors(angles->NumberOfAngles());
	rawAngularDistribution->SetName("RawAngularDistribution");
	auto angularDistribution = new TGraphErrors(angles->NumberOfAngles());
	angularDistribution->SetName("AngularDistribution");
	auto mixedAngularDistribution = new TGraphErrors(angles->NumberOfAngles());
	mixedAngularDistribution->SetName("MixedAngularDistribution");

	// loop over all matrices
	for(int i = 0; i < angles->NumberOfAngles(); ++i) {
		// get the three histograms we need: prompt, time random, and event mixed
		auto prompt = static_cast<TH2*>(input.Get(Form("AngularCorrelation%d", i)));
		if(prompt == nullptr) {
			std::cerr<<"Failed to find histogram '"<<Form("AngularCorrelation%d", i)<<"', should have "<<angles->NumberOfAngles()<<" angles in total!"<<std::endl;
			return 1;
		}
		auto bg = static_cast<TH2*>(input.Get(Form("AngularCorrelationBG%d", i)));
		if(bg == nullptr) {
			std::cerr<<"Failed to find histogram '"<<Form("AngularCorrelationBG%d", i)<<"', should have "<<angles->NumberOfAngles()<<" angles in total!"<<std::endl;
			return 1;
		}
		auto mixed = static_cast<TH2*>(input.Get(Form("AngularCorrelationMixed%d", i)));
		if(mixed == nullptr) {
			std::cerr<<"Failed to find histogram '"<<Form("AngularCorrelationMixed%d", i)<<"', should have "<<angles->NumberOfAngles()<<" angles in total!"<<std::endl;
			return 1;
		}
		
		// first subtract time random background from prompt data and enable proper error propagation for prompt and mixed data
		prompt->Sumw2();
		prompt->Add(bg, -timeRandomNorm);
		mixed->Sumw2();

		// TODO: multiple background gates and peaks?
		// project onto x-axis
		auto proj = prompt->ProjectionX(Form("proj%d",i), prompt->GetYaxis()->FindBin(projGateLow), prompt->GetYaxis()->FindBin(projGateHigh));
		auto projMixed = mixed->ProjectionX(Form("projMixed%d",i), mixed->GetYaxis()->FindBin(projGateLow), mixed->GetYaxis()->FindBin(projGateHigh));
		// project background gate onto x-axis
		auto projBg = prompt->ProjectionX(Form("projBg%d",i), prompt->GetYaxis()->FindBin(projBgLow), prompt->GetYaxis()->FindBin(projBgHigh));
		auto projMixedBg = mixed->ProjectionX(Form("projMixedBg%d",i), mixed->GetYaxis()->FindBin(projBgLow), mixed->GetYaxis()->FindBin(projBgHigh));
		// subtract background gate (+1 because the projection includes first and last bin)
		proj->Add(projBg, -(prompt->GetYaxis()->FindBin(projGateHigh)-prompt->GetYaxis()->FindBin(projGateLow)+1)/(prompt->GetYaxis()->FindBin(projBgHigh)-prompt->GetYaxis()->FindBin(projBgLow)+1));
		projMixed->Add(projMixedBg, -(mixed->GetYaxis()->FindBin(projGateHigh)-mixed->GetYaxis()->FindBin(projGateLow)+1)/(mixed->GetYaxis()->FindBin(projBgHigh)-mixed->GetYaxis()->FindBin(projBgLow)+1));

		// fit the projections, we create separate peak fitters and peaks for the prompt and mixed histograms
		TPeakFitter pf(peakLow, peakHigh);
		TRWPeak peak(peakPos);
		pf.AddPeak(&peak);
		pf.Fit(proj);

		TPeakFitter pfMixed(peakLow, peakHigh);
		TRWPeak peakMixed(peakPos);
		pfMixed.AddPeak(&peakMixed);
		pfMixed.Fit(projMixed);

		// save the fitted histograms to the output file and the areas of the peaks
		proj->Write();
		projMixed->Write();

		// TODO: set an error for the angles?
		rawAngularDistribution->SetPoint(i, angles->AverageAngle(i), peak.Area());
		rawAngularDistribution->SetPointError(i, 0., peak.AreaErr());
		mixedAngularDistribution->SetPoint(i, angles->AverageAngle(i), peakMixed.Area());
		mixedAngularDistribution->SetPointError(i, 0., peakMixed.AreaErr());
		angularDistribution->SetPoint(i, angles->AverageAngle(i), peak.Area()/peakMixed.Area());
		angularDistribution->SetPointError(i, 0., peak.Area()/peakMixed.Area() * TMath::Sqrt(TMath::Power(peak.AreaErr()/peak.Area(), 2) + TMath::Power(peakMixed.AreaErr()/peakMixed.Area(), 2)));
	}

	rawAngularDistribution->Write();
	angularDistribution->Write();
	mixedAngularDistribution->Write();
	
	output.Close();
	input.Close();

	return 0;
}
