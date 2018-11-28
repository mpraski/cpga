# C++ parallel genetic algorithm framework

#### Directories
The source is contained in the /src directory.

The basic classes and definitions are found in core.hpp/cpp files.

Particular PGA models are defined in the /models directory.

Commonly used genetic operators (genetic classes) are defined in /operators directory.

Exemplary genetic operators (for one-max problem) are defined in /example directory.

A sample run of global model for the one-max problem is defined in main.cpp. The indivudual type is an array of boolean values (std::vector<bool>), fitness value is an interger.