/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.helpernovis;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;

/**
 *
 * @author pmichalski
 */
public class FileHelp {
    
    /**
     * Sometime I just want to wait for a folder to be created, but don't 
     * care about any files in that folder.
     * 
     * @param path The absolute path of the folder we're waiting for.
     */
    
//    public static void waitForFolder(Path path){
//        long startTime;
//        long oldTime;
//        long currentTime;
//        if(!Files.exists(path)){
//            System.out.println("Waiting for the folder " + path.toString() + " to be created.");
//            startTime = System.currentTimeMillis();
//            oldTime = startTime;
//            while(true){
//                if(Files.exists(path)){
//                    System.out.println("Found the folder " + path.toString() + ".");
//                    break;
//                }
//                currentTime = System.currentTimeMillis();
//                if(currentTime - oldTime > 60*1000){
//                    System.out.println("Still waiting for folder.  Waited for " + (currentTime - startTime)/(60*1000) + " minutes.");
//                    oldTime = currentTime;
//                }
//            }
//        }
//    }
    
    /**
     * In several of the data classes I have to wait for a file to be 
     * created.
     * @param path  This is the absolute path to the directory of the file
     *              we are looking for.  We will check to make sure that the 
     *              folder holding the file has also been created.
     * @param filename This is the name of the file we're waiting for.
     * 
     * @return Returns the file specified by the given path and filename.
     */

//    public static File waitForFile(Path path, String filename){
//        // First look to see if the path exists
//        long startTime;
//        long oldTime;
//        long currentTime;
//        if(!Files.exists(path)){
//            System.out.println("Waiting for the folder " + path.toString() + " to be created.");
//            startTime = System.currentTimeMillis();
//            oldTime = startTime;
//            while(true){
//                if(Files.exists(path)){
//                    System.out.println("Found the folder.");
//                    break;
//                }
//                currentTime = System.currentTimeMillis();
//                if(currentTime - oldTime > 60*1000){
//                    System.out.println("Still waiting for folder.  Waited for " + (currentTime - startTime)/(60*1000) + " minutes.");
//                    oldTime = currentTime;
//                }
//            }
//        }
//        
//        if(!Files.exists(path.resolve(filename))){
//            System.out.println("Waiting for the file " + filename + " to be created in folder " + path.toString());
//            startTime = System.currentTimeMillis();
//            oldTime = startTime;
//            while(true){
//                if(Files.exists(path.resolve(filename))){
//                    System.out.println("Found the file.");
//                    System.out.println();
//                    break;
//                }
//                currentTime = System.currentTimeMillis();
//                if(currentTime - oldTime > 60*1000){
//                    System.out.println("Still waiting for file.  Waited for " + (currentTime - startTime)/(60*1000) + " minutes.");
//                    oldTime = currentTime;
//                }
//            }
//        }
//        
//        return path.resolve(filename).toFile();
//    }
    
    /**
     * A method to delete all of the files in a given folder which 
     * begin with the given string.  
     * 
     * @param folder The folder where we want to delete files.
     * @param startString A substring giving the initial characters in the files
     *                    we want to delete.
     */
    
//    public static void deleteAllFilesStartWith(File folder, String startString){
//        String fname = null;
//        try{
//            String [] filename = folder.list();
//            for(int i=0;i<filename.length;i++){
//                File f = new File(folder, filename[i]);
//                fname = f.getName();
//                if(fname.startsWith(startString)){
//                    Files.delete(f.toPath());
//                }
//            }
//        } catch(NoSuchFileException fne){
//            System.out.println("Could not find a file while deleting partial data files.  Filename " + fname);
//        } catch(IOException ie){
//            ie.printStackTrace(System.out);
//        }
//    }
    
    /**
     * 
     * I DON'T NEED THIS METHOD ANY MORE. Now that I actually write a file
     * to indicate that the previous run is finished, I know that no other 
     * programs will be trying to read or write the files that the current
     * program is deleting.
     * 
     * I found that I was trying to delete files too quickly.  Basically
     * I'd wait for the file to be created and then I'd try to delete it, but
     * I never gave the other process time to finish writing the file.  Thus,
     * the delete method would fail and leave the file behind.  Now I wait some
     * additional time before deleting the file.  This is a crude way of doing 
     * this because it assumes that the file is written in less time than we 
     * wait, but I'll wait a really really long time (in computer terms), say, 
     * 15 seconds, which is long enough to write any of the data files 
     * I'll be deleting.
     * @param path
     * @throws IOException 
     */
    
//    public static void safelyDelete(Path path) throws IOException {
//        System.out.println("Waiting 15 seconds to delete " + path.toString() + " .");
//        System.out.println();
//        long starttime = System.currentTimeMillis();
//        long currenttime;
//        while(true){
//            currenttime = System.currentTimeMillis();
//            if(currenttime - starttime > 15000){
//                break;
//            }
//        }
//        
//        Files.delete(path);
//        
//    }
    
}
