# Open Source Face Image Quality (OFIQ)

The __OFIQ__ (Open Source Face Image Quality) is a software library for computing quality 
aspects of a facial image. OFIQ is written in the C/C++ programming language.
OFIQ is the reference implementation for the ISO/IEC 29794-5 international
standard; see [https://bsi.bund.de/dok/OFIQ-e](https://bsi.bund.de/dok/OFIQ-e).

## License
Before using __OFIQ__ or distributing parts of __OFIQ__ one should have a look
on OFIQ's license and the license of its dependencies: [LICENSE.md](LICENSE.md)
  
## Getting started
For a tutorial on how to compile and operate OFIQ, see [here](BUILD.md).

## Language Wrappers

### C# Wrapper
A modern .NET 8 wrapper library is available for OFIQ, providing a type-safe, performant C# API for facial image quality assessment.

**Features:**
- Full ISO/IEC 29794-5 compliance (28 quality measures)
- Cross-platform support (Windows, Linux, macOS)
- Memory-safe interop with proper resource cleanup
- Comprehensive error handling
- Multiple image format support (PNG, JPEG, BMP)

**Location:** [`OFIQ-CSharp-Wrapper/`](OFIQ-CSharp-Wrapper/)

**Documentation:** See [OFIQ-CSharp-Wrapper/README.md](OFIQ-CSharp-Wrapper/README.md) for detailed usage instructions and examples.

### Python Build System
OFIQ includes an enhanced Python-based build system that provides flexible build options and automatic dependency management.

**Key Features:**
- Modern Python build script replacing batch scripts
- Automatic Conan detection at custom installation paths
- Flexible build options: `--debug`, `--arch`, `--compiler`, `--no-conan`, `--no-download`, `--skip-deps`
- Comprehensive build documentation and troubleshooting guide

**Location:** [`scripts/build_ofiq.py`](scripts/build_ofiq.py)

**Documentation:** See [scripts/BUILD_INSTRUCTIONS.md](scripts/BUILD_INSTRUCTIONS.md) for detailed build instructions.

## Reference manual
A full documentation of __OFIQ__ including compilation, configuration and a comprehensive doxygen documentation of 
the C/C++ API is contained in the reference manual:
see [doc/refman.pdf](doc/refman.pdf).

## Known issues
For a list of known issues, see [here](ISSUES.md)
