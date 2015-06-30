using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO {
    [TestClass]
    public class ChannelTests {
        [TestMethod]
        public void ChannelConstructorAndGettersWork() {
            var id = "test channel";
            var namespaces = new List<OWFNamespace>();

            var channel = new OWFChannel(id, namespaces);

            Assert.AreEqual("test channel", channel.Id.Value, "Channel should have the smae id it was made with.");
            CollectionAssert.AreEqual(new List<OWFNamespace>(), channel.Namespaces,
                "Channel should have the same namespaces it was made with.");
        }

        [TestMethod]
        public void ChannelSizeWorks() {
            var ns = new OWFNamespace("GEWAVE", new DateTime(2015, 6, 8, 5, 30, 22), new TimeSpan(0, 0, 3),
                new List<OWFSignal>(), new List<OWFEvent>(), new List<OWFAlarm>());
            var channel = new OWFChannel("BED_42", new List<OWFNamespace>(new[] {ns}));
            Assert.AreEqual(0x3cu, channel.GetSizeInBytes());
        }
    }
}