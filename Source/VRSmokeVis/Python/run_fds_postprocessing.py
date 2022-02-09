import sys
import os
import yaml
from fdsreader import Simulation, settings
import fdsreader.export as exp


input_dir = sys.argv[1]
output_dir = sys.argv[2]

settings.IGNORE_ERRORS = True
settings.DEBUG = False

sim = Simulation(input_dir)
meta = {"Obstructions": list(), "Slices": list(), "Volumes": list()}

for obst in sim.obstructions:
    exp.export_obst_raw(obst, os.path.join(output_dir, "obst"), 'F')
    meta["Obstructions"].append(obst)

for slc in sim.slices:
    exp.export_slcf_raw(slc, os.path.join(output_dir, "slices", slc.quantity.name.replace(' ', '_').lower()), 'F')
    meta["Slices"].append(slc)

for smoke in sim.smoke_3d:
    exp.export_smoke_raw(smoke, os.path.join(output_dir, "smoke", smoke.quantity.name.replace(' ', '_').lower()), 'F')
    meta["Volumes"].append(smoke)

with open(os.path.join(base_path, "fds_post", case), 'w') as metafile:
    yaml.dump(meta, metafile)
        
print("Success")
