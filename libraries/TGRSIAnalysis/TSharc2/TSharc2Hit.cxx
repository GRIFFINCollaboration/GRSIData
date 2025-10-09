#include "TSharc2Hit.h"

#include "TSharc2.h"

TSharc2Hit::TSharc2Hit()
{
   Clear("ALL");
}

TSharc2Hit::TSharc2Hit(const TSharc2Hit& rhs) : TDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

void TSharc2Hit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   fBackHit.Copy(static_cast<TObject&>(static_cast<TSharc2Hit&>(rhs).fBackHit));
   fPadHit.Copy(static_cast<TObject&>(static_cast<TSharc2Hit&>(rhs).fPadHit));
}

void TSharc2Hit::Copy(TObject& rhs, bool waveform) const
{
   Copy(rhs);
   if(waveform) {
      CopyWave(rhs);
   }
}

void TSharc2Hit::Clear(Option_t* options)
{
   TDetectorHit::Clear(options);   //
   fBackHit.Clear(options);        //
   fPadHit.Clear(options);         //
}

void TSharc2Hit::Print(Option_t*) const
{
   Print(std::cout);
}

void TSharc2Hit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << DGREEN << "[D/F/B] = " << std::setw(2) << GetDetector() << "\t/" << std::setw(2) << GetFrontStrip() << "\t/" << std::setw(2) << GetBackStrip() << RESET_COLOR << std::endl;
   out << str.str();
}

TVector3 TSharc2Hit::GetPosition(Double_t) const
{
   // return  fposition; // returned from this -> i.e front...
   // PC BENDER PLEASE LOOK AT THIS.
   //
   // this is fine, in all reality this function should not be used in sharc analysis,
   // the buildhits function now properly sets fPosition in the base class, for finer
   // position tweaks of the target, one should just use the static function in the
   // sharc mother class.   pcb.

   return TSharc2::GetPosition(GetDetector(), GetSegment(), GetBack().GetSegment(),
                               TSharc2::GetXOffset(), TSharc2::GetYOffset(), TSharc2::GetZOffset());
}

TVector3 TSharc2Hit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}

Double_t TSharc2Hit::GetTheta(double Xoff, double Yoff, double Zoff) const
{
   TVector3 posOff;
   posOff.SetXYZ(Xoff, Yoff, Zoff);
   return (GetPosition() + posOff).Theta();
}

void TSharc2Hit::SetFront(const TFragment& frag)
{
   frag.Copy(*this);
}

void TSharc2Hit::SetBack(const TFragment& frag)
{
   frag.Copy(fBackHit);
}

void TSharc2Hit::SetPad(const TFragment& frag)
{
   frag.Copy(fPadHit);
}
