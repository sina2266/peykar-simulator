namespace rSharp.Examples;

public class SampleInstanceClass
{
    public static string StaticString = "A static string from a non-static class";

    public int FieldIntegerOne;
    public int FieldIntegerTwo;

    public double FieldDoubleOne;
    public double FieldDoubleTwo;

    public SampleInstanceClass()
    {
    }

    public SampleInstanceClass(int f1)
    {
        FieldIntegerOne = f1;
    }

    public SampleInstanceClass(int f1, int f2)
       : this(f1)
    {
        FieldIntegerTwo = f2;
    }

    public SampleInstanceClass(double d1)
    {
        FieldDoubleOne = d1;
    }

    public SampleInstanceClass(double d1, double d2)
       : this(d1)
    {
        FieldDoubleTwo = d2;
    }

    public SampleInstanceClass(int f1, int f2, double d1, double d2)
    {
        FieldIntegerOne = f1;
        FieldIntegerTwo = f2;
        FieldDoubleOne = d1;
        FieldDoubleTwo = d2;
    }

    public string GetAString()
   {
      return "A string from instance class";
   }
}