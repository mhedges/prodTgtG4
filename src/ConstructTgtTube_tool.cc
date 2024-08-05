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
// Lines of code can be modified to use G4 UnionSolid, G4 MultiUnion, or neither of them
// G4 MultiUnion is not supported on Root as of this code being pushed
// locations to remark code to change the solids used can be done
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
#include "Offline/Mu2eG4/inc/finishNesting.hh"
#include "Offline/Mu2eG4/inc/findMaterialOrThrow.hh" // added to directly find the material instead of retreiving the string from the geometry file again

// G4 includes
#include "Geant4/G4Transform3D.hh"
#include "Geant4/G4MultiUnion.hh"
#include "Geant4/G4String.hh"
#include "Geant4/G4Polycone.hh"
#include "Geant4/G4ThreeVector.hh"
#include "Geant4/G4Material.hh"
#include "Geant4/G4Color.hh"
#include "Geant4/G4Tubs.hh"
#include "Geant4/G4UnionSolid.hh"
#include "Geant4/G4LogicalVolume.hh"
#include "Geant4/G4PVPlacement.hh"
#include "Geant4/G4NistManager.hh"
#include "Geant4/G4RotationMatrix.hh"
#include "Offline/Mu2eG4Helper/inc/Mu2eG4Helper.hh"
#include "CLHEP/Units/SystemOfUnits.h"

#include "WireMesh.hh"

using namespace std;
// deep down to internvals and a class is dervied form the G4 event loop
// use the offline
// potentially use G4 beamline
// no imeadeate way
namespace mu2e {
  vector<array<double, 3>> parsePoints(const vector<string>& points);
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

    // G4bool boxVisible        = _config.getBool("box.visible",true);
    //  G4bool boxSolid          = _config.getBool("box.solid",true);

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
    
    vector<array<double, 3>> center;
    center =  parsePoints(centerCoordinates);
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
    //polycone paramters and algorithm


    vector<double> polyconeCenter;
    _config.getVectorDouble("polytube.centerInWorld", polyconeCenter);
    int horizontalWires = _config.getInt("polytube.horizontalWires");
    int verticalWires = _config.getInt("polytube.verticalWires");
    double pHalfLength = _config.getDouble("polytube.halfLength");
    double pRin = _config.getDouble("polytube.rIn");
    double pRout = _config.getDouble("polytube.rOut");
    double verticalSpan = _config.getDouble("polytube.verticalTubeSpan");
      G4Material* pMaterial = materialFinder.get("polytube.material");
  
      TubeWireMesh metalMesh({polyconeCenter[0],polyconeCenter[1],polyconeCenter[2]}, horizontalWires, verticalWires, pHalfLength, pRin, pRout, false, verticalSpan);

 
    metalMesh.createWireMeshSolid();
    int numberOfPolycones =  horizontalWires * verticalWires + verticalWires* verticalWires; 

   vector<TubsParams> pParams = metalMesh.getParams();
   vector <array<double, 3>> pCenter = metalMesh.getCoordinates();
   string polytubeBase;

   // remark the solid you are not using. Remark both if not using Unions at all
   G4UnionSolid* UnionSolid = nullptr;
   //  G4MultiUnion* UnionSolid = new G4MultiUnion("WireMesh");
   
 for (int i = 0; i < numberOfPolycones; i++) {
            polytubeBase = G4String("Polycone" + to_string(i + 1));
	    


	    // paramaters regardless of using union solid or Multi Union
	    

	    // /* 
       G4RotationMatrix* rotation = new G4RotationMatrix();
      G4ThreeVector translation(pCenter[i][0],pCenter[i][1], pCenter[i][2]);
      G4Tubs* cylinder = new G4Tubs(polytubeBase, pParams[i].innerRadius(),  pParams[i].outerRadius(),  pParams[i].zHalfLength(),  pParams[i].phi0(),  pParams[i].phiTotal());
      // */
      //remark above here to not use G4 Union Solid
      
      // to be used for multiUnion, not Union Solid 
      //   G4Transform3D tr(*rotation, translation);
      //   UnionSolid->AddNode(*cylinder , tr);
      // using Union Solid
      
      // /*
      if( i == 0){
       UnionSolid = new G4UnionSolid("UnionSolid", cylinder, cylinder, rotation, translation);
      }

      else{
       UnionSolid = new G4UnionSolid("UnionSolid", UnionSolid, cylinder, rotation , translation);
      }
      // */
   
       
            // Non G4 unionSolid mode
	    /*
            VolumeInfo tubeVInfo(nestTubs(polytubeBase,
                pParams[i],
                pMaterial,
                0, // no rotation,
                G4ThreeVector(pCenter[i][0], pCenter[i][1], pCenter[i][2]),
                parentVInfo,
                0,
                // we assign a non-0 copy number for volume tracking purposes.
                tubeVisible,
                orange,
                tubeSolid,
                forceAuxEdgeVisible,
                placePV,
                doSurfaceCheck));
	    */
      // end of non Union Solids used
	    
 }
 // closes Union Solid/ Remark if not used.
 
