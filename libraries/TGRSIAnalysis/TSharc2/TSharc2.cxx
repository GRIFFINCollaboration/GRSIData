#include "TSharc2.h"
#include "TMnemonic.h"

#include <cstdio>
#include <iostream>
#include <cmath>

#include "TClass.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TSharc2)
/// \endcond



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

//all units in mm unless otherwise noted

//default offset of things. Assume 0 unless otherwise noted.
double TSharc2::fXoffset = +0.00; //
double TSharc2::fYoffset    = +0.00; //
double TSharc2::fZoffset    = +0.00; //

//UBOX details
double TSharc2::fXdim   = +72.0; // total X dimension of all boxes
double TSharc2::fYdim   = +72.0; // total Y dimension of all boxes
double TSharc2::fZdim   = +48.0; // total Z dimension of all boxes
double TSharc2::fXposUB = +42.5;
double TSharc2::fYminUB = -36.0;
double TSharc2::fZminUB = -5.00;

//Downstream cS2 details
double TSharc2::fZposDS2 = 58.5; // short spacer
//double TSharc2::fZposDS2 = 41.5; //long spacer
double TSharc2::fXminDS2 = 11.0; //inner radius of cS2 active area
double TSharc2::fStripPitchDS2 = 0.5; //0.5 mm strip width;
double TSharc2::fSectorWidthDS2 = 2*TMath::Pi()/16.0; //16 sectors. this is in radians

//Upstream cS2 details
double TSharc2::fZposUS2 = 72.0; // distance from target to UcS2
double TSharc2::fXminUS2 = 11.0; //inner radius of cS2 active area
double TSharc2::fStripPitchUS2 = 0.5; //0.5 mm strip width;
double TSharc2::fSectorWidthUS2 = 2*TMath::Pi()/16.0; //16 sectors. this is in radians


// const int TSharc2::frontstripslist[16]     = {16,16,16,16,  24,24,24,24,  24,24,24,24,  16,16,16,16};
// const int TSharc2::backstripslist[16]      = {24,24,24,24,  48,48,48,48,  48,48,48,48,  24,24,24,24};

// const double TSharc2::frontpitchlist[16]   = {2.0,2.0,2.0,2.0,  3.0,3.0,3.0,3.0,  3.0,3.0,3.0,3.0,  2.0,2.0,2.0,2.0};
// const double TSharc2::backpitchlist[16]    = {TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,
// 1.0,1.0,1.0,1.0,  1.0,1.0,1.0,1.0,  TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48,TMath::Pi()/48};
// QQQ back pitches are angles
//
double TSharc2::fStripFPitch = TSharc2::fYdim / 24.0; // TSharc2::frontstripslist[5]; // 72.0/24 = 3.0 mm
double TSharc2::fStripBPitch = TSharc2::fZdim / 48.0; // TSharc2::backstripslist[5] ; // 48.0/48 = 1.0 mm

// The dimensions are described for a single detector of each type UQ,UB,DB,DQ, and all other detectors can be
// calculated by rotating this

TSharc2::TSharc2()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TSharc2::~TSharc2() = default;

TSharc2::TSharc2(const TSharc2& rhs) : TDetector()
{
	Class()->IgnoreTObjectStreamer(kTRUE);
	Clear("ALL");
	rhs.Copy(*this);
}

void TSharc2::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	if(frag == nullptr || chan == nullptr) {
		return;
	}
	switch(chan->GetMnemonic()->ArraySubPosition()) {
		case TMnemonic::EMnemonic::kD:
			if(chan->GetMnemonic()->CollectedCharge() == TMnemonic::EMnemonic::kP) {
				fFrontFragments.push_back(*frag);
			} else {
				fBackFragments.push_back(*frag);
			}
			break;
			//pads here
		/*
			case TMnemonic::EMnemonic::kE:
			fPadFragments.push_back(*frag);
			break;
		*/
		default:
			break;
	};
}

