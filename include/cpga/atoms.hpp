#include <caf/atom.hpp>

using namespace caf;

namespace cpga {
namespace atoms {
// Atoms used by global model actors
using init_population = atom_constant<atom("ip")>;
using execute_phase_1 = atom_constant<atom("exp1")>;
using execute_phase_2 = atom_constant<atom("exp2")>;
using execute_phase_3 = atom_constant<atom("exp3")>;
using execute_phase_4 = atom_constant<atom("exp4")>;
using compute_fitness = atom_constant<atom("cf")>;
using finish = atom_constant<atom("f")>;

// Atoms used by island model actors
using execute_migration = atom_constant<atom("mireq")>;
using execute_generation = atom_constant<atom("exge")>;
using receive_migration = atom_constant<atom("mirec")>;
using worker_finished = atom_constant<atom("wof")>;
using finish_worker = atom_constant<atom("fiw")>;
using dispatcher_finished = atom_constant<atom("dif")>;
using assign_id = atom_constant<atom("assi")>;

// Atoms used by grid model actors
using execute_computation = atom_constant<atom("excomp")>;

// Atoms used by reporter actors
using init_reporter = atom_constant<atom("ir")>;
using report = atom_constant<atom("re")>;
using report_individual = atom_constant<atom("rein")>;
using report_population = atom_constant<atom("repo")>;
using report_new_line = atom_constant<atom("renl")>;
using note_start = atom_constant<atom("nst")>;
using note_end = atom_constant<atom("nen")>;
using exit_reporter = atom_constant<atom("er")>;

// Atoms used by cluster nodes
using stage_discover_reporters = atom_constant<atom("rinf")>;
using stage_discover_workers = atom_constant<atom("winf")>;
using stage_collect_workers = atom_constant<atom("wpin")>;
using stage_distribute_reporter_info = atom_constant<atom("swre")>;
using stage_initiate_worker_node = atom_constant<atom("siwn")>;
using message_bus_receive = atom_constant<atom("mbr")>;
}
}