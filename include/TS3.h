#ifndef TS3_H
#define TS3_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>
#include <TRandom2.h>

#include "TDetector.h"
#include "TS3Hit.h"
#include "TChannel.h"

class TS3 : public TDetector {
public:
   enum class ES3Bits {
      kPixelsSet = BIT(0),
      kBit1      = BIT(1),
      kBit2      = BIT(2),
      kBit3      = BIT(3),
      kBit4      = BIT(4),
      kBit5      = BIT(5),
      kBit6      = BIT(6),
      kBit7      = BIT(7)
   };

   enum class ES3GlobalBits {
      kPreSector  = BIT(0),   ///< Preference sector energy when building pixels
      kMultHit    = BIT(1),   ///< Attempt to reconstruct multi strip-hit events
      kKeepShared = BIT(2),   ///< When kMultHit, reconstruct rather than discard charge sharing
      kGBit3      = BIT(3)
   };

   TS3();
   TS3(const TS3&);
   TS3(TS3&&) noexcept = default;
   TS3& operator=(const TS3&);
   TS3& operator=(TS3&&) noexcept = default;
   ~TS3() override                = default;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
   void BuildHits() override {}   ///< no need to build any hits, everything already done in AddFragment

   Short_t GetRingMultiplicity() const { return fS3RingHits.size(); }
   Short_t GetSectorMultiplicity() const { return fS3SectorHits.size(); }

   Int_t GetPixelMultiplicity();
   void  SetFrontBackEnergy(double de)
   {
      fFrontBackEnergy = de;
      SetPixels(false);
   }   ///< Set fractional allowed energy difference
   void SetFrontBackEOffset(double de)
   {
      fFrontBackOffset = de;
      SetPixels(false);
   }   ///< Set fractional allowed energy difference
   void SetFrontBackTime(int time)
   {
      fFrontBackTime = time;
      SetPixels(false);
   }   ///< Set absolute allow time difference

   TS3Hit* GetS3Hit(const int& i) const { return static_cast<TS3Hit*>(GetHit(i)); }
   TS3Hit* GetRingHit(const int& i);
   TS3Hit* GetSectorHit(const int& i);
   TS3Hit* GetPixelHit(const int& i);

   static bool PreferenceSector(bool set = true)
   {
      SetGlobalBit(ES3GlobalBits::kPreSector, set);
      return set;
   }   //!<!
   static bool SectorPreference() { return TestGlobalBit(ES3GlobalBits::kPreSector); }   //!<!
   static bool SetMultiHit(bool set = true)
   {
      SetGlobalBit(ES3GlobalBits::kMultHit, set);
      return set;
   }   //!<!
   static bool MultiHit() { return TestGlobalBit(ES3GlobalBits::kMultHit); }   //!<!
   static bool SetKeepShared(bool set = true)
   {
      SetGlobalBit(ES3GlobalBits::kKeepShared, set);
      return set;
   }   //!<!
   static bool KeepShared() { return TestGlobalBit(ES3GlobalBits::kKeepShared); }   //!<!

   bool PixelsSet() const { return TestBitNumber(ES3Bits::kPixelsSet); }
   void SetPixels(bool flag = true) { SetBitNumber(ES3Bits::kPixelsSet, flag); }
   void BuildPixels();

   static TVector3 GetPosition(int ring, int sector, bool smear = false);
   static TVector3 GetPosition(int ring, int sector, double offsetphi, double offsetZ, bool sectorsdownstream,
                               bool smear = false);

   void SetTargetDistance(double dist) { fTargetDistance = dist; }

   void ResetRingsSectors();

   void ClearTransients() override
   {
      fS3Bits = 0;
      for(const auto& hit : fS3PixelHits) {
         hit.ClearTransients();
      }
      for(const auto& hit : fS3RingHits) {
         hit.ClearTransients();
      }
      for(const auto& hit : fS3SectorHits) {
         hit.ClearTransients();
      }
   }

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "all") override;      //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

private:
   std::vector<TS3Hit> fS3RingHits;     ///< vector to store hits of the ring side
   std::vector<TS3Hit> fS3SectorHits;   ///< vector to store hits of the sector side
   std::vector<TS3Hit> fS3PixelHits;    //!<! transient vector to hold the on-the-fly calculated pixel hits

   TTransientBits<UChar_t> fS3Bits;   ///< flags for transient members
   void                    ClearStatus() { fS3Bits = 0; }
   void                    SetBitNumber(ES3Bits bit, Bool_t set = true);
   Bool_t                  TestBitNumber(ES3Bits bit) const { return (fS3Bits.TestBit(bit)); }

   static TTransientBits<UShort_t> fGlobalS3Bits;   ///< Global Bit
   static void                     SetGlobalBit(ES3GlobalBits bit, Bool_t set = true) { fGlobalS3Bits.SetBit(bit, set); }
   static Bool_t                   TestGlobalBit(ES3GlobalBits bit) { return (fGlobalS3Bits.TestBit(bit)); }

   /// for geometery
   static int fRingNumber;     //!<!
   static int fSectorNumber;   //!<!

   static double fOffsetPhiCon;   //!<!
   static double fOffsetPhiSet;   //!<!

   static double fOuterDiameter;    //!<!
   static double fInnerDiameter;    //!<!
   static double fTargetDistance;   //!<!

   //In cfd units for historic reasons
   static Int_t  fFrontBackTime;     //!<!
   static double fFrontBackEnergy;   //!<!
   static double fFrontBackOffset;   //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TS3, 4)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
