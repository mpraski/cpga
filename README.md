# C++ parallel genetic algorithm framework

#### Overview
The basic classes and definitions are found in common.(hpp|cpp) and /core/*.(hpp|cpp) files.

Particular PGA models (and their cluster implementations) are defined in the `/models` directory.

Commonly used genetic operators are defined in `/operators` directory.

Exemplary genetic operators (for one-max and code fault detection problems) are defined in `/example` directory.

A sample run of global model for the one-max problem is defined in `main.cpp`. The indivudual type is an array of boolean values (`std::vector<char>`), fitness value is an interger (`int`).

I will annotate the code properly using Doxygen once main development work is completed.
