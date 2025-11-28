#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
VERSION_FILE="$REPO_ROOT/.tarball-version"
FRAMEWORKS_DIR=""

usage() {
  cat <<'EOF'
Usage: build_idevicebackup2_universal.sh [--frameworks-dir PATH]

Builds idevicebackup2 for both arm64 and x86_64, creates a universal binary,
and fixes framework dependency paths (same behavior as fix_framework_paths.sh).

Options:
  --frameworks-dir PATH   Optional explicit Frameworks directory to patch.
                          If omitted, the script tries common locations:
                            - <repo>/macos/Frameworks
                            - <repo>/../dispute_buddy/macos/Frameworks
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --frameworks-dir)
      shift || { echo "Error: --frameworks-dir requires a path"; exit 1; }
      FRAMEWORKS_DIR="$1"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1"
      usage
      exit 1
      ;;
  esac
  shift
done

bump_patch_version() {
  local default_label="dbuddy"
  local default_major="1"
  local default_minor="0"
  local default_patch="0"
  local current=""

  if [[ -f "$VERSION_FILE" ]]; then
    current=$(tr -d '\r' < "$VERSION_FILE" | head -n 1)
    current="${current%$'\n'}"
  fi

  if [[ -z "$current" ]]; then
    current="${default_label} ${default_major}.${default_minor}.${default_patch}"
  fi

  local label major minor patch
  if [[ $current =~ ^([[:alnum:]_.-]+)[[:space:]]+([0-9]+)\.([0-9]+)(\.([0-9]+))?$ ]]; then
    label="${BASH_REMATCH[1]}"
    major="${BASH_REMATCH[2]}"
    minor="${BASH_REMATCH[3]}"
    patch="${BASH_REMATCH[5]}"
  else
    echo "Warning: Unable to parse version '$current'. Resetting to default."
    label="$default_label"
    major="$default_major"
    minor="$default_minor"
    patch="$default_patch"
  fi

  patch="${patch:-0}"
  patch=$((patch + 1))

  local new_version="${label} ${major}.${minor}.${patch}"
  echo "$new_version" > "$VERSION_FILE"
  export RELEASE_VERSION="$new_version"
  echo "==> Version bumped to $new_version"
}

detect_frameworks_dir() {
  local candidates=(
    "$REPO_ROOT/macos/Frameworks"
    "$REPO_ROOT/../dispute_buddy/macos/Frameworks"
  )

  for candidate in "${candidates[@]}"; do
    if [[ -d "$candidate" ]]; then
      FRAMEWORKS_DIR="$(cd "$candidate" && pwd)"
      return
    fi
  done
}

build_universal() {
  echo "==> Building idevicebackup2 for arm64 and x86_64"
  (cd "$REPO_ROOT" && BUILD_X86_64=1 ./build_macos.sh)

  echo "==> Creating universal binaries"
  (cd "$REPO_ROOT" && ./create_universal_binary.sh)
}

fix_install_names() {
  local target_dir=$1
  echo "==> Normalizing install names inside $target_dir"

  while IFS= read -r -d '' dylib; do
    local base
    base=$(basename "$dylib")
    echo "    • @rpath/$base"
    install_name_tool -id "@rpath/$base" "$dylib"
  done < <(find "$target_dir" -type f -name "*.dylib" -print0)
}

rewrite_dependency_paths() {
  local target_dir=$1
  echo "==> Rewriting embedded dependency paths"

  while IFS= read -r -d '' bin; do
    local bin_name
    bin_name=$(basename "$bin")
    echo "    Inspecting $bin_name"

    while IFS= read -r dep; do
      [[ -z "$dep" ]] && continue
      if [[ "$dep" == /System/* ]] || [[ "$dep" == /usr/lib/* ]] || [[ "$dep" == @* ]]; then
        continue
      fi

      local base new_path
      base=$(basename "$dep")
      new_path="@executable_path/../Frameworks/$base"
      echo "      ↳ $dep → $new_path"
      install_name_tool -change "$dep" "$new_path" "$bin"
    done < <(otool -L "$bin" 2>/dev/null | tail -n +2 | awk '{print $1}')
  done < <(find "$target_dir" -type f \( -perm -111 -o -name "*.dylib" \) -print0)
}

check_missing_dependencies() {
  local target_dir=$1
  local temp_file
  temp_file=$(mktemp)

  gather_missing "$target_dir" "$temp_file"

  if [[ -s "$temp_file" ]]; then
    echo ""
    echo "⚠️  Missing dependencies that should be in $target_dir:"
    sort -u "$temp_file" | while read -r dep; do
      echo "  - $dep"
    done
    echo ""
  else
    echo "==> All dependencies accounted for in $target_dir"
  fi

  rm -f "$temp_file"
}

gather_missing() {
  local target_dir=$1
  local output_file=$2

  scan_binary_deps "$target_dir" "$output_file"
  scan_framework_deps "$target_dir" "$output_file"
}

scan_binary_deps() {
  local target_dir=$1
  local output_file=$2

  while IFS= read -r -d '' bin; do
    capture_missing "$bin" "$target_dir" "$output_file"
  done < <(find "$target_dir" -type f \( -perm -111 -o -name "*.dylib" \) -print0)
}

scan_framework_deps() {
  local target_dir=$1
  local output_file=$2

  while IFS= read -r -d '' framework; do
    local framework_bin
    framework_bin=$(find "$framework" -type f -perm -111 | head -1 || true)
    [[ -z "$framework_bin" ]] && continue
    capture_missing "$framework_bin" "$target_dir" "$output_file" "$(basename "$framework")"
  done < <(find "$target_dir" -type d -name "*.framework" -print0)
}

capture_missing() {
  local bin=$1
  local target_dir=$2
  local output_file=$3
  local framework_name=${4:-}

  while IFS= read -r dep; do
    [[ -z "$dep" ]] && continue
    if [[ "$dep" == /System/* ]] || [[ "$dep" == /usr/lib/* ]]; then
      continue
    fi

    local base=""
    if [[ "$dep" == @executable_path/../Frameworks/* ]] || [[ "$dep" == @rpath/* ]]; then
      base=$(basename "$dep")
    elif [[ "$dep" == @loader_path/* ]]; then
      continue
    elif [[ "$dep" == @* ]]; then
      base=$(basename "$dep")
    else
      base=$(basename "$dep")
    fi

    [[ -z "$base" ]] && continue
    if [[ -n "$framework_name" && "$base" == "$framework_name" ]]; then
      continue
    fi
    if [[ ! -e "$target_dir/$base" ]]; then
      echo "$base" >> "$output_file"
    fi
  done < <(otool -L "$bin" 2>/dev/null | awk '{print $1}' | grep -v ":")
}

main() {
  if [[ -z "$FRAMEWORKS_DIR" ]]; then
    detect_frameworks_dir
  fi

  if [[ -z "$FRAMEWORKS_DIR" ]] || [[ ! -d "$FRAMEWORKS_DIR" ]]; then
    echo "Error: Frameworks directory not found. Provide one with --frameworks-dir PATH."
    exit 1
  fi

  echo "Frameworks directory: $FRAMEWORKS_DIR"

  bump_patch_version
  build_universal
  fix_install_names "$FRAMEWORKS_DIR"
  rewrite_dependency_paths "$FRAMEWORKS_DIR"
  check_missing_dependencies "$FRAMEWORKS_DIR"

  echo "==> Universal idevicebackup2 build complete:"
  echo "    $REPO_ROOT/build/universal/bin/idevicebackup2"
}

main "$@"

