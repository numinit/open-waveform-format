using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OWF
{
    public class Signal
    {
        protected string id;
        public string Id
        {
            get
            {
                return Id;
            }
        }

        protected string unit;
        public string Unit
        {
            get
            {
                return unit;
            }
        }

        protected double[] samples;
        public double[] Samples
        {
            get
            {
                return samples;
            }
        }
    }
}
