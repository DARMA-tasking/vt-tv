variable "REPO" {
  default = "lifflander1/vt"
}

function "arch" {
  params = [item]
  result = lookup(item, "arch", "amd64")
}

function "variant" {
  params = [item]
  result = lookup(item, "variant", "")
}

function "target_suffix" {
  params = [item]
  result = variant(item) == "" ? "" : "-${variant(item)}"
}

function "vt-tv-python-bindings" {
  params = [item]
  result = lookup(item, "vt-tv-python-bindings", "0")
}

function "vt-tv-openmp" {
  params = [item]
  result = lookup(item, "vt-tv-openmp", "1")
}

function "vt-tv-tests" {
  params = [item]
  result = lookup(item, "vt-tv-tests", "1")
}

function "vt-tv-coverage" {
  params = [item]
  result = lookup(item, "vt-tv-coverage", "0")
}

function "vt-tv-docs" {
  params = [item]
  result = lookup(item, "vt-tv-docs", "0")
}

function "vt-tv-werror" {
  params = [item]
  result = lookup(item, "vt-tv-werror", "0")
}

target "vt-tv-build" {
  target = "build"
  context = "."
  dockerfile = "ci/docker/vt-tv.dockerfile"

  platforms = [
    "linux/amd64",
  ]
  ulimits = [
    "core=0"
  ]
}

target "vt-tv-build-all" {
  name = "vt-tv-build-${replace(item.image, ".", "-")}${target_suffix(item)}"
  inherits = ["vt-tv-build"]
  tags = ["${REPO}:vt-tv-${item.image}"]

  args = {
    ARCH = arch(item)
    IMAGE = "wf-${item.image}"
    REPO = REPO
    VT_TV_COVERAGE_ENABLED = vt-tv-coverage(item)
    VT_TV_OPENMP_ENABLED = vt-tv-openmp(item)
    VT_TV_PYTHON_BINDINGS_ENABLED = vt-tv-python-bindings(item)
    VT_TV_TESTS_ENABLED = vt-tv-tests(item)
  }

  matrix = {
    item = [
      {
        image = "amd64-ubuntu-24.04-clang-16-vtk-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-12-vtk-cpp",
        vt-tv-coverage = 1
      },
    ]
  }
}
