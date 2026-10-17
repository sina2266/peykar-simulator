using System;
using System.Runtime.InteropServices;

namespace ClrFacade;

public enum RSharpValueType
{
   Int,
   Float,
   Double,
   Bool,
   IntArray,
   FloatArray,
   DoubleArray,
   BoolArray,
   String,
   StringArray,
   Object,
   Null,
   Intptr,
   ObjectArray
}

public static class RSharpGenericValueExtensions
{
   public static T[] GetArray<T>(this RSharpGenericValue genericValue)
   {
      if (genericValue.Type == RSharpValueType.IntArray && typeof(T) == typeof(int))
      {
         var array = new int[genericValue.Size];
         Marshal.Copy(genericValue.Value, array, 0, genericValue.Size);
         return array as T[];
      }

      if (genericValue.Type == RSharpValueType.FloatArray && typeof(T) == typeof(float))
      {
         var array = new float[genericValue.Size];
         Marshal.Copy(genericValue.Value, array, 0, genericValue.Size);
         return array as T[];
      }

      if (genericValue.Type == RSharpValueType.DoubleArray && typeof(T) == typeof(double))
      {
         var array = new double[genericValue.Size];
         Marshal.Copy(genericValue.Value, array, 0, genericValue.Size);
         return array as T[];
      }

      if (genericValue.Type == RSharpValueType.BoolArray && typeof(T) == typeof(bool))
      {
         var array = new bool[genericValue.Size];
         for (int i = 0; i < genericValue.Size; i++)
         {
            array[i] = Marshal.ReadByte(genericValue.Value, i) != 0;
         }

         return array as T[];
      }

      throw new InvalidOperationException("Invalid array type");
   }

   // Factory method to create RSharpGenericValue from an object
   public static RSharpGenericValue FromObject(object obj)
   {
      if (obj == null)
         return new RSharpGenericValue { Type = RSharpValueType.Null, Value = IntPtr.Zero };
      // Determine the ValueType based on the actual type of the object
      RSharpValueType type;
      switch (obj)
      {
         case int:
            type = RSharpValueType.Int;
            break;
         case IntPtr ptr:
            return new RSharpGenericValue
            {
               Value = ptr,
               Type = RSharpValueType.Intptr
            };
         case float:
            type = RSharpValueType.Float;
            break;
         case double:
            type = RSharpValueType.Double;
            break;
         case bool:
            type = RSharpValueType.Bool;
            break;
         case string objString:
         {
            type = RSharpValueType.String;

            var tempRes = new RSharpGenericValue
            {
               Value = Marshal.StringToHGlobalAnsi(objString),
               Type = type
            };

            return tempRes;
         }
         case Array arrayObj:
         {
            Type elementType = arrayObj.GetType().GetElementType();
            if (elementType == typeof(int))
            {
               type = RSharpValueType.IntArray;
            }
            else if (elementType == typeof(float))
            {
               type = RSharpValueType.FloatArray;
            }
            else if (elementType == typeof(double))
            {
               type = RSharpValueType.DoubleArray;
            }
            else if (elementType == typeof(bool))
            {
               type = RSharpValueType.BoolArray;
            }
            else if (elementType == typeof(string))
            {
               type = RSharpValueType.StringArray;
            }
            else //keeping it like that for now - not sure we should be supporting arrays of objects
            {
               throw new ArgumentException($"Unsupported array element type: {elementType}");
            }

            break;
         }
         // non of the above, so we treat it as a generic object
         default:
            type = RSharpValueType.Object;
            break;
      }

      // Convert the object to IntPtr
      var handle = GCHandle.Alloc(obj, GCHandleType.Normal);
      var valuePtr = (IntPtr)handle;

      var result = new RSharpGenericValue
      {
         Value = valuePtr,
         Type = type
      };

      return result;
   }

   public static object GetObject(this RSharpGenericValue genericValue)
   {
      if (genericValue.Type == RSharpValueType.Object)
      {
         GCHandle handle = GCHandle.FromIntPtr(genericValue.Value);
         return handle.Target;
      }

      throw new InvalidOperationException("Invalid object type");
   }
}

[StructLayout(LayoutKind.Sequential)]
public struct RSharpGenericValue
{
   public RSharpValueType Type { get; set; }
   public IntPtr Value { get; set; }
   public int Size { get; set; }
}