#include "TTACHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TTAC.h"

TTACHit::TTACHit()
{
	// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTACHit::TTACHit(const TTACHit& rhs) : TDetectorHit(rhs)
{
	// Copy Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

void TTACHit::Copy(TObject& rhs) const
{
	// Copies a TTACHit
	TDetectorHit::Copy(rhs);
	static_cast<TTACHit&>(rhs).fFilter = fFilter;
}

void TTACHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

Double_t TTACHit::GetTempCorrectedCharge(TGraph* correction_graph) const {
	//Applies the kValue ot the charge
	if(correction_graph == nullptr){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	return GetCharge()*correction_graph->Eval(GetTime()/1e9);//The graph should be defined in seconds
}

Double_t TTACHit::TempCorrectedCharge(TGraph* correction_graph) const {
	//Returns the raw charge with no kValue applied
	if(correction_graph == nullptr){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	return Charge()*correction_graph->Eval(GetTime()/1e9);//The graph should be defined in seconds
}

Double_t TTACHit::GetTempCorrectedEnergy(TGraph* correction_graph) const {
	//This will not overwrite the normal energy, nor will it get stored as the energy.
	if(correction_graph == nullptr){
		std::cout << "Graph for temperture corrections is null" << std::endl;
	}

	TChannel* channel = GetChannel();
	if(channel == nullptr) {
		return 0.0;
	}
	if(GetKValue() > 0) {
		return channel->CalibrateENG(TempCorrectedCharge(correction_graph), static_cast<int>(GetKValue()));
	} 
	if(channel->UseCalFileIntegration()) {
		return channel->CalibrateENG(TempCorrectedCharge(correction_graph), 0);
	}
	return channel->CalibrateENG(TempCorrectedCharge(correction_graph));
}

bool TTACHit::InFilter(Int_t)
{
	// check if the desired filter is in wanted filter;
	// return the answer;
	// currently does nothing
	return true;
}

void TTACHit::Clear(Option_t*)
{
	// Clears the TACHit
	fFilter = 0;
	TDetectorHit::Clear();
}

void TTACHit::Print(Option_t*) const
{
	/// Prints the TACHit. Returns:
	/// Detector
	/// Energy
	/// Time
	Print(std::cout);
}

void TTACHit::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<"TAC Detector:   "<<GetDetector()<<std::endl;
	str<<"TAC hit energy: "<<GetEnergy()<<std::endl;
	str<<"TAC hit time:   "<<GetTime()<<std::endl;
	out<<str.str();
}
