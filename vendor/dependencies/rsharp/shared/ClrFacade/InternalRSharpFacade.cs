using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace ClrFacade;

public static class InternalRSharpFacade
{
   public static int CallInstanceMethod(IntPtr obj, string methodName, IntPtr arguments, int numObjects, IntPtr returnValue)
   {
      try
      {
         var objectArguments = convertToObjectArguments(arguments, numObjects);

         var instPtr = Marshal.ReadIntPtr(obj, 0);
         var t = Marshal.PtrToStructure<RSharpGenericValue>(instPtr);
         var instance = convertRSharpParameters(new[] { t })[0];

         var result = CallInstanceMethod(instance, methodName, objectArguments);
         var tempRetVal = RSharpGenericValueExtensions.FromObject(result);
         Marshal.StructureToPtr(tempRetVal, returnValue, false);
      }
      catch (Exception ex)
      {
         return processExceptionForR(ex, returnValue);
      }
      return 1234;
   }

   private static object[] convertToObjectArguments(IntPtr arguments, int numObjects)
   {
      var tempArray = new RSharpGenericValue[numObjects];

      for (var i = 0; i < numObjects; ++i)
      {
         var structPtr = Marshal.ReadIntPtr(arguments, i * IntPtr.Size);
         tempArray[i] = Marshal.PtrToStructure<RSharpGenericValue>(structPtr);
      }

      var objectArguments = convertRSharpParameters(tempArray);
      objectArguments = convertSpecialObjects(objectArguments);
      return objectArguments;
   }

   internal static int CreateSexpWrapper(IntPtr sexp, IntPtr returnValue)
   {
      if (sexp == IntPtr.Zero)
         return -1;
      var result = new SymbolicExpressionWrapper(DataConverter.CreateSymbolicExpression(sexp));
      var tempRetVal = RSharpGenericValueExtensions.FromObject(result);

      Marshal.StructureToPtr(tempRetVal, returnValue, false);

      return 1;
   }

   public static int CallStaticMethod(string typename, string methodName, IntPtr arguments, int numObjects, IntPtr returnValue)
   {
      var objectArguments = convertToObjectArguments(arguments, numObjects);

      LastCallException = string.Empty;
      try
      {
         var result = CallStaticMethod(typename, methodName, objectArguments);

         var tempRetVal = RSharpGenericValueExtensions.FromObject(result);

         Marshal.StructureToPtr(tempRetVal, returnValue, false);
      }
      catch (Exception ex)
      {
         return processExceptionForR(ex, returnValue);
      }

      return 1234;
   }

   internal static object InternalCallStaticMethod(Type classType, string methodName, bool tryUseConverter, object[] arguments)
   {
      if (arguments.GetType() == typeof(string[]))
         arguments = [arguments];

      // In order to handle the R Date and POSIX time conversion, we have to standardize on UTC in the C layer. 
      // The CLR hosting API seems to only marshall to date-times to Unspecified (probably cannot do otherwise)
      // We need to make sure these are Utc DateTime at this point.
      arguments = convertSpecialObjects(arguments);
      var types = getTypes(arguments);
      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Static | BindingFlags.InvokeMethod;
      var method = findMethod(classType, methodName, bindingFlags, types);
      if (method != null)
         return invokeMethod(null, arguments, method, tryUseConverter);

      InternalReflectionHelper.ThrowMissingMethod(classType, methodName, "static", types);

      return null;
   }

   public static object CallStaticMethod(string typename, string methodName, params object[] objects)
   {
      return InternalCallStaticMethod(GetType(typename), methodName, true, objects);
   }

