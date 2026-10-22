#!/usr/bin/env bash

# Produce a Markdown PR comment for one bake target, as in magistrate.
# Usage: generate_comment_body.sh BUILD_DIR BAKE_TARGET JOB_STATUS
# Reads configure.log, build.log, tests.log, python-build.log, python-tests.log.
# GitHub metadata is provided through GITHUB_* environment variables.

set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 BUILD_DIR BAKE_TARGET JOB_STATUS" >&2
    exit 1
fi

build_dir="$1"
bake_target="$2"
job_status="$3"
max_log_chars=1200

case "$job_status" in
    success|failure|cancelled|skipped) ;;
    *) echo "Unknown job status: $job_status" >&2; exit 1 ;;
esac

titles=("CMake configuration" "C++ compilation" "C++ tests" "Python package build" "Python tests")
logs=(configure.log build.log tests.log python-build.log python-tests.log)

printf 'Build for %s\n\n' "${GITHUB_SHA:-unknown}"
printf 'Target: `%s` — **%s**\n' "$bake_target" "$job_status"

for index in "${!logs[@]}"; do
    log_file="$build_dir/${logs[$index]}"
    if [[ ! -f "$log_file" ]]; then
        detail='Log unavailable; this stage may not have run.'
    elif [[ ! -s "$log_file" ]]; then
        detail='Log is empty; check the workflow logs.'
    else
        # Strip ANSI terminal escapes and normalize carriage-return progress lines.
        log=$(sed -E $'s/\033\\[[0-?]*[ -/]*[@-~]//g' "$log_file" | tr '\r' '\n')
        if [[ "$job_status" == success ]]; then
            case "${logs[$index]}" in
                configure.log|build.log|python-build.log)
                    summary=$(printf '%s\n' "$log" | grep -i 'warning' || true)
                    ;;
                tests.log)
                    summary=$(printf '%s\n' "$log" | grep -E '^\[[[:space:]]*(==========|PASSED|FAILED)[[:space:]]*\]' || true)
                    ;;
                *) summary='' ;;
            esac
            detail='Completed successfully.'
            if [[ -n "$summary" ]]; then
                detail+=$'\n'"$summary"
            fi
        else
            detail="${log:-Log is empty; check the workflow logs.}"
        fi

        if (( ${#detail} > max_log_chars )); then
            detail=$'[Earlier output omitted; see workflow logs.]\n'"${detail: -max_log_chars}"
        fi
    fi

    printf '\n**%s**\n\n' "${titles[$index]}"
    # Indented code blocks keep diagnostic text from breaking Markdown fences.
    printf '%s\n' "$detail" | sed 's/^/    /'
done

printf '\n[View workflow logs](%s/%s/actions/runs/%s/attempts/%s)\n' \
    "${GITHUB_SERVER_URL:-https://github.com}" "${GITHUB_REPOSITORY:-}" \
    "${GITHUB_RUN_ID:-}" "${GITHUB_RUN_ATTEMPT:-1}"
