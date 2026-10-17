using Xunit;

namespace RDotNet
{
   // Tests in this collection mutate process-wide state (the R_HOME environment variable, the
   // current directory, the embedded R engine), so they must never run in parallel with tests
   // from other collections, regardless of the runner configuration.
   [CollectionDefinition("R.NET unit tests", DisableParallelization = true)]
   public class RDotNetUnitTestsCollection
   {
   }
}
