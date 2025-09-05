#----------------------------------------------------------------------------
# Set up colors
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(GRSIData)

#----------------------------------------------------------------------------
# find the git package (needed to determine the version of the code)
find_package(Git REQUIRED)

#----------------------------------------------------------------------------
# find the root package (and add COMPONENTS)
#find_package(ROOT REQUIRED COMPONENTS Gui GuiHtml Minuit Spectrum)
find_package(ROOT OPTIONAL_COMPONENTS XMLParser)

if(${ROOT_xml_FOUND})
	add_compile_options(-DHAS_XML)
	set(CLING_OPTIONS "-DHAS_XML")
else()
	message("${Red}XML feature for ROOT not found (ROOT_xml_FOUND = ${ROOT_xml_FOUND})!${ColourReset}")
endif()

#----------------------------------------------------------------------------
# find X11 packages
find_package(X11 REQUIRED)

#----------------------------------------------------------------------------
# set c++ standard to the one used for root
if(DEFINED ROOT_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD ${ROOT_CXX_STANDARD})
else()
	set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

#----------------------------------------------------------------------------
# configure the include directories and set other variables
include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
link_directories(${CMAKE_BINARY_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_BINARY_DIR}/bin)

#----------------------------------------------------------------------------
# copy includes
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_BINARY_DIR})

#----------------------------------------------------------------------------
# define all libraries and root dictionaries
# for some we also create dependencies on other libraries to remove linking errors later on

add_library(TMidas SHARED
	${PROJECT_SOURCE_DIR}/libraries/TMidas/TMidasEvent.cxx
	${PROJECT_SOURCE_DIR}/libraries/TMidas/TMidasFile.cxx
	${PROJECT_SOURCE_DIR}/libraries/TMidas/TXMLOdb.cxx
	)
root_generate_dictionary(G__TMidas TXMLOdb.h TMidasEvent.h TMidasFile.h MODULE TMidas LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TMidas/LinkDef.h OPTIONS ${CLING_OPTIONS})
target_link_libraries(TMidas TGRSIFormat ${ROOT_LIBRARIES})
add_dependencies(TMidas GRSIDataVersionCompile GRSIDataVersionBuild)

add_library(TGRSIFormat SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIFormat/TGRSIDetectorInformation.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIFormat/TGRSIMnemonic.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TCSM/TCSM.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TCSM/TCSMHit.cxx
	)
root_generate_dictionary(G__TGRSIFormat TGRSIDetectorInformation.h TGRSIMnemonic.h TCSM.h TCSMHit.h MODULE TGRSIFormat LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIFormat/CMakeLinkDef.h)
target_link_libraries(TGRSIFormat TAries TDescant TDemand TEmma TGenericDetector TGriffin TLaBr TPaces TRcmp TRF TS3 TSceptar TSharc TSharc2 TSiLi TTAC TTigress TTip TTrific TTriFoil TZeroDegree)

add_library(TGRSIDataParser SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIDataParser/TGRSIDataParser.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIDataParser/TGRSIDataParserException.cxx
	)
target_link_libraries(TGRSIDataParser TMidas)

add_library(TAngularCorrelation SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TAngularCorrelation/TAngularCorrelation.cxx
	)
root_generate_dictionary(G__TAngularCorrelation TAngularCorrelation.h MODULE TAngularCorrelation LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TAngularCorrelation/LinkDef.h)
target_link_libraries(TAngularCorrelation TGriffin)

add_library(TAries SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TAries/TAries.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TAries/TAriesHit.cxx
	)
root_generate_dictionary(G__TAries TAries.h TAriesHit.h MODULE TAries LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TAries/LinkDef.h)
target_link_libraries(TAries ${GRSI_LIBRARIES})

add_library(TDescant SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDescant/TDescant.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDescant/TDescantHit.cxx
	)
root_generate_dictionary(G__TDescant TDescant.h TDescantHit.h MODULE TDescant LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDescant/LinkDef.h)
target_link_libraries(TDescant ${GRSI_LIBRARIES})

add_library(TDemand SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDemand/TDemand.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDemand/TDemandHit.cxx
	)
root_generate_dictionary(G__TDemand TDemand.h TDemandHit.h MODULE TDemand LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TDemand/LinkDef.h)
target_link_libraries(TDemand ${GRSI_LIBRARIES})

add_library(TEmma SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TEmma/TEmma.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TEmma/TEmmaHit.cxx
	)
