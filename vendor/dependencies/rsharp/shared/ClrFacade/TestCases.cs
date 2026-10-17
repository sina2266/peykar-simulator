using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Numerics;
using System.Text;
using RDotNet;

namespace ClrFacade;

// Classes and methods in this file are used dynamically by their names. 
// Do not change the names or signatures without also updating usages in R
public class TestCases
{
   public static bool SingleObjectArgument(TestObject arg)
   {
      return arg != null && arg.GetType() == typeof(TestObject);
   }

   public static bool ArrayOfObjectsArgument(Object[] arg)
   {
      if (arg == null)
         return false;

      for (var i = 0; i < arg.Length; i++)
      {
         if (arg[i] == null || arg[i].GetType() != typeof(TestObject))
            return false;
      }

      return true;
   }

   public static bool GetTrue()
   {
      return true;
   }

   public static bool GetFalse()
   {
      return false;
   }

   public static bool IsTrue(bool arg)
   {
      return arg;
   }

   public static bool DateEquals(DateTime dt, string isoDateTime)
   {
      var res = DateEquals(dt, DateTime.Parse(isoDateTime));
      if (!res)
      {
         Console.WriteLine(isoDateTime);
      }

      return res;
   }

   public static bool UtcDateEquals(DateTime dt, string isoDateTime, string timeZoneId = "UTC")
   {
      if (dt.Kind != DateTimeKind.Utc)
      {
         Console.WriteLine("Tested date time is not of Kind 'Utc'");
         return false;
      }

      var expected = UtcDateForTimeZone(isoDateTime, timeZoneId);
      var res = DateEquals(dt, expected);
      if (!res)
      {
         Console.WriteLine(isoDateTime);
      }

      return res;
   }

   public static bool UtcDateEquals(DateTime[] dts, string[] isoDateTimes, string timeZoneId = "UTC")
   {
      if (dts.Length != isoDateTimes.Length)
         return false;

      for (int i = 0; i < dts.Length; i++)
      {
         if (!UtcDateEquals(dts[i], isoDateTimes[i], timeZoneId))
            return false;
      }

      return true;
   }

   public static bool DateEquals(DateTime dt1, DateTime dt2)
   {
      var res = dt1 == dt2;
      if (res)
         return res;

      Console.WriteLine("DateEqualsFails");
      Console.WriteLine(dt1.ToString());

      return false;
   }

   public static bool DateEquals(DateTime[] dt1, DateTime[] dt2)
   {
      if (dt1.Length != dt2.Length)
         return false;
      return !dt1.Where((t, i) => !t.Equals(dt2[i])).Any();
   }

   public static void SinkDateTime(DateTime ignored)
   {
      // do nothing
   }

   /// <summary>
   ///    A method to help test whether memory is not leaking in rSharp when passing objects.
   /// </summary>
   public static void SinkLargeObject(object obj)
   {
      int breakpoint = 1;
   }

   public static double[] CreateArrayMemFootprint(long memFp)
   {
      var arraySize = memFp / 8;
      return new double[arraySize];
   }

   public static int GetWorkingSetMemoryMegabytes()
   {
      return (int)(Process.GetCurrentProcess().WorkingSet64 / 1e6);
   }

   public static int GetPrivateMemoryMegabytes()
   {
      return (int)(Process.GetCurrentProcess().PrivateMemorySize64 / 1e6);
   }

   public static DateTime CreateDate(string isoDateTime, string datetimeKind = "")
   {
      var res = DateTime.Parse(isoDateTime);
      if (string.IsNullOrEmpty(datetimeKind))
         return res;

      if (Enum.TryParse(datetimeKind, out DateTimeKind dtk))
         res = new DateTime(res.Ticks, dtk);

      return res;
   }

   public static DateTime[] CreateDateArray(string isoDateTimeStart, int numDays)
   {
      var startDate = CreateDate(isoDateTimeStart);
      DateTime[] result = createDailySequence(numDays, startDate);
      return result;
   }

