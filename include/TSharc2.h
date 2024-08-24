#ifndef TSHARC2_H
#define TSHARC2_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <map>
#include <set>

#include "TMath.h"
#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TSharc2Hit.h"

class TSharc2 : public TDetector {
public:
   TSharc2();
   ~TSharc2() override;
   TSharc2(const TSharc2& rhs);

   TSharc2Hit* GetSharc2Hit(const int& i) const { return static_cast<TSharc2Hit*>(GetHit(i)); }
   static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X = 0.00, double Y = 0.00,
                               double Z = 0.00); //!<!
   static double   GetXOffset() { return fXoffset; }
   static double   GetYOffset() { return fYoffset; }
   static double   GetZOffset() { return fZoffset; }
   static TVector3 GetOffset() { return {fXoffset, fYoffset, fZoffset}; }
   static void SetXYZOffset(const double x, const double y, const double z)
   {
      fXoffset = x;
      fYoffset = y;
      fZoffset = z;
   }

   int     GetSize() const { return Hits().size(); } //!<!

   void Copy(TObject&) const override;        //!<!
   void Clear(Option_t* = "") override;       //!<!
   void Print(Option_t* = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   TSharc2& operator=(const TSharc2& rhs)
   {
      if(this != &rhs) {
         rhs.Copy(*this);
      }
      return *this;
   } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void BuildHits() override;

private:
   int                    CombineHits(TSharc2Hit*, TSharc2Hit*, int, int);       //!<!
   void                   RemoveHits(std::vector<TSharc2Hit>*, std::set<int>*); //!<!

   // TODO: I think this class could be sped up with a flag method on these transient vectors.
	// Not sure what is supposed to get sped up here, the transient vectors are only used
	// to store fragments in them before the hits are build.
	// After that these vectors aren't used again.
   std::vector<TFragment> fFrontFragments; //!
   std::vector<TFragment> fBackFragments;  //!

   static double fXoffset; //!<!
   static double fYoffset; //!<!
   static double fZoffset; //!<!

   // various sharc dimensions set in mm
   // BOX dimensions
   static double fXdim; //! total X dimension of all boxes
   static double fYdim; //! total Y dimension of all boxes
   static double fZdim; //! total Z dimension of all boxes
   static double fXposUB; //!
   static double fYminUB; //!
   static double fZminUB; //!

   //downstream cS2
   static double fZposDS2; //!
   static double fXminDS2; //!
   static double fStripPitchDS2; //!
   static double fSectorWidthDS2; //!
   
   //upstream cS2
   static double fZposUS2; //!
   static double fXminUS2; //!
   static double fStripPitchUS2; //!
   static double fSectorWidthUS2; //!


   // segmentation
   // static const int frontstripslist[16]   ;
   // static const int backstripslist[16]    ;
   // pitches
   static double fStripFPitch;  //!
   static double fStripBPitch;  //!
   static double fRingPitch;    //!
   static double fSegmentPitch; //! angular pitch, degrees

   /// \cond CLASSIMP
   ClassDefOverride(TSharc2, 7) // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
