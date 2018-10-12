#include <iostream>
#include <fstream>
#include <boost/detail/endian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <iterator>
#include <set>
#include "LeptonWeighter/Weighter.h"

//==============================================================================================
//==============================================================================================

int main(int argc, char ** argv) {
    if(argc!=2)
        throw std::runtime_error("usage: configure.lic");

    std::string configuration_filename(argv[1]);
    std::cout << "single mode" << std::endl;
    LW::RangeSimulationDetails rsd(configuration_filename);
    std::cout << "multi mode" << std::endl;
    std::vector<std::shared_ptr<LW::Generator>> generators = LW::MakeGeneratorsFromLICFile(configuration_filename);
    std::cout << "victory: " << generators.size() << std::endl;

    for (auto g : generators){
      auto x = std::dynamic_pointer_cast<LW::RangeGenerator>(g)->GetSimulationDetails();

      std::cout << x.Get_NumberOfEvents() << std::endl;
      std::cout << x.Get_MinEnergy() << std::endl;
      std::cout << x.Get_MaxEnergy() << std::endl;
      std::cout << x.Get_MinZenith() << std::endl;
      std::cout << x.Get_MaxZenith() << std::endl;
      std::cout << x.Get_MinAzimuth() << std::endl;
      std::cout << x.Get_MaxAzimuth() << std::endl;
      std::cout << x.Get_PowerLawIndex() << std::endl;
    }

    return 0;
}

