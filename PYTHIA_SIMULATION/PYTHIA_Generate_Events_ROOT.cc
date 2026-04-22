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
    double px1, py1, pz1;
    double px2, py2, pz2;
    double m1, m2;

    // Create branches in the tree
    tree->Branch("event", &event, "event/I");
    tree->Branch("size", &size, "size/I");
    tree->Branch("no", &no, "no/I");
    tree->Branch("id", &id, "id/I");
    tree->Branch("m1", &m1, "m1/D");
    tree->Branch("px1", &px1, "px1/D");
    tree->Branch("py1", &py1, "py1/D");
    tree->Branch("pz1", &pz1, "pz1/D");
    tree->Branch("m2", &m2, "m2/D");
    tree->Branch("px2", &px2, "px2/D");
    tree->Branch("py2", &py2, "py2/D");
    tree->Branch("pz2", &pz2, "pz2/D");

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
    pythia.readString("PartonLevel:ISR = off");
    pythia.readString("PartonLevel:FSR = off");

    pythia.init();

    for(int i = 0; i < nevents; ++i)
    {
        if (!pythia.next()) continue;
    
        // Create a vector to store the particles we actually want (the final state)
        std::vector<int> finalParticles;
        for (int j = 0; j < pythia.event.size(); ++j) {
            if (pythia.event[j].isFinal()) {
                finalParticles.push_back(j);
            }
        }
    
        // Ensure we found at least 2 final-state particles
        if (finalParticles.size() < 2) continue;
    
        // Particle 1 (First scattered particle)
        int idx1 = finalParticles[0];
        px1 = pythia.event[idx1].px();
        py1 = pythia.event[idx1].py();
        pz1 = pythia.event[idx1].pz();
        m1  = pythia.event[idx1].m();
    
        // Particle 2 (Second scattered particle)
        int idx2 = finalParticles[1];
        px2 = pythia.event[idx2].px();
        py2 = pythia.event[idx2].py();
        pz2 = pythia.event[idx2].pz();
        m2  = pythia.event[idx2].m();
    
        tree->Fill();
    }

    // Write the tree to the ROOT file
    output->Write();
    output->Close();

    return 0;
}
