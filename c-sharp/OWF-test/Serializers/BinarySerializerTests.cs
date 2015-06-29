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
            // XXX: Do ticks work like this?
            var t0 = OWFTime.ToDateTime(14334371443018100L);
            var dt = new TimeSpan(0, 0, 3);

            var n = new OWFNamespace("GEWAVE", t0, dt, new List<OWFSignal>(), new List<OWFEvent>(), new List<OWFAlarm>());
            var c = new OWFChannel("BED_42", new List<OWFNamespace>(new[] {n}));
            var p = new OWFPackage(new List<OWFChannel>(new[] {c}));
            var buffer = BinarySerializer.Convert(p);
            var expected = this.ReadOWF("binary_valid_empty_namespace");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty namespace");
        }
    }
}