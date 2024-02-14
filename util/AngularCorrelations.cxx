#include "TFile.h"
#include "TH2.h"
#include "TH1.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLegend.h"
#include "Fit/Fitter.h"
#include "TMatrixD.h"
#include "TPaveText.h"

#include "ArgParser.h"
#include "TUserSettings.h"
#include "TGriffinAngles.h"
#include "TPeakFitter.h"
#include "TRWPeak.h"

TGraph* MixingMethod(TGraphErrors* data, TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4, int twoJhigh, int twoJmid, int twoJlow, std::vector<double>& bestParameters);
std::vector<double> A2a4Method(TGraphErrors* data, TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4);

double GetYError(TGraphErrors* graph, const double& x)
{
	/// general function to get the error of a graph at point x (takes the maximum of the errors of the bracketing points)
	for(int i = 0; i < graph->GetN(); ++i) {
		// exact match: return the error at this point
		if(graph->GetPointX(i) == x) return graph->GetErrorY(i);
		// first point with larger x: take maximum of this point and previous point
		// TGraphErrors::GetErrorY returns -1 if index is negative, so we don't need to check for this
		else if(graph->GetPointX(i) > x) return std::max(graph->GetErrorY(i-1), graph->GetErrorY(i));
	}
	return 0.;
}

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
	std::string theoryFile;
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
	parser.option("theory", &theoryFile, true).description("File with simulated z0-, z2-, and z4-graphs.");
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
	auto rawChiSquares = new TGraph(angles->NumberOfAngles());
	rawChiSquares->SetName("RawChiSquares");
	auto mixedChiSquares = new TGraph(angles->NumberOfAngles());
	mixedChiSquares->SetName("MixedChiSquares");

	auto fitDir = output.mkdir("fits", "Projections with fits", true);

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
		pf.Fit(proj, "qretryfit");

		TPeakFitter pfMixed(peakLow, peakHigh);
		TRWPeak peakMixed(peakPos);
		pfMixed.AddPeak(&peakMixed);
		for(auto bgPeak : bgPeakPos) {
			pfMixed.AddPeak(new TRWPeak(bgPeak));
		}
		pfMixed.Fit(projMixed, "qretryfit");

		// save the fitted histograms to the output file and the areas of the peaks
		fitDir->cd();
		proj->Write();
		projMixed->Write();

		// TODO: set an error for the angles?
		rawAngularDistribution->SetPoint(i, angles->AverageAngle(i), peak.Area());
		rawAngularDistribution->SetPointError(i, 0., peak.AreaErr());
		mixedAngularDistribution->SetPoint(i, angles->AverageAngle(i), peakMixed.Area());
		mixedAngularDistribution->SetPointError(i, 0., peakMixed.AreaErr());
		angularDistribution->SetPoint(i, angles->AverageAngle(i), peak.Area()/peakMixed.Area());
		angularDistribution->SetPointError(i, 0., peak.Area()/peakMixed.Area() * TMath::Sqrt(TMath::Power(peak.AreaErr()/peak.Area(), 2) + TMath::Power(peakMixed.AreaErr()/peakMixed.Area(), 2)));

		rawChiSquares->SetPoint(i, angles->AverageAngle(i), peak.GetReducedChi2());
		mixedChiSquares->SetPoint(i, angles->AverageAngle(i), peakMixed.GetReducedChi2());

		std::cout<<std::setw(3)<<i<<" of "<<angles->NumberOfAngles()<<" done\r"<<std::flush;
	}
	std::cout<<"Fitting of projections done."<<std::endl;

	// check if we have a theory file
	if(!theoryFile.empty()) {
		TFile theory(theoryFile.c_str());

		if(theory.IsOpen()) {
			// read graphs from file
			auto z0 = static_cast<TGraphErrors*>(theory.Get("graph000"));
			auto z2 = static_cast<TGraphErrors*>(theory.Get("graph010"));
			auto z4 = static_cast<TGraphErrors*>(theory.Get("graph100"));

			if(z0 != nullptr && z2 != nullptr && z4 != nullptr) {
				// calculate chi2 vs mixing graphs
				std::vector<double> parametersSpin0;
				auto spin0 = MixingMethod(angularDistribution, z0, z2, z4, 0, 2, 0, parametersSpin0);
				std::vector<double> parametersSpin1;
				auto spin1 = MixingMethod(angularDistribution, z0, z2, z4, 2, 2, 0, parametersSpin1);
				std::vector<double> parametersSpin2;
				auto spin2 = MixingMethod(angularDistribution, z0, z2, z4, 4, 2, 0, parametersSpin2);
				std::vector<double> parametersSpin3;
				auto spin3 = MixingMethod(angularDistribution, z0, z2, z4, 6, 2, 0, parametersSpin3);
				std::vector<double> parametersSpin4;
				auto spin4 = MixingMethod(angularDistribution, z0, z2, z4, 8, 2, 0, parametersSpin4);

				// create canvas and plot graphs on it
				auto canvas = new TCanvas;

				// determine maximum y-value
				double max = TMath::Max(TMath::MaxElement(spin0->GetN(), spin0->GetY()), TMath::MaxElement(spin1->GetN(), spin1->GetY()));
				max = TMath::Max(max, TMath::MaxElement(spin2->GetN(), spin2->GetY()));
				max = TMath::Max(max, TMath::MaxElement(spin3->GetN(), spin3->GetY()));
				max = TMath::Max(max, TMath::MaxElement(spin4->GetN(), spin4->GetY()));

				spin1->SetTitle("");
				spin1->SetMinimum(0.);
				spin1->SetMaximum(1.1*max);

				spin1->SetLineColor(2); spin1->SetMarkerColor(2); spin1->SetLineWidth(2);
				spin2->SetLineColor(3); spin2->SetMarkerColor(3); spin2->SetLineWidth(2);
				spin3->SetLineColor(4); spin3->SetMarkerColor(4); spin3->SetLineWidth(2);
				spin4->SetLineColor(6); spin4->SetMarkerColor(6); spin4->SetLineWidth(2);

				spin1->Draw("ac");
				spin2->Draw("c");
				spin3->Draw("c");
				spin4->Draw("c");
				spin0->Draw("*");

				auto confidenceLevel = new TLine(-1.5, 1.535, 1.5, 1.535);

				confidenceLevel->Draw();

				auto legend = new TLegend(0.1, 0.3);
				legend->AddEntry(spin0, "J = 0", "p");
				legend->AddEntry(spin1, "J = 1", "l");
				legend->AddEntry(spin2, "J = 2", "l");
				legend->AddEntry(spin3, "J = 3", "l");
				legend->AddEntry(spin4, "J = 4", "l");

				legend->Draw();

				canvas->SetLogy();

				spin1->GetHistogram()->GetXaxis()->SetRangeUser(-1.5, 1.5);
				spin1->GetHistogram()->GetXaxis()->SetTitle("atan(#delta) [rad]");
				spin1->GetHistogram()->GetXaxis()->CenterTitle();
				spin1->GetHistogram()->GetYaxis()->SetTitle("red. #chi^2");
				spin1->GetHistogram()->GetYaxis()->CenterTitle();

				// write graphs and canvas to output file
				output.cd();
				z0->Write("graph000");
				z2->Write("graph010");
				z4->Write("graph100");
				spin0->Write("spin0");
				spin1->Write("spin1");
				spin2->Write("spin2");
				spin3->Write("spin3");
				spin4->Write("spin4");
				canvas->Write("MixingCanvas");

				// create theory graphs with best fit for each spin, and write them to file
				auto spin0Fit = new TGraphErrors(angularDistribution->GetN());
				auto spin1Fit = new TGraphErrors(angularDistribution->GetN());
				auto spin2Fit = new TGraphErrors(angularDistribution->GetN());
				auto spin3Fit = new TGraphErrors(angularDistribution->GetN());
				auto spin4Fit = new TGraphErrors(angularDistribution->GetN());
				auto x = angularDistribution->GetX();
				for(int p = 0; p < angularDistribution->GetN(); ++p) {
					spin0Fit->SetPoint(p, x[p], parametersSpin0[0]*((1.-parametersSpin0[1]-parametersSpin0[2])*z0->Eval(x[p]) + parametersSpin0[1]*z2->Eval(x[p]) + parametersSpin0[2]*z4->Eval(x[p])));
					spin0Fit->SetPointError(p, 0., TMath::Sqrt(parametersSpin0[0]*(TMath::Power((1.-parametersSpin0[1]-parametersSpin0[2])*GetYError(z0, x[p]), 2) + TMath::Power(parametersSpin0[1]*GetYError(z2, x[p]), 2) + TMath::Power(parametersSpin0[2]*GetYError(z4, x[p]), 2))));
					spin1Fit->SetPoint(p, x[p], parametersSpin1[0]*((1.-parametersSpin1[1]-parametersSpin1[2])*z0->Eval(x[p]) + parametersSpin1[1]*z2->Eval(x[p]) + parametersSpin1[2]*z4->Eval(x[p])));
					spin1Fit->SetPointError(p, 0., TMath::Sqrt(parametersSpin1[0]*(TMath::Power((1.-parametersSpin1[1]-parametersSpin1[2])*GetYError(z0, x[p]), 2) + TMath::Power(parametersSpin1[1]*GetYError(z2, x[p]), 2) + TMath::Power(parametersSpin1[2]*GetYError(z4, x[p]), 2))));
					spin2Fit->SetPoint(p, x[p], parametersSpin2[0]*((1.-parametersSpin2[1]-parametersSpin2[2])*z0->Eval(x[p]) + parametersSpin2[1]*z2->Eval(x[p]) + parametersSpin2[2]*z4->Eval(x[p])));
					spin2Fit->SetPointError(p, 0., TMath::Sqrt(parametersSpin2[0]*(TMath::Power((1.-parametersSpin2[1]-parametersSpin2[2])*GetYError(z0, x[p]), 2) + TMath::Power(parametersSpin2[1]*GetYError(z2, x[p]), 2) + TMath::Power(parametersSpin2[2]*GetYError(z4, x[p]), 2))));
					spin3Fit->SetPoint(p, x[p], parametersSpin3[0]*((1.-parametersSpin3[1]-parametersSpin3[2])*z0->Eval(x[p]) + parametersSpin3[1]*z2->Eval(x[p]) + parametersSpin3[2]*z4->Eval(x[p])));
					spin3Fit->SetPointError(p, 0., TMath::Sqrt(parametersSpin3[0]*(TMath::Power((1.-parametersSpin3[1]-parametersSpin3[2])*GetYError(z0, x[p]), 2) + TMath::Power(parametersSpin3[1]*GetYError(z2, x[p]), 2) + TMath::Power(parametersSpin3[2]*GetYError(z4, x[p]), 2))));
					spin4Fit->SetPoint(p, x[p], parametersSpin4[0]*((1.-parametersSpin4[1]-parametersSpin4[2])*z0->Eval(x[p]) + parametersSpin4[1]*z2->Eval(x[p]) + parametersSpin4[2]*z4->Eval(x[p])));
					spin4Fit->SetPointError(p, 0., TMath::Sqrt(parametersSpin4[0]*(TMath::Power((1.-parametersSpin4[1]-parametersSpin4[2])*GetYError(z0, x[p]), 2) + TMath::Power(parametersSpin4[1]*GetYError(z2, x[p]), 2) + TMath::Power(parametersSpin4[2]*GetYError(z4, x[p]), 2))));
				}
				spin1Fit->SetLineColor(2); spin1Fit->SetMarkerColor(2); spin1Fit->SetLineWidth(2);
				spin2Fit->SetLineColor(3); spin2Fit->SetMarkerColor(3); spin2Fit->SetLineWidth(2);
				spin3Fit->SetLineColor(4); spin3Fit->SetMarkerColor(4); spin3Fit->SetLineWidth(2);
				spin4Fit->SetLineColor(6); spin4Fit->SetMarkerColor(6); spin4Fit->SetLineWidth(2);

				spin0Fit->Write("Spin0Fit");
				spin1Fit->Write("Spin1Fit");
				spin2Fit->Write("Spin2Fit");
				spin3Fit->Write("Spin3Fit");
				spin4Fit->Write("Spin4Fit");

				output.WriteObject(&parametersSpin0, "ParametersSpin0");
				output.WriteObject(&parametersSpin1, "ParametersSpin1");
				output.WriteObject(&parametersSpin2, "ParametersSpin2");
				output.WriteObject(&parametersSpin3, "ParametersSpin3");
				output.WriteObject(&parametersSpin4, "ParametersSpin4");

				auto a2a4Parameters = A2a4Method(angularDistribution, z0, z2, z4);
				output.WriteObject(&a2a4Parameters, "ParametersA2a4Fit");
			} else { // if(z0 != nullptr && z2 != nullptr && z4 != nullptr)
				std::cerr<<"Failed to find z0 ("<<z0<<", \"graph000\"), z2 ("<<z2<<", \"graph010\"), or z4 ("<<z4<<", \"graph100\") in "<<theoryFile<<std::endl; 
			}
		} else { // if(theory.IsOpen())
			std::cerr<<"Failed to open "<<theoryFile<<std::endl;
		}
	} else { // if(!theoryFile.empty())
		std::cout<<"No file with simulation results (--theory flag), so we won't produce chi2 vs mixing angle plot."<<std::endl;
	}
	output.cd();

	rawAngularDistribution->Write();
	angularDistribution->Write();
	mixedAngularDistribution->Write();
	rawChiSquares->Write();
	mixedChiSquares->Write();

	output.Close();
	input.Close();

	return 0;
}

