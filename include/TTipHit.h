#ifndef TTIPHIT_H
#define TTIPHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TDetectorHit.h"

class TTipHit : public TDetectorHit {
public:
   TTipHit();
   explicit TTipHit(const TFragment&);
   TTipHit(const TTipHit&);
   TTipHit(TTipHit&&) noexcept            = default;
   TTipHit& operator=(const TTipHit&)     = default;
   TTipHit& operator=(TTipHit&&) noexcept = default;
   ~TTipHit() override                    = default;

private:
   Int_t    fFilter{0};   //
   Double_t fPID{0.};     //
   Int_t    fChiSq{0};
   Int_t    fFitType{0};

   // Double_t fFastAmplitude;
   // Double_t fSlowAmplitude;
   // Double_t fGammaAmplitude;

   bool csi_flag{true};

   Int_t fTipChannel{0};

   Double_t fTimeFit{0.};
   Double_t fSig2Noise{0.};

public:
   /////////////////////////    /////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; }   //!<!
   inline void SetPID(Double_t x) { fPID = x; }                  //!<!
   inline void SetTipChannel(const int x) { fTipChannel = x; }   //!<!

   inline Int_t    GetFilterPattern() const { return fFilter; }      //!<!
   inline Double_t GetPID() const { return fPID; }                   //!<!
   inline Int_t    GetFitChiSq() const { return fChiSq; }            //!<!
   inline Double_t GetFitTime() const { return fTimeFit; }           //!<!
   inline Int_t    GetFitType() const { return fFitType; }           //!<!
   inline Double_t GetSignalToNoise() const { return fSig2Noise; }   //!<!
   inline Int_t    GetTipChannel() const { return fTipChannel; }     //!<!

   inline bool IsCsI() const { return csi_flag; }              //!<!
   inline void SetCsI(bool flag = true) { csi_flag = flag; }   //!<!
   inline void SetFitChiSq(int chisq) { fChiSq = chisq; }      //!<!

   bool InFilter(Int_t);   //!<!

   // void SetVariables(const TFragment &frag) { SetAddress(frag.ChannelAddress);
   //									 SetCfd(frag.GetCfd());
   //                                   SetCharge(frag.GetCharge());
   //                                     SetTimeStamp(frag.GetTimeStamp()); }

   void SetUpNumbering(TChannel*)
   {
      TChannel* channel = GetChannel();
      if(channel == nullptr) {
         Error("SetDetector", "No TChannel exists for address %u", GetAddress());
         return;
      }
      Int_t tmp = atoi(channel->GetMnemonic()->ArraySubPositionString().c_str());
      SetTipChannel(10 * channel->GetMnemonic()->ArrayPosition() + tmp);
      if(channel->GetMnemonic()->SubSystemString().compare(0, 1, "W") == 0) {
         SetCsI();
      }
   }

   void SetWavefit(const TFragment&);

   void SetPID(const TFragment&);

   TVector3 GetPosition() const override;

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTipHit, 1);   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