   public static DateTime UtcDateForTimeZone(string isoDateTime, string timeZoneId = "UTC")
   {
      // Using TimeZoneInfo on dates far back towards the start of the christian calendar era poses problem. Don't do it if TZ = UTC
      if (timeZoneId == "UTC")
         return CreateDate(isoDateTime, "Utc");

      TimeZoneInfo tz;
      tz = string.IsNullOrEmpty(timeZoneId) ? TimeZoneInfo.Local : TimeZoneInfo.FindSystemTimeZoneById(timeZoneId);
      var dateTime = CreateDate(isoDateTime);
      return TimeZoneInfo.ConvertTimeToUtc(dateTime, tz);
   }

   public static DateTime[] UtcDateForTimeZone(string[] isoDateTimes, string timeZoneId = "UTC")
   {
      return Array.ConvertAll(isoDateTimes, x => UtcDateForTimeZone(x, timeZoneId: timeZoneId));
   }

   public static DateTime TimeZoneToLocalDate(string isoDateTime, string timeZoneId = "UTC")
   {
      return UtcDateForTimeZone(isoDateTime, timeZoneId).ToLocalTime();
   }

   public static DateTime ConvertTime(string isoDateTime, string fromTimeZoneId = "UTC", string toTimeZoneId = "UTC")
   {
      return TimeZoneInfo.ConvertTime(CreateDate(isoDateTime),
         TimeZoneInfo.FindSystemTimeZoneById(fromTimeZoneId),
         TimeZoneInfo.FindSystemTimeZoneById(toTimeZoneId));
   }

   public static bool NumArrayEquals(double[] array)
   {
      var expected = CreateNumArray();
      return checkNumArrayEquals(array, expected);
   }

   public static bool NumArrayMissingValuesEquals(double[] array)
   {
      var expected = CreateNumArray();
      expected[2] = double.NaN;
      return checkNumArrayEquals(array, expected);
   }

   private static bool checkNumArrayEquals(double[] array, double[] expected)
   {
      if (expected.Length != array.Length) return false;
      for (var i = 0; i < expected.Length; i++)
      {
         if (double.IsNaN(array[i]) && !double.IsNaN(expected[i]))
            return false;
         if (!double.IsNaN(array[i]) && double.IsNaN(expected[i]))
            return false;
         if (Math.Abs(array[i] - expected[i]) > 1e-10)
            // there ARE rounding artifacts  even in an R statement such as 1:5*1.1
            return false;
      }

      return true;
   }

   public static double[] CreateNumArray()
   {
      return new[] { 1.1, 2.2, 3.3, 4.4, 5.5 };
   }

   public static double[] CreateNumArrayMissingVal()
   {
      var result = CreateNumArray();
      result[2] = double.NaN;
      return result;
   }

   private static DateTime[] createDailySequence(int numDays, DateTime startDate)
   {
      var result = new DateTime[numDays];
      for (var i = 0; i < result.Length; i++)
         result[i] = startDate.AddDays(i);
      return result;
   }

   public static DateTime[] CreateDateArraySeconds(string isoDateTimeStart, int seconds)
   {
      var startDate = CreateDate(isoDateTimeStart);
      var result = new DateTime[seconds];
      for (var i = 0; i < result.Length; i++)
         result[i] = startDate.AddSeconds(i);
      return result;
   }

   public static bool CheckIsDailySequence(DateTime[] values, string isoDateTimeStart, int numDays)
   {
      if (values.Length != numDays) return false;
      var startDate = CreateDate(isoDateTimeStart);
      var expected = createDailySequence(numDays, startDate);
      return !expected.Where((t, i) => values[i] != t).Any();
   }

   public static TimeSpan[] CreateTimeSpanArray(double secondsFirst, int bySecondIncrements)
   {
      var result = new TimeSpan[bySecondIncrements];
      for (var i = 0; i < result.Length; i++)
         result[i] = TimeSpan.FromSeconds(secondsFirst + i * bySecondIncrements);
      return result;
   }

