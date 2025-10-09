#include <string>

#include "TClassRef.h"

#include "TMidasFile.h"
#include "TGRSIDataParser.h"
#include "GRSIDataVersion.h"
#include "TChannel.h"

extern "C" TMidasFile* CreateFile(std::string& fileName) { return new TMidasFile(fileName.c_str()); }
extern "C" void        DestroyFile(TMidasFile* obj) { delete obj; }

extern "C" TGRSIDataParser* CreateParser() { return new TGRSIDataParser; }
extern "C" void             DestroyParser(TGRSIDataParser* obj) { delete obj; }

extern "C" std::string LibraryVersion() { return {GRSIDATA_RELEASE}; }

extern "C" void InitLibrary() { TChannel::SetMnemonicClass(TClassRef("TGRSIMnemonic")); }
