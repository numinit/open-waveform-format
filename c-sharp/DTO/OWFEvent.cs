using System;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    ///     Event is a way of reporting a non-alarm event from a device.
    /// </summary>
    public class OWFEvent {
        private readonly Int64 _t0;
        private readonly OWFString _message;

        public OWFEvent(OWFString message, Int64 t0) {
            this._t0 = t0;
            this._message = message;
        }

        public OWFEvent(string message, DateTime time)
            : this(new OWFString(message), OWFTime.FromDateTime(time)) {}

        public OWFString Message
        {
            get { return this._message; }
        }

        public Int64 T0
        {
            get { return this._t0; }
        }

        public DateTime DateTime
        {
            get { return OWFTime.ToDateTime(this._t0); }
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

            return other.T0 == this.T0 &&
                   other.Message.Equals(this.Message);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Message.GetHashCode();
                hash = (hash * 16777619) ^ this.T0.GetHashCode();
                return hash;
            }
        }
    }
}