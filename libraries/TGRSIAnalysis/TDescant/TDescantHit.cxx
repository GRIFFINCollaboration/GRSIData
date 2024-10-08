#include "TDescantHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TDescant.h"
#include "TGRSIOptions.h"
#include "TChannel.h"
#include "GValue.h"

TDescantHit::TDescantHit()
{
   Clear();
}

TDescantHit::TDescantHit(const TDescantHit& rhs) : TDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

TDescantHit::TDescantHit(const TFragment& frag)
{
   frag.Copy(*this);

   SetZc(frag.GetZc());
   SetCcShort(frag.GetCcShort());
   SetCcLong(frag.GetCcLong());

   // if(TDescant::SetWave()) {
   if(TGRSIOptions::Get()->ExtractWaves()) {
      if(frag.GetWaveform()->empty()) {
      }
      frag.CopyWave(*this);
      if(!GetWaveform()->empty()) {
         AnalyzeWaveform();
      }
   }
}

void TDescantHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   if(TGRSIOptions::Get()->ExtractWaves()) {
      TDetectorHit::CopyWave(rhs);
   }
   static_cast<TDescantHit&>(rhs).fFilter     = fFilter;
   static_cast<TDescantHit&>(rhs).fZc         = fZc;
   static_cast<TDescantHit&>(rhs).fCcShort    = fCcShort;
   static_cast<TDescantHit&>(rhs).fCcLong     = fCcLong;
   static_cast<TDescantHit&>(rhs).fPsd        = fPsd;
   static_cast<TDescantHit&>(rhs).fCfdMonitor = fCfdMonitor;
   static_cast<TDescantHit&>(rhs).fPartialSum = fPartialSum;
}

void TDescantHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

TVector3 TDescantHit::GetPosition(Double_t dist) const
{
   /// TDetectorHit::GetPosition
   return TDescant::GetPosition(GetDetector(), dist);
}

TVector3 TDescantHit::GetPosition() const
{
   return TDescant::GetPosition(GetDetector(), GetDefaultDistance());
}

bool TDescantHit::InFilter(Int_t)
{
   /// check if the desired filter is in wanted filter;
   /// return the answer;
   return true;
}

Float_t TDescantHit::GetCfd() const
{
   /// special function for TDescantHit to return CFD after mapping out the high bits
   /// which are the remainder between the 125 MHz data and the 100 MHz timestamp clock
   return static_cast<Float_t>(static_cast<Int_t>(TDetectorHit::GetCfd()) & 0x3fffff) + static_cast<Float_t>(gRandom->Uniform());
}

Int_t TDescantHit::GetRemainder() const
{
   /// returns the remainder between 100 MHz/10ns timestamp and 125 MHz/8 ns data in ns
   return static_cast<Int_t>(TDetectorHit::GetCfd()) >> 22;
}

void TDescantHit::Clear(Option_t*)
{
   fFilter  = 0;
   fPsd     = -1;
   fZc      = 0;
   fCcShort = 0;
   fCcLong  = 0;
   fCfdMonitor.clear();
   fPartialSum.clear();
   TDetectorHit::Clear();
}

void TDescantHit::Print(Option_t*) const
{
   Print(std::cout);
}

void TDescantHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Descant Detector:   " << GetDetector() << std::endl;
   str << "Descant hit energy: " << GetEnergy() << std::endl;
   str << "Descant hit time:   " << GetTime() << std::endl;
   out << str.str();
}

bool TDescantHit::AnalyzeWaveform()
{
   bool error = false;

   if(!HasWave()) {
      return false;   // Error!
   }
   std::vector<Int_t>   baselineCorrections(8, 0);
   std::vector<Short_t> newWaveform;

   // all timing algorithms use interpolation with this many steps between two samples (all times are stored as
   // integers)
   unsigned int interpolationSteps  = 256;
   int          delay               = 8;
   double       attenuation         = 24. / 64.;
   int          halfSmoothingWindow = 0;   // 2*halfsmoothingwindow + 1 = number of samples in moving window.

   // baseline algorithm: correct each adc with average of first two samples in that adc
   for(size_t i = 0; i < 8 && i < WaveSize(); ++i) {
      baselineCorrections[i] = GetWaveform()->at(i);
   }
   for(size_t i = 8; i < 16 && i < WaveSize(); ++i) {
      baselineCorrections[i - 8] =
         ((baselineCorrections[i - 8] + GetWaveform()->at(i)) + ((baselineCorrections[i - 8] + GetWaveform()->at(i)) > 0 ? 1 : -1)) >>
         1;   // Average
   }
   for(size_t i = 0; i < WaveSize(); ++i) {
      newWaveform[i] -= baselineCorrections[i % 8];
   }
   SetWaveform(newWaveform);

   SetCfd(CalculateCfd(attenuation, delay, halfSmoothingWindow, interpolationSteps));

   // PSD
   // time to zero-crossing algorithm: time when sum reaches n% of the total sum minus the cfd time
   double fraction = 0.90;

   SetPsd(CalculatePsd(fraction, interpolationSteps));

   SetCharge(CalculatePartialSum().back());

   if(fPsd < 0) {
      error = true;
   }

   return !(error);
}

