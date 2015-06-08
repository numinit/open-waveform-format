using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using System.Collections.Generic;

namespace OWF_test.DTO
{
    [TestClass]
    public class ChannelTests
    {
        [TestMethod]
        public void ChannelConstructorAndGettersWork()
        {
            var id = "test channel";
            var namespaces = new List<Namespace>();

            var channel = new Channel(id, namespaces);

            Assert.AreEqual(channel.Id, "test channel", "Channel should have the smae id it was made with.");
            CollectionAssert.AreEqual(channel.Namespaces, new List<Namespace>(), "Channel should have the same namespaces it was made with.");
        }
    }
}
