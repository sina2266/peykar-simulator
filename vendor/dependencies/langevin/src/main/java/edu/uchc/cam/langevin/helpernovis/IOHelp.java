/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.helpernovis;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Scanner;

/**
 *
 * @author pmichalski
 */
public class IOHelp {
    
    public static String ERROR = "Error";
    
    public static int POSITIVE = 0;
    public static int NONNEGATIVE = 1;
    
    // An array of decimal formatters for file output.  DF[i] is used to output
    // a double with i digits after the decimal.
    public static final DecimalFormat [] DF = new DecimalFormat[]{new DecimalFormat("0."), new DecimalFormat("0.0"), 
        new DecimalFormat("0.00"), new DecimalFormat("0.000"), new DecimalFormat("0.0000"), new DecimalFormat("0.00000"),
        new DecimalFormat("0.000000"), new DecimalFormat("0.0000000"), new DecimalFormat("0.00000000")};
    
    public static DecimalFormat scientificFormat = new DecimalFormat("0.00#E0");
    
    public static Scanner makeScanner(ArrayList<String> stringArray){
        StringBuilder sb = new StringBuilder();
        for (String string : stringArray) {
            sb.append(string);
            sb.append("\n");
        }
        return new Scanner(sb.toString());
    }
    
    // Print without braces
    public static String printArrayList(ArrayList<Double> list, int decimalDigits){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        StringBuilder sb = new StringBuilder();
        for(Double d : list){
            sb.append(DF[decimalDigits].format(d)).append(" ");
        }
        return sb.toString();
        // </editor-fold>
    }
    
    
    // This method should be used when we know that the next few entries of the
    // scanner are a name in quotes, and we want to extract that collection of 
    // individual strings as a single string, and we want to drop the quotes. 
    public static String getNameInQuotes(Scanner sc){
        StringBuilder sb  = new StringBuilder();
        String s = sc.next();
        char quote = '"';
        char singlequote = '\'';
        char rightbracket = '}';
        if(!(s.charAt(0) == quote || s.charAt(0) == singlequote)){
            if(s.charAt(0) == rightbracket){
                return s;
            } else if(s.equals("***")){
                return s;
            } else {
                System.out.println("Helper.getNameInQuotes() was started on " + s 
                        + ", a string that did not begin with a quote.");
                return ERROR;
            }
        } else {
            s = s.substring(1,s.length());
            // Now look to see if it has a trailing quote
            if(s.charAt(s.length()-1) == quote || s.charAt(s.length()-1) == singlequote){
                s = s.substring(0,s.length()-1);
                return s;
            } else {
                sb.append(s);
                while(sc.hasNext()){
                    String s1 = sc.next();
                    if(s1.charAt(s1.length()-1) == quote || s1.charAt(s1.length()-1) == singlequote){
                        s1 = s1.substring(0,s1.length()-1);
                        sb.append(" ").append(s1);
                        break;
                    } else {
                        sb.append(" ").append(s1);
                    }
                }
                return sb.toString();
            }
        }
    }
    
    /*
     * In several places I have to append one integer to another, but I need
     * to add in a certain amount of padding so that the resulting integer
     * always has the same digits. For example, let's say we're appending 
     * a number between 0 and 999 onto the number 4506.  Then, if the number
     * to append is 12, I would want the result to be 4506012.  If the number
     * to append is 8, we want 4506008. You get the idea.  We'll give this method
     * the current number, the number to append, and the "padding", which would
     * be 3 in the above example.
     */
    
    public static int appendNumber(int current, int toAppend, int padding){
        if(padding < Integer.toString(toAppend).length()){
            System.out.println("ERROR: Tried to append a number with more digits than the padding"
                    + " allows. Number was " + toAppend + " with a padding of " + padding + ".");
            return -10000;
        } else {
            StringBuilder sb = new StringBuilder(Integer.toString(current));
            switch(padding){
                case 1: {
                    sb.append(toAppend);
                    break;
                }
                case 2: {
                    if(toAppend < 10){
                        sb.append("0").append(toAppend);
                    } else {
                        sb.append(toAppend);
                    }
                    break;
                }
                case 3: {
                    if(toAppend < 10){
                        sb.append("00").append(toAppend);
                    } else if(toAppend < 100){
                        sb.append("0").append(toAppend);
                    } else {
                        sb.append(toAppend);
                    }
                    break;
                }
                case 4: {
                    if(toAppend < 10){
                        sb.append("000").append(toAppend);
                    } else if(toAppend < 100){
                        sb.append("00").append(toAppend);
                    } else if(toAppend < 1000){
                        sb.append("0").append(toAppend);
                    } else {
                        sb.append(toAppend);
                    }
                    break;
                }
                case 5: {
                    if(toAppend < 10){
                        sb.append("0000").append(toAppend);
                    } else if(toAppend < 100){
                        sb.append("000").append(toAppend);
                    } else if(toAppend < 1000){
                        sb.append("00").append(toAppend);
                    } else if(toAppend < 10000){
                        sb.append("0").append(toAppend);
                    } else {
                        sb.append(toAppend);
                    }
                    break;
                }
                default: {

                    System.out.println("The padding function was not made to handle padding > 5.");
                    return -10000;
                }
            }
            return Integer.parseInt(sb.toString());
        }
    }
    
