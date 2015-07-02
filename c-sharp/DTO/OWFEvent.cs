using System;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    /// An event is an object representing an instant in time that something happens,
    /// as opposed to an ongoing alarm.
    /// </summary>
    public class OWFEvent : OWFObject {
        private readonly Int64 _t0;
        private readonly OWFString _message;

        /// <summary>
        /// Initializes this OWFEvent.
        /// </summary>
        /// <param name="message">The message.</param>
        /// <param name="t0">The time that this event happened.</param>
        public OWFEvent(OWFString message, Int64 t0) {
            this._t0 = t0;
            this._message = message;
        }

        public OWFEvent(string message, DateTime time)
            : this(new OWFString(message), OWFTime.FromDateTime(time)) {}

        /// <summary>
        /// Gets the message for this event.
        /// </summary>
        public OWFString Message
        {
            get { return this._message; }
        }

        /// <summary>
        /// Gets the timestamp for this event.
        /// </summary>
        public Int64 T0
        {
            get { return this._t0; }
        }

        /// <summary>
        /// Gets the timestamp for this event as a DateTime object.
        /// </summary>
        public DateTime DateTime
        {
            get { return OWFTime.ToDateTime(this._t0); }
        }

        protected override UInt32 ComputeSizeInBytes() {
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