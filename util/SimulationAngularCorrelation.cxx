#include <iostream>
#include <iomanip>

#include "TChain.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH2D.h"

#include "Globals.h"
#include "TRedirect.h"
#include "TUserSettings.h"
#include "TGriffin.h"
#include "TGriffinBgo.h"
#include "TGriffinAngles.h"

int main(int argc, char** argv)
{
   // TODO: add user settings file, make all input parameter available via settings and command line
   bool printUsage = (argc == 1);

   std::array<std::vector<std::string>, 3> inputFilenames;
   std::string                             outputFilename = "SimulatedAngularCorrelation.root";
   double                                  gateEnergy     = 0.;
   double                                  fitEnergy      = 0.;
   bool                                    addback        = false;
   bool                                    singleCrystal  = false;
   double                                  distance       = 145.;
   int                                     bins           = 2000;
   double                                  minEnergy      = 0.;
   double                                  maxEnergy      = 2000.;
   int                                     verboseLevel   = 0;
   std::string                             settingsFile;

   // read command line arguments
   for(int i = 1; i < argc; ++i) {
      if(strncmp(argv[i], "-if", 2) == 0) {
         // check which coefficent this is for
         int coefficent = -1;
         if(strcmp(&argv[i][3], "000") == 0) {
            coefficent = 0;
         } else if(strcmp(&argv[i][3], "010") == 0) {
            coefficent = 1;
         } else if(strcmp(&argv[i][3], "100") == 0) {
            coefficent = 2;
         } else {
            std::cerr << "Unknown -if* command line flag " << argv[i] << ", should be -if000, -if010, or -if100!" << std::endl;
            printUsage = true;
            break;
         }
         // if we have another argument, check if it starts with '-'
         while(i + 1 < argc) {
            if(argv[i + 1][0] == '-') {
               break;
            }
            // if we get here we can add the next argument to the list of file names
            inputFilenames[coefficent].emplace_back(argv[++i]);
         }
      } else if(strcmp(argv[i], "-of") == 0) {
         if(i + 1 < argc) {
            outputFilename = argv[++i];
         } else {
            std::cout << "Error, -of flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-g") == 0) {
         if(i + 1 < argc) {
            gateEnergy = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -g flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-f") == 0) {
         if(i + 1 < argc) {
            fitEnergy = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -f flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-ab") == 0) {
         addback = true;
      } else if(strcmp(argv[i], "-sc") == 0) {
         singleCrystal = true;
      } else if(strcmp(argv[i], "-d") == 0) {
         if(i + 1 < argc) {
            distance = std::atof(argv[++i]);
         } else {
            std::cout << "Error, -d flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-vl") == 0) {
         if(i + 1 < argc) {
            verboseLevel = std::atoi(argv[++i]);
         } else {
            std::cout << "Error, -vl flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else if(strcmp(argv[i], "-sf") == 0) {
         if(i + 1 < argc) {
            settingsFile = argv[++i];
         } else {
            std::cout << "Error, -sf flag needs an argument!" << std::endl;
            printUsage = true;
         }
      } else {
         std::cerr << "Error, unknown flag \"" << argv[i] << "\"!" << std::endl;
         printUsage = true;
      }
   }

   // try to read user settings
   TUserSettings settings;
   if(!settingsFile.empty()) {
      settings.ReadSettings(settingsFile);

      if(inputFilenames[0].empty()) {
         inputFilenames[0] = settings.GetStringVector("Coefficient.000.Files");
      }
      if(inputFilenames[1].empty()) {
         inputFilenames[1] = settings.GetStringVector("Coefficient.010.Files");
      }
      if(inputFilenames[2].empty()) {
         inputFilenames[2] = settings.GetStringVector("Coefficient.100.Files");
      }
      if(outputFilename == "SimulatedAngularCorrelation.root") {
         outputFilename = settings.GetString("OutputFilename", "SimulatedAngularCorrelation.root");
      } else {
         std::cout << "Output filename has already been set to \"" << outputFilename << "\" via command line, ignoring settings file for this." << std::endl;
      }
      if(gateEnergy != 0.) {
         std::cerr << "Warning, already got gate energy " << gateEnergy << ", this could be overwritten by the settings file!" << std::endl;
      }
      gateEnergy = settings.GetDouble("Energy.Gate", gateEnergy);
      if(fitEnergy != 0.) {
         std::cerr << "Warning, already got fit energy " << fitEnergy << ", this could be overwritten by the settings file!" << std::endl;
      }
      fitEnergy = settings.GetDouble("Energy.Fit", fitEnergy);
      // GetBool doesn't have a default version, so catch any errors
      try {
         addback = settings.GetBool("Addback", true);
      } catch(std::out_of_range& e) {}
      try {
         singleCrystal = settings.GetBool("SingleCrystal", true);
      } catch(std::out_of_range& e) {}
      distance     = settings.GetDouble("Distance", distance);
      bins         = settings.GetInt("Bins", bins);
      minEnergy    = settings.GetDouble("Energy.Minimum", minEnergy);
      maxEnergy    = settings.GetDouble("Energy.Minimum", maxEnergy);
      verboseLevel = settings.GetInt("Verbosity", verboseLevel);
   }

   if(!printUsage && (inputFilenames[0].empty() || inputFilenames[1].empty() || inputFilenames[2].empty())) {
      std::cerr << "Error, need input files for all three !" << std::endl;
      printUsage = true;
   }

   if(!printUsage && (gateEnergy <= 0. || fitEnergy <= 0.)) {
      std::cerr << "Error, need energies for the gate (" << gateEnergy << ") and the fit (" << fitEnergy << ")!" << std::endl;
      printUsage = true;
   }

   if(!printUsage && singleCrystal && !addback) {
      std::cerr << "Error, can't have single crystal method without using addback!" << std::endl;
      printUsage = true;
   }

   if(printUsage) {
      std::cerr << "Arguments for " << argv[0] << ":" << std::endl
                << "-if000 <input files>                                (required, geant4 simulation files for parameters 000)" << std::endl
                << "-if010 <input files>                                (required, geant4 simulation files for parameters 010)" << std::endl
                << "-if100 <input files>                                (required, geant4 simulation files for parameters 100)" << std::endl
                << "-g     <gate energy>                                (required)" << std::endl
                << "-f     <fit energy>                                 (required)" << std::endl
                << "-of    <output filename>                            (optional, default = \"SimulatedAngularCorrelation.root\")" << std::endl
                << "-ab                                                 (optional, enables addback)" << std::endl
                << "-sc                                                 (optional, enables single crystal method, needs addback to be enabled!)" << std::endl
                << "-d     <GRIFFIN distance (110. or 145.)>            (optional, default = 145.)" << std::endl
                << "-vl    <verbose level>                              (optional)" << std::endl;

      return 1;
   }

   if(distance != 145. && distance != 110.) {
      std::cout << "Warning, distance is set to non-standard " << distance << " mm, are you sure this is correct?" << std::endl
                << "Nominal distances are 145 mm in full suppression mode, and 110 mm in forward/high efficiency mode." << std::endl;
   }

   // open the output file
   TFile output(outputFilename.c_str(), "recreate");
   if(!output.IsOpen()) {
      std::cerr << "Failed to open file " << outputFilename << ", maybe check permissions and disk space?" << std::endl;
      return 1;
   }

   // create angle map, never use folding and/or grouping, that is done in the AngularCorrelations program
   if(verboseLevel > 0) { TGriffinAngles::Verbosity(EVerbosity::kAll); }
   TGriffinAngles angles(distance, false, false, addback);
   // intermediate storage of hits
   std::map<uint32_t, TFragment> fragments;
   TGriffin                      griffin;
   TGriffinBgo                   grifBgo;

   // output histograms and graphs
   std::string conditions = addback ? "using addback" : "without addback";
   conditions += singleCrystal ? " single crystal" : "";

   std::array<std::vector<TH2D*>, 3> hists;
   std::array<TGraphErrors, 3>       graph;

   graph[0].SetName("graph000");
   graph[0].SetTitle(Form("Simulated angular correlation coeff. 000, using %s;angle [#circ]", conditions.c_str()));

   graph[1].SetName("graph010");
   graph[1].SetTitle(Form("Simulated angular correlation coeff. 010, using %s;angle [#circ]", conditions.c_str()));

   graph[2].SetName("graph100");
   graph[2].SetTitle(Form("Simulated angular correlation coeff. 100, using %s;angle [#circ]", conditions.c_str()));

   // loop over the three coefficents
   for(int c = 0; c < 3; ++c) {
      // resize and allocate histograms and graphs
      hists[c].resize(angles.NumberOfAngles());
      for(auto angle = angles.begin(); angle != angles.end(); ++angle) {
         int i       = std::distance(angles.begin(), angle);
         hists[c][i] = new TH2D(Form("AngularCorrelation%d_%d", c, i), Form("%.1f^{o}: Simulated suppressed #gamma-#gamma %s", *angle, conditions.c_str()), bins, minEnergy, maxEnergy, bins, minEnergy, maxEnergy);
      }
      graph[c].Set(angles.NumberOfAngles());

      // add all input files to the chain and set the branch addresses
      TChain chain("ntuple");

      for(auto filename : inputFilenames[c]) {
         chain.Add(filename.c_str());
      }

      Int_t eventNumber = 0;
      chain.SetBranchAddress("eventNumber", &eventNumber);
      Int_t systemID = 0;
      chain.SetBranchAddress("systemID", &systemID);
      Int_t detNumber = 0;
      chain.SetBranchAddress("detNumber", &detNumber);
      Int_t cryNumber = 0;
      chain.SetBranchAddress("cryNumber", &cryNumber);
      Double_t depEnergy = 0.;
      chain.SetBranchAddress("depEnergy", &depEnergy);
      Double_t time = 0.;
      chain.SetBranchAddress("time", &time);

      // main loop over entries of the simulation tree
      int         oldEventNumber = 0;
      uint32_t    address        = 0xdead;
      std::string mnemonic;
      std::string crystalColor = "BGRW";
      TChannel*   channel      = nullptr;
      long int    nEntries     = chain.GetEntries();
      if(verboseLevel > 1) {
         std::cout << "Starting loop over " << nEntries << " entries, using angles:" << std::endl;
         angles.Print();
      }
      std::map<double, int> unknownAngles;
      for(long int e = 0; e < nEntries; ++e) {
         int status = chain.GetEntry(e);
         if(status == -1) {
            std::cerr << "Error occured, couldn't read entry " << e << " from tree " << chain.GetName() << " in file " << chain.GetFile()->GetName() << std::endl;
            continue;
         } else if(status == 0) {
            std::cerr << "Error occured, entry " << e << " in tree " << chain.GetName() << " in file " << chain.GetFile()->GetName() << " doesn't exist" << std::endl;
            break;
         }
         if(eventNumber != oldEventNumber) {
            // new event number, use the current data to create detectors and fill histograms

            // We want to fill the detectors time ordered. We can't sort the map by the time,
            // but at this stage we don't need the key (address) anymore, so we can just
            // copy everything into a vector and sort it.
            std::vector<TFragment> fragmentCopy;
            std::transform(fragments.begin(), fragments.end(), std::back_inserter(fragmentCopy), [](std::pair<const uint32_t, TFragment>& kv) { return kv.second; });
            std::sort(fragmentCopy.begin(), fragmentCopy.end());   //uses the default < operator of TFragment (sorts by timestamp)

            for(auto& frag : fragmentCopy) {
               channel = TChannel::GetChannel(frag.GetAddress());
               switch(frag.GetAddress() / 1000) {
               case 0:   // Griffin
                  griffin.AddFragment(std::make_shared<TFragment>(frag), channel);
                  break;
               case 1:   // Griffin BGO
                  grifBgo.AddFragment(std::make_shared<TFragment>(frag), channel);
                  break;
               default:
                  break;
               }
            }

            if((verboseLevel > 2 && (addback ? griffin.GetSuppressedAddbackMultiplicity(&grifBgo) : griffin.GetSuppressedMultiplicity(&grifBgo)) > 1) || verboseLevel > 3) {
               std::cout << "New event number is " << eventNumber << ", processing old event " << oldEventNumber << std::endl;
               std::cout << "From " << fragments.size() << " fragments in the map, we got " << fragmentCopy.size() << " time sorted fragments, and " << griffin.GetMultiplicity() << " griffin hits, " << grifBgo.GetMultiplicity() << " BGO hits, " << griffin.GetSuppressedMultiplicity(&grifBgo) << " suppressed griffin hits, and " << griffin.GetSuppressedAddbackMultiplicity(&grifBgo) << " suppressed addback hits" << std::endl;
            }
            // now we have the detectors filled so we can simply loop over the suppressed hits we want (addback or not)
            for(int g1 = 0; g1 < (addback ? griffin.GetSuppressedAddbackMultiplicity(&grifBgo) : griffin.GetSuppressedMultiplicity(&grifBgo)); ++g1) {
               if(singleCrystal && griffin.GetNSuppressedAddbackFrags(g1) > 1) { continue; }
               auto* grif1 = (addback ? griffin.GetSuppressedAddbackHit(g1) : griffin.GetSuppressedHit(g1));
               for(int g2 = g1 + 1; g2 < (addback ? griffin.GetSuppressedAddbackMultiplicity(&grifBgo) : griffin.GetSuppressedMultiplicity(&grifBgo)); ++g2) {
                  if(singleCrystal && griffin.GetNSuppressedAddbackFrags(g2) > 1) { continue; }
                  auto* grif2 = (addback ? griffin.GetSuppressedAddbackHit(g2) : griffin.GetSuppressedHit(g2));
                  // calculate the angle
                  double angle = grif1->GetPosition(distance).Angle(grif2->GetPosition(distance)) * 180. / TMath::Pi();
                  if(verboseLevel > 4) {
                     std::cout << "det,/cry. " << grif1->GetDetector() << "/" << grif1->GetCrystal() << " with " << grif2->GetDetector() << "/" << grif2->GetCrystal() << " = " << angle << std::endl;
                  }
                  int angleIndex = 0;
                  if(verboseLevel < 3) {
                     TRedirect r("/dev/null");
                     angleIndex = angles.Index(angle);
                  } else {
                     angleIndex = angles.Index(angle);
                  }
                  if(verboseLevel > 4) {
                     std::cout << "Filling histograms at index " << angleIndex << " = " << angle << " degree, with " << grif1->GetEnergy() << ", " << grif2->GetEnergy() << std::endl;
                  }
                  if(angleIndex >= 0) {
                     hists[c].at(angleIndex)->Fill(grif1->GetEnergy(), grif2->GetEnergy());
                     hists[c].at(angleIndex)->Fill(grif2->GetEnergy(), grif1->GetEnergy());
                  } else {
                     unknownAngles[angle]++;
                     if(verboseLevel > 4) {
                        std::cout << "Not filling histogram for griffin hits:" << std::endl;
                        grif1->Print();
                        grif2->Print();
                     }
                  }
               }
            }

            griffin.Clear("a");
            grifBgo.Clear("a");
            fragments.clear();
         }
         switch(systemID) {
         case 1000:
            address = 4 * detNumber + cryNumber;
            break;
         case 1010:   //left extension suppressor
         case 1020:   //right extension suppressor
         case 1030:   //left casing suppressor
         case 1040:   //right casing suppressor
         case 1050:   //back suppressor
            address = 1000 + 10 * detNumber + cryNumber;
            break;
         default:
            address = 0xdead;
            break;
         }
         if(fragments.count(address) == 1) {
            if(verboseLevel > 3) {
               std::cout << "Found second hit for address " << address << ", updating time and energy from " << fragments[address].GetTime() << ", " << fragments[address].GetCharge();
            }
            // only update the time if the energy deposited is larger
            if(depEnergy > fragments[address].GetCharge()) {
               // timestamp is in 10 ns units, cfd is 10/16th of a nanosecond units and replaces the lowest 18 bits of the timestamp
               // so we multiply the time by 16e8, and use only the lowest 22 bits
               fragments[address].SetTimeStamp(time * 1e8);
               fragments[address].SetCfd(static_cast<int>(time * 16e8) & 0x3fffff);
            }
            fragments[address].SetCharge(fragments[address].GetCharge() + static_cast<float>(depEnergy));
            if(verboseLevel > 3) {
               std::cout << " to " << fragments[address].GetTime() << ", " << fragments[address].GetCharge() << std::endl;
            }
         } else {
            fragments[address].SetAddress(address);
            fragments[address].SetTimeStamp(time * 1e8);
            fragments[address].SetCfd(static_cast<int>(time * 16e8) & 0x3fffff);
            fragments[address].SetCharge(static_cast<float>(depEnergy));
            channel = TChannel::GetChannel(address);
            if(channel == nullptr) {
               switch(systemID) {
               case 1000:
                  mnemonic = Form("GRG%02d%cN00A", detNumber, crystalColor[cryNumber]);
                  break;
               case 1010:                                                                             //left extension suppressor
               case 1020:                                                                             //right extension suppressor
               case 1030:                                                                             //left casing suppressor
               case 1040:                                                                             //right casing suppressor
               case 1050:                                                                             //back suppressor
                  mnemonic = Form("GRS%02d%cN%02d", detNumber, crystalColor[cryNumber], cryNumber);   // third argument should be 1-5, not sure how to set this properly
                  break;
               default:
                  break;
               }
               channel = new TChannel;
               channel->SetAddress(address);
               channel->SetName(mnemonic.c_str());
               channel->SetDetectorNumber(detNumber + 1);
               channel->SetCrystalNumber(cryNumber);
               channel->SetDigitizerType(TPriorityValue<std::string>("GRF16", EPriority::kRootFile));
               TChannel::AddChannel(channel);
            }
         }

         oldEventNumber = eventNumber;

         if(e % 1000 == 0) {
            std::cout << std::setw(3) << (100 * e) / nEntries << "% done\r" << std::flush;
         }
      }   // end of entry loop

      std::cout << "100% done" << std::endl;

      if(verboseLevel > 0) {
         std::cout << "coeff. " << c << ": done looping over the tree and creating histograms" << std::endl;
      }

      if(unknownAngles.size() > 0) {
         std::cout << unknownAngles.size() << " unknown angles:" << std::endl;
         for(auto& angle : unknownAngles) {
            std::cout << std::setw(10) << angle.first << " found " << std::setw(8) << angle.second << " times" << std::endl;
         }
      }

      // create the graphs by getting the bin contents of the 2D matrix
      for(size_t i = 0; i < hists[c].size(); ++i) {
         auto counts = hists[c][i]->GetBinContent(hists[c][i]->GetXaxis()->FindBin(gateEnergy), hists[c][i]->GetYaxis()->FindBin(fitEnergy));
         graph[c].SetPoint(i, angles.Angle(i), counts / angles.Count(angles.Angle(i)));
         graph[c].SetPointError(i, 0., TMath::Sqrt(counts) / angles.Count(angles.Angle(i)));
      }

      // write histograms and graph
      for(auto* hist : hists[c]) {
         hist->Write();
         delete hist;
      }

      graph[c].Write();

      if(verboseLevel > 0) {
         std::cout << c << ": done writing histograms and graph" << std::endl;
      }
   }   // end of coeff. loop

   settings.Write();

   output.Close();

   std::cout << "done, wrote results to " << outputFilename << std::endl;

   return 0;
}
