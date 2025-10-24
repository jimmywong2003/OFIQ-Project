namespace OFIQ.Core.QualityMeasures;

/// <summary>
/// Represents the quality measures defined in ISO/IEC 29794-5 standard.
/// </summary>
public enum QualityMeasure
{
    /// <summary>
    /// Unified quality score measure (0x41)
    /// </summary>
    UnifiedQualityScore = 0x41,

    /// <summary>
    /// Background uniformity measure (0x42)
    /// </summary>
    BackgroundUniformity = 0x42,

    /// <summary>
    /// Illumination uniformity measure (0x43)
    /// </summary>
    IlluminationUniformity = 0x43,

    /// <summary>
    /// Luminance mean measure (0x44)
    /// </summary>
    LuminanceMean = 0x44,

    /// <summary>
    /// Luminance variance measure (0x45)
    /// </summary>
    LuminanceVariance = 0x45,

    /// <summary>
    /// Under-exposure prevention measure (0x46)
    /// </summary>
    UnderExposurePrevention = 0x46,

    /// <summary>
    /// Over-exposure prevention measure (0x47)
    /// </summary>
    OverExposurePrevention = 0x47,

    /// <summary>
    /// Dynamic range measure (0x48)
    /// </summary>
    DynamicRange = 0x48,

    /// <summary>
    /// Sharpness measure (0x49)
    /// </summary>
    Sharpness = 0x49,

    /// <summary>
    /// Absence of compression artifacts measure (0x4A)
    /// </summary>
    NoCompressionArtifacts = 0x4A,

    /// <summary>
    /// Natural colour measure (0x4B)
    /// </summary>
    NaturalColour = 0x4B,

    /// <summary>
    /// Single face present measure (0x4C)
    /// </summary>
    SingleFacePresent = 0x4C,

    /// <summary>
    /// Eyes open measure (0x4D)
    /// </summary>
    EyesOpen = 0x4D,

    /// <summary>
    /// Mouth closed measure (0x4E)
    /// </summary>
    MouthClosed = 0x4E,

    /// <summary>
    /// Eyes visible measure (0x4F)
    /// </summary>
    EyesVisible = 0x4F,

    /// <summary>
    /// Mouth occlusion prevention measure (0x50)
    /// </summary>
    MouthOcclusionPrevention = 0x50,

    /// <summary>
    /// Face occlusion prevention measure (0x51)
    /// </summary>
    FaceOcclusionPrevention = 0x51,

    /// <summary>
    /// Inter-eye distance measure (0x52)
    /// </summary>
    InterEyeDistance = 0x52,

    /// <summary>
    /// Head size measure (0x53)
    /// </summary>
    HeadSize = 0x53,

    /// <summary>
    /// Leftward crop of the face image measure (0x54)
    /// </summary>
    LeftwardCropOfTheFaceImage = 0x54,

    /// <summary>
    /// Rightward crop of the face image measure (0x55)
    /// </summary>
    RightwardCropOfTheFaceImage = 0x55,

    /// <summary>
    /// Margin above of the face image measure (0x56)
    /// </summary>
    MarginAboveOfTheFaceImage = 0x56,

    /// <summary>
    /// Margin below of the face image measure (0x57)
    /// </summary>
    MarginBelowOfTheFaceImage = 0x57,

    /// <summary>
    /// Pose angle yaw frontal alignment measure (0x58)
    /// </summary>
    PoseAngleYawFrontalAlignment = 0x58,

    /// <summary>
    /// Pose angle pitch frontal alignment measure (0x59)
    /// </summary>
    PoseAnglePitchFrontalAlignment = 0x59,

    /// <summary>
    /// Pose angle roll frontal alignment measure (0x5A)
    /// </summary>
    PoseAngleRollFrontalAlignment = 0x5A,

    /// <summary>
    /// Expression neutrality measure (0x5B)
    /// </summary>
    ExpressionNeutrality = 0x5B,

    /// <summary>
    /// Absence of head coverings measure (0x5C)
    /// </summary>
    NoHeadCoverings = 0x5C
}
