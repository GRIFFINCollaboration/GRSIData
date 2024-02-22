#include "TGriffinAngles.h"
#include "TGriffin.h"
#include "TGRSIOptions.h"

ClassImp(TGriffinAngles)

TGriffinAngles::TGriffinAngles(double distance, bool folding, bool grouping, bool addback)
	: fDistance(distance), fFolding(folding), fGrouping(grouping), fAddback(addback)
{
	SetName("GriffinAngles");
	// get user settings for excluded detectors/crystals
	if(TGRSIOptions::Get() != nullptr) {
		auto settings = TGRSIOptions::Get()->UserSettings();
		if(settings != nullptr) {
			fExcludedCrystals = settings->GetIntVector("ExcludedCrystal");
			fExcludedDetectors = settings->GetIntVector("ExcludedDetector");
		} else {
			std::cout<<"Failed to find user settings in TGRSIOptions, can't get user settings for excluded detectors/crystals"<<std::endl;
		}
	} else {
		std::cout<<"Failed to find TGRSIOptions, can't get user settings for excluded detectors/crystals"<<std::endl;
	}
	
   // loop over all possible detector/crystal combinations
   for(int firstDet = 1; firstDet <= 16; ++firstDet) {
		if(ExcludeDetector(firstDet)) continue;
      for(int firstCry = 0; firstCry < 4; ++firstCry) {
			if(ExcludeCrystal(firstDet, firstCry)) continue;
         for(int secondDet = 1; secondDet <= 16; ++secondDet) {
				if(ExcludeDetector(secondDet)) continue;
            for(int secondCry = 0; secondCry < 4; ++secondCry) {
					if(ExcludeCrystal(secondDet, secondCry)) continue;
               // exclude hits in the same crystal or, if addback is enabled, in the same detector
               if(firstDet == secondDet && (firstCry == secondCry || fAddback)) continue;
               double angle = TGriffin::GetPosition(firstDet, firstCry, fDistance).Angle(TGriffin::GetPosition(secondDet, secondCry, fDistance)) *180./TMath::Pi();
               // if folding is enable we fold the distribution at 90 degree and only use angles between 0 and 90 degree
               if(fFolding && angle > 90.) {
                  angle = 180. - angle;
               }
               if(fAngles.lower_bound(angle-fRounding) == fAngles.upper_bound(angle+fRounding)) {
						fAngles.insert(angle);
					}
            }
         }
      }
   }

	// create map of indices before we group so that we have an index for each (folded) angle
	for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
		fAngleMap.insert(std::make_pair(*it, std::distance(fAngles.begin(), it)));
	}

   if(fGrouping) {
      // If grouping is enable we group angles that are close to each other together.
      // Which angles are grouped is somewhat arbitrary, this grouping was chosen to get similar numbers of detector combinations and thus statistics for each angle group.
      // Due to the way lower_bound works, we use the highest angle of each group as the angle of that group.
      // This is just for the purpose of this algorithm, when plotting the correct average angle of the group should be used!
		std::set<double> groupedAngles;
		// for the angle map we need to update the indices: 3->2, 4->3, 5->3, 6->4, 7->4, 8->5, 9->5, 10->5, 11->6, 12->6, 13->6 and so on
      for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
			auto i = std::distance(fAngles.begin(), it);
         switch(i) {
         case 0:
         case 1:
            // first and second angle are not grouped
            groupedAngles.insert(*it);
            break;
         case 2:
         case 4:
         case 6:
            // three groups of two angles each
				// 2->2, 4->3, 6->4
				fAngleMap[*it] -= (i-2)/2;
            ++it;
				// 3->2, 5->3, 7->4
				fAngleMap[*it] -= (i-2)/2+1;
            groupedAngles.insert(*it);
            break;
         default:
            // all others are groups of three
				//  8->5, 11->6, 14->7, ...
				fAngleMap[*it] -= (i-5)/3*2+1;
            ++it;
				//  9->5, 12->6, 15->7, ...
				fAngleMap[*it] -= (i-5)/3*2+2;
            ++it;
				// 10->5, 13->6, 16->7, ...
				fAngleMap[*it] -= (i-5)/3*2+3;
            groupedAngles.insert(*it);
            break;
         }
      }
      fAngles = groupedAngles;
   }
}

int TGriffinAngles::Index(double angle)
{
	auto matches = [angle, this](std::pair<double, int> val) { return std::abs(val.first-angle) < fRounding; };
	auto it = std::find_if(fAngleMap.begin(), fAngleMap.end(), matches);

	if(it == fAngleMap.end()) {
		std::cout<<"Failed to find angle "<<angle<<" in map!"<<std::endl;
		return -1;
	}
	if(it->second >= static_cast<int>(fAngles.size())) {
		std::cout<<"Found index "<<it->second<<" for angle "<<angle<<" which is outside range ("<<fAngles.size()<<")"<<std::endl;
		return -1;
	}
	return it->second;
}

double TGriffinAngles::AverageAngle(int index) const
{
	double result = 0.;
	int nofMatches = 0;
	for(const auto& val : fAngleMap) {
		if(val.second == index) {
			result += val.first;
			++nofMatches;
		}
	}
	return result/nofMatches;
}

