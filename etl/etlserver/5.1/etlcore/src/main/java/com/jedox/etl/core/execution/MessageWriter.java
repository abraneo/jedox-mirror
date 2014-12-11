package com.jedox.etl.core.execution;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.logging.ILogListener;

public class MessageWriter implements ILogListener {
	
	private static final Log log = LogFactory.getLog(MessageWriter.class);
	private static final int defaultBufferSize = 100;
	private List<Message> messages = Collections.synchronizedList(new LinkedList<Message>());
	private Long stateId;
	private int bufferSize = defaultBufferSize;
	
	public MessageWriter(Long stateId) {
		this.stateId = stateId;
	}
	
	public Long getStateId() {
		return stateId;
	}
	
	
	/**
	 * gets all Messages, each wrapping a log entry
	 * @return the list of messages
	 */
	public List<Message> getMessages() {
		return messages;
	}

	/**
	 * sets the messages, each wrapping a log entry
	 * @param messages the messages
	 */
	public void setMessages(List<Message> messages) {
		this.messages = messages;
	}
	
	public void flush() {
		if (bufferSize != Integer.MAX_VALUE && messages.size() > 0) synchronized(messages) { //only save and strip if persistence context is active.
			try {
				StateManager.getInstance().saveMessages(messages);
				messages.clear();
			}
			catch (Exception e) {
				log.error("Messages can not be persisted: "+e.getMessage());
			}
		}
	}
	
	/**
	 * adds a message wrapping a log entry
	 * @param type the type of the message (e.g. error, warning, ...)
	 * @param timestamp the timestamp of the message
	 * @param message the message text
	 * @throws RuntimeException 
	 */
	public void addMessage(String type, Long timestamp, String message) {
		synchronized(messages) {
			messages.add(new Message(getStateId(),type,timestamp, message));
			if (type.equals("ERROR") || messages.size() >= bufferSize) {
				flush();
			}
		}
	}

	public void setBufferSize(int bufferSize) {
		this.bufferSize = bufferSize;
	}

	public int getBufferSize() {
		return bufferSize;
	}

	@Override
	public String getMessagesText() {
		StringBuffer buffer = new StringBuffer();
		for (Message m : messages) {
			buffer.append(m.getMessage());
		}
		return buffer.toString();
	}

}
