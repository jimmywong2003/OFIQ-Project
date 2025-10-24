#!/usr/bin/env python3
"""
OFIQ Library Build Script (Python)
Builds ofiq_lib.dll using MSVC and CMake

This script provides a modern Python interface to build the OFIQ native library
with better error handling, progress reporting, and flexibility than the batch script.

Usage:
  python build_ofiq.py [options]

Options:
  --arch x64|x86          Target architecture (default: x64)
  --compiler 16|17        Visual Studio version (16=2019, 17=2022, default: 16)
  --debug                 Build Debug configuration (default: Release)
  --no-conan              Build dependencies from source instead of using Conan
  --no-download           Skip downloading models and external libraries
  --help                  Show this help message
"""

import argparse
import os
import subprocess
import sys
import shutil
from pathlib import Path
import platform


class OFIQBuilder:
    """Builds OFIQ native library using MSVC and CMake"""
    
    def __init__(self):
        self.root_dir = Path(__file__).parent.parent
        self.script_dir = Path(__file__).parent
        self.build_dir = self.root_dir / "build" / "build_win"
        self.install_dir = "install_x86_64"
        self.architecture = "x64"
        self.compiler_version = 16  # VS 2019
        self.use_conan = True
        self.download = True
        self.config = "Release"
        self.vs_version = "vc16"
        
        # Platform-specific settings
        self.generator = "Visual Studio 16 2019"
        self.compiler_flags = ""
        self.conan_set_arch = ""
        self.onnxruntime_flags = ""
        self.set_architecture = "-A x64"
        
    def parse_arguments(self):
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description="Build OFIQ native library (ofiq_lib.dll) using MSVC and CMake",
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog="""
Examples:
  python build_ofiq.py                    # Default build (x64, Release, VS2019)
  python build_ofiq.py --arch x86         # Build 32-bit version
  python build_ofiq.py --compiler 17      # Use Visual Studio 2022
  python build_ofiq.py --debug            # Build Debug configuration
  python build_ofiq.py --no-conan         # Build dependencies from source
  python build_ofiq.py --no-download      # Skip downloading external files
            """
        )
        
        parser.add_argument("--arch", choices=["x64", "x86"], default="x64",
                          help="Target architecture (default: x64)")
        parser.add_argument("--compiler", type=int, choices=[16, 17], default=16,
                          help="Visual Studio version (16=2019, 17=2022, default: 16)")
        parser.add_argument("--debug", action="store_true",
                          help="Build Debug configuration (default: Release)")
        parser.add_argument("--no-conan", action="store_true",
                          help="Build dependencies from source instead of using Conan")
        parser.add_argument("--no-download", action="store_true",
                          help="Skip downloading models and external libraries")
        parser.add_argument("--skip-deps", action="store_true",
                          help="Skip dependency building (use available dependencies only)")
        
        args = parser.parse_args()
        
        # Apply arguments
        self.architecture = args.arch
        self.compiler_version = args.compiler
        self.config = "Debug" if args.debug else "Release"
        self.use_conan = not args.no_conan
        self.download = not args.no_download
        self.skip_deps = args.skip_deps
        
        return args
    
    def setup_environment(self):
        """Setup build environment based on architecture and compiler"""
        print(f"Setting up build environment:")
        print(f"  Architecture: {self.architecture}")
        print(f"  Compiler: Visual Studio {self.compiler_version}")
        print(f"  Configuration: {self.config}")
        print(f"  Use Conan: {self.use_conan}")
        print(f"  Download: {self.download}")
        
        # Architecture settings
        if self.architecture == "x86":
            self.conan_set_arch = "-s:a arch=x86"
            self.install_dir = "install_x86"
            self.onnxruntime_flags = "--x86"
            self.set_architecture = "-A Win32"
        else:  # x64
            self.conan_set_arch = ""
            self.install_dir = "install_x86_64"
            self.onnxruntime_flags = ""
            self.set_architecture = "-A x64"
        
        # Compiler settings
        if self.compiler_version == 16:
            self.generator = "Visual Studio 16 2019"
            self.compiler_flags = ""
            self.vs_version = "vc16"
        else:  # 17
            self.generator = "Visual Studio 17 2022"
            self.compiler_flags = "-s:a compiler.version=193"
            self.vs_version = "vc17"
    
    def check_prerequisites(self):
        """Check if required tools are available"""
        print("Checking prerequisites...")
        
        # Always required tools
        required_tools = ["cmake"]
        
        # Conan is only required if we're using it
        if self.use_conan:
            # Check for conan in PATH first
            try:
                result = subprocess.run(["conan", "--version"], capture_output=True, text=True, check=True)
                version = result.stdout.strip()
                print(f"  ✓ conan: {version}")
                self.conan_path = "conan"
            except (subprocess.CalledProcessError, FileNotFoundError):
                # Check for conan at common installation paths
                conan_paths = [
                    r"C:\tools\conan-2.21.0-windows-x86_64\conan.exe",
                    r"C:\Program Files\Conan\conan.exe",
                    r"C:\conan\conan.exe"
                ]
                
                conan_found = False
                for conan_path in conan_paths:
                    if Path(conan_path).exists():
                        self.conan_path = conan_path
                        try:
                            result = subprocess.run([conan_path, "--version"], capture_output=True, text=True, check=True)
                            version = result.stdout.strip()
                            print(f"  ✓ conan: {version} (found at {conan_path})")
                            conan_found = True
                            break
                        except (subprocess.CalledProcessError, FileNotFoundError):
                            continue
                
                if not conan_found:
                    print(f"  ✗ conan: Not found in PATH or common locations")
                    print("  Please ensure conan is installed and available in PATH, or specify the path manually")
                    sys.exit(1)
        
        missing_tools = []
        
        for tool in required_tools:
            try:
                result = subprocess.run([tool, "--version"], capture_output=True, text=True, check=True)
                version_line = result.stdout.split('\n')[0]
                print(f"  ✓ {tool}: {version_line}")
            except (subprocess.CalledProcessError, FileNotFoundError):
                missing_tools.append(tool)
                print(f"  ✗ {tool}: Not found")
        
        if missing_tools:
            print(f"\nError: Missing required tools: {', '.join(missing_tools)}")
            print("Please install the missing tools and try again.")
            sys.exit(1)
        
        # Check if we're on Windows
        if platform.system() != "Windows":
            print("\nWarning: This script is designed for Windows. Building on other platforms may not work correctly.")
        
        print("✓ All prerequisites satisfied")
    
    def build_dependencies(self):
        """Build or install dependencies"""
        if self.skip_deps:
            print(f"\nSkipping dependency building (--skip-deps specified)")
            print("  Using available dependencies only")
            return
        
        print(f"\nBuilding dependencies (Conan: {self.use_conan})...")
        
        if self.use_conan:
            self._build_with_conan()
        else:
            self._build_from_source()
    
    def _build_with_conan(self):
        """Build dependencies using Conan"""
        conan_dir = self.root_dir / "build" / "conan"
        
        # Clean conan directory
        if conan_dir.exists():
            print("  Cleaning Conan cache...")
            shutil.rmtree(conan_dir)
        
        # Select profile based on configuration
        if self.config == "Release":
            profile_file = "conan_profile_release.txt"
        else:
            profile_file = "conan_profile_debug.txt"
        
        profile_path = self.root_dir / "conan" / profile_file
        
        # Build conan command using the detected conan path
        cmd = [
            self.conan_path, "install", str(self.root_dir / "conan" / "conanfile.txt"),
            "--build", "missing",
            "--profile:build", str(profile_path),
            "--profile:host", str(profile_path),
            "--output-folder=" + str(self.root_dir / "build" / "conan")
        ]
        
        # Add compiler and architecture flags
        if self.compiler_flags:
            cmd.extend(self.compiler_flags.split())
        if self.conan_set_arch:
            cmd.extend(self.conan_set_arch.split())
        
        print(f"  Running: {' '.join(cmd)}")
        
        try:
            subprocess.run(cmd, check=True, cwd=self.root_dir)
            print("  ✓ Conan dependencies built successfully")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ Conan build failed: {e}")
            sys.exit(1)
    
    def _build_from_source(self):
        """Build dependencies from source"""
        print("  Building dependencies from source...")
        
        # Check what dependencies we have available
        opencv_dir = self.root_dir / "extern" / "opencv-4.5.5"
        gtest_dir = self.root_dir / "extern" / "googletest"
        onnx_dir = self.root_dir / "extern" / "onnxruntime"
        onnx_prebuilt = self.root_dir / "extern" / "onnxruntime-win-x64-1.17.3"
        
        available_deps = []
        missing_deps = []
        
        if opencv_dir.exists():
            available_deps.append("OpenCV")
        else:
            missing_deps.append("OpenCV")
        
        if gtest_dir.exists():
            available_deps.append("GoogleTest")
        else:
            missing_deps.append("GoogleTest")
        
        if onnx_dir.exists() or onnx_prebuilt.exists():
            available_deps.append("ONNX Runtime")
        else:
            missing_deps.append("ONNX Runtime")
        
        print(f"  Available dependencies: {', '.join(available_deps) if available_deps else 'None'}")
        print(f"  Missing dependencies: {', '.join(missing_deps) if missing_deps else 'None'}")
        
        if missing_deps:
            print(f"\n  ❌ Cannot build from source - missing dependencies: {', '.join(missing_deps)}")
            print("\n  To build from source, you need to:")
            print("  1. Download external dependencies using:")
            print("     cmake -P ../cmake/DownloadExternalSourceCode.cmake")
            print("  2. Or download the full OFIQ release from the ISO portal")
            print("  3. Or use Conan (remove --no-conan flag)")
            print("\n  Alternatively, you can try to build with available dependencies only:")
            print("  (This may work if you only need basic functionality)")
            print("  python build_ofiq.py --no-conan --skip-deps")
            sys.exit(1)
        
        print("  ✓ All external dependencies found")
        
        # Build OpenCV
        print("  Building OpenCV...")
        opencv_build_dir = opencv_dir / "build"
        
        if opencv_build_dir.exists():
            shutil.rmtree(opencv_build_dir)
        
        cmd = [
            "cmake", "-S", ".", "-G", self.generator, self.set_architecture, "-B", "build",
            "-DBUILD_LIST=core,calib3d,imgcodecs,improc,dnn,ml",
            "-DBUILD_opencv_apps=OFF",
            "-DBUILD_opencv_java=OFF", "-DBUILD_opencv_python=OFF", "-DWITH_FFMPEG=OFF", "-DWITH_TIFF=OFF",
            "-DWITH_WEBP=OFF", "-DBUILD_IPP=OFF", "-DWITH_IPP=OFF",
            "-DWITH_OPENCL=OFF", "-DWITH_LAPACK=OFF", "-DWITH_QUIRC=OFF",
            "-DBUILD_ZLIB=ON", "-DWITH_ZLIB=ON",
            "-DBUILD_PNG=ON", "-DWITH_PNG=ON",
            "-DBUILD_JPEG=ON", "-DWITH_JPEG=ON",
            "-DBUILD_OPENEXR=OFF", "-DWITH_OPENEXR=OFF",
            "-DBUILD_SHARED_LIBS=OFF", "-DBUILD_WITH_STATIC_CRT=OFF",
            "-DWITH_ADE=OFF", "-DCMAKE_INSTALL_PREFIX=./build/install"
        ]
        
        try:
            subprocess.run(cmd, check=True, cwd=opencv_dir)
            subprocess.run(["cmake", "--build", "build", "--config", self.config, "--target", "install", "-j", "8"], 
                         check=True, cwd=opencv_dir)
            print("  ✓ OpenCV built successfully")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ OpenCV build failed: {e}")
            sys.exit(1)
        
        # Build GoogleTest
        print("  Building GoogleTest...")
        cmd = [
            "cmake", "-S", ".", "-G", self.generator, self.set_architecture, "-B", "build",
            "-DBUILD_GMOCK=OFF", "-DINSTALL_GTEST=OFF", "-DBUILD_SHARED_LIBS=ON"
        ]
        
        try:
            subprocess.run(cmd, check=True, cwd=gtest_dir)
            subprocess.run(["cmake", "--build", "build/googletest", "--config", self.config], 
                         check=True, cwd=gtest_dir)
            print("  ✓ GoogleTest built successfully")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ GoogleTest build failed: {e}")
            sys.exit(1)
        
        # Build ONNX Runtime
        print("  Building ONNX Runtime...")
        cmd = [
            "build.bat", "--config", self.config, "--build_shared_lib", "--parallel",
            "--compile_no_warning_as_error", "--skip_submodule_sync",
            "--cmake_generator", "Visual Studio 17 2022", "--update", "--build"
        ]
        
        if self.onnxruntime_flags:
            cmd.append(self.onnxruntime_flags)
        
        try:
            subprocess.run(cmd, check=True, cwd=onnx_dir, shell=True)
            print("  ✓ ONNX Runtime built successfully")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ ONNX Runtime build failed: {e}")
            sys.exit(1)
    
    def configure_cmake(self):
        """Configure CMake for the main project"""
        print(f"\nConfiguring CMake...")
        
        # Clean build directory
        if self.build_dir.exists():
            print(f"  Removing existing build directory: {self.build_dir}")
            shutil.rmtree(self.build_dir)
        
        self.build_dir.mkdir(parents=True, exist_ok=True)
        
        # Build CMake command
        cmd = [
            "cmake", "-S", ".", "-G", self.generator, self.set_architecture, "-B", str(self.build_dir),
            f"-DCMAKE_INSTALL_PREFIX={self.install_dir}",
            f"-DDOWNLOAD_ONNX={'OFF' if self.use_conan else 'ON'}",
            f"-DUSE_CONAN={'ON' if self.use_conan else 'OFF'}",
            "-DOS=windows",
            f"-DCMAKE_BUILD_TYPE={self.config}",
            f"-DARCHITECTURE={self.architecture}",
            f"-DDOWNLOAD_MODELS_AND_IMAGES={'ON' if self.download else 'OFF'}",
            f"-DVS_VERSION={self.vs_version}"
        ]
        
        print(f"  Running: {' '.join(cmd)}")
        
        try:
            subprocess.run(cmd, check=True, cwd=self.root_dir)
            print("  ✓ CMake configuration successful")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ CMake configuration failed: {e}")
            sys.exit(1)
    
    def build_project(self):
        """Build the main OFIQ project"""
        print(f"\nBuilding OFIQ project ({self.config})...")
        
        cmd = [
            "cmake", "--build", str(self.build_dir), "--config", self.config,
            "--target", "install", "-j", "8"
        ]
        
        print(f"  Running: {' '.join(cmd)}")
        
        try:
            subprocess.run(cmd, check=True, cwd=self.root_dir)
            print("  ✓ OFIQ build successful")
        except subprocess.CalledProcessError as e:
            print(f"  ✗ OFIQ build failed: {e}")
            sys.exit(1)
    
    def verify_build(self):
        """Verify that the build was successful"""
        print(f"\nVerifying build...")
        
        # Check for ofiq_lib.dll
        install_bin_dir = self.root_dir / self.install_dir / "Release" / "bin"
        if self.config == "Debug":
            install_bin_dir = self.root_dir / self.install_dir / "Debug" / "bin"
        
        ofiq_lib_path = install_bin_dir / "ofiq_lib.dll"
        
        if ofiq_lib_path.exists():
            print(f"  ✓ ofiq_lib.dll found: {ofiq_lib_path}")
            file_size = ofiq_lib_path.stat().st_size
            print(f"  ✓ File size: {file_size:,} bytes")
        else:
            print(f"  ✗ ofiq_lib.dll not found at: {ofiq_lib_path}")
            # Check what files are in the directory
            if install_bin_dir.exists():
                files = list(install_bin_dir.iterdir())
                print(f"  Files in directory: {[f.name for f in files]}")
            sys.exit(1)
        
        # Check for OFIQSampleApp
        sample_app_path = install_bin_dir / "OFIQSampleApp.exe"
        if sample_app_path.exists():
            print(f"  ✓ OFIQSampleApp.exe found: {sample_app_path}")
        else:
            print(f"  ✗ OFIQSampleApp.exe not found")
        
        print("  ✓ Build verification completed successfully")
    
    def run(self):
        """Main execution flow"""
        print("OFIQ Python Build Script")
        print("=" * 50)
        
        try:
            self.parse_arguments()
            self.setup_environment()
            self.check_prerequisites()
            self.build_dependencies()
            self.configure_cmake()
            self.build_project()
            self.verify_build()
            
            print("\n" + "=" * 50)
            print("🎉 BUILD SUCCESSFUL!")
            print(f"📁 Output directory: {self.root_dir / self.install_dir}")
            print(f"📄 Library: ofiq_lib.dll")
            print(f"🚀 Sample: OFIQSampleApp.exe")
            print("\nYou can now use the built library in your projects.")
            
        except KeyboardInterrupt:
            print("\n\nBuild interrupted by user")
            sys.exit(1)
        except Exception as e:
            print(f"\n\nBuild failed with error: {e}")
            sys.exit(1)


def main():
    """Main entry point"""
    builder = OFIQBuilder()
    builder.run()


if __name__ == "__main__":
    main()
