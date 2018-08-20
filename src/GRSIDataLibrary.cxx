#include <string>

#include "TMidasFile.h"
#include "TTriumfDataParser.h"
#include "GRSIDataVersion.h"

extern "C" TMidasFile* CreateFile(std::string& fileName) { return new TMidasFile(fileName.c_str()); }
extern "C" void DestroyFile(TMidasFile* obj) { delete obj; }

extern "C" TTriumfDataParser* CreateParser() { return new TTriumfDataParser; }
extern "C" void DestroyParser(TTriumfDataParser* obj) { delete obj; }

extern "C" std::string LibraryVersion() { return std::string(GRSIDATA_RELEASE); }
