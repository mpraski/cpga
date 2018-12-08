#include "caf/atom.hpp"

using namespace caf;

// Atoms used by global model actors
using init_population = atom_constant<atom("ip")>;
using execute_phase_1 = atom_constant<atom("exp1")>;
using execute_phase_2 = atom_constant<atom("exp2")>;
using execute_phase_3 = atom_constant<atom("exp3")>;
using compute_fitness = atom_constant<atom("cf")>;
using finish = atom_constant<atom("f")>;

// Atoms used by island model actors
using migrate_request = atom_constant<atom("mireq")>;
using migrate_receive = atom_constant<atom("mirec")>;
using migrate = atom_constant<atom("mir")>;
using execute_generation = atom_constant<atom("exge")>;
using worker_finished = atom_constant<atom("wof")>;
using dispatcher_finished = atom_constant<atom("dif")>;

// Atoms used by reporter actors
using init_reporter = atom_constant<atom("ir")>;
using report = atom_constant<atom("re")>;
using report_info = atom_constant<atom("rei")>;
using report_individual = atom_constant<atom("rein")>;
using report_population = atom_constant<atom("repo")>;
using report_new_line = atom_constant<atom("renl")>;
using note_start = atom_constant<atom("nst")>;
using note_end = atom_constant<atom("nen")>;
using exit_reporter = atom_constant<atom("er")>;
