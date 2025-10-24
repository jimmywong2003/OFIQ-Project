using System.Runtime.InteropServices;
using System.Text;

namespace OFIQ.Native.Interop;

/// <summary>
/// Provides P/Invoke declarations for the OFIQ native library.
/// </summary>
public static partial class OFIQNative
{
    private const string LibraryName = "ofiq_lib";

    /// <summary>
    /// Initializes the OFIQ library with the specified configuration.
    /// </summary>
    /// <param name="configDir">The directory containing the configuration file.</param>
    /// <param name="configFile">The name of the configuration file.</param>
    /// <returns>A return status indicating success or failure.</returns>
    [LibraryImport(LibraryName, EntryPoint = "OFIQ_initialize", StringMarshalling = StringMarshalling.Utf8)]
    public static partial ReturnStatus Initialize(string configDir, string configFile);

    /// <summary>
    /// Assesses the quality of a facial image.
    /// </summary>
    /// <param name="imageData">Pointer to the image data.</param>
    /// <param name="width">The width of the image.</param>
    /// <param name="height">The height of the image.</param>
    /// <param name="channels">The number of color channels (3 for RGB, 1 for grayscale).</param>
    /// <param name="assessment">Output parameter for the quality assessment results.</param>
    /// <returns>A return status indicating success or failure.</returns>
    [LibraryImport(LibraryName, EntryPoint = "OFIQ_assessQuality")]
    public static partial ReturnStatus AssessQuality(
        nint imageData,
        int width,
        int height,
        int channels,
        out FaceImageQualityAssessment assessment);

    /// <summary>
    /// Assesses the quality of a facial image with preprocessing results.
    /// </summary>
    /// <param name="imageData">Pointer to the image data.</param>
    /// <param name="width">The width of the image.</param>
    /// <param name="height">The height of the image.</param>
    /// <param name="channels">The number of color channels.</param>
    /// <param name="assessment">Output parameter for the quality assessment results.</param>
    /// <param name="preprocessing">Output parameter for the preprocessing results.</param>
    /// <returns>A return status indicating success or failure.</returns>
    [LibraryImport(LibraryName, EntryPoint = "OFIQ_assessQualityWithPreprocessing")]
    public static partial ReturnStatus AssessQualityWithPreprocessing(
        nint imageData,
        int width,
        int height,
        int channels,
        out FaceImageQualityAssessment assessment,
        out FaceImageQualityPreprocessing preprocessing);

    /// <summary>
    /// Releases resources associated with the OFIQ library.
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "OFIQ_cleanup")]
    public static partial void Cleanup();

    /// <summary>
    /// Gets the version information of the OFIQ library.
    /// </summary>
    /// <param name="versionBuffer">Buffer to store the version string.</param>
    /// <param name="bufferSize">Size of the version buffer.</param>
    /// <returns>The length of the version string, or -1 if the buffer is too small.</returns>
    [LibraryImport(LibraryName, EntryPoint = "OFIQ_getVersion")]
    public static partial int GetVersion(nint versionBuffer, int bufferSize);
}

/// <summary>
/// Represents the return status from native OFIQ functions.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ReturnStatus
{
    /// <summary>
    /// The return code.
    /// </summary>
    public ReturnCode Code;

    /// <summary>
    /// An optional error message.
    /// </summary>
    public nint ErrorMessage;
}

/// <summary>
/// Represents the return codes from native OFIQ functions.
/// </summary>
public enum ReturnCode
{
    /// <summary>
    /// Operation completed successfully.
    /// </summary>
    Success = 0,

    /// <summary>
    /// Operation failed.
    /// </summary>
    Error = 1,

    /// <summary>
    /// Invalid parameter provided.
    /// </summary>
    InvalidParameter = 2,

    /// <summary>
    /// Memory allocation failed.
    /// </summary>
    OutOfMemory = 3,

    /// <summary>
    /// Configuration error.
    /// </summary>
    ConfigurationError = 4,

    /// <summary>
    /// Image format not supported.
    /// </summary>
    UnsupportedImageFormat = 5
}

/// <summary>
/// Represents the quality assessment results for a facial image.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct FaceImageQualityAssessment
{
    /// <summary>
    /// The number of quality measures in the results.
    /// </summary>
    public int MeasureCount;

    /// <summary>
    /// Pointer to the array of quality measure results.
    /// </summary>
    public nint Measures;

    /// <summary>
    /// The overall quality score (0-100).
    /// </summary>
    public double OverallQuality;
}

/// <summary>
/// Represents the preprocessing results for a facial image.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct FaceImageQualityPreprocessing
{
    /// <summary>
    /// The number of detected faces.
    /// </summary>
    public int FaceCount;

    /// <summary>
    /// Pointer to the array of face bounding boxes.
    /// </summary>
    public nint FaceBoundingBoxes;

    /// <summary>
    /// The number of landmarks detected.
    /// </summary>
    public int LandmarkCount;

    /// <summary>
    /// Pointer to the array of landmark points.
    /// </summary>
    public nint Landmarks;

    /// <summary>
    /// The head pose angles (yaw, pitch, roll).
    /// </summary>
    public HeadPoseAngles HeadPose;
}

/// <summary>
/// Represents head pose angles in degrees.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct HeadPoseAngles
{
    /// <summary>
    /// Yaw angle (left-right rotation).
    /// </summary>
    public double Yaw;

    /// <summary>
    /// Pitch angle (up-down rotation).
    /// </summary>
    public double Pitch;

    /// <summary>
    /// Roll angle (tilt rotation).
    /// </summary>
    public double Roll;
}

/// <summary>
/// Represents a bounding box for a detected face.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct FaceBoundingBox
{
    /// <summary>
    /// X coordinate of the top-left corner.
    /// </summary>
    public double X;

    /// <summary>
    /// Y coordinate of the top-left corner.
    /// </summary>
    public double Y;

    /// <summary>
    /// Width of the bounding box.
    /// </summary>
    public double Width;

    /// <summary>
    /// Height of the bounding box.
    /// </summary>
    public double Height;

    /// <summary>
    /// Confidence score of the detection.
    /// </summary>
    public double Confidence;
}

/// <summary>
/// Represents a single quality measure result.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NativeQualityMeasureResult
{
    /// <summary>
    /// The quality measure identifier.
    /// </summary>
    public int MeasureId;

    /// <summary>
    /// The native quality score.
    /// </summary>
    public double RawScore;

    /// <summary>
    /// The quality component value (0-100).
    /// </summary>
    public double QualityValue;

    /// <summary>
    /// The return code for this measure.
    /// </summary>
    public int ReturnCode;
}
