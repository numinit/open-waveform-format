using System;
using System.Xml;

namespace OWF.DTO {
    /// <summary>
    /// OWF time helpers.
    /// OWF stores its timestamps as 64-bit integers representing the number of 100ns intervals
    /// before or after the UNIX epoch (seven decimal places to the right of the number of seconds)
    /// This class provides some static helpers to build OWF timestamps from C# time constructs.
    /// </summary>
    public static class OWFTime {
        /// <summary>
        /// The epoch offset.
        /// </summary>
        private static readonly Int64 EpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTimeUtc();

        /// <summary>
        /// Converts a TimeSpan to a UInt64 to be used in a duration.
        /// </summary>
        /// <param name="span">The TimeSpan to convert</param>
        /// <returns>A UInt64 duration value.</returns>
        public static UInt64 FromTimeSpan(TimeSpan span) {
            return checked((UInt64)span.Ticks);
        }

        /// <summary>
        /// Converts a UInt64 duration value to a TimeSpan.
        /// </summary>
        /// <param name="duration">The duration value</param>
        /// <returns>A TimeSpan representing that duration</returns>
        public static TimeSpan ToTimeSpan(UInt64 duration) {
            return new TimeSpan(checked((Int64)duration));
        }

        /// <summary>
        /// Converts a DateTime to an Int64 OWF timestamp.
        /// </summary>
        /// <param name="time">The timestamp</param>
        /// <returns>An Int64 representing 100ns increments before or after the UNIX epoch</returns>
        public static Int64 FromDateTime(DateTime time) {
            return FromFileTime(time.ToFileTimeUtc());
        }

        /// <summary>
        /// Converts an Int64 OWF timestamp to a DateTime.
        /// </summary>
        /// <param name="unixTime">An Int64 representing 100ns increments before or after the UNIX epoch</param>
        /// <returns>A DateTime object</returns>
        public static DateTime ToDateTime(Int64 unixTime) {
            return DateTime.FromFileTimeUtc(ToFileTime(unixTime));
        }

        /// <summary>
        /// Converts a .NET filetime to an Int64.
        /// </summary>
        /// <param name="fileTime">The filetime</param>
        /// <returns>An Int64 representing 100ns increments before or after the UNIX epoch</returns>
        public static Int64 FromFileTime(Int64 fileTime) {
            return checked(fileTime - EpochOffset);
        }

        /// <summary>
        /// Converts an OWF Int64 timestamp to a .NET filetime.
        /// </summary>
        /// <param name="unixTime">An Int64 representing 100ns increments before or after the UNIX epoch</param>
        /// <returns>A filetime representing the provided UNIX timestamp</returns>
        public static Int64 ToFileTime(Int64 unixTime) {
            return checked(unixTime + EpochOffset);
        }

        /// <summary>
        /// Converts an RFC3339 timestamp to an OWF Int64
        /// </summary>
        /// <param name="str">The RFC3339 timestamp, as a string</param>
        /// <returns>An Int64 representing 100ns increments before or after the UNIX epoch</returns>
        public static Int64 FromString(string str) {
            return FromDateTime(XmlConvert.ToDateTime(str, XmlDateTimeSerializationMode.Utc));
        }

        /// <summary>
        /// Converts a DateTime to a RFC3339 string
        /// </summary>
        /// <param name="time">The DateTime to convert</param>
        /// <returns>An RFC3339 timestamp string</returns>
        public static string ToString(DateTime time) {
            return XmlConvert.ToString(time, XmlDateTimeSerializationMode.Utc);
        }

        /// <summary>
        /// Converts an Int64 representing 100ns increments before or after the UNIX epoch to a RFC3339 string
        /// </summary>
        /// <param name="unixTime">An Int64 representing 100ns increments before or after the UNIX epoch</param>
        /// <returns>An RFC3339 timestamp string</returns>
        public static string ToString(Int64 unixTime) {
            return ToString(ToDateTime(unixTime));
        }

        /// <summary>
        /// Returns an Int64 representing 100ns increments before or after the UNIX epoch,
        /// based on the current value of the hardware clock
        /// </summary>
        /// <returns>An Int64 representing 100ns increments before or after the UNIX epoch</returns>
        public static Int64 Now() {
            return FromDateTime(DateTime.UtcNow);
        }
    }
}