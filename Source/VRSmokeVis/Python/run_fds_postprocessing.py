from fdsreader import Simulation, settings
import fdsreader.export as exp


input_dir = sys.argv[1]
output_dir = sys.argv[2]

settings.IGNORE_ERRORS = True
settings.DEBUG = False

sim = Simulation(input_dir)

print("Simulation-File:", exp.export_sim(sim, output_dir, ordering='F'))