   public static Type GetType(string typename)
   {
      if (string.IsNullOrEmpty(typename))
         throw new ArgumentException("missing type specification");

      var t = Type.GetType(typename);
      if (t != null)
         return t;

      var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
      var typeComponents = typename.Split(',');
      if (typeComponents.Length > 1)
      {
         var aName = typeComponents[^1];
         var assembly = loadedAssemblies.FirstOrDefault((x => x.GetName().Name == aName));
         if (assembly == null)
         {
            Console.WriteLine($"Assembly not found: {aName}");
            return null;
         }

         t = assembly.GetType(typeComponents[0]);
      }
      else
      {
         // Then we only have something like "TheNamespace.TheShortTypeName", Need to parse all the assemblies.
         var tName = typeComponents[0];
         foreach (var item in loadedAssemblies)
         {
            var types = item.GetTypes();
            t = types.FirstOrDefault((x => x.FullName == tName));
            if (t != null)
               return t;
         }
      }

      if (t != null)
         return t;

      var msg = $"Type not found: {typename}";
      Console.WriteLine(msg);
      return null;
   }

   public static int CurrentObject(IntPtr returnValue)
   {
      try
      {
         var result = DataConverter?.CurrentObject;
         var tempRetVal = RSharpGenericValueExtensions.FromObject(result);
         Marshal.StructureToPtr(tempRetVal, returnValue, false);
      }
      catch (Exception ex)
      {
         return processExceptionForR(ex, returnValue);
      }
      return 1234;
   }

   public static int CreateInstance(string typename, IntPtr arguments, int numObjects, IntPtr returnValue)
   {
      try
      {
         object result = null;

         var objectArguments = convertToObjectArguments(arguments, numObjects);

         var t = GetType(typename);
         if (t != null)
            result = ((objectArguments == null || objectArguments.Length == 0)
               ? Activator.CreateInstance(t)
               : Activator.CreateInstance(t, objectArguments));
         else
            throw new ArgumentException($"Could not determine Type from string '{typename}'");
         
         var tempRetVal = RSharpGenericValueExtensions.FromObject(result);
         Marshal.StructureToPtr(tempRetVal, returnValue, false);
      }
      catch (Exception ex)
      {
         return processExceptionForR(ex, returnValue);
      }
      
      return 1234;
   }

   public static string GetObjectTypeName(object instance)
   {
      return instance.GetType().FullName;
   }

   public static IntPtr GetObjectTypeName(IntPtr obj)
   {
      var instPtr = Marshal.ReadIntPtr(obj, 0);
      var t = Marshal.PtrToStructure<RSharpGenericValue>(instPtr);
      var instance = convertRSharpParameters([t])[0];

      return Marshal.StringToHGlobalAnsi(GetObjectTypeName(instance));
   }

   public static int LoadFrom(string pathOrAssemblyName, IntPtr returnValue)
   {
      Assembly result;
      try
      {
         if (File.Exists(pathOrAssemblyName))
            result = Assembly.LoadFrom(pathOrAssemblyName);
         else if (isFullyQualifiedAssemblyName(pathOrAssemblyName))
            result = Assembly.Load(pathOrAssemblyName);
         else
            // the use of LoadWithPartialName is deprecated, but this is highly convenient for the end user until there is 
            // another safer and convenient alternative
#pragma warning disable 618, 612
            result = Assembly.LoadWithPartialName(pathOrAssemblyName);
#pragma warning restore 618, 612
      }
      catch (Exception ex)
      {
         return processExceptionForR(ex, returnValue);
      }

      if (result != null)
      {
         Marshal.StructureToPtr(RSharpGenericValueExtensions.FromObject(true), returnValue, false);
         return 1;
      }
      
      Marshal.StructureToPtr(RSharpGenericValueExtensions.FromObject($"Assembly was not loaded. Is the file name and path correct: {pathOrAssemblyName}"), returnValue, false);
      return -1;

   }

   public static void SetFieldOrProperty(object obj, string name, object value)
   {
      if (obj == null)
         throw new ArgumentNullException();

      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Instance;
      internalSetFieldOrProperty(obj.GetType(), name, bindingFlags, obj, value);
   }

   public static void SetFieldOrProperty(string typename, string name, object value)
   {
      var t = GetType(typename);
      if (t != null)
         SetFieldOrProperty(t, name, value);
      else
         throw new ArgumentException($"Type not found: {typename}");
   }

