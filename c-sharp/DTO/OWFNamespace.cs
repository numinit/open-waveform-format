using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO {
    public class OWFNamespace {
        private readonly DateTime _t0;
        private readonly TimeSpan _dt;
        private readonly OWFString _id;
        
        private readonly List<OWFSignal> _signals;
        private readonly List<OWFEvent> _events;
        private readonly List<OWFAlarm> _alarms;

        public OWFNamespace(string id, DateTime start, TimeSpan dt, List<OWFSignal> signals, List<OWFEvent> events,
            List<OWFAlarm> alarms) {
            this._id = new OWFString(id);
            this._dt = dt;
            this._t0 = start;
            this._alarms = alarms;
            this._events = events;
            this._signals = signals;
        }

        public OWFString Id
        {
            get { return this._id; }
        }

        public List<OWFSignal> Signals
        {
            get { return this._signals; }
        }

        public List<OWFAlarm> Alarms
        {
            get { return this._alarms; }
        }

        public List<OWFEvent> Events
        {
            get { return this._events; }
        }

        public DateTime T0
        {
            get { return this._t0; }
        }

        public TimeSpan Dt
        {
            get { return this._dt; }
        }

        public UInt32 GetSizeInBytes() {
            checked {
                const UInt32 timeSize = sizeof(UInt64) * 2;
                var idSize = this.Id.GetSizeInBytes();
                var signalsSize = this.Signals.Aggregate<OWFSignal, uint>(sizeof(UInt32), (current, signal) => current + signal.GetSizeInBytes());
                var eventsSize = this.Events.Aggregate<OWFEvent, uint>(sizeof(UInt32), (current, evt) => current + evt.GetSizeInBytes());
                var alarmsSize = this.Alarms.Aggregate<OWFAlarm, uint>(sizeof(UInt32), (current, alarm) => current + alarm.GetSizeInBytes());

                return sizeof(UInt32) + timeSize + idSize + signalsSize + eventsSize + alarmsSize;
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFNamespace);
        }

        public bool Equals(OWFNamespace other) {
            if (other == null) {
                return false;
            }

            return other.Id.Equals(this.Id) &&
                   other.T0.Equals(this.T0) &&
                   other.Dt.Equals(this.Dt) &&
                   other.Signals.SequenceEqual(this.Signals) &&
                   other.Alarms.SequenceEqual(this.Alarms) &&
                   other.Events.SequenceEqual(this.Events);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Id.GetHashCode();
                hash = (hash * 16777619) ^ this.T0.GetHashCode();
                hash = (hash * 16777619) ^ this.Dt.GetHashCode();
                hash = (hash * 16777619) ^ this.Signals.GetHashCode();
                hash = (hash * 16777619) ^ this.Alarms.GetHashCode();
                hash = (hash * 16777619) ^ this.Events.GetHashCode();
                return hash;
            }
        }
    }
}