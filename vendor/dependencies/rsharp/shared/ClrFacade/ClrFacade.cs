using System;
using System.Reflection;

namespace ClrFacade;

// Classes and methods in this file are used dynamically by their names. 
// Do not change the names or signatures without also updating usages in R

public static class ClrFacade
{
   public static string LastException => InternalRSharpFacade.LastException;

   public delegate int CallInstanceMethodDelegate(IntPtr obj, string methodName, IntPtr arguments, int numObjects, IntPtr returnValue);

   public static int CallInstanceMethod(IntPtr obj, string methodName, IntPtr arguments, int numObjects, IntPtr returnValue) => InternalRSharpFacade.CallInstanceMethod(obj, methodName, arguments, numObjects, returnValue);

   public delegate int CreateSexpWrapperDelegate(long ptrValue, IntPtr returnValue);

   public static int CreateSexpWrapperLong(long ptrValue, IntPtr returnValue) => InternalRSharpFacade.CreateSexpWrapper(new IntPtr(ptrValue), returnValue);

   public delegate int CallStaticMethodDelegate(string typename, string methodName, IntPtr objects, int numObjects, IntPtr returnValue);

   public static int CallStaticMethod(string typename, string methodName, IntPtr objects, int numObjects, IntPtr returnValue) => InternalRSharpFacade.CallStaticMethod(typename, methodName, objects, numObjects, returnValue);

   public delegate int CurrentObjectDelegate(IntPtr returnValue);

   public static int CurrentObject(IntPtr returnValue) => InternalRSharpFacade.CurrentObject(returnValue);

   public delegate int CreateInstanceDelegate(string typename, IntPtr objects, int numObjects, IntPtr returnValue);

   public static int CreateInstance(string typename, IntPtr objects, int numObjects, IntPtr returnValue) => InternalRSharpFacade.CreateInstance(typename, objects, numObjects, returnValue);

   public delegate IntPtr GetObjectTypeNameDelegate(IntPtr obj);

   public static IntPtr GetObjectTypeName(IntPtr obj) => InternalRSharpFacade.GetObjectTypeName(obj);

   public static string GetObjectTypeName(object obj) => InternalRSharpFacade.GetObjectTypeName(obj);

   public delegate int LoadFromDelegate(string pathOrAssemblyName, IntPtr returnValue);

   public static int LoadFrom(string pathOrAssemblyName, IntPtr returnValue) => InternalRSharpFacade.LoadFrom(pathOrAssemblyName, returnValue);

   public static void SetFieldOrProperty(object obj, string name, object value) => InternalRSharpFacade.SetFieldOrProperty(obj, name, value);

   public static void SetFieldOrProperty(string typename, string name, object value) => InternalRSharpFacade.SetFieldOrProperty(typename, name, value);

   public static object GetFieldOrProperty(string typename, string name) => InternalRSharpFacade.GetFieldOrProperty(typename, name);

   public static object GetFieldOrProperty(object obj, string name) => InternalRSharpFacade.GetFieldOrProperty(obj, name);

   public static Type GetType(string typename) => InternalRSharpFacade.GetType(typename);

   public delegate void FreeObjectDelegate(IntPtr obj);

   public static void FreeObject(IntPtr obj) => InternalRSharpFacade.FreeObject(obj);
   
   public static string ToString(object obj) => InternalRSharpFacade.ToString(obj);

   /// <summary>
   ///    Gets the paths of assemblies if loaded in the current domain.
   /// </summary>
   public static string[] GetLoadedAssemblyURI(string[] assemblyNames) => InternalReflectionHelper.GetLoadedAssemblyURI(assemblyNames);

   /// <summary>
   ///    Gets the simple names of assemblies loaded in the current domain.
   /// </summary>
   public static string[] GetLoadedAssemblyNames(bool fullName = false) => InternalReflectionHelper.GetLoadedAssemblyNames(fullName);

   /// <summary>
   ///    Gets the full name of types (Type.FullName) contained in an assembly, given its simple name
   /// </summary>
   public static string[] GetTypesInAssembly(string assemblyName) => InternalReflectionHelper.GetTypesInAssembly(assemblyName);

   /// <summary>
   ///    Gets human-readable signatures of the member(s) of an object or its type.
   ///    The purpose is to explore CLR object members from R.
   /// </summary>
   /// <param name="obj">The object to reflect on, or the type of the object if already known</param>
   /// <param name="memberName">The name of the object/class member, e.g. the method name</param>
   public static string[] GetSignature(object obj, string memberName) => InternalReflectionHelper.GetSignature(obj, memberName);

   public static string[] GetSignature(string typeName, string memberName) => InternalReflectionHelper.GetSignature(typeName, memberName);
   
   /// <summary>
   ///    Gets all the non-static public methods of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceMethods(object obj, string pattern) => InternalReflectionHelper.GetInstanceMethods(obj, pattern);

   /// <summary>
   ///    Gets all the non-static public constructors of a class.
   /// </summary>
   /// <param name="typeName">type name</param>
   public static string[] GetConstructors(string typeName) => InternalReflectionHelper.GetConstructors(typeName);

   /// <summary>
   ///    Gets all the non-static public constructors of a class.
   /// </summary>
   /// <param name="type">type</param>
   public static string[] GetConstructors(Type type) => InternalReflectionHelper.GetConstructors(type);

   /// <summary>
   ///    Gets all the static public methods of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticMethods(object obj, string pattern) => InternalReflectionHelper.GetStaticMethods(obj, pattern);

   public static string[] GetStaticMethods(string typeName, string pattern) => InternalReflectionHelper.GetStaticMethods(typeName, pattern);

   /// <summary>
   ///    Gets all the public fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetFields(object obj, string pattern) => InternalReflectionHelper.GetFields(obj, pattern);

   /// <summary>
   ///    Gets all the non-static public fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceFields(object obj, string pattern) => InternalReflectionHelper.GetInstanceFields(obj, pattern);

   /// <summary>
   ///    Gets all the static fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticFields(object obj, string pattern) => InternalReflectionHelper.GetStaticFields(obj, pattern);

   public static string[] GetStaticFields(string typeName, string pattern) => InternalReflectionHelper.GetStaticFields(typeName, pattern);

   public static string[] GetStaticFields(Type type, string pattern) => InternalReflectionHelper.GetStaticFields(type, pattern);

   /// <summary>
   ///    Gets all the properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetProperties(object obj, string pattern) => InternalReflectionHelper.GetProperties(obj, pattern);

   /// <summary>
   ///    Gets all the non-static public properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceProperties(object obj, string pattern) => InternalReflectionHelper.GetInstanceProperties(obj, pattern);

   /// <summary>
   ///    Gets all the static public properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticProperties(object obj, string pattern) => InternalReflectionHelper.GetStaticProperties(obj, pattern);

   public static string[] GetStaticProperties(string typeName, string pattern) => InternalReflectionHelper.GetStaticProperties(typeName, pattern);

   public static string[] GetEnumNames(string enumTypename) => InternalReflectionHelper.GetEnumNames(enumTypename);

   public static string[] GetEnumNames(Type enumType) => InternalReflectionHelper.GetEnumNames(enumType);

   public static string[] GetEnumNames(Enum e) => InternalReflectionHelper.GetEnumNames(e);
   
   public static void SetRDotNet(bool setIt, string pathToNativeSharedObj = null) => InternalRDotNetDataConverter.SetRDotNet(setIt, pathToNativeSharedObj);

   public static void SetConvertAdvancedTypes(bool enable) => InternalRDotNetDataConverter.SetConvertAdvancedTypes(enable);

}