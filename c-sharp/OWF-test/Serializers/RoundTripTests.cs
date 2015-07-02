using System.IO;
using System.Security.Permissions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using OWF.Serializers;

namespace OWF_test.Serializers {
    [TestClass]
    public class RoundTripTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        private void RoundTrip(string file) {
            byte[] expected = this.ReadOWF(file);
            OWFPackage unpacked = new BinaryUnpacker().Convert(expected);
            byte[] repacked = new BinaryPacker().Convert(unpacked);
            OWFPackage reUnpacked = new BinaryUnpacker().Convert(repacked);
            byte[] rerepacked = new BinaryPacker().Convert(reUnpacked);
            //File.WriteAllBytes("repacked.owf", repacked);

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