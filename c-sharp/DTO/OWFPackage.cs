using System;
using System.Collections.Generic;
using System.Linq;

namespace OWF.DTO {
    /// <summary>
    /// An OWFPackage contains waveform data for a number of channels.
    /// </summary>
    public class OWFPackage : OWFObject {
        private readonly List<OWFChannel> _channels;

        /// <summary>
        /// Initializes this OWFPackage with a list of channels.
        /// </summary>
        /// <param name="channels"></param>
        public OWFPackage(List<OWFChannel> channels) {
            this._channels = channels;
        }

        /// <summary>
        /// Gets the list of channels.
        /// </summary>
        public List<OWFChannel> Channels
        {
            get { return this._channels; }
        } 

        protected override UInt32 ComputeSizeInBytes() {
            checked {
                var packageSize = this.Channels.Aggregate<OWFChannel, uint>(0, (current, channel) => current + channel.GetSizeInBytes());
                return sizeof(UInt32) + packageSize;
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFPackage);
        }

        public bool Equals(OWFPackage other) {
            if (other == null) {
                return false;
            }

            return other.Channels.SequenceEqual(this.Channels);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ _channels.GetHashCode();
                return hash;
            }
        }
    }
}