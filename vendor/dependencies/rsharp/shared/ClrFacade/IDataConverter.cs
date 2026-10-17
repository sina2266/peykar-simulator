using System;
using RDotNet;

namespace ClrFacade
{
   /// <summary>
   ///    Interface for objects that are converting CLR objects to a representation in R
   /// </summary>
   /// <remarks>
   ///    Currently the only concrete implementation is a data converter that uses RDotNet
   ///    to expose CLR objects to R
   /// </remarks>
   public interface IDataConverter
   {
      object ConvertToR(object obj);

      /// <summary>
      ///    Return a reference to the object currently handled by the custom data converter, if any is in use.
      /// </summary>
      object CurrentObject { get; }
      
      SymbolicExpression CreateSymbolicExpression(IntPtr sexp);

      object[] ConvertSymbolicExpressions(object[] arguments);

      object ConvertSymbolicExpression(object obj);
   }
}