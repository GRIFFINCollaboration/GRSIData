#include "TGriffinAngles.h"
#include "TGriffin.h"
#include "TGRSIOptions.h"

double     TGriffinAngles::fRounding  = 0.001;
EVerbosity TGriffinAngles::fVerbosity = EVerbosity::kQuiet;

TGriffinAngles::TGriffinAngles(double distance, bool folding, bool grouping, bool addback)
   : fDistance(distance), fFolding(folding), fGrouping(grouping), fAddback(addback)
{
   SetName("GriffinAngles");
   // get user settings for excluded detectors/crystals, and custom grouping
   if(TGRSIOptions::Get() != nullptr) {
      auto* settings = TGRSIOptions::UserSettings();
      if(settings != nullptr) {
         // try quietly to get the vectors of excluded crystals and detectors, catching (and disregarding) any exceptions
         try {
            fExcludedCrystals = settings->GetIntVector("ExcludedCrystal", true);
         } catch(std::out_of_range&) {}
         try {
            fExcludedDetectors = settings->GetIntVector("ExcludedDetector", true);
         } catch(std::out_of_range&) {}
         try {
            fCustomGrouping = settings->GetIntVector("CustomGrouping", true);
            std::sort(fCustomGrouping.begin(), fCustomGrouping.end());
         } catch(std::out_of_range&) {}
      } else if(fVerbosity > EVerbosity::kQuiet) {
         std::cout << "Failed to find user settings in TGRSIOptions, can't get user settings for excluded detectors/crystals" << std::endl;
      }
   } else if(fVerbosity > EVerbosity::kQuiet) {
      std::cout << "Failed to find TGRSIOptions, can't get user settings for excluded detectors/crystals" << std::endl;
   }

   // check that we only use grouping if folding is also enabled, and we do not have any custom grouping information
   if(fGrouping && !fFolding && fCustomGrouping.empty()) {
      std::cout << DRED << "Warning, grouping is only possible if folding is also active. Setting folding to true!" << RESET_COLOR << std::endl;
      fFolding = true;
   }

   if(fVerbosity > EVerbosity::kQuiet) {
      std::cout << "Creating angles for detectors " << fDistance << " mm from center of array, " << (fAddback ? "" : "not ") << "using addback, " << (fFolding ? "" : "not ") << "using folding, and " << (fGrouping ? "" : "not ") << "using grouping. Any angles less than " << fRounding << " degrees apart will be considered the same." << std::endl;
   }

   // loop over all possible detector/crystal combinations
   for(int firstDet = 1; firstDet <= 16; ++firstDet) {
      if(ExcludeDetector(firstDet)) { continue; }
      for(int firstCry = 0; firstCry < 4; ++firstCry) {
         if(ExcludeCrystal(firstDet, firstCry)) { continue; }
         for(int secondDet = 1; secondDet <= 16; ++secondDet) {
            if(ExcludeDetector(secondDet)) { continue; }
            for(int secondCry = 0; secondCry < 4; ++secondCry) {
               if(ExcludeCrystal(secondDet, secondCry)) { continue; }
               // exclude hits in the same crystal or, if addback is enabled, in the same detector
               if(firstDet == secondDet && (firstCry == secondCry || fAddback)) { continue; }
               double angle = TGriffin::GetPosition(firstDet, firstCry, fDistance).Angle(TGriffin::GetPosition(secondDet, secondCry, fDistance)) * 180. / TMath::Pi();
               if(fVerbosity == kAll) {
                  std::cout << "det./cry. " << firstDet << "/" << firstCry << " with  " << secondDet << "/" << secondCry << ", at " << fDistance << " mm = " << angle << std::endl;
               }
               // if folding is enable we fold the distribution at 90 degree and only use angles between 0 and 90 degree
               if(fFolding && angle > 90.) {
                  angle = 180. - angle;
               }
               // round down to zero (mainly here for nicer looks)
               if(angle < fRounding) {
                  angle = 0.;
               }
               // if the lower bound and the upper bound are the same we have a new angle
               if(fAngles.lower_bound(angle - fRounding) == fAngles.upper_bound(angle + fRounding)) {
                  fAngles.insert(angle);
               }
               // this should always work, either this is a new angle, in which case it gets initialized to zero and then incremented to one,
               // or we increment the existing counter
               // the key is integer, so by dividing by rounding and then casting to integer we can avoid duplicates close to each other
               // factor 2 to include that the "normal" rounding is +- fRounding
               fAngleCount[static_cast<int>(std::round(angle / fRounding))]++;
               if(fVerbosity == kAll) {
                  std::cout << "after folding and rounding: angle " << angle << ", counts " << fAngleCount[static_cast<int>(std::round(angle / fRounding))] << std::endl;
               }
            }   // second crystal loop
         }   //second detector loop
      }   // first crystal loop
   }   // first detector loop

   // create map of indices before we group so that we have an index for each (folded) angle
   for(auto it = fAngles.begin(); it != fAngles.end(); ++it) {
      fAngleMap.insert(std::make_pair(*it, std::distance(fAngles.begin(), it)));
   }

   if(fGrouping) {
      // If grouping is enable we group angles that are close to each other together.
      // Which angles are grouped is somewhat arbitrary, this grouping was chosen to get similar numbers of detector combinations and thus statistics for each angle group.
      // Due to the way lower_bound works, we use the highest angle of each group as the angle of that group.
      // This is just for the purpose of this algorithm, when plotting the correct average angle of the group should be used!

      // check if we have custom grouping supplied and its format
      if(!fCustomGrouping.empty()) {
         if(fCustomGrouping.size() != fAngles.size()) {
            std::ostringstream str;
            str << DRED << "Custom grouping with " << fCustomGrouping.size() << " entries supplied, but we have " << fAngles.size() << " angles!" << RESET_COLOR;
            throw std::runtime_error(str.str());
         }
         if(fCustomGrouping[0] != 0) {
            std::ostringstream str;
            str << DRED << "Custom grouping with " << fCustomGrouping.size() << " entries supplied, but the first entry is " << fCustomGrouping[0] << ", not 0!" << RESET_COLOR;
            throw std::runtime_error(str.str());
         }
         if(!std::is_sorted(fCustomGrouping.begin(), fCustomGrouping.end())) {
            std::ostringstream str;
            str << DRED << "Custom grouping with " << fCustomGrouping.size() << " entries is not sorted!" << RESET_COLOR;
            throw std::runtime_error(str.str());
         }
         auto tmp = fCustomGrouping;
         auto end = std::unique(tmp.begin(), tmp.end());
         if(fCustomGrouping.back() != end - tmp.begin() - 1) {
            std::ostringstream str;
            str << DRED << "Custom grouping with " << fCustomGrouping.size() << " entries supplied, but the last entry is " << fCustomGrouping[0] << " which does not match the number of unique groups minus one (" << end - tmp.begin() - 1 << ")!" << RESET_COLOR;
            throw std::runtime_error(str.str());
         }
      } else if(fDistance == 110.) {
         fCustomGrouping = {0, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10};
      } else if(fDistance == 145.) {
         fCustomGrouping = {0, 1, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11};
      } else {
         std::ostringstream str;
         str << DRED << "No custom grouping supplied and distance is not 110 mm or 145 mm, but " << fDistance << "!" << RESET_COLOR;
         throw std::runtime_error(str.str());
      }

      // now we can loop over the grouping vector and the angles and store the largest angle per group in the new groupedAngles vector
      // we also update our angle map with the new indices
      std::set<double> groupedAngles;
      auto             angle         = fAngles.begin();
      double           previousAngle = *angle;
      for(size_t i = 0; i < fCustomGrouping.size(); ++i, ++angle) {
         if(i != 0 && fCustomGrouping[i] != fCustomGrouping[i - 1]) {   // we found a new group so we insert the previous angle
            groupedAngles.insert(previousAngle);
         }
         fAngleMap[*angle] = fCustomGrouping[i];
         previousAngle     = *angle;
      }
      // add the last angle since we never get to the next group after the last
      groupedAngles.insert(previousAngle);

      // update the angles to the new grouped angles
      fAngles = groupedAngles;
   }
}