class Ac {
public:
	Ac(TGraphErrors* data = nullptr, TGraphErrors* z0 = nullptr, TGraphErrors* z2 = nullptr, TGraphErrors* z4 = nullptr)
		: fData(data), fZ0(z0), fZ2(z2), fZ4(z4) {}

	void Data(TGraphErrors* data) { fData = data; }
	void Z0(TGraphErrors* z0) { fZ0 = z0; }
	void Z2(TGraphErrors* z2) { fZ2 = z2; }
	void Z4(TGraphErrors* z4) { fZ4 = z4; }
	void SetZ(TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4) { fZ0 = z0; fZ2 = z2; fZ4 = z4; }

	int Np() { return fData->GetN(); }

	double operator()(const double* p) {
		double chi2 = 0;
		for(int point = 0; point < fData->GetN(); ++point) {
			// get data values
			double x = fData->GetPointX(point);
			double y = fData->GetPointY(point);
			double yError = fData->GetErrorY(point);

			// get simulation values
			// for the y-values we can use Eval (uses linear interpolation)
			double functionValue = p[0]*((1.-p[1]-p[2])*fZ0->Eval(x) + p[1]*fZ2->Eval(x) + p[2]*fZ4->Eval(x));
			// for the y uncertainties we need to find the index (for each graph)
			double errorSquare = TMath::Power(yError, 2) + TMath::Power(p[0], 2)*(TMath::Power((1.-p[1]-p[2])*GetYError(fZ0, x), 2) + TMath::Power(p[1]*GetYError(fZ2, x), 2) + TMath::Power(p[2]*GetYError(fZ4, x), 2));

			// calculate chi^2
			chi2 += TMath::Power((y-functionValue), 2)/errorSquare;
		}
		return chi2;
	}

private:
	TGraphErrors* fData{nullptr};
	TGraphErrors* fZ0{nullptr};
	TGraphErrors* fZ2{nullptr};
	TGraphErrors* fZ4{nullptr};
};

