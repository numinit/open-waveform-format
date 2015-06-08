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
        public Alarm(DateTime time, string data)
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
                UInt32 stringSize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(data));
                UInt32 paddingSize = stringSize % 4;
                return timeSize + stringSizeSize + stringSize + paddingSize;
            }
        }
    }
}
