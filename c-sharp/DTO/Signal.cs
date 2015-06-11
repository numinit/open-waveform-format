using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OWF.DTO
{
    public class Signal
    {
        public Signal(string id, string unit, double[] samples )
        {
            this.id = id;
            this.unit = unit;
            this.samples = samples;
        }

        readonly protected string id;
        public string Id
        {
            get
            {
                return id;
            }
        }

        readonly protected string unit;
        public string Unit
        {
            get
            {
                return unit;
            }
        }

        readonly protected double[] samples;
        public double[] Samples
        {
            get
            {
                return samples;
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

                UInt32 unitStringSizeSize = sizeof(UInt32);
                UInt32 unitStringSize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(unit));
                UInt32 unitStringPaddingSize = unitStringSize % 4;
                UInt32 unitSize = unitStringSizeSize + unitStringSize + unitStringPaddingSize;

                UInt32 samplesSizeSize = sizeof(UInt32);
                UInt32 samplesSize = (UInt32) (sizeof(double) * samples.Length);
                return idSize + unitSize + samplesSize + samplesSizeSize;
            }
        }
        
        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Signal other = o as Signal;
            if ((Object)other == null)
            {
                return false;
            }

            return other.id == id &&
                    other.unit == unit &&
                    Enumerable.SequenceEqual(other.samples, samples);
        }

        public bool Equals(Signal other)
        {
            if (other == null)
            {
                return false;
            }

            return other.id == id &&
                    other.unit == unit &&
                    Enumerable.SequenceEqual(other.samples, samples);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // based on FNV
                int hash = (int)2166136261;
                hash = (hash * 16777619) ^ id.GetHashCode();
                hash = (hash * 16777619) ^ unit.GetHashCode();
                hash = (hash * 16777619) ^ samples.GetHashCode();
                return hash;
            }
        }
    }
}
