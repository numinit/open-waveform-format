using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO
{
    /// <summary>
    /// OWFChannel contains the devices broadcasting on a particular data source, like a patient bed.
    /// </summary>
    public class Channel
    {
        public Channel( string id, List<Namespace> namespaces)
        {
            this.id = id;
            this.namespaces = namespaces;
        }

        readonly protected string id;
        public string Id
        {
            get
            {
                return id;
            }
        }

        readonly protected List<Namespace> namespaces;
        public List<Namespace> Namespaces
        {
            get
            {
                return namespaces;
            }
        }

        public UInt32 getSizeInBytes()
        {
            checked
            {
                UInt32 idStringSizeSize = sizeof(UInt32);
                UInt32 idStringSize = (id.Length == 0) ? (0) : ((UInt32)(System.Text.Encoding.UTF8.GetByteCount(id)) + 1);
                UInt32 idStringPaddingSize = (idStringSize % 4 == 0) ? (0) : ( 4 - (idStringSize % 4) );                
                UInt32 idSize = idStringPaddingSize + idStringSize + idStringSizeSize;
                
                UInt32 namespaceSize = 0;
                foreach (var ns in namespaces)
                {
                    namespaceSize += ns.getSizeInBytes();
                    namespaceSize += sizeof(UInt32); // each namespace, packed, is length-prefixed
                }

                return idSize + namespaceSize;
            }
        }

        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Channel other = o as Channel;
            if ((Object)other == null)
            {
                return false;
            }

            return other.id == id &&
                    Enumerable.SequenceEqual(other.namespaces, namespaces);
        }

        public bool Equals(Channel other)
        {
            if (other == null)
            {
                return false;
            }

            return other.id == id &&
                    Enumerable.SequenceEqual(other.namespaces, namespaces);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // based on FNV
                int hash = (int)2166136261;
                hash = (hash * 16777619) ^ id.GetHashCode();
                hash = (hash * 16777619) ^ namespaces.GetHashCode();
                return hash;
            }
        }
    }
}
