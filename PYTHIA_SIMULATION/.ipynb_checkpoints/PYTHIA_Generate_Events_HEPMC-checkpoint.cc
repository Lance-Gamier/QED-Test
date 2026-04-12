#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include "Pythia8/Pythia.h"
#include "HepMC3/GenEvent.h"
#include "HepMC3/GenParticle.h"
#include "HepMC3/FourVector.h"
#include "HepMC3/WriterAscii.h"

std::unordered_map<std::string, std::string> particle_mapping_dictionary
{
    {"electron", "11"},
    {"photon", "22"}
};

int main() {
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
    
    int nevents = 1e4;

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

    int a = std::abs(idA);
    int b = std::abs(idB);
    if ((a >= 10 && a < 20) && (b >= 10 && b < 20)) {
        subfolder = "Moller_like";
    } else if (a >= 20 || b >= 20) {
        subfolder = "Compton_like";
    }
    
    HepMC3::WriterAscii writer(("HEPMC_Results/" + subfolder + "/Simulation_" + idA_input + "_and_" + idB_input + ".hepmc").c_str());

    for (int i = 0; i < nevents; i++) 
    {
        if (!pythia.next()) continue;

        HepMC3::GenEvent event;

        // Convert Pythia event to HepMC format
        for (int j = 0; j < pythia.event.size(); j++) {
            Pythia8::Particle& p = pythia.event[j];
            
            // Create HepMC particle
            HepMC3::GenParticlePtr hep_particle = std::make_shared<HepMC3::GenParticle>(
                HepMC3::FourVector(p.px(), p.py(), p.pz(), p.e()),
                p.id(),
                p.status()
            );
            
            // Add the particle to the HepMC event
            event.add_particle(hep_particle);
        }

        // Write the event to the HepMC file
        writer.write_event(event);

        std::cout << "Event " << i << " written to HEPMC file" << std::endl;
    }

    std::cout << "All events saved" << std::endl;
    return 0;
}