Int_t TDescantHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                unsigned int interpolation_steps)
{

   std::vector<Short_t> monitor;

   return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolation_steps, monitor);
}

Int_t TDescantHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfSmoothingWindow,
                                          unsigned int interpolationSteps, std::vector<Short_t>& monitor)
{

   Short_t monitormax = 0;
   bool    armed      = false;

   Int_t cfd = 0;
   if(!HasWave()) {
      return INT_MAX;   // Error!
   }
   std::vector<Short_t> smoothedWaveform;

   if(WaveSize() > delay + 1) {
      if(halfSmoothingWindow > 0) {
         smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
      } else {
         smoothedWaveform = *GetWaveform();
      }

      monitor.clear();
      monitor.resize(smoothedWaveform.size() - delay);
      monitor[0] = static_cast<Short_t>(attenuation * smoothedWaveform[delay] - smoothedWaveform[0]);
      if(monitor[0] > monitormax) {
         armed      = true;
         monitormax = monitor[0];
      }

      for(size_t i = delay + 1; i < smoothedWaveform.size(); ++i) {
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

   if(TGRSIOptions::Get()->Debug()) {
      fCfdMonitor = monitor;
   }

   // correct for remainder between the 100MHz timestamp and the 125MHz start of the waveform
   // we save this in the upper bits, otherwise we can't correct the waveform themselves
   cfd = (cfd & 0x3fffff) | (static_cast<Int_t>(TDetectorHit::GetCfd()) & 0x7c00000);

   return cfd;
}

std::vector<Short_t> TDescantHit::CalculateSmoothedWaveform(unsigned int halfSmoothingWindow)
{

   if(!HasWave()) {
      return {};   // Error!
   }

   std::vector<Short_t> smoothedWaveform(std::max((static_cast<size_t>(0)), WaveSize() - 2 * halfSmoothingWindow),
                                         0);

   for(size_t i = halfSmoothingWindow; i < WaveSize() - halfSmoothingWindow; ++i) {
      for(int j = -static_cast<int>(halfSmoothingWindow); j <= static_cast<int>(halfSmoothingWindow); ++j) {
         smoothedWaveform[i - halfSmoothingWindow] += GetWaveform()->at(i + j);
      }
   }

   return smoothedWaveform;
}

std::vector<Short_t> TDescantHit::CalculateCfdMonitor(double attenuation, unsigned int delay,
                                                      unsigned int halfSmoothingWindow)
{
   if(!HasWave()) {
      return {};   // Error!
   }
   std::vector<Short_t> smoothedWaveform;

   if(halfSmoothingWindow > 0) {
      smoothedWaveform = TDescantHit::CalculateSmoothedWaveform(halfSmoothingWindow);
   } else {
      smoothedWaveform = *GetWaveform();
   }

   std::vector<Short_t> monitor(std::max((static_cast<size_t>(0)), smoothedWaveform.size() - delay), 0);

   for(size_t i = delay; i < WaveSize(); ++i) {
      monitor[i - delay] = static_cast<Short_t>(attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay]);
   }

   return monitor;
}

std::vector<Int_t> TDescantHit::CalculatePartialSum()
{
   if(!HasWave()) {
      return {};   // Error!
   }

   std::vector<Int_t> partialSums(WaveSize(), 0);

   partialSums[0] = GetWaveform()->at(0);
   for(size_t i = 1; i < WaveSize(); ++i) {
      partialSums[i] = partialSums[i - 1] + GetWaveform()->at(i);
   }

   if(TGRSIOptions::Get()->Debug()) {
      fPartialSum = partialSums;
   }

   return partialSums;
}

Int_t TDescantHit::CalculatePsd(double fraction, unsigned int interpolationSteps)
{
   std::vector<Int_t> partialSums;

   return CalculatePsdAndPartialSums(fraction, interpolationSteps, partialSums);
}

Int_t TDescantHit::CalculatePsdAndPartialSums(double fraction, unsigned int interpolationSteps,
                                              std::vector<Int_t>& partialSums)
{
   Int_t psd = 0;

   partialSums = CalculatePartialSum();
   if(partialSums.empty()) {
      return -1;
   }
   int totalSum = partialSums.back();

   fPsd = -1;
   if(partialSums[0] < fraction * totalSum) {
      for(size_t i = 1; i < partialSums.size(); ++i) {
         if(partialSums[i] >= fraction * totalSum) {
            psd = static_cast<Int_t>(static_cast<double>(i * interpolationSteps) - ((partialSums[i] - fraction * totalSum) * interpolationSteps) / GetWaveform()->at(i));
            break;
         }
      }
   }

   return psd;
}
