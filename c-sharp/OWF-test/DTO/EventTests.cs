using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO {
    [TestClass]
    public class EventTests {
        [TestMethod]
        public void EventConstructorAndGettersWork() {
            var message = "test event";
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);

            var evt = new OWFEvent(time, message);

            Assert.AreEqual(evt.Message.Value, message, "Event should have same message it started with.");
            Assert.AreEqual(evt.Time, new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc),
                "Event should have same time it started with.");
        }

        [TestMethod]
        public void EventEqualsWorks() {
            // we're going to try out some different combinations, and force different source refs
            var message = "duck";
            var message2 = "duck";
            var message3 = "goose";

            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time2 = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time3 = new DateTime(2015, 7, 1, 3, 4, 5, DateTimeKind.Utc);

            var evt = new OWFEvent(time, message);
            var evt2 = new OWFEvent(time2, message2);
            var evt3 = new OWFEvent(time3, message3);

            Assert.AreNotEqual(null, evt, "An event should never equal generic null");
            Assert.AreNotEqual((OWFEvent)null, evt, "An event should never equal event null");

            Assert.AreEqual(evt, evt2, "Two equivalent events should be equal");
            Assert.AreNotEqual(evt, evt3, "Two different events should not be equal");
        }

        [TestMethod]
        public void EventSizeWorks() {
            var time = new DateTime(1983, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var message = "";
            var evt = new OWFEvent(time, message);
            Assert.AreEqual(12u, evt.GetSizeInBytes(), "Events with no data should have aligned length 12");

            var message2 = "1";
            var evt2 = new OWFEvent(time, message2);
            Assert.AreEqual(16u, evt2.GetSizeInBytes(), "Events with length 1 data should have aligned length 16");

            var message3 = "12";
            var evt3 = new OWFEvent(time, message3);
            Assert.AreEqual(16u, evt3.GetSizeInBytes(), "Events with length 2 data should have aligned length 16");

            var message4 = "123";
            var evt4 = new OWFEvent(time, message4);
            Assert.AreEqual(16u, evt4.GetSizeInBytes(), "Events with length 3 data should have aligned length 16");

            var message5 = "1234";
            var evt5 = new OWFEvent(time, message5);
            Assert.AreEqual(20u, evt5.GetSizeInBytes(), "Events with length 4 data should have aligned length 24");
        }
    }
}