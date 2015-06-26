using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OWF.DTO;

namespace OWF_test.DTO
{
    [TestClass]
    public class SignalTests
    {
        [TestMethod]
        public void SignalConstructorAndGettersWork()
        {

            var id = "test signal";
            var unit = "mg";
            double [] samples = new double[] {1,2,3,4,5,double.NaN, double.PositiveInfinity, double.NegativeInfinity,0};
            
            var sig = new Signal(id, unit, samples );

            
            //Assert.AreEqual(sig.Id, "test signal", "Signal should have id assigned properly.");
            Assert.AreEqual(sig.Unit, "mg", "Signal should have units assigned properly.");

            CollectionAssert.AreEqual(sig.Samples,
                            new double[] { 1, 2, 3, 4, 5, double.NaN, double.PositiveInfinity, double.NegativeInfinity, 0 },
                            "Signal should have samples set properly.");
        }

        [TestMethod]
        public void SignalLengthWorks()
        {
            var id = "ECG_LEAD_2";
            var units = "mV";
            double[] data = new double[] { double.NegativeInfinity, -3.0, -2.0, -1.0, 0.0, double.NaN, 0.0, 1.0, 2.0, 3.0, double.PositiveInfinity };
            var sig = new Signal(id, units, data);
            Assert.AreEqual(0x74U, sig.getSizeInBytes());
        }
    }
}
