using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Runtime.InteropServices;
using RDotNet;
using RDotNet.NativeLibrary;

namespace ClrFacade;

internal class InternalRDotNetDataConverter : IDataConverter
{
   private InternalRDotNetDataConverter(string pathToNativeSharedObj)
   {
      var dllName = pathToNativeSharedObj;
      // HACK - this feels wrong, at least not clean. All I have time for.
      if (string.IsNullOrEmpty(dllName))
      {
         var assemblyPath = Assembly.GetAssembly(GetType()).Location;
         assemblyPath = Path.GetFullPath(assemblyPath);
         var libDir = Path.GetDirectoryName(assemblyPath);

         CultureInfo.DefaultThreadCurrentCulture = new CultureInfo("en-US");
         CultureInfo.DefaultThreadCurrentUICulture = new CultureInfo("en-US");
         CultureInfo.CurrentUICulture = new CultureInfo("en-US");
         
         if(NativeUtility.IsLinux)
            dllName = Path.Combine(libDir, "rSharp.linux.so");
         else if(NativeUtility.IsMac && NativeUtility.Architecture == Architecture.X64)
            dllName = Path.Combine(libDir, "rSharp.mac.x64.so");
         else if(NativeUtility.IsMac && NativeUtility.Architecture == Architecture.Arm64)
            dllName = Path.Combine(libDir, "rSharp.mac.arm64.so");
         else
            dllName = Path.Combine(libDir, "rSharp.dll");
         
      }

      DataConversionHelper.rSharpNativeDll = new RSharpUnmanagedDll(dllName);

      setupREngine();
      _converterFunctions = new Dictionary<Type, Func<object, SymbolicExpression>>();

      ConvertVectors = true;
      ConvertValueTypes = true;

      addBijectiveConversions();
      addMultidimensionalArrays();

      ConvertAdvancedTypes = true;
   }

   private void addGeneralTypes()
   {
      // Add some default converters for more general types
      if (_converterFunctions.ContainsKey(typeof(Array))) return;
      _converterFunctions.Add(typeof(Array), convertArrayObject);
      _converterFunctions.Add(typeof(object), convertObject);
   }

   private void removeGeneralTypes()
   {
      if (!_converterFunctions.ContainsKey(typeof(Array))) return;
      _converterFunctions.Remove(typeof(Array));
      _converterFunctions.Remove(typeof(object));
   }

   private void addDictionaries()
   {
      if (_converterFunctions.ContainsKey(typeof(Dictionary<string, double>))) return;
      _converterFunctions.Add(typeof(Dictionary<string, double>), convertDictionary<double>);
      _converterFunctions.Add(typeof(Dictionary<string, float>), convertDictionary<float>);
      _converterFunctions.Add(typeof(Dictionary<string, string>), convertDictionary<string>);
      _converterFunctions.Add(typeof(Dictionary<string, int>), convertDictionary<int>);
      _converterFunctions.Add(typeof(Dictionary<string, DateTime>), convertDictionary<DateTime>);

      _converterFunctions.Add(typeof(Dictionary<string, double[]>), convertDictionary<double[]>);
      _converterFunctions.Add(typeof(Dictionary<string, float[]>), convertDictionary<float[]>);
      _converterFunctions.Add(typeof(Dictionary<string, string[]>), convertDictionary<string[]>);
      _converterFunctions.Add(typeof(Dictionary<string, int[]>), convertDictionary<int[]>);
      _converterFunctions.Add(typeof(Dictionary<string, DateTime[]>), convertDictionary<DateTime[]>);
   }

   private void removeDictionaries()
   {
      if (!_converterFunctions.ContainsKey(typeof(Dictionary<string, double>))) return;
      _converterFunctions.Remove(typeof(Dictionary<string, double>));
      _converterFunctions.Remove(typeof(Dictionary<string, float>));
      _converterFunctions.Remove(typeof(Dictionary<string, string>));
      _converterFunctions.Remove(typeof(Dictionary<string, int>));
      _converterFunctions.Remove(typeof(Dictionary<string, DateTime>));

      _converterFunctions.Remove(typeof(Dictionary<string, double[]>));
      _converterFunctions.Remove(typeof(Dictionary<string, float[]>));
      _converterFunctions.Remove(typeof(Dictionary<string, string[]>));
      _converterFunctions.Remove(typeof(Dictionary<string, int[]>));
      _converterFunctions.Remove(typeof(Dictionary<string, DateTime[]>));
   }

