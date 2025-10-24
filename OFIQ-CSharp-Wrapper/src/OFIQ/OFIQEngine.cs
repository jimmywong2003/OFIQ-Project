using System.Drawing;
using System.Runtime.InteropServices;
using OFIQ.Core.Exceptions;
using OFIQ.Core.QualityMeasures;
using OFIQ.Core.Types;
using OFIQ.Native.Interop;

namespace OFIQ;

/// <summary>
/// Main entry point for the OFIQ facial image quality assessment library.
/// </summary>
public sealed class OFIQEngine : IDisposable
{
    private bool _disposed = false;
    private bool _initialized = false;

    /// <summary>
    /// Initializes a new instance of the <see cref="OFIQEngine"/> class.
    /// </summary>
    public OFIQEngine()
    {
    }

    /// <summary>
    /// Initializes the OFIQ engine with the specified configuration.
    /// </summary>
    /// <param name="configDirectory">The directory containing the configuration file.</param>
    /// <param name="configFileName">The name of the configuration file (default: "ofiq_config.jaxn").</param>
    /// <exception cref="InvalidOperationException">Thrown when the engine is already initialized.</exception>
    /// <exception cref="OFIQException">Thrown when initialization fails.</exception>
    public void Initialize(string configDirectory, string configFileName = "ofiq_config.jaxn")
    {
        if (_initialized)
            throw new InvalidOperationException("OFIQ engine is already initialized.");

        if (string.IsNullOrWhiteSpace(configDirectory))
            throw new ArgumentException("Configuration directory cannot be null or empty.", nameof(configDirectory));

        if (string.IsNullOrWhiteSpace(configFileName))
            throw new ArgumentException("Configuration file name cannot be null or empty.", nameof(configFileName));

        var status = OFIQNative.Initialize(configDirectory, configFileName);
        CheckStatus(status, "Failed to initialize OFIQ engine");

        _initialized = true;
    }

    /// <summary>
    /// Assesses the quality of a facial image from a file.
    /// </summary>
    /// <param name="imagePath">The path to the image file.</param>
    /// <returns>The quality assessment results.</returns>
    /// <exception cref="InvalidOperationException">Thrown when the engine is not initialized.</exception>
    /// <exception cref="ArgumentException">Thrown when the image path is invalid.</exception>
    /// <exception cref="OFIQException">Thrown when quality assessment fails.</exception>
    public Core.Types.FaceImageQualityAssessment AssessQuality(string imagePath)
    {
        EnsureInitialized();

        if (string.IsNullOrWhiteSpace(imagePath))
            throw new ArgumentException("Image path cannot be null or empty.", nameof(imagePath));

        if (!File.Exists(imagePath))
            throw new ArgumentException($"Image file does not exist: {imagePath}", nameof(imagePath));

        using var image = LoadImage(imagePath);
        return AssessQuality(image);
    }

    /// <summary>
    /// Assesses the quality of a facial image from a <see cref="Bitmap"/>.
    /// </summary>
    /// <param name="image">The image to assess.</param>
    /// <returns>The quality assessment results.</returns>
    /// <exception cref="InvalidOperationException">Thrown when the engine is not initialized.</exception>
    /// <exception cref="ArgumentNullException">Thrown when the image is null.</exception>
    /// <exception cref="OFIQException">Thrown when quality assessment fails.</exception>
    public Core.Types.FaceImageQualityAssessment AssessQuality(Bitmap image)
    {
        EnsureInitialized();

        if (image == null)
            throw new ArgumentNullException(nameof(image));

        var imageData = GetImageData(image, out var width, out var height, out var channels);
        try
        {
            var status = OFIQNative.AssessQuality(
                imageData,
                width,
                height,
                channels,
                out var nativeAssessment);

            CheckStatus(status, "Failed to assess image quality");

            return ConvertAssessment(nativeAssessment);
        }
        finally
        {
            Marshal.FreeHGlobal(imageData);
        }
    }

