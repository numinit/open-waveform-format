using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO {
    [TestClass]
    public class PackageTest {
        [TestMethod]
        public void PackageConstructorAndGettersWork() {
            var channels = new List<OWFChannel>();
            var package = new OWFPackage(channels);
            CollectionAssert.AreEqual(new List<OWFChannel>(), package.Channels,
                "Package should have the same channels it was made with.");
        }
    }
}