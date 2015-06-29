using System;

namespace OWF.DTO {
    /// <summary>
    ///     Alarm represents a device alarm, such as "SPO2 LO" or "LEAD FAIL".
    /// </summary>
    public class OWFAlarm {
        private readonly DateTime _startTime;
        private readonly TimeSpan _duration;
        private readonly byte _level;
        private readonly byte _volume;
        private readonly OWFString _message;
        private readonly OWFString _msgType;

        public OWFAlarm(DateTime time, TimeSpan duration, byte level, byte volume, string type, string message) {
            this._startTime = time;
            this._duration = duration;
            this._message = new OWFString(message);
            this._msgType = new OWFString(type);
            this._level = level;
            this._volume = volume;
        }

        public DateTime Time
        {
            get { return this._startTime; }
        }

        public TimeSpan Duration
        {
            get { return this._duration; }
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

            return other.Type.Equals(this.Type) &&
                   other.Duration.Equals(this.Duration) &&
                   other.Message.Equals(this.Message) &&
                   other.Level == this.Level &&
                   other.Volume == this.Volume &&
                   other.Time.Equals(this.Time);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Message.GetHashCode();
                hash = (hash * 16777619) ^ this.Type.GetHashCode();
                hash = (hash * 16777619) ^ this.Level.GetHashCode();
                hash = (hash * 16777619) ^ this.Volume.GetHashCode();
                hash = (hash * 16777619) ^ this.Duration.GetHashCode();
                hash = (hash * 16777619) ^ this.Time.GetHashCode();
                return hash;
            }
        }
    }
}