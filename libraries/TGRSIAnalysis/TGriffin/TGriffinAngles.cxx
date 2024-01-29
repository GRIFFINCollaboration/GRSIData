#include "TGriffinAngles.h"
#include "TGriffin.h"

ClassImp(TGriffinAngles)

TGriffinAngles::TGriffinAngles(double distance, bool folding, bool grouping, bool addback)
	: fAngles([this](const double& lhs, const double& rhs) { return std::abs(lhs-rhs) < fRounding; })
{
   // loop over all possible detector/crystal combinations
   for(int firstDet = 1; firstDet <= 16; ++firstDet) {
      for(int firstCry = 0; firstCry < 4; ++firstCry) {
         for(int secondDet = 1; secondDet <= 16; ++secondDet) {
            for(int secondCry = 0; secondCry < 4; ++secondCry) {
               // exclude hits in the same crystal or, if addback is enabled, in the same detector
               if(firstDet == secondDet && (firstCry == secondCry || addback)) continue;
               double angle = TGriffin::GetPosition(firstDet, firstCry, distance).Angle(TGriffin::GetPosition(secondDet, secondCry, distance)) *180./TMath::Pi();
               // if folding is enable we fold the distribution at 90 degree and only use angles between 0 and 90 degree
               if(folding && angle > 90.) {
                  angle = 180. - angle;
               }
               fAngles.insert(angle);
            }
         }
      }
   }

	// create map of indices before we group so that we have an index for each (folded) angle
	for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
		fAngleMap.insert(std::make_pair(*it, std::distance(fAngles.begin(), it)));
	}

   if(grouping) {
      // If grouping is enable we group angles that are close to each other together.
      // Which angles are grouped is somewhat arbitrary, this grouping was chosen to get similar numbers of detector combinations and thus statistics for each angle group.
      // Due to the way lower_bound works, we use the highest angle of each group as the angle of that group.
      // This is just for the purpose of this algorithm, when plotting the correct average angle of the group should be used!
		std::set<double, std::function<bool(const double&, const double&)>> groupedAngles([this](const double& lhs, const double& rhs) { return std::abs(lhs-rhs) < fRounding; });
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
				fAngleMap[*it] -= (i-2)/3*2+2;
            ++it;
				// 10->5, 13->6, 16->7, ...
				fAngleMap[*it] -= (i-2)/3*2+2;
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

	return (it != fAngleMap.end()) ? it->second : -1;
}

void TGriffinAngles::Print() const
{
	std::cout<<"List of unique angles:"<<std::endl;
	for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
		std::cout<<std::distance(fAngles.begin(), it)<<": "<<*it<<std::endl;//<<" - index "<<Index(*it)<<std::endl;
	}
	std::cout<<"Map from angles to indices:"<<std::endl;
	for(auto const& it : fAngleMap) {
		std::cout<<"angle "<<it.first<<", index "<<it.second<<std::endl;
	}
}
