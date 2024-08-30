//Experiment: S1607

//Root version: 6.08/02
//GRSISort:3.1.3.2

//Compilation: If placed in the GRSISort/scripts directory, GRSISort will automatically compile it with a make

//Usage is: TAC_offset /path/to/analysistree.root

//The analog TAC signals are delayed by a (rather) fixed time. This can create problems when building the AnalysisTree coincidences. To avoid that, a constant offset can be applied to the TAC timestamp. This value is given in the calfile. This script helps determining which value should be set. It produces a set of histograms with the timestamp difference between each TAC and its starting LaBr. One set of histograms is done with the TAC offset timestamp set to 0 and the other with the current one from the calfile. You should adjust the offset so the main peak is roughly at 0.

//NOTE: take into account that the histograms are in ns, but the calfile parameters (offset or time build window, for example) are in 10 ns.

#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TMath.h"

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"

#include <vector>
#include <string>

constexpr int FirstChannel = 84;
constexpr int  LastChannel = 91;

int main(int argc, char** argv) {
	if(argc == 1) {
		std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
		return 1;
	}

	std::cout<<std::endl
	         <<"WARNING: This script assumes that the TACs are in channels "<<FirstChannel<<" - "<<LastChannel<<" (which are the default). Should have they been assigned to other channel numbers, the script should be edited acordingly"<<std::endl
	         <<std::endl;

	auto* file = new TFile(argv[1]);

	auto* AnalysisTree = static_cast<TTree*>(file->Get("AnalysisTree"));

	TChannel::ReadCalFromTree(AnalysisTree);

	TChannel* channel = nullptr;

	std::array<double, 8> offset;

	for(int n = FirstChannel; n <= LastChannel; n++) {
		channel = TChannel::GetChannelByNumber(n);
		offset[n-FirstChannel] = static_cast<double>(channel->GetTimeOffset());
		std::cout<<"Current TAC offset in the calfile:  "<<offset[n-FirstChannel]<<" for channel #"<<n<<std::endl;
	}


	TLaBr* labr = nullptr;
	TTAC* tac = nullptr;
	TGriffin* grif = nullptr;

   if(AnalysisTree->SetBranchAddress("TLaBr", &labr) == TTree::kMissingBranch) {
		labr = new TLaBr;
	}
   if(AnalysisTree->SetBranchAddress("TTAC", &tac) == TTree::kMissingBranch) {
		tac = new TTAC;
	}
   if(AnalysisTree->SetBranchAddress("TGriffin", &grif) == TTree::kMissingBranch) {
		grif = new TGriffin;
	}

	Long64_t nEntries = AnalysisTree->GetEntries();

	std::string outname = argv[1];
	outname.replace(outname.begin(), outname.end()-14, "TAC_offset");
	std::cout<<"writing to '"<<outname<<"'"<<std::endl;

	TFile outfile(outname.c_str(), "recreate");

	TList list;

	UInt_t labr1 = 0;
	UInt_t labr2 = 0;
	UInt_t tac1 = 0; //counters

	Double_t  progress = 0.;
	Int_t  multi_labr = 0;
	Int_t multi_tac = 0;
	Int_t multi_grif = 0;

	//TAC offset histograms
	std::array<TH1D*, 8> TAC_offset;
	for(int i = 0; i < 8; ++i) {
		TAC_offset[i] = new TH1D(Form("TAC_offset_%d",i), Form("TAC_offset_%d; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(TAC_offset[i]); 
	}

	std::array<TH1D*, 8> TAC_offset_corrected;
	for(int i = 0; i < 8; ++i) {
		TAC_offset_corrected[i] = new TH1D(Form("TAC_offset_corrected_%d",i), Form("TAC_offset_corrected_%d; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(TAC_offset_corrected[i]); 
	}

	std::array<TH1D*, 8> time_diff;
	for(int i = 0; i < 8; ++i) {
		time_diff[i] = new TH1D(Form("time_diff%d",i), Form("Time difference for LaBr_%d - LaBr with TAC coincidence; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(time_diff[i]); 
	}

	std::array<TH1D*, 8> time_diff_noTAC;
	for(int i = 0; i < 8; ++i) {
		time_diff_noTAC[i] = new TH1D(Form("time_diff_noTAC%d",i), Form("Time difference for LaBr_%d - LaBr with no TAC coincidence; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(time_diff_noTAC[i]); 
	}

	std::array<TH1D*, 8> timestamp_diff_noTACcoinc;
	for(int i = 0; i < 8; ++i) {
		timestamp_diff_noTACcoinc[i] = new TH1D(Form("timestamp_diff_noTACcoinc%d",i), Form("Timestamp difference for LaBr_%d - LaBr, no TAC coincidence; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(timestamp_diff_noTACcoinc[i]); 
	}

	std::array<TH1D*, 8> timestamp_diff_TACcoinc;
	for(int i = 0; i < 8; ++i) {
		timestamp_diff_TACcoinc[i] = new TH1D(Form("timestamp_diff_TACcoinc%d",i), Form("Timestamp difference for LaBr_%d - LaBr, with TAC coincidence; time (ns); counts/ns",i), 10000,-5000.,5000.); list.Add(timestamp_diff_TACcoinc[i]); 
	}

	TH1D* timestamp_diff_hpge = new TH1D("timestamp_diff_hpge", "Timestamp difference for HPGe - LaBr, with TAC coincidence; time (ns); counts/ns", 10000,-5000.,5000.); list.Add(timestamp_diff_hpge); 
	TH1D* time_diff_hpge = new TH1D("time_diff_hpge", "Time difference for HPGe - LaBr, with TAC coincidence; time (ns); counts/ns", 10000,-5000.,5000.); list.Add(time_diff_hpge); 

	for(Long64_t n = 0; n < nEntries; ++n) {  //This is the main loop, that will cycle through the entire AnalysisTree
		AnalysisTree->GetEntry(n);

		//Builds the number of detector hits in each event (multiplicity of the coincidence)
		multi_labr = labr->GetMultiplicity();
		multi_tac = tac->GetMultiplicity();
		multi_grif = grif->GetMultiplicity();

		if(multi_labr==2) {
			labr1=labr->GetLaBrHit(0)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
			labr2=labr->GetLaBrHit(1)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
			timestamp_diff_noTACcoinc[labr2]->Fill(static_cast<Double_t>(labr->GetLaBrHit(1)->GetTimeStamp()-labr->GetLaBrHit(0)->GetTimeStamp()));
			time_diff_noTAC[labr2]->Fill(labr->GetLaBrHit(1)->GetTime()-labr->GetLaBrHit(0)->GetTime());
		}

		if(multi_labr==2 && multi_tac==1) { //It only looks for LaBrx2+TAC coincidences
			labr1=labr->GetLaBrHit(0)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
			labr2=labr->GetLaBrHit(1)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
			tac1=tac->GetTACHit(0)->GetDetector()-1;//GetDetector goes from 1-8, while the counter goes from 0-7, hence the -1
			if(labr1<labr2) {
				if(tac1==labr1) {
					TAC_offset[tac1]->Fill(labr->GetLaBrHit(0)->GetTime()-tac->GetTACHit(0)->GetTime()+offset[tac1]*10);
					TAC_offset_corrected[tac1]->Fill(labr->GetLaBrHit(0)->GetTime()-tac->GetTACHit(0)->GetTime());
					time_diff[labr2]->Fill(labr->GetLaBrHit(1)->GetTime()-labr->GetLaBrHit(0)->GetTime());
					timestamp_diff_TACcoinc[labr2]->Fill(static_cast<Double_t>(labr->GetLaBrHit(1)->GetTimeStamp()-labr->GetLaBrHit(0)->GetTimeStamp()));
					if(multi_grif>0) {
						for(int i=0; i<multi_grif;i++) {
							time_diff_hpge->Fill(grif->GetGriffinHit(i)->GetTime()-labr->GetLaBrHit(0)->GetTime());
							timestamp_diff_hpge->Fill(static_cast<Double_t>(grif->GetGriffinHit(i)->GetTimeStamp()-labr->GetLaBrHit(0)->GetTimeStamp()));
						}
					}
				}
			} else if(labr1>labr2) {
				if(tac1==labr2) {
					TAC_offset[tac1]->Fill(labr->GetLaBrHit(1)->GetTime()-tac->GetTACHit(0)->GetTime()+offset[tac1]*10);
					TAC_offset_corrected[tac1]->Fill(labr->GetLaBrHit(1)->GetTime()-tac->GetTACHit(0)->GetTime());
					time_diff[labr2]->Fill(labr->GetLaBrHit(1)->GetTime()-labr->GetLaBrHit(0)->GetTime());
					timestamp_diff_TACcoinc[labr2]->Fill(static_cast<Double_t>(labr->GetLaBrHit(1)->GetTimeStamp()-labr->GetLaBrHit(0)->GetTimeStamp()));
					if(multi_grif>0) {
						for(int i=0; i<multi_grif;i++) {
							time_diff_hpge->Fill(grif->GetGriffinHit(i)->GetTime()-labr->GetLaBrHit(0)->GetTime());
							timestamp_diff_hpge->Fill(static_cast<Double_t>(grif->GetGriffinHit(i)->GetTimeStamp()-labr->GetLaBrHit(0)->GetTimeStamp()));
						}
					}
				}
			}
		}

		if(n%10000 == 0) {
			progress = (static_cast<double>(n))/(static_cast<double>(nEntries));
			std::cout<<std::setw(4)<<100*progress<<"% done\r"<<std::flush;
		}

	}//Analysistree loop
	std::cout<<"100% done"<<std::endl;

	list.Sort();
	list.Write();

	return 0;
}
