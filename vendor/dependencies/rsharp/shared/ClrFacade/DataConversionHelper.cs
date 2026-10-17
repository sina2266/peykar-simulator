using System;
using System.Linq;
using System.Runtime.InteropServices;
using RDotNet;
using RDotNet.Internals;

namespace ClrFacade;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate IntPtr ClrObjectToSexpDelegate(IntPtr variant);

public interface IUnmanagedDll
{
   ClrObjectToSexpDelegate ClrObjectToSexp { get; set; }
   IntPtr GetFunctionAddress(string entryPointName);
}

/// <summary>
/// A helper class to inspect data and determine what it is converted to in the unmanaged code.
/// </summary>
/// <remarks>
/// Acknowledgments go to Lim Bio Liong for some of this code. See http://limbioliong.wordpress.com/2011/09/04/using-variants-in-managed-code-part-1/ 
/// and  http://limbioliong.wordpress.com/2011/03/20/c-interop-how-to-return-a-variant-from-an-unmanaged-function/. 
/// Very useful and impressive series of articles.
/// </remarks>
public static class DataConversionHelper
{
   public static IUnmanagedDll rSharpNativeDll = null;
}

/// <summary>
/// A wrapper around a symbolic expression. This is necessary to wrap safehandle around.
/// </summary>
public class SymbolicExpressionWrapper
{
   private static int _counter = 0;

   public SymbolicExpressionWrapper(SymbolicExpression sexp)
   {
      Sexp = sexp;
      _counter++;
   }

   public SymbolicExpression Sexp { get; }

   ~SymbolicExpressionWrapper()
   {
      _counter--;
   }

   public object ToClrEquivalent()
   {
      return Sexp.Type switch
      {
         SymbolicExpressionType.CharacterVector => convertVector(Sexp.AsCharacter().ToArray()),
         SymbolicExpressionType.ComplexVector => convertVector(Sexp.AsComplex().ToArray()),
         SymbolicExpressionType.IntegerVector => convertVector(Sexp.AsInteger().ToArray()),
         SymbolicExpressionType.LogicalVector => convertVector(Sexp.AsLogical().ToArray()),
         SymbolicExpressionType.NumericVector => convertNumericVector(Sexp),
         SymbolicExpressionType.RawVector => convertVector(Sexp.AsRaw().ToArray()),
         // case SymbolicExpressionType.S4:
         //     {
         //         var s4sxp = Sexp.AsS4();
         //         if (!s4sxp.HasSlot("clrobj")) return Sexp;
         //         var fromIntPtr = GCHandle.FromIntPtr(s4sxp["clrobj"].DangerousGetHandle());
         //         return fromIntPtr.Target;
         //     }
         SymbolicExpressionType.List => convertVector(convertList(Sexp.AsList().ToArray())),
         SymbolicExpressionType.Null => null,
         _ => Sexp
      };
   }

   private static object convertNumericVector(SymbolicExpression sexp)
   {
      var values = sexp.AsNumeric().ToArray();
      var classNames = InternalRDotNetDataConverter.GetClassAttribute(sexp);
      if (classNames == null)
         return convertVector(values);

      if (classNames.Contains("Date"))
         return convertVector(rDateToDateTime(values));

      if (classNames.Contains("POSIXct"))
         return convertVector(rPOSIXctToDateTime(sexp, values));

      if (classNames.Contains("difftime"))
         return convertVector(rDiffTimeToTimespan(sexp, values));

      return convertVector(values);
   }

   private static readonly DateTime _rDateOrigin = new(1970, 1, 1);

   private static readonly string[] _timeDiffUnits = { "secs", "mins", "hours", "days", "weeks" };

   private static TimeSpan[] rDiffTimeToTimespan(SymbolicExpression sexp, double[] values)
   {
      var units = InternalRDotNetDataConverter.GetAttribute(sexp, "units")[0];
      if (!_timeDiffUnits.Contains(units)) throw new NotSupportedException("timeDiff units {0} are not supported");
      return units switch
      {
         "secs" => Array.ConvertAll(values, TimeSpan.FromSeconds),
         "mins" => Array.ConvertAll(values, TimeSpan.FromMinutes),
         "hours" => Array.ConvertAll(values, TimeSpan.FromHours),
         "days" => Array.ConvertAll(values, TimeSpan.FromDays),
         "weeks" => Array.ConvertAll(values, x => TimeSpan.FromDays(x * 7)),
         _ => throw new NotSupportedException()
      };
      // This should never be reached.
   }

   private static DateTime[] rPOSIXctToDateTime(SymbolicExpression sexp, double[] values)
   {
      var tz = InternalRDotNetDataConverter.GetTimeZoneAttribute(sexp);
      if (!isSupportedTimeZone(tz))
         throw new NotSupportedException("POSIXct conversion supported only for UTC or unspecified (local) time zone, not for " + tz);

      //number of seconds since 1970-01-01 UTC
      return Array.ConvertAll(values,
         v =>
         {
            var utc = isUtc(tz);
            return InternalRSharpFacade.ForceDateKind(_rDateOrigin + TimeSpan.FromTicks((long)(TimeSpan.TicksPerSecond * v)), utc);
         }
      );
   }

   private static bool isSupportedTimeZone(string tz)
   {
      return isUtc(tz) || string.IsNullOrEmpty(tz);
   }

   private static bool isUtc(string tz)
   {
      if (string.IsNullOrEmpty(tz))
         return false;
      var t = tz.ToUpper();
      return t is "UTC" or "GMT";
   }

   private static DateTime[] rDateToDateTime(double[] values)
   {
      //number of days since 1970-01-01
      return Array.ConvertAll(values, v => _rDateOrigin + TimeSpan.FromTicks((long)(TimeSpan.TicksPerDay * v)));
   }

   private object[] convertList(SymbolicExpression[] symbolicExpression)
   {
      // Fall back on R enable vecsxp in C layer;
      throw new NotSupportedException("Not supported; would need to be able to unpack e.g. S4 objects.");
   }

   private static object convertVector<T>(T[] p)
   {
      if (p.Length == 1)
         return p[0];
      else
         return p;
   }
}