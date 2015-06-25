using System;
using System.Collections.Generic;
using System.Text;


namespace OWF.DTO
{
    /// <summary>
    /// Alarm represents a device alarm, such as "SPO2 LO" or "LEAD FAIL".
    /// </summary>
    public class Alarm
    {
        public Alarm(DateTime time, TimeSpan duration, byte level, byte volume, string type, string message)
        {
            this.startTime = time;
            this.duration = duration;
            this.message = message;
            this.msgType = type;
            this.level = level;
            this.volume = volume;
        }

        readonly protected DateTime startTime;
        public DateTime Time
        {
            get
            {
                return startTime;
            }
        }

        readonly protected TimeSpan duration;
        public TimeSpan Duration
        {
            get
            {
                return duration;
            }
        }

        readonly protected byte level;
        public byte Level
        {
            get
            {
                return level;
            }
        }

        readonly protected byte volume;
        public byte Volume
        {
            get
            {
                return volume;
            }
        }
        
        readonly protected string message;
        public string Message
        {
            get
            {
                return message;
            }
        }

        readonly protected string msgType;
        public string Type
        {
            get
            {
                return msgType;
            }
        }

        public UInt32 getSizeInBytes()
        {
            checked
            {
                UInt32 timeSize = sizeof(Int64);
                UInt32 durationSize = sizeof(UInt64);
                UInt32 levelSize = sizeof(byte);
                UInt32 volumeSize = sizeof(byte);
                UInt32 paddingSize = 2 * sizeof(byte); // 2 extra unsued bytes after level and volume
                UInt32 typeStringSizeSize = sizeof(UInt32);                
                UInt32 typeStringSize = (msgType.Length == 0 ) ? (0) : ((UInt32)(System.Text.Encoding.UTF8.GetByteCount(msgType)) + 1 );
                UInt32 typeStringpaddingSize = (typeStringSize % 4 ==0) ? (0) : (4 - (typeStringSize % 4));
                UInt32 messageStringSizeSize = sizeof(UInt32);
                UInt32 messageStringSize = (message.Length == 0 ) ? (0) : ((UInt32)(System.Text.Encoding.UTF8.GetByteCount(message)) + 1);
                UInt32 messageStringpaddingSize = (messageStringSize % 4 ==0)? (0) : (4 - (messageStringSize % 4));

                return timeSize + durationSize
                        + levelSize + volumeSize + paddingSize
                        + typeStringSizeSize + typeStringSize + typeStringpaddingSize
                        + messageStringSizeSize + messageStringSize + messageStringpaddingSize;
            }
        }

        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Alarm other = o as Alarm;
            if ( (Object)other == null)
            {
                return false;
            }

            return other.msgType == msgType &&
                    other.duration.Equals(duration) &&
                    other.message == message &&
                    other.level == level &&
                    other.volume == volume &&
                    other.startTime.Equals(startTime);
        }

        public bool Equals(Alarm other)
        {
            if (other == null)
            {
                return false;
            }

            return other.msgType == msgType &&
                    other.duration.Equals(duration) &&
                    other.message == message &&
                    other.level == level &&
                    other.volume == volume &&
                    other.startTime.Equals(startTime);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // based on FNV
                int hash = (int)2166136261;
                hash = (hash * 16777619) ^ message.GetHashCode();
                hash = (hash * 16777619) ^ msgType.GetHashCode();
                hash = (hash * 16777619) ^ level;
                hash = (hash * 16777619) ^ volume;
                hash = (hash * 16777619) ^ duration.GetHashCode();
                hash = (hash * 16777619) ^ startTime.GetHashCode();
                return hash;
            }
        }
    }
}
