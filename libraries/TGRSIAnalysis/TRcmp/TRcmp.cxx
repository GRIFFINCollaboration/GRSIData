#include <iostream>
#include <vector>
#include "TRcmp.h"

#include <cstdlib>
#include "TRandom.h"
#include "TClass.h"
#include <cmath>
#include "TMath.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TRcmp)
/// \endcond

// function implementations are below

TRcmp::TRcmp()
{
   /// this is the default constructor
   Clear();
}

TRcmp::~TRcmp()
{
   /// this is the destructor for the class that deletes the pointers to avoid memory leaks

   // using range-based for-loop to iterate through the containers
   for(auto* hit : fFrontPMulVector) { // "hit" is a pointer to the element inside the vector
	   delete hit;
   }
   for(auto* hit : fBackNMulVector) {
	   delete hit;
   }
   for(auto* hit : fPixelMulVector) {
	   delete hit;
   }
}

TRcmp& TRcmp::operator=(const TRcmp& rhs)
{
   rhs.Copy(*this);
   return *this;
}

TRcmp::TRcmp(const TRcmp& rhs) : TDetector(rhs)
{
   /// this is the copy constructor
   rhs.Copy(*this);
}

void TRcmp::Copy(TObject& rhs) const
{
   /// Copy function
   TDetector::Copy(rhs); // first call the copy function from the parent class

   // copy the data members of the TRcmp class now

   // copy the fragment vectors
   static_cast<TRcmp&>(rhs).fFrontPFragVector = fFrontPFragVector;
   static_cast<TRcmp&>(rhs).fBackNFragVector  = fBackNFragVector;

   static_cast<TRcmp&>(rhs).fZeroMultiplicity = fZeroMultiplicity;

   static_cast<TRcmp&>(rhs).fCoincidenceTime = fCoincidenceTime;

   // delete the entries and then clear the transient vectors since these are not written to file
   // we first delete the pointers and then clear the vectors, because the vectors are of TRcmpHit class not TRcmp
   for(auto* hit : static_cast<TRcmp&>(rhs).fFrontPMulVector) {
      delete hit;
   }
   static_cast<TRcmp&>(rhs).fFrontPMulVector.clear();
   for(auto* hit : static_cast<TRcmp&>(rhs).fBackNMulVector) {
      delete hit;
   }
   static_cast<TRcmp&>(rhs).fBackNMulVector.clear();
   for(auto* hit : static_cast<TRcmp&>(rhs).fPixelMulVector) {
      delete hit;
   }
   static_cast<TRcmp&>(rhs).fPixelMulVector.clear();
}

void TRcmp::Clear(Option_t* opt)
{
   // Clears the mother and all of the hits
   TDetector::Clear(opt);
   // delete all hits from the transient vectors
   for(auto* hit : fFrontPMulVector) {
      delete hit;
   }
   for(auto* hit : fBackNMulVector) {
      delete hit;
   }
   for(auto* hit : fPixelMulVector) {
      delete hit;
   }
   // clear all vectors
   fFrontPMulVector.clear();
   fBackNMulVector.clear();
   fPixelMulVector.clear();
   fFrontPFragVector.clear();
   fBackNFragVector.clear();
   
   // set data members to default values
   fZeroMultiplicity = 0;
}

void TRcmp::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// this function takes the fragments and stores them into separate front and back vectors

   if(frag == nullptr || chan == nullptr) {
      return;
   }

   const TMnemonic* mnemonic = chan->GetMnemonic();   // declaring a pointer to the TMnemonic class that has the mnemonic for the channel

   if(mnemonic == nullptr) {
      std::cerr << "Trying to add fragment to TRcmp w/o mnemonic in TChannel! mnemonic == nullptr" << std::endl;
      return;
   }

   // group the fragments into two vectors: 1) front fragments and 2) back fragments
   // P means the junction-side (it is front-side for RCMP)
   if(mnemonic->CollectedCharge() == TMnemonic::EMnemonic::kP) { fFrontPFragVector.push_back(*frag); }     // add the fragment to the front vector is collected charge is on the P-side
   // N means the ohmic-side (it is back-side for RCMP)
   else if(mnemonic->CollectedCharge() == TMnemonic::EMnemonic::kN) { fBackNFragVector.push_back(*frag); } // add the fragment to the back vector is collected charge is on the N-side
}

