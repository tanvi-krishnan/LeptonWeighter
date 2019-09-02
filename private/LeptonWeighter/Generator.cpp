#include <LeptonWeighter/Generator.h>
#include <LeptonWeighter/Constants.h>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <cmath>

//#define DEBUGPROBABILITY

namespace LW {

double Generator::probability(Event& e) const {
    double p;
    p = probability_e(e.energy);
#ifdef DEBUGPROBABILITY
    std::cout << std::endl << "Probability components" << std::endl;
    std::cout << "pe " << probability_e(e.energy) << std::endl;
#endif
    if(p==0)
        return 0;
    p *= probability_dir(e.zenith,e.azimuth);
#ifdef DEBUGPROBABILITY
    std::cout << "pdir " << probability_dir(e.zenith,e.azimuth) << " " << e.zenith << " " << std::endl;
#endif
    if(p==0)
        return 0;
    p *= probability_area();
#ifdef DEBUGPROBABILITY
    std::cout << "parea " << probability_area() << std::endl;
#endif
    if(p==0)
        return 0;
    p *= probability_pos(e.x,e.y,e.z);
#ifdef DEBUGPROBABILITY
    std::cout << "ppos " << probability_pos(e.x,e.y,e.z) << std::endl;
#endif
    if(p==0)
        return 0;
#ifdef DEBUGPROBABILITY
    std::cout << "pfs " << probability_final_state(e.final_state_particle_0,e.final_state_particle_1) << std::endl;
    std::cout << "pint " << probability_interaction(e.energy,e.interaction_x,e.interaction_y) << std::endl;
#endif
    return p*probability_stat()*probability_final_state(e.final_state_particle_0,e.final_state_particle_1)*
        probability_interaction(e.energy,e.interaction_y)*probability_interaction(e.energy,e.interaction_x,e.interaction_y)/number_of_targets(e);
}

double Generator::probability_final_state(ParticleType final_state_particle_0_,ParticleType final_state_particle_1_) const{
    if(sim_details.Get_ParticleType1() == final_state_particle_1_ and sim_details.Get_ParticleType0() == final_state_particle_0_)
        return 1.;
    else if (sim_details.Get_ParticleType0() == final_state_particle_1_ and sim_details.Get_ParticleType1() == final_state_particle_0_)
        return 1.;
    else
        return 0.;
}

double Generator::probability_e(double energy) const {
    double norm=0;
    double powerlawIndex=sim_details.Get_PowerLawIndex();
    double energyMin=sim_details.Get_MinEnergy();
    double energyMax=sim_details.Get_MaxEnergy();

    if(energy>energyMax or energy<energyMin)
        return 0;

    if(powerlawIndex!=1)
        norm=(1-powerlawIndex)/(pow(energyMax,1-powerlawIndex)-pow(energyMin,1-powerlawIndex));
    else if(powerlawIndex==1)
        norm=1./log(energyMax/energyMin);
    return (norm*pow(energy,-powerlawIndex));
}

double Generator::probability_dir(double zenith, double azimuth) const {
    if(zenith>sim_details.Get_MaxZenith() or zenith<sim_details.Get_MinZenith())
        return 0;
    if(azimuth>sim_details.Get_MaxAzimuth() or azimuth<sim_details.Get_MinAzimuth())
        return 0;
    return 1./((sim_details.Get_MaxAzimuth()-sim_details.Get_MinAzimuth())*(cos(sim_details.Get_MinZenith())-cos(sim_details.Get_MaxZenith())));
}

double Generator::probability_stat() const {
    return (sim_details.Get_NumberOfEvents());
}

double Generator::probability_interaction(double enu, double y) const {
  return 1.;
  /*
   *  EXPERIMENTAL GLASHOW SAMPLER -- NOT USED
    if(deduceInitialType(sim_details.Get_ParticleType0(),sim_details.Get_ParticleType1()) == ParticleType::NuEBar){
      using namespace nusquids;
      // GR support comes from nusquids. CAD
      double GeV = 1.e9;
      double e_out = (1-y)*enu;
      double total_xs = grxs.TotalCrossSection(e_out*GeV,
              NeutrinoCrossSections::electron,NeutrinoCrossSections::antineutrino,NeutrinoCrossSections::GR);
      double diff_xs = grxs.SingleDifferentialCrossSection(enu*GeV,e_out*GeV,
              NeutrinoCrossSections::electron,NeutrinoCrossSections::antineutrino,NeutrinoCrossSections::GR);
      std::cout << "xsr" << diff_xs/total_xs << std::endl;
      return diff_xs/total_xs;
    } else {
      return 1.;
    }
  */
}

double Generator::probability_interaction(double enu, double x,double y) const {
    // DIS cross sections assumes all flavors to be equal in cross sections
    int centerbuffer[3];
    double xx[3];

    xx[0] = log10(enu);
    xx[1] = log10(x);
    xx[2] = log10(y);

    double differential_xs, total_xs;
    if(sim_details.Get_DifferentialSpline()->searchcenters(xx,centerbuffer))
        differential_xs = pow(10.0,sim_details.Get_DifferentialSpline()->ndsplineeval(xx,centerbuffer,0));
    else
        throw std::runtime_error("Could not evaluate total neutrino cross section spline.");
    if(sim_details.Get_TotalSpline()->searchcenters(xx,centerbuffer))
        total_xs = pow(10.0,sim_details.Get_TotalSpline()->ndsplineeval(xx,centerbuffer,0));
    else
        throw std::runtime_error("Could not evaluate total neutrino cross section spline.");

    return differential_xs/total_xs;
}

double RangeGenerator::probability_area() const {
    return 1./(1e4*M_PI*range_sim_details.Get_InjectionRadius()*range_sim_details.Get_InjectionRadius());
}

RangeSimulationDetails RangeSimulationDetails::ReadFromFile(std::string configuration_filename){
    std::ifstream is(configuration_filename,std::ios::binary);
    BlockHeader h;
    // read first header
    is >> h;
    if(h.block_name != "EnumDef")
        throw std::runtime_error("LW::RangeSimulationDetails: Configuration file does not have a particle enumerator definitions.");
    if(not is.good() or is.eof())
        throw std::runtime_error("LW::RangeSimulationDetails: Configuration file error while reading.");
    EnumDefBlock edb;
    is >> edb;
    if(not CheckParticleEnumeration(edb))
        throw std::runtime_error("LW::RangeSimulationDetails: Configuration file particle enumeration are not compatible with current definitions. Alarm.");
    if(not is.good() or is.eof())
        throw std::runtime_error("LW::RangeSimulationDetails: Configuration file error while reading.");
    is >> h;
    if(h.block_name != "RangedInjectionConfiguration")
        throw std::runtime_error("LW::RangeSimulationDetails: Expected RangedInjectionConfiguration configuration block after enum definitions, but got " + h.block_name);
    RangedInjectionConfiguration ric;
    is >> ric;
    is.close();

    return MakeFromRangeInjectorConfiguration(ric);
}

RangeSimulationDetails RangeSimulationDetails::MakeFromRangeInjectorConfiguration(RangedInjectionConfiguration ric) {
    using splinetable_=photospline::splinetable<>;
    std::shared_ptr<splinetable_> differentialCrossSectionData = std::make_shared<splinetable_>();
    differentialCrossSectionData->read_fits_mem(ric.differentialCrossSectionData.data(),ric.differentialCrossSectionData.size());
    std::shared_ptr<splinetable_> totalCrossSectionData = std::make_shared<splinetable_>();
    totalCrossSectionData->read_fits_mem(ric.totalCrossSectionData.data(),ric.totalCrossSectionData.size());

    return RangeSimulationDetails(ric.injectionRadius,ric.injectionCap,
            ric.number_of_events,
            ric.final_state_particle_0,ric.final_state_particle_1,
            differentialCrossSectionData,totalCrossSectionData,
            0,// legacy. CAD
            ric.azimuthMin,ric.azimuthMax,ric.zenithMin,ric.zenithMax,ric.energyMin,ric.energyMax,ric.powerlawIndex);
}

VolumeSimulationDetails VolumeSimulationDetails::ReadFromFile(std::string configuration_filename){
    std::ifstream is(configuration_filename,std::ios::binary);
    BlockHeader h;
    // read first header
    is >> h;
    if(h.block_name != "EnumDef")
        throw std::runtime_error("LW::VolumeSimulationDetails: Configuration file does not have a particle enumerator definitions.");
    if(not is.good() or is.eof())
        throw std::runtime_error("LW::VolumeSimulationDetails: Configuration file error while reading.");
    EnumDefBlock edb;
    is >> edb;
    if(not CheckParticleEnumeration(edb))
        throw std::runtime_error("LW::VolumeSimulationDetails: Configuration file particle enumeration are not compatible with current definitions. Alarm.");
    if(not is.good() or is.eof())
        throw std::runtime_error("LW::VolumeSimulationDetails: Configuration file error while reading.");
    is >> h;
    if(h.block_name != "VolumeSimulationDetails")
        throw std::runtime_error("LW::VolumeSimulationDetails: Expected VolumeSimulationDetails configuration block after enum definitions, but got " + h.block_name);
    VolumeInjectionConfiguration vic;
    is >> vic;
    is.close();

    return MakeFromVolumeInjectorConfiguration(vic);
}

VolumeSimulationDetails VolumeSimulationDetails::MakeFromVolumeInjectorConfiguration(VolumeInjectionConfiguration vic){
    using splinetable_=photospline::splinetable<>;
    std::shared_ptr<splinetable_> differentialCrossSectionData = std::make_shared<splinetable_>();
    differentialCrossSectionData->read_fits_mem(vic.differentialCrossSectionData.data(),vic.differentialCrossSectionData.size());
    std::shared_ptr<splinetable_> totalCrossSectionData = std::make_shared<splinetable_>();
    totalCrossSectionData->read_fits_mem(vic.totalCrossSectionData.data(),vic.totalCrossSectionData.size());

    return VolumeSimulationDetails(vic.cylinderRadius,vic.cylinderHeight,
            vic.number_of_events,
            vic.final_state_particle_0,vic.final_state_particle_1,
            differentialCrossSectionData,totalCrossSectionData,
            0,// legacy. CAD
            vic.azimuthMin,vic.azimuthMax,vic.zenithMin,vic.zenithMax,vic.energyMin,vic.energyMax,vic.powerlawIndex);
}

double VolumeGenerator::probability_pos(double x, double y, double z) const {
    if(z>vol_sim_details.Get_CylinderHeight())
        return 0;

    double r = sqrt(x*x + y*y);
    if(r>vol_sim_details.Get_CylinderRadius())
        return 0;

    return 1/(1e4*M_PI*vol_sim_details.Get_CylinderRadius()*vol_sim_details.Get_CylinderRadius()*vol_sim_details.Get_CylinderHeight());
}

double RangeGenerator::number_of_targets(Event& e) const {
    return Constants::Na*e.total_column_depth;
}

double VolumeGenerator::number_of_targets(Event& e) const {
    return Constants::Na*e.total_column_depth;
}

std::vector<std::shared_ptr<Generator>> MakeGeneratorsFromLICFile(std::string configuration_filename){
    std::ifstream is(configuration_filename,std::ios::binary);
    if(!is.good())
      throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Configuration file " + configuration_filename + " does not exist or its corrupted.");
    is.seekg (0, is.end);
    int length = is.tellg();
    is.seekg (0, is.beg);
    std::vector<std::shared_ptr<Generator>> generator_vector;

    // read first header // expect particle enumerator definitions
    BlockHeader h;
    is >> h;
    if(h.block_name != "EnumDef")
        throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Configuration file does not have a particle enumerator definitions. Got block " + h.block_name);
    if(not is.good() or is.eof())
        throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Configuration file error while reading.");
    EnumDefBlock edb;
    is >> edb;

    // Trust
    //if(not CheckParticleEnumeration(edb))
    //    throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Configuration file particle enumeration are not compatible with current definitions. Alarm.");

    while((not is.eof()) and is.good() and not (length==is.tellg())){
      // get all the simulations generators on the file
      if(not is.good() or is.eof())
          throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Configuration file error while reading.");
      try {
        is >> h;
      } catch (std::runtime_error e){
        std::cout << e.what() << std::endl;
        break;
      }

      if(h.block_name == "RangedInjectionConfiguration"){
        RangedInjectionConfiguration ric;
        is >> ric;
        generator_vector.push_back(std::make_shared<RangeGenerator>(RangeSimulationDetails::MakeFromRangeInjectorConfiguration(ric)));

      } else if (h.block_name == "VolumeInjectionConfiguration"){
        VolumeInjectionConfiguration vic;
        is >> vic;
        generator_vector.push_back(std::make_shared<VolumeGenerator>(VolumeSimulationDetails::MakeFromVolumeInjectorConfiguration(vic)));
      } else {
        throw std::runtime_error("LW::MakeGeneratorsFromLICFile: Expected either VolumeSimulationDetails or RangedInjectionConfiguration block after enum definitions, but got " + h.block_name);
      }
    }

    is.close();
    return generator_vector;
}

/// print stuff
//std::ostream& operator<<(std::ostream& os, RangeSimulationDetails& e) {
//  return e.
//}
//std::ostream& operator<<(std::ostream& os, VolumeSimulationDetails& e);

} // namespace LW

