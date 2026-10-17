/*
 * This class contains the methods I need to take the macroscopic on-rate
 * for a bimolecular reaction and convert it to a first-order rate used to
 * determine if two reactive particles within a reaction radius will react
 * during one time step. 
 */
package edu.uchc.cam.langevin.helpernovis;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class OnRateSolver {

    public static final Logger lg = LogManager.getLogger(OnRateSolver.class);
    /*
     *                Irreversible Reaction
     * The function f gives the macroscopic on rate in terms of the physical 
     * radius, p, the reaction radius, R, the diffusion constant, D, and 
     * a constant with dimensions of length, r0, which is related to the 
     * first-order reaction rate, lambda, through r0 = sqrt(D/lambda).
     */
    public static double f(double p, double R, double D, double r0){
 
        double alpha = (R-p)/r0;
        double cosh = Math.cosh(alpha);
        double sinh = Math.sinh(alpha);
        
        return 4.0*Math.PI*R*D * (1 - (r0/R)*( (p*cosh + r0*sinh) / (p*sinh + r0*cosh) ));
    }
    
    /* 
     *                 Reversible Reaction
     * The function g gives the macroscopic on rate in terms of the physical 
     * parameters of the system.  These are the same parameters in the 
     * irreversible case, plus an additional parameter a which is the 
     * dissociation radius.  
     * Since the form of g depends on whether a is greater than or less than R,
     * I'll actually defined two functions.  gin will apply when a < R, while
     * gout will apply when a > R. 
     */
    
    public static double gin(double p, double a, double R, double D, double r0){
        double A = (R-p)/r0;
        double B = (a-p)/r0;
        double coshA = Math.cosh(A);
        double coshB = Math.cosh(B);
        double sinhA = Math.sinh(A);
        double sinhB = Math.sinh(B);
        
        return 4.0*Math.PI * D * (a/r0) * (r0*(R-p)*coshA + (p*R-r0*r0)*sinhA)/(r0*sinhB + p*coshB);
    }
    
    public static double gout(double p, double a, double R, double D, double r0){
        double A = (R-p)/r0;
        double coshA = Math.cosh(A);
        double sinhA = Math.sinh(A);
        double phi = 1 - (r0/R)*(r0*sinhA + p*coshA)/(r0*coshA + p*sinhA);
        
        return 4.0*Math.PI*R*D*phi/(1-(R/a)*phi);
    }
    
    public static double g(double p, double a, double R, double D, double r0){
        if(a <= R){
            return gin(p,a,R,D,r0);
        } else {
            return gout(p,a,R,D,r0);
        }
    }
    
    /*
     *                Irreversible Reaction
     * I needed the derivative of f with respect to r0 in order to use 
     * Newton's method to solve for r0.  Even though I no longer use Newton's 
     * method, there's no point in erasing these lines.  They could come in 
     * useful in the future. 
     */
    public static double fprime(double p, double R, double D, double r0){

        double alpha = (R-p)/r0;
        double cosh = Math.cosh(alpha);
        double sinh = Math.sinh(alpha);
        
        return (4.0*Math.PI*D / Math.pow((p*sinh + r0*cosh),2)) * ( alpha*(r0*r0 - p*p)*cosh*cosh - (p*p + r0*r0)*sinh*cosh + (R*(p*p-r0*r0)/r0 - p*(p*p + r0*r0)/r0)*sinh*sinh );
    }
    
    /*
     *                Irreversible Reaction
     * I will use a variation of the secant method to determine r0 given kon. 
     * Newton's method frequently didn't converge because f is very flat for 
     * large r0 but has a huge derivative right near the zero of f-kon. Because
     * of this, the conventional secant method usually fails too.  Here I use 
     * a modified secant method.  Define g(r) = f(r) -kon. I choose two initial
     * points, r_left and r_right, such that g(r_left) > 0 and g(r_right) < 0 
     * (so I know the zero point lies between r_left and r_right). I then use 
     * the usual secant step to get
     * r_next = (r_left*g(r_right) - r_right*g(r_left))/(g(r_right) - g(r_left)).
     * I then check whether g(r_next) is either > 0 or <0,and assign it to either
     * r_left or r_right accordingly.  Usually only one of the two gets updated
     * during the entire iterative process, so to cut off the process we just
     * check the size of abs(g(r_next)) instead of the difference between
     * r_left and r_right. This method does not converge nearly as fast as 
     * Newton's method, but as I only call this function a few times in the very
     * beginning, speed isn't really an issue here.  More importantly, it's 
     * a very stable method for this function, which kind of looks like a 
     * Gaussian. 
     * 
     * THE METHOD INTERNALLY SOLVES FOR THE CORRECT r0, BUT IT THEN RETURNS
     * THE CORRECT lambda USING lambda = D/r0^2.
     * 
     * 
     */
    // This is the old version of the method, which is not used anymore.  It is kept here for reference.
    // We are now using Boris's method which is better.
    @Deprecated
    public static double getrootIrreversible(double p, double R, double D, double kon){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        // Make a guess for r_left, and then adjust it so that f(r_left)-kon>0.
        double r_left = 0.01;
        double g_left = f(p,R,D,r_left)-kon;
        while(g_left < 0){
            r_left = r_left/2.0;
            g_left = f(p,R,D,r_left)-kon;
        }
        
        // Make a guess for r_right, and adjust it if we need to.
        double r_right = 5.0;
        double g_right = f(p,R,D,r_right)-kon;
        while(g_right>0){
            r_right = 2*r_right;
            g_right = f(p,R,D,r_right)-kon;
        }
        
        double g_min = 100;
        double r_min = 100;
        
        if(g_left > -g_right){
            g_min = -g_right;
            r_min = r_right;
        } else {
            g_min = g_left;
            r_min = r_left;
        }
        
        int count = 0;
        double r_next;
        double g_next;
        while(g_min > 0.000001){
           
            r_next = (r_left*g_right - r_right*g_left)/(g_right - g_left);
            g_next = f(p,R,D,r_next) - kon;
            
            if(g_next > 0){
                r_left = r_next;
                g_left = g_next;
            } else {
                r_right = r_next;
                g_right = g_next;
            }
            
            if(g_left > -g_right){
                g_min = -g_right;
                r_min = r_right;
            } else {
                g_min = g_left;
                r_min = r_left;
            }
            
            count++;
            if(count > 10000000) {
                // we don't throw because the lambda computed this way is not used. We just want to discover what
                // would have gone wrong (and why) if we had used this method.
                // The new method is more robust and is being used instead.
                lg.debug("getrootIrreversible: finding a root using OnRateSolver.getroot() failed to converge.");
                lg.debug("   p: " + p + ",  R: " + R + ",  D: " + D + ",  kon: " + kon + ",  g_min: " + g_min);
                break;
            }
        }
        
        return D/(r_min*r_min);
        // </editor-fold>
    }

    @Deprecated
    public static double getrootReversible(double p, double a, double R, double D, double kon){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        // Make a guess for r_left, and then adjust it so that g(r_left)-kon>0.
        double r_left = 10;
        double g_left = g(p,a,R,D,r_left)-kon;
        while(g_left < 0){
            r_left = r_left/1.1;
            g_left = g(p,a,R,D,r_left)-kon;
        }
        
        // Make a guess for r_right, and adjust it if we need to.
        double r_right = 0.1;
        double g_right = g(p,a,R,D,r_right)-kon;
        while(g_right>0){
            r_right = 1.1*r_right;
            g_right = g(p,a,R,D,r_right)-kon;
        }
        
        double g_min = 100;
        double r_min = 100;
        
        if(g_left > -g_right){
            g_min = -g_right;
            r_min = r_right;
        } else {
            g_min = g_left;
            r_min = r_left;
        }
        
        int count = 0;
        double r_next;
        double g_next;
        while(g_min > 0.0001){
           
            r_next = (r_left*g_right - r_right*g_left)/(g_right - g_left);
            g_next = g(p,a,R,D,r_next) - kon;
            
            if(g_next > 0){
                r_left = r_next;
                g_left = g_next;
            } else {
                r_right = r_next;
                g_right = g_next;
            }
            
            if(g_left > -g_right){
                g_min = -g_right;
                r_min = r_right;
            } else {
                g_min = g_left;
                r_min = r_left;
            }
            
            count++;
            
            if(count > 10000000){
                // we don't throw because the lambda computed this way is not used. We just want to discover what
                // would have gone wrong (and why) if we had used this method.
                // The new method is more robust and is being used instead.
                lg.debug("getrootReversible: finding a root using OnRateSolver.getroot() failed to converge.");
                lg.debug("  p: " + p + ",  a: " + a + ",  R: " + R + ",  D: " + D + ",  kon: " + kon  + ",  g_min: " + g_min);
                break;
            }
        }
        
        return D/(r_min*r_min);
        // </editor-fold>
    }
    
    /*
     * There are several inequalities which must be satisfied in order for 
     * my program to accurately simulate the physics I want to reproduce. This
     * function will check these inequalities and tell me if they are satisfied.
     * 
     * 1) The maximum possible on-rate is given by kon,max = 4*pi*R*D, so we'd
     * better have kon < 4*pi*R*D . If that isn't satisfied then nothing else
     * will work. 
     * 
     * 2) For testing purposes, I'd like to stay in the dilue concentration
     * limit.  That means that the volume occupied by the particles (really
     * the full volume contained in their reaction radii) should be much
     * less than the volume of the system.  The cutoff is somewhat arbitrary, 
     * but a safe threshold is probably V_particles < 0.0001 * V_system. 
     * 
     * 3) The root-mean-squared step distance should be much less than the 
     * smallest distance in the problem, either p or R-p.  
     * Noting rms = sqrt(2*D*dt), we would like 2*D*dt << (R-p)^2 . Exactly
     * how much less is not clear yet.  I thought I'd need 8*rms < (R-p), but 
     * that seems too restrictive. 
     * 
     * 4) Lastly, we'd better make sure that the probabilty of reacting during 
     * a given time step is much less than 1, that is, lambda*dt << 1.  
     * 
     * This method will take in p and R in nanometers, D in um^2/s, kon in
     * uM-1.s-1, and dt in seconds, and will check the status of each of these
     * inequalities.  We'll also need the particle number and system volume.
     *
     * Used only in @Tests
     * TODO: to make this really useful we should also test validity of lambda calculated the new way (Boris)
     *  TODO: we should move this to the client and raise issues if the inequalities are not satisfied.
     */
    public static void checkInequalities(double p, double R, double D, double kon, double dt){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        double kon_scale = 1660000.0 * kon;
        double D_scale = D * 1000000.0;
        
        double rhs1 = 4.0*Math.PI*R*D_scale;
        
        boolean check1 = (kon_scale < rhs1);
        
        lg.info("kon = " + kon_scale + ", 4*pi*R*D = " + rhs1 + "; kon < 4*pi*R*D is " + check1);
        
        if(!check1){
            lg.info("Not going to bother with other checks.");
            lg.info("");
        } else{
            lg.info("");
            double rms = Math.sqrt(2*D_scale*dt);
            lg.info("rms = " + rms + ". ");
            double rhs2;
            if(R-p < p){
                rhs2 = R-p;
                lg.info("Smallest distance scale is R-p = " + rhs2 + ". ");
            } else {
                rhs2 = p;
                lg.info("Smallest distance scale is p = " + rhs2 + ". ");
            }
            boolean check2 = (rms < rhs2);
            lg.info("rms < dist_min is " + check2 + ".  Ratio:" + rms/rhs2);
            lg.info("");
            double lambda = getrootIrreversible(p,R,D_scale,kon_scale);
            lg.info("lambda = " + lambda + ". lambda*dt = " + lambda*dt);
        }
        lg.info("");
        lg.info("***********");
        lg.info("");
        // </editor-fold>
    }
            
}
