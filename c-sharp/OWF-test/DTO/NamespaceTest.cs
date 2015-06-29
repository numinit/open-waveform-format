using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO {
    [TestClass]
    public class NamespaceTest {
        [TestMethod]
        public void NamespaceConstructorAndGettersWork() {
            var id = "test namespace";
            var t0 = new DateTime(2015, 6, 8, 5, 30, 22);
            var span = new TimeSpan(1, 2, 3);
            var signals = new List<OWFSignal>();
            var events = new List<OWFEvent>();
            var alarms = new List<OWFAlarm>();

            var ns = new OWFNamespace(id, t0, span, signals, events, alarms);

            Assert.AreEqual("test namespace", ns.Id.Value, "Namespace should have its id set correctly.");
            Assert.AreEqual(new DateTime(2015, 6, 8, 5, 30, 22), ns.T0, 
                "Namespace should have its start time set correctly.");
            Assert.AreEqual(new TimeSpan(1, 2, 3), ns.Dt, "Namespace should have its duration set correctly.");
            CollectionAssert.AreEqual(new List<OWFSignal>(), ns.Signals,
                "Namespace should have its signals set correctly.");
            CollectionAssert.AreEqual(new List<OWFEvent>(), ns.Events, "Namespace should have its events set correctly.");
            CollectionAssert.AreEqual(new List<OWFAlarm>(), ns.Alarms, "Namespace should have its alarms set correctly.");
        }

        [TestMethod]
        public void NamespaceSizeWorks() {
            var ns = new OWFNamespace("GEWAVE", new DateTime(2015, 6, 8, 5, 30, 22), new TimeSpan(0, 0, 3),
                new List<OWFSignal>(), new List<OWFEvent>(), new List<OWFAlarm>());
            Assert.AreEqual(0x2cu, ns.GetSizeInBytes());
        }
    }
}