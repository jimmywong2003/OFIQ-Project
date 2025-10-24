# OFIQ C# Wrapper Build Script
# Builds the entire solution and runs basic verification

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

Write-Host "OFIQ C# Wrapper Build Script" -ForegroundColor Green
Write-Host "=============================" -ForegroundColor Green
Write-Host ""

# Check if .NET 8 is available
$dotnetVersion = dotnet --version
if ($LASTEXITCODE -ne 0) {
    Write-Error ".NET SDK is not available. Please install .NET 8.0 or later."
    exit 1
}

Write-Host "Using .NET SDK version: $dotnetVersion" -ForegroundColor Yellow

# Build the solution
Write-Host "Building solution..." -ForegroundColor Yellow
dotnet build "OFIQ-CSharp-Wrapper.sln" --configuration $Configuration --platform $Platform

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host "Build completed successfully!" -ForegroundColor Green

# Display project information
Write-Host ""
Write-Host "Project Structure:" -ForegroundColor Cyan
Write-Host "------------------" -ForegroundColor Cyan
Write-Host "Core Projects:" -ForegroundColor White
Write-Host "  - OFIQ.Native: P/Invoke interop layer"
Write-Host "  - OFIQ.Core: Core data structures and types"
Write-Host "  - OFIQ: Main API (OFIQEngine)"
Write-Host ""
Write-Host "Samples:" -ForegroundColor White
Write-Host "  - OFIQ.Samples.Console: Console application example"
Write-Host ""
Write-Host "Target Framework: .NET 8.0" -ForegroundColor White
Write-Host "Platform: $Platform" -ForegroundColor White
Write-Host "Configuration: $Configuration" -ForegroundColor White

# Display usage instructions
Write-Host ""
Write-Host "Usage Instructions:" -ForegroundColor Cyan
Write-Host "-------------------" -ForegroundColor Cyan
Write-Host "1. Ensure OFIQ native library is available in your runtime path"
Write-Host "2. Use the OFIQEngine class to initialize and assess image quality"
Write-Host "3. Sample console application:"
Write-Host "   dotnet run --project samples/OFIQ.Samples.Console -- <config-dir> <image-path>"
Write-Host ""
Write-Host "Example:"
Write-Host "  dotnet run --project samples/OFIQ.Samples.Console -- C:\OFIQ\config C:\images\face.jpg"
Write-Host ""

Write-Host "Build script completed!" -ForegroundColor Green
