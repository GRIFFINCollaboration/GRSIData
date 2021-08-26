#include <iostream>
#include "TTrific.h"
#include "TRandom.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TTrific)
/// \endcond

//declaration of static variables that won't change from event to event
Int_t TTrific::gridX = 0;
Int_t TTrific::gridY = 0;

//declaration of constant variables

//grid parameters for TRIFIC
//double TTrific::xmm[12]={-42,-27,-21,-15,-9,-3,3,9,15,21,27,42};
const double TTrific::xmm[12]={-33,-27,-21,-15,-9,-3,3,9,15,21,27,33}; //mm distance to the middle of each wire set readout from centre of grid
//double TTrific::xW[12]={12,3,3,3,3,3,3,3,3,3,3,12}; //number of wires in eachreadout set

//double TTrific::ymm[12]={48,36,28,20,12,4,-4,-12,-20,-28,-36,-48};
const double TTrific::ymm[12]={42,36,28,20,12,4,-4,-12,-20,-28,-36,-42};
//double TTrific::yW[12]={8,4,4,4,4,4,4,4,4,4,4,8};//number of wires, 2 mm spacing

//angle that grids are offset by (in rads)
const double TTrific::angle = (60./180.)*TMath::Pi();
const TVector3 TTrific::normalGridVec = TVector3(0,-TMath::Cos(angle),TMath::Sin(angle)); //vector that points normal to the grid

//normal to the faces
//double spacing = 10.0; //mm between each grid
//double initialSpacing = 20.0; //mm from the window to the first grid

//in cartesian coordinates
const double TTrific::spacingCart = 13.0; //mm
const double TTrific::initialSpacingCart = 28.0; //mm


bool TTrific::fSetCoreWave = false;

TTrific::TTrific() : TDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTrific::TTrific(const TTrific& rhs) : TDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TTrific::Copy(TObject& rhs) const
{
	TDetector::Copy(rhs);

	static_cast<TTrific&>(rhs).fSetCoreWave = fSetCoreWave;
	
	static_cast<TTrific&>(rhs).fXFragments = fXFragments;
	static_cast<TTrific&>(rhs).fYFragments = fYFragments;
	static_cast<TTrific&>(rhs).fSingFragments = fSingFragments;
	
}

TTrific::~TTrific()
{
	// Default Destructor
}

void TTrific::Print(Option_t*) const
{
	// Prints out TTrific members, currently does nothing.
	printf("%lu fHits\n", fHits.size());
	printf("%lu xHits\n",fXFragments.size());
	printf("%lu yHits\n",fYFragments.size());
	printf("%lu singHits\n",fSingFragments.size());
}

TTrific& TTrific::operator=(const TTrific& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TTrific::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	TTrificHit* hit = new TTrificHit(*frag);
	
	//separate the fragments depending on if they are x, y, or single readout grids.
	//this is based on the ArraySubPosition value of the hit. 
	switch(chan->GetMnemonic()->ArraySubPosition()){
		case TMnemonic::EMnemonic::kX:
			//std::cout << "\nIdentified an X subposition";
			fXFragments.push_back(hit);
			break;
		case TMnemonic::EMnemonic::kY:
			fYFragments.push_back(hit);
			//std::cout << "\nIdentified a Y subposition";
			break;
		default:
			fSingFragments.push_back(hit);
			break;
	};
	
	//fHits.push_back(hit);
	fHits.push_back(std::move(hit));
	
	return;
}

void TTrific::Clear(Option_t* option)
{
	TDetector::Clear(option);
	fHits.clear();
	
	fXFragments.clear(); //!
	fYFragments.clear(); //!
	fSingFragments.clear(); //!
	
	return;
}

void TTrific::GetXYGrid()
{
	//check if we have already found the X grid location yet. If so, we don't need to do it again.
	if (!gridX){ //if gridX == 0, then this will trigger indicating that we haven't found the X grid number yet	
		if (fXFragments.size()){ //we have to have an x-grid hit in this event to determine the x-grid number
			 TDetectorHit *hit = fXFragments.at(0);
			 TTrificHit *trifXHit = static_cast<TTrificHit*>(hit);
			 gridX = trifXHit->GetDetector();
		}
	}
	//check if we have already found the Y grid location yet. If so, we don't need to do it again.
	if (!gridY){ //if gridY == 0, then this will trigger indicating that we haven't found the Y grid number yet
		if (fYFragments.size()){//we have to have an y-grid hit in this event to determine the y-grid number
			 TDetectorHit *hit = fYFragments.at(0);
			 TTrificHit *trifYHit = static_cast<TTrificHit*>(hit);
			 gridY = trifYHit->GetDetector();
		}	
	}

	return;
}