   public static object GetFieldOrProperty(string typename, string name)
   {
      var t = GetType(typename);
      return t == null ? throw new ArgumentException($"Type not found: {typename}") : getFieldOrPropertyType(t, name);
   }

   public static object GetFieldOrProperty(object obj, string name)
   {
      obj = convertSpecialObject(obj);
      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Instance;
      var t = obj.GetType();
      return internalGetFieldOrProperty(t, name, bindingFlags, obj);
   }

   public static object CallInstanceMethod(object instance, string methodName, params object[] arguments)
   {
      return InternalCallInstanceMethod(instance, methodName, true, arguments);
   }

   internal static object InternalCallInstanceMethod(object obj, string methodName, bool tryUseConverter, object[] arguments)
   {
      object result = null;

      LastCallException = string.Empty;

      arguments = convertSpecialObjects(arguments);

      var types = getTypes(arguments);
      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Instance | BindingFlags.InvokeMethod;
      var classType = obj.GetType();
      var method = findMethod(classType, methodName, bindingFlags, types);
      if (method != null)
         result = invokeMethod(obj, arguments, method, tryUseConverter);
      else
         InternalReflectionHelper.ThrowMissingMethod(classType, methodName, "instance", types);

      return result;
   }

   private static int processExceptionForR(Exception ex, IntPtr returnValue)
   {
      LastCallException = FormatExceptionInnermost(ex);
      LastException = LastCallException;

      var tempRetVal = RSharpGenericValueExtensions.FromObject(LastCallException);
      Marshal.StructureToPtr(tempRetVal, returnValue, false);

      return -1;
   }

   private static string checkSehExceptionAdditionalErrorMessage(Exception ex)
   {
      if (ex is SEHException)
      {
         return ErrorMessageProvider == null ? "Caught an SEHException, but no additional information is available via ErrorMessageProvider" : ErrorMessageProvider();
      }

      return null;
   }

   public static string FormatExceptionInnermost(Exception ex)
   {
      var innermost = ex;
      while (innermost.InnerException != null)
         innermost = innermost.InnerException;

      var additionalMsg = checkSehExceptionAdditionalErrorMessage(innermost);

      var reflectionTypeLoadException = innermost as ReflectionTypeLoadException;
      var sb = new StringBuilder();
      sb.Append(!string.IsNullOrEmpty(additionalMsg) ? formatCustomMessage(additionalMsg) : formatExceptionMessage(innermost));

      if (reflectionTypeLoadException == null)
         return sb.ToString();

      foreach (var e in reflectionTypeLoadException.LoaderExceptions)
         sb.Append(formatExceptionMessage(e));

      return sb.ToString();
   }

   private static string formatCustomMessage(string additionalMsg)
   {
      var result = $"External Error Message: {additionalMsg}\n";
      return toUnixNewline(result);
   }

   private static string toUnixNewline(string result)
   {
      return result.Replace("\r\n", "\n");
   }

   private static string formatExceptionMessage(Exception ex)
   {
      // Note that if using Environment.NewLine below instead of "\n", the R GUI prompt is losing it
      // Actually even with the latter it is, but less so. Annoying.
      var result = $"Type:    {ex.GetType()}\nMessage: {ex.Message}\nMethod:  {ex.TargetSite}\nStack trace:\n{ex.StackTrace}\n\n";
      // See whether this helps with the R GUI prompt:
      return toUnixNewline(result);
   }

   /// <summary>
   ///    Gets/sets a data converter to customize or extend the marshalling of data between R and the CLR
   /// </summary>
   public static IDataConverter DataConverter { get; set; }

   /// <summary>
   ///    Gets/sets a function delegate that can provide additional error message information.
   /// </summary>
   /// <remarks>
   ///    This is intended to cater for cases where exception caught is SEHException,
   ///    presumably because some native code called by .NET via P/Invoke failed.
   ///    The native code may provide a way to retrieve information; this property offers a path to retrieve this information.
   /// </remarks>
   public static Func<string> ErrorMessageProvider { get; set; }

