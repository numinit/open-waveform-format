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
    }
}
