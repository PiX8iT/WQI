/**
 * @file main.cpp
 * @brief Yamuna River Water Quality & Effluent Load Monitoring System (SDG 6)
 * @course B.Tech 3rd Semester - Object-Oriented Programming using C++ Lab
 *
 * Demonstrates: Encapsulation, Inheritance, Runtime Polymorphism,
 * Operator Overloading (+), Custom Exception Handling, and File I/O (fstream).
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <memory>

using namespace std;

// Custom Exception Class for Ecological Dead Zones (DO < 1.0 mg/L)
class EcologicalDeadZoneException : public runtime_error {
public:
    explicit EcologicalDeadZoneException(const string& stationName, double doLevel)
        : runtime_error("[EXCEPTION ALERT] Critical Anoxia at " + stationName +
                        " (DO: " + to_string(doLevel).substr(0, 3) + " mg/L < 1.0 mg/L threshold)") {}
};

// Abstract Base Class representing a Hydrological Monitoring Station
class RiverStation {
protected:
    string stationName;
    double pH;
    double dissolvedOxygen;   // mg/L (CPCB Safe >= 5.0)
    double bod;               // Biochemical Oxygen Demand in mg/L (CPCB Safe <= 3.0)
    int totalColiform;        // MPN/100ml (CPCB Safe <= 2500)
    string frothIndex;        // Surfactant/Phosphate foam severity

public:
    RiverStation(string name, double phVal, double doVal, double bodVal, int coliform, string froth)
        : stationName(name), pH(phVal), dissolvedOxygen(doVal),
          bod(bodVal), totalColiform(coliform), frothIndex(froth) {}

    virtual ~RiverStation() {}

    // Pure Virtual Functions for Runtime Polymorphism
    virtual string classifyCPCB() const = 0;
    virtual string getEcologicalStatus() const = 0;

    // Concrete method to check and throw exception if water is anoxic
    void verifyOxygenViability() const {
        if (dissolvedOxygen < 1.0) {
            throw EcologicalDeadZoneException(stationName, dissolvedOxygen);
        }
    }

    // Required Sewage Treatment Plant (STP) BOD reduction % to reach safe 3.0 mg/L
    double calculateRequiredBODReduction() const {
        if (bod <= 3.0) return 0.0;
        return ((bod - 3.0) / bod) * 100.0;
    }

    // Display formatted row in console and file stream
    void printRow(ostream& out) const {
        out << left << setw(19) << stationName
            << setw(6) << fixed << setprecision(1) << pH
            << setw(10) << dissolvedOxygen
            << setw(10) << bod
            << setw(11) << totalColiform
            << setw(11) << frothIndex
            << setw(12) << classifyCPCB()
            << getEcologicalStatus() << "\n";
    }

    // Getters
    string getName() const { return stationName; }
    double getDO() const { return dissolvedOxygen; }
    double getBOD() const { return bod; }
};

// Derived Class 1: Upstream Freshwater Zone (e.g., Palla, Wazirabad)
class UpstreamZone : public RiverStation {
public:
    UpstreamZone(string name, double phVal, double doVal, double bodVal, int coliform)
        : RiverStation(name, phVal, doVal, bodVal, coliform, "Low") {}

    string classifyCPCB() const override {
        if (dissolvedOxygen >= 6.0 && bod <= 2.0 && totalColiform <= 50) return "Class A";
        if (dissolvedOxygen >= 5.0 && bod <= 3.0 && totalColiform <= 2500) return "Class B";
        if (dissolvedOxygen >= 4.0 && bod <= 3.0 && totalColiform <= 5000) return "Class C";
        return "Class D";
    }

    string getEcologicalStatus() const override {
        return (bod <= 2.0) ? "Safe" : "Moderate";
    }
};

// Derived Class 2: Urban Industrial & Sewage Effluent Zone (e.g., ITO, Nizamuddin, Okhla)
class UrbanEffluentZone : public RiverStation {
public:
    UrbanEffluentZone(string name, double phVal, double doVal, double bodVal, int coliform, string froth)
        : RiverStation(name, phVal, doVal, bodVal, coliform, froth) {}

    string classifyCPCB() const override {
        if (dissolvedOxygen >= 4.0 && bod <= 6.0) return "Class D";
        if (dissolvedOxygen >= 2.0 && bod <= 10.0) return "Class E";
        return "Below-E";
    }

    string getEcologicalStatus() const override {
        if (frothIndex == "Critical") return "[TOXIC FOAM]";
        if (dissolvedOxygen < 1.0) return "[DEAD ZONE]";
        return "[POLLUTED]";
    }

    // Operator Overloading (+): Simulate merging a major untreated drain into a river segment
    UrbanEffluentZone operator+(const UrbanEffluentZone& drain) const {
        string mergedName = this->stationName + " + " + drain.stationName;
        double combinedPH = (this->pH + drain.pH) / 2.0;
        // Flow-weighted degradation of DO and elevation of BOD
        double combinedDO = max(0.0, (this->dissolvedOxygen * 0.35) + (drain.dissolvedOxygen * 0.65));
        double combinedBOD = (this->bod * 0.35) + (drain.bod * 0.65);
        int combinedColiform = this->totalColiform + drain.totalColiform;
        return UrbanEffluentZone(mergedName, combinedPH, combinedDO, combinedBOD, combinedColiform, "High");
    }
};

int main() {
    vector<unique_ptr<RiverStation>> stations;

    // Populate monitoring stations along the Yamuna River
    stations.push_back(make_unique<UpstreamZone>("Palla (Inflow)", 7.4, 6.8, 1.8, 1100));
    stations.push_back(make_unique<UpstreamZone>("Wazirabad Barrage", 7.6, 5.2, 2.9, 2400));
    stations.push_back(make_unique<UrbanEffluentZone>("ITO Bridge", 7.9, 0.8, 26.5, 24000, "High"));
    stations.push_back(make_unique<UrbanEffluentZone>("Nizamuddin Bridge", 8.0, 0.4, 29.0, 31000, "High"));
    stations.push_back(make_unique<UrbanEffluentZone>("Okhla Barrage", 8.2, 0.0, 34.0, 38000, "Critical"));

    ofstream reportFile("Yamuna_Audit_Report.txt");

    auto printHeader = [](ostream& out) {
        out << "========================================================================================\n";
        out << "           YAMUNA RIVER WATER QUALITY & EFFLUENT MONITORING SYSTEM (SDG 6)\n";
        out << "========================================================================================\n";
        out << left << setw(19) << "Station"
            << setw(6) << "pH"
            << setw(10) << "DO(mg/L)"
            << setw(10) << "BOD(mg/L)"
            << setw(11) << "Coliform"
            << setw(11) << "Froth-Idx"
            << setw(12) << "CPCB Class"
            << "Status\n";
        out << "----------------------------------------------------------------------------------------\n";
    };

    printHeader(cout);
    if (reportFile.is_open()) printHeader(reportFile);

    // Polymorphic iteration across all stations
    for (const auto& station : stations) {
        station->printRow(cout);
        if (reportFile.is_open()) station->printRow(reportFile);
    }

    cout << "----------------------------------------------------------------------------------------\n";
    if (reportFile.is_open()) {
        reportFile << "----------------------------------------------------------------------------------------\n";
    }

    // Exception Handling verification for Ecological Dead Zones
    for (const auto& station : stations) {
        try {
            station->verifyOxygenViability();
        } catch (const EcologicalDeadZoneException& e) {
            cout << e.what() << "\n";
            if (reportFile.is_open()) reportFile << e.what() << "\n";
        }
    }

    // Operator Overloading (+) Simulation: Merging Najafgarh Drain into Wazirabad segment
    UrbanEffluentZone wazirabadSegment("Wazirabad", 7.6, 5.2, 2.9, 2400, "Low");
    UrbanEffluentZone najafgarhDrain("Najafgarh Drain", 8.1, 0.0, 32.9, 42000, "High");

    UrbanEffluentZone postMerge = wazirabadSegment + najafgarhDrain;

    cout << "\n[SIMULATION] Merging Najafgarh Drain into Wazirabad Segment (+ Operator Overloading)...\n";
    cout << "-> Post-Merge Combined DO : " << fixed << setprecision(1) << postMerge.getDO()
         << " mg/L | Combined BOD : " << postMerge.getBOD() << " mg/L\n";
    cout << "-> Required STP Treatment : " << setprecision(1) << postMerge.calculateRequiredBODReduction()
         << "% BOD reduction needed to restore Class B bathing standard.\n";

    if (reportFile.is_open()) {
        reportFile << "\n[SIMULATION] Post-Merge BOD: " << postMerge.getBOD()
                   << " mg/L | Required STP BOD Reduction: "
                   << postMerge.calculateRequiredBODReduction() << "%\n";
        reportFile.close();
        cout << "[FILE I/O] Official Audit Log exported successfully to 'Yamuna_Audit_Report.txt'.\n";
    }

    cout << "========================================================================================\n";
    return 0;
}