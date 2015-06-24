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
            var message = "ALARM IS ON";
            var type = "TEST ALARM";
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var duration = new TimeSpan(5, 15, 10);
            byte level = 12;
            byte volume = 85;

            var alarm = new Alarm(time, duration, level, volume, type, message );

            Assert.AreEqual(alarm.Duration, duration, "Alarm should have same duration it started with.");
            Assert.AreEqual(alarm.Message, message, "Alarm should have same message it started with.");
            Assert.AreEqual(alarm.Level, level, "Alarm should have same level it started with.");
            Assert.AreEqual(alarm.Volume, volume, "Alarm should have same volume it started with.");
            Assert.AreEqual(alarm.Type, type, "Alarm should have the same type it started with.");
            Assert.AreEqual(alarm.Time, new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc), "Alarm should have same time it started with.");
        }

        [TestMethod]
        public void AlarmEqualsWorks()
        {
            // we're going to try out some different combinations, and force different source refs
            var message = "duck";
            var message2 = "duck";
            var message3 = "goose";

            var type = "duck alarm";
            var type2 = "duck alarm";
            var type3 = "goose alarm";

            var dur = new TimeSpan(5, 15, 10);
            var dur2 = new TimeSpan(5, 15, 10);
            var dur3 = new TimeSpan(2, 5, 7);

            byte level = 12;
            byte level2 = 12;
            byte level3 = 42;

            byte volume = 4;
            byte volume2 = 4;
            byte volume3 = 11;

            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time2 = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var time3 = new DateTime(2015, 7, 1, 3, 4, 5, DateTimeKind.Utc);

            var alarm = new Alarm(time, dur, level, volume, type, message);
            var alarm2 = new Alarm(time2, dur2, level2, volume2, type2, message2);
            var alarm3 = new Alarm(time3, dur3, level3, volume3, type3, message3);

            Assert.AreNotEqual(alarm, null, "An alarm should never equal generic null");
            Assert.AreNotEqual(alarm, (Alarm)null, "An alarm should never equal alarm null");

            Assert.AreEqual(alarm, alarm2, "Two equivalent alarms should be equal");
            Assert.AreNotEqual(alarm, alarm3, "Two different alarms should not be equal");
        }
    }
}