TGraph* MixingMethod(TGraphErrors* data, TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4, int twoJhigh, int twoJmid, int twoJlow, std::vector<double>& bestParameters)
{
	TGraph* result = nullptr;
	Ac ac(data, z0, z2, z4);
	ROOT::Fit::Fitter fitter;
	int nPar = 3;
	fitter.SetFCN(nPar, ac);
	for(int i = 0; i < nPar; ++i) {
		// parameter settings arguments are parameter name, initial value, step size, minimum, and maximum
		fitter.Config().ParSettings(i) = ROOT::Fit::ParameterSettings(Form("a_{%d}", 2*i), 0.5, 0.0001, -10., 10.);
	}
	fitter.Config().MinimizerOptions().SetPrintLevel(0);
	fitter.Config().SetMinimizer("Minuit2", "Migrad"); // or simplex?

	// j1 is the spin of the highest level
	// j2 is the spin of the middle level
	// j3 is the spin of the bottom level
	double j1 = 0.5*twoJhigh;
	double j2 = 0.5*twoJmid;
	double j3 = 0.5*twoJlow;
	// l1 is the transition between j1 and j2
	// a is the lowest allowed spin
	// b is the mixing spin
	int l1a = TMath::Abs(twoJhigh-twoJmid)/2;
	if(l1a == 0) l1a = 1;
	int l1b = l1a + 1;
	// l2 is the transition between j2 and j3
	// a is the lowest allowed spin
	// b is the mixing spin
	int l2a = TMath::Abs(twoJmid-twoJlow)/2;
	if(l2a == 0) l2a = 1;
	int l2b = l2a + 1;

	// run some quick checks on the mixing ratios
	if((twoJhigh==0 && twoJmid==0) || (twoJmid==0 && twoJlow==0)) {
		std::cout<<"!!!!!!!!!!!!!!! ERROR !!!!!!!!!!!!!!!"<<std::endl
			<<"Can't have gamma transition between J=0 states (high "<<twoJhigh<<", mid "<<twoJmid<<", low "<<twoJlow<<")."<<std::endl
			<<"Aborting..."<<std::endl;
		return result;
	}
	if(l1a==TMath::Abs(twoJhigh+twoJmid)/2) {
		//std::cout<<"!!!!!!!!!!!!!!! ALERT !!!!!!!!!!!!!!!"<<std::endl
		//	<<"Only one angular momentum allowed for high->middle transition (l1a "<<l1a<<" == "<<TMath::Abs(twoJhigh+twoJmid)/2<<")."<<std::endl
		//	<<"That mixing ratio (delta1) will be fixed at zero."<<std::endl
		//	<<"!!!!!!!!!!!!! END ALERT !!!!!!!!!!!!!"<<std::endl;
		l1b = l1a;
	}
	if(l2a==TMath::Abs(twoJmid+twoJlow)/2) {
		//std::cout<<"!!!!!!!!!!!!!!! ALERT !!!!!!!!!!!!!!!"<<std::endl
		//	<<"Only one angular momentum allowed for middle->low transition (l2a "<<l2a<<" == "<<TMath::Abs(twoJmid+twoJlow)/2<<")."<<std::endl
		//	<<"That mixing ratio (delta2) will be fixed at zero."<<std::endl
		//	<<"!!!!!!!!!!!!! END ALERT !!!!!!!!!!!!!"<<std::endl;
		l2b = l2a;
	}

	// -------------------------------------------------------------------//
	//                       Constrained fitting
	// -------------------------------------------------------------------//
	// The basic idea is to select a particular set of physical quantities,
	// calculate a2/a4, fix the a2/a4 parameters, and fit the scaling
	// factor a0. Then output the specifications for that set of physical
	// quantities and the chi^2 for further analysis.
	// -------------------------------------------------------------------//

	// delta runs from -infinity to infinity (unless constrained by known physics)
	// in this case, it then makes more sense to sample evenly from tan^{-1}(delta)
	// The next few lines are where you may want to include limits to significantly speed up calculations
	// mixing for the high-middle transition
	double mixingAngle1Minimum = -TMath::Pi()/2;
	double mixingAngle1Maximum = TMath::Pi()/2;
	int steps1 = 100;
	double stepSize1 = (mixingAngle1Maximum-mixingAngle1Minimum)/steps1;
	// mixing for the middle-low transition
	double mixingAngle2Minimum = -TMath::Pi()/2;
	double mixingAngle2Maximum = TMath::Pi()/2;
	int steps2 = 100;
	double stepSize2 = (mixingAngle2Maximum-mixingAngle2Minimum)/steps2;

	// if appropriate, constrain the delta values
	if(l1a==l1b) { mixingAngle1Minimum = 0; steps1 = 1; }
	if(l2a==l2b) { mixingAngle2Minimum = 0; steps2 = 1; }

	result = new TGraph(steps1);
	double minChi2 = 1e6;
	for(int i=0; i < steps1; i++) {
		double mixangle1 = mixingAngle1Minimum + i*stepSize1;
		double delta1 = TMath::Tan(mixangle1);
		for(int j=0; j < steps2; j++) {
			double mixangle2 = mixingAngle2Minimum + j*stepSize2;
			double delta2 = TMath::Tan(mixangle2);
			// calculate a2
			double a2 = TGRSIFunctions::CalculateA2(j1,j2,j3,l1a,l1b,l2a,l2b,delta1,delta2);
			// fix a2
			fitter.Config().ParSettings(1).Set("a_{2}", a2);
			fitter.Config().ParSettings(1).Fix();
			// calculate a4
			double a4 = TGRSIFunctions::CalculateA4(j1,j2,j3,l1a,l1b,l2a,l2b,delta1,delta2);
			// fix a4
			fitter.Config().ParSettings(2).Set("a_{4}", a4);
			fitter.Config().ParSettings(2).Fix();
			if(!fitter.FitFCN()) {
				std::cerr<<i<<", "<<j<<": Fit failed using a2 "<<a2<<", a4 "<<a4<<std::endl;
				continue;
			}
			auto fitResult = fitter.Result();
			// MinFcnValue() is the minimum chi2, ac.Np gives the number of data points
			result->SetPoint(i, mixangle1, fitResult.MinFcnValue()/(ac.Np()-fitResult.NFreeParameters()));
			if(fitResult.MinFcnValue()/(ac.Np()-fitResult.NFreeParameters()) < minChi2) {
				minChi2 = fitResult.MinFcnValue()/(ac.Np()-fitResult.NFreeParameters());
				bestParameters.assign(fitResult.GetParams(), fitResult.GetParams()+nPar);
			}
		}
	}
	return result;
}


