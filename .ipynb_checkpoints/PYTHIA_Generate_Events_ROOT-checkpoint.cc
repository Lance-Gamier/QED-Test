#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include "TFile.h"
#include "TTree.h"
#include "Pythia8/Pythia.h"

std::unordered_map <std::string, std::string> particle_mapping_dictionary
{
    {"electron", "11"},
    {"photon", "22"}
};

int main()
{
    std::string idA_input, idB_input, subfolder;

    std::cout << "Fermions:\n";
    for (const auto& [name, code] : particle_mapping_dictionary)
    {
        int pdg = std::abs(std::stoi(code));
        if (pdg >= 10 && pdg < 20)
        {
            std::cout << "  " << name << '\n';
        }
    }
    
    std::cout << "\nBosons:\n";
    for (const auto& [name, code] : particle_mapping_dictionary)
    {
        int pdg = std::abs(std::stoi(code));
        if (pdg >= 20)
        {
            std::cout << "  " << name << '\n';
        }
    }

    std::cout << "Enter the name of the first particle: ";
    std::cin >> idA_input;
    
    std::cout << "Enter the name of the second particle: ";
    std::cin >> idB_input;

    auto idA_str = particle_mapping_dictionary.find(idA_input);
    auto idB_str = particle_mapping_dictionary.find(idB_input);
    
    int idA = std::stoi(idA_str->second);
    int idB = std::stoi(idB_str->second);
    
    // Open ROOT file to store the tree
    int a = std::abs(idA);
    int b = std::abs(idB);
    if ((a >= 10 && a < 20) && (b >= 10 && b < 20)) {
        subfolder = "Moller_like";
    } else if (a >= 20 || b >= 20) {
        subfolder = "Compton_like";
    }
    
    TFile *output = new TFile(("ROOT_Results/" + subfolder +"/Simulation_" + idA_input + "_and_" + idB_input + ".root").c_str(), "recreate");

    // Create a TTree to hold the data
    TTree *tree = new TTree("tree", "tree");

    // Declare variables for branches
    int id, event, size, no;
    double m, px, py, pz;

    // Create branches in the tree
    tree->Branch("event", &event, "event/I");
    tree->Branch("size", &size, "size/I");
    tree->Branch("no", &no, "no/I");
    tree->Branch("id", &id, "id/I");
    tree->Branch("m", &m, "m/D");
    tree->Branch("px", &px, "px/D");
    tree->Branch("py", &py, "py/D");
    tree->Branch("pz", &pz, "pz/D");

    int nevents = 1e4;

    // Initialize Pythia
    Pythia8::Pythia pythia;

    pythia.readString("Beams:idA = " + std::to_string(idA));
    pythia.readString("Beams:idB = " + std::to_string(idB));
    pythia.readString("Beams:eCM = 14.e3");
    pythia.readString("SoftQCD:all = off");
    pythia.readString("HardQCD::all = off");
    pythia.readString("WeakBosonExchange:all = on");
    pythia.readString("WeakZ0:gmZmode = 0");
    // 1. Force the photon to be point-like (Direct)
    pythia.readString("Photon:ProcessType = 1"); // 1 = Direct processes only
    // 2. Disable the resolved hadronic components
    pythia.readString("Photon:resolved = off");
    // 3. Specifically enable Compton scattering (e gamma -> e gamma)
    pythia.readString("PromptPhoton:qg2qgamma = on"); // Note: In ee/eg contexts, this maps to Compton
    // OR use the specific electroweak flag:
    // pythia.readString("WeakBosonExchange:ff2ff(t:gmZ) = on");

    Pythia8::Hist hpz("Momentum Distribution", 100, -10, 10);

    pythia.init();

    // Loop over events
    for(int i = 0; i < nevents; i++)
    {
        if(!pythia.next()) continue;  // Skip the event if it's not successful
        int entries = pythia.event.size();  // Get the number of particles in the event
        
        event = i;
        size = entries;

        // Loop over particles in the event
        for(int j = 0; j < entries; j++)
        {
            id = pythia.event[j].id();  // Fix: Get the particle ID, not mass

            no = j;   // Particle index

            m = pythia.event[j].m();   // Particle mass
            px = pythia.event[j].px();  // Particle px
            py = pythia.event[j].py();  // Particle py
            pz = pythia.event[j].pz();  // Particle pz

            hpz.fill(pz);  // Fill histogram with pz value
        }

        tree->Fill();  // Fill the tree with the current event's data
    }

    // Write the tree to the ROOT file
    output->Write();
    output->Close();

    return 0;
}
