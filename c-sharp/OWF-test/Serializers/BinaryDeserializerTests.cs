using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.Serializers;

namespace OWF_test.Serializers {
    [TestClass]
    public class BinaryDeserializerTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        [TestMethod]
        public void DeserializesValidEmptyPacket() {
            var buf = this.ReadOWF("binary_valid_empty");
            var p = BinaryDeserializer.Convert(buf);
            Assert.AreEqual(p.Channels.Count, 0);
        }

        [TestMethod]
        public void DeserializesValidEmptyChannel() {
            var buf = this.ReadOWF("binary_valid_empty_channel");
            var p = BinaryDeserializer.Convert(buf);
            Assert.AreEqual(p.Channels.Count, 1);
            Assert.AreEqual(p.Channels[0].Id, "BED_42");
            Assert.AreEqual(p.Channels[0].Namespaces.Count, 0);
        }

        [TestMethod]
        public void DeserializesValidEmptyNamespace() {
            var buf = this.ReadOWF("binary_valid_empty_namespace");
            var p = BinaryDeserializer.Convert(buf);
            Assert.AreEqual(p.Channels.Count, 1);
            Assert.AreEqual(p.Channels[0].Id, "BED_42");
            Assert.AreEqual(p.Channels[0].Namespaces.Count, 1);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Id, "GEWAVE");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Dt, new TimeSpan(0, 0, 3));
            Assert.AreEqual(p.Channels[0].Namespaces[0].Signals.Count, 0);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Events.Count, 0);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms.Count, 0);
        }

        [TestMethod]
        public void DeserializesValidPacket1() {
            var buf = this.ReadOWF("binary_valid_1");
            var p = BinaryDeserializer.Convert(buf);
            Assert.AreEqual(p.Channels.Count, 1);
            Assert.AreEqual(p.Channels[0].Id, "BED_42");
            Assert.AreEqual(p.Channels[0].Namespaces.Count, 1);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Id, "GEWAVE");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Dt, new TimeSpan(0, 0, 3));
            Assert.AreEqual(p.Channels[0].Namespaces[0].Signals.Count, 1);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Signals[0].Id, "ECG_LEAD_2");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Signals[0].Unit, "mV");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Signals[0].Samples.Length, 11);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Events[0].Message, "POST OK");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms.Count, 1);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms[0].Level, 0);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms[0].Volume, 255);
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms[0].Type, "SPO2 LO");
            Assert.AreEqual(p.Channels[0].Namespaces[0].Alarms[0].Message, "43");
        }
    }
}