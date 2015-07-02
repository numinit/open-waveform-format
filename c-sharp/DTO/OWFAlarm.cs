using System;

namespace OWF.DTO {
    /// <summary>
    /// An OWFAlarm represents an ongoing device alarm measured at a particular point in time.
    /// </summary>
    public class OWFAlarm : OWFObject {
        private readonly Int64 _t0;
        private readonly UInt64 _dt;
        private readonly byte _level;
        private readonly byte _volume;
        private readonly OWFString _msgType;
        private readonly OWFString _message;

        /// <summary>
        /// Initializes this OWFAlarm.
        /// </summary>
        /// <param name="type">The type of the alarm.</param>
        /// <param name="message">The alarm message.</param>
        /// <param name="t0">The time at which this alarm was measured.</param>
        /// <param name="dt">The total time this alarm has been ongoing. Subtracting this from t0 will get the start time of the alarm.</param>
        /// <param name="level">The importance level of this alarm, in 00 to ff. User-defined.</param>
        /// <param name="volume">The volume, or "loudness," of this alarm, in 00 to ff. User-defined</param>
        public OWFAlarm(OWFString type, OWFString message, Int64 t0, UInt64 dt, byte level, byte volume) {
            this._t0 = t0;
            this._dt = dt;
            this._level = level;
            this._volume = volume;
            this._msgType = type;
            this._message = message;
        }

        public OWFAlarm(string type, string message, DateTime t0, TimeSpan dt, byte level, byte volume) :
            this(new OWFString(type), new OWFString(message), OWFTime.FromDateTime(t0), OWFTime.FromTimeSpan(dt), level, volume) {}

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
        /// Gets the alarm level.
        /// </summary>
        public byte Level
        {
            get { return this._level; }
        }

        /// <summary>
        /// Gets the alarm volume.
        /// </summary>
        public byte Volume
        {
            get { return this._volume; }
        }

        /// <summary>
        /// Gets the alarm message.
        /// </summary>
        public OWFString Message
        {
            get { return this._message; }
        }

        /// <summary>
        /// Gets the alarm type.
        /// </summary>
        public OWFString Type
        {
            get { return this._msgType; }
        }

        protected override UInt32 ComputeSizeInBytes() {
            checked {
                const uint timeSize = sizeof(Int64);
                const uint durationSize = sizeof(UInt64);
                const uint levelSize = sizeof(byte);
                const uint volumeSize = sizeof(byte);
                const uint paddingSize = sizeof(UInt16);
                var typeStringSize = this.Type.GetSizeInBytes();
                var messageStringSize = this.Message.GetSizeInBytes();

                return timeSize + durationSize + levelSize + volumeSize + paddingSize + typeStringSize + messageStringSize;
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFAlarm);
        }

        public bool Equals(OWFAlarm other) {
            if (other == null) {
                return false;
            }

            return other.T0 == this.T0 &&
                   other.Dt == this.Dt &&
                   other.Type.Equals(this.Type) &&
                   other.Message.Equals(this.Message) &&
                   other.Level == this.Level &&
                   other.Volume == this.Volume;
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Message.GetHashCode();
                hash = (hash * 16777619) ^ this.Type.GetHashCode();
                hash = (hash * 16777619) ^ this.Level.GetHashCode();
                hash = (hash * 16777619) ^ this.Volume.GetHashCode();
                hash = (hash * 16777619) ^ this.TimeSpan.GetHashCode();
                hash = (hash * 16777619) ^ this.T0.GetHashCode();
                return hash;
            }
        }
    }
}