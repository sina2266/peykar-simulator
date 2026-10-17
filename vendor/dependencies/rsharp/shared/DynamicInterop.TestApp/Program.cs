using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DynamicInterop.TestApp
{
   class MainClass
   {
      public delegate IntPtr dlopen([MarshalAs(UnmanagedType.LPStr)] string filename, int flag);

      [return: MarshalAs(UnmanagedType.LPStr)]
      private delegate string dlerror();

      public static void Main(string[] args)
      {
         var nat = new UnmanagedDll(args[0]);
         Console.WriteLine("Looks good");
         // lowLevelTest (args);
      }

      static void lowLevelTest(IReadOnlyList<string> args)
      {
         var nat = new UnmanagedDll("libdl");
         var open = nat.GetFunction<dlopen>("dlopen");
         var error = nat.GetFunction<dlerror>("dlerror");
         var handle = open(args[0], 0x01);
         if (IntPtr.Zero == handle)
         {
            Console.WriteLine("failed!!");
            var msg = error();
            if (!string.IsNullOrEmpty(msg))
               Console.WriteLine(msg);
         }
         else
         {
            Console.WriteLine("Success!!");
         }
      }
   }
}