   private void addMultidimensionalArrays()
   {
      if (_converterFunctions.ContainsKey(typeof(float[,]))) return;
      _converterFunctions.Add(typeof(float[,]), convertMatrixSingle);
      _converterFunctions.Add(typeof(double[,]), convertMatrixDouble);
      _converterFunctions.Add(typeof(int[,]), convertMatrixInt);
      _converterFunctions.Add(typeof(string[,]), convertMatrixString);

      _converterFunctions.Add(typeof(float[][]), convertMatrixJaggedSingle);
      _converterFunctions.Add(typeof(double[][]), convertMatrixJaggedDouble);
      _converterFunctions.Add(typeof(int[][]), convertMatrixJaggedInt);
      _converterFunctions.Add(typeof(string[][]), convertMatrixJaggedString);
   }

   private void addBijectiveConversions()
   {
      _converterFunctions.Add(typeof(float), convertSingle);
      _converterFunctions.Add(typeof(double), convertDouble);
      _converterFunctions.Add(typeof(byte), convertByte);
      _converterFunctions.Add(typeof(char), convertChar);
      _converterFunctions.Add(typeof(bool), convertBool);
      _converterFunctions.Add(typeof(int), convertInt);
      _converterFunctions.Add(typeof(string), convertString);
      _converterFunctions.Add(typeof(DateTime), convertDateTime);
      _converterFunctions.Add(typeof(TimeSpan), convertTimeSpan);
      _converterFunctions.Add(typeof(Complex), convertComplex);

      _converterFunctions.Add(typeof(float[]), convertArraySingle);
      _converterFunctions.Add(typeof(double[]), convertArrayDouble);
      _converterFunctions.Add(typeof(byte[]), convertArrayByte);
      _converterFunctions.Add(typeof(bool[]), convertArrayBool);
      _converterFunctions.Add(typeof(int[]), convertArrayInt);
      _converterFunctions.Add(typeof(string[]), convertArrayString);
      _converterFunctions.Add(typeof(char[]), convertArrayChar);
      _converterFunctions.Add(typeof(DateTime[]), convertArrayDateTime);
      _converterFunctions.Add(typeof(TimeSpan[]), convertArrayTimeSpan);
      _converterFunctions.Add(typeof(Complex[]), convertArrayComplex);
   }
   
   public object CurrentObject => CurrentObjectToConvert;

   private static void setUseRDotNet(bool useIt)
   {
      var useRDotNet = DataConversionHelper.rSharpNativeDll.GetFunctionAddress("use_rdotnet");
      if (useRDotNet == IntPtr.Zero)
      {
         throw new EntryPointNotFoundException("Native symbol use_rdotnet not found");
      }

      Marshal.WriteInt32(useRDotNet, useIt ? 1 : 0);
   }

   /// <summary>
   ///    Enable/disable the use of this data converter in the R-CLR interop data marshalling.
   /// </summary>
   public static void SetRDotNet(bool setIt, string pathToNativeSharedObj = null)
   {
      InternalRSharpFacade.DataConverter = setIt ? getInstance(pathToNativeSharedObj) : null;
      setUseRDotNet(setIt);
   }

   /// <summary>
   ///    Enable/disable the use of this data converter for more "advanced" but unidirectional.
   /// </summary>
   public static void SetConvertAdvancedTypes(bool enable)
   {
      getInstance(null).ConvertAdvancedTypes = enable;
   }

   /// <summary>
   ///    Convert an object, if possible, using RDotNet capabilities
   /// </summary>
   /// <remarks>
   ///    If a conversion to an RDotNet SymbolicExpression was possible,
   ///    this returns the IntPtr SafeHandle.DangerousGetHandle() to be passed to R.
   ///    If the object is null or such that no known conversion is possible, the same object
   ///    as the input parameter is returned.
   /// </remarks>
   public object ConvertToR(object obj)
   {
      clearSexpHandles();
      if (obj == null)
         return null;

      if (obj is SymbolicExpression sexp)
         return returnHandle(sexp);

      sexp = tryConvertToSexp(obj);

      return sexp == null ? obj : returnHandle(sexp);
   }

   private static void clearSexpHandles()
   {
      // Release, on the R main thread, any SEXP handles whose SafeHandle finalized on the
      // CLR finalizer thread — that release was deferred (queued) there to avoid corrupting
      // R's heap. Without this, those preservations leak and the R objects (and any CLR
      // objects they pin) are never collected.
      DeferredRelease.Drain();

      // Dispose the previous batch of return-value SEXPs here, on the R main thread (this
      // runs during a marshalling call), instead of merely dropping the references and
      // letting the GC finalizer thread call R_ReleaseObject — which corrupts R's heap. By
      // this point R has already consumed the prior return handles.
      foreach (var sexp in _handles)
      {
         try { sexp?.Dispose(); }
         catch { /* never let cleanup throw across the native boundary */ }
      }
      _handles.Clear();
   }

