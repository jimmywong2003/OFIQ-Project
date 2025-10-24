using OFIQ.Core.QualityMeasures;

namespace OFIQ.Core.Types;

/// <summary>
/// Represents the result of a quality measure assessment.
/// </summary>
/// <param name="Measure">The quality measure that was assessed.</param>
/// <param name="RawScore">The native quality score computed by the measure.</param>
/// <param name="QualityValue">The quality component value (0-100).</param>
/// <param name="ReturnCode">The return code indicating the success or failure of the assessment.</param>
public readonly record struct QualityMeasureResult(
    QualityMeasure Measure,
    double RawScore,
    double QualityValue,
    QualityMeasureReturnCode ReturnCode)
{
    /// <summary>
    /// Gets a value indicating whether the quality measure was successfully computed.
    /// </summary>
    public bool IsSuccess => ReturnCode == QualityMeasureReturnCode.Success;

    /// <summary>
    /// Gets a value indicating whether the quality value is within the valid range (0-100).
    /// </summary>
    public bool IsValidQualityValue => QualityValue >= 0 && QualityValue <= 100;

    /// <summary>
    /// Creates a successful quality measure result.
    /// </summary>
    /// <param name="measure">The quality measure.</param>
    /// <param name="rawScore">The native quality score.</param>
    /// <param name="qualityValue">The quality component value.</param>
    /// <returns>A successful quality measure result.</returns>
    public static QualityMeasureResult Success(QualityMeasure measure, double rawScore, double qualityValue)
        => new(measure, rawScore, qualityValue, QualityMeasureReturnCode.Success);

    /// <summary>
    /// Creates a failed quality measure result.
    /// </summary>
    /// <param name="measure">The quality measure.</param>
    /// <param name="returnCode">The failure return code.</param>
    /// <returns>A failed quality measure result.</returns>
    public static QualityMeasureResult Failure(QualityMeasure measure, QualityMeasureReturnCode returnCode)
        => new(measure, double.NaN, double.NaN, returnCode);
}

/// <summary>
/// Represents the return codes for quality measure assessments.
/// </summary>
public enum QualityMeasureReturnCode
{
    /// <summary>
    /// The quality measure was successfully computed.
    /// </summary>
    Success = 0,

    /// <summary>
    /// The quality measure could not be assessed due to insufficient data.
    /// </summary>
    FailureToAssess = 1,

    /// <summary>
    /// The quality measure computation failed due to an internal error.
    /// </summary>
    InternalError = 2,

    /// <summary>
    /// The quality measure is not implemented.
    /// </summary>
    NotImplemented = 3,

    /// <summary>
    /// The quality measure is not configured.
    /// </summary>
    NotConfigured = 4
}
