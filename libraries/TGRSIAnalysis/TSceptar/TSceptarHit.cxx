#include "TSceptarHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TSceptar.h"
#include "TGRSIOptions.h"

TSceptarHit::TSceptarHit()
{
	// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TSceptarHit::TSceptarHit(const TSceptarHit& rhs) : TDetectorHit(rhs)
{
	// Copy Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

TSceptarHit::TSceptarHit(const TFragment& frag)
{
	frag.Copy(*this);
	if(TSceptar::SetWave()) {
		if(frag.GetWaveform()->empty()) {
			std::cout<<"Warning, TSceptar::SetWave() set, but data waveform size is zero!"<<std::endl;
		}
		frag.CopyWave(*this);
		if(!GetWaveform()->empty()) {
			AnalyzeWaveform();
		}
	}
}

void TSceptarHit::Copy(TObject& rhs) const
{
	// Copies a TSceptarHit
	TDetectorHit::Copy(rhs);
	static_cast<TSceptarHit&>(rhs).fFilter = fFilter;
}

void TSceptarHit::Copy(TObject& obj, bool waveform) const
{
	Copy(obj);
	if(waveform) {
		CopyWave(obj);
	}
}

TVector3 TSceptarHit::GetPosition(Double_t) const
{
	// Gets the position of the current TSceptarHit
	return TSceptar::GetPosition(GetDetector());
}

TVector3 TSceptarHit::GetPosition() const
{
	// Gets the position of the current TSceptarHit
	return GetPosition(GetDefaultDistance());
}

bool TSceptarHit::InFilter(Int_t)
{
	// check if the desired filter is in wanted filter;
	// return the answer;
	return true;
}

void TSceptarHit::Clear(Option_t*)
{
	// Clears the SceptarHit
	fFilter = 0;
	TDetectorHit::Clear();
}

void TSceptarHit::Print(Option_t*) const
{
	/// Prints the SceptarHit. Returns:
	/// Detector
	/// Energy
	/// Time
	Print(std::cout);
}

void TSceptarHit::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<"Sceptar Detector:   "<<GetDetector()<<std::endl;
	str<<"Sceptar hit energy: "<<GetEnergy()<<std::endl;
	str<<"Sceptar hit time:   "<<GetTime()<<std::endl;
	out<<str.str();
}

bool TSceptarHit::AnalyzeWaveform()
{
	// Calculates the cfd time from the waveform
	bool error = false;
	if(!HasWave()) {
		return false; // Error!
	}

	std::vector<Int_t>   baselineCorrections(8, 0);
	std::vector<Short_t> newWaveform(*GetWaveform());

	// all timing algorithms use interpolation with this many steps between two samples (all times are stored as
	// integers)
	unsigned int interpolationSteps  = 256;
	int          delay               = 8;
	double       attenuation         = 24. / 64.;
	int          halfsmoothingwindow = 0; // 2*halfsmoothingwindow + 1 = number of samples in moving window.

	// baseline algorithm: correct each adc with average of first two samples in that adc
	for(size_t i = 0; i < 8 && i < WaveSize(); ++i) {
		baselineCorrections[i] = GetWaveform()->at(i);
	}
	for(size_t i = 8; i < 16 && i < WaveSize(); ++i) {
		baselineCorrections[i - 8] =
			((baselineCorrections[i - 8] + GetWaveform()->at(i)) + ((baselineCorrections[i - 8] + GetWaveform()->at(i)) > 0 ? 1 : -1)) >>
			1;
	}
	for(size_t i = 0; i < WaveSize(); ++i) {
		newWaveform[i] -= baselineCorrections[i % 8];
	}
	SetWaveform(newWaveform);

	SetCfd(CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolationSteps));

	return !error;
}

Int_t TSceptarHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
		unsigned int interpolationSteps)
{
	// Used when calculating the CFD from the waveform
	std::vector<Short_t> monitor;

	return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolationSteps, monitor);
}

Int_t TSceptarHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
		unsigned int interpolationSteps, std::vector<Short_t>& monitor)
{
	// Used when calculating the CFD from the waveform

	Short_t monitormax = 0;

	bool armed = false;

	Int_t cfd = 0;

	std::vector<Short_t> smoothedWaveform;

	if(!HasWave()) {
		return INT_MAX; // Error!
	}

	if(static_cast<unsigned int>(WaveSize()) > delay + 1) {

		if(halfsmoothingwindow > 0) {
			smoothedWaveform = TSceptarHit::CalculateSmoothedWaveform(halfsmoothingwindow);
		} else {
			smoothedWaveform = *GetWaveform();
		}

		monitor.resize(smoothedWaveform.size() - delay);
		monitor[0] = static_cast<Short_t>(attenuation * smoothedWaveform[delay] - smoothedWaveform[0]);
		if(monitor[0] > monitormax) {
			armed      = true;
			monitormax = monitor[0];
		}

		for(unsigned int i = delay + 1; i < smoothedWaveform.size(); ++i) {
			monitor[i - delay] = static_cast<Short_t>(attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay]);
			if(monitor[i - delay] > monitormax) {
				armed      = true;
				monitormax = monitor[i - delay];
			} else {
				if(armed && monitor[i - delay] < 0) {
					armed = false;
					if(monitor[i - delay - 1] - monitor[i - delay] != 0) {
						// Linear interpolation.
						cfd = (i - delay - 1) * interpolationSteps +
							(monitor[i - delay - 1] * interpolationSteps) / (monitor[i - delay - 1] - monitor[i - delay]);
					} else {
						// Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
						cfd = 0;
					}
				}
			}
		}
	} else {
		monitor.resize(0);
	}

	return cfd;
}

std::vector<Short_t> TSceptarHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow)
{
	// Used when calculating the CFD from the waveform

	if(!HasWave()) {
		return {};  // Error!
	}

	std::vector<Short_t> smoothedWaveform(std::max(static_cast<size_t>(0), WaveSize() - 2 * halfsmoothingwindow),
			0);

	for(size_t i = halfsmoothingwindow; i < WaveSize() - halfsmoothingwindow; ++i) {
		for(int j = -static_cast<int>(halfsmoothingwindow); j <= static_cast<int>(halfsmoothingwindow); ++j) {
			smoothedWaveform[i - halfsmoothingwindow] += GetWaveform()->at(i + j);
		}
	}

	return smoothedWaveform;
}

std::vector<Short_t> TSceptarHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow)
{
	// Used when calculating the CFD from the waveform

	if(!HasWave()) {
		return {};  // Error!
	}

	std::vector<Short_t> smoothedWaveform;

	if(halfsmoothingwindow > 0) {
		smoothedWaveform = TSceptarHit::CalculateSmoothedWaveform(halfsmoothingwindow);
	} else {
		smoothedWaveform = *GetWaveform();
	}

	std::vector<Short_t> monitor(std::max(static_cast<size_t>(0), smoothedWaveform.size() - delay), 0);

	for(size_t i = delay; i < smoothedWaveform.size(); ++i) {
		monitor[i - delay] = static_cast<Short_t>(attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay]);
	}

	return monitor;
}
