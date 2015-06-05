using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OWF
{
    public class Namespace
    {
        protected string id;
        public string Id
        {
            get
            {
                return id;
            }
        }

        protected List<Signal> signals = new List<Signal>();
        public List<Signal> Signals
        {
            get
            {
                return signals;
            }
        }

        protected List<Alarm> alarms = new List<Alarm>();
        public List<Alarm> Alarms
        {
            get
            {
                return alarms;
            }
        }

        protected List<Event> events = new List<Event>();
        public List<Event> Events
        {
            get
            {
                return events;
            }
        }

        protected DateTime startTime;
        public DateTime t0
        {
            get
            {
                return startTime;
            }
        }

        protected TimeSpan duration;
        public TimeSpan dt
        {
            get
            {
                return duration;
            }
        }

    }
}
