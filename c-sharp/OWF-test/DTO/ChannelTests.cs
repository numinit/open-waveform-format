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

        [TestMethod]
        public void ChannelSizeWorks()
        {
            var ns = new Namespace("GEWAVE", new DateTime(2015, 6, 8, 5, 30, 22), new TimeSpan(0, 0, 3), new List<Signal>(), new List<Event>(), new List<Alarm>());
            var channel = new Channel("BED_42", new List<Namespace>(new Namespace[] { ns }));
            Assert.AreEqual(0x38U, channel.getSizeInBytes());
        }
    }
}
