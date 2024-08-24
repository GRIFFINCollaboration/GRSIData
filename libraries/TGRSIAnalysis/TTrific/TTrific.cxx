#include <iostream>
#include "TTrific.h"
#include "TRandom.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TTrific)
/// \endcond

//declaration of static variables that won't change from event to event
Int_t TTrific::fGridX = 0;
Int_t TTrific::fGridY = 0;

//declaration of constant variables

//grid parameters for TRIFIC
const std::array<double, 12> TTrific::fXmm={-33,-27,-21,-15,-9,-3,3,9,15,21,27,33}; //mm distance to the middle of each wire set readout from centre of grid
const std::array<double, 12> TTrific::fYmm={42,36,28,20,12,4,-4,-12,-20,-28,-36,-42};

//angle that grids are offset by (in rads)
const double TTrific::fAngle = (60./180.)*TMath::Pi();
const TVector3 TTrific::fNormalGridVec = TVector3(0,-TMath::Cos(fAngle),TMath::Sin(fAngle)); //vector that points normal to the grid

//normal to the faces
//double spacing = 10.0; //mm between each grid
//double initialSpacing = 20.0; //mm from the window to the first grid

//in cartesian coordinates
const double TTrific::fSpacingCart = 13.0; //mm
const double TTrific::fInitialSpacingCart = 28.0; //mm
double TTrific::fTargetToWindowCart = 700.0; //mm. Non-constant static that represents the target to window distance. Will be different depending on SHARC, TIP, etc
//can be changed via SetSharc(), SetTip(), SetCustomTargetChamber().

bool TTrific::fSetCoreWave = false;

TTrific::TTrific()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTrific::TTrific(const TTrific& rhs) : TDetector(rhs)
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TTrific::Copy(TObject& rhs) const
{
	TDetector::Copy(rhs);

	static_cast<TTrific&>(rhs).fXFragments = fXFragments;
	static_cast<TTrific&>(rhs).fYFragments = fYFragments;
	static_cast<TTrific&>(rhs).fSingFragments = fSingFragments;

	static_cast<TTrific&>(rhs).fTrificBits = 0;
	
}

void TTrific::Print(Option_t*) const
{
	// Prints out TTrific members.
	Print(std::cout);
}

void TTrific::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<GetMultiplicity()<<" hits"<<std::endl;
	str<<fXFragments.size()<<" xHits"<<std::endl;
	str<<fYFragments.size()<<" yHits"<<std::endl;
	str<<fSingFragments.size()<<" singHits"<<std::endl;
	out<<str.str();
}

TTrific& TTrific::operator=(const TTrific& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TTrific::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	auto* hit = new TTrificHit(*frag);
	
	//separate the fragments depending on if they are x, y, or single readout grids.
	//this is based on the ArraySubPosition value of the hit. 
	switch(chan->GetMnemonic()->ArraySubPosition()){
		case TMnemonic::EMnemonic::kX:
			fXFragments.push_back(hit);
			break;
		case TMnemonic::EMnemonic::kY:
			fYFragments.push_back(hit);
			break;
		default:
			fSingFragments.push_back(hit);
			break;
	};

	AddHit(hit);
}

void TTrific::Clear(Option_t* option)
{

	TDetector::Clear(option);
	
	fXFragments.clear(); //!
	fYFragments.clear(); //!
	fSingFragments.clear(); //!
	
	fTrificBits = 0;

	//clear the static event variables
	//fParticle.SetXYZ(0,0,0); //!
	//fRange = 0; //!
}

void TTrific::GetXYGrid()
{
   //check if we have already found the X grid location yet. If so, we don't need to do it again.
   if(fGridX == 0) {                   //if fGridX == 0, then this will trigger indicating that we haven't found the X grid number yet
      if(!fXFragments.empty()) {   //we have to have an x-grid hit in this event to determine the x-grid number
         TTrificHit* hit = fXFragments.at(0);
         fGridX          = hit->GetDetector();
      }
   }
   //check if we have already found the Y grid location yet. If so, we don't need to do it again.
   if(fGridY == 0) {                   //if fGridY == 0, then this will trigger indicating that we haven't found the Y grid number yet
      if(!fYFragments.empty()) {   //we have to have an y-grid hit in this event to determine the y-grid number
         TTrificHit* hit = fYFragments.at(0);
         fGridY          = hit->GetDetector();
      }
   }
}

