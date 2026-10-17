using ClrFacade;
using RDotNet;
using Xunit;

namespace rSharpTests
{
   public class RdotnetTests
   {
      [Fact]
      public void test_sexp_wrapper()
      {
         var e = REngine.GetInstance();
         var pi = e.CreateNumeric(3.1415);
         var symbolicExpressionWrapper = new SymbolicExpressionWrapper(pi);
      }
   }
}