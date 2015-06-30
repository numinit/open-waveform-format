using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO {
    [TestClass]
    public class SignalTests {
        [TestMethod]
        public void SignalConstructorAndGettersWork() {
            var id = "test signal";
            var unit = "mg";
            double[] samples = {1, 2, 3, 4, 5, double.NaN, double.PositiveInfinity, double.NegativeInfinity, 0};

            var sig = new OWFSignal(id, unit, samples);


            Assert.AreEqual("test signal", sig.Id.Value, "Signal should have id assigned properly.");
            Assert.AreEqual("mg", sig.Unit.Value, "Signal should have units assigned properly.");

            CollectionAssert.AreEqual(sig.Samples,
                new double[] {1, 2, 3, 4, 5, double.NaN, double.PositiveInfinity, double.NegativeInfinity, 0},
                "Signal should have samples set properly.");
        }

        [TestMethod]
        public void SignalLengthWorks() {
            var id = "ECG_LEAD_2";
            var units = "mV";
            double[] data = {
                double.NegativeInfinity, -3.0, -2.0, -1.0, 0.0, double.NaN, 0.0, 1.0, 2.0, 3.0, double.PositiveInfinity
            };
            var sig = new OWFSignal(id, units, data);
            Assert.AreEqual(0x74U, sig.GetSizeInBytes());
        }
    }
}