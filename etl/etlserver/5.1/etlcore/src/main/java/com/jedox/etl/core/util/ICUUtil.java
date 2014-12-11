/**
 * 
 */
package com.jedox.etl.core.util;

//import com.ibm.icu.text.Collator;

/**
 * @author khaddadin
 * uncomment this ifneeded and uncomment the needed dependency in POM
 *
 */
public class ICUUtil {
	
	private static ICUUtil instance = null;
	//static Collator myCollator = Collator.getInstance();
	
	private ICUUtil(){
	}
	
	public static ICUUtil getInstance(){
		if(instance==null){
			instance = new ICUUtil();
			//myCollator.setStrength(Collator.SECONDARY);
		}
		return instance;	
	}
	
	public boolean isEqual(String text1,String text2){
		
		//return myCollator.equals(text1, text2);
		return false;
	}

}
