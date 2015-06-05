using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;



namespace OWF
{
    /// <summary>
    /// OWFPackage is a class which contains the waveform data for some number of channels.
    /// </summary>
    public class Package
    {

        protected List<Channel> channels = new List<Channel>();

        public List<Channel> Channels
        {
            get
            {
                return channels;
            }
        }

    }
}