void TSharc2::BuildHits()
{
	std::vector<TFragment>::iterator front;
	std::vector<TFragment>::iterator back;
	std::vector<TFragment>::iterator pad;

	for(front = fFrontFragments.begin(); front != fFrontFragments.end();) {
		bool front_used = false;
		bool back_used  = false;
		for(back = fBackFragments.begin(); back != fBackFragments.end(); back++) {
			if(front->GetDetector() == back->GetDetector()) {
				front_used = true;
				back_used  = true;
				break;
			}
		}
		if(front_used && back_used) {
			TSharc2Hit* hit = new TSharc2Hit;
			hit->SetFront(*front);
			hit->SetBack(*back);
			fHits.push_back(hit);
			front = fFrontFragments.erase(front);
			back  = fBackFragments.erase(back);
		} else {
			front++;
		}
	}
	//this is the thick full energy pads in the downstream BOX from SHARC-1. If we add pads to SHARC-2 later, this is a good starting point
	/*
	for(auto& fSharcHit : fHits) {
		for(pad = fPadFragments.begin(); pad != fPadFragments.end(); pad++) {
			if(fSharcHit->GetDetector() == pad->GetDetector()) {
				static_cast<TSharc2Hit*>(fSharcHit)->SetPad(*pad);
				pad = fPadFragments.erase(pad);
				break;
			}
		}
	}
	*/
}

void TSharc2::RemoveHits(std::vector<TSharc2Hit>* hits, std::set<int>* to_remove)
{
	for(auto iter = to_remove->rbegin(); iter != to_remove->rend(); ++iter) {
		if(*iter == -1) {
			continue;
		}
		hits->erase(hits->begin() + *iter);
	}
}

void TSharc2::Clear(Option_t* option)
{
	TDetector::Clear(option);
	fHits.clear();

	fFrontFragments.clear(); //!
	fBackFragments.clear();  //!
	//fPadFragments.clear();   //!

	if(strcmp(option, "ALL") == 0) {
		fXoffset = 0.00;
		fYoffset = 0.00;
		fZoffset = 0.00;
	}
	return;
}

void TSharc2::Print(Option_t*) const
{
	Print(std::cout);
}

void TSharc2::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<"not yet written..."<<std::endl;
	out<<str.str();
}

void TSharc2::Copy(TObject& rhs) const
{
	// if(!rhs.InheritsFrom("TSharc2"))
	//  return;
	TDetector::Copy(rhs);

	static_cast<TSharc2&>(rhs).fXoffset   = fXoffset;
	static_cast<TSharc2&>(rhs).fYoffset   = fYoffset;
	static_cast<TSharc2&>(rhs).fZoffset   = fZoffset;
}

