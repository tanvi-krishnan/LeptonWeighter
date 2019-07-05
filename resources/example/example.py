import LeptonWeighter as LW
import numpy as np
import tables
import argparse

parser = argparse.ArgumentParser(description='Run LeptonWeighter python example.')

detault_data_location="../data/"

parser.add_argument('-LIConfiguration')
parser.add_argument('-LIEvents')
parser.add_argument('--dsdxdy_nu_CC', default=detault_data_location+"dsdxdy-numu-N-cc-HERAPDF15NLO_EIG_central.fits")
parser.add_argument('--dsdxdy_nubar_CC', default=detault_data_location+"dsdxdy-numubar-N-cc-HERAPDF15NLO_EIG_central.fits")
parser.add_argument('--dsdxdy_nu_NC', default=detault_data_location+"dsdxdy-numu-N-nc-HERAPDF15NLO_EIG_central.fits")
parser.add_argument('--dsdxdy_nubar_NC', default=detault_data_location+"dsdxdy-numubar-N-nc-HERAPDF15NLO_EIG_central.fits")
parser.add_argument('--nuSQFluxKaon', default=detault_data_location+"HondaGaisserKaon.hdf5")
parser.add_argument('--nuSQFluxPion', default=detault_data_location+"HondaGaisserPion.hdf5")
parser.add_argument('--OutFile', default="out")

parser.set_defaults(use_cache=True)
args=parser.parse_args()

# setup lepton weighter

simulation_generators = LW.MakeGeneratorsFromLICFile(args.LIConfiguration)

pion_nusquids_flux = LW.nuSQUIDSAtmFlux(args.nuSQFluxKaon)
kaon_nusquids_flux = LW.nuSQUIDSAtmFlux(args.nuSQFluxPion)

xs = LW.CrossSectionFromSpline(args.dsdxdy_nu_CC,args.dsdxdy_nubar_CC,
                               args.dsdxdy_nu_NC,args.dsdxdy_nubar_NC)

weighter = LW.Weighter([pion_nusquids_flux,kaon_nusquids_flux],xs,simulation_generators)

# get some events
h5file = tables.open_file(args.LIEvents,"r")

# pour contents to arrays

for i,event in enumerate(h5file.root.EventProperties[:]):
    LWevent = LW.Event()
    # this numbers can be seen from the struct on the h5file
    LWevent.primary_type = LW.ParticleType(event[12])
    LWevent.final_state_particle_0 = LW.ParticleType(event[10])
    LWevent.final_state_particle_1 = LW.ParticleType(event[11])
    LWevent.interaction_x = event[8]
    LWevent.interaction_y = event[9]
    LWevent.energy = event[5]
    LWevent.zenith = event[6]
    LWevent.azimuth = event[7]
    LWevent.x = 0.
    LWevent.y = 0.
    LWevent.z = event[16]
    LWevent.radius = event[15]
    LWevent.total_column_depth = event[14]
    print(weighter(LWevent))

h5file.close()