   private static object returnHandle(SymbolicExpression sexp)
   {
      addSexpHandle(sexp);
      return sexp.DangerousGetHandle();
   }

   private static void addSexpHandle(SymbolicExpression sexp)
   {
      _handles.Add(sexp);
   }

   /// <summary>
   ///    A list to reference to otherwise transient SEXP created by this class.
   ///    This is to prevent .NET and R to trigger GC before rSharp function calls have returned to R.
   /// </summary>
   private static readonly List<SymbolicExpression> _handles = new();
   
   /// <summary>
   ///    Gets/sets whether to convert vectors using R.NET. Most users should never need to modify the default.
   /// </summary>
   public bool ConvertVectors { get; set; }

   /// <summary>
   ///    Gets/sets whether to convert non-primitive value types and vector thereof, e.g. TimeSpan and DateTime.
   ///    Most users should never need to modify the default.
   /// </summary>
   public bool ConvertValueTypes { get; set; }

   private bool _convertAdvancedTypes;

   /// <summary>
   ///    Gets/sets whether to convert more complicated types such as dictionaries, arrays of reference types, etc.
   /// </summary>
   public bool ConvertAdvancedTypes
   {
      get => _convertAdvancedTypes;
      set
      {
         _convertAdvancedTypes = value;
         if (value)
         {
            addDictionaries();
            addGeneralTypes();
         }
         else
         {
            removeDictionaries();
            removeGeneralTypes();
         }
      }
   }

   private static void setupREngine()
   {
      if (_engine != null) 
         return;

      _engine = REngine.GetInstance(initialize: false);
      _engine.Initialize(setupMainLoop: false);
      _engine.AutoPrint = false;
   }

   private static InternalRDotNetDataConverter _singleton;

   private static InternalRDotNetDataConverter getInstance(string pathToNativeSharedObj)
   {
      // Make sure this is set only once (RDotNet known limitation to one engine per session, effectively a singleton).
      return _singleton ??= new InternalRDotNetDataConverter(pathToNativeSharedObj);
   }

   private readonly Dictionary<Type, Func<object, SymbolicExpression>> _converterFunctions;

   private SymbolicExpression tryConvertToSexp(object obj)
   {
      if (obj != null)
      {
         var converter = tryGetConverter(obj);
         var sHandle = converter?.Invoke(obj);
         return sHandle;
      }

      throw new ArgumentNullException(nameof(obj));
   }

   private Func<object, SymbolicExpression> tryGetConverter(object obj)
   {
      var t = obj.GetType();
      if (_converterFunctions.TryGetValue(t, out var converter))
         return converter;

      if (tryGetGenericConverters(obj, out converter))
         return converter;

      return null;
   }

   private Func<object, SymbolicExpression> tryGetConverter(Type t)
   {
      return _converterFunctions.GetValueOrDefault(t);
   }

   private bool tryGetGenericConverters(object obj, out Func<object, SymbolicExpression> converter)
   {
      var t = obj.GetType();
      if (typeof(Array).IsAssignableFrom(t))
      {
         var array = obj as Array;
         if (array.Rank == 1)
            return _converterFunctions.TryGetValue(typeof(Array), out converter);
      }

      return _converterFunctions.TryGetValue(typeof(object), out converter);
   }

   private SymbolicExpression convertToSexp(object obj)
   {
      if (obj == null) return null;
      var result = tryConvertToSexp(obj);

      if (result == null)
         throw new NotSupportedException($"Cannot yet expose type {obj.GetType().FullName} as a SEXP");

      return result;
   }

   private GenericVector convertDictionary<T>(object obj)
   {
      var dict = (IDictionary<string, T>)obj;
      if (!_converterFunctions.ContainsKey(typeof(T[])))
         throw new NotSupportedException("Cannot convert a dictionary of type " + dict.GetType());

      var values = _converterFunctions[typeof(T[])].Invoke(dict.Values.ToArray());
      SetAttribute(values, dict.Keys.ToArray());
      return values.AsList();
   }

   private SymbolicExpression convertAll(IReadOnlyList<object> objects, Func<object, SymbolicExpression> converter = null)
   {
      var sexpArray = new SymbolicExpression[objects.Count];

      for (var i = 0; i < objects.Count; i++)
         sexpArray[i] = converter == null ? convertToSexp(objects[i]) : converter(objects[i]);

      return new GenericVector(_engine, sexpArray);
   }

   private SymbolicExpression convertArrayDouble(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (double[])obj;
      return _engine.CreateNumericVector(array);
   }

