using System;
using RDotNet;

namespace ClrFacade;

public class RdotnetDataConverterTests
{
   public class MemTestObjectRDotnet : NumericVector
   {
      public static int Counter = 0;

      public MemTestObjectRDotnet(double[] values)
         : base(REngine, values)
      {
         Counter++;
      }

      ~MemTestObjectRDotnet()
      {
         Counter--;
      }
   }

   public static int GetMemTestObjCounterRDotnet()
   {
      return MemTestObjectRDotnet.Counter;
   }

   public static object CreateMemTestObjRDotnet()
   {
      return new MemTestObjectRDotnet(TestCases.CreateNumArray());
   }

   internal static REngine REngine
   {
      get
      {
         var rDotNetConverter = InternalRSharpFacade.DataConverter as InternalRDotNetDataConverter;
         return rDotNetConverter == null ? null : InternalRDotNetDataConverter.GetEngine();
      }
   }

   private static Tuple<string, SymbolicExpression> tc(string name, double[] values)
   {
      return Tuple.Create<string, SymbolicExpression>(name, REngine.CreateNumericVector(values));
   }

   private static Tuple<string, SymbolicExpression> tc(string name, string[] values)
   {
      return Tuple.Create<string, SymbolicExpression>(name, REngine.CreateCharacterVector(values));
   }

   public static SymbolicExpression CreateTestDataFrame()
   {
      var dfFun = REngine.Evaluate("data.frame").AsFunction();
      var result = dfFun.InvokeNamed(
         tc("name", new[] { "a", "b", "c" }),
         tc("a", new[] { 1.0, 2.0, 3.0 })
      );
      return result;
   }
}