   public static bool TimeSpanEquals(TimeSpan timeSpan, string timeSpanString)
   {
      var expected = TimeSpan.Parse(timeSpanString, CultureInfo.InvariantCulture);
      return timeSpan.Equals(expected);
   }

   public static int[] CreateIntArray(int n)
   {
      var result = new int[n];
      for (int i = 0; i < result.Length; i++)
         result[i] = i;
      return result;
   }

   public static float[] CreateFloatArray()
   {
      return Array.ConvertAll(CreateNumArray(), x => (float)x);
   }

   public static float[][] CreateJaggedFloatArray()
   {
      var result = new float[3][];
      for (var i = 0; i < result.Length; i++)
      {
         var a = result[i] = new float[5];
         for (var j = 0; j < a.Length; j++)
         {
            result[i][j] = 1.0f + a.Length * i + j;
         }
      }

      return result;
   }

   public static double[][] CreateJaggedDoubleArray()
   {
      return CreateJaggedFloatArray().ToDouble();
   }

   public static double[,] CreateRectDoubleArray()
   {
      return CreateJaggedFloatArray().ToDoubleRect();
   }

   public static float[,] CreateRectFloatArray()
   {
      return CreateJaggedFloatArray().ToFloatRect();
   }

   public static bool NumericMatrixEquals(double[,] matrix)
   {
      var expected = CreateJaggedFloatArray().ToDoubleRect();
      if (expected.GetLength(0) != matrix.GetLength(0))
      {
         Console.WriteLine("dim 0 different lengths");
         return false;
      }

      if (expected.GetLength(1) != matrix.GetLength(1))
      {
         Console.WriteLine("dim 1 different lengths");
         return false;
      }

      for (var i = 0; i < expected.GetLength(0); i++)
      {
         for (var j = 0; j < expected.GetLength(1); j++)
         {
            if (Math.Abs(matrix[i, j] - expected[i, j]) > 1e-10)
            {
               return false;
            }
         }
      }

      return true;
   }

   public static bool StringArrayEquals(string[] array)
   {
      var expected = CreateStringArray();
      return checkArrayElements(array, expected);
   }

   public static bool StringArrayMissingValuesEquals(string[] array)
   {
      var expected = CreateStringArray();
      expected[2] = null;
      return checkArrayElements(array, expected);
   }

   private static bool checkArrayElements(string[] array, string[] expected)
   {
      if (expected.Length != array.Length)
      {
         Console.WriteLine("array of strings: different lengths");
         return false;
      }

      for (var i = 0; i < expected.Length; i++)
      {
         if (array[i] != expected[i])
         {
            Console.WriteLine("Element index {0} is '{1}'", i, array[i]);
            return false;
         }
      }

      return true;
   }

   public static Dictionary<string, string> CreateStringDictionary()
   {
      return new Dictionary<string, string> { { "a", "A" }, { "b", "B" } };
   }

   public static Dictionary<string, object> CreateObjectDictionary()
   {
      return new Dictionary<string, object>
      {
         { "a", new ArbitraryObject("A") },
         { "b", "B" },
         { "c", 123 }
      };
   }

   public static Dictionary<string, double[]> CreateStringDoubleArrayDictionary()
   {
      return new Dictionary<string, double[]>
      {
         { "a", new[] { 1.0, 2.0, 3.0, 3.5, 4.3, 11 } },
         { "b", new[] { 1.0, 2.0, 3.0, 3.5, 4.3 } },
         { "c", new[] { 2.2, 3.3, 6.5 } }
      };
   }

   private static TestObject[] createArrayOfReferenceObjects()
   {
      return new[]
      {
         new TestObject(),
         new TestObject()
      };
   }

   private static TestObject[][] createJaggedArrayOfReferenceObjects()
   {
      return new[]
      {
         createArrayOfReferenceObjects(),
         createArrayOfReferenceObjects()
      };
   }

