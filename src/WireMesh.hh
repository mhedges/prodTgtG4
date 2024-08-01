#include <iostream>
#include <string>
#include <array>
#include <vector>

// Geant4 libraries
#include "Geant4/G4String.hh"
#include "Geant4/G4ThreeVector.hh"
#include "Geant4/G4Material.hh"
#include "Geant4/G4Color.hh"
#include "Geant4/G4Tubs.hh"
#include "Geant4/G4Polycone.hh"
#include "Geant4/G4UnionSolid.hh"


#include "Offline/GeomPrimitives/inc/TubsParams.hh"
#include "Offline/Mu2eG4/inc/nestTubs.hh"
#include "Offline/Mu2eG4Helper/inc/VolumeInfo.hh"

using namespace std;

class TubeWireMesh {
  
private:
    double innerRadius;
    double outerRadius;
    double halfLength;
    int horizontalWireQuantity;
    int verticalWireQuantity;
    const array<double, 3> centerCoordinates;
    vector<array<double, 3>> unionCenter;
    vector<mu2e::TubsParams> parameters;
  double phiTotal;

public:
    // constructor
  TubeWireMesh(array<double, 3> cCoordinate, int hWireCount, int vWireCount, double hLength, double rIn, double rOut, bool defaultSpan, double totalPhi )
    : innerRadius(rIn), outerRadius(rOut), halfLength(hLength), horizontalWireQuantity(hWireCount), verticalWireQuantity(vWireCount), centerCoordinates(cCoordinate), phiTotal(totalPhi) {

    if (defaultSpan == true){
      phiTotal = 3.1415 / verticalWireQuantity;
    }
  }

    // getters and mutators
    vector<mu2e::TubsParams> getParams() {
        return parameters;
    }

    vector<array<double, 3>> getCoordinates() {
        return unionCenter;
    }

    // union solids are instances of g4 volume
    void createWireMeshSolid() {
        // creates the wire quantities

         const double incrementLength = (halfLength * 2 ) / horizontalWireQuantity; // gap between vertical wires
        double coordinate = centerCoordinates[2] - halfLength;

        const double gap = (outerRadius - innerRadius) / verticalWireQuantity; // gap between horizontal wires
      

        // Horizontal wires
        for (int i = 0; i < horizontalWireQuantity; i++) { // coordinate increases in the loop
            double iR = innerRadius;
            double oR = innerRadius + gap / 2;
            for (int j = 0; j < verticalWireQuantity; j++) { // radius is increased in this loop
                unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], coordinate});
                parameters.push_back(mu2e::TubsParams(iR, oR, incrementLength /8));
                iR += gap;
                oR += gap;
            }
	    // The Root program creates uneven increment so the center length is increase based on the number of horziontal wires

	    //coordinates are increased evenly based on the numbe or hirzontal wires
	    // the wrie queantity must be casted toa  double to work
            coordinate +=  incrementLength*(1 + 1/double(horizontalWireQuantity));
        }

        // Reset coordinate for vertical wires
        coordinate = centerCoordinates[2] - halfLength ;



	/*
        // Vertical wires
        for (int i = 0; i < horizontalWireQuantity; i++) { // coordinate increases
            double iR = innerRadius + gap/2 ;
            double oR = innerRadius + gap * 1.5;
            for (int j = 0; j < verticalWireQuantity; j++) { // radius increases
                for (int k = 0; k < verticalWireQuantity; k++) { // phi span increases
                    double phi0 = k * 2 * 3.1415 / verticalWireQuantity;
                    // Corrected calculation
                    unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], coordinate});
                    parameters.push_back(mu2e::TubsParams(iR, oR, incrementLength / 8, phi0, phiTotal));
                }
                iR += gap;
                oR += gap;
            }
            coordinate +=  incrementLength * (1+ 1/double(horizontalWireQuantity));
        }
	*/




	
	//connecting wires
	// a potential algorithm could be added where there isnt an increment of connecting wires

	    //	coordinate = centerCoordinates[2] - halfLength +  incrementLength;
	double iR = innerRadius + gap/2;
         double oR = innerRadius + gap;
	  for (int i = 0; i < verticalWireQuantity; i++) { // radius increases
        
            for (int j = 0; j < verticalWireQuantity; j++) { // span  increases
                
                    double phi0 = j * 2 * 3.1415 / verticalWireQuantity;
               
                    unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], centerCoordinates[2]});
                    parameters.push_back(mu2e::TubsParams(iR, oR, halfLength +  incrementLength /8, phi0, phiTotal));
                
               
            }
            iR += gap;
            oR += gap;
	  }
    


	// second algorithm
	  /*
	coordinate = centerCoordinates[2] - halfLength +  incrementLength;
	  for (int i = 0; i < horizontalWireQuantity; i++) { // coordinate increases
            double iR = innerRadius;
            double oR = innerRadius + gap / 2;
            for (int j = 0; j < verticalWireQuantity; j++) { // radius increases
                for (int k = 0; k < verticalWireQuantity; k++) { // phi span increases
                    double phi0 = k * 2 * 3.1415 / verticalWireQuantity;
               
                    unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], coordinate});
                    parameters.push_back(mu2e::TubsParams(iR, oR, incrementLength, phi0, phiTotal));
                }
                iR += gap;
                oR += gap;
            }
            coordinate +=  incrementLength  * ( 1 - 1/double(horizontalWireQuantity));
	  }
	  */
    }
};
