# GRSIData

This library provides the data parser and detector classes for data taken with TIGRESS or GRIFFIN at TRIUMF.

The data parser is set up to parse data taken with either the TIGRESS or the GRIFFIN DAQ using the TRIUMF/PSI midas format.

The implemented detector classes span TIGRESS and its ancillary detectors (TIGRESS, CSM, GenericDetector, RF, S3, Sharc, SiLi, Tip, and TriFoil) and GRIFFIN and its ancillary detectors (GRIFFIN, Descant, LaBr + TAC, Paces, Sceptar, and ZeroDegree).

Other implemented classes are:
- the file, event, and bank classes and other structures for .mid files,
- the TGRSIMnemonic which provides enumerations for the detector systems and ditigizers,
- the TGRSIDetectorInformation which provides functions and members to tell which detectors are present in the data,
- the TGRSIDetectorHit which overrides the GetTime function of TDetectorHit to properly handle the different timestamp and CFD data from the different digitizers used, and
- the TAngularCorrelation class which can be used to look at angular correlations of GRIFFIN detectors.

 To compile simply call ```make``` inside the GRSIData directory.