   /// <summary>
   ///    Gets if there is a custom data converter set on this facade
   /// </summary>
   public static bool DataConverterIsSet => DataConverter != null;

   private static bool isFullyQualifiedAssemblyName(string p)
   {
      return p.Contains("PublicKeyToken=");
   }

   [Obsolete("This may be superseded as this was swamping user with too long a list", false)]
   public static string[] GetMembers(object obj)
   {
      var members = new List<MemberInfo>();
      var classType = obj.GetType();
      members.AddRange(obj.GetType().GetMembers());
      var ifTypes = classType.GetInterfaces();
      foreach (var t in ifTypes)
      {
         members.AddRange(t.GetMembers());
      }

      var result = Array.ConvertAll(members.ToArray(), (x => x.Name));
      result = result.Distinct().ToArray();
      Array.Sort(result);
      return result;
   }

   /// <summary>
   ///    Returns a string that represents the parameter passed.
   /// </summary>
   /// <remarks>This is useful e.g. to quickly check from R the CLR equivalent of an R POSIX time object</remarks>
   public static string ToString(object obj)
   {
      return (obj == null) ? "null" : obj.ToString();
   }

   public static void SetFieldOrProperty(Type type, string name, object value)
   {
      if (type == null)
         throw new ArgumentNullException();

      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Static;
      internalSetFieldOrProperty(type, name, bindingFlags, null, value);
   }

   private static void internalSetFieldOrProperty(Type t, string name, BindingFlags b, object objOrNull, object value)
   {
      var field = t.GetField(name, b);
      if (field == null)
      {
         var property = t.GetProperty(name, b);
         if (property == null)
            throw new ArgumentException($"Public instance field or property name {name} not found");

         property.SetValue(objOrNull, value, null);
      }
      else
         field.SetValue(objOrNull, value);
   }

   private static object getFieldOrPropertyType(Type type, string name)
   {
      const BindingFlags bindingFlags = BindingFlags.Public | BindingFlags.Static;
      return internalGetFieldOrProperty(type, name, bindingFlags, null);
   }

   static object internalGetFieldOrProperty(Type t, string name, BindingFlags b, object objOrNull)
   {
      var field = t.GetField(name, b);
      if (field != null)
         return field.GetValue(objOrNull);

      var property = t.GetProperty(name, b);
      if (property == null)
         throw new ArgumentException($"Field or property name '{name}' not found on object of type '{t.Name}', for binding flags '{b.ToString()}'");

      var getFieldOrProperty = property.GetValue(objOrNull, null);

      if(getFieldOrProperty is Enum enumeration)
         return Convert.ToInt32(enumeration);

      return getFieldOrProperty;
   }

   /// <summary>
   ///    A default binder for finding methods; a placeholder for a way to customize or refine the method selection process
   ///    for rSharp.
   /// </summary>
   private static readonly Binder _methodBinder = Type.DefaultBinder; // reverting; this causes problems for parameters with the params keyword

   private static MethodInfo findMethod(Type classType, string methodName, BindingFlags bf, Type[] types)
   {
      return InternalReflectionHelper.GetMethod(classType, methodName, _methodBinder, bf, types);
   }