void TGriffinAngles::FoldOrGroup(TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4, bool verbose) const 
{
	/// Apply folding and/or grouping to the theory graphs.
	/// This assumes that the theory graphs all have the exact same length of 49 or 51 for singles or addback, respectively.

	// these are simulated data, so if we add points together we take their average as the new value and change the uncertainties accordingly
	if(verbose) {
		std::cout<<"-------------------- original --------------------"<<std::endl;
		for(int i = 0; i < z0->GetN(); ++i) {
			std::cout<<std::setw(8)<<z0->GetPointX(i)<<": "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
		}
		std::cout<<"--------------------------------------------------"<<std::endl;
	}

	// folding first
	if(fFolding) {
		// we first change the angles of the data points, then we re-sort the graphs, and finally we combine points for the same angle into one
		auto angle0 = z0->GetX();
		auto angle2 = z2->GetX();
		auto angle4 = z4->GetX();
		for(int i = 0; i < z0->GetN(); ++i) {
			if(angle0[i] > 90.) {
				if(verbose) {
					std::cout<<i<<": Folding from "<<std::setw(8)<<z0->GetPointX(i)<<" "<<std::setw(8)<<z2->GetPointX(i)<<" "<<std::setw(8)<<z4->GetPointX(i);
				}
				angle0[i] = 180. - angle0[i];
				angle2[i] = angle0[i];
				angle4[i] = angle0[i];
				if(verbose) {
					std::cout<<" to "<<std::setw(8)<<z0->GetPointX(i)<<" "<<std::setw(8)<<z2->GetPointX(i)<<" "<<std::setw(8)<<z4->GetPointX(i)<<std::endl;
				}
			}
		}
		z0->Sort();
		z2->Sort();
		z4->Sort();
		if(verbose) {
			std::cout<<"-------------------- sorted --------------------"<<std::endl;
			for(int i = 0; i < z0->GetN(); ++i) {
				std::cout<<std::setw(8)<<z0->GetPointX(i)<<": "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
			}
			std::cout<<"------------------------------------------------"<<std::endl;
		}
		angle0 = z0->GetX();
		auto counts0 = z0->GetY(); auto errors0 = z0->GetEY();
		auto counts2 = z2->GetY(); auto errors2 = z2->GetEY();
		auto counts4 = z4->GetY(); auto errors4 = z4->GetEY();
		for(int i = 1; i < z0->GetN(); ++i) {
			if(std::abs(angle0[i] - angle0[i-1]) < fRounding) {
				if(verbose) {
					std::cout<<i<<": Adding angles "<<angle0[i-1]<<" and "<<angle0[i]<<std::endl;
				}
				// add counts from this point to the previous point, then delete this point, and update our pointers to the data
				counts0[i-1] += counts0[i]; counts0[i-1] /= 2.; errors0[i-1] = TMath::Sqrt(TMath::Power(errors0[i-1], 2) + TMath::Power(errors0[i], 2))/2.;
				counts2[i-1] += counts2[i]; counts2[i-1] /= 2.; errors2[i-1] = TMath::Sqrt(TMath::Power(errors2[i-1], 2) + TMath::Power(errors2[i], 2))/2.;
				counts4[i-1] += counts4[i]; counts4[i-1] /= 2.; errors4[i-1] = TMath::Sqrt(TMath::Power(errors4[i-1], 2) + TMath::Power(errors4[i], 2))/2.;
				z0->RemovePoint(i);
				z2->RemovePoint(i);
				z4->RemovePoint(i);
				angle0 = z0->GetX();
				counts0 = z0->GetY(); errors0 = z0->GetEY();
				counts2 = z2->GetY(); errors2 = z2->GetEY();
				counts4 = z4->GetY(); errors4 = z4->GetEY();
				--i; // decrement to re-check the new ith point
			} else {
				if(verbose) {
					std::cout<<i<<": Not adding angles "<<angle0[i-1]<<" and "<<angle0[i]<<std::endl;
				}
			}
		}
	}
	if(verbose) {
		std::cout<<"-------------------- after folding --------------------"<<std::endl;
		for(int i = 0; i < z0->GetN(); ++i) {
			std::cout<<std::setw(8)<<z0->GetPointX(i)<<": "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
		}
		std::cout<<"-------------------------------------------------------"<<std::endl;
	}

	// grouping
	if(fGrouping) {
		// Due to the way lower_bound works, we use the highest angle of each group as the angle of that group.
		// This is just for the purpose of this algorithm, when plotting the correct average angle of the group should be used!
		auto counts0 = z0->GetY(); auto errors0 = z0->GetEY();
		auto counts2 = z2->GetY(); auto errors2 = z2->GetEY();
		auto counts4 = z4->GetY(); auto errors4 = z4->GetEY();
		for(int i = 0; i < z0->GetN(); ++i) {
			switch(i) {
				case 0:
				case 1: // first and second angle are not grouped
					if(verbose) {
						std::cout<<i<<": Leaving as is "<<z0->GetPointX(i)<<": "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
					}
					break;
				case 2:
				case 3:
				case 4:
					// three groups of two angles each, so we add the this point to the next one, delete it, and update the pointers
					// because we delete the point, we don't skip cases here like when we create the angles above
					if(verbose) {
						std::cout<<i<<": Grouping "<<z0->GetPointX(i)<<" and "<<z0->GetPointX(i+1)<<" from "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i);
					}
					counts0[i+1] += counts0[i]; counts0[i+1] /= 2.; errors0[i+1] = TMath::Sqrt(TMath::Power(errors0[i], 2) + TMath::Power(errors0[i+1], 2))/2.;
					counts2[i+1] += counts2[i]; counts2[i+1] /= 2.; errors2[i+1] = TMath::Sqrt(TMath::Power(errors2[i], 2) + TMath::Power(errors2[i+1], 2))/2.;
					counts4[i+1] += counts4[i]; counts4[i+1] /= 2.; errors4[i+1] = TMath::Sqrt(TMath::Power(errors4[i], 2) + TMath::Power(errors4[i+1], 2))/2.;
					z0->RemovePoint(i);
					z2->RemovePoint(i);
					z4->RemovePoint(i);
					counts0 = z0->GetY(); errors0 = z0->GetEY();
					counts2 = z2->GetY(); errors2 = z2->GetEY();
					counts4 = z4->GetY(); errors4 = z4->GetEY();
					if(verbose) {
						std::cout<<" to "<<z0->GetPointX(i)<<" with "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
					}
					// no need to decrement the index, by removing one point the old i+1 became i and we don't want to process that one (we just added to it)
					break;
				default:
					// all others are groups of three, so we add this point and the next one to the one two ahead, delete them, and update the pointers
					if(verbose) {
						std::cout<<i<<": Grouping "<<z0->GetPointX(i)<<", "<<z0->GetPointX(i+1)<<", and "<<z0->GetPointX(i+2)<<" from "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i);
					}
					counts0[i+2] += counts0[i] + counts0[i+1]; counts0[i+2] /= 3.; errors0[i+2] = TMath::Sqrt(TMath::Power(errors0[i], 2) + TMath::Power(errors0[i+1], 2) + TMath::Power(errors0[i+2], 2))/3.;
					counts2[i+2] += counts2[i] + counts2[i+1]; counts2[i+2] /= 3.; errors2[i+2] = TMath::Sqrt(TMath::Power(errors2[i], 2) + TMath::Power(errors2[i+1], 2) + TMath::Power(errors0[i+2], 2))/3.;
					counts4[i+2] += counts4[i] + counts4[i+1]; counts4[i+2] /= 3.; errors4[i+2] = TMath::Sqrt(TMath::Power(errors4[i], 2) + TMath::Power(errors4[i+1], 2) + TMath::Power(errors0[i+2], 2))/3.;
					z0->RemovePoint(i);
					z2->RemovePoint(i);
					z4->RemovePoint(i);
					z0->RemovePoint(i+1);
					z2->RemovePoint(i+1);
					z4->RemovePoint(i+1);
					counts0 = z0->GetY(); errors0 = z0->GetEY();
					counts2 = z2->GetY(); errors2 = z2->GetEY();
					counts4 = z4->GetY(); errors4 = z4->GetEY();
					if(verbose) {
						std::cout<<" to "<<z0->GetPointX(i)<<" with "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
					}
					// no need to decrement the index, by removing two points the old i+2 became i and we don't want to processs that one since we just added to it
					break;
			}
		}
	}
	if(verbose) {
		std::cout<<"-------------------- after folding and grouping --------------------"<<std::endl;
		for(int i = 0; i < z0->GetN(); ++i) {
			std::cout<<std::setw(8)<<z0->GetPointX(i)<<": "<<std::setw(8)<<z0->GetPointY(i)<<" "<<std::setw(8)<<z2->GetPointY(i)<<" "<<std::setw(8)<<z4->GetPointY(i)<<std::endl;
		}
		std::cout<<"--------------------------------------------------------------------"<<std::endl;
	}
}

bool TGriffinAngles::ExcludeDetector(int detector) const
{
	for(auto exclude : fExcludedDetectors) {
		if(detector == exclude) return true;
	}
	return false;
}

bool TGriffinAngles::ExcludeCrystal(int detector, int crystal) const
{
	for(auto exclude : fExcludedCrystals) {
		if(4*(detector-1)+crystal+1 == exclude) return true;
	}
	return false;
}

void TGriffinAngles::Print(Option_t*) const
{
	std::cout<<"List of unique angles:"<<std::endl;
	for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
		std::cout<<std::distance(fAngles.begin(), it)<<": "<<*it<<std::endl;//<<" - index "<<Index(*it)<<std::endl;
	}
	std::cout<<"Map from angles to indices:"<<std::endl;
	int i = 0;
	for(auto const& it : fAngleMap) {
		std::cout<<i<<": angle "<<it.first<<", index "<<it.second<<" - average angle "<<AverageAngle(it.second)<<std::endl;
		++i;
	}
}
