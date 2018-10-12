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

} // namespace LW

