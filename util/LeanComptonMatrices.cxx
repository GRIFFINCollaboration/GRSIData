/*
This script is a derivative of the ComptonMatrices.c file.  It is called lean because I have taken out
many of the portions that were simply for comparing different methods.  As I have essentially settled
on one method, these additional steps are just bloating.  Part of this fact means that I no longer
do any weighting other that event mixing, and I also no longer include the metzger Nparallel/Nperpendicular
method of Compton polarimetry.  This script only includes the portions that I believe would be useful for 
actual implementation of this code.  To use this you should only need to ensure the output directory/name 
is appropriate, you will need to make sure the max entries is set, and you need to ensure everything is as
expected in the Parameter Setup section. 

Compile:
g++ LeanComptonMatrices.c -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries `grsi-config --cflags --all-libs` `root-config --cflags --libs` -lTreePlayer -lMathMore -lSpectrum -o MakeComptonMatrices
Run:
Run with selected filename and no max entries:
./MakeComptonMatrices /pathtotree/AnalysisTree.root

This script is designed to create just the histograms required for producing the Compton polarimetry
plots.  These histograms can the be combined for multiple subruns using the hadd function.  The summed
file can be analysed using the AnalyzeComptonMatrices.c.  To run on multiple runs/subruns at once simply
put multiple parameters or use the * character to select multiple files. 
i.e.
./MakeComptonMatrices /pathtotrees/analysis*.root

______________________________________________________________________________________________
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <Globals.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <list>

#include "TTree.h"
#include "TTreeIndex.h"
#include "TVirtualIndex.h"
#include "TFile.h"
#include "TList.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TChain.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TVector.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TStopwatch.h"
#include "TSpectrum.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"

#include "TRunInfo.h"
#include "TGRSISortInfo.h"
#include "TGriffin.h"

//Set to 0 for all entries.
constexpr int MaxEntries = 0;

TList* ComptonHists(TTree* tree, int64_t maxEntries, TStopwatch* w);

int main(int argc, char** argv)
{
   if(argc < 2) {
      std::cout << "usage: " << argv[0] << " <analysis tree files>" << std::endl;
      return 0;
   }
   //We use a stopwatch so that we can watch progress
   TStopwatch w;
   w.Start();

   TFile* file    = nullptr;
   TFile* outfile = nullptr;
   TList* outlist = nullptr;
   for(int f = 0; f < argc - 1; ++f) {
      file = new TFile(argv[f + 1]);   //maybe should be new?

      if(file == nullptr || !file->IsOpen()) {
         std::cout << "Failed to open file '" << argv[f + 1] << "'!" << std::endl;
         continue;
      }

      std::cout << "Analyzing file: " << DBLUE << file->GetName() << RESET_COLOR << std::endl;

      //Opening AnalysisTree and getting run info.
      auto* tree = static_cast<TTree*>(file->Get("AnalysisTree"));
      if(tree == nullptr) {
         printf("Failed to find analysis tree in file '%s'!\n", argv[f + 1]);
         return 1;
      }
      TChannel::ReadCalFromTree(tree);

      auto*       runInfo   = static_cast<TRunInfo*>(file->Get("RunInfo"));
      std::string inputname = argv[f + 1];
      std::string outputname;
      if(runInfo == nullptr) {
         std::cout << "Failed to find run information in file '" << argv[f + 1] << "'!" << std::endl
                   << "Trying to accomodate filenames" << std::endl;
         for(int pop = 0; pop < 5; pop++) { inputname.pop_back(); }
         while(inputname.size() > 9) { inputname.erase(inputname.begin()); }
         outputname = Form("test_%s.root", inputname.c_str());
      } else {
         int runnumber    = TRunInfo::RunNumber();
         int subrunnumber = TRunInfo::SubRunNumber();
         printf("Run number = %d and subrun number = %d\n", runnumber, subrunnumber);
         outputname = Form("CompHists_%05d_%03d.root", runnumber, subrunnumber);
      }

      outfile = new TFile(outputname.c_str(), "recreate");
      std::cout << argv[0] << ": starting Analysis on " << argv[f + 1] << " after " << w.RealTime() << " seconds" << std::endl;
      w.Continue();

      outlist = ComptonHists(tree, MaxEntries, &w);
      if(outlist == nullptr) {
         std::cout << "ComptonHists returned TList* nullptr!" << std::endl;
         continue;
      }
      outlist->Write();
      outfile->Close();
      std::cout << argv[f + 1] << " done after " << w.RealTime() << " seconds" << std::endl;
      w.Continue();
   }
   return 0;
}

// ****************************************************************** //
// ********************* FUNCTION DEFINITIONS *********************** //
// ****************************************************************** //

TList* ComptonHists(TTree* tree, int64_t maxEntries, TStopwatch* w)
{
   if(w == nullptr) {
      w = new TStopwatch;
      w->Start();
   }

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Parameter Setup -----------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////
   //Energy Spectrum Paramaters
   Double_t low     = 0.;
   Double_t high    = 5000.;
   Int_t    nofBins = 10000;
   //Coincidence Parameters
   bool     usetimestamps = true;   // Will use timestamps instead of time.
   Double_t ggTlow        = 0.0;    //Ensure the units here match with the units of GetTimeStamps, or if
   Double_t ggThigh       = 20.0;   //false ensure that they are the same as GetTime

   //These are the time values for each gamma. (Used in Section 2 for coincidence gating)
   int tg1 = 0;
   int tg2 = 0;
   int tg3 = 0;
   //These are the indices of the two hits being compared.  (Used in both Section 1 and Section 2)
   int one   = 0;
   int two   = 0;
   int three = 0;
   //Experimental Angles to be calculated: (Used in both Section 1 and Section 2)
   double Xi      = 0.;
   double CoTheta = 0.;
   //Vectors (Used in both Section 1 and Section 2)
   TVector3 v1;
   TVector3 v2;
   TVector3 v3;
   TVector3 n1;
   TVector3 n2;
   TVector3 d1;
   TVector3 d2;

   int ThetaBins = 180;   // Binsize = 180deg / bins
   int XiBins    = 180;

   std::array<double, 2> gEnergy       = {1332.5, 1173.2};
   double                gEnergyErr    = 5.0;          //Energies within this on eithr side of gEnergy will be considered a good interaction
   double                gEnergySumErr = gEnergyErr;   //If the energy of the two scattering gamma rays sum to within this from either side of gEnergy then they are considered a good interaction.

   bool ForceG1 = true;   //If true it will make gEnergy[0] the energy of the scattered gamma ray in the cascade.

   int gEnergyIndex1 = 0;                     //The index for gEnergy of gamma 1.  Set within the code.
                                              //These parameters could potentially be automated but right now my attempts were unsuccesful
   std::vector<int> MissingClovers = {};      // = {1,2,3,4};//13 //Put Missing Detectors in vector below.  Any detector combinations including these detectors will be ignored.  Detectors are 1 through 16.
   double           DetectorHeight = 145.0;   //This should be found from runinfo->HPGeArrayPosition();, however this may not have been actively set for the experiment. For
                                              //for run02397 it seems that this returned the default of 110mm, but the calculated crystal position magnitudes were 58.112mm,
                                              // lists in which to store the array numbers, energies, and entry numbers of earlier events used for
                                              // event mixing.
   std::list<int>    NonCo_CryNums;
   std::list<double> NonCo_Energies;
   std::list<int>    NonCo_Entries;

   ////////////////////////////////////////////////////////////////////////////////////
   //--------------------- Setting up output histograms -----------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   auto* list = new TList;   //Output list

   auto* XiHist2DGeo_DetDet = new TH2D("XiHist2D_DetDetCoincidenceTheta_Geo", "Possible #xi Angles in GRIFFIN Array for Coincidence Angle #theta (Measured from Clover Faces)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2DGeo_DetDet);
   auto* XiHist2DGeo_CryCry = new TH2D("XiHist2D_CryCryCoincidenceTheta_Geo", "Possible #xi Angles in GRIFFIN Array for Coincidence Angle #theta (Measured from Crystal Positions)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2DGeo_CryCry);

   auto* XiHist2D_DetDet = new TH2D("XiHist2D_DetDetCoincidenceTheta", "Measured #xi Angles for Coincidence Angle #theta (Measured from Clover Faces)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2D_DetDet);
   auto* XiHist2D_CryCry = new TH2D("XiHist2D_CryCryCoincidenceTheta", "Measured #xi Angles for Coincidence Angle #theta (Measured from Crystal Positions)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2D_CryCry);

   auto* XiHist2DNonCo_DetDet = new TH2D("XiHist2D_DetDetCoincidenceTheta_NonCo", "Measured #xi Angles for Non-Coincidenct #gamma_{1} and #gamma_{2} at Angle #theta (Measured from Clover Faces)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2DNonCo_DetDet);
   auto* XiHist2DNonCo_CryCry = new TH2D("XiHist2D_CryCryCoincidenceTheta_NonCo", "Measured #xi Angles for Non-Coincidenct #gamma_{1} and #gamma_{2} at Angle #theta (Measured from Crystal Positions)", XiBins, 0.0, 180.000001, ThetaBins, 0.0, 180.000001);
   list->Add(XiHist2DNonCo_CryCry);

   auto* gammaSinglesAll = new TH1D("gammaSingles", "#gamma singles (All Events);Energy [keV]", nofBins, low, high);
   list->Add(gammaSinglesAll);
   auto* gammagammaAll = new TH2D("gammagamma", "#gamma - #gamma (All Events);Energy [keV];Energy [keV]", nofBins, low, high, nofBins, low, high);
   list->Add(gammagammaAll);
   auto* gammaCrystalAll = new TH2D("gammaCrystal", "#gamma Crystal (All Events);Energy [keV];Crystal Number", nofBins, low, high, 64, 0, 64);
   list->Add(gammaCrystalAll);
   auto* XiCrystalGeo = new TH2D("XiCrystalGeo", "Possible #xi Crystal (All Events);#xi;Crystal Number", XiBins, 0.0, 180.000001, 64, 0, 64);
   list->Add(XiCrystalGeo);
   auto* thetaCrystalGeo = new TH2D("thetaCrystalGeo", "Possible #theta Crystal (All Events);#theta;Crystal Number", ThetaBins, 0.0, 180.000001, 64, 0, 64);
   list->Add(thetaCrystalGeo);
   auto* XiCrystalAll = new TH2D("XiCrystal", "#xi Crystal (All Events);#xi;Crystal Number", XiBins, 0.0, 180.000001, 64, 0, 64);
   list->Add(XiCrystalAll);
   auto* thetaCrystalAll = new TH2D("thetaCrystal", "#theta Crystal (All Events);#theta;Crystal Number", ThetaBins, 0.0, 180.000001, 64, 0, 64);
   list->Add(thetaCrystalAll);
   auto* gammaXi_g1 = new TH2D("gammaXi_g1", "#xi - #gamma (Only #gamma_{1} from Triple Coincidence);#xi;Energy [keV]", XiBins, 0.0, 180.000001, nofBins, low, high);
   list->Add(gammaXi_g1);
   auto* gammaXi_g2 = new TH2D("gammaXi_g2", "#xi - #gamma (Only #gamma_{2} from Triple Coincidence);#xi;Energy [keV]", XiBins, 0.0, 180.000001, nofBins, low, high);
   list->Add(gammaXi_g2);
   auto* gammaSingles_g1 = new TH1D("gammaSingles_g1", "#gamma singles (Only #gamma_{1} from Triple Coincidence);Energy [keV]", nofBins, low, high);
   list->Add(gammaSingles_g1);
   auto* gammaSingles_g2 = new TH1D("gammaSingles_g2", "#gamma singles (Only #gamma_{2} from Triple Coincidence);Energy [keV]", nofBins, low, high);
   list->Add(gammaSingles_g2);
   auto* gammaSingles_g3 = new TH1D("gammaSingles_g3", "#gamma singles (Only #gamma_{3} from Triple Coincidence);Energy [keV]", nofBins, low, high);
   list->Add(gammaSingles_g3);
   auto* ggTimeDiff_g1g2 = new TH1D("ggTimeDiff_g1g2", "#gamma_{1}-#gamma_{2} time difference", 300, 0, 300);
   list->Add(ggTimeDiff_g1g2);
   auto* ggTimeDiff_g1g3 = new TH1D("ggTimeDiff_g1g3", "#gamma_{1}-#gamma_{3} time difference", 300, 0, 300);
   list->Add(ggTimeDiff_g1g3);
   auto* ggTimeDiff_g2g3 = new TH1D("ggTimeDiff_g2g3", "#gamma_{2}-#gamma_{3} time difference", 300, 0, 300);
   list->Add(ggTimeDiff_g2g3);

   ////////////////////////////////////////////////////////////////////////////////////
   //------------------------------- Entry Setup ------------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   if(maxEntries == 0 || maxEntries > tree->GetEntries()) {
      maxEntries = tree->GetEntries();
   }
   TGriffin* grif = nullptr;
   tree->SetBranchAddress("TGriffin", &grif);

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Main Section 1 ------------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   //This section produces the hists which provide the number of occurances of a particular
   //Xi/theta combination in the array.  This is useful to ensure the proper angles are being produced
   //in the main code.
   for(one = 0; one < 64; one++) {
      if(std::binary_search(MissingClovers.begin(), MissingClovers.end(), one / 4 + 1)) { continue; }
      v1 = TGriffin::GetPosition(one / 4 + 1, one % 4, DetectorHeight);
      d1 = TGriffin::GetPosition(one / 4 + 1, 5, DetectorHeight);
      for(two = (one / 4) * 4; two < (one / 4 + 1) * 4; two++) {
         if(one == two) { continue; }
         v2 = TGriffin::GetPosition(two / 4 + 1, two % 4, DetectorHeight);
         for(three = 0; three < 64; three++) {
            if(three / 4 == one / 4) { continue; }
            if(std::binary_search(MissingClovers.begin(), MissingClovers.end(), three / 4 + 1)) { continue; }
            v3 = TGriffin::GetPosition(three / 4 + 1, three % 4, DetectorHeight);
            d2 = TGriffin::GetPosition(three / 4 + 1, 5, DetectorHeight);

            n1      = v3.Cross(v1);
            n2      = v1.Cross(v2);
            Xi      = n1.Angle(n2) * TMath::RadToDeg();
            CoTheta = v3.Angle(v1) * TMath::RadToDeg();

            std::cout << one << ", " << two << ", " << three << ": " << Xi << ", " << d1.Angle(d2) * TMath::RadToDeg() << ", " << CoTheta << std::endl;
            XiHist2DGeo_DetDet->Fill(Xi, TMath::RadToDeg() * d1.Angle(d2));
            XiHist2DGeo_CryCry->Fill(Xi, CoTheta);
            XiCrystalGeo->Fill(Xi, one);
            thetaCrystalGeo->Fill(CoTheta, one);
            //printf("%d %d %d %f %f\n",one,two,three,CoTheta,Xi);
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Main Section 2 ------------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   //This section is the main work horse.  Below each entry is iterated over, and those which meet
   //the required gates (i.e. a proper scatter, energy/coincidence gates) are use to calculate Xi and
   //CoTheta.  This region also produces unpolarized counts for normalization purposes.
   //This is done by through event mixing (internally referred to as NonCo, for NonCorrelated).

   std::cout << "Starting event analysis (Section 2) after " << w->RealTime() << " seconds" << std::endl;
   w->Continue();

   // int curloopval = 0;
   int PrintRate = 1000;
   while(PrintRate >= maxEntries) {
      //This section just adapts the print rate for absurdly small # of entry runs (# entries < PrintRate)
      PrintRate /= 10;
      if(PrintRate < 10) {
         PrintRate = 1;
         break;
      }
   }

   for(int entry = 1; entry < maxEntries; ++entry) {
      if(entry == 1 || entry == maxEntries - 1 || entry % (maxEntries / PrintRate) == 0) {
         std::cout << "\r\t\t\t\t\t\t\t\t "
                   << "\rOn entry: " << std::setw(10) << std::right << entry + 1 << "/" << std::setw(10) << std::left << maxEntries << "  (" << std::setw(3) << std::right << static_cast<int>(static_cast<double>(100 * (entry + 1)) / static_cast<double>(maxEntries)) << "%)";
      }
      tree->GetEntry(entry);
      grif->ResetAddback();
      for(one = 0; one < static_cast<int>(grif->GetMultiplicity()); ++one) {
         //if(std::binary_search(MissingClovers.begin(),MissingClovers.end(), grif->GetGriffinHit(one)->GetDetector() )) continue;  //These could be added to gate out clovers.
         gammaSinglesAll->Fill(grif->GetGriffinHit(one)->GetEnergy());
         gammaCrystalAll->Fill(grif->GetGriffinHit(one)->GetEnergy(), 4 * (grif->GetGriffinHit(one)->GetDetector() - 1) + grif->GetGriffinHit(one)->GetCrystal());

         //------------------------------------Beginning of list maintenance------------------------------------//
         //This updates the last utilized for event mixing.

         NonCo_Entries.push_back(entry);
         NonCo_CryNums.push_back(4 * (grif->GetGriffinHit(one)->GetDetector() - 1) + grif->GetGriffinHit(one)->GetCrystal());   // 0 -> 63
         NonCo_Energies.push_back(grif->GetGriffinHit(one)->GetEnergy());

         if(one == 0) {   //only needs to be done once per entry, as it doesn't depend on current entries multiplicity.
                          // eliminate parts of lists that are too old
            int counter = 0;
            for(int NonCo_Entry : NonCo_Entries) {
               // the number subtracted from entry in the next line determines how far back the BG loop looks for coincidences
               // it should be one more than the number of entries you want to compare
               // i.e. if you want to compare ten entries, this number should be eleven
               if(NonCo_Entry < entry - 11) { counter++; }
            }
            for(int i = 0; i < counter; i++) {
               NonCo_Entries.pop_front();
               NonCo_Energies.pop_front();
               NonCo_CryNums.pop_front();
            }
         }
         //---------------------------------------End of list maintenance---------------------------------------//

         //one will be the first gamma ray of the scatter event
         if(usetimestamps) {
            tg1 = grif->GetGriffinHit(one)->GetTimeStamp();
         } else {
            tg1 = static_cast<int>(grif->GetGriffinHit(one)->GetTime());
         }

         for(two = 0; two < static_cast<int>(grif->GetMultiplicity()); ++two) {
            //if(std::binary_search(MissingClovers.begin(),MissingClovers.end(), grif->GetGriffinHit(two)->GetDetector() )) continue; //These could be added to gate out clovers.
            if(two == one) { continue; }
            //Two will be the second event in the scatter.  It must have lower energy and sum with g1 to
            //have energy within range of an expected gamma ray.  It must also be in true coincidence with
            //gamma 1.  Further restrictions may be made.
            if(usetimestamps) {
               tg2 = grif->GetGriffinHit(two)->GetTimeStamp();
            } else {
               tg2 = static_cast<int>(grif->GetGriffinHit(two)->GetTime());
            }

            if(ggTlow > TMath::Abs(tg2 - tg1) || TMath::Abs(tg2 - tg1) > ggThigh) { continue; }

            gammagammaAll->Fill(grif->GetGriffinHit(one)->GetEnergy(), grif->GetGriffinHit(two)->GetEnergy());

            if(grif->GetGriffinHit(one)->GetDetector() != grif->GetGriffinHit(two)->GetDetector()) { continue; }
            if(grif->GetGriffinHit(one)->GetCrystal() == grif->GetGriffinHit(two)->GetCrystal()) { continue; }
            if(grif->GetGriffinHit(one)->GetEnergy() < grif->GetGriffinHit(two)->GetEnergy()) { continue; }
            if(TMath::Abs(gEnergy[0] - (grif->GetGriffinHit(one)->GetEnergy() + grif->GetGriffinHit(two)->GetEnergy())) < gEnergySumErr) {
               gEnergyIndex1 = 0;
            } else if((TMath::Abs(gEnergy[1] - (grif->GetGriffinHit(one)->GetEnergy() + grif->GetGriffinHit(two)->GetEnergy())) < gEnergySumErr)) {
               gEnergyIndex1 = 1;
            } else {
               continue;
            }
            if(ForceG1 && gEnergyIndex1 != 0) { continue; }   //This gives the option of forcing which gamma is the scatter one.

            //----------------------------------------------------------------------------------------//

            //At this stage we have g1 and g2 being two coincident gammas which enter different crystals
            //in the same detector, and g1 has higher energy than g2.  Togetehr they sum to one of the
            //expected gamma energies.

            //Polarized Work - Use a third coincident gamma ray to provide orientation axis.
            v1 = grif->GetGriffinHit(one)->GetPosition(DetectorHeight);
            v2 = grif->GetGriffinHit(two)->GetPosition(DetectorHeight);
            d1 = TGriffin::GetPosition(grif->GetGriffinHit(one)->GetDetector(), 5, DetectorHeight);

            for(three = 0; three < static_cast<int>(grif->GetMultiplicity()); ++three) {
               //if(std::binary_search(MissingClovers.begin(),MissingClovers.end(), grif->GetGriffinHit(three)->GetDetector() )) continue; //These could be added to gate out clovers.

               if(one == three || two == three) { continue; }
               if(grif->GetGriffinHit(three)->GetDetector() == grif->GetGriffinHit(one)->GetDetector()) { continue; }
               if(TMath::Abs(gEnergy[1 - gEnergyIndex1] - grif->GetGriffinHit(three)->GetEnergy()) > gEnergyErr) { continue; }
               if(usetimestamps) {
                  tg3 = grif->GetGriffinHit(three)->GetTimeStamp();
               } else {
                  tg3 = static_cast<int>(grif->GetGriffinHit(three)->GetTime());
               }

               bool TimeCoincident = ggTlow < TMath::Abs(tg3 - tg1) && TMath::Abs(tg3 - tg1) < ggThigh && ggTlow < TMath::Abs(tg3 - tg2) && TMath::Abs(tg3 - tg2) < ggThigh;
               ggTimeDiff_g1g2->Fill(TMath::Abs(tg2 - tg1));
               ggTimeDiff_g1g3->Fill(TMath::Abs(tg3 - tg1));
               ggTimeDiff_g2g3->Fill(TMath::Abs(tg3 - tg2));

               //Begin Polarized Assymetry Determinations
               v3 = grif->GetGriffinHit(three)->GetPosition(DetectorHeight);
               d2 = TGriffin::GetPosition(grif->GetGriffinHit(three)->GetDetector(), 5, DetectorHeight);

               n1 = v3.Cross(v1);
               n2 = v1.Cross(v2);

               Xi      = n1.Angle(n2) * TMath::RadToDeg();
               CoTheta = v3.Angle(v1) * TMath::RadToDeg();

               if(TimeCoincident) {
                  XiHist2D_DetDet->Fill(Xi, TMath::RadToDeg() * d1.Angle(d2));
                  XiHist2D_CryCry->Fill(Xi, CoTheta);
                  XiCrystalAll->Fill(Xi, 4 * (grif->GetGriffinHit(one)->GetDetector() - 1) + grif->GetGriffinHit(one)->GetCrystal());
                  thetaCrystalAll->Fill(CoTheta, 4 * (grif->GetGriffinHit(one)->GetDetector() - 1) + grif->GetGriffinHit(one)->GetCrystal());

                  gammaXi_g1->Fill(Xi, grif->GetGriffinHit(one)->GetEnergy());
                  gammaXi_g2->Fill(Xi, grif->GetGriffinHit(two)->GetEnergy());
                  gammaSingles_g1->Fill(grif->GetGriffinHit(one)->GetEnergy());
                  gammaSingles_g2->Fill(grif->GetGriffinHit(two)->GetEnergy());
                  gammaSingles_g3->Fill(grif->GetGriffinHit(three)->GetEnergy());
               }
            }   //end of real event 3

            //----------------------------------------------------------------------------------------//

            //Unpolarized Work - Third gamma chosen from uncorrelated (in time) events.  This is used
            //for event mixing weighting of triplets.

            auto it_Energy = NonCo_Energies.begin();
            auto it_CryNum = NonCo_CryNums.begin();
            for(int NonCo_Entry : NonCo_Entries) {
               if(NonCo_Entry <= entry - 2) {   // if the second entry is more than two behind... (this gives a sufficient time window to ensure no time correlation)
                  double Energy3    = *it_Energy;
                  int    CrystalNum = *it_CryNum;
                  if((grif->GetGriffinHit(one)->GetDetector() != CrystalNum / 4 + 1) && (TMath::Abs(gEnergy[1 - gEnergyIndex1] - Energy3) < gEnergyErr)) {
                     d2 = TGriffin::GetPosition((CrystalNum / 4) + 1, 5, DetectorHeight);
                     v3 = TGriffin::GetPosition((CrystalNum / 4) + 1, CrystalNum % 4, DetectorHeight);
                     n1 = v3.Cross(v1);
                     n2 = v1.Cross(v2);

                     Xi      = n1.Angle(n2) * TMath::RadToDeg();
                     CoTheta = v3.Angle(v1) * TMath::RadToDeg();

                     XiHist2DNonCo_DetDet->Fill(Xi, TMath::RadToDeg() * d1.Angle(d2));
                     XiHist2DNonCo_CryCry->Fill(Xi, CoTheta);
                  }
               }
               ++it_Energy;
               ++it_CryNum;
            }   // End of event mixing (NonCo) work.
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   //---------------------------- Main Section e ------------------------------------//
   ////////////////////////////////////////////////////////////////////////////////////

   //Make things pretty (or at least understandable)

   XiHist2DGeo_DetDet->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2DGeo_DetDet->GetXaxis()->CenterTitle();
   XiHist2DGeo_DetDet->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2DGeo_DetDet->GetYaxis()->CenterTitle();
   XiHist2DGeo_DetDet->GetZaxis()->SetTitle("Counts");
   XiHist2DGeo_DetDet->GetZaxis()->CenterTitle();
   XiHist2DGeo_DetDet->SetMarkerStyle(24);
   XiHist2DGeo_DetDet->SetMarkerColor(kRed);
   XiHist2DGeo_DetDet->SetMarkerSize(0.5);

   XiHist2DGeo_CryCry->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2DGeo_CryCry->GetXaxis()->CenterTitle();
   XiHist2DGeo_CryCry->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2DGeo_CryCry->GetYaxis()->CenterTitle();
   XiHist2DGeo_CryCry->GetZaxis()->SetTitle("Counts");
   XiHist2DGeo_CryCry->GetZaxis()->CenterTitle();
   XiHist2DGeo_CryCry->SetMarkerStyle(24);
   XiHist2DGeo_CryCry->SetMarkerColor(kRed);
   XiHist2DGeo_CryCry->SetMarkerSize(0.5);

   XiHist2DNonCo_DetDet->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2DNonCo_DetDet->GetXaxis()->CenterTitle();
   XiHist2DNonCo_DetDet->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2DNonCo_DetDet->GetYaxis()->CenterTitle();
   XiHist2DNonCo_DetDet->GetZaxis()->SetTitle("Counts");
   XiHist2DNonCo_DetDet->GetZaxis()->CenterTitle();
   XiHist2DNonCo_DetDet->SetMarkerStyle(24);
   XiHist2DNonCo_DetDet->SetMarkerColor(kRed);
   XiHist2DNonCo_DetDet->SetMarkerSize(0.5);

   XiHist2DNonCo_CryCry->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2DNonCo_CryCry->GetXaxis()->CenterTitle();
   XiHist2DNonCo_CryCry->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2DNonCo_CryCry->GetYaxis()->CenterTitle();
   XiHist2DNonCo_CryCry->GetZaxis()->SetTitle("Counts");
   XiHist2DNonCo_CryCry->GetZaxis()->CenterTitle();
   XiHist2DNonCo_CryCry->SetMarkerStyle(24);
   XiHist2DNonCo_CryCry->SetMarkerColor(kRed);
   XiHist2DNonCo_CryCry->SetMarkerSize(0.5);

   XiHist2D_DetDet->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2D_DetDet->GetXaxis()->CenterTitle();
   XiHist2D_DetDet->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2D_DetDet->GetYaxis()->CenterTitle();
   XiHist2D_DetDet->GetZaxis()->SetTitle("Counts");
   XiHist2D_DetDet->GetZaxis()->CenterTitle();
   XiHist2D_DetDet->SetMarkerSize(0.5);
   XiHist2D_DetDet->SetMarkerStyle(20);

   XiHist2D_CryCry->GetXaxis()->SetTitle("Experimental Angle #xi (#circ)");
   XiHist2D_CryCry->GetXaxis()->CenterTitle();
   XiHist2D_CryCry->GetYaxis()->SetTitle("Coincidence Angle #theta (#circ)");
   XiHist2D_CryCry->GetYaxis()->CenterTitle();
   XiHist2D_CryCry->GetZaxis()->SetTitle("Counts");
   XiHist2D_CryCry->GetZaxis()->CenterTitle();
   XiHist2D_CryCry->SetMarkerSize(0.5);
   XiHist2D_CryCry->SetMarkerStyle(20);

   std::cout << std::endl;
   return list;
}
