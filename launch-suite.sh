#!/usr/bin/env bash

set -euo pipefail

app="All"
configuration="Release"
qt_prefix="${KNOTBRAID_QT_PREFIX:-}"
build_if_missing=0
rebuild=0
no_launch=0

usage() {
    cat <<'EOF'
Usage: launch-suite.sh [options]

Options:
  --app <All|KnotBraid|KnotBraidLauncher|LogiKnotting|LogiBraiding>
  --configuration <Release|Debug>
  --qt-prefix <path>
  --build-if-missing
  --rebuild
  --no-launch
  --help
EOF
}

resolve_qt_prefix() {
    if [[ -n "${qt_prefix}" ]]; then
        printf '%s\n' "${qt_prefix}"
        return
    fi

    if command -v qtpaths6 >/dev/null 2>&1; then
        qtpaths6 --qt-query QT_INSTALL_PREFIX 2>/dev/null || true
        return
    fi

    if command -v qmake6 >/dev/null 2>&1; then
        qmake6 -query QT_INSTALL_PREFIX 2>/dev/null || true
        return
    fi

    printf '\n'
}

binary_name() {
    printf '%s\n' "$1"
}

find_binary() {
    local project_dir="$1"
    local name="$2"
    local build_dir="${repo_root}/${project_dir}/build"

    local candidates=(
        "${build_dir}/${configuration}/${name}"
        "${build_dir}/${name}"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -f "${candidate}" ]]; then
            printf '%s\n' "${candidate}"
            return 0
        fi
    done

    return 1
}

build_app() {
    local name="$1"
    local project_dir="$2"
    local target_name="$3"
    local project_path="${repo_root}/${project_dir}"
    local build_path="${project_path}/build"

    echo "[build] Configuring ${name}..."
    local cmake_args=(
        -S "${project_path}"
        -B "${build_path}"
        "-DCMAKE_BUILD_TYPE=${configuration}"
    )
    if [[ -n "${qt_prefix}" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=${qt_prefix}")
    fi
    cmake "${cmake_args[@]}"

    echo "[build] Building ${name}..."
    cmake --build "${build_path}" --target "${target_name}"
}

launch_binary() {
    local path="$1"
    local working_dir
    working_dir="$(dirname "${path}")"

    echo "[run] Starting $(basename "${path}")..."
    (
        cd "${working_dir}"
        nohup "${path}" >/dev/null 2>&1 &
    )
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --app)
            app="${2:-}"
            shift 2
            ;;
        --configuration)
            configuration="${2:-}"
            shift 2
            ;;
        --qt-prefix)
            qt_prefix="${2:-}"
            shift 2
            ;;
        --build-if-missing)
            build_if_missing=1
            shift
            ;;
        --rebuild)
            rebuild=1
            shift
            ;;
        --no-launch)
            no_launch=1
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

case "${app}" in
    All|KnotBraid|KnotBraidLauncher|LogiKnotting|LogiBraiding)
        ;;
    *)
        echo "Invalid --app value: ${app}" >&2
        usage >&2
        exit 1
        ;;
esac

case "${configuration}" in
    Release|Debug)
        ;;
    *)
        echo "Invalid --configuration value: ${configuration}" >&2
        usage >&2
        exit 1
        ;;
esac

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
qt_prefix="$(resolve_qt_prefix | tr -d '\r')"

if [[ -n "${qt_prefix}" ]]; then
    echo "[qt] Using Qt prefix: ${qt_prefix}"
else
    echo "[qt] Using system Qt discovery."
fi

declare -a app_specs
case "${app}" in
    All)
        app_specs=(
            "LogiKnotting|LogiKnotting|$(binary_name LogiKnotting)|LogiKnotting"
            "LogiBraiding|LogiBraiding|$(binary_name LogiBraiding)|LogiBraiding"
        )
        ;;
    KnotBraid)
        app_specs=("KnotBraid|KnotBraidLauncher|$(binary_name KnotBraid)|KnotBraid")
        ;;
    KnotBraidLauncher)
        app_specs=("KnotBraidLauncher|KnotBraidLauncher|$(binary_name KnotBraidLauncher)|KnotBraidLauncher")
        ;;
    LogiKnotting)
        app_specs=("LogiKnotting|LogiKnotting|$(binary_name LogiKnotting)|LogiKnotting")
        ;;
    LogiBraiding)
        app_specs=("LogiBraiding|LogiBraiding|$(binary_name LogiBraiding)|LogiBraiding")
        ;;
esac

for spec in "${app_specs[@]}"; do
    IFS='|' read -r name project_dir executable_name target_name <<<"${spec}"

    if [[ "${rebuild}" -eq 1 ]]; then
        rm -rf "${repo_root}/${project_dir}/build"
    fi

    binary_path=""
    if ! binary_path="$(find_binary "${project_dir}" "${executable_name}" 2>/dev/null)"; then
        if [[ "${build_if_missing}" -eq 1 || "${rebuild}" -eq 1 ]]; then
            build_app "${name}" "${project_dir}" "${target_name}"
            binary_path="$(find_binary "${project_dir}" "${executable_name}")"
        else
            echo "Executable not found for ${name}. Use --build-if-missing." >&2
            exit 1
        fi
    fi

    if [[ "${no_launch}" -eq 1 ]]; then
        echo "[ok] Ready: ${binary_path}"
        continue
    fi

    launch_binary "${binary_path}"
done

echo "Done."
