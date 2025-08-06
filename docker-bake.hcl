variable "REPO" {
  default = "lifflander1/vt"
}

variable "GIT_BRANCH" {}

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

target "vt-tv-build" {
  target = "build"
  context = "."
  dockerfile = "ci/docker/build-and-test-ubuntu.dockerfile"

  platforms = [
    "linux/amd64",
  ]
  ulimits = [
    "core=0"
  ]

  secret = ["id=GITHUB_TOKEN,env=GITHUB_TOKEN"]
}

target "vt-tv-build-all" {
  name = "vt-tv-build-${replace(item.image, ".", "-")}${target_suffix(item)}"
  inherits = ["vt-tv-build"]
  tags = ["${REPO}:vt-tv-${item.image}"]

  args = {
    ARCH = arch(item)
    GIT_BRANCH = "${GIT_BRANCH}"
    IMAGE = "wf-${item.image}"
    REPO = REPO
  }

  matrix = {
    item = [
      {
        image = "amd64-ubuntu-24-04-clang-16-vtk-cpp"
      },
      {
        image = "amd64-ubuntu-22-04-gcc-12-vtk-cpp"
      },
    ]
  }
}
