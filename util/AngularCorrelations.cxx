#include "TFile.h"
#include "TH2.h"
#include "TH1.h"
#include "TGraphErrors.h"

#include "ArgParser.h"
#include "TUserSettings.h"
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
	double projGateLow = -1.;
	double projGateHigh = -1.;
	double projBgLow = -1.;
	double projBgHigh = -1.;
	double timeRandomNorm = -1.;
	double peakPos = -1.;
	double peakLow = -1.;
	double peakHigh = -1.;
	std::string baseName = "AngularCorrelation";
	std::string inputFile;
	std::string outputFile = "AngularCorrelations.root";
	std::string settingsFile;

	// set up the argument parser
	ArgParser parser;
	parser.option("h help ?", &help, true).description("Show this help message.");
	parser.option("projection-low proj-low", &projGateLow, true).description("Low edge of projection gate.");
	parser.option("projection-high proj-high", &projGateHigh, true).description("High edge of projection gate.");
	parser.option("background-low bg-low", &projBgLow, true).description("Low edge of background gate (for multiple gates use settings file).");
	parser.option("background-high bg-high", &projBgHigh, true).description("High edge of background gate (for multiple gates use settings file).");
	parser.option("time-random-normalization", &timeRandomNorm, true).description("Normalization factor for subtraction of time-random matrix. If negative (default) it will be calculated automatically.");
	parser.option("base-name", &baseName, true).description("Base name of matrices.").default_value("AngularCorrelation");
	parser.option("peak-pos peak", &peakPos, true).description("Peak position (for multiple peaks use settings file).");
	parser.option("peak-low", &peakLow, true).description("Low edge of peak fit.");
	parser.option("peak-high", &peakHigh, true).description("High edge of peak fit.");
	parser.option("settings", &settingsFile, true).description("Settings file with user settings, these do not overwrite anything provided on command line!");
	parser.option("input", &inputFile, true).description("Input file with gamma-gamma matrices for each angle (coincident, time-random, and event mixed).");
	parser.option("output", &outputFile, true).description("Name of output file, default is \"AngularCorrelations.root\".");

	parser.parse(argc, argv, true);

	if(help) {
		std::cout<<parser<<std::endl;
		return 1;
	}

	// open the input root file and read any settings stored there, then add the ones potentially read in from command line
	if(inputFile.empty()) {
		std::cerr<<"Need an input file!"<<std::endl;
		return 1;
	}

	TFile input(inputFile.c_str());

	if(!input.IsOpen()) {
		std::cerr<<"Failed to open input file "<<inputFile<<std::endl;
		return 1;
	}

	auto settings = static_cast<TUserSettings*>(input.Get("UserSettings"));

	// if we have a path for a settings file provided on command line, we either add it to the ones read
	// from the root file or (if there weren't any) create a new instance from it
	if(!settingsFile.empty()) {
		if(settings == nullptr) {
			settings = new TUserSettings(settingsFile);
		} else {
			settings->Read(settingsFile);
		}
	}

	// check that we got settings either from the root file or a path provided on command line
	if(settings == nullptr || settings->empty()) {
		std::cerr<<"Failed to get user settings from input file."<<std::endl;
		return 1;
	}

	// set all variables from settings if they haven't been set from command line
	if(projGateLow == -1.) projGateLow = settings->GetDouble("Projection.Low");
	if(projGateHigh == -1.) projGateHigh = settings->GetDouble("Projection.High");
	if(timeRandomNorm == -1.) timeRandomNorm = settings->GetDouble("TimeRandomNormalization");
	if(peakPos == -1.) peakPos = settings->GetDouble("Peak.Position");
	if(peakLow == -1.) peakLow = settings->GetDouble("Peak.Low");
	if(peakHigh == -1.) peakHigh = settings->GetDouble("Peak.High");
	// for the background-peak positions and background gates we could have multiple, so we create vectors for them
	std::vector<double> bgPeakPos;
	std::vector<double> bgLow;
	std::vector<double> bgHigh;
	// we only fill the background gates from the settings if there were none provided on the command line
	if(projBgLow == -1. || projBgHigh == -1. || projBgLow >= projBgHigh) {
		for(int i = 1;; ++i) {
			try {
				auto low = settings->GetDouble(Form("Background.%d.Low",i));
				auto high = settings->GetDouble(Form("Background.%d.High",i));
				if(low >= high) {
					std::cout<<i<<". background gate, low edge not lower than the high edge: "<<low<<" >= "<<high<<std::endl;
					break;
				}
				bgLow.push_back(low);
				bgHigh.push_back(high);
			} catch (std::out_of_range& e) {
				break;
			}
		}
	} else {
		bgLow.push_back(projBgLow);
		bgHigh.push_back(projBgHigh);
	}
	// background-peak positions can only be set via settings file right now
	// we loop until we fail to find an entry
	for(int i = 1;; ++i) {
		try {
			auto pos = settings->GetDouble(Form("Peak.Position.%d",i));
			if(pos <= peakLow || pos >= peakHigh) {
				std::cout<<i<<". background peak outside of fit range: "<<pos<<" <= "<<peakLow<<" or "<<pos<<" >= "<<peakHigh<<std::endl;
				break;
			}
			bgPeakPos.push_back(pos);
		} catch (std::out_of_range& e) {
			break;
		}
	}

	// check if all necessary settings have been provided
	if(projGateLow >= projGateHigh) {
		std::cerr<<"Need a projection gate with a low edge that is smaller than the high edge, "<<projGateLow<<" >= "<<projGateHigh<<std::endl;
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

	// for the background gates we checked that the low edge is below the high edge before adding them to the vector
	// so we only need to check that the vectors aren't empty (sizes should always be the same, but we check anyway)
	if(bgLow.empty() || bgLow.size() != bgHigh.size()) {
		std::cerr<<"Background gate information missing, either no low/high edges or a mismatching amount of low and high edges: "<<bgLow.size()<<" low edges, and "<<bgHigh.size()<<" high edges"<<std::endl;
		return 1;
	}

	// get the angles from the input file
	auto angles = static_cast<TGriffinAngles*>(input.Get("GriffinAngles"));

	if(angles == nullptr) {
		std::cerr<<"Failed to find 'GriffinAngles' in '"<<inputFile<<"'"<<std::endl;
		return 1;
	}

	// open output file and create graphs
	TFile output(outputFile.c_str(), "recreate");

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

		// project onto x-axis
		auto proj =     prompt->ProjectionX(Form("proj%d",i),     prompt->GetYaxis()->FindBin(projGateLow), prompt->GetYaxis()->FindBin(projGateHigh));
		auto projMixed = mixed->ProjectionX(Form("projMixed%d",i), mixed->GetYaxis()->FindBin(projGateLow),  mixed->GetYaxis()->FindBin(projGateHigh));
		// project background gate(s) onto x-axis
		auto projBg = prompt->ProjectionX(Form("projBg%d",i), prompt->GetYaxis()->FindBin(bgLow[0]), prompt->GetYaxis()->FindBin(bgHigh[0]));
		auto projMixedBg = mixed->ProjectionX(Form("projMixedBg%d",i), mixed->GetYaxis()->FindBin(bgLow[0]), mixed->GetYaxis()->FindBin(bgHigh[0]));
		double bgGateWidth =     prompt->GetYaxis()->FindBin(bgHigh[0]) - prompt->GetYaxis()->FindBin(bgLow[0])+1;
		double mixedBgGateWidth = mixed->GetYaxis()->FindBin(bgHigh[0]) -  mixed->GetYaxis()->FindBin(bgLow[0])+1;
		for(size_t g = 1; g < bgLow.size(); ++g) {
			projBg->Add(prompt->ProjectionX(Form("projBg%d",i), prompt->GetYaxis()->FindBin(bgLow[g]), prompt->GetYaxis()->FindBin(bgHigh[g])));
			projMixedBg->Add(mixed->ProjectionX(Form("projMixedBg%d",i), mixed->GetYaxis()->FindBin(bgLow[g]), mixed->GetYaxis()->FindBin(bgHigh[g])));
			bgGateWidth +=     prompt->GetYaxis()->FindBin(bgHigh[g]) - prompt->GetYaxis()->FindBin(bgLow[g])+1;
			mixedBgGateWidth += mixed->GetYaxis()->FindBin(bgHigh[g]) -  mixed->GetYaxis()->FindBin(bgLow[g])+1;
		}

		// subtract background gate (+1 because the projection includes first and last bin)
		proj->Add(projBg, -(prompt->GetYaxis()->FindBin(projGateHigh)-prompt->GetYaxis()->FindBin(projGateLow)+1)/bgGateWidth);
		projMixed->Add(projMixedBg, -(mixed->GetYaxis()->FindBin(projGateHigh)-mixed->GetYaxis()->FindBin(projGateLow)+1)/mixedBgGateWidth);

		// fit the projections, we create separate peak fitters and peaks for the prompt and mixed histograms
		TPeakFitter pf(peakLow, peakHigh);
		TRWPeak peak(peakPos);
		pf.AddPeak(&peak);
		for(auto bgPeak : bgPeakPos) {
			pf.AddPeak(new TRWPeak(bgPeak));
		}
		pf.Fit(proj);

		TPeakFitter pfMixed(peakLow, peakHigh);
		TRWPeak peakMixed(peakPos);
		pfMixed.AddPeak(&peakMixed);
		for(auto bgPeak : bgPeakPos) {
			pfMixed.AddPeak(new TRWPeak(bgPeak));
		}
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