    /**
     * I'd like to format my times so that it tells me the total time in 
     * days, hours, minutes, and seconds instead of just seconds.
     * 
     * @param startTime 
     * @param stopTime
     * @return The string in the form xx days yy hours zz minutes uu.uu seconds.
     */
    
    public static String formatTime(long startTime, long stopTime){
        StringBuilder sb = new StringBuilder();
        long totalMillis = stopTime - startTime;
        long days = totalMillis/(24*3600*1000);
        totalMillis -= days*24*3600*1000;
        long hours = totalMillis/(3600*1000);
        totalMillis -= hours*3600*1000;
        long minutes = totalMillis/(60*1000);
        totalMillis -= minutes*60*1000;
        long seconds = totalMillis/1000;
        totalMillis -= seconds*1000;
        String millis;
        if(totalMillis<10){
            millis = "00" + totalMillis;
        } else if(totalMillis < 100){
            millis = "0" + totalMillis;
        } else {
            millis = Long.toString(totalMillis);
        }
        
        if(days != 0){
            sb.append(days).append(" day");
            if(days > 1){
                sb.append("s ");
            } else {
                sb.append(" ");
            }
        }
        if(hours != 0 || days != 0){
            sb.append(hours).append(" hour");
            if(hours > 1){
                sb.append("s ");
            } else {
                sb.append(" ");
            }
        }
        if(minutes != 0 || days != 0 || hours != 0){
            sb.append(minutes).append(" min. ");
        }
        sb.append(seconds).append(".");
        sb.append(millis).append(" sec.");
        
        return sb.toString();
    }

    /*
     * This method takes a time in nanoseconds and formats it into a human-readable output
     * for durations ranging from microseconds up to weeks.
     * Mirrors the semantics of formatTime() above, but for nanoseconds.
     *
     * Examples:
     *      formatNanoseconds(500);                     // "500ns"
     *      formatNanoseconds(12_000);                  // "12µs"
     *      formatNanoseconds(8_000_000);               // "8ms"
     *      formatNanoseconds(2_345_678_900L);          // "2s 345ms 678µs 900ns"
     *      formatNanoseconds(3_600_000_000_000L);      // "1h"
     *      formatNanoseconds(172_800_000_000_000L);    // "2d"
     */
    public static String formatNanoseconds(long ns) {
        if (ns < 0) {
            return "0ns";
        }

        final long NS_PER_US = 1_000L;
        final long NS_PER_MS = 1_000_000L;
        final long NS_PER_SEC = 1_000_000_000L;
        final long NS_PER_MIN = NS_PER_SEC * 60;
        final long NS_PER_HOUR = NS_PER_MIN * 60;
        final long NS_PER_DAY = NS_PER_HOUR * 24;

        long days = ns / NS_PER_DAY;
        ns %= NS_PER_DAY;

        long hours = ns / NS_PER_HOUR;
        ns %= NS_PER_HOUR;

        long minutes = ns / NS_PER_MIN;
        ns %= NS_PER_MIN;

        long seconds = ns / NS_PER_SEC;
        ns %= NS_PER_SEC;

        long millis = ns / NS_PER_MS;
        ns %= NS_PER_MS;

        long micros = ns / NS_PER_US;
        ns %= NS_PER_US;

        long nanos = ns;

        StringBuilder sb = new StringBuilder();

        if (days > 0) sb.append(days).append("d ");
        if (hours > 0) sb.append(hours).append("h ");
        if (minutes > 0) sb.append(minutes).append("m ");
        if (seconds > 0) sb.append(seconds).append("s ");
        if (millis > 0) sb.append(millis).append("ms ");
        if (micros > 0) sb.append(micros).append("µs ");
        if (nanos > 0 || sb.length() == 0) sb.append(nanos).append("ns");

        return sb.toString().trim();
    }

