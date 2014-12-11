package com.jedox.etl.core.logging;

public interface ILogListener {
	
	public void addMessage(String type, Long timestamp, String message);
	public String getMessagesText();
	public void flush();
}
