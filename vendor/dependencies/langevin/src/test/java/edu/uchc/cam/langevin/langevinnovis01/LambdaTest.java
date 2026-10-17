package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.OnRateSolver;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.VCellMessagingLocal;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.text.DecimalFormat;
import java.util.Arrays;

public class LambdaTest {

    @Test
    public void testLambda() {

        double lambdaIrreversible = OnRateSolver.getrootIrreversible(5, 10, 0.08*1000000.0, 5*1660000.0);
        System.out.println(lambdaIrreversible);

        OnRateSolver.checkInequalities(4,6,20,5,Math.pow(10,-9));
        OnRateSolver.checkInequalities(4,6,20,5,Math.pow(10,-9));
        OnRateSolver.checkInequalities(2,2.2,2,5,2*Math.pow(10,-9));

        for(int i=0;i<10;i++){
            System.out.println(OnRateSolver.g(0.5,0.6+i,5,1,0.005));
        }

        // double lambda = OnRateSolver.getrootReversible(0.2, 1.2, 1.2, 20.0*1000000.0, 1000*1660000.0);
        double lambdaReversible = OnRateSolver.getrootReversible(0.2, 1.2, 1.2, 20.0*1000000.0, 1000*1660000.0);
        System.out.println(lambdaReversible);
    }

    @Test
    public void testLambda2() {

        double[] kon = {0.05, 0.1, 0.5, 1.0, 5.0, 10.0 , 40.0};    // uM^-1 * s^-1
//        double[] kon = { 5.0, 10.0 , 15.0, 20.0, 25.0, 30.0, 35.0, 40.0};    // uM^-1 * s^-1
        double koff = 25.0;
        double[] dt = { 1.0E-9, 5.0E-9, 1.0E-8, 5.0E-8 };

        String[] relativeLambdaErrorExpected = { "0.01", "0.02", "0.12", "0.24", "1.32", "2.99", "55.65"};
        String[] relativeLambdaErrorCalculated = new String[kon.length];

        String relativeOffProbabilityErrorExpected = "2.25";
        String relativeOffProbabilityErrorCalculated = "-1.0";

        double siteRadius1 = 1.0;       // nm
        double siteRadius2 = 1.0;
        double reactionRadius1 = 1.5;   // nm
        double reactionRadius2 = 1.5;
        double diffusionRate1 = 1.0;    // um^2/s
        double diffusionRate2 = 1.0;

        double p = siteRadius1 + siteRadius2;           // site radius
        double R = reactionRadius1 + reactionRadius2;   // reaction radius
        double D = 1000000.0 * (diffusionRate1 + diffusionRate2);   // diffusion rate

        System.out.println("");
        System.out.println("Kon (uM^-1*s^-1) : % relative error (old lambda vs. new lambda)");
        System.out.println("dt (sec) : % relative error (old probability rate vs. new probability rate)  ...");
        System.out.println("");

        for(int i=0; i<kon.length; i++) {

            double rescalekon = kon[i] * 1660000.0;
            double lambdaOld = OnRateSolver.getrootIrreversible(p, R, D, rescalekon);
            double lambdaNew = getLambdaNew(p, R, D, rescalekon);
            double relativeError1 = Math.abs((lambdaOld - lambdaNew) / lambdaOld * 100.0);
            String adjustedRelativeLambdaError = adjust(relativeError1);
            relativeLambdaErrorCalculated[i] = adjustedRelativeLambdaError;
//            System.out.println(kon[i] + ": " + adjustedRelativeLambdaError + "%");

            // compute off rates the old way and the new way (with koff intrinsic)
            double offProbOld = koff*dt[3];
            double kOffIntrinsic = getKOffIntrinsic(R, D, koff, rescalekon);
            double offProbNew = 1.0 - Math.pow(Math.E, -(kOffIntrinsic*dt[3]));
            Assertions.assertFalse(Double.isNaN(offProbNew), "offProbNew is NaN: " + offProbNew);
            Assertions.assertFalse(Double.isInfinite(offProbNew), "offProbNew is infinite: " + offProbNew);

            double relativeError = Math.abs((offProbOld - offProbNew) / offProbOld * 100.0);
            if(i==3) {      // compute only one relative error for off probability rate based on credible numbers
                relativeOffProbabilityErrorCalculated = adjust(relativeError);
            }
//            System.out.println("  --- offProb relative error: " + relativeError + "%");


            // compute error over a range of dt, using the old vs new formula
//            for(int j=0; j<dt.length; j++) {
//                double onProbOld = lambdaOld * dt[j];
//                double onProbNew = 1.0 - Math.pow(Math.E, -(lambdaNew * dt[j]));
//                double relativeError2 = Math.abs((onProbOld - onProbNew) / onProbOld * 100);
//                System.out.print(dt[j] + ": " + adjust(relativeError2) + "%    ");
//            }
            System.out.println("");
//            System.out.println(" ------------------- ");
//            System.out.println("");
        }
        Assertions.assertTrue(Arrays.equals(relativeLambdaErrorExpected, relativeLambdaErrorCalculated));
        Assertions.assertTrue(relativeOffProbabilityErrorExpected.equals(relativeOffProbabilityErrorCalculated));
    }

    private static double getLambdaNew(double p, double R, double D, double rescalekon) {
        double volReact = 4.0 * Math.PI * (Math.pow(R, 3) - Math.pow(p, 3)) / 3.0;
        double kD = 4 * Math.PI * R * D;
        double kOnIntrinsic = getKOnIntrinsic(R, D, rescalekon);
        double lambdaNew = kOnIntrinsic / volReact;
        return lambdaNew;
    }

    private static double getKOnIntrinsic(double R, double D, double rescalekon) {
        double kD = 4 * Math.PI * R * D;
        double kOnIntrinsic = (rescalekon * kD) / (kD - rescalekon);    // can't be negative
        if (kOnIntrinsic <= 0.0) {
            // it will fail for kon = 50.0
            throw new RuntimeException("Kon is too large");
        }
        return kOnIntrinsic;
    }

    private static double getKOffIntrinsic(double R, double D, double koff, double rescalekon) {
        double kOnIntrinsic = getKOnIntrinsic(R, D, rescalekon);
        double kOffIntrinsic = koff * kOnIntrinsic / rescalekon;
        return kOffIntrinsic;
    }

    private static String adjust(double number) {
        DecimalFormat standardFormat = new DecimalFormat("0.00");
        DecimalFormat exponentialFormat = new DecimalFormat("0.00E0");

        int exponent = (int) Math.floor(Math.log10(Math.abs(number)));
        String formattedNumber;
        if (exponent < -2 || exponent > 3) {
            formattedNumber = exponentialFormat.format(number);
        } else {
            formattedNumber = standardFormat.format(number);
        }
        return(formattedNumber);
    }

}