   private static object invokeMethod(object obj, object[] arguments, MethodInfo method, bool tryUseConverter)
   {
      var parameters = method.GetParameters();
      var numberOfParameters = parameters.Length;
      if (numberOfParameters > arguments.Length)
      {
         var newArgs = new object[numberOfParameters];
         arguments.CopyTo(newArgs, 0);
         if (numberOfParameters == (arguments.Length + 1))
         {
            var lastParamInfo = parameters[^1];
            if (InternalReflectionHelper.IsVarArg(lastParamInfo))
               newArgs[numberOfParameters - 1] = Array.CreateInstance(lastParamInfo.ParameterType.GetElementType(), 0);
         }
         else
         {
            // Assume this is because of parameters with default values, and handle as per:
            // http://msdn.microsoft.com/en-us/library/x0acewhc.aspx
            for (var i = arguments.Length; i < newArgs.Length; i++)
               newArgs[i] = Type.Missing;
         }

         arguments = newArgs;
      }
      else if (parameters.Length > 0)
      {
         // check whether we have a method with the last argument with a 'params' keyword
         // This is not handled magically when using reflection.
         var p = parameters[^1];
         if (InternalReflectionHelper.IsVarArg(p))
            arguments = packParameters(arguments, numberOfParameters, p);
      }

      return marshallDataToR(method.Invoke(obj, arguments), tryUseConverter);
   }

   private static object[] packParameters(object[] arguments, int np, ParameterInfo p)
   {
      var arrayType = p.ParameterType;
      if (np < 1)
         throw new ArgumentException("numParameters must be strictly positive");
      if (!arrayType.IsArray)
         throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");
      return PackParameters(arguments, np, arrayType);
   }

   public static object[] PackParameters(object[] arguments, int np, Type arrayType)
   {
      var na = arguments.Length;
      var tElement = arrayType.GetElementType(); // Int32 for an array int[]
      var result = new object[np];
      Array.Copy(arguments, result, np - 1); // obj, string

      if (np == na && arrayType == arguments[na - 1].GetType())
      {
         // we already have an int[] pre-packed. 
         // {obj, "methodName", new int[]{p1, p2, p3})  length 3
         // NOTE Possible singular and ambiguous cases: params object[] or params Array[]
         Array.Copy(arguments, na - 1, result, na - 1, 1);
      }
      else
      {
         // {obj, "methodName", p1, p2, p3)  length 5
         Array paramParam = Array.CreateInstance(tElement, na - np + 1); // na - np + 1 = 5 - 3 + 1 = 3
         Array.Copy(arguments, np - 1, paramParam, 0, na - np + 1); // np - 1 = 3 - 1 = 2 start index
         result.SetValue(paramParam, np - 1);
      }

      return result;
   }

   private static object marshallDataToR(object obj, bool tryUseConverter)
   {
      var result = tryUseConverter && DataConverter != null ? DataConverter.ConvertToR(obj) : obj;

      if (result is SafeHandle)
         throw new NotSupportedException($"Object '{result.GetType()}' is a SafeHandle and cannot be returned to native R");

      return result;
   }

   private static object[] convertRSharpParameters(RSharpGenericValue[] arguments)
   {
      var resultObjectList = new object[arguments.Length];

      for (var i = 0; i < arguments.Length; i++)
      {
         resultObjectList[i] = ConvertRSharpGenericValue(arguments[i]);
      }

      return resultObjectList;
   }

   public static object ConvertRSharpGenericValue(RSharpGenericValue argument)
   {
      switch (argument.Type)
      {
         case RSharpValueType.Double:
            return BitConverter.ToDouble(BitConverter.GetBytes(Marshal.ReadInt64(argument.Value)), 0);
         case RSharpValueType.Bool:
            return BitConverter.ToBoolean(BitConverter.GetBytes(Marshal.ReadInt32(argument.Value)), 0);
         case RSharpValueType.Int:
            return Marshal.ReadInt32(argument.Value);
         case RSharpValueType.Float:
            return BitConverter.ToSingle(BitConverter.GetBytes(Marshal.ReadInt32(argument.Value)), 0);
         case RSharpValueType.String:
            return Marshal.PtrToStringAnsi(argument.Value);
         case RSharpValueType.IntArray:
            return argument.GetArray<int>();
         case RSharpValueType.FloatArray:
            return argument.GetArray<float>();
         case RSharpValueType.StringArray:
            return argument.GetArray<string>();
         case RSharpValueType.ObjectArray:
            return convertToObjectArguments(argument.Value, argument.Size);
         case RSharpValueType.Object:
            return argument.GetObject();
         case RSharpValueType.DoubleArray:
         case RSharpValueType.BoolArray:
         case RSharpValueType.Null:
         case RSharpValueType.Intptr:
         default:
            //ToDo: WE SHOULD BE HANDLING THE EXCEPTION HERE BETTER
            Console.WriteLine($"Unknown value type: {argument.Type}");
            return null;
      }
   }

