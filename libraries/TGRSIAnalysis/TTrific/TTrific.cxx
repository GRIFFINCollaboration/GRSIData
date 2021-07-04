#include <iostream>
#include "TTrific.h"
#include "TRandom.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TTrific)
/// \endcond

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
}

TTrific::~TTrific()
{
	// Default Destructor
}

void TTrific::Print(Option_t*) const
{
	// Prints out TTrific members, currently does nothing.
	printf("%lu fHits\n", fHits.size());
}

TTrific& TTrific::operator=(const TTrific& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TTrific::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TTrificHit* hit = new TTrificHit(*frag);
	fHits.push_back(std::move(hit));
}

Int_t TTrific::GetXYGrid(char grid, const TTrific& event)
{
	//used to determine which grid number are x or y grids
	//this is used so that if we change the location of the x and y grids in TRIFIC,
	//the only thing that will need to be modified is the ODB

	Int_t gridNumber = 0; //GetDetector indexes at 1, so if we return a grid number of 0, we weren't able to find the position grid in this event
	if('y' == grid || 'Y' == grid){
		//for(auto i = 0; i < GetMultiplicity(); i++){
		for(auto i = 0; i < event.GetMultiplicity(); i++){
			//y grids are [16,27], so need greater than 15 and less than 28
			//if(static_cast<TTrificHit*>(GetHit(i))->GetSegment() > 15 && static_cast<TTrificHit*>(GetHit(i))->GetSegment() < 28){
			if(GetTrificHit(i)->GetSegment() > 15 && GetTrificHit(i)->GetSegment() < 28){
				gridNumber = GetTrificHit(i)->GetDetector();
				break;
			}
		}
	} 
	else if('x' == grid || 'X' == grid){
		for(auto i = 0; i < event.GetMultiplicity(); i++){
		//for(auto i = 0; i < GetMultiplicity(); i++){
			//x grids are [1,12], so need greater than 0 and less than 13
			if(GetTrificHit(i)->GetSegment() > 0 && GetTrificHit(i)->GetSegment() < 13){
				gridNumber = GetTrificHit(i)->GetDetector();
				break;
			}
		}
	}


	return gridNumber;
}



TVector3 TTrific::GetPosition(Int_t DetectorNumber)
{

	// **** Under Development Still ****

	//this is called on a TRIFIC hit, and will use the GetPosition() function below calculate
	//the x,y,z position of the hit at that grid number
	
	//GetRange();
	
	//return position;
	return TVector3(0,0,DetectorNumber);

}


