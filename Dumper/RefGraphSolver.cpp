#include "RefGraphSolver.h"

std::unordered_map<std::string, std::string> RefGraphSolver::typeDefMap;

std::vector<RefGraphSolver::Node*> RefGraphSolver::packageNodes;

std::unordered_map<std::string, RefGraphSolver::Node*> RefGraphSolver::nodesMap;