   private static object[] convertSpecialObjects(object[] arguments)
   {
      if (DataConverterIsSet)
         arguments = DataConverter.ConvertSymbolicExpressions(arguments);
      arguments = makeDatesUtcKind(arguments);

      return arguments;
   }

   private static object[] makeDatesUtcKind(object[] arguments)
   {
      var newArgs = (object[])arguments.Clone();
      for (var i = 0; i < arguments.Length; i++)
      {
         var obj = arguments[i];
         if (obj is DateTime time)
            newArgs[i] = ForceUtcKind(time);
         else if (obj is DateTime[] times)
            newArgs[i] = forceUtcKind(times);
      }

      return newArgs;
   }

   private static DateTime[] forceUtcKind(DateTime[] dateTimes)
   {
      var result = new DateTime[dateTimes.Length];
      for (var i = 0; i < result.Length; i++)
      {
         result[i] = ForceUtcKind(dateTimes[i]);
      }

      return result;
   }

   public static DateTime ForceUtcKind(DateTime dateTime)
   {
      return ForceDateKind(dateTime, utc: true);
   }

   private static object convertSpecialObject(object obj)
   {
      if (DataConverterIsSet)
         obj = DataConverter.ConvertSymbolicExpression(obj);
      if (obj is DateTime time)
         obj = ForceUtcKind(time);
      return obj;
   }

   private static Type[] getTypes(object[] arguments)
   {
      var result = new Type[arguments.Length];
      for (var i = 0; i < arguments.Length; i++)
      {
         result[i] = (arguments[i] == null ? typeof(object) : arguments[i].GetType());
      }

      return result;
   }

   // Work around https://r2clr.codeplex.com/workitem/67

   /// <summary>
   ///    A transient property with the printable format of the innermost exception to the latest clrCall[...] call.
   /// </summary>
   public static string LastCallException { get; private set; }

   /// <summary>
   ///    A property with the printable format of the innermost exception to the last failed clrCall[...] call.
   /// </summary>
   public static string LastException { get; private set; }

   public static DateTime ForceDateKind(DateTime dateTime, bool utc = false)
   {
      return new DateTime(dateTime.Ticks, (utc ? DateTimeKind.Utc : DateTimeKind.Unspecified));
   }

   private static readonly DateTime _rDateOrigin = new(1970, 1, 1);

   private static TimeSpan getUtcTimeSpanROrigin(ref DateTime date)
   {
      date = date.ToUniversalTime();
      var timeSpan = date - _rDateOrigin;
      return timeSpan;
   }

   public static double GetRPosixCtDoubleRepresentation(DateTime date)
   {
      var timeSpan = getUtcTimeSpanROrigin(ref date);
      var res = timeSpan.TotalSeconds;
      return res;
   }

   public static void FreeObject(IntPtr instPtr)
   {
      var genericValue = Marshal.PtrToStructure<RSharpGenericValue>(instPtr);
      switch (genericValue.Type)
      {
         case RSharpValueType.Null:
         case RSharpValueType.Intptr:
            // Null carries IntPtr.Zero; Intptr carries a caller-owned pointer. Nothing to free.
            break;
         case RSharpValueType.String:
            // FromObject allocates via Marshal.StringToHGlobalAnsi, so pair with FreeHGlobal.
            Marshal.FreeHGlobal(genericValue.Value);
            break;
         default:
            // Every other case in FromObject (primitives, arrays, Object) ends in GCHandle.Alloc.
            ((GCHandle)genericValue.Value).Free();
            break;
      }
   }
}