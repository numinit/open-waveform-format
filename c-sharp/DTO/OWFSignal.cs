using System;
using System.Linq;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    /// A signal is a collection of measurements from a sensor on a device.
    /// </summary>
    public class OWFSignal : OWFObject {
        private readonly OWFString _id;
        private readonly OWFString _unit;
        private readonly double[] _samples;
        
        /// <summary>
        /// Initializes this OWFSignal.
        /// </summary>
        /// <param name="id">The signal ID.</param>
        /// <param name="unit">The units that the signal is measured in.</param>
        /// <param name="samples">An array of double-precision samples.</param>
        public OWFSignal(OWFString id, OWFString unit, double[] samples) {
            this._id = id;
            this._unit = unit;
            this._samples = samples;
        }

        public OWFSignal(string id, string unit, double[] samples)
            : this(new OWFString(id), new OWFString(unit), samples) {} 

        /// <summary>
        /// Gets the ID of this signal.
        /// </summary>
        public OWFString Id
        {
            get { return this._id; }
        }

        /// <summary>
        /// Gets the units for this signal.
        /// </summary>
        public OWFString Unit
        {
            get { return this._unit; }
        }

        /// <summary>
        /// Gets the sample array for this signal.
        /// </summary>
        public double[] Samples
        {
            get { return this._samples; }
        }

        protected override UInt32 ComputeSizeInBytes() {
            checked {
                var idSize = this.Id.GetSizeInBytes();
                var unitSize = this.Unit.GetSizeInBytes();
                return idSize + unitSize + sizeof(UInt32) + sizeof(double) * (UInt32)this.Samples.Length;
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFSignal);
        }

        public bool Equals(OWFSignal other) {
            if (other == null) {
                return false;
            }

            return other.Id.Equals(this.Id) &&
                   other.Unit.Equals(this.Unit) &&
                   other.Samples.SequenceEqual(this.Samples);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Id.GetHashCode();
                hash = (hash * 16777619) ^ this.Unit.GetHashCode();
                hash = (hash * 16777619) ^ this.Samples.GetHashCode();
                return hash;
            }
        }
    }
}