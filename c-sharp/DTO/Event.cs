using System;
using System.Collections.Generic;
using System.Text;

namespace OWF.DTO
{
    /// <summary>
    /// Event is a way of reporting a non-alarm event from a device.
    /// </summary>
    public class Event
    {

        public Event( DateTime time, string data)
        {
            this.data = data;
            this.startTime = time;
        }

        readonly protected string data;
        public string Data
        {
            get
            {
                return data;
            }
        }

        readonly protected DateTime startTime;
        public DateTime Time
        {
            get
            {
                return startTime;
            }
        }

        public UInt32 getSizeInBytes()
        {
            checked
            {
                UInt32 timeSize = sizeof(Int64);
                UInt32 stringSizeSize = sizeof(UInt32);
                UInt32 stringSize = (data.Length == 0) ? (0) : ((UInt32)(System.Text.Encoding.UTF8.GetByteCount(data)) + 1);
                UInt32 paddingSize = (stringSize % 4 == 0) ? (0) : ( 4 - (stringSize % 4));
                return timeSize + stringSizeSize + stringSize + paddingSize;
            }
        }

        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Event other = o as Event;
            if ((Object)other == null)
            {
                return false;
            }

            return other.data == data &&
                    other.startTime.Equals(startTime);
        }

        public bool Equals(Event other)
        {
            if (other == null)
            {
                return false;
            }

            return other.data == data &&
                    other.startTime.Equals(startTime);
        }

        public override int GetHashCode()
        {
            unchecked {
                // based on FNV
                int hash = (int) 2166136261;
                hash = (hash * 16777619) ^ data.GetHashCode();
                hash = (hash * 16777619) ^ startTime.GetHashCode();
                return hash;
            }            
        }
    }
}
