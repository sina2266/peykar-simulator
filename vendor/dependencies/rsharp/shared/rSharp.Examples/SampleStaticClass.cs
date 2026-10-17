namespace rSharp.Examples;

public static class SampleStaticClass
{
    public static string StaticString = "A string from static class";
   public static string GetAString()
   {
      return StaticString;
   }

    public static int Add(int a, int b)
    {
        return a + b;
    }

    public static SampleInstanceClass GetInstanceObject()
    {
        return new SampleInstanceClass();
    }
}