using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO {
    /// <summary>
    /// An OWFChannel contains devices broadcasting on a particular data source.
    /// </summary>
    public class OWFChannel : OWFObject {
        private readonly OWFString _id;
        private readonly List<OWFNamespace> _namespaces;

        /// <summary>
        /// Initializes this OWFChannel.
        /// </summary>
        /// <param name="id">The id</param>
        /// <param name="namespaces">A list of namespaces</param>
        public OWFChannel(OWFString id, List<OWFNamespace> namespaces) {
            this._id = id;
            this._namespaces = namespaces;
        }

        public OWFChannel(string id, List<OWFNamespace> namespaces)
            : this(new OWFString(id), namespaces) {}

        /// <summary>
        /// Gets the ID of this channel.
        /// </summary>
        public OWFString Id
        {
            get { return this._id; }
        }

        /// <summary>
        /// Gets a list of namespaces.
        /// </summary>
        public List<OWFNamespace> Namespaces
        {
            get { return this._namespaces; }
        }

        protected override UInt32 ComputeSizeInBytes() {
            checked {
                var idSize = this.Id.GetSizeInBytes();
                var namespaceSize = this.Namespaces.Aggregate<OWFNamespace, uint>(0, (current, ns) => current + ns.GetSizeInBytes());
                return sizeof(UInt32) + idSize + namespaceSize;
            }
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            return this.Equals(o as OWFChannel);
        }

        public bool Equals(OWFChannel other) {
            if (other == null) {
                return false;
            }

            return other.Id.Equals(this.Id) &&
                   other.Namespaces.SequenceEqual(this.Namespaces);
        }

        public override int GetHashCode() {
            unchecked {
                // based on FNV
                var hash = (int)2166136261;
                hash = (hash * 16777619) ^ this.Id.GetHashCode();
                hash = (hash * 16777619) ^ this.Namespaces.GetHashCode();
                return hash;
            }
        }
    }
}