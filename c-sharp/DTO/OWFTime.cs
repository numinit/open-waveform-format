using System;
using System.Xml;

namespace OWF.DTO {
    public static class OWFTime {
        private static readonly Int64 EpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTimeUtc();

        public static UInt64 FromTimeSpan(TimeSpan span) {
            return checked((UInt64)span.Ticks);
        }

        public static TimeSpan ToTimeSpan(UInt64 duration) {
            return new TimeSpan(checked((Int64)duration));
        }

        public static Int64 FromDateTime(DateTime time) {
            return FromFileTime(time.ToFileTimeUtc());
        }

        public static DateTime ToDateTime(Int64 unixTime) {
            return DateTime.FromFileTimeUtc(ToFileTime(unixTime));
        }

        public static Int64 FromFileTime(Int64 fileTime) {
            return checked(fileTime - EpochOffset);
        }

        public static Int64 ToFileTime(Int64 unixTime) {
            return checked(unixTime + EpochOffset);
        }

        public static Int64 FromString(string str) {
            return FromDateTime(XmlConvert.ToDateTime(str, XmlDateTimeSerializationMode.Utc));
        }

        public static string ToString(DateTime time) {
            return XmlConvert.ToString(time, XmlDateTimeSerializationMode.Utc);
        }

        public static string ToString(Int64 unixTime) {
            return ToString(ToDateTime(unixTime));
        }

        public static Int64 Now() {
            return FromDateTime(DateTime.UtcNow);
        }
    }
}