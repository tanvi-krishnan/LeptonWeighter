#include <LeptonWeighter/Weighter.h>

//#define DEBUGWEIGHTER

namespace LW {

double Weighter::weight(Event& e) const{
    double generation_weight = 0;
    for(auto g : gv)
        generation_weight += (*g)(e);
    double flux=0;
    for(auto f : fv)
        flux += (*f)(e);
#ifdef DEBUGWEIGHTER
    std::cout << flux << " " << (*cs)(e) << " " << generation_weight << std::endl;
#endif
    if(generation_weight == 0)
        throw std::runtime_error("Out of declared generation phase space. Impossible event.");
    return flux*(*cs)(e)/generation_weight;
}

double Weighter::get_oneweight(Event& e) const{
    double generation_weight = 0;
    for(auto g : gv)
        generation_weight += (*g)(e);
    if(generation_weight == 0)
        throw std::runtime_error("Out of declared generation phase space. Impossible event.");
    return (*cs)(e)/generation_weight;
}

#if defined(NUS_FOUND)
double Weighter::get_effective_tau_weight(Event & e) const{
    // needs to be a muon-neutrino simulation
    //std::cout << "Begin eff. weight calculation" << std::endl;
    if(not(e.primary_type == ParticleType::NuMu or e.primary_type == ParticleType::NuMuBar)){
        return 0.0;
    }
    //std::cout << "Pass primary check" << std::endl;
    // needs to be a charged-current event
    if(not(e.final_state_particle_0 == ParticleType::MuMinus or e.final_state_particle_0 == ParticleType::MuPlus)){
        return 0.0;
    }
    //std::cout << "Pass secondary check" << std::endl;
    // first compute the generation bias assuming its a muon-neutrino
    double generation_weight = 0;
    for(auto g : gv){
        generation_weight += (*g)(e);
    }
    if(generation_weight == 0){
        throw std::runtime_error("Out of declared generation phase space. Impossible event.");
    }
    // now convert this to a tau neutrino to compute the physical probability
    Event e_tau = e;
    // redefine your neutrino vertex
    e_tau.primary_type = (e.primary_type == ParticleType::NuMu) ? ParticleType::NuTau : ParticleType::NuTauBar;
    e_tau.final_state_particle_0 = (e.primary_type == ParticleType::MuMinus) ? ParticleType::TauMinus : ParticleType::TauPlus;
    e_tau.final_state_particle_1 = ParticleType::Hadrons;
    double flux=0;
    for(auto f : fv){
        flux += (*f)(e);
    }
    double y_max = 1.0;
    double y_min = 0.0;
    AdaptiveQuad::Options intOpt;
    nusquids::TauDecaySpectra tds;
    double int_precision = 1.e-6;
    double eff_xs = AdaptiveQuad::integrate([&](double y_tau){
                      double Etau = (1.-y_tau)*e_tau.energy;
                      double Emu = (1.-e_tau.interaction_y)*e_tau.energy;
                      if(Emu > Etau)
                        return 0.0;
                      double dxs = cs->DoubleDifferentialCrossSection(e_tau.primary_type, e_tau.final_state_particle_0, e_tau.final_state_particle_1,
                                                                      e_tau.energy, e_tau.interaction_x, y_tau);
                      double dndz = tds.TauDecayToLepton(Etau,Emu)*tds.GetTauToLeptonBranchingRatio();
                      return dxs*dndz;
                    },
                    y_min, y_max, int_precision, &intOpt);
    if(intOpt.outOfTolerance){
        throw std::runtime_error("Integral when computing effective tau cross section did not achieve the requested accuracy.");
    }
    //std::cout << flux << " " << eff_xs << " " << generation_weight << std::endl;
    return flux*eff_xs/generation_weight;
}

#endif

} // namespace LW

