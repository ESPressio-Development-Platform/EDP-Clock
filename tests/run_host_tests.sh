#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/tests/.test-build"
SYSTEM_DIR="${EDP_SYSTEM_SOURCE_DIR:?EDP_SYSTEM_SOURCE_DIR must point at an EDP-System checkout}"
PLATFORM_DIR="${EDP_PLATFORM_SOURCE_DIR:?EDP_PLATFORM_SOURCE_DIR must point at an EDP-Platform checkout}"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

CXX="${CXX:-c++}"
COMMON_FLAGS=(
    -std=c++20
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -pthread
    -I"${ROOT_DIR}/src"
    -I"${SYSTEM_DIR}/src"
    -I"${PLATFORM_DIR}/src"
)

compile_and_run() {
    local name="$1"
    shift

    echo "EDP-Clock: compiling ${name}"
    "${CXX}" \
        "${COMMON_FLAGS[@]}" \
        "$@" \
        -o "${BUILD_DIR}/${name}"

    echo "EDP-Clock: executing ${name}"
    "${BUILD_DIR}/${name}"
}

compile_and_run \
    MonotonicClockTests \
    "${ROOT_DIR}/tests/monotonic/MonotonicClockTests.cpp" \
    "${ROOT_DIR}/tests/monotonic/MonotonicNowConsumer.cpp"

compile_and_run \
    StopwatchTests \
    "${ROOT_DIR}/tests/stopwatch/StopwatchTests.cpp"

compile_and_run \
    ClockCorrelationProviderTests \
    "${ROOT_DIR}/tests/synchronized/ClockCorrelationProviderTests.cpp"

compile_and_run \
    ClockCorrelationTests \
    "${ROOT_DIR}/tests/synchronized/ClockCorrelationTests.cpp"

compile_and_run \
    FourTimestampEstimatorTests \
    "${ROOT_DIR}/tests/synchronized/FourTimestampEstimatorTests.cpp"

compile_and_run \
    SynchronizedClockTests \
    "${ROOT_DIR}/tests/synchronized/SynchronizedClockTests.cpp" \
    "${ROOT_DIR}/tests/synchronized/SynchronizedNowConsumer.cpp"

echo "All EDP-Clock host tests passed."