 //  UnionSolid -> Voxelize();

    // used for UnionSolid or MultiUnion
    // /*
	VolumeInfo mesh;
	mesh.name = "Wire Mesh";
	mesh.solid = UnionSolid;
	finishNesting(mesh,
		      pMaterial,
		      0,
		      G4ThreeVector(polyconeCenter[0],polyconeCenter[1],polyconeCenter[2]),
		      parentVInfo.logical,
		      1,
		      tubeVisible,
		      orange,
		      tubeSolid,
		      forceAuxEdgeVisible,
		      placePV,
		      doSurfaceCheck);
	// */
    

 

    
    // Box parameters
	/*
    int numberOfBoxes=  _config.getInt("box.numberOfBoxes");
    
    vector<string> boxMaterials;
    _config.getVectorString("box.wallMaterialName",boxMaterials);

    vector <string> boxDimensions;
    _config.getVectorString("box.halfLengths", boxDimensions);
    vector<array<double, 3>> boxParams = parsePoints(boxDimensions);
    
    vector<int> boxNumbers;
    _config.getVectorInt("box.copyNumber", boxNumbers);
    
    vector<string> boxCenterCoordinates;
    _config.getVectorString("box.centerInWorld", boxCenterCoordinates);
    vector<array<double, 3>> boxCenter = parsePoints(boxCenterCoordinates);
    
    string boxBase;
    
    
    
  //rotation matrix code to rotate the tubes around 

G4RotationMatrix* rotation = new G4RotationMatrix();
rotation->rotateY(90.0*deg); // Rotate 90 degrees around the y-axis
     
	

     for ( int i = 0; i < numberOfBoxes; i++){
       
    // adds a new cylinder to the array
    boxBase  = "Box " + to_string(i+1);
    
    G4Material* boxMaterial =  findMaterialOrThrow(boxMaterials[i]);

    const G4ThreeVector boxCenterInWorld(boxCenter[i][0],boxCenter[i][1],boxCenter[i][2]);

    
    G4RotationMatrix* boxRotation = new G4RotationMatrix();
    boxRotation->rotateY(90.0*CLHEP::deg); // Rotate 90 degrees around the y-axis
    vector<double> boxParamVec(boxParams[i].begin(), boxParams[i].end());
    VolumeInfo boxVInfo(nestBox( boxBase,
                                 boxParamVec,
                                 boxMaterial,
                                 boxRotation, // no rotation
                                 boxCenterInWorld,
                                 parentVInfo,
                                 boxNumbers[i],
                                 // we assign a non 0 copy nuber for
                                 // volume tracking purposes
                                 boxVisible,
                                 orange,
                                 boxSolid,
                                 forceAuxEdgeVisible,
                                 placePV,
                                 doSurfaceCheck
                                 ));


     }
    
 */

     // traversing through the polly cones

     
   
    return 0;
  }
  // function to parse a vector string retrieved from the geometry file to turn into a vector of arrays
  // the offline doesnt seem to support taking vectors of arrays or 2dimensional vectors
  //when Simple config is being used.
vector<array<double, 3>> parsePoints(const vector<string>& points) {
    vector<array<double, 3>> set;
    string toAdd;
    int index = 0;
    array<double, 3> currentTriplet;

    for (const auto& coordinate : points) {
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
        set.push_back(currentTriplet);
        index = 0;
      }
    }
    return set;
  }
  
}

DEFINE_ART_CLASS_TOOL(mu2e::ConstructTgtTube)