int TGriffinAngles::Index(double angle)
{
   auto matches = [angle, this](std::pair<double, int> val) { return std::abs(val.first - angle) < fRounding; };
   auto it      = std::find_if(fAngleMap.begin(), fAngleMap.end(), matches);

   if(it == fAngleMap.end()) {
      std::cout << "Failed to find angle " << angle << " in map!" << std::endl;
      return -1;
   }
   if(it->second >= static_cast<int>(fAngles.size())) {
      std::cout << "Found index " << it->second << " for angle " << angle << " which is outside range (" << fAngles.size() << ")" << std::endl;
      return -1;
   }
   return it->second;
}

double TGriffinAngles::AverageAngle(int index) const
{
   double result     = 0.;
   int    nofMatches = 0;
   for(const auto& val : fAngleMap) {
      if(val.second == index) {
         result += val.first;
         ++nofMatches;
      }
   }
   return result / nofMatches;
}

void TGriffinAngles::FoldOrGroup(TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4, bool verbose) const
{
   /// Apply folding and/or grouping to the theory graphs.
   /// This assumes that the theory graphs all have the exact same length of 49 or 51 for singles or addback, respectively.

   // these are simulated data, so if we add points together we take their average as the new value and change the uncertainties accordingly
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
   if(verbose) {
      std::cout << "-------------------- original --------------------" << std::endl;
      for(int i = 0; i < z0->GetN(); ++i) {
         std::cout << std::setw(8) << z0->GetPointX(i) << ": " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
      }
      std::cout << "--------------------------------------------------" << std::endl;
   }
#endif

   // folding first
   if(fFolding) {
      // we first change the angles of the data points, then we re-sort the graphs, and finally we combine points for the same angle into one
      auto* angle0 = z0->GetX();
      auto* angle2 = z2->GetX();
      auto* angle4 = z4->GetX();
      for(int i = 0; i < z0->GetN(); ++i) {
         if(angle0[i] > 90.) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << i << ": Folding from " << std::setw(8) << z0->GetPointX(i) << " " << std::setw(8) << z2->GetPointX(i) << " " << std::setw(8) << z4->GetPointX(i);
            }
#endif
            angle0[i] = 180. - angle0[i];
            angle2[i] = angle0[i];
            angle4[i] = angle0[i];
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << " to " << std::setw(8) << z0->GetPointX(i) << " " << std::setw(8) << z2->GetPointX(i) << " " << std::setw(8) << z4->GetPointX(i) << std::endl;
            }
#endif
         }
      }
      z0->Sort();
      z2->Sort();
      z4->Sort();
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
      if(verbose) {
         std::cout << "-------------------- sorted --------------------" << std::endl;
         for(int i = 0; i < z0->GetN(); ++i) {
            std::cout << std::setw(8) << z0->GetPointX(i) << ": " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
         }
         std::cout << "------------------------------------------------" << std::endl;
      }