TVector3 TTrific::GetPosition()
{
	//Will calculate the x,y,z vector for the TRIFIC event and then return a 3-D vector
	//that somehow represents the TRIFIC event


	
	//angle that grids are offset by (in rads)
	double angle = (60./180.)*TMath::Pi();
	TVector3 normalGridVec = TVector3(0,-TMath::Cos(angle),TMath::Sin(angle)); //vector that points normal to the grid
	
	
	//normal to the faces
	//double spacing = 10.0; //mm between each grid
	//double initialSpacing = 20.0; //mm from the window to the first grid

	//in cartesian coordinates
	double spacingCart = 13.0; //mm
	double initialSpacingCart = 28.0; //mm

	
	//grid parameters
	//double xmm[12]={-42,-27,-21,-15,-9,-3,3,9,15,21,27,42};
    double xmm[12]={-33,-27,-21,-15,-9,-3,3,9,15,21,27,33}; //mm distance to the middle of each wire set readout from centre of grid
    //double xW[12]={12,3,3,3,3,3,3,3,3,3,3,12}; //number of wires in eachreadout set
    
	//double ymm[12]={48,36,28,20,12,4,-4,-12,-20,-28,-36,-48};
    double ymm[12]={42,36,28,20,12,4,-4,-12,-20,-28,-36,-42};
    //double yW[12]={8,4,4,4,4,4,4,4,4,4,4,8};//number of wires, 2 mm spacing
   

	//Get the grids that are the X and Y grids in this setup
	Int_t gridX = GetXYGrid('x',*this); //GetXYGrid() returns 0 if it is unable to find the grid in the event
	Int_t gridY = GetXYGrid('y',*this);

	//std::cout << "\nX and Y grids are " << gridX << " " << gridY << "\n"; fflush(stdout);

	if(!gridY || !gridX){
		//if we don't have both an x and y grid hit in this event, position reconstruction won't be possible.
		//std::cout << "\nUnable to find both X and Y grids for this event.\n";
		return TVector3(-1,-1,-1);
	}	


	std::vector<double> xGridEnergy; //for keeping track of the hit energies
	std::vector<double> yGridEnergy;
	
	Int_t xMulti = 0; //keep track of the multiplicity of the x and y grids
	Int_t yMulti = 0; //if we only have multiplicity 1, we won't need to do a weighted average later


	for(auto i = 0; i < GetMultiplicity(); i++){
		Int_t det = GetTrificHit(i)->GetDetector();
	
		//immediately discard any hits that aren't on the x or y grid
		if(det != gridX && det != gridY) continue;

		UInt_t seg = GetTrificHit(i)->GetSegment();
		Double_t eng = GetTrificHit(i)->GetEnergy();

		//set a lower threshold on the energy of the hit
		if(3 > eng) continue; //arbitrary threshold at the moment

		//if we have a segment of 0, something went weird, so we discard this hit
		if(0 == seg) continue;

		//std::cout << "\nDetector and segment are " << det << " " << seg; fflush(stdout);

		if(det == gridX){
			//this will eventually create a vector with as many entries as the highest segment number
			//and fill every non-zero entry with the energy from that segment
			while(xGridEnergy.size() < seg) xGridEnergy.push_back(0); 
			xGridEnergy[seg-1]= eng; //minus 1 because vector indexes at 0 and x segments start at 1
			xMulti++; //increase the multiplicity of the x-hit
		} else if(det == gridY){
			//same thing, create the vector for the y-grids
			//std::cout << "\nSeg is " << seg << " and seg mod is " << seg % 16 << " and size of the grid is " << yGridEnergy.size(); fflush(stdout);
			while(yGridEnergy.size() < (seg%15)) yGridEnergy.push_back(0); //have to mod this by 15 so that if there is a hit seg 16,
																		   //it will create an entry in the matrix (since 16%16=0)
			yGridEnergy[seg % 16] = eng; // mod 16 because the y-grids index starting at 16 and vector at 0
			yMulti++; //increase multiplicity of y-hit
		}
		
	}
	

	Double_t xMean = 0.0; //keeping track of the weighted mean
	Double_t xEnergyTotal = 0.0; //keeping track of the total energy

	if(xMulti > 1){ //we only have to loop through the x-hits if we have more than 1. 
					 //it's possible that this will get caught if we had a single x-hit, but it was on segment 0 somehow. Have to think about this more

		bool bStarted = false; //this will be used to see if we have started counting the hit

		//go through the two grids and do a weighted average of the non-zero points in them
		for(unsigned int i = 0; i < xGridEnergy.size(); i++){
			xEnergyTotal += xGridEnergy[i]; //add the energy of the hit to the total.

			if(!bStarted){//if we haven't hit a non-zero point yet, check to see if the current one is non-zer
				if(xGridEnergy[i]){ //if it is, we've hit the first segment with an energy from the hit. 
					xMean += xmm[i]*xGridEnergy[i]; //add the weighted mean
					bStarted = true; //signal that we have started averaging
				}
			} else if(xGridEnergy[i]){//since we've started averaging, now we need to check for continuity. If adjacent segments that aren't on the end don't both have energy, then 
				//this is a discontinuous hit. Ex: we want vectors that look like [0,0,4,6,8,7,5,0,0], not [0,0,4,6,0,4,2,0,0]
				if(xGridEnergy[i-1]){
					xMean += xmm[i]*xGridEnergy[i]; //add the weighted mean		
				} else {
					//if we have a discontinuous hit, don't bother to try to reconstruct the position
					//std::cout << "\nProblem here on entry " << i;	
					return TVector3(-1,-1,-2);				
					//break;
				}
			}
		}
		xMean /= xEnergyTotal; //divide the weighted sum by the total energy of the hits in that grid
	} else { //if we have multiplicity 1 event, the mean will just be the value of the segment that was the multiplicity 1 event.
		for(unsigned int i = 0; i<xGridEnergy.size(); i++){
			if(!xGridEnergy[i]) xMean = xmm[i]; //
		}
	}

	Double_t yMean = 0.0; //keeping track of the weighted mean
	Double_t yEnergyTotal = 0.0; //keeping track of the total energy

	if(yMulti > 1){ //we only have to loop through the x-hits if we have more than 1. 
					 //it's possible that this will get caught if we had a single x-hit, but it was on segment 0 somehow. Have to think about this more

		bool bStarted = false; //this will be used to see if we have started counting the hit

		//go through the two grids and do a weighted average of the non-zero points in them
		for(unsigned int i = 0; i < yGridEnergy.size(); i++){
			yEnergyTotal += yGridEnergy[i]; //add the energy of the hit to the total.

			if(!bStarted){//if we haven't hit a non-zero point yet, check to see if the current one is non-zer
				if(yGridEnergy[i]){ //if it is, we've hit the first segment with an energy from the hit. 
					yMean += ymm[i]*yGridEnergy[i]; //add the weighted mean
					bStarted = true; //signal that we have started averaging
				}
			} else if(yGridEnergy[i]){//since we've started averaging, now we need to check for continuity. If adjacent segments that aren't on the end don't both have energy, then 
				//this is a discontinuous hit. Ex: we want vectors that look like [0,0,4,6,8,7,5,0,0], not [0,0,4,6,0,4,2,0,0]
				if (yGridEnergy[i-1]){
					yMean += ymm[i]*yGridEnergy[i]; //add the weighted mean		
				} else {
					//if we have a discontinuous hit, don't bother to try to reconstruct the position
					//std::cout << "\nProblem here on entry " << i;	
					return TVector3(-1,-1,-2);				
					//break;
				}
			}
		}
		yMean /= yEnergyTotal;
	} else { //if we have multiplicity 1 event, the mean will just be the value of the segment that was the multiplicity 1 event.
		for(unsigned int i = 0; i<yGridEnergy.size(); i++){
			if(!yGridEnergy[i]) yMean = ymm[i]; //
		}
	}

	/*
	std::cout << "\nTest print here\n";
	for (int i = 0; i < xGrid.size(); i++){
		std::cout << "\nI is " << i << " and value is " << xGrid[i];
	}*/

	//convert them into cartesion coordinates 
	double yCart = yMean*TMath::Sin(angle); //shifts from grid coordinates to XYZ coordinates
	double zCart = initialSpacingCart + spacingCart*gridY+yMean*TMath::Cos(angle); //add the initial distance from the window to the grid, plus the number of gaps between the initial grid and the Y grid, plus the extra z-amount due to the hit location

	double tanX = xMean/initialSpacingCart + spacingCart*gridX; //determine tangent of the angle in the XY plane
	double tanY = yCart/zCart; //tan of angle in YZ plane

	TVector3 particle = TVector3(tanX,tanY,1); //unnormalized "unit" vector of the particle's trajectory

	//double ratioZX = 1./normalGridVec.Dot(particle.Unit()); //the Z->R corretion factor
	
	fflush(stdout);

	return particle;
	
	//return TVector3(xMean, yMean, -5);
}

Int_t TTrific::GetRange()
{
	//Gets the last grid with an event in it
	//can't just use the size of fHits because there is no guarantee that every
	//grid gets an event, plus the XY position grids may have multiplicity>1
	Int_t range = 0;
	for(unsigned i = 0; i < fHits.size(); i++){
	//for (auto i = 0; i < GetMultiplicity(); i++){
		//if (static_cast<TTrificHit*>(GetHit(i))->GetDetector() > range) range = static_cast<TTrificHit*>(GetHit(i))->GetDetector();
		if(GetTrificHit(i)->GetDetector() > range) range = GetTrificHit(i)->GetDetector();			
	}
	
	return range;
}
