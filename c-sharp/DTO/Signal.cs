using System;
using System.Collections.Generic;
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

                UInt32 samplesSize = (UInt32) (sizeof(double) * samples.Length);
                return idSize + unitSize + samplesSize;
            }
        }
    }
}