Short_t TRcmp::GetFrontMultiplicity()
{
   /// this function returns the multiplicity of front strip hits

   // clear the vector when calling the function for the first time in the helper (i.e., the bit for the transient vector is not set)
   if(!fRcmpBits.TestBit(ERcmpBits::kFrontSet)) {    // if the flag for transient vector is true then don't clear the vector
	   for(auto* hit : fFrontPMulVector) {
		   delete hit;
	   }
      fFrontPMulVector.clear();
   }

   if(fFrontPFragVector.empty()) {   // only go ahead if the vector contains at least 1 entry
      return 0;
   }

   if(fFrontPMulVector.empty()) {                                             // only add hits to the transient vector if the vector is cleared and empty
      for(UInt_t i = 0; i < fFrontPFragVector.size(); i++) {
         // if(threshold condition) {                                         // software threshold can be enabled here
         fFrontPMulVector.push_back(new TRcmpHit(fFrontPFragVector.at(i)));   // building an RCMP hit out of the fragment and adding it to the front multiplicity vector
         // }
      }
   }

   SetBitNumber(ERcmpBits::kFrontSet, true);   // set the bit for the transient vector to indicate that it has been filled for this build window

   return fFrontPMulVector.size();             // return the size of the vector (i.e., front multiplicity)
}

Short_t TRcmp::GetBackMultiplicity()
{
   /// this function returns the multiplicity of back strip hits

   // clear the vector when calling the function for the first time in the helper (i.e., the bit for the transient vector is not set)
   if(!fRcmpBits.TestBit(ERcmpBits::kBackSet)) {    // if the flag for transient vector is true then don't clear the vector
	   for(auto* hit : fBackNMulVector) {
		   delete hit;
	   }
      fBackNMulVector.clear();
   }

   if(fBackNFragVector.empty()) {   // only go ahead if the vector contains at least 1 entry
      return 0;
   }

   if(fBackNMulVector.empty()) {                                            // only add hits to the transient vector if the vector is cleared and empty
      for(UInt_t i = 0; i < fBackNFragVector.size(); i++) {
         // if(threshold condition) {                                       // software threshold can be enabled here
         fBackNMulVector.push_back(new TRcmpHit(fBackNFragVector.at(i)));   // building an RCMP hit out of the fragment and adding it to the back multiplicity vector
         // }
      }
   }

   SetBitNumber(ERcmpBits::kBackSet, true);   // set the bit for the transient vector to indicate that it has been filled for this build window

   return fBackNMulVector.size();             // return the size of the vector (i.e., back multiplicity)
}

Short_t TRcmp::GetPixelMultiplicity()
{
   /// this function returns the multiplicity of hits that were combined for front and back strips (i.e., pixels)

   // clear the vector when calling the function for the first time in the helper (i.e., the bit for the transient vector is not set)
   if(!fRcmpBits.TestBit(ERcmpBits::kPixelSet)) {    // if the flag for transient vector is true then don't clear the vector
	   for(auto* hit : fPixelMulVector) {
		   delete hit;
	   }
      fPixelMulVector.clear();
   }

   if(fFrontPFragVector.empty() || fBackNFragVector.empty()) {   // only go ahead if both vectors contain at least 1 entry
      return 0;
   }

   if(fPixelMulVector.empty()) {                                                                           // only add hits to the transient vector if the vector is cleared and empty
      for(UInt_t i = 0; i < GetFrontFragmentMultiplicity(); i++) {                                         // loop over the front strips
         for(UInt_t j = 0; j < GetBackFragmentMultiplicity(); j++) {                                       // loop over the back strips
            // if(check threshold condition for both front and back) {                                     // software threshold can be enabled here
            if((fFrontPFragVector.at(i).GetDetector() == fBackNFragVector.at(j).GetDetector())) {          // check if the front and back strips are from the same detector
	            fPixelMulVector.push_back(new TRcmpHit(fFrontPFragVector.at(i), fBackNFragVector.at(j)));   // building an RCMP hit out of the two fragment and adding it to the pixel multiplicity vector
            } else {
               fZeroMultiplicity++;                                                                        // increment this counter if the condition above was not met
            }
            // }
         }
      }
   }

   SetBitNumber(ERcmpBits::kPixelSet, true);   // set the bit for the transient vector to indicate that it has been filled for this build window

   return fPixelMulVector.size();              // return the size of the vector (i.e., pixel multiplicity)
}

void TRcmp::Print(Option_t*) const
{
   Print(std::cout);
}

void TRcmp::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Rcmp contains: " << std::endl;
   str << std::setw(6) << GetFrontFragmentMultiplicity() << " front hits" << std::endl;
   str << std::setw(6) << GetBackFragmentMultiplicity() << " back hits" << std::endl;

   out << str.str();
}

void TRcmp::SetBitNumber(ERcmpBits bit, Bool_t set)
{
   /// Used to set the flags that are stored in TRcmp
   fRcmpBits.SetBit(bit, set);
}
