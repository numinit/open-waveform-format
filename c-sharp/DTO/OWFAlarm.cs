using System;

namespace OWF.DTO {
    /// <summary>
    ///     Alarm represents a device alarm, such as "SPO2 LO" or "LEAD FAIL".
    /// </summary>
    public class OWFAlarm {
        private readonly Int64 _t0;
        private readonly UInt64 _dt;
        private readonly byte _level;
        private readonly byte _volume;
        private readonly OWFString _msgType;
        private readonly OWFString _message;

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

        public Int64 T0
        {
            get { return this._t0; }
        }

        public UInt64 Dt
        {
            get { return this._dt; }
        }

        public DateTime DateTime
        {
            get { return OWFTime.ToDateTime(this._t0); }
        }

        public TimeSpan TimeSpan
        {
            get { return OWFTime.ToTimeSpan(this._dt); }
        }

        public byte Level
        {
            get { return this._level; }
        }

        public byte Volume
        {
            get { return this._volume; }
        }

        public OWFString Message
        {
            get { return this._message; }
        }

        public OWFString Type
        {
            get { return this._msgType; }
        }

        public uint GetSizeInBytes() {
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