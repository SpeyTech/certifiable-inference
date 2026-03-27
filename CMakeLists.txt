##
# CMakeLists.txt — certifiable-inference (L2)
#
# Copyright (c) 2026 The Murray Family Innovation Trust
# SPDX-License-Identifier: GPL-3.0-or-later
#
# SRS: SRS-005-CR-001 v1.1-Frozen
# DVEC: v1.3
# Determinism: D1 — Strict Deterministic
##

cmake_minimum_required(VERSION 3.10)
project(certifiable_inference C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ==============================================================================
# Compiler Flags — SRS-005 Hardened
# ==============================================================================

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -fwrapv")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-tree-vectorize -fno-builtin -fno-fast-math")

# Additional flags for static analysis
include(CheckCCompilerFlag)

check_c_compiler_flag("-fanalyzer" HAS_FANALYZER)
if (HAS_FANALYZER)
  add_compile_options(-fanalyzer)
endif()

# ==============================================================================
# Include Directories
# ==============================================================================

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/core)
include_directories(${PROJECT_SOURCE_DIR}/include/containers)

# ==============================================================================
# Core Library
# ==============================================================================

add_library(certifiable_inference
    src/containers/deterministic_hash.c
    src/core/fixed_point.c
    src/core/matrix.c
    src/core/activations.c
    src/core/convolution.c
    src/core/pooling.c
)

# ==============================================================================
# Example Programs
# ==============================================================================

add_executable(xor_gate examples/xor_gate.c)
target_link_libraries(xor_gate certifiable_inference m)

add_executable(edge_detection examples/edge_detection.c)
target_link_libraries(edge_detection certifiable_inference m)

add_executable(debug_conv examples/debug_conv.c)
target_link_libraries(debug_conv certifiable_inference m)

add_executable(debug_pool examples/debug_pool.c)
target_link_libraries(debug_pool certifiable_inference m)

# ==============================================================================
# Benchmarks
# ==============================================================================

add_executable(timing_benchmark tests/benchmarks/test_timing_consistency.c)
target_link_libraries(timing_benchmark certifiable_inference m)

# ==============================================================================
# Testing
# ==============================================================================

enable_testing()

function(ci_add_unit_test name)
  add_executable(${name} ${ARGN})
  target_compile_options(${name} PRIVATE -UNDEBUG)
  target_link_libraries(${name} PRIVATE certifiable_inference m)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# Unit tests
ci_add_unit_test(test_hash_basic              tests/unit/test_hash_basic.c)
ci_add_unit_test(test_hash_consistency        tests/unit/test_hash_consistency.c)
ci_add_unit_test(test_fixed_point             tests/unit/test_fixed_point.c)
ci_add_unit_test(test_matrix_reproducibility  tests/unit/test_matrix_reproducibility.c)
ci_add_unit_test(test_activations             tests/unit/test_activations.c)
ci_add_unit_test(test_convolution             tests/unit/test_convolution.c)
ci_add_unit_test(test_pooling                 tests/unit/test_pooling.c)

# SRS-005-CR-001 compliance test
ci_add_unit_test(test_srs005_compliance       tests/test_srs005_compliance.c)

# ==============================================================================
# Forbidden Pattern Scan (SRS-005-SHALL-068)
# ==============================================================================

add_custom_target(forbidden-pattern-scan
    COMMAND ${CMAKE_COMMAND} -E echo "SRS-005-SHALL-068: Scanning for forbidden shift operators..."
    COMMAND bash -c "if grep -n '<<\\|>>' ${PROJECT_SOURCE_DIR}/src/core/fixed_point.c 2>/dev/null | grep -v '//'; then echo 'FAIL: Signed shift operators found in fixed_point.c'; exit 1; else echo 'PASS: No forbidden patterns in arithmetic core'; fi"
    COMMENT "Forbidden pattern scan"
    VERBATIM
)

# ==============================================================================
# Static Analysis
# ==============================================================================

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK}
            --enable=warning,style,performance,portability
            --error-exitcode=1
            --suppress=missingIncludeSystem
            --suppress=unusedFunction
            --std=c99
            -I ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/
        COMMENT "Running cppcheck static analysis"
    )

    # Stricter check that includes unusedFunction (for application builds)
    add_custom_target(
        cppcheck-strict
        COMMAND ${CPPCHECK}
            --enable=all
            --error-exitcode=1
            --suppress=missingIncludeSystem
            --std=c99
            -I ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/
        COMMENT "Running strict cppcheck (includes unusedFunction)"
    )
endif()

find_program(CPPCHECK_MISRA cppcheck)
if(CPPCHECK_MISRA)
    add_custom_target(
        misra-check
        COMMAND ${CPPCHECK_MISRA}
            --addon=misra
            --error-exitcode=1
            --suppress=missingIncludeSystem
            --suppress=unusedFunction
            -I ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/
        COMMENT "Running MISRA-C compliance check"
    )
endif()

# ==============================================================================
# Coverage
# ==============================================================================

option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# ==============================================================================
# Aggregate Targets
# ==============================================================================

add_custom_target(
    test-all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS test_hash_basic
            test_hash_consistency
            test_fixed_point
            test_matrix_reproducibility
            test_activations
            test_convolution
            test_pooling
            test_srs005_compliance
    COMMENT "Running all tests"
)

add_custom_target(
    verify-all
    DEPENDS test-all forbidden-pattern-scan
    COMMENT "Running complete verification suite"
)

if(CPPCHECK)
    add_dependencies(verify-all cppcheck)
endif()

add_custom_target(
    benchmarks
    COMMAND ./timing_benchmark
    DEPENDS timing_benchmark
    COMMENT "Running performance benchmarks"
)

# ==============================================================================
# Configuration Summary
# ==============================================================================

message(STATUS "")
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "  Certifiable Inference Engine v0.2.0")
message(STATUS "  SRS-005-CR-001 v1.1-Frozen Conformant")
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "  C Compiler:    ${CMAKE_C_COMPILER}")
message(STATUS "  C Standard:    C99")
message(STATUS "  Build Type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "SRS-005 Compliance:")
message(STATUS "  ✓ No signed shifts (multiplication/division only)")
message(STATUS "  ✓ Truncation toward zero (no rounding)")
message(STATUS "  ✓ INT32_MIN overflow saturation")
message(STATUS "  ✓ L2 types delegate to L1 (axilog/types.h)")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  ✓ Fixed-point arithmetic (Q16.16)")
message(STATUS "  ✓ Matrix operations")
message(STATUS "  ✓ Convolution (2D)")
message(STATUS "  ✓ Activation functions (ReLU)")
message(STATUS "  ✓ Max Pooling (2×2 stride-2)")
message(STATUS "  ✓ Deterministic hash table")
message(STATUS "")
message(STATUS "Tests:")
message(STATUS "  ✓ Unit tests (7 test suites)")
message(STATUS "  ✓ SRS-005 compliance tests")
message(STATUS "  ✓ Timing benchmarks")
message(STATUS "  ✓ Example programs")
message(STATUS "")
message(STATUS "Build Targets:")
message(STATUS "  make                      - Build all")
message(STATUS "  make test-all             - Run all tests")
message(STATUS "  make forbidden-pattern-scan - SRS-005-SHALL-068 check")
message(STATUS "  make verify-all           - Tests + static analysis")
message(STATUS "  make benchmarks           - Timing benchmarks")
message(STATUS "═══════════════════════════════════════════════")
message(STATUS "")
