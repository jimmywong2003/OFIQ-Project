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
For a tutorial on how to compile and operatate OFIQ on mobile platforms, 
see [here](mobile/BUILD.md).

## Reference manual
A full documentation of __OFIQ__ including compilation, configuration and a comprehensive doxygen 
documentation of the C/C++ API is contained in the reference manual: 
see [doc/refman.pdf](doc/refman.pdf).

## ICAO Compliance Checker

OFIQ includes an ICAO compliance checker tool that assesses facial images according to ICAO (International Civil Aviation Organization) standards for passport and ID document applications. The tool provides comprehensive quality assessment with 28 different measures.

### Features:
- **28 quality measures** including face detection, landmarks, pose estimation, and image quality metrics
- **ICAO compliance checking** with configurable thresholds
- **Detailed JSON output** with compliance status for each measure
- **Face detection** with 98-point landmark tracking
- **Head pose estimation** (yaw, pitch, roll)
- **Configurable thresholds** via JSON configuration files

### Usage:
```bash
# Build the ICAO compliance checker
./build_icao_checker.sh  # Linux/macOS
build_icao_checker.cmd   # Windows

# Run the compliance checker
./build/icao_compliance_checker/Release/icao_compliance_checker.exe \
  -i image.jpg \
  -c data \
  -t icao_compliance_config.json \
  -o result.json
```

### Output:
The tool generates a comprehensive JSON report including:
- Face detection information with bounding box
- 98 facial landmarks
- Head pose angles
- 28 quality measures with compliance status
- Overall compliance summary

For more details on building and using the ICAO compliance checker, see the [BUILD.md](BUILD.md) documentation.

## Known issues
For a list of known issues, see [here](ISSUES.md)