   private SymbolicExpression convertArrayBool(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (bool[])obj;
      return _engine.CreateLogicalVector(array);
   }

   private SymbolicExpression convertArrayByte(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (byte[])obj;
      return _engine.CreateRawVector(array);
   }

   private SymbolicExpression convertArraySingle(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (float[])obj;
      return convertArrayDouble(Array.ConvertAll(array, x => (double)x));
   }

   private SymbolicExpression convertArrayInt(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (int[])obj;
      return _engine.CreateIntegerVector(array);
   }

   private SymbolicExpression convertArrayString(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (string[])obj;
      return _engine.CreateCharacterVector(array);
   }

   private SymbolicExpression convertArrayChar(object obj)
   {
      if (!ConvertVectors)
         return null;

      var array = (char[])obj;
      return _engine.CreateCharacterVector(Array.ConvertAll(array, x => x.ToString()));
   }

   private SymbolicExpression convertArrayComplex(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var array = (Complex[])obj;
      return _engine.CreateComplexVector(array);
   }

   private SymbolicExpression convertArrayDateTime(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var array = (DateTime[])obj;

      var doubleArray = Array.ConvertAll(array, InternalRSharpFacade.GetRPosixCtDoubleRepresentation);
      var result = convertArrayDouble(doubleArray);
      AddPOSIXctAttributes(result);
      return result;
   }

   private SymbolicExpression convertArrayTimeSpan(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var array = (TimeSpan[])obj;
      var doubleArray = Array.ConvertAll(array, x => x.TotalSeconds);
      var result = convertArrayDouble(doubleArray);
      AddDiffTimeAttributes(result);
      return result;
   }

   private SymbolicExpression convertDouble(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (double)obj;
      
      return _engine.CreateNumeric(value);
   }

   private SymbolicExpression convertSingle(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (float)obj;
      return convertArrayDouble((double)value);
   }

   private SymbolicExpression convertByte(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (byte)obj;
      return _engine.CreateRaw(value);
   }

   private SymbolicExpression convertChar(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (char)obj;
      return _engine.CreateCharacter(value.ToString());
   }

   private SymbolicExpression convertBool(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (bool)obj;
      return _engine.CreateLogical(value);
   }

   private SymbolicExpression convertInt(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (int)obj;
      return _engine.CreateInteger(value);
   }

   private SymbolicExpression convertString(object obj)
   {
      if (!ConvertVectors)
         return null;

      var value = (string)obj;
      return _engine.CreateCharacter(value);
   }

   private SymbolicExpression convertTimeSpan(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var value = (TimeSpan)obj;
      var doubleValue = value.TotalSeconds;
      var result = convertDouble(doubleValue);
      AddDiffTimeAttributes(result);
      return result;
   }

   private SymbolicExpression convertDateTime(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var value = (DateTime)obj;
      var doubleValue = InternalRSharpFacade.GetRPosixCtDoubleRepresentation(value);
      var result = convertDouble(doubleValue);
      AddPOSIXctAttributes(result);
      return result;
   }

   private SymbolicExpression convertComplex(object obj)
   {
      if (!ConvertVectors)
         return null;

      if (!ConvertValueTypes)
         return null;

      var value = (Complex)obj;
      return _engine.CreateComplex(value);
   }

   private SymbolicExpression convertArrayObject(object obj)
   {
      var a = (Array)obj;
      return convertToList(a);
   }

   private SymbolicExpression convertObject(object obj)
   {
      return obj == null ? _engine.NilValue : createClrObj(obj);
   }

   private Function _createClrS4Object;

   public Function CreateClrS4Object => _createClrS4Object ??= _engine.Evaluate("invisible(rSharp:::.getCurrentConvertedObject)").AsFunction();

   public static object CurrentObjectToConvert { get; private set; }

   private S4Object createClrObj(object obj)
   {
      CurrentObjectToConvert = obj;
      var result = CreateClrS4Object.Invoke().AsS4();
      CurrentObjectToConvert = null;
      return result;
   }

   private SymbolicExpression convertToList(Array array)
   {
      if (array.Rank > 1)
         throw new NotSupportedException("Generic array converter is limited to uni-dimensional arrays");

      // CAUTION: The following, while efficient, means that more specialized converters
      // will not be picked up.
      var elementConverter = tryGetConverter(array.GetType().GetElementType());

      var tmp = new object[array.GetLength(0)];
      Array.Copy(array, tmp, tmp.Length);
      return convertAll(tmp, elementConverter);
   }

