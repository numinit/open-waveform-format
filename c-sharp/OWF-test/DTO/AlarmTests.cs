using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWFTest.DTO {
    [TestClass]
    public class AlarmTests {
        [TestMethod]
        public void AlarmConstructorAndGettersWork() {
            var message = "ALARM IS ON";
            var type = "TEST ALARM";
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var duration = new TimeSpan(5, 15, 10);
            byte level = 12;
            byte volume = 85;

            var alarm = new OWFAlarm(type, message, time, duration, level, volume);

            Assert.AreEqual(new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc), alarm.DateTime,
                "Alarm should have same time it started with.");
            Assert.AreEqual(duration, alarm.TimeSpan, "Alarm should have same duration it started with.");
            Assert.AreEqual(type, alarm.Type.Value, "Alarm should have the same type it started with.");
            Assert.AreEqual(message, alarm.Message.Value, "Alarm should have same message it started with.");
            Assert.AreEqual(level, alarm.Level, "Alarm should have same level it started with.");
            Assert.AreEqual(volume, alarm.Volume, "Alarm should have same volume it started with.");
        }

        [TestMethod]
        public void AlarmEqualsWorks() {
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

            var alarm = new OWFAlarm(type, message, time, dur, level, volume);
            var alarm2 = new OWFAlarm(type2, message2, time2, dur2, level2, volume2);
            var alarm3 = new OWFAlarm(type3, message3, time3, dur3, level3, volume3);

            Assert.AreNotEqual(null, alarm, "An alarm should never equal generic null");
            Assert.AreNotEqual((OWFAlarm)null, alarm, "An alarm should never equal alarm null");

            Assert.AreEqual(alarm, alarm2, "Two equivalent alarms should be equal");
            Assert.AreNotEqual(alarm, alarm3, "Two different alarms should not be equal");
        }

        [TestMethod]
        public void AlarmSizeWorks() {
            var message = "";
            var type = "";
            var dur = new TimeSpan(5, 15, 10);
            byte level = 12;
            byte volume = 4;
            var time = new DateTime(1969, 7, 1, 3, 4, 5, DateTimeKind.Utc);
            var alarm = new OWFAlarm(type, message, time, dur, level, volume);
            var alarmSize = alarm.GetSizeInBytes();
            Assert.AreEqual(28u, alarmSize,
                "Alarms with message length 0 and type length 0 should have aligned length 28");

            var message2 = "12";
            var type2 = "1";
            var alarm2 = new OWFAlarm(type2, message2, time, dur, level, volume);
            var alarm2Size = alarm2.GetSizeInBytes();
            Assert.AreEqual(36u, alarm2Size,
                "Alarms with message length 2 and type length 1 should have aligned length 36");

            var message3 = "1234";
            var type3 = "12345";
            var alarm3 = new OWFAlarm(type3, message3, time, dur, level, volume);
            var alarm3Size = alarm3.GetSizeInBytes();
            Assert.AreEqual(44u, alarm3Size,
                "Alarms with message length 4 and type length 5 should have aligned length 44");
        }
    }
}