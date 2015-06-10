using System;
using System.Collections.Generic;
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
                UInt32 idStringSize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(id));
                UInt32 idStringPaddingSize = idStringSize % 4;
                UInt32 idSize = idStringPaddingSize + idStringSize + idStringSizeSize;

                UInt32 signalsSize = 0;
                foreach (var signal in signals)
                {
                    signalsSize += signal.getSizeInBytes();
                    signalsSize += sizeof(UInt32); // each signal, packed, is length-prefixed
                }

                UInt32 eventsSize = 0;
                foreach (var evt in events)
                {
                    eventsSize += evt.getSizeInBytes();
                    eventsSize += sizeof(UInt32); // each event, packed, is length-prefixed
                }

                UInt32 alarmsSize = 0;
                foreach (var alarm in alarms)
                {
                    alarmsSize += alarm.getSizeInBytes();
                    alarmsSize += sizeof(UInt32); // each alarm, packed, is length-prefixed
                }                

                return idSize
                       + signalsSize
                       + alarmsSize
                       + eventsSize;
            }
        }
    }
}
