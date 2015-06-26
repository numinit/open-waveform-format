using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO
{
    public class Namespace
    {

        public Namespace(string id, DateTime start, TimeSpan dt, List<Signal> signals, List<Event> events, List<Alarm> alarms)
        {
            this.id = id;
            this.duration = dt;
            this.startTime = start;
            this.alarms = alarms;
            this.events = events;
            this.signals = signals;
        }

        readonly protected string id;
        public string Id
        {
            get
            {
                return id;
            }
        }

        readonly protected List<Signal> signals = new List<Signal>();
        public List<Signal> Signals
        {
            get
            {
                return signals;
            }
        }

        readonly protected List<Alarm> alarms = new List<Alarm>();
        public List<Alarm> Alarms
        {
            get
            {
                return alarms;
            }
        }

        readonly protected List<Event> events = new List<Event>();
        public List<Event> Events
        {
            get
            {
                return events;
            }
        }

        readonly protected DateTime startTime;
        public DateTime t0
        {
            get
            {
                return startTime;
            }
        }

        readonly protected TimeSpan duration;
        public TimeSpan dt
        {
            get
            {
                return duration;
            }
        }

        public UInt32 getSizeInBytes()
        {
            checked
            {
                UInt32 idStringSizeSize = sizeof(UInt32);
                UInt32 idStringSize = (id.Length == 0) ? (0) : ((UInt32)(System.Text.Encoding.UTF8.GetByteCount(id)) + 1);
                UInt32 idStringPaddingSize = (idStringSize % 4 == 0) ? (0) : (4 - (idStringSize % 4));                
                UInt32 idSize = idStringPaddingSize + idStringSize + idStringSizeSize;
                UInt32 staticSize = sizeof(UInt64) * 2;

                UInt32 signalsSize = sizeof(UInt32); // length of signals
                foreach (var signal in signals)
                {
                    signalsSize += signal.getSizeInBytes();
                }

                UInt32 eventsSize = sizeof(UInt32); // length of events
                foreach (var evt in events)
                {
                    eventsSize += evt.getSizeInBytes();
                }

                UInt32 alarmsSize = sizeof(UInt32); // length of alarms
                foreach (var alarm in alarms)
                {
                    alarmsSize += alarm.getSizeInBytes();
                }

                return idSize
                       + signalsSize
                       + alarmsSize
                       + eventsSize
                       + staticSize;
            }
        }

        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Namespace other = o as Namespace;
            if ((Object)other == null)
            {
                return false;
            }

            return other.id == id &&
                                other.t0.Equals(t0) &&
                                other.dt.Equals(dt) &&
                                Enumerable.SequenceEqual(other.signals, signals) &&
                                Enumerable.SequenceEqual(other.alarms, alarms) &&
                                Enumerable.SequenceEqual(other.events, events);
        }

        public bool Equals(Namespace other)
        {
            if (other == null)
            {
                return false;
            }

            return other.id == id &&
                    other.t0.Equals(t0) &&
                    other.dt.Equals(dt) &&
                    Enumerable.SequenceEqual(other.signals, signals) &&
                    Enumerable.SequenceEqual(other.alarms, alarms) &&
                    Enumerable.SequenceEqual(other.events, events);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // based on FNV
                int hash = (int)2166136261;
                hash = (hash * 16777619) ^ id.GetHashCode();
                hash = (hash * 16777619) ^ t0.GetHashCode();
                hash = (hash * 16777619) ^ dt.GetHashCode();
                hash = (hash * 16777619) ^ signals.GetHashCode();
                hash = (hash * 16777619) ^ alarms.GetHashCode();
                hash = (hash * 16777619) ^ events.GetHashCode();
                return hash;
            }
        }
    }
}
