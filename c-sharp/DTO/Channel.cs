using System;
using System.Collections.Generic;
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
                UInt32 idStringSize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(id));
                UInt32 idStringPaddingSize = idStringSize % 4;
                UInt32 idSize = idStringPaddingSize + idStringSize + idStringSizeSize;
                
                UInt32 namespaceSize = 0;
                foreach (var ns in namespaces)
                {
                    namespaceSize += ns.getSizeInBytes();
                }

                return idSize + namespaceSize;
            }
        }
    }
}
