#!/bin/bash

# Init local variables
CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")" # Current directory
BUILD_DIR=${CURRENT_DIR}/build
GIT_REPO_DOCS=DARMA-tasking.github.io
GHPAGE=${BUILD_DIR}/${GIT_REPO_DOCS}
GIT_OUTPUT_FOLDER=vt_tv_docs
MCSS=${BUILD_DIR}/m.css
TOKEN=${1:-}

# Set env vars
export DOXYGEN_IN=${CURRENT_DIR}/docs/Doxyfile.in
export DOXYGEN_OUT=${BUILD_DIR}/Doxyfile
export DOXYGEN_PROJECT_NAME="VT TV"
export VERSION_MAJOR="1"
export VERSION_MINOR="0"
export VERSION_PATCH="0"
export DOXYGEN_INPUT_DIR=${CURRENT_DIR}/src/
export DOXYGEN_DOCS_DIR=${CURRENT_DIR}/docs/
export DOXYGEN_EXAMPLE_DIR=${CURRENT_DIR}/examples/
export DOXYGEN_MAIN_PAGE=${CURRENT_DIR}/docs/md/mainpage.md
export DOXYGEN_OUTPUT_DIR=${BUILD_DIR}/${GIT_OUTPUT_FOLDER}/
export DOXYGEN_EXECUTABLE=$(which doxygen)

# Create buid dir
mkdir -p ${BUILD_DIR}

# Copy Doxygen configuration files
cp ${DOXYGEN_IN} ${DOXYGEN_OUT}
cp ${DOXYGEN_IN}-mcss ${DOXYGEN_OUT}-mcss

# Replace by env variables 
env | grep -E 'DOXYGEN|VERSION_|PROJECT_SOURCE_DIR|PROJECT_BINARY_DIR' | while IFS= read -r line; do
  value=${line#*=} 
  name=${line%%=*} 
  valueEscape=$( echo $value | sed 's/\//\\\//g' )

  # Replace @VARIBALE_NAME@ by is value 
  sed -i "s/@${name}@/${valueEscape}/" ${DOXYGEN_OUT}
done

# Go to build folder
cd ./build

# Launch Doxygen generation
${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}

# GIT Clone documentation repository and m.css
git clone --depth=1 "https://${TOKEN}@github.com/DARMA-tasking/${GIT_REPO_DOCS}"
git clone https://github.com/mosra/m.css

# GIT Checkout m.css on master 
cd m.css
git checkout master
cd ../

# Launch doxygen python script to apply style on generated documentation 
"$MCSS/documentation/doxygen.py" ${DOXYGEN_OUT}-mcss

# Copy into the documentation repository and go to the into the repository 
mv "$DOXYGEN_OUTPUT_DIR" "$GHPAGE/$GIT_OUTPUT_FOLDER"
cd "$GHPAGE"

# Change branch [dev]
git checkout -b 112-add-documentation
git config --global user.email "jliffla@sandia.gov"
git config --global user.name "Jonathan Lifflander"

git add "$GIT_OUTPUT_FOLDER"
git commit -m "Update $GIT_OUTPUT_FOLDER (auto-build)"
git push origin 112-add-documentation