   private SymbolicExpression convertMatrixJaggedSingle(object obj)
   {
      var array = (float[][])obj;
      return array.IsRectangular() ? convertMatrixDouble(array.ToDoubleRect()) : convertToList(array.ToDouble());
   }

   private SymbolicExpression convertMatrixJaggedDouble(object obj)
   {
      var array = (double[][])obj;
      return array.IsRectangular() ? convertMatrixDouble(array.ToRect()) : convertToList(array);
   }

   private SymbolicExpression convertMatrixJaggedInt(object obj)
   {
      var array = (int[][])obj;
      return array.IsRectangular() ? convertMatrixInt(array.ToRect()) : convertToList(array);
   }

   private SymbolicExpression convertMatrixJaggedString(object obj)
   {
      var array = (string[][])obj;
      return array.IsRectangular() ? convertMatrixString(array.ToRect()) : convertToList(array);
   }

   private NumericMatrix convertMatrixSingle(object obj)
   {
      var array = (float[,])obj;
      return convertMatrixDouble(array.ToDoubleRect());
   }

   private NumericMatrix convertMatrixDouble(object obj)
   {
      var array = (double[,])obj;
      return _engine.CreateNumericMatrix(array);
   }

   private IntegerMatrix convertMatrixInt(object obj)
   {
      var array = (int[,])obj;
      return _engine.CreateIntegerMatrix(array);
   }

   private CharacterMatrix convertMatrixString(object obj)
   {
      var array = (string[,])obj;
      return _engine.CreateCharacterMatrix(array);
   }

   public static void SetTimeZoneAttribute(SymbolicExpression sexp, string timeZoneId)
   {
      SetAttribute(sexp, [timeZoneId], attributeName: "tzone");
   }

   public static void SetUnitsAttribute(SymbolicExpression sexp, string units)
   {
      SetAttribute(sexp, [units], attributeName: "units");
   }

   public static void SetClassAttribute(SymbolicExpression sexp, params string[] classes)
   {
      SetAttribute(sexp, classes, attributeName: "class");
   }

   public static void SetAttribute(SymbolicExpression sexp, string[] attributeValues, string attributeName = "names")
   {
      var names = new CharacterVector(_engine, attributeValues);
      sexp.SetAttribute(attributeName, names);
   }

   public static void AddPOSIXctAttributes(SymbolicExpression result)
   {
      SetClassAttribute(result, "POSIXct", "POSIXt");
      SetTimeZoneAttribute(result, "UTC");
   }

   public static bool IsOfClass(SymbolicExpression sexp, string className)
   {
      var classNames = GetClassAttribute(sexp);
      return classNames != null && classNames.Contains(className);
   }

   public static string[] GetAttribute(SymbolicExpression sexp, string attributeName)
   {
      var classes = sexp.GetAttribute(attributeName);
      var classNames = classes?.AsCharacter().ToArray();
      return classNames;
   }

   public static string[] GetClassAttribute(SymbolicExpression sexp)
   {
      return GetAttribute(sexp, "class");
   }

   public static string GetTimeZoneAttribute(SymbolicExpression sexp)
   {
      var v = GetAttribute(sexp, "tzone");
      return v?[0];
   }

   public static void AddDiffTimeAttributes(SymbolicExpression result)
   {
      SetClassAttribute(result, "difftime");
      SetUnitsAttribute(result, "secs");
   }
   
   public static REngine GetEngine()
   {
      return _engine;
   }

   private static REngine _engine;

   public SymbolicExpression CreateSymbolicExpression(IntPtr sexp)
   {
      return _engine.CreateFromNativeSexp(sexp);
   }

   public object[] ConvertSymbolicExpressions(object[] arguments)
   {
      var result = (object[])arguments.Clone();
      for (var i = 0; i < result.Length; i++)
      {
         result[i] = ConvertSymbolicExpression(arguments[i]);
      }

      return result;
   }

   public object ConvertSymbolicExpression(object obj)
   {
      if (obj is SymbolicExpressionWrapper wrapper)
         return convertSymbolicExpression(wrapper);

      return obj;
   }

   private object convertSymbolicExpression(SymbolicExpressionWrapper sexpWrap)
   {
      var result = sexpWrap.ToClrEquivalent();
      // Once the argument SEXP's value has been marshalled to a CLR object, release it here
      // on the R main thread so its SafeHandle never finalizes (and calls R_ReleaseObject)
      // on the GC finalizer thread. Skip when the wrapper hands back the SEXP itself
      // (unconverted types still need it alive).
      if (!ReferenceEquals(result, sexpWrap.Sexp))
      {
         try { sexpWrap.Sexp?.Dispose(); }
         catch { /* never let cleanup throw across the native boundary */ }
      }
      return result;
   }
}