namespace OFIQ.Core.Types;

/// <summary>
/// Represents a 2D point with X and Y coordinates.
/// </summary>
/// <param name="X">The X coordinate of the point.</param>
/// <param name="Y">The Y coordinate of the point.</param>
public readonly record struct LandmarkPoint(double X, double Y)
{
    /// <summary>
    /// Gets a value indicating whether this point is valid (non-negative coordinates).
    /// </summary>
    public bool IsValid => X >= 0 && Y >= 0;

    /// <summary>
    /// Represents an invalid landmark point.
    /// </summary>
    public static readonly LandmarkPoint Invalid = new(-1, -1);

    /// <summary>
    /// Calculates the Euclidean distance between two points.
    /// </summary>
    /// <param name="other">The other point to calculate distance to.</param>
    /// <returns>The Euclidean distance between the two points.</returns>
    public double DistanceTo(LandmarkPoint other)
    {
        var dx = X - other.X;
        var dy = Y - other.Y;
        return Math.Sqrt(dx * dx + dy * dy);
    }

    /// <summary>
    /// Creates a new point by adding the specified offsets.
    /// </summary>
    /// <param name="dx">The X offset to add.</param>
    /// <param name="dy">The Y offset to add.</param>
    /// <returns>A new point with the added offsets.</returns>
    public LandmarkPoint Add(double dx, double dy) => new(X + dx, Y + dy);

    /// <summary>
    /// Creates a new point by scaling the coordinates.
    /// </summary>
    /// <param name="scale">The scale factor to apply.</param>
    /// <returns>A new point with scaled coordinates.</returns>
    public LandmarkPoint Scale(double scale) => new(X * scale, Y * scale);
}
