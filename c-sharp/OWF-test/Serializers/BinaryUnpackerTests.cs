using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.Serializers;

namespace OWF_test.Serializers {
    [TestClass]
    public class BinaryUnpackerTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        [TestMethod]
        public void UnpacksValidEmptyPacket() {
            var buf = this.ReadOWF("binary_valid_empty");
            var p = new BinaryUnpacker().Convert(buf);
            Assert.AreEqual(0, p.Channels.Count);
        }

        [TestMethod]
        public void UnpacksValidEmptyChannel() {
            var buf = this.ReadOWF("binary_valid_empty_channel");
            var p = new BinaryUnpacker().Convert(buf);
            Assert.AreEqual(1, p.Channels.Count);
            Assert.AreEqual("BED_42", p.Channels[0].Id.Value);
            Assert.AreEqual(0, p.Channels[0].Namespaces.Count);
        }

        [TestMethod]
        public void UnpacksValidEmptyNamespace() {
            var buf = this.ReadOWF("binary_valid_empty_namespace");
            var p = new BinaryUnpacker().Convert(buf);
            Assert.AreEqual(1, p.Channels.Count);
            Assert.AreEqual("BED_42", p.Channels[0].Id.Value);
            Assert.AreEqual(1, p.Channels[0].Namespaces.Count);
            Assert.AreEqual("GEWAVE", p.Channels[0].Namespaces[0].Id.Value);
            Assert.AreEqual(new TimeSpan(0, 0, 1), p.Channels[0].Namespaces[0].TimeSpan);
            Assert.AreEqual(0, p.Channels[0].Namespaces[0].Signals.Count);
            Assert.AreEqual(0, p.Channels[0].Namespaces[0].Events.Count);
            Assert.AreEqual(0, p.Channels[0].Namespaces[0].Alarms.Count);
        }

        [TestMethod]
        public void UnpacksValidPacket1() {
            var buf = this.ReadOWF("binary_valid_1");
            var p = new BinaryUnpacker().Convert(buf);
            Assert.AreEqual(1, p.Channels.Count, 1);
            Assert.AreEqual("BED_42", p.Channels[0].Id.Value);
            Assert.AreEqual(1, p.Channels[0].Namespaces.Count);
            Assert.AreEqual("GEWAVE", p.Channels[0].Namespaces[0].Id.Value);
            Assert.AreEqual(new TimeSpan(0, 0, 1), p.Channels[0].Namespaces[0].TimeSpan);
            Assert.AreEqual(1, p.Channels[0].Namespaces[0].Signals.Count);
            Assert.AreEqual("ECG_LEAD_2", p.Channels[0].Namespaces[0].Signals[0].Id.Value);
            Assert.AreEqual("mV", p.Channels[0].Namespaces[0].Signals[0].Unit.Value);
            Assert.AreEqual(9, p.Channels[0].Namespaces[0].Signals[0].Samples.Length);
            Assert.AreEqual("POST OK", p.Channels[0].Namespaces[0].Events[0].Message.Value);
            Assert.AreEqual(1, p.Channels[0].Namespaces[0].Alarms.Count);
            Assert.AreEqual((byte)0, p.Channels[0].Namespaces[0].Alarms[0].Level);
            Assert.AreEqual((byte)255, p.Channels[0].Namespaces[0].Alarms[0].Volume);
            Assert.AreEqual("SPO2 LO", p.Channels[0].Namespaces[0].Alarms[0].Type.Value);
            Assert.AreEqual("43", p.Channels[0].Namespaces[0].Alarms[0].Message.Value);
        }
    }
}