#endif
      angle0        = z0->GetX();
      auto* counts0 = z0->GetY();
      auto* errors0 = z0->GetEY();
      auto* counts2 = z2->GetY();
      auto* errors2 = z2->GetEY();
      auto* counts4 = z4->GetY();
      auto* errors4 = z4->GetEY();
      for(int i = 1; i < z0->GetN(); ++i) {
         if(std::abs(angle0[i] - angle0[i - 1]) < fRounding) {
            if(verbose) {
               std::cout << i << ": Adding angles " << angle0[i - 1] << " and " << angle0[i] << std::endl;
            }
            // add counts from this point to the previous point, then delete this point, and update our pointers to the data
            counts0[i - 1] += counts0[i];
            counts0[i - 1] /= 2.;
            errors0[i - 1] = TMath::Sqrt(TMath::Power(errors0[i - 1], 2) + TMath::Power(errors0[i], 2)) / 2.;
            counts2[i - 1] += counts2[i];
            counts2[i - 1] /= 2.;
            errors2[i - 1] = TMath::Sqrt(TMath::Power(errors2[i - 1], 2) + TMath::Power(errors2[i], 2)) / 2.;
            counts4[i - 1] += counts4[i];
            counts4[i - 1] /= 2.;
            errors4[i - 1] = TMath::Sqrt(TMath::Power(errors4[i - 1], 2) + TMath::Power(errors4[i], 2)) / 2.;
            z0->RemovePoint(i);
            z2->RemovePoint(i);
            z4->RemovePoint(i);
            angle0  = z0->GetX();
            counts0 = z0->GetY();
            errors0 = z0->GetEY();
            counts2 = z2->GetY();
            errors2 = z2->GetEY();
            counts4 = z4->GetY();
            errors4 = z4->GetEY();
            --i;   // decrement to re-check the new ith point
         } else {
            if(verbose) {
               std::cout << i << ": Not adding angles " << angle0[i - 1] << " and " << angle0[i] << std::endl;
            }
         }
      }
   }
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
   if(verbose) {
      std::cout << "-------------------- after folding --------------------" << std::endl;
      for(int i = 0; i < z0->GetN(); ++i) {
         std::cout << std::setw(8) << z0->GetPointX(i) << ": " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
      }
      std::cout << "-------------------------------------------------------" << std::endl;
   }