   #region helper functions for vignette

   private static Func<object>[] getComplexDataTestCases()
   {
      return new Func<object>[]
      {
         CreateStringDictionary,
         CreateStringDoubleArrayDictionary,
         CreateObjectDictionary,
         createArrayOfReferenceObjects,
         createJaggedArrayOfReferenceObjects
      };
   }

   public static int GetNumComplexDataCases()
   {
      return getComplexDataTestCases().Length;
   }

   public static object GetComplexDataCase(int index)
   {
      return getComplexDataTestCases()[index]();
   }

   public static string GetComplexDataTypeName(int index)
   {
      var obj = GetComplexDataCase(index);
      return obj.GetType().FullName;
   }

   #endregion

   public static Function GetRFunctionInvoke(string rFunctionName)
   {
      var engine = REngine.GetInstance();
      return engine.GetSymbol(rFunctionName).AsFunction();
   }

   public static void ThrowException(int stackDepth)
   {
      stackDepth = Math.Max(1, Math.Min(stackDepth, 100));
      if (stackDepth == 1)
         throw new Exception("An exception designed with a particular stack trace length");
      ThrowException(stackDepth - 1);
   }

   public static string GetExceptionMessage()
   {
      return InternalRSharpFacade.FormatExceptionInnermost(CreateInnerExceptions());
   }

   public static Exception CreateInnerExceptions()
   {
      Exception exception = null;
      for (var i = 2; i >= 0; i--)
         exception = CreateException(i, exception);
      return exception;
   }

   public static Exception CreateException(int depth, Exception innerException)
   {
      return new Exception("Depth: " + depth, innerException);
   }

   public static TestEnum GetTestEnum(string name)
   {
      Enum.TryParse<TestEnum>(name, out var result);
      return result;
   }

   private class ArbitraryObject
   {
      public string Value { get; private set; }

      public ArbitraryObject(string value)
      {
         Value = value;
      }
   }

   public static string[] CreateStringArray()
   {
      return new[] { "ab", "bc", "cd", "de", "ef", };
   }

   public static T[] CreateEmptyArray<T>()
   {
      return Array.Empty<T>();
   }

   public static double[] CreateEmptyArrayDouble()
   {
      return CreateEmptyArray<double>();
   }

   public static float[] CreateEmptyArrayFloat()
   {
      return CreateEmptyArray<float>();
   }

   public static long[] CreateEmptyArrayLong()
   {
      return CreateEmptyArray<long>();
   }

   public static int[] CreateEmptyArrayInt()
   {
      return CreateEmptyArray<int>();
   }

   public static bool[] CreateEmptyArrayBool()
   {
      return CreateEmptyArray<bool>();
   }

   public static string[] CreateEmptyArrayString()
   {
      return CreateEmptyArray<string>();
   }

   public static byte[] CreateEmptyArrayByte()
   {
      return CreateEmptyArray<byte>();
   }

   public static double CreateDouble()
   {
      return 123.0;
   }

   public static bool DoubleEquals(double value)
   {
      return (123.0 == value);
   }

   public static float CreateFloat()
   {
      return 123.0f;
   }

   public static long CreateLong()
   {
      return 123;
   }

   public static int CreateInt()
   {
      return 123;
   }

   public static bool IntEquals(int value)
   {
      return (123 == value);
   }

   public static string CreateString()
   {
      return "ab";
   }

   public static bool StringEquals(string value)
   {
      return "ab" == value;
   }

   public static Complex[] CreateComplex(double[] real, double[] imaginary)
   {
      if (real.Length != imaginary.Length)
         throw new ArgumentException("array of real and imaginary numbers not of the same length");
      var res = new Complex[real.Length];
      for (int i = 0; i < real.Length; i++)
      {
         res[i] = new Complex(real[i], imaginary[i]);
      }

      return res;
   }

