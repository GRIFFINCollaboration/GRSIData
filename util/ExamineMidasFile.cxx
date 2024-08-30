
#include <Globals.h>

#include <cstdio>
#include <map>
#include <fstream>

#include <TStopwatch.h>
#include <TMidasFile.h>
#include <TMidasEvent.h>

void ExamineMidasFile(const char *filename) {
	std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
	if(!in.is_open()) {
		printf("unable to open file %s\n",filename);
		return;
	}   
	in.seekg(0, std::ifstream::end);
	int64_t filesize = in.tellg();
	in.close();

	TMidasFile  mfile; 
	std::shared_ptr<TMidasEvent> mevent = std::make_shared<TMidasEvent>();
	std::map<int,int> type_counter;

	mfile.Open(filename);

	int64_t starttime    = 0;
	int64_t stoptime     = 0;
	int currenteventnumber = 0;
	int64_t bytesread = 0;

	bool loop = true;

	TStopwatch sw;
	sw.Start();
	if(mfile.GetOdbEvent() != nullptr) {
		printf( DGREEN );
		mfile.GetOdbEvent()->Print();
		printf( RESET_COLOR );
		starttime = mfile.GetOdbEvent()->GetTimeStamp();
	} else {
		printf("Failed to find any midas event!\n");
		return;
	}
	while(loop) {
		bytesread += mfile.Read(mevent);
		switch(mevent->GetEventId()) {
			case 0x8001: //run stop
				printf( "                                                                             \r");
				printf( DRED );
				mevent->Print();
				printf( RESET_COLOR );
				stoptime = mevent->GetTimeStamp();
				loop = false;
				break;
			default:
				type_counter[mevent->GetEventId()]++;
				break;
		}
		if((++currenteventnumber%15000)== 0) {
			std::streamsize precision = std::cout.precision();
			std::cout.precision(2);
			std::cout<<" Looping over event "<<currenteventnumber<<" have looped "<<static_cast<double>(bytesread)/1000000.<<"MB/"<<static_cast<double>(filesize)/1000000.<<" MB => "<<std::setprecision(1)<<static_cast<double>(bytesread)/1000000/sw.RealTime()<<" MB/s              \r";
			std::cout.precision(precision);
			sw.Continue();
		}
	}
	std::streamsize precision = std::cout.precision();
	std::cout.precision(2);
	std::cout<<" Looping over event "<<currenteventnumber<<" have looped "<<static_cast<double>(bytesread)/1000000.<<"MB/"<<static_cast<double>(filesize)/1000000.<<" MB => "<<std::setprecision(1)<<static_cast<double>(bytesread)/1000000/sw.RealTime()<<" MB/s              \r";
	std::cout.precision(precision);
	printf("\n\n");
	printf("EventTypes Seen: \n");
	for(auto& it : type_counter) {
		printf("\tEventId[0x%x]  =  %i\n",it.first,it.second);
	}
	printf("\n");
	printf("Run length =  %lli  seconds\n",static_cast<long long int>(stoptime-starttime));
	printf("\n");
}


#ifndef __CINT__

void PrintUsage() {
	printf("Usage:  ./ExamineMidasFile <runXXXXX.mid>  \n");
	printf("Can take multiple midas files. Currently no other options.\n");
}

int main(int argc, char** argv) {
	if(argc<2) {
		PrintUsage();
		return 1;
	}

	for(int x=1;x<argc;x++) {
		ExamineMidasFile(argv[x]);
	}

	return 0;
}

#endif


