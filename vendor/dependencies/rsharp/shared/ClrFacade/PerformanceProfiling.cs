using System;
using System.Diagnostics;

namespace ClrFacade;

// Classes and methods in this file are used dynamically by their names. 
// Do not change the names or signatures without also updating usages in R

/// <summary>
///    A class to use in order to measure the time cost of rSharp features.
/// </summary>
public class PerformanceProfiling
{
   public static double GetElapsedSeconds(Stopwatch sw)
   {
      var nanoSecondsPerTick = 1000L * 1000L * 1000L / Stopwatch.Frequency;
      var deltaNano = sw.ElapsedTicks * nanoSecondsPerTick;
      return deltaNano / 1e9;
   }

   public void SetDoubleArrays(int seed, int length, int numArrays)
   {
      _doubleArray = new double[numArrays][];
      var r = new Random(seed);
      for (var i = 0; i < _doubleArray.Length; i++)
      {
         _doubleArray[i] = new double[length];
         for (var j = 0; j < length; j++)
            _doubleArray[i][j] = r.NextDouble();
      }

      _counterDouble = 0;
   }

   private double[][] _doubleArray;
   private int _counterDouble;

   public double[] GetNextArrayDouble()
   {
      var res = _doubleArray[_counterDouble % _doubleArray.Length];
      _counterDouble++;
      return res;
   }

   public void CallMethodWithArrayDouble(double[] someParameter)
   {
      // nothing
   }

   public void ArrayDoubleSink(double[] ignored)
   {
      // nothing.
   }

   public void DoNothing()
   {
      // nothing.
   }
}