// @PhysicsData.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "ContactSolverReferenceService.h"
#include "ContactTesterReferenceService.h"

namespace base_engine::physics {
constexpr size_t kTesterTableMatrixSize = 2;
using ContactTesterReferenceServiceTable =
    std::array<std::array<std::unique_ptr<ContactTesterReferenceService>,
                          kTesterTableMatrixSize>,
               kTesterTableMatrixSize>;
constexpr size_t kSolverTableMatrixSize = 2;
using ContactSolverReferenceServiceTable =
    std::array<std::array<std::unique_ptr<ContactSolverReferenceService>,
                          kTesterTableMatrixSize>,
               kTesterTableMatrixSize>;
struct PhysicsData {
  ContactTesterReferenceServiceTable contact_tester_table;
  ContactSolverReferenceServiceTable contact_solver_table;
};
}  // namespace base_engine::physics
