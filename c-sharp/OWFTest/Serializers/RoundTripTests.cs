using System.IO;
using System.Security.Permissions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using OWF.Serializers;

namespace OWFTest.Serializers {
    [TestClass]
    public class RoundTripTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        private void RoundTrip(string file) {
            byte[] expected = this.ReadOWF(file);
            OWFPackage unpacked = BinaryUnpacker.Unpack(expected);
            byte[] repacked = BinaryPacker.Pack(unpacked);
            OWFPackage reUnpacked = BinaryUnpacker.Unpack(repacked);
            byte[] rerepacked = BinaryPacker.Pack(reUnpacked);

            CollectionAssert.AreEqual(expected, repacked);
            Assert.AreEqual(true, unpacked.Equals(reUnpacked));
            CollectionAssert.AreEqual(repacked, rerepacked);
        }

        [TestMethod]
        public void TestRoundTripEmpty() {
            this.RoundTrip("binary_valid_empty");
        }

        [TestMethod]
        public void TestRoundTripEmptyNamespace() {
            this.RoundTrip("binary_valid_empty_namespace");
        }

        [TestMethod]
        public void TestRoundTripEmptyChannel() {
            this.RoundTrip("binary_valid_empty_channel");
        }

        [TestMethod]
        public void TestRoundTripEmptyAll() {
            this.RoundTrip("binary_valid_empty_all");
        }

        [TestMethod]
        public void TestRoundTrip1() {
            this.RoundTrip("binary_valid_1");
        }

        [TestMethod]
        public void TestRoundTrip2() {
            this.RoundTrip("binary_valid_2");
        }

        [TestMethod]
        public void TestRoundTrip3() {
            this.RoundTrip("binary_valid_3");
        }
    }
}