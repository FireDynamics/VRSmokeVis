import sys
from fdsreader import Simulation, settings
import fdsreader.export as exp


def main():
    input_file = sys.argv[1]
    output_dir = sys.argv[2]
    
    settings.IGNORE_ERRORS = True
    settings.DEBUG = False
    
    sim = Simulation(input_file)
    
    print(exp.export_sim(sim, output_dir, ordering='F'))


if __name__ == "__main__":
    main()
