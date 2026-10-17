/*
 * Faster random number generator. Makes random numbers based on
 * the xorshift rng.
 * 
 * Somewhere online it says that the period of this rng is 2^64 -1 ,
 * or 1.8 * 10^19 .  That's way more than I'll ever need! Nice. 
 */
package edu.uchc.cam.langevin.helpernovis;

public class Rand {

    private static long x; //The random number
    
    private static final double maxValue = (double)Long.MAX_VALUE;
    
    private static double nextGaussian = 0;
    private static boolean haveNextGaussian = false;

    public Rand(long seed){  //We seed the rng to get it going
        x = seed;
    }

    public static void seedRand(long seed){
        x = seed;
    }
    
    public static long randomLong(){
        x ^= (x << 21);
        x ^= (x >>> 35);
        x ^= (x << 4);
        return x;
    }

    public static long randomPosLong(){
        x ^= (x << 21);
        x ^= (x >>> 35);
        x ^= (x << 4);
        if(x > 0){
            return x;
        } else {
            return -x;
        }
    }

    public static double randomPosDouble(){
        x ^= (x << 21);
        x ^= (x >>> 35);
        x ^= (x << 4);
        if(x > 0){
            return x/maxValue;
        } else {
            return -x/maxValue;
        }
    }
    
    public static double randomDouble(){
        x ^= (x << 21);
        x ^= (x >>> 35);
        x ^= (x << 4);
        return x/maxValue;
    }
    
    public static double randomGaussian(){
        if(haveNextGaussian){
            haveNextGaussian = false;
            return nextGaussian;
        } else {
            double v1, v2, s;
            do{
                v1 = randomDouble();
                v2 = randomDouble();
                s = v1*v1 + v2*v2;
            } while (s >= 1 || s == 0);
            double multiplier = Math.sqrt(-2*Math.log(s)/s);
            nextGaussian = v2 * multiplier;
            haveNextGaussian = true;
            return v1 * multiplier;
        }
    }
    
    public static double randomDouble(double xmin, double xmax){
        return xmin + (xmax -xmin)*randomPosDouble();
    }

}