#endif

   // grouping
   if(fGrouping) {
      // Due to the way lower_bound works, we use the highest angle of each group as the angle of that group.
      // This is just for the purpose of this algorithm, when plotting the correct average angle of the group should be used!
      auto* counts0 = z0->GetY();
      auto* errors0 = z0->GetEY();
      auto* counts2 = z2->GetY();
      auto* errors2 = z2->GetEY();
      auto* counts4 = z4->GetY();
      auto* errors4 = z4->GetEY();
      for(int i = 0; i < z0->GetN(); ++i) {
         switch(i) {
         case 0:
         case 1:   // first and second angle are not grouped
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << i << ": Leaving as is " << z0->GetPointX(i) << ": " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
            }
#endif
            break;
         case 2:
         case 3:
         case 4:
            // three groups of two angles each, so we add the this point to the next one, delete it, and update the pointers
            // because we delete the point, we don't skip cases here like when we create the angles above
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << i << ": Grouping " << z0->GetPointX(i) << " and " << z0->GetPointX(i + 1) << " from " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i);
            }
#endif
            counts0[i + 1] += counts0[i];
            counts0[i + 1] /= 2.;
            errors0[i + 1] = TMath::Sqrt(TMath::Power(errors0[i], 2) + TMath::Power(errors0[i + 1], 2)) / 2.;
            counts2[i + 1] += counts2[i];
            counts2[i + 1] /= 2.;
            errors2[i + 1] = TMath::Sqrt(TMath::Power(errors2[i], 2) + TMath::Power(errors2[i + 1], 2)) / 2.;
            counts4[i + 1] += counts4[i];
            counts4[i + 1] /= 2.;
            errors4[i + 1] = TMath::Sqrt(TMath::Power(errors4[i], 2) + TMath::Power(errors4[i + 1], 2)) / 2.;
            z0->RemovePoint(i);
            z2->RemovePoint(i);
            z4->RemovePoint(i);
            counts0 = z0->GetY();
            errors0 = z0->GetEY();
            counts2 = z2->GetY();
            errors2 = z2->GetEY();
            counts4 = z4->GetY();
            errors4 = z4->GetEY();
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << " to " << z0->GetPointX(i) << " with " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
            }
#endif
            // no need to decrement the index, by removing one point the old i+1 became i and we don't want to process that one (we just added to it)
            break;
         default:
            // all others are groups of three, so we add this point and the next one to the one two ahead, delete them, and update the pointers
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << i << ": Grouping " << z0->GetPointX(i) << ", " << z0->GetPointX(i + 1) << ", and " << z0->GetPointX(i + 2) << " from " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i);
            }
