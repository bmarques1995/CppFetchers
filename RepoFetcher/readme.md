# Repo Fetcher

Repo Fetcher is a CLI that downloads(with git) and compiles C/C++ projects on Windows and Linux, with future plans to port it for MacOS and FreeBSD.

Now it only supports compile cmake and raw CLI(like openssl) pipelines.

# Usage

The program works on the following configuration, you send, via argv, some arguments:

- JSON file with the info \*
- Build mode (Debug|Release)
- Install Prefix
- Module Root Destination \(the repository will be cloned to \<module_root_destination\>/modules/\<project_name\>\)
- Compiler path (windows only, to init the vcvars64, optional)

\* The pipeline is indicated by the infix extension, ex: zstd.cmake.json, with:

- `.cmake` linking to cmake
- `.custom` linking to raw pipelines
- `.meson` linking to meson (will be added soon)

# Pipeline

Here is an example of cmake pipeline:

```json

{
  "git": {
    "location": "https://github.com/google/flatbuffers.git",
    "output_suffix": "flatbuffers",
    "branch": "",
    "commit": "1c514626e83c20fffa8557e75641848e1e15cd5e",
    "patch": "flatbuffers.patch"

  },
  "cmake": {
    "build_system": "default",
    "relative_root_location": "",
    "flags": [ "-DFLATBUFFERS_BUILD_SHAREDLIB=ON" ],
    "os_properties": {
      "windows": {
        "c_compiler": "cl",
        "cxx_compiler": "cl"
      },
      "linux": {
        "c_compiler": "clang",
        "cxx_compiler": "clang++"
      },
      "macos": {
        "c_compiler": "clang",
        "cxx_compiler": "clang++"
      },
      "freebsd": {
        "c_compiler": "clang",
        "cxx_compiler": "clang++"
      }
    }
  }
}


```

the git controller is shared between pipelines, with 5 fields:

- location: the url of the project
- output_suffix: the name of the folder of source and solution
- branch: the branch to clone, if empty, uses the default branch
- commit: you can roll back to a commit, and if this field isn't empty it will rollback for the indicated commit
- patch: you can also apply a patch, if this field is present