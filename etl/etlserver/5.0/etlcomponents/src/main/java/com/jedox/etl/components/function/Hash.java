package com.jedox.etl.components.function;

import java.security.MessageDigest;

import com.jedox.etl.core.component.ConfigurationException;
import com.jedox.etl.core.function.Function;
import com.jedox.etl.core.function.FunctionException;
import com.jedox.etl.core.node.IColumn;
import com.jedox.etl.core.node.Row;

public class Hash extends Function {

	static final String HEXES = "0123456789ABCDEF";
	
	private String getHex( byte [] raw ) {
	  if ( raw == null ) {
	    return null;
	  }
	  final StringBuilder hex = new StringBuilder( 2 * raw.length );
	  for ( final byte b : raw ) {
	    hex.append(HEXES.charAt((b & 0xF0) >> 4))
	       .append(HEXES.charAt((b & 0x0F)));
	  }
	  return hex.toString();
	}

	
	@Override
	protected Object transform (Row inputs) throws FunctionException {
		 StringBuffer buffer = new StringBuffer();
		 for (IColumn c : inputs.getColumns()) {
			 buffer.append(c.getValueAsString()+"|");
		 }
		 try {
			 MessageDigest digest = java.security.MessageDigest.getInstance("MD5");
			 digest.update(buffer.toString().getBytes());
			 byte[] hash = digest.digest();
			 return getHex(hash);
		 }
		 catch (Exception e) {
			 throw new FunctionException(e);
		 }
	}
	
	public void validateInputs() throws ConfigurationException {
		checkInputSize(1,true);
	}

}
