using System;
using System.Reflection;
using ClrFacade;
using Xunit;

namespace rSharpTests;

public class reflection_tests
{
   private class MyTestClass
   {
      public void NoParams()
      {
      }

      public void IntParams(params int[] p)
      {
      }

      public void DoubleIntParams(double d, params int[] p)
      {
      }

      public void StringSameNameParams(string s, params int[] p)
      {
      }

      public void StringSameNameParams(string s, params double[] p)
      {
      }

      public void UniqueNameStrStrParams(string s, string s2, params double[] p)
      {
      }

      public void NoDiffSameNameParams(params int[] p)
      {
      }

      public void NoDiffSameNameParams(params double[] p)
      {
      }

      public void OptionalInt(int i = 0)
      {
      }

      public void IntOptionalInt(int blah, int i = 0)
      {
      }

      public void DoubleOptionalInt(double blah, int i = 0)
      {
      }

      public void DoubleOptionalIntDoubleString(double blah, int i = 0, double d2 = 5.6, string tag = "tag")
      {
      }

      public string OptionalArgsMatch(IMyInterface anInterface, int i = 0)
      {
         return "IMyInterface";
      }

      public string OptionalArgsMatch(LevelOneClass anInterface, int i = 0)
      {
         return "LevelOneClass";
      }

      public string OptionalArgsMatch(LevelTwoClass anInterface, int i = 0)
      {
         return "LevelTwoClass";
      }
   }

   private interface IMyInterface
   {
   }

   private class LevelOneClass : IMyInterface
   {
   }

   private class OtherLevelOneClass : IMyInterface
   {
   }

   private class LevelTwoClass : LevelOneClass
   {
   }

   [Fact]
   public void test_variable_argument_method_binding()
   {
      const BindingFlags bf = BindingFlags.Public | BindingFlags.Instance | BindingFlags.InvokeMethod;
      var t = typeof(MyTestClass);
      Assert.False(InternalReflectionHelper.HasVarArgs(getSingleMethod(t, "NoParams", bf)));
      Assert.True(InternalReflectionHelper.HasVarArgs(getSingleMethod(t, "IntParams", bf)));

      var tint = typeof(int);
      var td = typeof(double);
      var to = typeof(object);
      var ts = typeof(string);

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "IntParams", null, bf, new[] { tint, tint, tint, tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "IntParams", null, bf, new[] { tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "IntParams", null, bf, Type.EmptyTypes));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "IntParams", null, bf, new[] { to, to }));

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "StringSameNameParams", null, bf, new[] { ts, tint, tint, tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "StringSameNameParams", null, bf, new[] { ts, td, td, td }));

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "NoDiffSameNameParams", null, bf, new[] { tint, tint, tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "NoDiffSameNameParams", null, bf, new[] { td, td, td }));
   }

   [Fact]
   public void test_reflection_type_load_exception()
   {
      var reflectionTypeLoadException = new ReflectionTypeLoadException(
         new[] { GetType() },
         new[] { new Exception("some inner message") },
         "reflection type load exception message");

      string s = InternalRSharpFacade.FormatExceptionInnermost(reflectionTypeLoadException);
      Assert.Contains("some inner message", s);
      Assert.Contains("reflection type load exception message", s);
   }

   [Fact]
   public void test_optional_parameters_method_binding()
   {
      var bf = BindingFlags.Public | BindingFlags.Instance | BindingFlags.InvokeMethod;
      var t = typeof(MyTestClass);
      Assert.False(InternalReflectionHelper.HasOptionalParams(getSingleMethod(t, "NoParams", bf)));
      Assert.True(InternalReflectionHelper.HasOptionalParams(getSingleMethod(t, "OptionalInt", bf)));
      Assert.True(InternalReflectionHelper.HasOptionalParams(getSingleMethod(t, "IntOptionalInt", bf)));

      var tint = typeof(int);
      var td = typeof(double);
      var to = typeof(object);
      var ts = typeof(string);

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "OptionalInt", null, bf, new[] { tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "OptionalInt", null, bf, Type.EmptyTypes));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "OptionalInt", null, bf, new[] { tint, tint }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "OptionalInt", null, bf, new[] { td, tint }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "OptionalInt", null, bf, new[] { tint, td }));

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "IntOptionalInt", null, bf, new[] { tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "IntOptionalInt", null, bf, new[] { tint, tint }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "IntOptionalInt", null, bf, new[] { tint, tint, tint }));

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalInt", null, bf, new[] { td }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalInt", null, bf, new[] { td, tint }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "DoubleOptionalInt", null, bf, new[] { td, tint, tint }));

      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, tint, td, ts }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, tint, td }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, tint }));
      Assert.NotNull(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td }));

      Assert.Null(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, tint, td, ts, to }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, tint, td, to }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, new[] { td, to }));
      Assert.Null(InternalReflectionHelper.GetMethod(t, "DoubleOptionalIntDoubleString", null, bf, Type.EmptyTypes));
   }

   [Fact]
   public void test_optional_parameters_method_invocation()
   {
      // TODO tighter checks. Start with: it does not bomb...
      var obj = new MyTestClass();
      InternalRSharpFacade.CallInstanceMethod(obj, "OptionalInt");
      InternalRSharpFacade.CallInstanceMethod(obj, "OptionalInt", 3);

      InternalRSharpFacade.CallInstanceMethod(obj, "IntOptionalInt", 3);
      InternalRSharpFacade.CallInstanceMethod(obj, "IntOptionalInt", 3, 5);

      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalInt", 3.0);
      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalInt", 3.0, 5);

      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalIntDoubleString", 3.0, 5, 4.5, "blah");
      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalIntDoubleString", 3.0, 5);
      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalIntDoubleString", 3.0, 5);
      InternalRSharpFacade.CallInstanceMethod(obj, "DoubleOptionalIntDoubleString", 3.0);

      Assert.Equal("LevelOneClass", InternalRSharpFacade.CallInstanceMethod(obj, "OptionalArgsMatch", new LevelOneClass()));
      Assert.Equal("LevelTwoClass", InternalRSharpFacade.CallInstanceMethod(obj, "OptionalArgsMatch", new LevelTwoClass()));
      Assert.Equal("IMyInterface", InternalRSharpFacade.CallInstanceMethod(obj, "OptionalArgsMatch", new OtherLevelOneClass()));
   }

   private MethodInfo getSingleMethod(Type classType, string methodName, BindingFlags bf, Binder binder = null, Type[] types = null)
   {
      types ??= Type.EmptyTypes;
      var method = classType.GetMethod(methodName, bf, binder, types, null);
      return method != null ? method : classType.GetMethod(methodName);
   }
}