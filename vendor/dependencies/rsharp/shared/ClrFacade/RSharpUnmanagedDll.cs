using System;
using System.IO;
using DynamicInterop;

namespace ClrFacade;

public class RSharpUnmanagedDll : IUnmanagedDll
{
   public RSharpUnmanagedDll(string dllName)
   {
      if (!File.Exists(dllName))
      {
         throw new FileNotFoundException(dllName);
      }

      _dll = new UnmanagedDll(dllName);
      ClrObjectToSexp = _dll.GetFunction<ClrObjectToSexpDelegate>("rsharp_object_to_SEXP");
   }

   public ClrObjectToSexpDelegate ClrObjectToSexp { get; set; }

   private readonly UnmanagedDll _dll;

   public IntPtr GetFunctionAddress(string entryPointName)
   {
      return _dll.GetFunctionAddress(entryPointName);
   }
}