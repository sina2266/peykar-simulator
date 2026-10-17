using Xunit;

namespace RDotNet
{
   public class PerformanceTest : RDotNetTestFixture
   {
      [Fact(Skip = "as imported")]
      public void TestCreateNumericVector()
      {
         SetUpTest();
         var engine = this.Engine;
         RuntimeDiagnostics r = new RuntimeDiagnostics(engine);
         int n = (int)1e6;
         var dt = r.MeasureRuntime(RuntimeDiagnostics.CreateNumericVector, n);
         Assert.True(dt < 100, "Creation of a 1 million element numeric vector is less than a tenth of a second");
      }
   }
}