using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OWF
{
    /// <summary>
    /// Alarm represents a device alarm, such as "SPO2 LO" or "LEAD FAIL".
    /// </summary>
    class Alarm
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
    }
}
