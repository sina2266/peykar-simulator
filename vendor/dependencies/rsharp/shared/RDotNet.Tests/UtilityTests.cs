using System;
using System.IO;
using System.Numerics;
using System.Runtime.InteropServices;
using RDotNet.NativeLibrary;
using RDotNet.Utilities;
using Xunit;

namespace RDotNet
{
   [Collection("R.NET unit tests")]
   public class UtilityTests
   {
      private const string MacFrameworkLibR = "/Library/Frameworks/R.framework/Resources/lib/libR.dylib";

      [Fact]
      public void CanSerializeComplexValues()
      {
         var result = RTypesUtil.SerializeComplexToDouble(new[] { new Complex(1, 0), new Complex(0, 1), new Complex(1, 1) });

         Assert.Equal(result, (new[] { 1d, 0, 0, 1, 1, 1 }));
      }

      [SkippableFact]
      public void MacOSRLibraryFileNameComesFromRHome()
      {
         Skip.IfNot(IsMacOS());
         var rHome = CreateTemporaryDirectory();
         var libR = Path.Combine(rHome, "lib", "libR.dylib");
         Directory.CreateDirectory(Path.GetDirectoryName(libR));
         File.WriteAllText(libR, string.Empty);
         var previousRHome = Environment.GetEnvironmentVariable("R_HOME");
         try
         {
            Environment.SetEnvironmentVariable("R_HOME", rHome);
            Assert.Equal(libR, NativeUtility.GetRLibraryFileName());
         }
         finally
         {
            Environment.SetEnvironmentVariable("R_HOME", previousRHome);
            Directory.Delete(rHome, true);
         }
      }

      [SkippableFact]
      public void MacOSRLibraryFileNameResolvesARelativeRHomeToAnAbsolutePath()
      {
         Skip.IfNot(IsMacOS());
         var rHome = CreateTemporaryDirectory();
         var previousRHome = Environment.GetEnvironmentVariable("R_HOME");
         var previousDirectory = Directory.GetCurrentDirectory();
         try
         {
            Directory.SetCurrentDirectory(Path.GetDirectoryName(rHome));
            // Build the expected path from the current directory: SetCurrentDirectory resolves
            // symlinks (on macOS the temp path /var/... is really /private/var/...), and the
            // resolved path is what a relative R_HOME gets anchored to.
            var libR = Path.Combine(Directory.GetCurrentDirectory(), Path.GetFileName(rHome), "lib", "libR.dylib");
            Directory.CreateDirectory(Path.GetDirectoryName(libR));
            File.WriteAllText(libR, string.Empty);

            Environment.SetEnvironmentVariable("R_HOME", Path.GetFileName(rHome));

            Assert.Equal(libR, NativeUtility.GetRLibraryFileName());
         }
         finally
         {
            Directory.SetCurrentDirectory(previousDirectory);
            Environment.SetEnvironmentVariable("R_HOME", previousRHome);
            Directory.Delete(rHome, true);
         }
      }

      [SkippableFact]
      public void MacOSRLibraryFileNameFallsBackToFrameworkWithoutAnRHomeLibrary()
      {
         Skip.IfNot(IsMacOS());
         var rHome = CreateTemporaryDirectory();
         var previousRHome = Environment.GetEnvironmentVariable("R_HOME");
         try
         {
            Environment.SetEnvironmentVariable("R_HOME", rHome);

            Assert.Equal(MacFrameworkLibR, NativeUtility.GetRLibraryFileName());

            Environment.SetEnvironmentVariable("R_HOME", null);

            Assert.Equal(MacFrameworkLibR, NativeUtility.GetRLibraryFileName());
         }
         finally
         {
            Environment.SetEnvironmentVariable("R_HOME", previousRHome);
            Directory.Delete(rHome, true);
         }
      }

      private static bool IsMacOS()
      {
         return RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
      }

      private static string CreateTemporaryDirectory()
      {
         var path = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
         Directory.CreateDirectory(path);
         return path;
      }
   }
}