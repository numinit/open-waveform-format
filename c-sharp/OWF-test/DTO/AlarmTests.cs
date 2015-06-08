using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWFTests
{
    [TestClass]
    public class AlarmTests
    {
        [TestMethod]
        public void AlarmConstructorAndGettersWork()
        {            
            var message = "test";
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            
            var alarm = new Alarm(time, message);

            Assert.AreEqual(alarm.Data, message, "Alarm should have same message it started with.");
            Assert.AreEqual(alarm.Time, new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc), "Alarm should have same time it started with.");
        }
    }
}
