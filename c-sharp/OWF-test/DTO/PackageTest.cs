using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using System.Collections.Generic;


namespace OWF_test.DTO
{
    [TestClass]
    public class PackageTest
    {
        [TestMethod]
        public void PackageConstructorAndGettersWork()
        {
            var channels = new List<Channel>();

            var package = new Package(channels);

            CollectionAssert.AreEqual(package.Channels, new List<Channel>(), "Package should have the same channels it was made with.");
        }
    }
}