   public static Complex CreateComplex(double real, double imaginary)
   {
      return new Complex(real, imaginary);
   }

   public static bool ComplexEquals(Complex complex, double real, double imaginary)
   {
      return complex.Real == real && complex.Imaginary == imaginary;
   }

   public static bool ComplexEquals(Complex[] complexArray, double[] real, double[] imaginary)
   {
      if (complexArray.Length != real.Length || complexArray.Length != imaginary.Length)
         return false;

      return !complexArray.Where((t, i) => !ComplexEquals(t, real[i], imaginary[i])).Any();
   }

   private class MemTestObject
   {
      public string Text { get; set; }
      public static int Counter = 0;

      public MemTestObject()
      {
         Counter++;
      }

      ~MemTestObject()
      {
         Counter--;
      }
   }

   public static int GetMemTestObjCounter()
   {
      return MemTestObject.Counter;
   }

   public static object CreateMemTestObj()
   {
      return new MemTestObject();
   }

   /// <summary>
   ///    This function takes time. Only call it when you need to.
   /// </summary>
   public static void CallGC()
   {
      GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
      GC.WaitForPendingFinalizers();
      GC.GetTotalMemory(true);
   }

   public static TestObject CreateTestObject()
   {
      return new TestObject();
   }

   public static DataFrame CreateTestDataFrame()
   {
      var e = REngine.GetInstance();

      var colNames = new List<string>();
      var columns = new List<IEnumerable>();
      for (var i = 0; i < 2; i++)
      {
         colNames.Add("column" + i);
         var values = new double[7];
         for (var j = 0; j < 7; j++)
         {
            values[j] = j + i * 0.1;
         }

         columns.Add(values);
      }

      return e.CreateDataFrame(columns.ToArray(), colNames.ToArray(), stringsAsFactors: false);
   }

   public static NumericVector CreateTestNumericVector()
   {
      var e = REngine.GetInstance();
      return e.CreateNumericVector(new[] { 1.0, 2, 3, 4, 5, 6 });
   }

   public static TestObjectGeneric<string> CreateTestObjectGenericInstance()
   {
      return new TestObjectGeneric<string>("Hi");
   }

   public static TestObjectGeneric<string>[] CreateTestArrayGenericObjects()
   {
      var result = new TestObjectGeneric<string>[3];
      for (var i = 0; i < result.Length; i++)
         result[i] = CreateTestObjectGenericInstance();
      return result;
   }

   public static ITestInterface[] CreateTestArrayInterface()
   {
      var result = new ITestInterface[3];
      for (var i = 0; i < result.Length; i++)
         result[i] = new ImplITestInterface();
      return result;
   }

   public static ITestGenericInterface<string>[] CreateTestArrayGenericInterface()
   {
      var result = new ITestGenericInterface<string>[3];
      for (var i = 0; i < result.Length; i++)
         result[i] = new ImplITestGenericInterface();
      return result;
   }
   
   public static object IsNull(object obj)
   {
      return obj == null; 
   }

   public static object IsNA(object obj)
   {
      return obj;
   }

   public static object IsNaN(object obj)
   {
      if (!(obj is double d))
         return false;

      return double.IsNaN(d);
   }
   
   public static object IsNullInArray(object[] obj)
   {
      object[] validationObjectArray = new object[] { (double)1, null, (double)3 };
      
      if (validationObjectArray.Length != obj.Length)
         return false;

      for (int i = 0; i < validationObjectArray.Length; i++)
      {
         if ((double?)validationObjectArray[i] != (double?)obj[i])
               return false;
      }

      return true;
   }

   public static object IsNaNInArray(double[] obj)
   {
      double[] validationObjectArray = new double[] { (double)1, double.NaN, (double)3 };
      
      if (validationObjectArray.Length != obj.Length)
         return false;