#endif
            counts0[i + 2] += counts0[i] + counts0[i + 1];
            counts0[i + 2] /= 3.;
            errors0[i + 2] = TMath::Sqrt(TMath::Power(errors0[i], 2) + TMath::Power(errors0[i + 1], 2) + TMath::Power(errors0[i + 2], 2)) / 3.;
            counts2[i + 2] += counts2[i] + counts2[i + 1];
            counts2[i + 2] /= 3.;
            errors2[i + 2] = TMath::Sqrt(TMath::Power(errors2[i], 2) + TMath::Power(errors2[i + 1], 2) + TMath::Power(errors0[i + 2], 2)) / 3.;
            counts4[i + 2] += counts4[i] + counts4[i + 1];
            counts4[i + 2] /= 3.;
            errors4[i + 2] = TMath::Sqrt(TMath::Power(errors4[i], 2) + TMath::Power(errors4[i + 1], 2) + TMath::Power(errors0[i + 2], 2)) / 3.;
            z0->RemovePoint(i);
            z2->RemovePoint(i);
            z4->RemovePoint(i);
            z0->RemovePoint(i + 1);
            z2->RemovePoint(i + 1);
            z4->RemovePoint(i + 1);
            counts0 = z0->GetY();
            errors0 = z0->GetEY();
            counts2 = z2->GetY();
            errors2 = z2->GetEY();
            counts4 = z4->GetY();
            errors4 = z4->GetEY();
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
            if(verbose) {
               std::cout << " to " << z0->GetPointX(i) << " with " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
            }
#endif
            // no need to decrement the index, by removing two points the old i+2 became i and we don't want to processs that one since we just added to it
            break;
         }
      }
   }
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 20, 0)
   if(verbose) {
      std::cout << "-------------------- after folding and grouping --------------------" << std::endl;
      for(int i = 0; i < z0->GetN(); ++i) {
         std::cout << std::setw(8) << z0->GetPointX(i) << ": " << std::setw(8) << z0->GetPointY(i) << " " << std::setw(8) << z2->GetPointY(i) << " " << std::setw(8) << z4->GetPointY(i) << std::endl;
      }
      std::cout << "--------------------------------------------------------------------" << std::endl;
   }
#endif
}

bool TGriffinAngles::ExcludeDetector(int detector) const
{
   /// Returns true if any of the detectors in fExcludedDetectors matches the given detector.
   return std::any_of(fExcludedDetectors.begin(), fExcludedDetectors.end(), [&detector](auto exclude) { return detector == exclude; });
}

bool TGriffinAngles::ExcludeCrystal(int detector, int crystal) const
{
   /// Returns true if any of the crystals in fExcludedCrystals matches the given detector and crystal (using 4*(detector-1)+crystal+1).
   return std::any_of(fExcludedCrystals.begin(), fExcludedCrystals.end(), [&detector, &crystal](auto exclude) { return 4 * (detector - 1) + crystal + 1 == exclude; });
}

void TGriffinAngles::Print(Option_t*) const
{
   std::cout << "List of unique angles " << std::setw(2) << fAngles.size() << " Map from angles to indices " << std::setw(2) << fAngleMap.size() << "   # of combinations " << std::setw(2) << fAngleCount.size() << std::endl;
   //List of unique angles aa Map from angles to indices mm   # of combinations cc"<<std::endl;
   std::cout << "index   angle            angle   index  average angle    angle   counts" << std::endl;
   //ii:     aa.aaaa          aa.aaaa ii     aa.aaaa          aa.aaaa ccc
   auto it  = fAngles.begin();
   auto it2 = fAngleMap.begin();
   auto it3 = fAngleCount.begin();
   while(it != fAngles.end() || it2 != fAngleMap.end() || it3 != fAngleCount.end()) {
      if(it != fAngles.end()) {
         std::cout << std::setw(2) << std::distance(fAngles.begin(), it) << ":     " << std::setw(7) << *it << "          ";
         ++it;
      } else {
         //ii:     aa.aaaa          aa.aaaa ii     aa.aaaa          aa.aaaa ccc
         std::cout << "                         ";
      }
      if(it2 != fAngleMap.end()) {
         std::cout << std::setw(7) << it2->first << " " << std::setw(2) << it2->second << "     " << std::setw(7) << AverageAngle(it2->second) << "          ";
         ++it2;
      } else {
         //aa.aaaa ii     aa.aaaa          aa.aaaa ccc
         std::cout << "                                ";
      }
      if(it3 != fAngleCount.end()) {
         std::cout << std::setw(7) << it3->first * fRounding << " " << std::setw(3) << it3->second;
         ++it3;
      }
      std::cout << std::endl;
   }
}

