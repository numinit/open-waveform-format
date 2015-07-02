using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;
using OWF.Serializers;

namespace OWF_test.Serializers {
    [TestClass]
    public class BinaryPackerTests {
        private byte[] ReadOWF(string filename) {
            return File.ReadAllBytes(string.Join("/", "..", "..", "..", "..", "example", "owf1_" + filename + ".owf"));
        }

        [TestMethod]
        public void PacksCorrectEmptyObject() {
            var p = new OWFPackage(new List<OWFChannel>());
            var buffer = new BinaryPacker().Convert(p);
            var expected = this.ReadOWF("binary_valid_empty");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty object");
        }

        [TestMethod]
        public void PacksCorrectEmptyChannelObject() {
            var c = new OWFChannel("BED_42", new List<OWFNamespace>());
            var p = new OWFPackage(new List<OWFChannel>(new[] {c}));
            var buffer = new BinaryPacker().Convert(p);
            var expected = this.ReadOWF("binary_valid_empty_channel");
            CollectionAssert.AreEqual(buffer, expected, "Incorrect empty channel");
        }

        [TestMethod]
        public void PacksCorrectEmptyNamespaceObject() {
            var t0 = OWFTime.ToDateTime(OWFTime.FromString("2015-07-01T00:00:00.0000000Z"));
            var dt = new TimeSpan(0, 0, 1);

            var n = new OWFNamespace("GEWAVE", t0, dt, new List<OWFSignal>(), new List<OWFEvent>(), new List<OWFAlarm>());
            var c = new OWFChannel("BED_42", new List<OWFNamespace>(new[] {n}));
            var p = new OWFPackage(new List<OWFChannel>(new[] {c}));
            var buffer = new BinaryPacker().Convert(p);
            var expected = this.ReadOWF("binary_valid_empty_namespace");
            CollectionAssert.AreEqual(expected, buffer, "Incorrect empty namespace");
        }

