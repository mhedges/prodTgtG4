// Class is utilized to create a mesh of wires using the construction of multiple tubes from a few parameters
//
//
// Author: Albert Szewczyk
// primary contact email: aszew@illinois.edu , alberts@fnal.gov
// If unable to contact Albert on this code, contact Michael Hedges
#include <iostream>
#include <array>
#include <vector>
// Geant4 Libraries
#include "Geant4/G4Tubs.hh"
//Offline Libraries
#include "Offline/GeomPrimitives/inc/TubsParams.hh"

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
    TubeWireMesh(array<double, 3> cCoordinate, int hWireCount, int vWireCount, double hLength, double rIn, double rOut, bool defaultSpan, double totalPhi)
        : innerRadius(rIn), outerRadius(rOut), halfLength(hLength), horizontalWireQuantity(hWireCount), verticalWireQuantity(vWireCount), centerCoordinates(cCoordinate), phiTotal(totalPhi) {

        if (defaultSpan) {
            phiTotal = 3.1415 / verticalWireQuantity;
            // code can be added here to set the length of the horizontal wires
        }
    }

    // getters and mutators
    vector<mu2e::TubsParams> getParams() {
        return parameters;
    }

    vector<array<double, 3>> getCoordinates() {
        return unionCenter;
    }

    void createWireMeshSolid() {
        // creates the wire quantities

        // If the length is customized, this parameter may have to be moved into the private variables
        const double incrementLength = (halfLength * 2) / horizontalWireQuantity; // gap between vertical wires
        double coordinate = centerCoordinates[2] - halfLength;

        const double gap = (outerRadius - innerRadius) / verticalWireQuantity; // gap between horizontal wires

        double iR = innerRadius;
        double oR = innerRadius + gap / 2;
        // Horizontal wires set
        for (int i = 0; i < horizontalWireQuantity; i++) { // center coordinate increases

            for (int j = 0; j < verticalWireQuantity; j++) { // radius is increased in this loop
                unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], coordinate});
                parameters.push_back(mu2e::TubsParams(iR, oR, incrementLength / 8));
                iR += gap;
                oR += gap;
            }
            // The Root program creates uneven increment so the center length is increased based on the number of horizontal wires
            // coordinates are increased evenly based on the number of horizontal sets of wires
            coordinate += incrementLength * (1 + 1 / double(horizontalWireQuantity));
        }

        // connecting wires
        // a potential algorithm could be added where there isn't an increment of connecting wires

        // resets radius parameters
        iR = innerRadius + gap / 2;
        oR = innerRadius + gap;
        for (int i = 0; i < verticalWireQuantity; i++) { // radius increases

            for (int j = 0; j < verticalWireQuantity; j++) { // span increases

                double phi0 = j * 2 * 3.1415 / verticalWireQuantity;

                unionCenter.push_back({centerCoordinates[0], centerCoordinates[1], centerCoordinates[2]});
                parameters.push_back(mu2e::TubsParams(iR, oR, halfLength + incrementLength / 8, phi0, phiTotal));
            }
            iR += gap;
            oR += gap;
        }

        // memory deallocation
      
    }
};
