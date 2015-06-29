using System;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    ///     Event is a way of reporting a non-alarm event from a device.
    /// </summary>
    public class OWFEvent {
        private readonly DateTime _startTime;
        private readonly OWFString _message;

        public OWFEvent(DateTime time, string data) {
            this._message = new OWFString(data);
            this._startTime = time;
        }

        public OWFString Message
        {
            get { return this._message; }
        }

        public DateTime Time
        {
            get { return this._startTime; }
        }

        public uint GetSizeInBytes() {
            checked {
                return sizeof(UInt64) + this.Message.GetSizeInBytes();
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFEvent);
        }

        public bool Equals(OWFEvent other) {
            if (other == null) {
                return false;
            }

            return other.Message.Equals(this.Message) &&
                   other.Time.Equals(this.Time);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Message.GetHashCode();
                hash = (hash * 16777619) ^ this.Time.GetHashCode();
                return hash;
            }
        }
    }
}