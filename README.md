# Pheno
This is the program used for the collider analysis in "[Exotic Lepton-Flavor Violating Higgs Decays](http://arxiv.org/abs/arXiv:1910.07533)" project.

## Info 

  *Exotic Lepton-Flavor Violating Higgs Decays*\
  Jared A. Evans, Philip Tanedo, and Mohammadreza Zakeri\
  http://arxiv.org/abs/arXiv:1910.07533\


If you use this package please cite:
```
  Mohammadreza Zakeri. (2020, January 9).
  ZAKI1905/Pheno v1.0.1 (Version v1.0.1). Zenodo.
  http://doi.org/10.5281/zenodo.3603138
```

For BibTeX style citation use:
```
  @software{mohammadreza_zakeri_2019_3482349,
    author       = {Mohammadreza Zakeri},
    title        = {ZAKI1905/Pheno: Beta-Release},
    month        = oct,
    year         = 2019,
    publisher    = {Zenodo},
    version      = {v1.0.0-beta},
    doi          = {10.5281/zenodo.3482349},
    url          = {https://doi.org/10.5281/zenodo.3482349}
  }
```

-Last updated January 3, 2020

## Python Modules
You can setup the python program following these steps:
  1. Add the C++ pythia script file into "~/main/"
  2. Compile the C++ pythia script following the procedure below
  3. Move your model file ("UFO") inside "~/python/Model_Files/"
  4. Edit "~/python/interface.py" to input a new channel, and 
    define the directories for MadGraph, etc.
  5. Edit "~/python/run_par.py" for changing the MadGraph
    run parameters.
  6. You can also edit the work directory, 
    and other settings in "~/python/pheno.py"

You can then run the program by running "python  ~/python/pheno.py"


## Compiling the C++ files
To make the binary files:
  1. Choose the main c++ file (which is inside "~/main") to be compiled, by typing it in front of "MAIN" & "LINK_TARGET" in the make file. 
  2. Run "make"
  
  For makefile help run: "make help"

## Running the C++ binaries (stand-alone)
You can also run the C++ binaries by themselves, if you already have lhe files and want to run pythia and fastjet.

To run the binaries:
  1. Put the LHE file(s) in the desired directory, with the following naming scheme:
  {"XYZ_0.lhe", "XYZ_1.lhe", "XYZ_2.lhe", ...}
  where all "XYZ.lhe" files are from the same process, but chopped for parallel computing by OMP.
  Note: If there is only one LHE file, still add "_0.lhe" to its name.

  Step 2 depends on the main c++ file format, for the ones inside "~/main" do

  2. run `./bin/[BINARY] [LHE_FILE] [NUM_EVENTS] [OMP_THREADS] [REPORT_BOOL]`
  where:

    "BINARY" stands for the name of the binary file (inside ~/bin)
    "LHE_FILE" is the path+name of the LHE file compared to the current path in terminal
    Note that the extra extensions {"_0", "_1", ...} should be omitted.

    "NUM_EVENTS" is the number of events in the LHE file
    "OMP_THREADS" is the number of threads to be requested from OMP
    "REPORT_BOOL" can be '0' (no cut, etc. reporting) or '1' (reports cuts, etc. in .txt files)

  Example: run `./bin/cms_8TeV lhe_dir/my_model.lhe 1000 2 0` in terminal, 
  where the lhe_dir contains 2 lhe files each having 500 events named as:
    {my_model_0.lhe, my_model_1.lhe}

To add your own main c++ file:
  1. write a c++ file with a main() function and put it inside "~/main".
  2. Add the name of the main file in the make file for compiling.
  3. Compile using "make" command
  4. A binary with the same name will be created inside "~/bin"
  5. Run as explained above.

## References
This package uses PYTHIA-8 and FastJet.

PYTHIA-8: 
  - T. Sjöstrand, S. Mrenna and P. Skands, JHEP05 (2006) 026, Comput. Phys. Comm. 178 (2008) 852.
  
FastJet: 
  - M. Cacciari, G.P. Salam and G. Soyez, Eur.Phys.J. C72 (2012) 1896 [arXiv:1111.6097].