TVector3 TTrific::GetPosition(Int_t detectorNumber)
{
	//this is called on a TRIFIC hit, and will use the GetPosition() function below to calculate
	//the x,y,z position of the hit at that grid number
	//gives outputs in (x,y,z) in cartesian (beam) coordinates
	// the coordinates (0,0,0) correspond to the centre of the target location, where (presumably) the beam is hitting the target

	//detectorNumber is indexed at 1. 

	//TRIFIC only holds 24 detectors, so doesn't make sense to get the position for detectors 25+. Also doesn't make sense to get the position for grid numbers <1.
	if (24 < detectorNumber || 1 > detectorNumber) return {1.,-1.,-1.*abs(detectorNumber)};
	//-1*abs(detNum) ensures we return a value of (-1,-1,-#), which indicates a problem

	TVector3 vec = TTrific::GetPosition();

	//check if TTrific::GetPosition() returned an error vector for bad reconstruction. If so, we don't need to do anything more and can return an error vector
	if (TVector3(-100,-100,-100) == vec) return vec; //this vector should be well outside expected XY coordinates

	double zCart = fTargetToWindowCart + fInitialSpacingCart + fSpacingCart*detectorNumber; //cartesian distance to the grid of choice in the z-direction
	//this ignores the extra (or lack of) Z due to the tilt of the grids
	//this includes the target-to-window distance. This can be set via SetSharc() or SetTip() (or via other yet-defined functions)

	zCart += zCart*vec.Y()/(TMath::Sqrt(3)-vec.Y()); //this adds (or subtracts) the extra Z distance (Z') due to the offset in Y (which are tilted at 30 degs. from vertical)
	//notes on geometry: 30-60-90 triangle, so Y=sqrt(3)*Z', and Y/(Z+Z') = tan(theta)
	
	return {zCart*vec.X(),zCart*vec.Y(),zCart};
}

TVector3 TTrific::GetPosition()
{
	//Will calculate the x,y,z vector for the TRIFIC event and then return a 3-D vector
	//that represents the TRIFIC event

	//check if we've already calculated the position for this event. If so, just return it. If not, then reset the position variable.
	if (fTrificBits.TestBit(ETrificBits::kPositionCalculated)) return fParticle;
	fParticle.SetXYZ(0,0,0);
	//flag that we're going to (try to at least) calculate the position vector for this event
	fTrificBits.SetBit(ETrificBits::kPositionCalculated,true);

	//Get the grids that are the X and Y grids in this setup
	GetXYGrid();

	//if we don't have both an x and y grid hit in this event, position reconstruction won't be possible.
	if(fXFragments.empty() || fYFragments.empty()){
		//return an error vector
		fParticle.SetXYZ(-100,-100,-100);
		return fParticle;
	} 

	double xMean = 0.0;
	double xEngTotal = 0.0;

	std::vector<int> hitXDets; //vector to hold the x-segments that have a nonzero hit in them

	for(auto* hit: fXFragments){
		//TDetectorHit *hit = fXFragments.at(i);
		//TDetectorHit *hit = i;
		Int_t seg = hit->GetSegment();
		
		if (3 > hit->GetEnergy()) continue; //arbitrary threshold to avoid weird E<0 events
		
		xMean += fXmm[seg]*hit->GetEnergy();
		xEngTotal += hit->GetEnergy();
		
		hitXDets.push_back(seg);
	}

	//check if hitXDets.size() is zero. This happens when we have an x-grid hit or hits, but the energy for all hits is below our arbitrary "noise" threshold.
	//without this, the function will seg-fault when it tries to check for the continuity
   if(hitXDets.empty()) {
      //return an error vector
		fParticle.SetXYZ(-100,-100,-100);
		return fParticle;
   }

   //to check for hit continuity in the grids, we need to sort the vector of segments and then check that every segment
	//between the first and last segment with a nonzero energy in the array is present. If not, we have a discontinuous hit 
	//and will return an error vector for now. in the future we might instead flag this event
	//Ex: we want hits that like this: [0,0,4,3,5,6,5,3] not [0,0,5,0,0,2,3,0]
	//where the index of the example vector is the segment number and the value is the energy

	std::sort(hitXDets.begin(),hitXDets.end());

	for (unsigned int i = 1; i< hitXDets.size(); i++){ //note: this currently just rejects pileup events outright. The class will be modified later to deal with pileup properly
    	if(hitXDets[i] != hitXDets[i-1]+1){
        	fParticle.SetXYZ(-100,-100,-100);
        	return fParticle;
   		}
	} 

	//divide weighted mean by total energy
	xMean /= xEngTotal;

	//now we do the same for the y-hits

	double yMean = 0.0;
	double yEngTotal = 0.0;

	std::vector<int> hitYDets; //vector to hold the y-segments that have a nonzero hit in them

	for (auto* hit: fYFragments){
		//TDetectorHit *hit = i;
		Int_t seg = hit->GetSegment();
		
		if (3 > hit->GetEnergy()) continue; //arbitrary threshold to avoid weird E<0 events
		
		yMean += fYmm[seg]*hit->GetEnergy();
		yEngTotal += hit->GetEnergy();
		
		hitYDets.push_back(seg);
	}

	//check if hitYDets.size() is zero. This happens when we have an y-grid hit or hits, but the energy for all hits is below our arbitrary "noise" threshold.
	//without this, the function will seg-fault when it tries to check for the continuity
	if (hitYDets.empty()){
		//return an error vector
		fParticle.SetXYZ(-100,-100,-100);
		return fParticle;
	}

	//check again for continuity

	std::sort(hitYDets.begin(),hitYDets.end());

	//this will search through the sorted vector to ensure continuity
	for (unsigned int i = 1; i< hitYDets.size(); i++){
    	if(hitYDets[i] != hitYDets[i-1]+1){ //again, this rejects pileup events
        	fParticle.SetXYZ(-100,-100,-100);
        	return fParticle;
   		}
	} 

	//divide weighted mean by total energy
	yMean /= yEngTotal;

	//convert them into cartesion coordinates 
	double yCart = yMean*TMath::Sin(fAngle); //shifts from grid coordinates to XYZ coordinates
	double zYCart = fTargetToWindowCart + fInitialSpacingCart + fSpacingCart*fGridY+yMean*TMath::Cos(fAngle); //add the initial distance from target to grid, window to the grid, plus the number of gaps between the initial grid and the Y grid, plus the extra z-amount due to the hit location
	double zXCart = fTargetToWindowCart + fInitialSpacingCart + fSpacingCart*fGridX; //adds the initial distance from target to grid, window to grid, plus gaps until the x grid. No angle dependence since the grids aren't shifted in angle in x-direction

	double tanX = xMean/zXCart; //determine tangent of the angle in the XY plane
	double tanY = yCart/zYCart; //tan of angle in YZ plane

	//the particle trajectory below is working under the assumption that the beam is centered and hitting the target at exactly (0,0,0)
	//while this may not be 100% true, the beam should be tuned enough that any deviations from that will be small

	fParticle.SetXYZ(tanX,tanY,1); //unnormalized "unit" vector of the particle's trajectory.
	// "X" and "Y" coordinates are the tangent of the angles of the trajectory in the XY and YZ planes, respectively
	//this is all done relative to the target centre

	//TVector3 particleCart = TVector3(xMean,yCart,1); //this makes the vector in cartesian as opposed to angles.

	//double ratioZX = 1./fNormalGridVec.Dot(fParticle.Unit()); //the Z->R corretion factor

	//fParticle.Print();

	return fParticle;
}

