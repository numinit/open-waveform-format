using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    /// A namespace represents a collection of signals, events, and alarms from a device on a channel.
    /// </summary>
    public class OWFNamespace : OWFObject {
        private readonly Int64 _t0;
        private readonly UInt64 _dt;
        private readonly OWFString _id;
        
        private readonly List<OWFSignal> _signals;
        private readonly List<OWFEvent> _events;
        private readonly List<OWFAlarm> _alarms;

        /// <summary>
        /// Initializes this OWFNamespace.
        /// </summary>
        /// <param name="id">The namespace ID.</param>
        /// <param name="t0">The timestamp at which this measurement starts.</param>
        /// <param name="dt">The length that this measurement goes on for.</param>
        /// <param name="signals">An array of signals.</param>
        /// <param name="events">An array of events.</param>
        /// <param name="alarms">An array of alarms.</param>
        public OWFNamespace(OWFString id, Int64 t0, UInt64 dt, List<OWFSignal> signals, List<OWFEvent> events, List<OWFAlarm> alarms) {
            this._t0 = t0;
            this._dt = dt;
            this._id = id;
            this._signals = signals;
            this._events = events;
            this._alarms = alarms;
        }

        public OWFNamespace(string id, DateTime t0, TimeSpan dt, List<OWFSignal> signals, List<OWFEvent> events, List<OWFAlarm> alarms)
                : this(new OWFString(id), OWFTime.FromDateTime(t0), OWFTime.FromTimeSpan(dt), signals, events, alarms) {}

        /// <summary>
        /// Gets the time of measurement as an Int64.
        /// </summary>
        public Int64 T0
        {
            get { return this._t0; }
        }

        /// <summary>
        /// Gets the total duration as a UInt64.
        /// </summary>
        public UInt64 Dt
        {
            get { return this._dt; }
        }

        /// <summary>
        /// Gets the time of measurement as a DateTime.
        /// </summary>
        public DateTime DateTime
        {
            get { return OWFTime.ToDateTime(this._t0); }
        }

        /// <summary>
        /// Gets the total duration as a TimeSpan.
        /// </summary>
        public TimeSpan TimeSpan
        {
            get { return OWFTime.ToTimeSpan(this._dt); }
        }

        /// <summary>
        /// Gets the namespace ID.
        /// </summary>
        public OWFString Id
        {
            get { return this._id; }
        }

        /// <summary>
        /// Gets the list of signals.
        /// </summary>
        public List<OWFSignal> Signals
        {
            get { return this._signals; }
        }

        /// <summary>
        /// Gets the list of alarms.
        /// </summary>
        public List<OWFAlarm> Alarms
        {
            get { return this._alarms; }
        }

        /// <summary>
        /// Gets the list of events.
        /// </summary>
        public List<OWFEvent> Events
        {
            get { return this._events; }
        }

        protected override UInt32 ComputeSizeInBytes() {
            checked {
                const UInt32 timeSize = sizeof(UInt64) * 2;
                var idSize = this.Id.GetSizeInBytes();
                var signalsSize = this.Signals.Aggregate<OWFSignal, UInt32>(sizeof(UInt32), (current, signal) => current + signal.GetSizeInBytes());
                var eventsSize = this.Events.Aggregate<OWFEvent, UInt32>(sizeof(UInt32), (current, evt) => current + evt.GetSizeInBytes());
                var alarmsSize = this.Alarms.Aggregate<OWFAlarm, UInt32>(sizeof(UInt32), (current, alarm) => current + alarm.GetSizeInBytes());

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
        
        /// <summary>
        /// Returns true if this namespace covers the given timestamp.
        /// That is: if the passed timestamp falls in the open interval [T0, T0 + Dt),
        /// this method will return true.
        /// </summary>
        /// <param name="timestamp">The timestamp to test</param>
        /// <returns>True if the namespace covers the timestamp, false otherwise.</returns>
        public bool Covers(Int64 timestamp) {
            Int64 start = this.T0, end = start + (Int64)this.Dt;
            return timestamp >= start && timestamp < end;
        }
    }
}