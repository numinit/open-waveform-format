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
    }
}
