using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using System.Collections.Generic;

namespace OWF_test.DTO
{
    [TestClass]
    public class NamespaceTest
    {
        [TestMethod]
        public void NamespaceConstructorAndGettersWork()
        {
            var id = "test namespace";
            var t0 = new DateTime(2015,6,8,5,30,22);
            var span = new TimeSpan(1,2,3);
            var signals = new List<Signal>();
            var events = new List<Event>();
            var alarms = new List<Alarm>();
            
            var ns = new Namespace(id, t0, span, signals, events, alarms);

            Assert.AreEqual(ns.Id, "test namespace", "Namespace should have its id set correctly.");
            Assert.AreEqual(ns.t0, new DateTime(2015, 6, 8, 5, 30, 22), "Namespace should have its start time set correctly.");
            Assert.AreEqual(ns.dt, new TimeSpan(1, 2, 3), "Namespace should have its duration set correctly.");
            CollectionAssert.AreEqual(ns.Signals, new List<Signal>(), "Namespace should have its signals set correctly.");
            CollectionAssert.AreEqual(ns.Events, new List<Event>(), "Namespace should have its events set correctly.");
            CollectionAssert.AreEqual(ns.Alarms, new List<Alarm>(), "Namespace should have its alarms set correctly.");
        }

        [TestMethod]
        public void NamespaceSizeWorks()
        {
            var ns = new Namespace("GEWAVE", new DateTime(2015, 6, 8, 5, 30, 22), new TimeSpan(0, 0, 3), new List<Signal>(), new List<Event>(), new List<Alarm>());
            Assert.AreEqual(0x28U, ns.getSizeInBytes());
        }
    }
}
