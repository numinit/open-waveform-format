using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using OWF.Serializers;

namespace OWF_test.Serializers {
    [TestClass]
    public class BinarySerializerTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        [TestMethod]
        public void GeneratesCorrectEmptyObject() {
            var p = new OWFPackage(new List<OWFChannel>());
            var buffer = BinarySerializer.Convert(p);
            var expected = this.ReadOWF("binary_valid_empty");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty object");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyChannelObject() {
            var c = new OWFChannel("BED_42", new List<OWFNamespace>());
            var p = new OWFPackage(new List<OWFChannel>(new[] {c}));
            var buffer = BinarySerializer.Convert(p);
            var expected = this.ReadOWF("binary_valid_empty_channel");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty channel");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyNamespaceObject() {
            var t0 = OWFTime.ToDateTime(14334371443018100L);
            var dt = new TimeSpan(0, 0, 3);

            var n = new OWFNamespace("GEWAVE", t0, dt, new List<OWFSignal>(), new List<OWFEvent>(), new List<OWFAlarm>());
            var c = new OWFChannel("BED_42", new List<OWFNamespace>(new[] {n}));
            var p = new OWFPackage(new List<OWFChannel>(new[] {c}));
            var buffer = BinarySerializer.Convert(p);
            var expected = this.ReadOWF("binary_valid_empty_namespace");
            CollectionAssert.AreEqual(expected, buffer, "Incorrect empty namespace");
        }

        [TestMethod]
        public void GeneratesCorrectOWF1() {
            double[] data = {
                double.NegativeInfinity, -3.0, -2.0, -1.0, 0.0, double.NaN, 0.0, 1.0, 2.0, 3.0, double.PositiveInfinity
            };
            var signals = new List<OWFSignal>(new OWFSignal[] {new OWFSignal("ECG_LEAD_2", "mv", data)});
            var alarms = new List<OWFAlarm>(new OWFAlarm[] {new OWFAlarm(new OWFString("SPO2 LO"), new OWFString("43"), OWFTime.FromString("2015-06-04T16:59:04.3018100"), 30000000UL, 0, 255)});
            var events = new List<OWFEvent>(new OWFEvent[] {new OWFEvent(new OWFString("POST OK"), OWFTime.FromString("2015-06-04T16:59:04.3018350"))});
            var ns = new List<OWFNamespace>(new OWFNamespace[] {new OWFNamespace(new OWFString("GEWAVE"), OWFTime.FromString("2015-06-04T16:59:04.3018100"), 30000000UL, signals, events, alarms)});
            var channels = new List<OWFChannel>(new OWFChannel[] {new OWFChannel("BED_42", ns)});
            var package = new OWFPackage(channels);
            var buffer = BinarySerializer.Convert(package);
            var expected = this.ReadOWF("binary_valid_1");
            CollectionAssert.AreEqual(expected, buffer, "Incorrect binary_valid_1");
        }
    }
}