TVector3 TTrific::GetPosition(Int_t detectorNumber)
{
	//this is called on a TRIFIC hit, and will use the GetPosition() function below calculate
	//the x,y,z position of the hit at that grid number
	//gives outputs in (x,y,z) in cartesian (beam) coordinates

	//detectorNumber is indexed at 1. 

	//TRIFIC only holds 24 detectors, so doesn't make sense to get the position for detectors 25+. Also doesn't make sense to get the position for grid numbers <1.
	if (24 < detectorNumber || 1 > detectorNumber) return TVector3(-1,-1,detectorNumber);

	double zCart = initialSpacingCart + spacingCart*detectorNumber; //cartesian distance to the grid of choice in the z-direction
	//this ignores the extra (or lack of) Z due to the tilt of the grids

	TVector3 vec = TTrific::GetPosition();

	zCart += zCart*vec.Y()/(TMath::Sqrt(3)-vec.Y()); //this adds (or subtracts) the extra Z distance due to the offset in Y (which are tilted at 30 degs. from vertical)
	//notes on geometry: 30-60-90 triangle, so Y=sqrt(3)*Z', and Y/(Z+Z') = tan(theta)
	
	TVector3 particle(zCart*vec.X(),zCart*vec.Y(),zCart);
	
	//return TVector3(0,0,detectorNumber);
	return particle;
}

