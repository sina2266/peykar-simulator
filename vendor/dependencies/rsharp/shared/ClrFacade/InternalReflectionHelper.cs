using ClrFacade.Tests;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace ClrFacade;
   
public static class InternalReflectionHelper
{
   /// <summary>
   ///    Gets the simple names of assemblies loaded in the current domain.
   /// </summary>
   public static string[] GetLoadedAssemblyNames(bool fullName = false)
   {
      var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
      return Array.ConvertAll(loadedAssemblies, x => fullName ? x.GetName().FullName : x.GetName().Name);
   }

   /// <summary>
   ///    Gets the paths of assemblies if loaded in the current domain.
   /// </summary>
   public static string[] GetLoadedAssemblyURI(string[] assemblyNames)
   {
      var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
      var result = new string[assemblyNames.Length];
      for (var i = 0; i < assemblyNames.Length; i++)
      {
         var s = assemblyNames[i];
         var assembly = loadedAssemblies.FirstOrDefault(x => matchAssemblyName(x, s));
         result[i] = assembly == null ? "<not found>" : assembly.Location;
      }

      return result;
   }

   /// <summary>
   ///    Gets the full name of types (Type.FullName) contained in an assembly, given its simple name
   /// </summary>
   public static string[] GetTypesInAssembly(string assemblyName)
   {
      var loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
      var assembly = loadedAssemblies.FirstOrDefault((x => x.GetName().Name == assemblyName));
      if (assembly == null)
         return [$"Assembly '{assemblyName}' not found"];

      var types = assembly.GetExportedTypes();
      return Array.ConvertAll(types, t => t.FullName);
   }

   /// <summary>
   ///    Gets human-readable signatures of the member(s) of an object or its type.
   ///    The purpose is to explore CLR object members from R.
   /// </summary>
   /// <param name="obj">The object to reflect on, or the type of the object if already known</param>
   /// <param name="memberName">The name of the object/class member, e.g. the method name</param>
   public static string[] GetSignature(object obj, string memberName)
   {
      if (obj is Type type)
         return GetSignature_Type(type, memberName);

      return GetSignature_Type(obj.GetType(), memberName);
   }

   public static string[] GetSignature(string typeName, string memberName)
   {
      var type = InternalRSharpFacade.GetType(typeName);
      return type != null ? GetSignature_Type(type, memberName) : [];
   }

   /// <summary>
   ///    Gets human-readable signatures of the member(s) of a type.
   /// </summary>
   /// <param name="type">The type to reflect on</param>
   /// <param name="memberName">The name of the object/class member, e.g. the method name</param>
   public static string[] GetSignature_Type(Type type, string memberName)
   {
      var members = type.GetMember(memberName);
      var result = summarize(members);
      return result;
   }

   /// <summary>
   ///    Finds the first method in a type that matches a method name.
   ///    Explicit interface implementations are searched if required.
   /// </summary>
   public static MethodInfo GetMethod(Type classType, string methodName, Binder binder, BindingFlags bindingFlags, Type[] types)
   {
      var method = classType.GetMethod(methodName, bindingFlags, binder, types, null);
      if (method == null)
      {
         var ifTypes = classType.GetInterfaces();
         foreach (var t in ifTypes)
         {
            method = t.GetMethod(methodName, bindingFlags, binder, types, null);
            if (method != null)
               return method;
         }
      }

      if (method == null)
         method = findDefaultParameterMethod(classType, methodName, bindingFlags, types);
      if (method == null)
         method = findVarargMethod(classType, methodName, bindingFlags, types);
      return method;
   }

