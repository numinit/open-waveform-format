using System;
using System.Collections.Generic;
using System.Linq;

namespace OWF.DTO {
    /// <summary>
    ///     OWFPackage is a class which contains the waveform data for some number of channels.
    /// </summary>
    public class OWFPackage : OWFObject {
        private readonly List<OWFChannel> _channels;

        public OWFPackage(List<OWFChannel> channels) {
            this._channels = channels;
        }

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

            return other.Channels.Equals(this.Channels);
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