using OFIQ.Core.QualityMeasures;

namespace OFIQ.Core.Types;

/// <summary>
/// Represents the complete quality assessment results for a facial image.
/// </summary>
/// <param name="QualityMeasures">The collection of individual quality measure results.</param>
/// <param name="OverallQuality">The overall quality score (0-100).</param>
public readonly record struct FaceImageQualityAssessment(
    IReadOnlyList<QualityMeasureResult> QualityMeasures,
    double OverallQuality)
{
    /// <summary>
    /// Gets a value indicating whether the assessment contains any quality measures.
    /// </summary>
    public bool HasMeasures => QualityMeasures?.Count > 0;

    /// <summary>
    /// Gets the quality measure result for the specified measure.
    /// </summary>
    /// <param name="measure">The quality measure to retrieve.</param>
    /// <returns>The quality measure result, or null if not found.</returns>
    public QualityMeasureResult? GetMeasureResult(QualityMeasure measure)
    {
        return QualityMeasures?.FirstOrDefault(r => r.Measure == measure);
    }

    /// <summary>
    /// Gets all successful quality measure results.
    /// </summary>
    /// <returns>A collection of successful quality measure results.</returns>
    public IEnumerable<QualityMeasureResult> GetSuccessfulMeasures()
    {
        return QualityMeasures?.Where(r => r.IsSuccess) ?? Enumerable.Empty<QualityMeasureResult>();
    }

    /// <summary>
    /// Gets all failed quality measure results.
    /// </summary>
    /// <returns>A collection of failed quality measure results.</returns>
    public IEnumerable<QualityMeasureResult> GetFailedMeasures()
    {
        return QualityMeasures?.Where(r => !r.IsSuccess) ?? Enumerable.Empty<QualityMeasureResult>();
    }

    /// <summary>
    /// Gets a summary of the quality assessment.
    /// </summary>
    /// <returns>A summary string.</returns>
    public string GetSummary()
    {
        var successfulCount = GetSuccessfulMeasures().Count();
        var totalCount = QualityMeasures?.Count ?? 0;
        return $"Overall Quality: {OverallQuality:F1}, Successful Measures: {successfulCount}/{totalCount}";
    }
}
