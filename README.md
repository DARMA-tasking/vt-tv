[![Build and Test Ubuntu 22.04 gcc 11 x64](https://github.com/DARMA-tasking/vt-tv/actions/workflows/build-and-test-vt-tv.yml/badge.svg)](https://github.com/DARMA-tasking/vt-tv/actions/workflows/build-and-test-vt-tv.yml)

# VT-TV -- Task Visualizer

Included workflows:
* [*check-pr-fixes-issue*](https://github.com/DARMA-tasking/check-pr-fixes-issue) - checking if PR description contains phrase "Fixes #issue", and if PR title, description and branch mention the same issue number
* [*find-unsigned-commits*](https://github.com/DARMA-tasking/find-unsigned-commits) - checking if there are any unsigned commits in PR
* [*find-trailing-whitespace*](https://github.com/DARMA-tasking/find-trailing-whitespace) - checking if there are any trailing whitespaces in files
* [*check-commit-format*](https://github.com/DARMA-tasking/check-commit-format) - checking if commit message is properly formatted - either starts with "*Merge ...*" or fullfils template: "*#issue_number: short commit description*"
* [*action-git-diff-check*](https://github.com/joel-coffman/action-git-diff-check) - checking if changes introduce conflict markers or whitespace errors