        [TestMethod]
        public void PacksCorrectOWF1() {
            double[] data = {
                double.NegativeInfinity, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, double.PositiveInfinity
            };
            var signals = new List<OWFSignal>(new[] {
                new OWFSignal("ECG_LEAD_2", "mV", data)
            });
            var alarms = new List<OWFAlarm>(new[] {
                new OWFAlarm(new OWFString("SPO2 LO"), new OWFString("43"), OWFTime.FromString("2015-07-01T00:00:00.8901234Z"), 10001234UL, 0, 255)
            });
            var events = new List<OWFEvent>(new[] {
                new OWFEvent(new OWFString("POST OK"), OWFTime.FromString("2015-07-01T00:00:00.1234567Z"))
            });
            var ns = new List<OWFNamespace>(new[] {
                new OWFNamespace(new OWFString("GEWAVE"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, signals, events, alarms)
            });
            var channels = new List<OWFChannel>(new[] {
                new OWFChannel("BED_42", ns)
            });
            var package = new OWFPackage(channels);
            var buffer = new BinaryPacker().Convert(package);
            var expected = this.ReadOWF("binary_valid_1");
            CollectionAssert.AreEqual(expected, buffer, "Incorrect binary_valid_1");
        }

        [TestMethod]
        public void PacksCorrectOWF2() {
            /* BED_42 */
            double[] bed42Ns1Signal1Data = {
                double.NegativeInfinity, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, double.PositiveInfinity
            };
            double[] bed42Ns1Signal2Data = {
                80, 82, 83, 81, 80, 0, 0
            };
            var bed42Ns1Signals = new List<OWFSignal>(new[] {
                new OWFSignal("ECG_LEAD_2", "mV", bed42Ns1Signal1Data),
                new OWFSignal("HR_SENSOR", "bpm", bed42Ns1Signal2Data) 
            });
            var bed42Ns1Events = new List<OWFEvent> (new[] {
                new OWFEvent(new OWFString("POST OK"), OWFTime.FromString("2015-07-01T00:00:00.1234567Z")),
                new OWFEvent(new OWFString("PING"), OWFTime.FromString("2015-07-01T00:00:00.8901234Z"))
            });
            var bed42Ns1Alarms = new List<OWFAlarm>(new[] {
                new OWFAlarm(new OWFString("SPO2 LO"), new OWFString("43"), OWFTime.FromString("2015-07-01T00:00:00.5678901Z"), 20001234UL, 0, 255),
                new OWFAlarm(new OWFString("ASYSTOLE"), new OWFString(), OWFTime.FromString("2015-07-01T00:00:00.2345678Z"), 30001234UL, 255, 255)
            });

            var bed42Ns2Signals = new List<OWFSignal>();
            var bed42Ns2Events = new List<OWFEvent>(new[] {
                new OWFEvent(new OWFString("POWER_GOOD"), OWFTime.FromString("2015-07-01T00:00:00.1234567Z")),
                new OWFEvent(new OWFString("PING"), OWFTime.FromString("2015-07-01T00:00:00.8901234Z"))
            });
            var bed42Ns2Alarms = new List<OWFAlarm>();
            var bed42Namespaces = new List<OWFNamespace>(new[] {
                new OWFNamespace(new OWFString("GEWAVE"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, bed42Ns1Signals, bed42Ns1Events, bed42Ns1Alarms),
                new OWFNamespace(new OWFString("DRAGER_VENT"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, bed42Ns2Signals, bed42Ns2Events, bed42Ns2Alarms)
            });

            var bed42 = new OWFChannel(new OWFString("BED_42"), bed42Namespaces);

            /* BED_43 */
            var bed43Ns1Signals = new List<OWFSignal>();
            var bed43Ns1Events = new List<OWFEvent>(new[] {
                new OWFEvent(new OWFString("VENT PHONE HOME"), OWFTime.FromString("2015-07-01T00:00:00.1234567Z")),
                new OWFEvent(new OWFString("POWER GOOD"), OWFTime.FromString("2015-07-01T00:00:00.8901234Z")) 
            });
            var bed43Ns1Alarms = new List<OWFAlarm>();
            var bed43Namespaces = new List<OWFNamespace>(new[] {
                new OWFNamespace(new OWFString("DRAGER_VENT"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, bed43Ns1Signals, bed43Ns1Events, bed43Ns1Alarms)
            });

            var bed43 = new OWFChannel(new OWFString("BED_43"), bed43Namespaces);

            /* RICE_RESEARCH_RPI_2 */
            double[] rpiNs1Signal1Data = {
                -20.5, -18.5, -3.2, -14
            };
            double[] rpiNs1Signal2Data = {
                -20.5, -18.5, -3.2, -14
            };
            var rpiNs1Signals = new List<OWFSignal>(new[] {
                new OWFSignal(new OWFString("left_channel"), new OWFString("dB"), rpiNs1Signal1Data),
                new OWFSignal(new OWFString("right_channel"), new OWFString("dB"), rpiNs1Signal2Data),
            });
            var rpiNs1Events = new List<OWFEvent>(new[] {
                new OWFEvent(new OWFString("{\"msg\" : \"ping\"}"), OWFTime.FromString("2015-07-01T00:00:00.1234567Z"))
            });
            var rpiNs1Alarms = new List<OWFAlarm>();

            double[] rpiNs2Signal1Data = {
                20.5
            };
            var rpiNs2Signals = new List<OWFSignal>(new[] {
                new OWFSignal(new OWFString("pressure"), new OWFString("pa"), rpiNs2Signal1Data)
            });
            var rpiNs2Events = new List<OWFEvent>();
            var rpiNs2Alarms = new List<OWFAlarm>();

            var rpiNamespaces = new List<OWFNamespace>(new[] {
                new OWFNamespace(new OWFString("Audio Sensor"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, rpiNs1Signals, rpiNs1Events, rpiNs1Alarms),
                new OWFNamespace(new OWFString("Pressure Sensor"), OWFTime.FromString("2015-07-01T00:00:00.0000000Z"), 10000000UL, rpiNs2Signals, rpiNs2Events, rpiNs2Alarms) 
            });

            var rpi = new OWFChannel(new OWFString("RICE_RESEARCH_RPI_2"), rpiNamespaces);

            /* PACKAGE */
            var package = new OWFPackage(new List<OWFChannel>(new[] {bed42, bed43, rpi}));
            var buffer = new BinaryPacker().Convert(package);
            var expected = this.ReadOWF("binary_valid_2");
            CollectionAssert.AreEqual(expected, buffer, "Incorrect binary_valid_2");
        }
    }
}