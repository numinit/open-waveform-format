using System;
using System.Web;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using OWF.Serializers;
using OWF.DTO;
using System.Collections.Generic;

namespace OWF_test.Serializers
{
    [TestClass]
    public class BinarySerializerTests
    {
        private byte[] ReadOWF(string filename)
        {
            return System.IO.File.ReadAllBytes(String.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        [TestMethod]
        public void GeneratesCorrectEmptyObject()     
        {
            var p = new Package(new List<Channel>());
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = ReadOWF("binary_valid_empty");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty object");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyChannelObject()
        {
            var c = new Channel("BED_42", new List<Namespace>());
            var p = new Package(new List<Channel>(new Channel[] { c }));
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = ReadOWF("binary_valid_empty_channel");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty channel");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyNamespaceObject()
        {
            // XXX: Do ticks work like this?
            var t0 = DateTime.FromFileTimeUtc(14334371443018100L);
            var dt = new TimeSpan(0, 0, 3);

            var n = new Namespace("GEWAVE", t0, dt, new List<Signal>(), new List<Event>(), new List<Alarm>());
            var c = new Channel("BED_42", new List<Namespace>(new Namespace[] { n }));
            var p = new Package(new List<Channel>(new Channel[] { c }));
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = ReadOWF("binary_valid_empty_namespace");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty namespace");
        }
    }
}
