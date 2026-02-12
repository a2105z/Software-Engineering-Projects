// Python bindings for HelixForge using pybind11
// 
// Build with: pip install pybind11
// Then: c++ -O3 -Wall -shared -std=c++17 -fPIC $(python3 -m pybind11 --includes) \
//       helixforge_bindings.cpp -o helixforge$(python3-config --extension-suffix)

#ifdef USE_PYBIND11

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "../../src/phase1_core/sequence.hpp"
#include "../../src/phase2_evaluation/scoring.hpp"
#include "../../src/phase2_evaluation/constraint_engine.hpp"
#include "../../src/phase3_mutation/mutation_engine.hpp"
#include "../../src/phase4_optimization/optimizer.hpp"

namespace py = pybind11;
using namespace helixforge;

PYBIND11_MODULE(helixforge, m) {
    m.doc() = "HelixForge: High-performance biological sequence optimization";
    
    // ========================================================================
    // Phase 1: Core Sequence
    // ========================================================================
    
    py::enum_<SequenceType>(m, "SequenceType")
        .value("DNA", SequenceType::DNA)
        .value("RNA", SequenceType::RNA)
        .value("PROTEIN", SequenceType::PROTEIN)
        .value("UNKNOWN", SequenceType::UNKNOWN);
    
    py::class_<Sequence>(m, "Sequence")
        .def(py::init<std::string, SequenceType, std::string, std::string>(),
             py::arg("data"), py::arg("type"), 
             py::arg("id") = "", py::arg("description") = "")
        .def_static("from_dna", &Sequence::from_dna)
        .def_static("from_rna", &Sequence::from_rna)
        .def_static("from_protein", &Sequence::from_protein)
        .def_static("from_fasta", &Sequence::from_fasta)
        .def_property_readonly("data", &Sequence::data)
        .def_property_readonly("type", &Sequence::type)
        .def_property_readonly("id", &Sequence::id)
        .def_property_readonly("length", &Sequence::length)
        .def("gc_content", &Sequence::gc_content)
        .def("reverse", &Sequence::reverse)
        .def("complement", &Sequence::complement)
        .def("reverse_complement", &Sequence::reverse_complement)
        .def("transcribe", &Sequence::transcribe)
        .def("translate", &Sequence::translate, 
             py::arg("frame") = 0, py::arg("to_stop") = false)
        .def("to_fasta", &Sequence::to_fasta, py::arg("line_width") = 80);
    
    // ========================================================================
    // Phase 2: Evaluation
    // ========================================================================
    
    py::class_<ScoreReport>(m, "ScoreReport")
        .def("get_score", &ScoreReport::get_score)
        .def("total_score", &ScoreReport::total_score)
        .def("to_json", &ScoreReport::to_json);
    
    py::class_<ScoringEngine>(m, "ScoringEngine")
        .def(py::init<>())
        .def("register_metric", 
             py::overload_cast<const std::string&>(&ScoringEngine::register_metric))
        .def("evaluate", &ScoringEngine::evaluate);
    
    py::class_<ConstraintEngine>(m, "ConstraintEngine")
        .def(py::init<>())
        .def("add_constraint_from_string", &ConstraintEngine::add_constraint_from_string)
        .def("satisfies_all", &ConstraintEngine::satisfies_all);
    
    // ========================================================================
    // Phase 3: Mutation
    // ========================================================================
    
    py::class_<MutationConfig>(m, "MutationConfig")
        .def(py::init<>())
        .def_readwrite("substitution_rate", &MutationConfig::substitution_rate)
        .def_readwrite("insertion_rate", &MutationConfig::insertion_rate)
        .def_readwrite("deletion_rate", &MutationConfig::deletion_rate);
    
    py::class_<MutationResult>(m, "MutationResult")
        .def_readonly("mutated_sequence", &MutationResult::mutated_sequence)
        .def_readonly("constraint_satisfied", &MutationResult::constraint_satisfied);
    
    py::class_<MutationEngine>(m, "MutationEngine")
        .def(py::init<uint64_t>(), py::arg("seed") = 0)
        .def("set_config", &MutationEngine::set_config)
        .def("apply_random_mutation", &MutationEngine::apply_random_mutation)
        .def("apply_n_mutations", &MutationEngine::apply_n_mutations);
    
    // ========================================================================
    // Phase 4: Optimization
    // ========================================================================
    
    py::enum_<OptimizerType>(m, "OptimizerType")
        .value("HILL_CLIMBING", OptimizerType::HILL_CLIMBING)
        .value("SIMULATED_ANNEALING", OptimizerType::SIMULATED_ANNEALING)
        .value("BEAM_SEARCH", OptimizerType::BEAM_SEARCH)
        .value("GENETIC_ALGORITHM", OptimizerType::GENETIC_ALGORITHM);
    
    py::enum_<ObjectiveType>(m, "ObjectiveType")
        .value("MAXIMIZE_GC", ObjectiveType::MAXIMIZE_GC)
        .value("MAXIMIZE_CAI", ObjectiveType::MAXIMIZE_CAI)
        .value("MAXIMIZE_EXPRESSION", ObjectiveType::MAXIMIZE_EXPRESSION);
    
    py::class_<OptimizationConfig>(m, "OptimizationConfig")
        .def(py::init<>())
        .def_readwrite("max_iterations", &OptimizationConfig::max_iterations)
        .def_readwrite("verbose", &OptimizationConfig::verbose);
    
    py::class_<OptimizationResult>(m, "OptimizationResult")
        .def_readonly("best_sequence", &OptimizationResult::best_sequence)
        .def_readonly("best_score", &OptimizationResult::best_score)
        .def_readonly("iterations", &OptimizationResult::iterations)
        .def("to_json", &OptimizationResult::to_json);
    
    py::class_<Optimizer, std::shared_ptr<Optimizer>>(m, "Optimizer")
        .def("optimize", &Optimizer::optimize,
             py::arg("initial_sequence"),
             py::arg("objective"),
             py::arg("constraints") = nullptr)
        .def("set_config", &Optimizer::set_config);
    
    m.def("create_optimizer", &OptimizerFactory::create,
          py::arg("type"), py::arg("seed") = 0);
    
    m.def("create_objective", &ObjectiveFactory::create,
          py::arg("type"), py::arg("parameter") = 0.0);
    
    // ========================================================================
    // Convenience functions
    // ========================================================================
    
    m.def("optimize", 
        [](const Sequence& seq, const std::string& objective_name, 
           const std::string& algorithm_name, uint64_t seed) {
            auto optimizer = OptimizerFactory::create(
                OptimizerType::SIMULATED_ANNEALING, seed);
            auto objective = ObjectiveFactory::create(
                ObjectiveType::MAXIMIZE_EXPRESSION);
            return optimizer->optimize(seq, objective);
        },
        py::arg("sequence"),
        py::arg("objective") = "expression",
        py::arg("algorithm") = "anneal",
        py::arg("seed") = 42,
        "Optimize a sequence with simple interface"
    );
}

#else

// Stub main for when pybind11 is not available
int main() {
    return 0;
}

#endif // USE_PYBIND11
