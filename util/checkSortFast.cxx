#include <deque>

#include "TFile.h"
#include "TTree.h"
#include "TGriffin.h"

int main(int argc, char** argv)
{
	if(argc == 1) {
		std::cout<<"Usage: "<<argv[0]<<" <analysis root-file>"<<std::endl;
		return 1;
	}

	for(int file = 1; file < argc; ++file) {
		TFile input(argv[file]);
		if(!input.IsOpen()) {
			std::cout<<MAGENTA<<"Failed to open "<<argv[file]<<RESET_COLOR<<std::endl;
			return 1;
		}

		TTree* tree = static_cast<TTree*>(input.Get("AnalysisTree"));
		if(tree == nullptr) {
			std::cout<<"No AnalysysTree in "<<argv[file]<<std::endl;
			continue;
		}
		TGriffin* grif = new TGriffin;
		if(tree->SetBranchAddress("TGriffin", &grif) == TTree::kMissingBranch) {
			std::cout<<"No TGriffin found in file "<<argv[file]<<std::endl;
			continue;
		}

		std::deque<int64_t> lastTS(3, 0);
		std::deque<double> lastE(3, 0.);

		Long64_t nEntries = tree->GetEntries();
		Long64_t e = 0;

		for(e = 0; e < nEntries; ++e) {
			// check if we can advance 100 events, otherwise read last event
			if(e+100 < nEntries) e += 100;
			else e = nEntries - 1;

			tree->GetEntry(e);

			if(e%1000 == 0) {
				std::cout<<"\r"<<(100*e)/nEntries<<"% done"<<std::flush;
			}

			// skip events without griffin hits
			if(grif->GetMultiplicity() == 0) {
				continue;
			}
			// we always compare the first hit only
			auto hit = grif->GetGriffinHit(0);
			bool sameE = true;
			for(auto energy : lastE) {
				if(hit->GetEnergy() != energy) {
					sameE = false;
					break;
				}
			}
			bool sameTS = true;
			for(auto ts : lastTS) {
				if(hit->GetTimeStamp() != ts) {
					sameTS = false;
					break;
				}
			}
			if(sameE && sameTS) {
				std::cerr<<"\r"<<RED<<"Got the same timestamp and energy for entry "<<e<<". File "<<YELLOW<<argv[file]<<RED<<" is bad!"<<RESET_COLOR<<std::endl;
				break;
			}
			lastTS.emplace_back(hit->GetTimeStamp());
			lastE.emplace_back(hit->GetEnergy());
			lastTS.pop_front();
			lastE.pop_front();
		}
		if(e == nEntries) {
			std::cerr<<"\r"<<GREEN<<"File "<<BLUE<<argv[file]<<GREEN<<" is good!"<<RESET_COLOR<<std::endl;
		}

		input.Close();
	}

	return 0;
}