   public static bool HasOptionalParams(MethodInfo method)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0)
         return false;

      var p = parameters[^1];
      return p.IsOptional;
   }

   public static bool HasVarArgs(MethodInfo method)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0)
         return false;

      var p = parameters[^1];
      return IsVarArg(p);
   }

   public static bool IsVarArg(ParameterInfo parameterInfo)
   {
      var parameterAttribute = parameterInfo.GetCustomAttributes(typeof(ParamArrayAttribute), false);
      return parameterAttribute.Length > 0;
   }

   /// <summary>
   ///    Gets all the non-static public methods of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceMethods(object obj, string pattern)
   {
      var type = obj.GetType();
      return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Instance);
   }

   /// <summary>
   ///    Gets all the non-static public constructors of a class.
   /// </summary>
   /// <param name="typeName">type name</param>
   public static string[] GetConstructors(string typeName)
   {
      return GetConstructors(InternalRSharpFacade.GetType(typeName));
   }

   /// <summary>
   ///    Gets all the non-static public constructors of a class.
   /// </summary>
   /// <param name="type">type</param>
   public static string[] GetConstructors(Type type)
   {
      return getConstructors(type, BindingFlags.Public | BindingFlags.Instance);
   }

   /// <summary>
   ///    Gets all the static public methods of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticMethods(object obj, string pattern)
   {
      var type = obj.GetType();
      return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   public static string[] GetStaticMethods(string typeName, string pattern)
   {
      var type = InternalRSharpFacade.GetType(typeName);
      return getMethods(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   /// <summary>
   ///    Gets all the public fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetFields(object obj, string pattern)
   {
      var type = obj.GetType();
      return getFields(type, pattern, BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
   }

   /// <summary>
   ///    Gets all the non-static public fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceFields(object obj, string pattern)
   {
      Type type = obj.GetType();
      return getFields(type, pattern, BindingFlags.Public | BindingFlags.Instance);
   }

   /// <summary>
   ///    Gets all the static fields of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticFields(object obj, string pattern)
   {
      Type type = obj.GetType();
      return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   public static string[] GetStaticFields(string typeName, string pattern)
   {
      Type type = InternalRSharpFacade.GetType(typeName);
      return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   public static string[] GetStaticFields(Type type, string pattern)
   {
      return getFields(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   /// <summary>
   ///    Gets all the properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetProperties(object obj, string pattern)
   {
      var type = obj.GetType();
      return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
   }

   /// <summary>
   ///    Gets all the non-static public properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetInstanceProperties(object obj, string pattern)
   {
      var type = obj.GetType();
      return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Instance);
   }
   
   /// <summary>
   ///    Gets all the static public properties of an object with a name that contains a specific string.
   /// </summary>
   /// <param name="obj">The object to reflect on, or its type</param>
   /// <param name="pattern">The case-sensitive string to look for in member names</param>
   public static string[] GetStaticProperties(object obj, string pattern)
   {
      var type = obj.GetType();
      return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   public static string[] GetStaticProperties(string typeName, string pattern)
   {
      var type = InternalRSharpFacade.GetType(typeName);
      return getProperties(type, pattern, BindingFlags.Public | BindingFlags.Static);
   }

   public static string[] GetEnumNames(string enumTypename)
   {
      var t = InternalRSharpFacade.GetType(enumTypename);
      if (t != null) 
         return GetEnumNames(t);

      throw new ArgumentException($"Type not found: {enumTypename}");

   }

   public static string[] GetEnumNames(Type enumType)
   {
      if (enumType == null)
         throw new ArgumentNullException();

      if (typeof(Enum).IsAssignableFrom(enumType) == false)
         throw new ArgumentException($"{enumType} is not the type of an Enum");

      return Enum.GetNames(enumType);
   }

   public static string[] GetEnumNames(Enum e)
   {
      return GetEnumNames(e.GetType());
   }
   
   private static bool matchAssemblyName(Assembly a, string name)
   {
      var an = a.GetName();
      return an.FullName == name || an.Name == name;
   }

   private static string[] summarize(MemberInfo[] members)
   {
      var result = new string[members.Length];
      for (var i = 0; i < members.Length; i++)
         result[i] = summarize(members[i]);
      return result;
   }

   private static string summarize(MemberInfo member)
   {
      var ctor = member as ConstructorInfo;
      if (ctor != null) return summarizeConstructor(ctor);
      var field = member as FieldInfo;
      if (field != null) return summarizeField(field);
      var prop = member as PropertyInfo;
      if (prop != null) return summarizeProperty(prop);
      var method = member as MethodInfo;
      if (method != null) return summarizeMethod(method);
      throw new ArgumentException("MemberInfo is not a constructor, field, property of method-info, but of type {1}", member.GetType().ToString());
   }

   private static string summarizeConstructor(ConstructorInfo ctor)
   {
      var result = new StringBuilder();
      addMethodBaseInfo(ctor, result);
      result.Append($"Constructor: {ctor.Name}");

      var parameters = ctor.GetParameters();
      addParametersSummary(result, parameters);
      return result.ToString();
   }

   private static string summarizeMethod(MethodInfo method)
   {
      var result = new StringBuilder();
      addMethodBaseInfo(method, result);
      result.Append($"Method: {method.ReturnType.Name} {method.Name}");

      var parameters = method.GetParameters();
      addParametersSummary(result, parameters);
      return result.ToString();
   }

   private static string summarizeProperty(PropertyInfo prop)
   {
      return $"Property {prop.Name}, {prop.PropertyType.Name}, can write: {prop.CanWrite}";
   }

   private static string summarizeField(FieldInfo field)
   {
      return $"Field {field.Name}, {field.FieldType.Name}";
   }

   private static void addMethodBaseInfo(MethodBase methodBase, StringBuilder result)
   {
      if (methodBase.IsGenericMethod) result.Append("Generic, ");
      if (methodBase.IsGenericMethodDefinition) result.Append("Generic definition, ");
      if (methodBase.IsStatic) result.Append("Static, ");
      if (methodBase.IsAbstract) result.Append("abstract, ");
   }

   private static void addParametersSummary(StringBuilder result, ParameterInfo[] parameters)
   {
      if (parameters.Length > 0)
         result.Append(", ");
      result.Append(SummarizeTypes(parameters));
   }

   public static string SummarizeTypes(ParameterInfo[] parameters)
   {
      var types = parameters.Select(p => p.ParameterType).ToArray();
      return SummarizeTypes(types);
   }
   
   private static string[] getConstructors(Type type, BindingFlags flags)
   {
      return sort(Array.ConvertAll(type.GetConstructors(flags), summarizeConstructor));
   }

   private static MethodInfo findDefaultParameterMethod(Type classType, string methodName, BindingFlags bf, Type[] types)
   {
      var methods = classType.GetMethods(bf).Where(m => m.Name == methodName).Where(HasOptionalParams).ToList();

      if (!methods.Any())
         return null;

      var mi = methods.Where(m => exactTypeMatchesOptionalParams(m, types)).ToList();
      if (!mi.Any())
         mi = methods.Where(m => assignableTypesMatchesOptionalParams(m, types)).ToList();
      if (mi.Count > 1)
         mi = getLowestParameterMatch(mi, types, getFirstExactMatchOptionalParams).ToList();
      if (mi.Count > 1)
         throwAmbiguousMatch(mi);
      return mi.FirstOrDefault();
   }

   private static MethodInfo findVarargMethod(Type classType, string methodName, BindingFlags bf, Type[] types)
   {
      var methods = classType.GetMethods(bf).Where(m => m.Name == methodName).Where(HasVarArgs).ToList();
      if (!methods.Any())
         return null;

      var mi = methods.Where(m => exactTypeMatchesVarArgs(m, types)).ToList();
      if (!mi.Any())
         mi = methods.Where(m => assignableTypesMatchesVarArgs(m, types)).ToList();

      if (mi.Count == 1)
         return mi.FirstOrDefault();

      if (mi.Count > 1)
      {
         // Try to see whether an exact match on the non-params parameters, or on 
         // the params parameters, removes the ambiguity.
         var disambiguation = mi.Where(m => partialExactTypesMatchesVarArgs(m, types)).ToList();
         if (disambiguation.Count == 1)
            return disambiguation.FirstOrDefault();
         disambiguation = mi.Where(m => varArgsExactTypesMatchesVarArgs(m, types)).ToList();
         if (disambiguation.Count == 1)
            return disambiguation.FirstOrDefault();

         // last resort
         var closestMatches = getLowestParameterMatch(mi, types, getFirstExactMatchVarargMethods);
         if (closestMatches.Count <= 1)
            return closestMatches.FirstOrDefault();
         closestMatches = getHighestParameterMatch(closestMatches, types, getLastExactMatchVarargMethods);
         if (closestMatches.Count <= 1)
            return closestMatches.FirstOrDefault();

         // too hard basket. For now. 
         throwAmbiguousMatch(closestMatches);
      }

      // nothing found
      return null;
   }

   private static void throwAmbiguousMatch(IEnumerable<MethodInfo> mi)
   {
      var s = Environment.NewLine + TestUtilities.Concat(mi.Select(x => x.ToString()), Environment.NewLine) + Environment.NewLine;
      throw new AmbiguousMatchException(s);
   }

   private static bool exactTypeMatchesOptionalParams(MethodInfo method, Type[] types)
   {
      return testTypeMatchesOptionalParams(method, types, equals);
   }

   private static bool assignableTypesMatchesOptionalParams(MethodInfo method, Type[] types)
   {
      return testTypeMatchesOptionalParams(method, types, isAssignable);
   }

   private static bool exactTypeMatchesVarArgs(MethodInfo method, Type[] types)
   {
      return testTypeMatchesVarArgs(method, types, equals, equals);
   }

   private static bool assignableTypesMatchesVarArgs(MethodInfo method, Type[] types)
   {
      return testTypeMatchesVarArgs(method, types, isAssignable, isAssignable);
   }

   private static bool partialExactTypesMatchesVarArgs(MethodInfo method, Type[] types)
   {
      return testTypeMatchesVarArgs(method, types, equals, isAssignable);
   }

   private static bool varArgsExactTypesMatchesVarArgs(MethodInfo method, Type[] types)
   {
      return testTypeMatchesVarArgs(method, types, isAssignable, equals);
   }

   private static IReadOnlyList<MethodInfo> getLowestParameterMatch(IReadOnlyList<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
   {
      return getBestParameterMatch(mi, types, indexTest, x => x.Min());
   }

   private static IReadOnlyList<MethodInfo> getHighestParameterMatch(IReadOnlyList<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
   {
      return getBestParameterMatch(mi, types, indexTest, x => x.Max());
   }

   private static int getFirstExactMatchOptionalParams(MethodInfo m, Type[] types)
   {
      return indexFirstTypeMatchOptionalParams(m, types, equals);
   }

   private static int getFirstExactMatchVarargMethods(MethodInfo m, Type[] types)
   {
      return indexFirstTestTypeMatchesVarArgs(m, types, equals, equals);
   }

   private static int getLastExactMatchVarargMethods(MethodInfo m, Type[] types)
   {
      return indexBeforeTransitionToNoMatchVarArgs(m, types, equals);
   }
   private static bool testTypeMatchesOptionalParams(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0 && types.Length == 0)
         return true;

      if (types.Length > parameters.Length)
         return false; // this may be an issue with mix of default values and params keyword. So be it; feature later.

      if (types.Length < parameters.Length && !parameters[types.Length].IsOptional)
         return false; // there remains at least one non-optional parameters that is missing.

      return !types.Where((t, i) => !matchTest(parameters[i].ParameterType, t)).Any();
   }

   private static bool testTypeMatchesVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> stdParamsMatchTest, Func<Type, Type, bool> paramsParamsMatchTest)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0 && types.Length == 0)
         return true;

      if (types.Length < parameters.Length - 1)
         return false; // this may be an issue with mix of default values and params keyword. So be it; feature later.

      for (var i = 0; i < parameters.Length - 1; i++)
      {
         if (!stdParamsMatchTest(parameters[i].ParameterType, types[i]))
            return false;
      }

      var arrayType = parameters[^1].ParameterType;
      if (!arrayType.IsArray)
         throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");

      var t = arrayType.GetElementType();
      for (var i = parameters.Length - 1; i < types.Length; i++)
      {
         if (!paramsParamsMatchTest(t, types[i]))
            return false;
      }

      return true;
   }

   private static IReadOnlyList<MethodInfo> getBestParameterMatch(IReadOnlyList<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest, Func<IEnumerable<int>, int> bestScore)
   {
      var candidates = mi.ToList();
      var indicesFirstMatch = getIndexFirstMatch(mi, types, indexTest);
      var validIndices = GetPositivesOnly(indicesFirstMatch);

      if (!validIndices.Any())
         return Array.Empty<MethodInfo>();

      var bestIndex = bestScore(validIndices);

      return candidates.Where((_, i) => indicesFirstMatch[i] == bestIndex).ToList();
   }

   private static int indexFirstTypeMatchOptionalParams(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0 && types.Length == 0)
         return -1;

      if (types.Length > parameters.Length)
         return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.

      if (types.Length < parameters.Length && !parameters[types.Length].IsOptional)
         return -1; // there remains at least one non-optional parameters that is missing.

      for (var i = 0; i < types.Length; i++)
      {
         if (matchTest(parameters[i].ParameterType, types[i]))
            return i;
      }

      return -1;
   }


   private static int indexFirstTestTypeMatchesVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> stdParamsMatchTest, Func<Type, Type, bool> paramsParamsMatchTest)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0 && types.Length == 0)
         return -1;

      if (types.Length < parameters.Length - 1)
         return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.

      for (var i = 0; i < parameters.Length - 1; i++)
      {
         if (stdParamsMatchTest(parameters[i].ParameterType, types[i]))
            return i;
      }

      var arrayType = parameters[^1].ParameterType;
      if (!arrayType.IsArray)
         throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");

      var t = arrayType.GetElementType();
      for (var i = parameters.Length - 1; i < types.Length; i++)
      {
         if (paramsParamsMatchTest(t, types[i]))
            return i;
      }

      return -1;
   }

   /// <summary>
   ///    Finds the last index of a parameter that matches a criteria, before a transition to false.
   /// </summary>
   private static int indexBeforeTransitionToNoMatchVarArgs(MethodInfo method, Type[] types, Func<Type, Type, bool> matchTest)
   {
      var parameters = method.GetParameters();
      if (parameters.Length == 0 && types.Length == 0)
         return -1;

      if (types.Length < (parameters.Length - 1))
         return -1; // this may be an issue with mix of default values and params keyword. So be it; feature later.

      var hasHitMatch = false;
      for (var i = 0; i < parameters.Length - 1; i++)
      {
         if (!matchTest(parameters[i].ParameterType, types[i]))
         {
            if (hasHitMatch)
               return i - 1;
         }
         else
            hasHitMatch = true;
      }

      var arrayType = parameters[^1].ParameterType;
      if (!arrayType.IsArray)
         throw new ArgumentException("Inconsistent - arguments should not be packed with a non-array method parameter");

      var t = arrayType.GetElementType();
      for (var i = parameters.Length - 1; i < types.Length; i++)
      {
         if (!matchTest(t, types[i]))
         {
            if (hasHitMatch)
               return i - 1;
         }
         else
            hasHitMatch = true;
      }

      if (hasHitMatch)
         // for cases where blah(object, int, params int[]) called with int, int, int, int, int
         return types.Length;

      return -1;
   }

   private static List<int> getIndexFirstMatch(IEnumerable<MethodInfo> mi, Type[] types, Func<MethodInfo, Type[], int> indexTest)
   {
      var indicesFirstMatch = mi.Select(m => indexTest(m, types)).ToList();
      return indicesFirstMatch;
   }

   private static IReadOnlyList<int> GetPositivesOnly(List<int> indicesFirstMatch)
   {
      return indicesFirstMatch.Where(x => x >= 0).ToList();
   }

   private static bool equals(Type methodType, Type paramType)
   {
      return methodType == paramType;
   }

   private static bool isAssignable(Type methodType, Type paramType)
   {
      return methodType.IsAssignableFrom(paramType);
   }

   private static string[] getMethods(Type type, string pattern, BindingFlags flags)
   {
      var methodNames = type.GetMethods(flags).Where(method => method.Name.Contains(pattern)).Select(method => method.Name);
      return sort(methodNames.ToArray());
   }

   private static string[] getFields(Type type, string pattern, BindingFlags flags)
   {
      var fieldNames = type.GetFields(flags).Where(field => field.Name.Contains(pattern)).Select(field => field.Name);
      return sort(fieldNames.ToArray());
   }

   private static string[] getProperties(Type type, string pattern, BindingFlags flags)
   {
      var propNames = type.GetProperties(flags).Where(property => property.Name.Contains(pattern)).Select(property => property.Name);
      return sort(propNames.ToArray());
   }
   private static string[] sort(string[] result)
   {
      Array.Sort(result);
      return result;
   }

   internal static string SummarizeTypes(Type[] types)
   {
      var result = new StringBuilder();
      for (var i = 0; i < types.Length - 1; i++)
      {
         result.Append(types[i].Name);
         result.Append(", ");
      }

      if (types.Length > 0)
         result.Append(types[^1].Name);

      return result.ToString();
   }

   internal static string[] GetMethodParameterTypes(MethodBase method)
   {
      var parameters = method.GetParameters();

      return parameters.Select(parameter => parameter.ParameterType.FullName).ToArray();
   }

   internal static void ThrowMissingMethod(Type classType, string methodName, string modifier, Type[] types)
   {
      var s = types.Length == 0 ? "without method parameters" : "for method parameters " + InternalReflectionHelper.SummarizeTypes(types);
      throw new MissingMethodException($"Could not find a suitable {modifier} method {methodName} on type {classType.FullName} {s}");
   }

}