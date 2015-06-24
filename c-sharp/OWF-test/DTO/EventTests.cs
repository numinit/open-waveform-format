using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO
{
    [TestClass]
    public class EventTests
    {
        [TestMethod]
        public void EventConstructorAndGettersWork()
        {
            var message = "test event";
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            
            var evt = new Event(time, message);

            Assert.AreEqual(evt.Data, message, "Event should have same message it started with.");
            Assert.AreEqual(evt.Time, new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc), "Event should have same time it started with.");
        }
        
        [TestMethod]
        public void EventEqualsWorks()
        {
            // we're going to try out some different combinations, and force different source refs
            var message = "duck";
            var message2 = "duck";
            var message3 = "goose";

            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time2 = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time3 = new DateTime(2015, 7, 1, 3, 4, 5, DateTimeKind.Utc);

            var evt = new Event(time, message);
            var evt2 = new Event(time2, message2);
            var evt3 = new Event(time3, message3);

            Assert.AreNotEqual(evt, null, "An event should never equal generic null");
            Assert.AreNotEqual(evt, (Event)null, "An event should never equal event null");

            Assert.AreEqual(evt, evt2, "Two equivalent events should be equal");
            Assert.AreNotEqual(evt, evt3, "Two different events should not be equal");
        }
    }
}
