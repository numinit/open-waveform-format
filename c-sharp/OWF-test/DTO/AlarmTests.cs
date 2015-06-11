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

        [TestMethod]
        public void AlarmEqualsWorks()
        {
            // we're going to try out some different combinations, and force different source refs
            var message = "duck";
            var message2 = "duck";
            var message3 = "goose";

            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time2 = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time3 = new DateTime(2015, 7, 1, 3, 4, 5, DateTimeKind.Utc);

            var alarm = new Alarm(time, message);
            var alarm2 = new Alarm(time2, message2);
            var alarm3 = new Alarm(time3, message3);

            Assert.AreNotEqual(alarm, null, "An alarm should never equal generic null");
            Assert.AreNotEqual(alarm, (Alarm)null, "An alarm should never equal alarm null");

            Assert.AreEqual(alarm, alarm2, "Two equivalent alarms should be equal");
            Assert.AreNotEqual(alarm, alarm3, "Two different alarms should not be equal");
        }
    }
}
