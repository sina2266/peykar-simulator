/**
 * I don't want to use the Vector3D class in Java3D because almost all of its
 * operations are done "in place" and that makes the code look really strange.
 * I could download a vector library from the web, but it won't be too hard 
 * to write my own class.
 */

package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.IOHelp;

public class MyVector {

    /**
     * I usually don't like to use public fields, but they were used in the 
     * Vector3D class and it was nice to be able to write something like 
     * position.x instead of position.getX() each time I needed the coordinate. 
     */
    
    public double x;
    public double y;
    public double z;
    
    public MyVector(double x, double y, double z){
        this.x = x;
        this.y = y;
        this.z = z;
    }
    
    public MyVector(double [] position){
        this.x = position[0];
        this.y = position[1];
        this.z = position[2];
    }
    
    public MyVector(){
        x = 0;
        y = 0;
        z = 0;
    }
    
    // toString() will just print out as row vector
    @Override
    public String toString(){
        return "{" + IOHelp.DF[3].format(x) + ", " + IOHelp.DF[3].format(y) + ", " + IOHelp.DF[3].format(z) + "}"; 
    }
    
    // SOME USEFUL VECTOR METHODS.
    
    public double dot(MyVector v){
        return x*v.x + y*v.y + z*v.z; 
    }
    
    public MyVector add(MyVector v){
        return new MyVector(x+v.x, y+v.y, z+v.z);
    }
    
    public MyVector subtract(MyVector v){
        return new MyVector(x-v.x, y-v.y, z-v.z);
    }
    
    public MyVector multiply(double a){
        return new MyVector(a*x, a*y, a*z);
    }
    
    public void scale(double a){
        x = a*x;
        y = a*y;
        z = a*z;
    }
    
    public void translate(MyVector v){
        x += v.x;
        y += v.y;
        z += v.z;
    }
    
    public void translate(double dx, double dy, double dz){
        x += dx;
        y += dy;
        z += dz;
    }
    
    public double length2(){
        return x*x + y*y + z*z;
    }
    
    public double length(){
        return Math.sqrt(x*x + y*y + z*z);
    }
    
    public MyVector unitVector(){
        double len = length();
        return new MyVector(x/len, y/len, z/len);
    }
    
    
}
