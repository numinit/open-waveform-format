using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OWF
{
    /// <summary>
    /// Event is a way of reporting a non-alarm event from a device.
    /// </summary>
    class Event
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
    }
}
