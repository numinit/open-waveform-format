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
    }
}