      for (int i = 0; i < validationObjectArray.Length; i++)
      {
         if (double.IsNaN(validationObjectArray[i]) && double.IsNaN(obj[i]))
            continue;

         if (validationObjectArray[i] != obj[i])
            return false;
      }

      return true;
   }

   public static object GetNull()
   {
      return null;
   }

   public static object GetNaN()
   {
      return double.NaN;
   }
   
   public static object[] GetNullArray()
   {
      return [(double)1, null, (double)3];
   }

   public static double[] GetNaNArray()
   {
      return [1, double.NaN, 3];
   }

}

public class TestObjectWithEnum
{
   public TestEnum EnumValue { get; set; } = TestEnum.C;
}

public interface ITestInterface
{
}

internal class ImplITestInterface : ITestInterface
{
}

public interface ITestGenericInterface<T>
{
   T Value { get; }
}

internal class ImplITestGenericInterface : ITestGenericInterface<string>
{
   public string Value { get; set; }
}

public enum TestEnum
{
   A,
   B,
   C
}

[Flags]
public enum TestFlagEnum
{
   None = 0,
   A = 1,
   B = 2,
   C = 4
}

public class TestObjectGeneric<T>
{
   public T Value { get; private set; }

   public TestObjectGeneric(T value)
   {
      Value = value;
   }
}

public class TestObject
{
   // Helps test constructors
   public TestObject()
   {
   }

   public TestObject(int f1)
   {
      FieldIntegerOne = f1;
   }

   public TestObject(int f1, int f2)
      : this(f1)
   {
      FieldIntegerTwo = f2;
   }

   public TestObject(double d1)
   {
      FieldDoubleOne = d1;
   }

   public TestObject(double d1, double d2)
      : this(d1)
   {
      FieldDoubleTwo = d2;
   }

   public TestObject(int f1, int f2, double d1, double d2)
   {
      FieldIntegerOne = f1;
      FieldIntegerTwo = f2;
      FieldDoubleOne = d1;
      FieldDoubleTwo = d2;
   }

   public int PublicInt;
#pragma warning disable 169
   private int PrivateInt;
#pragma warning restore 169
   public int GetPublicInt()
   {
      return PublicInt;
   }

   public int FieldIntegerOne;
   public int FieldIntegerTwo;

   public double FieldDoubleOne;
   public double FieldDoubleTwo;

   public int GetFieldIntegerOne()
   {
      return FieldIntegerOne;
   }

   public int GetFieldIntegerTwo()
   {
      return FieldIntegerTwo;
   }

   public int GetMethodWithParameters(int pOne, string pTwo)
   {
      return FieldIntegerOne;
   }

   public int PropertyIntegerOne { get; set; }
   public int PropertyIntegerTwo { get; set; }

   public static int StaticPublicInt;
#pragma warning disable 169
   private static int StaticPrivateInt;
#pragma warning restore 169
   public static int StaticGetPublicInt()
   {
      return StaticPublicInt;
   }

   public static int StaticFieldIntegerOne;
   public static int StaticFieldIntegerTwo;

   public static int StaticGetFieldIntegerOne()
   {
      return StaticFieldIntegerOne;
   }

   public static int StaticGetFieldIntegerTwo()
   {
      return StaticFieldIntegerTwo;
   }

   public static int StaticGetMethodWithParameters(int pOne, string pTwo)
   {
      return StaticFieldIntegerOne;
   }

   public static int StaticPropertyIntegerOne { get; set; }
   public static int StaticPropertyIntegerTwo { get; set; }


   public string TestParams(string a, string b, params int[] c)
   {
      var sb = new StringBuilder();
      sb.Append(a);
      sb.Append(b);
      foreach (var i in c)
         sb.Append(i.ToString());
      return sb.ToString();
   }

   public string TestDefaultValues(string a, int b, int c = 1, int d = 2)
   {
      var sb = new StringBuilder();
      sb.Append(a);
      sb.Append(b);
      sb.Append(c);
      sb.Append(d);
      return sb.ToString();
   }
}