//
// Free function to create a geant4 test environment geometry
//
//
// Original author KLG
//
// Notes:
//
// one can nest volume inside other volumes if needed
// see other construct... functions for examples
//
// Modified by: Albert Szewczyk
// Modifications: Created a loop in the fucntion to allow multiple shapes to be tested in the geometry enivronment.
#include "art/Utilities/ToolMacros.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"

#include <string>
#include <vector>
#include <cstring>
#include <array>

// Mu2e includes.

#include "Offline/Mu2eG4/inc/MaterialFinder.hh"
#include "Offline/Mu2eG4/inc/InitEnvToolBase.hh"
#include "Offline/Mu2eG4/inc/nestTubs.hh"
#include "Offline/Mu2eG4/inc/nestBox.hh"
#include "Offline/Mu2eG4Helper/inc/VolumeInfo.hh"
#include "Offline/ConfigTools/inc/SimpleConfig.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/Mu2eG4/inc/findMaterialOrThrow.hh" // added to directly find the material instead of retreiving the string from the geometry file again

// G4 includes
#include "Geant4/G4String.hh"
#include "Geant4/G4ThreeVector.hh"
#include "Geant4/G4Material.hh"
#include "Geant4/G4Color.hh"
#include "Geant4/G4Tubs.hh"
#include "Geant4/G4RotationMatrix.hh"
#include "Offline/Mu2eG4Helper/inc/Mu2eG4Helper.hh"
#include "CLHEP/Units/SystemOfUnits.h"

using namespace std;
// deep down to internvals and a class is dervied form the G4 event loop
// use the offline
// potentially use G4 beamline
// no imeadeate way
namespace mu2e {

  class ConstructTgtTube: public InitEnvToolBase {
  public:
    ConstructTgtTube(const fhicl::ParameterSet& PSet) {}
    int construct(VolumeInfo const& ParentVInfo, SimpleConfig const& Config);
  };

//-----------------------------------------------------------------------------
  int ConstructTgtTube::construct(VolumeInfo const& parentVInfo, SimpleConfig const& _config) {

    const bool forceAuxEdgeVisible = _config.getBool("g4.forceAuxEdgeVisible");
    const bool doSurfaceCheck      = _config.getBool("g4.doSurfaceCheck");
    const bool placePV             = true;

    // Extract box information from the config file.

     G4bool boxVisible        = _config.getBool("box.visible",true);
     G4bool boxSolid          = _config.getBool("box.solid",true);

     // Due to creating multiple tubes or boxes,
     // Using material finder may not be neccesary as you can directly fin the material Using findaMaterialOrThrow.
    MaterialFinder materialFinder(_config);
    // arrays of information for each cylinder are declared
    // information from the geometry file(ProdTgt4) tube is retrieved using the online function
    G4bool tubeVisible        = _config.getBool("tube.visible",true);
    G4bool tubeSolid          = _config.getBool("tube.solid",true);
    string tubeBase = "tube";
    G4Colour  orange  (.75, .55, .0);
    int numberOfTubes =  _config.getInt("tube.numberOfTubes");
    vector <double> tubeRIns;
    _config.getVectorDouble("tube.rIn",tubeRIns);
    vector <double> tuberOuts;
    _config.getVectorDouble("tube.rOut", tuberOuts);
    vector <double> halfLengths;
    _config.getVectorDouble("tube.halfLength", halfLengths);
    vector <double> phiZeroes;
    _config.getVectorDouble("tube.phi0", phiZeroes);
    vector <double> phiSpans;
    _config.getVectorDouble("tube.phiSpan",phiSpans );
    vector <double> phiRotZs;
    _config.getVectorDouble("tube.phiRotZ",phiRotZs);
    vector <double> signs;
    _config.getVectorDouble("tube.sign",signs);
    vector <int> copyNumbers;
    _config.getVectorInt("tube.copyNumber", copyNumbers);
    //varaibales to be used to retrieve the variables from the vector string, convert them to doubles
    vector<string> centerCoordinates;
    _config.getVectorString("tube.centerInWorld", centerCoordinates);
    vector <string> material;
    _config.getVectorString("tube.wallMaterialName" ,material);
    // traverse the string of center coordinates to convert them to vector arrays with each array containing 3 doubles.
    // get the data in a string and convert it into a vector
   vector <array<double,3>> center;
   string toAdd;
   int index = 0;
   array<double,3> currentTriplet; 
   for (const auto& coordinate : centerCoordinates) {
        for (char c : coordinate) {
            if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == 'e' || c == 'E') {
                toAdd += c;
            } else if (c == ',' || c == '}') {
                if (!toAdd.empty()) {
                    currentTriplet[index++] = stod(toAdd);
                    toAdd.clear();
                }
            }
        }
        // Handle the last value
        if (!toAdd.empty()) {
            currentTriplet[index++] = stod(toAdd);
            toAdd.clear();
        }
        // Add the triplet to the result vector if it's complete
        if (index == 3) {
            center.push_back(currentTriplet);
            index = 0;
        }
    }
   // Loop creates multiple tube with the information passed down.
    for ( int i = 0; i < numberOfTubes; i++){
       
    // adds a new cylinder to the array
     tubeBase = "Tube " + to_string(i+1);
   
     
     

    
     // directly finds material from the G4 library because the material was retrived earlier
     // The material 
     G4Material* tubeMaterial = findMaterialOrThrow(material[i]);
     
     const G4ThreeVector tubeCenterInWorld(center[i][0],center[i][1],center[i][2]);
     // tube paramaters are taken in as an array of double parameters instead.
    VolumeInfo tubeVInfo(nestTubs( tubeBase,
                                   {tubeRIns[i], tuberOuts[i], halfLengths[i],phiZeroes[i],phiSpans[i]},
                                   tubeMaterial,
                                   0, // no rotation, 
                                   tubeCenterInWorld,
                                   parentVInfo,
                                   0,
                                   // we assign a non 0 copy nuber for
                                   // volume tracking purposes. 
                                   tubeVisible,
                                   orange,
                                   tubeSolid,
                                   forceAuxEdgeVisible,
                                   placePV,
                                   doSurfaceCheck
                                   ));
    }
    // Box parameters
    // looping to make multiple boxes wasnt needed when code was mofified
    // copy the functionality for making multiple tubes but ensure that the box parameters are used
    
    
    vector<double> boxParams;
    _config.getVectorDouble( "box.halfLengths", boxParams);

    
    G4Material* boxMaterial = materialFinder.get("box.wallMaterialName");

    const G4ThreeVector boxCenterInWorld(_config.getHep3Vector("box.centerInWorld"));

    
    
    VolumeInfo boxVInfo(nestBox( "Box",
                                 boxParams,
                                 boxMaterial,
                                 0, // no rotation
                                 boxCenterInWorld,
                                 parentVInfo,
                                 _config.getInt("box.copyNumber",2),
                                 // we assign a non 0 copy nuber for
                                 // volume tracking purposes
                                 boxVisible,
                                 orange,
                                 boxSolid,
                                 forceAuxEdgeVisible,
                                 placePV,
                                 doSurfaceCheck
                                 ));
   
    return 0;
  }

}

DEFINE_ART_CLASS_TOOL(mu2e::ConstructTgtTube)
