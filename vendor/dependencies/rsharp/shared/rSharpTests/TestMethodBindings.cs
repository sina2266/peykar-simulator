using System;
using System.Reflection;
using ClrFacade;
using ClrFacade.Tests.RefClasses;
using Xunit;
using static ClrFacade.InternalRSharpFacade;

namespace rSharpTests
{
   /// <summary>
   ///    Do not modify the .cs file: T4 generated class to support the unit tests for method binding
   /// </summary>
   public class test_method_bindings
   {
      [Fact]
      public void test_method_binding_optional_parameters()
      {
         var typeName = typeof(TestMethodBinding).FullName;
         var anInt = 1;
         var aDouble = Math.PI;
         var anObject = new object();

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anObject, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anObject, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anObject, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anObject, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anObject, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anInt, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anInt, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anInt, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anObject, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anObject, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anObject, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anObject, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anObject, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anObject, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anObject, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anInt, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anInt, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anInt, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anObject, anInt, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anObject, anInt, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anObject, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anObject, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anObject, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anObject, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anObject, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anInt, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anInt, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anInt, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anObject, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anObject, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anObject, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anObject, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anObject, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anObject, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anObject, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anObject, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anObject, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anObject, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anObject),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anInt, anObject));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, aDouble, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, aDouble, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, aDouble, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, aDouble, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, aDouble, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, anInt, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, anInt, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, anInt, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, aDouble, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, aDouble, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, aDouble, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, aDouble, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, aDouble, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, aDouble, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, aDouble, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, anInt, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, anInt, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, anInt, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(aDouble, anInt, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", aDouble, anInt, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, aDouble, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, aDouble, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, aDouble, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, aDouble, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, aDouble, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, anInt, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, anInt, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, aDouble, anInt, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, aDouble, anInt, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, aDouble, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, aDouble, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, aDouble, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, aDouble, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, aDouble, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, aDouble, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, aDouble, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, aDouble, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, aDouble, anInt));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, aDouble),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anInt, aDouble));

         Assert.Equal(
            TestMethodBinding.SomeMethodWithVarArgs(anInt, anInt, anInt, anInt, anInt),
            CallStaticMethod(typeName, "SomeMethodWithVarArgs", anInt, anInt, anInt, anInt, anInt));

         Assert.Equal(
            TestMethodBinding.MultipleMatchVarArgs(anObject, new LevelOneClass(), anObject, anObject, anObject),
            CallStaticMethod(typeName, "MultipleMatchVarArgs", anObject, new LevelOneClass(), anObject, anObject, anObject));

         Assert.Equal(
            TestMethodBinding.MultipleMatchVarArgs(anObject, new LevelTwoClass(), anObject, anObject, anObject),
            CallStaticMethod(typeName, "MultipleMatchVarArgs", anObject, new LevelTwoClass(), anObject, anObject, anObject));

         Assert.Throws<AmbiguousMatchException>(
            () => { CallStaticMethod(typeName, "MultipleMatchVarArgs", anObject, new LevelThreeClass(), anObject, anObject, anObject); });
      }
   }
}