TVector3 TSharc2::GetPosition(int detector, int frontstrip, int backstrip, double X, double Y, double Z)
{
	int FrontDet = detector;
	int FrontStr = frontstrip;
	// int BackDet  = detector;
	int BackStr = backstrip;
	double nrots   = 0.; // allows us to rotate into correct position

	TVector3 position;
	TVector3 position_offset;
	position_offset.SetXYZ(X, Y, Z);

	//no forward box in SHARC2!! 
	//We will keep detectors 5-8 for the forward box to keep this consistent with the SHARC-1 class

	if(FrontDet >= 9 && FrontDet <= 12) { // backward box
		nrots    = FrontDet - 9; // edited to make box 9 on the beam left (+x direction).  assuming rotation consistent with TIGRESS (clockwise when looking along beam travel direction)!
		double x = fXposUB;
		double y = fYminUB + (FrontStr + 0.5) * fStripFPitch; // [(-36.0) - (+36.0)]
		double z = fZminUB - (BackStr + 0.5) * fStripBPitch;  // [(-5.0) - (-53.0)]
		position.SetXYZ(x, y, z);
		position.RotateZ(TMath::Pi() * nrots / 2.); //we rotate here because the rotation will be different for the cS2 detectors
	} 

	//only 1 up/downstream detector for SHARC-2 instead of the 4 QQQs from SHARC-1. We will set them at detector number 1 and 16
	//so that we have a few spare detectors in case we want to add e.g. a thick pad behind the downstream S2 later.
	else if(FrontDet == 1) { // forward (downstream) compact S2
		nrots = (FrontStr - 10) + 0.5; //sector 10 is the sector just clockwise from (+x,y=0). We remove that number to get the nrots needed. We add 0.5 to rotate to the middle of the sector too
		double x = fXminDS2 + fStripPitchDS2 * (BackStr + 0.5); //Counting from the inside radius to the outside radius. Add 0.5 to center in the middle of the ring.
		double y = 0; //we start this with the hit oriented at beam left (+x,y=0). Will rotate afterwards
		double z = fZposDS2;
		position.SetXYZ(x, y, z);
		position.RotateZ(fSectorWidthDS2 * nrots);
	} 
	else if(FrontDet == 16) { // backward (upstream) compact S2
		nrots = (FrontStr - 3) + 0.5; //sector 3 is the sector just clockwise from (+x,y=0). We remove that number to get the nrots needed. We add 0.5 to rotate to the middle of the sector too
		double x = fXminUS2 + fStripPitchUS2 * (BackStr + 0.5); //Counting from the inside radius to the outside radius. Add 0.5 to center in the middle of the ring.
		double y = 0; //we start this with the hit oriented at beam left (+x,y=0). Will rotate afterwards
		double z = -1*fZposUS2; //minus because we are upstream of the reaction target
		position.SetXYZ(x, y, z);
		position.RotateZ(-1 * fSectorWidthUS2 * nrots); //we multiply this one by -1 because the upstream cS2 needs to be rotated the opposite direction. Both cS2 detectors are rotated CW relative to beam, but they are mounted opposite

	}

	
	return (position + position_offset);
}

//these are all old from the SHARC-1 class. Aren't really used in analysis.
/*
double TSharc2::GetDetectorThickness(TSharc2Hit& hit, double dist)
{
	static double fDetectorThickness[16] = {998., 998.,  998.,  1001., 141., 142., 133., 143.,
		999., 1001., 1001., 1002., 390., 390., 383., 385.};
	if(dist < 0.0) {
		dist = fDetectorThickness[hit.GetDetector()];
	}

	double phi_90 = fmod(std::fabs(hit.GetPosition().Phi()), TMath::Pi() / 2);
	double phi_45 = phi_90;
	if(phi_90 > (TMath::Pi() / 4.)) {
		phi_45 = TMath::Pi() / 2 - phi_90;
	}

	if(hit.GetDetector() >= 5 && hit.GetDetector() <= 12) {
		return dist / (TMath::Sin(hit.GetPosition().Theta()) * TMath::Cos(phi_45));
	}
	return std::fabs(dist / (TMath::Cos(hit.GetPosition().Theta())));
}

double TSharc2::GetDeadLayerThickness(TSharc2Hit& hit)
{
	static double fDeadLayerThickness[16] = {0.7, 0.7, 0.7, 0.7, 0.1, 0.1, 0.1, 0.1,
		0.1, 0.1, 0.1, 0.1, 0.7, 0.7, 0.7, 0.7};
	return GetDetectorThickness(hit, fDeadLayerThickness[hit.GetDetector()]);
}

double TSharc2::GetPadThickness(TSharc2Hit& hit)
{
	static double fPadThickness[16] = {0.0, 0.0, 0.0, 0.0, 1534, 1535, 1535, 1539,
		0.0, 0.0, 0.0, 0.0, 0.0,  0.0,  0.0,  0.0};
	return GetDetectorThickness(hit, fPadThickness[hit.GetDetector()]);
}

double TSharc2::GetPadDeadLayerThickness(TSharc2Hit& hit)
{
	static double fPadDeadLayerThickness[16] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	return GetDetectorThickness(hit, fPadDeadLayerThickness[hit.GetDetector()]);
}
*/