Int_t TTrific::GetRange()
{
	//Gets the last grid with an event in it
	//can't just use the size of fHits because there is no guarantee that every
	//grid gets an event, plus the XY position grids may have multiplicity>1

	//check if we've already calculated the fRange for this event. If so, return it. If not, we need to reset the range.
	if (fTrificBits.TestBit(ETrificBits::kRangeCalculated)) return fRange;
	fRange=0;

	//first we'll check the single grid fragment, since there are more of them and they extend further
	for(auto* hit: fSingFragments){
		if(hit->GetDetector() > fRange) fRange = hit->GetDetector();		
	}
	//check if the range is less than the furthest position grid. If so, we need to check that the range isn't at the x or y grid
	//this was changed because there is no guarantee that the x and y grids will stay at positions 3 and 5, so this is generic

	//determine the x and y grid locations
	GetXYGrid();

	//check if the range is less than the max of the grid numbers+1. The +1 is because the higher grid number will be at std::max(xGrid,yGrid) by default
	if (fRange < std::max(fGridX,fGridY)+1){

		if (fRange < fGridX && !fXFragments.empty()) fRange = fGridX; //we need to check if the current range is less than the x grid AND that there is an x-grid hit in this event
		if (fRange < fGridY && !fYFragments.empty()) fRange = fGridY; //we need to check if the current range is less than the y grid AND that there is a y-grid hit in this event
	} //there may be a problem here if fXFragments.size() != 0 (or Y frags too) but all the fragments have E<arb cutoff. However, GetRange() isn't referenced in any other function currently,
	//and the likelihood of that occuring is very small I think.

	//flag that we've calculated the range for this event
	fTrificBits.SetBit(ETrificBits::kRangeCalculated,true);

	return fRange;
}

/*
TVector2 TTrific::GetEdESimple()
{
	//this will calculate a simple E vs dE based only on the energies in the grids at the start and end of trific 

	double dE = 0.;
	double E = 0.;

	for (auto hit: fSingFragments){
		if (hit->GetEnergy() < 3) continue; //arbitrary threshold to avoid E<0 events.
		if (hit->GetDetector() < 6){
			dE += hit->GetEnergy();
		} 
		else if (hit->GetDetector() > 18 && hit->GetDetector() < 23){
			 E += hit->GetEnergy();
		}
	}

	TVector2 EdE;
	EdE.Set(E,dE);

	return EdE;
}*/
