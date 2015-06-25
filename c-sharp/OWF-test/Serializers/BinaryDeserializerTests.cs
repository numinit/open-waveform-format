using System;
using System.Web;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using OWF.Serializers;
using OWF.DTO;
using System.Collections.Generic;

namespace OWF_test.Serializers
{
    [TestClass]
    public class BinaryDeserializerTests
    {
        public byte[] ReadOWF(string filename)
        {
            return System.IO.File.ReadAllBytes(String.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }
        [TestMethod]
        public void DeserializesValidEmptyPacket()
        {
            byte[] buf = ReadOWF("binary_valid_empty");
            try
            {
                Package p = BinaryDeserializer.convert(buf);
                Assert.Equals(p.Channels.Count, 0);
            }
            catch (Exception e)
            {
                Assert.Fail(e.ToString());
            }
        }

        [TestMethod]
        public void DeserializesValidPacket1()
        {
            byte[] buf = ReadOWF("binary_valid_1");
            try
            {
                Package p = BinaryDeserializer.convert(buf);
                Assert.Equals(p.Channels.Count, 1);
                Assert.Equals(p.Channels[0].Id, "BED_42");
                Assert.Equals(p.Channels[0].Namespaces.Count, 1);
                Assert.Equals(p.Channels[0].Namespaces[0].Id, "GEWAVE");
                Assert.Equals(p.Channels[0].Namespaces[0].Signals.Count, 1);
                Assert.Equals(p.Channels[0].Namespaces[0].Signals[0].Id, "ECG_LEAD_2");
                Assert.Equals(p.Channels[0].Namespaces[0].Signals[0].Unit, "mV");
                Assert.Equals(p.Channels[0].Namespaces[0].Signals[0].Samples.Length, 11);
                Assert.Equals(p.Channels[0].Namespaces[0].Events[0].Data, "POST OK");
                Assert.Equals(p.Channels[0].Namespaces[0].Alarms.Count, 1);
                Assert.Equals(p.Channels[0].Namespaces[0].Alarms[0].Level, 0);
                Assert.Equals(p.Channels[0].Namespaces[0].Alarms[0].Volume, 255);
                Assert.Equals(p.Channels[0].Namespaces[0].Alarms[0].Type, "SPO2 LO");
                Assert.Equals(p.Channels[0].Namespaces[0].Alarms[0].Message, "43");
            }
            catch (Exception e)
            {
                Assert.Fail(e.ToString());
            }
        }
    }
}