    /*
     * This method takes a time in nanoseconds and formats it into a human-readable output
     * for durations ranging from microseconds up to weeks, but only includes the specified
     * number of non-zero units in the output.
     *
     * Examples:
     *      formatNanoseconds(1, 500);                     // "500ns"
     *      formatNanoseconds(1, 12_000);                  // "12µs"
     *      formatNanoseconds(1, 8_000_000);               // "8ms"
     *      formatNanoseconds(1, 2_345_678_900L);          // "2s 345ms"
     *      formatNanoseconds(1, 3_600_000_000_000L);      // "1h"
     *      formatNanoseconds(1, 172_800_000_000_000L);    // "2d"
     */
    public static String formatNanoseconds(int positions, long ns) {
        if (ns < 0) {
            return "0ns";
        }

        final long NS_PER_US = 1_000L;
        final long NS_PER_MS = 1_000_000L;
        final long NS_PER_SEC = 1_000_000_000L;
        final long NS_PER_MIN = NS_PER_SEC * 60;
        final long NS_PER_HOUR = NS_PER_MIN * 60;
        final long NS_PER_DAY = NS_PER_HOUR * 24;

        long days = ns / NS_PER_DAY;
        ns %= NS_PER_DAY;

        long hours = ns / NS_PER_HOUR;
        ns %= NS_PER_HOUR;

        long minutes = ns / NS_PER_MIN;
        ns %= NS_PER_MIN;

        long seconds = ns / NS_PER_SEC;
        ns %= NS_PER_SEC;

        long millis = ns / NS_PER_MS;
        ns %= NS_PER_MS;

        long micros = ns / NS_PER_US;
        ns %= NS_PER_US;

        long nanos = ns;

        // Ordered list of units
        long[] values = {
                days, hours, minutes, seconds, millis, micros, nanos
        };

        String[] labels = {
                "d", "h", "m", "s", "ms", "µs", "ns"
        };

        // Find first non-zero unit
        int first = 0;
        while (first < values.length && values[first] == 0) {
            first++;
        }

        // If everything is zero
        if (first == values.length) {
            return "0ns";
        }

        // Build output with exactly "positions" units
        StringBuilder sb = new StringBuilder();
        int count = 0;

        for (int i = first; i < values.length && count < positions; i++) {
            sb.append(values[i]).append(labels[i]);
            if (count < positions - 1) sb.append(" ");
            count++;
        }

        return sb.toString().trim();
    }



    /*******************************************************************\
     *                    CHECK NEW VALUE METHODS                      *
     * We want to check the values of the various system parameters    *
     * as we read them in from the input file.  (There are checks to   *
     * make sure that the file is generated correctly, but the user    *
     * is free to modify that output file, and we need to make sure    *
     * that they haven't used a bad value somewhere.)                  *
     * We'll have two methods, one for doubles, one for integers.      *
    \*******************************************************************/
    
    public static double checkDouble(String newValue, String fieldID, int i){
        double value = 0;

        try{
            value = Double.parseDouble(newValue);
            if(i == POSITIVE){
                if(value <= 0){
                    System.out.println("Error: " + fieldID + " was not positive.");
                }
            } else if(i == NONNEGATIVE){
                if(value < 0){
                    System.out.println("Error: " + fieldID + " was negative.");
                }
            }
        } catch(NumberFormatException nfe){
            System.out.println("Error: " + fieldID + " could not be interpreted as a double value.  Received: " + newValue);
        }
        
        return value;
    }
    
    public static int checkInteger(String newValue, String fieldID, int i){
        int value = 0;
        try{
            value = Integer.parseInt(newValue);
            if(i == POSITIVE){
                if(value <= 0){
                    System.out.println("Error: " + fieldID + " was not positive.");
                }
            } else if(i== NONNEGATIVE){
                if(value < 0){
                    System.out.println("Error: " + fieldID + " was negative.");
                }
            }
        } catch(NumberFormatException nfe){

            System.out.println("Error: " + fieldID + " could not be interpreted as an integer value.  Received: " + newValue);
        }
        
        return value;
    }
    
}