root_generate_dictionary(G__TEmma TEmma.h TEmmaHit.h MODULE TEmma LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TEmma/LinkDef.h)
target_link_libraries(TEmma ${GRSI_LIBRARIES})

add_library(TGenericDetector SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGenericDetector/TGenericDetector.cxx
	)
root_generate_dictionary(G__TGenericDetector TGenericDetector.h MODULE TGenericDetector LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGenericDetector/LinkDef.h)
target_link_libraries(TGenericDetector ${GRSI_LIBRARIES})

add_library(TGriffin SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/TGriffinAngles.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/TGriffin.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/TGriffinHit.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/TGriffinBgo.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/TGriffinBgoHit.cxx
	)
root_generate_dictionary(G__TGriffin TGriffinAngles.h TGriffin.h TGriffinHit.h TGriffinBgo.h TGriffinBgoHit.h MODULE TGriffin LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TGriffin/LinkDef.h)
target_link_libraries(TGriffin ${GRSI_LIBRARIES})

add_library(TLaBr SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TLaBr/TLaBr.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TLaBr/TLaBrHit.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TLaBr/TLaBrBgo.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TLaBr/TLaBrBgoHit.cxx
	)
root_generate_dictionary(G__TLaBr TLaBr.h TLaBrHit.h TLaBrBgo.h TLaBrBgoHit.h MODULE TLaBr LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TLaBr/LinkDef.h)
target_link_libraries(TLaBr ${GRSI_LIBRARIES})

add_library(TPaces SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TPaces/TPaces.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TPaces/TPacesHit.cxx
	)
root_generate_dictionary(G__TPaces TPaces.h TPacesHit.h MODULE TPaces LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TPaces/LinkDef.h)
target_link_libraries(TPaces ${GRSI_LIBRARIES})

add_library(TRcmp SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TRcmp/TRcmp.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TRcmp/TRcmpHit.cxx
	)
root_generate_dictionary(G__TRcmp TRcmp.h TRcmpHit.h MODULE TRcmp LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TRcmp/LinkDef.h)
target_link_libraries(TRcmp ${GRSI_LIBRARIES})

add_library(TRF SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TRF/TRF.cxx
	)
root_generate_dictionary(G__TRF TRF.h MODULE TRF LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TRF/LinkDef.h)
target_link_libraries(TRF ${GRSI_LIBRARIES})

add_library(TS3 SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TS3/TS3.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TS3/TS3Hit.cxx
	)
root_generate_dictionary(G__TS3 TS3.h TS3Hit.h MODULE TS3 LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TS3/LinkDef.h)
target_link_libraries(TS3 ${GRSI_LIBRARIES})

add_library(TSceptar SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSceptar/TSceptar.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSceptar/TSceptarHit.cxx
	)
root_generate_dictionary(G__TSceptar TSceptar.h TSceptarHit.h MODULE TSceptar LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSceptar/LinkDef.h)
target_link_libraries(TSceptar ${GRSI_LIBRARIES})

add_library(TSharc SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc/TSharc.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc/TSharcHit.cxx
	)
root_generate_dictionary(G__TSharc TSharc.h TSharcHit.h MODULE TSharc LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc/LinkDef.h)
target_link_libraries(TSharc ${GRSI_LIBRARIES})

add_library(TSharc2 SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc2/TSharc2.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc2/TSharc2Hit.cxx
	)
root_generate_dictionary(G__TSharc2 TSharc2.h TSharc2Hit.h MODULE TSharc2 LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSharc2/LinkDef.h)
target_link_libraries(TSharc2 ${GRSI_LIBRARIES})

add_library(TSiLi SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSiLi/TSiLi.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSiLi/TSiLiHit.cxx
	)
root_generate_dictionary(G__TSiLi TSiLi.h TSiLiHit.h MODULE TSiLi LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TSiLi/LinkDef.h)
target_link_libraries(TSiLi ${GRSI_LIBRARIES})

add_library(TTAC SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTAC/TTAC.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTAC/TTACHit.cxx
	)
root_generate_dictionary(G__TTAC TTAC.h TTACHit.h MODULE TTAC LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTAC/LinkDef.h)
target_link_libraries(TTAC ${GRSI_LIBRARIES})

add_library(TTigress SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTigress/TTigress.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTigress/TTigressHit.cxx
	)
root_generate_dictionary(G__TTigress TTigress.h TTigressHit.h MODULE TTigress LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTigress/LinkDef.h)
target_link_libraries(TTigress ${GRSI_LIBRARIES})

