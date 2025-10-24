namespace OFIQ.Core.Exceptions;

/// <summary>
/// Represents errors that occur during OFIQ operations.
/// </summary>
public class OFIQException : Exception
{
    /// <summary>
    /// Initializes a new instance of the <see cref="OFIQException"/> class.
    /// </summary>
    public OFIQException()
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="OFIQException"/> class with a specified error message.
    /// </summary>
    /// <param name="message">The message that describes the error.</param>
    public OFIQException(string message) : base(message)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="OFIQException"/> class with a specified error message and a reference to the inner exception that is the cause of this exception.
    /// </summary>
    /// <param name="message">The error message that explains the reason for the exception.</param>
    /// <param name="innerException">The exception that is the cause of the current exception, or a null reference if no inner exception is specified.</param>
    public OFIQException(string message, Exception innerException) : base(message, innerException)
    {
    }
}

/// <summary>
/// Represents errors that occur during OFIQ configuration operations.
/// </summary>
public class ConfigurationException : OFIQException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ConfigurationException"/> class.
    /// </summary>
    public ConfigurationException()
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ConfigurationException"/> class with a specified error message.
    /// </summary>
    /// <param name="message">The message that describes the error.</param>
    public ConfigurationException(string message) : base(message)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ConfigurationException"/> class with a specified error message and a reference to the inner exception that is the cause of this exception.
    /// </summary>
    /// <param name="message">The error message that explains the reason for the exception.</param>
    /// <param name="innerException">The exception that is the cause of the current exception, or a null reference if no inner exception is specified.</param>
    public ConfigurationException(string message, Exception innerException) : base(message, innerException)
    {
    }
}

/// <summary>
/// Represents errors that occur during image loading operations.
/// </summary>
public class ImageLoadException : OFIQException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ImageLoadException"/> class.
    /// </summary>
    public ImageLoadException()
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ImageLoadException"/> class with a specified error message.
    /// </summary>
    /// <param name="message">The message that describes the error.</param>
    public ImageLoadException(string message) : base(message)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ImageLoadException"/> class with a specified error message and a reference to the inner exception that is the cause of this exception.
    /// </summary>
    /// <param name="message">The error message that explains the reason for the exception.</param>
    /// <param name="innerException">The exception that is the cause of the current exception, or a null reference if no inner exception is specified.</param>
    public ImageLoadException(string message, Exception innerException) : base(message, innerException)
    {
    }
}
