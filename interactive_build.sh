#!/bin/bash

echo "Welcome to the vt-tv Build Setup!"

# Function to prompt for a directory until a valid one is given
prompt_for_dir() {
  local prompt_message="$1"
  local default_val="$2"
  local dir_choice
  while true; do
    read -p "${prompt_message} ${default_val:+[default (Enter): ${default_val}]}: " dir_choice
    # If the user input is empty and a default value is provided, use it
    if [[ -z "${dir_choice}" && -n "${default_val}" ]]; then
      echo "${default_val}"
      return
    # If the user input is a valid directory, use it
    elif [[ -d "${dir_choice}" ]]; then
      echo "${dir_choice}"
      return
    else
      # This message should be displayed for invalid directories
      echo >&2 "Invalid directory. Please enter a valid path."
    fi
  done
}

# Function for yes/no prompts
prompt_for_yes_no() {
  local prompt_message="$1"
  local yn_choice
  while true; do
    read -p "${prompt_message} (y/n): " yn_choice
    case "${yn_choice}" in
      y|Y) echo "y"; return;;
      n|N) echo "n"; return;;
      *) echo "Please enter 'y' for yes or 'n' for no.";;
    esac
  done
}

# Function to prompt for a number
prompt_for_number() {
  local prompt_message="$1"
  local num_choice
  while true; do
    read -p "${prompt_message}: " num_choice
    if [[ "$num_choice" =~ ^[1-9][0-9]*$ || -z "$num_choice" ]]; then
      echo "${num_choice}"
      return
    else
      echo "Invalid input. Please enter a positive integer."
    fi
  done
}

# Prompt for the source directory
PROJECT_DIR=$(prompt_for_dir "Enter the vt-tv source directory" "$(pwd)")
# Prompt for the build directory
BUILD_DIR=$(prompt_for_dir "Enter the build directory" "${PROJECT_DIR}/build")

# Ask about VTK directory
USE_VTK_DIR=$(prompt_for_yes_no "Do you have a precompiled VTK directory?")
if [[ "${USE_VTK_DIR}" == "y" ]]; then
  VTK_DIR=$(prompt_for_dir "Enter the VTK build directory" "")
fi

# Ask about number of jobs for make
JOBS=$(prompt_for_number "Enter the number of jobs/threads for make (leave empty for system default)")

# Ask if they want to install
INSTALL_CHOICE=$(prompt_for_yes_no "Do you want to install the project after building?")
if [[ "${INSTALL_CHOICE}" == "y" ]]; then
  INSTALL_DIR=$(prompt_for_dir "Enter the vt-tv install directory" "${PROJECT_DIR}/install")
fi

# Start the build process
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake_opts=()

if [[ -n "${VTK_DIR}" ]]; then
  cmake_opts+=("-DVTK_DIR=${VTK_DIR}")
fi
if [[ -n "${INSTALL_DIR}" ]]; then
  cmake_opts+=("-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}")
fi

cmake "${cmake_opts[@]}" "${PROJECT_DIR}"

if [[ $? -ne 0 ]]; then
  echo "Error: CMake configuration failed!"
  exit 1
fi

if [[ -n "${JOBS}" ]]; then
  make -j "${JOBS}"
else
  make
fi

if [[ "${INSTALL_CHOICE}" == "y" ]]; then
  make install
  if [[ $? -ne 0 ]]; then
    echo "Error: Installation failed!"
    exit 1
  fi
fi

echo "vt-tv built successfully!"