std::vector<double> A2a4Method(TGraphErrors* data, TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4)
{
	// create a copy of the data with cos(theta) as x-axis and fit it with a legenre polynomial
	// this is to get initial conditions for our fit
	auto cosTheta = new TGraphErrors(*data);
	auto x = cosTheta->GetX();
	for(int i = 0; i < cosTheta->GetN(); ++i) {
		x[i] = TMath::Cos(x[i]/180.*TMath::Pi());
	}

	int nPar = 3;
	auto legendre = new TF1("legendre", TGRSIFunctions::LegendrePolynomial, -1., 1., nPar);
	legendre->SetParNames("a_{0}", "a_{2}", "a_{4}");
	legendre->SetParameters(1., 0.5, 0.5);
	cosTheta->Fit(legendre, "QN0");

	// the actual fitting
	Ac ac(data, z0, z2, z4);
	ROOT::Fit::Fitter fitter;
	fitter.SetFCN(nPar, ac);
	// this is a good guess for the initial scale
	fitter.Config().ParSettings(0) = ROOT::Fit::ParameterSettings("a_{0}", data->GetMaximum()/z0->GetMaximum(), 0.0001, -10., 10.);
	for(int i = 1; i < nPar; ++i) {
		// parameter settings arguments are parameter name, initial value, step size, minimum, and maximum
		fitter.Config().ParSettings(i) = ROOT::Fit::ParameterSettings(Form("a_{%d}", 2*i), legendre->GetParameter(i), 0.0001, -10., 10.);
	}
	fitter.Config().MinimizerOptions().SetPrintLevel(0);
	fitter.Config().SetMinimizer("Minuit2", "Migrad"); // or simplex?
	if(!fitter.FitFCN()) {
		std::cerr<<"Fit failed"<<std::endl;
		return std::vector<double>();
	}


	// get the fit result and print chi^2 and parameters
	auto fitResult = fitter.Result();
	// MinFcnValue() is the minimum chi2, ac.Np gives the number of data points
	std::cout<<"Reduced chi^2: "<<fitResult.MinFcnValue()/(ac.Np()-fitResult.NFreeParameters())<<std::endl;
	std::vector<double> parameters(fitResult.GetParams(), fitResult.GetParams()+nPar);
	auto errors = fitResult.GetErrors();
	std::cout<<"Parameters a_0: "<<parameters[0]<<" +- "<<errors[0]<<", a_2: "<<parameters[1]<<" +- "<<errors[1]<<", a_4: "<<parameters[2]<<" +- "<<errors[2]<<std::endl;
	TMatrixD covariance(nPar, nPar);
	fitResult.GetCovarianceMatrix(covariance);

	// create fit and residual graphs
	auto fit = static_cast<TGraphErrors*>(z0->Clone("fit"));
	for(int i = 0; i < fit->GetN(); ++i) {
		fit->SetPointY(i, parameters[0]*((1. - parameters[1] - parameters[2])*z0->GetPointY(i) + parameters[1]*z2->GetPointY(i) + parameters[2]*z4->GetPointY(i)));
		fit->SetPointError(i, 0., std::abs(parameters[0])*TMath::Sqrt(TMath::Power((1. - parameters[1] - parameters[2])*z0->GetErrorY(i), 2) + TMath::Power(parameters[1]*z2->GetErrorY(i), 2) + TMath::Power(parameters[2]*z4->GetErrorY(i), 2)));
	}

	auto residual = static_cast<TGraphErrors*>(data->Clone("residual"));
	for(int i = 0; i < residual->GetN(); ++i) {
		residual->SetPointY(i, data->GetPointY(i) - fit->GetPointY(i));
		residual->SetPointError(i, 0., TMath::Sqrt(TMath::Power(data->GetErrorY(i), 2) + TMath::Power(fit->GetErrorY(i), 2)));
	}

	// This text box will display the fit statistics
	// with the margins of the pads, the center in x is at 0.55 (0.545 to be exact_, so we center around that point
	auto stats = new TPaveText(0.35, 0.7, 0.75, 0.95, "NDC");
	stats->SetTextFont(133);
	stats->SetTextSize(20);
	stats->SetFillStyle(0);
	stats->SetBorderSize(0);
	stats->AddText(Form("a_{2} = %f #pm %f", parameters[1], errors[1]));
	stats->AddText(Form("a_{4} = %f #pm %f", parameters[2], errors[2]));
	stats->AddText(Form("#chi^{2}/NDF = %.2f", fitResult.MinFcnValue()/(ac.Np()-fitResult.NFreeParameters())));

	// create canvas and two pads (big one for comparison and small one for residuals)
	// wider left margin for y-axis labels and title
	// same for the bottom margin of the residuals
	auto canvas = new TCanvas;
	auto resPad = new TPad("resPad", "resPad", 0., 0., 1., 0.3);
	resPad->SetTopMargin(0.);
	resPad->SetBottomMargin(0.22);
	resPad->SetLeftMargin(0.1);
	resPad->SetRightMargin(0.01);
	resPad->Draw();
	auto compPad = new TPad("compPad", "compPad", 0., 0.3, 1., 1.);
	compPad->SetTopMargin(0.01);
	compPad->SetBottomMargin(0.);
	compPad->SetLeftMargin(0.1);
	compPad->SetRightMargin(0.01);
	compPad->Draw();

	// plot comparison of fit and data
	compPad->cd();

	auto multiGraph = new TMultiGraph;
	fit->SetLineColor(kRed);
	fit->SetFillColor(kRed);
	fit->SetMarkerColor(kRed);
	multiGraph->Add(fit, "l3"); // 3 = filled contour between upper and lower error bars
	data->SetMarkerStyle(kFullDotLarge);
	multiGraph->Add(data, "p");

	multiGraph->SetTitle(";;Normalized Counts;");

	multiGraph->GetXaxis()->SetRangeUser(0., 180.);

	multiGraph->GetYaxis()->CenterTitle();
	multiGraph->GetYaxis()->SetTitleSize(0.05);
	multiGraph->GetYaxis()->SetTitleOffset(1.);

	multiGraph->Draw("a");

	stats->Draw();

	// plot residuals
	resPad->cd();
	residual->SetTitle(";#vartheta [^{o}];Residual");

	residual->GetXaxis()->CenterTitle();
	residual->GetXaxis()->SetTitleSize(0.1);
	residual->GetXaxis()->SetLabelSize(0.1);
	residual->GetXaxis()->SetRangeUser(0., 180.);

	residual->GetYaxis()->CenterTitle();
	residual->GetYaxis()->SetTitleSize(0.1);
	residual->GetYaxis()->SetTitleOffset(0.5);
	residual->GetYaxis()->SetLabelSize(0.08);

	residual->Draw("ap");
	auto zeroLine = new TLine(0., 0., 180., 0.);
	zeroLine->Draw("same");

	fit->Write("A2a4Fit");
	residual->Write("Residual");
	multiGraph->Write("FitComparison");
	canvas->Write("A2a4Canvas");

	return parameters;
}
