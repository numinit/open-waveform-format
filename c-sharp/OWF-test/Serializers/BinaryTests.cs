using System;
using System.Web;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using OWF.Serializers;
using OWF.DTO;
using System.Collections.Generic;

namespace OWF_test.Serializers
{
    [TestClass]
    public class BinaryTests
    {
        [TestMethod]
        public void GeneratesCorrectEmptyObject()     
        {
            var p = new Package(new List<Channel>());
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = new byte[] {0x4f, 0x57, 0x46, 0x31, /**/ 0, 0, 0, 0};
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty object");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyChannelObject()
        {
            var c = new Channel("TEST_CHANNEL", new List<Namespace>());
            var p = new Package(new List<Channel>(new Channel[] { c }));
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = {
                0x4f, 0x57, 0x46, 0x31, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x14,
                0x00, 0x00, 0x00, 0x10, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x48, 0x41,
                0x4e, 0x4e, 0x45, 0x4c, 0x00, 0x00, 0x00, 0x00
            };
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty channel");
        }

        [TestMethod]
        public void GeneratesCorrectEmptyNamespaceObject()
        {
            var t0 = new DateTime(2015, 6, 8, 5, 30, 22);
            var dt = new TimeSpan(1,2,3);

            var n = new Namespace("TEST_NAMESPACE", t0, dt, new List<Signal>(), new List<Event>(), new List<Alarm>());
            var c = new Channel("TEST_CHANNEL", new List<Namespace>(new Namespace[] { n }));
            var p = new Package(new List<Channel>(new Channel[] { c }));
            byte[] buffer = BinarySerializer.convert(p);
            byte[] expected = {
                0x4f, 0x57, 0x46, 0x31, 0x00, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x48,
                0x00, 0x00, 0x00, 0x10, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x48, 0x41,
                0x4e, 0x4e, 0x45, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
                0x00, 0x32, 0xef, 0xcd, 0x61, 0x94, 0xab, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x38, 0xce, 0xf8, 0x00, 0x00, 0x00, 0x10, 0x54, 0x45, 0x53, 0x54,
                0x5f, 0x4e, 0x41, 0x4d, 0x45, 0x53, 0x50, 0x41, 0x43, 0x45, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            };
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty namespace");
        }
    }
}