TVector3 TTrific::GetPosition()
{
	//Will calculate the x,y,z vector for the TRIFIC event and then return a 3-D vector
	//that represents the TRIFIC event

	//Get the grids that are the X and Y grids in this setup
	GetXYGrid();

	//if we don't have both an x and y grid hit in this event, position reconstruction won't be possible.
	if(0 == fXFragments.size() || 0 == fYFragments.size()) return TVector3(-1,-1,-1);

	std::vector<double> xGridEnergy; //for keeping track of the hit energies
	std::vector<double> yGridEnergy;

	for (auto i: fXFragments){
		//TDetectorHit *hit = fXFragments.at(i);
		TDetectorHit *hit = i;
		TTrificHit *trifXHit = static_cast<TTrificHit*>(hit);
		UInt_t seg = trifXHit->GetSegment();
		
		if (3 > trifXHit->GetEnergy()) continue; //arbitrary threshold to avoid weird E<0 events
		
		while(xGridEnergy.size() <= seg) xGridEnergy.push_back(0);
		xGridEnergy[seg] = trifXHit->GetEnergy();
	}
	
	for (auto i: fYFragments){
	
		//TDetectorHit *hit = fYFragments.at(i);
		TDetectorHit *hit = i;
		TTrificHit *trifYHit = static_cast<TTrificHit*>(hit);
		UInt_t seg = trifYHit->GetSegment();
		
		if (3 > trifYHit->GetEnergy()) continue; //arbitrary threshold to avoid weird E<0 events
		
		while(yGridEnergy.size() <= seg) yGridEnergy.push_back(0);
		yGridEnergy[seg] = trifYHit->GetEnergy();
	}


	Double_t xMean = 0.0; //keeping track of the weighted mean
	Double_t xEnergyTotal = 0.0; //keeping track of the total energy

	if(fXFragments.size() > 1) { //we only have to loop through the x-hits if we have more than 1. 
	
		bool bStarted = false; //this will be used to see if we have started counting the hit

		//go through the two grids and do a weighted average of the non-zero points in them
		for(unsigned int i = 0; i < xGridEnergy.size(); i++) {
			xEnergyTotal += xGridEnergy[i]; //add the energy of the hit to the total.

			if(!bStarted) {//if we haven't hit a non-zero point yet, check to see if the current one is non-zero
				if(xGridEnergy[i]) { //if it is, we've hit the first segment with an energy from the hit. 
					xMean += xmm[i]*xGridEnergy[i]; //add the weighted mean
					bStarted = true; //signal that we have started averaging
				}
			} else if(xGridEnergy[i]) {//since we've started averaging, now we need to check for continuity. If adjacent segments that aren't on the end don't both have energy, then 
				//this is a discontinuous hit. Ex: we want vectors that look like [0,0,4,6,8,7,5,0,0], not [0,0,4,6,0,4,2,0,0]
				if(xGridEnergy[i-1]) {
					xMean += xmm[i]*xGridEnergy[i]; //add the weighted mean		
				} else {
					//if we have a discontinuous hit, don't bother to try to reconstruct the position
					return TVector3(-1,-1,-2);	
				}
			}
		}
		xMean /= xEnergyTotal; //divide the weighted sum by the total energy of the hits in that grid
	} else { //if we have multiplicity 1 event, the mean will just be the value of the segment that was the multiplicity 1 event.
		for(unsigned int i = 0; i<xGridEnergy.size(); i++) {
			if(!xGridEnergy[i]) xMean = xmm[i]; //
		}
	}

	Double_t yMean = 0.0; //keeping track of the weighted mean
	Double_t yEnergyTotal = 0.0; //keeping track of the total energy

	if(fYFragments.size() > 1) { //we only have to loop through the y-hits if we have more than 1. 

		bool bStarted = false; //this will be used to see if we have started counting the hit

		//go through the two grids and do a weighted average of the non-zero points in them
		for(unsigned int i = 0; i < yGridEnergy.size(); i++) {
			yEnergyTotal += yGridEnergy[i]; //add the energy of the hit to the total.

			if(!bStarted) {//if we haven't hit a non-zero point yet, check to see if the current one is non-zero
				if(yGridEnergy[i]) { //if it is, we've hit the first segment with an energy from the hit. 
					yMean += ymm[i]*yGridEnergy[i]; //add the weighted mean
					bStarted = true; //signal that we have started averaging
				}
			} else if(yGridEnergy[i]) {//since we've started averaging, now we need to check for continuity. If adjacent segments that aren't on the end don't both have energy, then 
				//this is a discontinuous hit. Ex: we want vectors that look like [0,0,4,6,8,7,5,0,0], not [0,0,4,6,0,4,2,0,0]
				if (yGridEnergy[i-1]) {
					yMean += ymm[i]*yGridEnergy[i]; //add the weighted mean		
				} else {
					//if we have a discontinuous hit, don't bother to try to reconstruct the position
					return TVector3(-1,-1,-2);				
				}
			}
		}
		yMean /= yEnergyTotal;
	} else { //if we have multiplicity 1 event, the mean will just be the value of the segment that was the multiplicity 1 event.
		for(unsigned int i = 0; i<yGridEnergy.size(); i++) {
			if(!yGridEnergy[i]) yMean = ymm[i]; //
		}
	}

	//convert them into cartesion coordinates 
	double yCart = yMean*TMath::Sin(angle); //shifts from grid coordinates to XYZ coordinates
	double zYCart = initialSpacingCart + spacingCart*gridY+yMean*TMath::Cos(angle); //add the initial distance from the window to the grid, plus the number of gaps between the initial grid and the Y grid, plus the extra z-amount due to the hit location
	double zXCart = initialSpacingCart + spacingCart*gridX; //adds the initial distance from window to grid, plus gaps until the x grid. No angle dependence since the grids aren't shifted in angle in x-direction

	double tanX = xMean/zXCart; //determine tangent of the angle in the XY plane
	double tanY = yCart/zYCart; //tan of angle in YZ plane

	//the particle trajectory below is working under the assumption that the beam is centered and hitting the window at exactly (0,0,0). 
	//while this may not be 100% true, the beam should be tuned enough that any deviations from that will be small

	TVector3 particle = TVector3(tanX,tanY,1); //unnormalized "unit" vector of the particle's trajectory.
	// "X" and "Y" coordinates are the tangent of the angles of the trajectory in the XY and YZ planes, respectively
	//TVector3 particleCart = TVector3(xMean,yCart,1); //this makes the vector in cartesian as opposed to angles.
	//particleCart.Print();

	//double ratioZX = 1./normalGridVec.Dot(particle.Unit()); //the Z->R corretion factor

	//fflush(stdout);
	
	//particle.Print();

	return particle;
}

Int_t TTrific::GetRange()
{
	//Gets the last grid with an event in it
	//can't just use the size of fHits because there is no guarantee that every
	//grid gets an event, plus the XY position grids may have multiplicity>1
	Int_t range = 0;
	for(auto hit: fHits) {
		if(hit->GetDetector() > range) range = hit->GetDetector();			
	}

	return range;
}
