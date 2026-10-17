using System;

namespace ClrFacade;

public class TestArrayMemoryHandling
{
   public object[] FieldArray_object;

   public static object[] CreateArray_object(int size)
   {
      return new object[size];
   }

   public static object[] CreateArray_object(int size, object value)
   {
      var result = new object[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public string[] FieldArray_string;

   public static string[] CreateArray_string(int size)
   {
      return new string[size];
   }

   public static string[] CreateArray_string(int size, string value)
   {
      var result = new string[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public double[] FieldArray_double;

   public static double[] CreateArray_double(int size)
   {
      return new double[size];
   }

   public static double[] CreateArray_double(int size, double value)
   {
      var result = new double[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public float[] FieldArray_float;

   public static float[] CreateArray_float(int size)
   {
      return new float[size];
   }

   public static float[] CreateArray_float(int size, float value)
   {
      var result = new float[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public int[] FieldArray_int;

   public static int[] CreateArray_int(int size)
   {
      return new int[size];
   }

   public static int[] CreateArray_int(int size, int value)
   {
      var result = new int[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public long[] FieldArray_long;

   public static long[] CreateArray_long(int size)
   {
      return new long[size];
   }

   public static long[] CreateArray_long(int size, long value)
   {
      var result = new long[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public bool[] FieldArray_bool;

   public static bool[] CreateArray_bool(int size)
   {
      return new bool[size];
   }

   public static bool[] CreateArray_bool(int size, bool value)
   {
      var result = new bool[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public DateTime[] FieldArray_DateTime;

   public static DateTime[] CreateArray_DateTime(int size)
   {
      return new DateTime[size];
   }

   public static DateTime[] CreateArray_DateTime(int size, DateTime value)
   {
      var result = new DateTime[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public TimeSpan[] FieldArray_TimeSpan;

   public static TimeSpan[] CreateArray_TimeSpan(int size)
   {
      return new TimeSpan[size];
   }

   public static TimeSpan[] CreateArray_TimeSpan(int size, TimeSpan value)
   {
      var result = new TimeSpan[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public byte[] FieldArray_byte;

   public static byte[] CreateArray_byte(int size)
   {
      return new byte[size];
   }

   public static byte[] CreateArray_byte(int size, byte value)
   {
      var result = new byte[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public char[] FieldArray_char;

   public static char[] CreateArray_char(int size)
   {
      return new char[size];
   }

   public static char[] CreateArray_char(int size, char value)
   {
      var result = new char[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   public Type[] FieldArray_Type;

   public static Type[] CreateArray_Type(int size)
   {
      return new Type[size];
   }

   public static Type[] CreateArray_Type(int size, Type value)
   {
      var result = new Type[size];
      for (var i = 0; i < result.Length; i++) result[i] = value;
      return result;
   }

   // To test the type of empty vectors:
   public static bool CheckElementType(Array array, Type expectedElementType)
   {
      return array.GetType().GetElementType() == expectedElementType;
   }
}