add_library(TTip SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTip/TTip.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTip/TTipHit.cxx
	)
root_generate_dictionary(G__TTip TTip.h TTipHit.h MODULE TTip LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTip/LinkDef.h)
target_link_libraries(TTip ${GRSI_LIBRARIES})

add_library(TTrific SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTrific/TTrific.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTrific/TTrificHit.cxx
	)
root_generate_dictionary(G__TTrific TTrific.h TTrificHit.h MODULE TTrific LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTrific/LinkDef.h)
target_link_libraries(TTrific ${GRSI_LIBRARIES})

add_library(TTriFoil SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTriFoil/TTriFoil.cxx
	)
root_generate_dictionary(G__TTriFoil TTriFoil.h MODULE TTriFoil LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TTriFoil/LinkDef.h)
target_link_libraries(TTriFoil ${GRSI_LIBRARIES})

add_library(TZeroDegree SHARED
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TZeroDegree/TZeroDegree.cxx
	${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TZeroDegree/TZeroDegreeHit.cxx
	)
root_generate_dictionary(G__TZeroDegree TZeroDegree.h TZeroDegreeHit.h MODULE TZeroDegree LINKDEF ${PROJECT_SOURCE_DIR}/libraries/TGRSIAnalysis/TZeroDegree/LinkDef.h)
target_link_libraries(TZeroDegree ${GRSI_LIBRARIES})

##----------------------------------------------------------------------------
## add all executable in util
set(GRSIDATA_LIBRARIES TAngularCorrelation TAries TDescant TDemand TEmma TGenericDetector TGriffin TGRSIDataParser TGRSIFormat TLaBr TMidas TPaces TRcmp TRF TS3 TSceptar TSharc TSharc2 TSiLi TTAC TTigress TTip TTrific TTriFoil TZeroDegree)
set(UTIL_NAMES AngularCorrelations bufferclean Deadtime ExamineMidasFile FixRunInfo GainMatchGRIFFIN GetTreeEntries GriffinCTFix LeanComptonMatrices offsetadd offsetfind offsetfix tac_calibrator)
foreach(UTIL IN LISTS UTIL_NAMES)
	add_executable(${UTIL} ${PROJECT_SOURCE_DIR}/util/${UTIL}.cxx)
	target_link_libraries(${UTIL} ${ROOT_LIBRARIES} ${GRSI_LIBRARIES} ${GRSIDATA_LIBRARIES} ${X11_LIBRARIES} ${X11_Xpm_LIB})
endforeach()

##----------------------------------------------------------------------------
# add the global library with the functions to create files and parsers
add_library(GRSIData SHARED
	${PROJECT_SOURCE_DIR}/src/GRSIDataLibrary.cxx
	)
target_link_libraries(GRSIData ${GRSIDATA_LIBRARIES})
add_dependencies(GRSIData ${UTIL_NAMES})

##----------------------------------------------------------------------------
# custom target to generate the version header
# CMAKE_SOURCE_DIR is the GRSISort directory
# PROJECT_SOURCE_DIR is the _deps/grsidata-src directory
add_custom_target(GRSIDataVersionCompile
	${CMAKE_COMMAND} -D SRC=${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h.in
	                 -D DST=${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h
                    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                    -P ${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake
						  BYPRODUCTS ${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h
						  COMMENT "Generating ${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h from ${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h.in"
  )
# couldn't figure out how to copy the file after generating it, so we generate it twice, once (above) in the _deps directory for other files to be compiled against, and once (below) in the build directory
add_custom_target(GRSIDataVersionBuild
	${CMAKE_COMMAND} -D SRC=${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h.in
	                 -D DST=${CMAKE_BINARY_DIR}/include/GRSIDataVersion.h
                    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                    -P ${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake
						  BYPRODUCTS ${CMAKE_BINARY_DIR}/include/GRSIDataVersion.h
						  COMMENT "Generating ${CMAKE_BINARY_DIR}/include/GRSIDataVersion.h from ${PROJECT_SOURCE_DIR}/include/GRSIDataVersion.h.in"
  )

#----------------------------------------------------------------------------
# create a compile_commands.json file which can be used for clang-tidy
if(NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
	message("ENV(CMAKE_EXPORT_COMPILE_COMMANDS) NOT DEFINED, setting it to TRUE")
	set($ENV{CMAKE_EXPORT_COMPILE_COMMANDS} TRUE)
endif()