void TGriffinAngles::Add(TGriffinAngles* griffinAngles)
{
	if(fDistance != griffinAngles->fDistance) {
		std::cerr << "Warning, merging files with different Griffin distances " << fDistance << " != " << griffinAngles->fDistance << std::endl;
	}
	if(fFolding != griffinAngles->fFolding) {
		std::cerr << "Warning, merging files with different folding settings " << ( fFolding ? "true" : "false")  << " != " << ( griffinAngles->fFolding ? "true" : "false")  << std::endl;
	}
	if(fGrouping != griffinAngles->fGrouping) {
		std::cerr << "Warning, merging files with different grouping settings " << ( fGrouping ? "true" : "false")  << " != " << ( griffinAngles->fGrouping ? "true" : "false")  << std::endl;
	}
	if(fAddback != griffinAngles->fAddback) {
		std::cerr << "Warning, merging files with different addback settings " << ( fAddback ? "true" : "false")  << " != " << ( griffinAngles->fAddback ? "true" : "false")  << std::endl;
	}
	if(fRounding != griffinAngles->fRounding) {
		std::cerr << "Warning, merging files with different rounding " << fRounding << " != " << griffinAngles->fRounding << std::endl;
	}
	if(fExcludedDetectors != griffinAngles->fExcludedDetectors) {
		std::cerr << "Warning, merging files with different detectors excluded < ";
		for(auto det : fExcludedDetectors) { std::cerr << det << " "; }
		std::cerr << "> != < ";
		for(auto det : griffinAngles->fExcludedDetectors) { std::cerr << det << " "; }
		std::cerr << ">" << std::endl;
	}
	if(fExcludedCrystals != griffinAngles->fExcludedCrystals) {
		std::cerr << "Warning, merging files with different crystals excluded < ";
		for(auto cry : fExcludedCrystals) { std::cerr << cry << " "; }
		std::cerr << "> != < ";
		for(auto cry : griffinAngles->fExcludedCrystals) { std::cerr << cry << " "; }
		std::cerr << ">" << std::endl;
	}
	if(fCustomGrouping != griffinAngles->fCustomGrouping) {
		std::cerr << "Warning, merging files with different custom grouping < ";
		for(auto group : fCustomGrouping) { std::cerr << group << " "; }
		std::cerr << "> != < ";
		for(auto group : griffinAngles->fCustomGrouping) { std::cerr << group << " "; }
		std::cerr << ">" << std::endl;
	}
	if(fAngles != griffinAngles->fAngles) {
		std::cerr << "Warning, merging files with different angles < ";
		for(auto angle : fAngles) { std::cerr << angle << " "; }
		std::cerr << "> != < ";
		for(auto angle : griffinAngles->fAngles) { std::cerr << angle << " "; }
		std::cerr << ">" << std::endl;
	}
	if(fAngleMap != griffinAngles->fAngleMap) {
		std::cerr << "Warning, merging files with different angle maps < ";
		for(auto angle : fAngleMap) { std::cerr << angle.first << "/" << angle.second << " "; }
		std::cerr << "> != < ";
		for(auto angle : griffinAngles->fAngleMap) { std::cerr << angle.first << "/" << angle.second << " "; }
		std::cerr << ">" << std::endl;
	}
	if(fAngleCount != griffinAngles->fAngleCount) {
		std::cerr << "Warning, merging files with different angle counts < ";
		for(auto angle : fAngleCount) { std::cerr << angle.first << "/" << angle.second << " "; }
		std::cerr << "> != < ";
		for(auto angle : griffinAngles->fAngleCount) { std::cerr << angle.first << "/" << angle.second << " "; }
		std::cerr << ">" << std::endl;
	}
}