    /// <summary>
    /// Gets the version of the OFIQ native library.
    /// </summary>
    /// <returns>The version string.</returns>
    public string GetVersion()
    {
        const int bufferSize = 256;
        var buffer = Marshal.AllocHGlobal(bufferSize);
        try
        {
            var length = OFIQNative.GetVersion(buffer, bufferSize);
            if (length <= 0 || length >= bufferSize)
                return "Unknown";

            return Marshal.PtrToStringUTF8(buffer, length) ?? "Unknown";
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }

    /// <summary>
    /// Releases all resources used by the OFIQ engine.
    /// </summary>
    public void Dispose()
    {
        if (!_disposed)
        {
            if (_initialized)
            {
                OFIQNative.Cleanup();
                _initialized = false;
            }
            _disposed = true;
        }
    }

    private void EnsureInitialized()
    {
        if (!_initialized)
            throw new InvalidOperationException("OFIQ engine is not initialized. Call Initialize() first.");
    }

    private static Bitmap LoadImage(string imagePath)
    {
        try
        {
            return new Bitmap(imagePath);
        }
        catch (Exception ex)
        {
            throw new ImageLoadException($"Failed to load image from {imagePath}", ex);
        }
    }

    private static unsafe nint GetImageData(Bitmap image, out int width, out int height, out int channels)
    {
        width = image.Width;
        height = image.Height;

        // Determine number of channels based on pixel format
        channels = image.PixelFormat switch
        {
            System.Drawing.Imaging.PixelFormat.Format24bppRgb => 3,
            System.Drawing.Imaging.PixelFormat.Format32bppArgb => 4,
            System.Drawing.Imaging.PixelFormat.Format8bppIndexed => 1,
            _ => throw new OFIQException($"Unsupported pixel format: {image.PixelFormat}")
        };

        var bitmapData = image.LockBits(
            new Rectangle(0, 0, width, height),
            System.Drawing.Imaging.ImageLockMode.ReadOnly,
            image.PixelFormat);

        try
        {
            var dataSize = width * height * channels;
            var imageData = Marshal.AllocHGlobal(dataSize);
            Buffer.MemoryCopy(bitmapData.Scan0.ToPointer(), imageData.ToPointer(), dataSize, dataSize);
            return imageData;
        }
        finally
        {
            image.UnlockBits(bitmapData);
        }
    }

    private static Core.Types.FaceImageQualityAssessment ConvertAssessment(Native.Interop.FaceImageQualityAssessment nativeAssessment)
    {
        var results = new List<QualityMeasureResult>();

        if (nativeAssessment.MeasureCount > 0 && nativeAssessment.Measures != nint.Zero)
        {
            var measureSize = Marshal.SizeOf<NativeQualityMeasureResult>();
            for (int i = 0; i < nativeAssessment.MeasureCount; i++)
            {
                var nativeMeasure = Marshal.PtrToStructure<NativeQualityMeasureResult>(
                    nativeAssessment.Measures + i * measureSize);

                var measure = (QualityMeasure)nativeMeasure.MeasureId;
                var returnCode = (QualityMeasureReturnCode)nativeMeasure.ReturnCode;

                var result = new QualityMeasureResult(
                    measure,
                    nativeMeasure.RawScore,
                    nativeMeasure.QualityValue,
                    returnCode);

                results.Add(result);
            }
        }

        return new Core.Types.FaceImageQualityAssessment(results, nativeAssessment.OverallQuality);
    }

    private static void CheckStatus(ReturnStatus status, string message)
    {
        if (status.Code != ReturnCode.Success)
        {
            var errorMessage = status.ErrorMessage != nint.Zero
                ? Marshal.PtrToStringUTF8(status.ErrorMessage) ?? "Unknown error"
                : "Unknown error";

            throw new OFIQException($"{message}: {errorMessage} (Code: {status.Code})");
        }
    }
}
