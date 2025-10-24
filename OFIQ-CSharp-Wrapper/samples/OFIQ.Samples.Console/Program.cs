using OFIQ;
using OFIQ.Core.QualityMeasures;
using OFIQ.Core.Types;

namespace OFIQ.Samples.Console;

/// <summary>
/// Simple console application demonstrating OFIQ usage.
/// </summary>
class Program
{
    static async Task Main(string[] args)
    {
        System.Console.WriteLine("OFIQ C# Wrapper Sample Application");
        System.Console.WriteLine("==================================");
        System.Console.WriteLine();

        try
        {
            // Check if configuration directory is provided
            if (args.Length < 1)
            {
                System.Console.WriteLine("Usage: OFIQ.Samples.Console <config-directory> [image-path]");
                System.Console.WriteLine();
                System.Console.WriteLine("Example:");
                System.Console.WriteLine("  OFIQ.Samples.Console C:\\OFIQ\\config C:\\images\\face.jpg");
                return;
            }

            var configDirectory = args[0];
            var imagePath = args.Length > 1 ? args[1] : null;

            await RunOFIQAssessment(configDirectory, imagePath);
        }
        catch (Exception ex)
        {
            System.Console.WriteLine($"Error: {ex.Message}");
            System.Console.WriteLine();
            System.Console.WriteLine("Stack trace:");
            System.Console.WriteLine(ex.StackTrace);
        }
    }

    private static async Task RunOFIQAssessment(string configDirectory, string? imagePath)
    {
        System.Console.WriteLine($"Configuration Directory: {configDirectory}");

        if (string.IsNullOrEmpty(imagePath))
        {
            System.Console.WriteLine("No image path provided. Please provide an image path to assess quality.");
            return;
        }

        System.Console.WriteLine($"Image Path: {imagePath}");
        System.Console.WriteLine();

        // Initialize OFIQ engine
        System.Console.WriteLine("Initializing OFIQ engine...");
        using var engine = new OFIQEngine();
        engine.Initialize(configDirectory);

        System.Console.WriteLine($"OFIQ Version: {engine.GetVersion()}");
        System.Console.WriteLine();

        // Assess image quality
        System.Console.WriteLine("Assessing image quality...");
        var assessment = engine.AssessQuality(imagePath);

        // Display results
        DisplayAssessmentResults(assessment);
    }

    private static void DisplayAssessmentResults(FaceImageQualityAssessment assessment)
    {
        System.Console.WriteLine("Quality Assessment Results");
        System.Console.WriteLine("==========================");
        System.Console.WriteLine($"Overall Quality: {assessment.OverallQuality:F1}/100");
        System.Console.WriteLine();

        System.Console.WriteLine("Individual Quality Measures:");
        System.Console.WriteLine("----------------------------");

        var successfulMeasures = assessment.GetSuccessfulMeasures().ToList();
        var failedMeasures = assessment.GetFailedMeasures().ToList();

        // Display successful measures
        foreach (var measure in successfulMeasures.OrderBy(m => m.Measure))
        {
            System.Console.WriteLine($"  {measure.Measure,-40} {measure.QualityValue,6:F1} (Raw: {measure.RawScore:F2})");
        }

        // Display failed measures
        if (failedMeasures.Any())
        {
            System.Console.WriteLine();
            System.Console.WriteLine("Failed Measures:");
            foreach (var measure in failedMeasures.OrderBy(m => m.Measure))
            {
                System.Console.WriteLine($"  {measure.Measure,-40} {measure.ReturnCode}");
            }
        }

        System.Console.WriteLine();
        System.Console.WriteLine($"Summary: {successfulMeasures.Count} successful, {failedMeasures.Count} failed out of {assessment.QualityMeasures.Count} total measures");
    }
}
