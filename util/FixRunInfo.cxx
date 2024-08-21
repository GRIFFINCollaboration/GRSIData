// little script to read run info from .odb file and write to root file(s)

#include <iostream>
#include <iomanip>
#include <fstream>

#include "TFile.h"
#include "TRunInfo.h"

int main(int argc, char** argv)
{
	// need at least two arguments
	if(argc < 3) {
		std::cout<<"Usage: "<<argv[0]<<" <odb file> <root file(s)>"<<std::endl;
		return 1;
	}

	std::ifstream odbFile(argv[1]);

	std::string line;
	std::string runComment;
	std::string runTitle;
	std::string subString;
	int runNumber = 0;
	int subRunNumber = 0;
	uint32_t runStart = 0;
	uint32_t runStop = 0;

	while(std::getline(odbFile, line)) {
		// Expected format for run parameters (comment and title)
		// [/Experiment/Run parameters]
		// Comment = STRING : [256] 
		// Run Title = STRING : [256]
		if(line == "[/Experiment/Run parameters]") {
			std::cout<<"found line "<<line<<std::endl;
			std::getline(odbFile, line);
			runComment = line.substr(25, std::string::npos);
			std::cout<<"got run comment '"<<runComment<<"' from line "<<line<<std::endl;
			std::getline(odbFile, line);
			runTitle = line.substr(27, std::string::npos);
			std::cout<<"got run title '"<<runTitle<<"' from line "<<line<<std::endl;
		}
		// [/Runinfo]
		// State = INT : 3
		// Online Mode = INT : 1
		// Run number = INT : 21360
		// Transition in progress = INT : 2
		// Start abort = INT : 0
		// Requested transition = INT : 0
		// Start time = STRING : [32] Fri Nov 26 13:53:32 2021
		// Start time binary = DWORD : 1637963612
		// Stop time = STRING : [32] Fri Nov 26 14:56:26 2021
		// Stop time binary = DWORD : 1637967386
		if(line == "[/Runinfo]") {
			std::cout<<"found line "<<line<<std::endl;
			std::getline(odbFile, line); // skipping state
			std::getline(odbFile, line); // skipping online mode
			std::getline(odbFile, line); // run number
			subString = line.substr(25, std::string::npos);
			runNumber = std::stoi(subString);
			std::cout<<"got run number "<<runNumber<<" from substring '"<<subString<<"' from line "<<line<<std::endl;
			std::getline(odbFile, line); // skipping transition in progress
			std::getline(odbFile, line); // skipping start abort
			std::getline(odbFile, line); // skipping requested transition
			std::getline(odbFile, line); // skipping start time
			std::getline(odbFile, line); // start time binary
			subString = line.substr(39, std::string::npos);
			runStart = std::stoi(subString);
			std::cout<<"got run start "<<runStart<<" from substring '"<<subString<<"' from line "<<line<<std::endl;
			std::getline(odbFile, line); // skipping stop time
			std::getline(odbFile, line); // stop time binary
			subString = line.substr(38, std::string::npos);
			runStop = std::stoi(subString);
			std::cout<<"got run stop "<<runStop<<" from substring '"<<subString<<"' from line "<<line<<std::endl;

			break;
		}
	}

	TRunInfo* runInfo = TRunInfo::Get();
	std::cout<<"old run info:"<<std::endl;
	runInfo->Print();
	runInfo->SetRunNumber(runNumber);
	runInfo->SetRunTitle(runTitle.c_str());
	runInfo->SetRunComment(runComment.c_str());
	runInfo->SetRunStart(runStart);
	runInfo->SetRunStop(runStop);
	runInfo->SetRunLength();

	for(int i = 2; i < argc; ++i) {
		// parse file name to get run number and sub-run number
		line = argv[i];
		std::size_t dot = line.rfind('_');
		std::size_t underscore = line.rfind('.');
		std::size_t start = line.rfind("fragment");
		if(start != std::string::npos) {
			start += 8;
		} else {
			start = line.rfind("analysis");
			if(start != std::string::npos) {
				start += 8;
			} else {
				std::cerr<<"malformed file name("<<line<<"), should contain either 'fragment' or 'analysis'"<<std::endl;
				continue;
			}
		}
		// check that the order is start - underscore - dot 
		if(start < underscore && underscore < dot && dot != std::string::npos) {
			// format should be path/<fragment or analysis>#####_###.root
			subString = line.substr(start, underscore-start);
			runNumber = std::stoi(subString);
			std::cout<<"got run number "<<runNumber<<" from substring '"<<subString<<"' from line "<<line<<std::endl;
			if(runNumber != runInfo->RunNumber()) {
				std::cerr<<"Mismatch between the run number of this file ("<<runNumber<<") and the run number in the ODB ("<<runInfo->RunNumber()<<"), skipping "<<line<<std::endl;
				continue;
			}
			subString = line.substr(underscore+1, dot-underscore-1);
			subRunNumber = std::stoi(subString);
			std::cout<<"got sub run number "<<runNumber<<" from substring '"<<subString<<"' from line "<<line<<std::endl;
			runInfo->SetSubRunNumber(subRunNumber);
			std::cout<<"new run info:"<<std::endl;
			runInfo->Print();
			TFile f(argv[i]);
			TRunInfo* fileRunInfo = static_cast<TRunInfo*>(f.Get("RunInfo"));
			if(fileRunInfo == nullptr) {
				std::cout<<"no run info found in "<<line<<std::endl;
			} else {
				std::cout<<"run info found in "<<line<<": "<<std::endl;
				fileRunInfo->Print();
			}
			runInfo->Write("RunInfo", TObject::kOverwrite);
			f.Close();
		} else {
			std::cerr<<"malformed file name, expected 'path/<fragment or analysis>#####_###.root', not "<<line<<std::endl;
		}
	}

	return 0;
}
