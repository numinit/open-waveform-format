using System;
using System.Collections.Generic;
using System.Text;


namespace OWF.DTO
{
    /// <summary>
    /// OWFPackage is a class which contains the waveform data for some number of channels.
    /// </summary>
    public class Package
    {
        public Package(List<Channel> channels)
        {
            this.channels = channels;
        }

        readonly protected List<Channel> channels = new List<Channel>();

        public List<Channel> Channels
        {
            get
            {
                return channels;
            }
        }

        public UInt32 getSizeInBytes()
        {
            checked
            {
                UInt32 packageSize = 0;
                foreach (var channel in channels)
                {
                    packageSize += channel.getSizeInBytes();
                }

                return packageSize;
            }
        }

        public override bool Equals(Object o)
        {
            if (o == null)
            {
                return false;
            }

            Package other = o as Package;
            if ((Object)other == null)
            {
                return false;
            }

            return other.channels.Equals(channels);           
        }

        public bool Equals(Package other)
        {
            if (other == null)
            {
                return false;
            }

            return other.channels.Equals(channels);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // based on FNV
                int hash = (int)2166136261;
                hash = (hash * 16777619) ^ channels.GetHashCode();
                return hash;
            }
        }
    }
}
