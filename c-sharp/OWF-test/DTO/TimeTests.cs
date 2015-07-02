using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWFTest.DTO {
    [TestClass]
    public class TimeTests {
        [TestMethod]
        public void TestFromTimeSpan() {
            UInt64 span = OWFTime.FromTimeSpan(TimeSpan.FromDays(1));
            Assert.AreEqual(864000000000UL, span);
        }

        [TestMethod]
        public void TestToTimeSpan() {
            TimeSpan ts = OWFTime.ToTimeSpan(864000000000UL);
            Assert.AreEqual(TimeSpan.FromDays(1), ts);
        }

        [TestMethod]
        public void TestFromDateTime() {
            Int64 t = OWFTime.FromDateTime(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc));
            Assert.AreEqual(0L, t);
        }

        [TestMethod]
        public void TestToDateTime() {
            DateTime t = OWFTime.ToDateTime(0L);
            Assert.AreEqual(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc), t);
        }

        [TestMethod]
        public void TestFromFileTime() {
            Int64 t = OWFTime.FromFileTime(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).ToFileTimeUtc());
            Assert.AreEqual(0L, t);
        }

        [TestMethod]
        public void TestToFileTime() {
            Int64 t = OWFTime.ToFileTime(0L);
            Assert.AreEqual(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).ToFileTimeUtc(), t);
        }

        [TestMethod]
        public void TestFromString() {
            Assert.AreEqual(0L, OWFTime.FromString("1970-01-01T00:00:00Z"));
        }

        [TestMethod]
        public void TestToStringLong() {
            Assert.AreEqual("1970-01-01T00:00:00Z", OWFTime.ToString(0L));
        }

        [TestMethod]
        public void TestToStringDateTime() {
            Assert.AreEqual("1970-01-01T00:00:00Z", OWFTime.ToString(new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)));
        }
    }
}