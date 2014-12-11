package com.jedox.etl.core.util;
import javax.mail.Authenticator;
import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.PasswordAuthentication;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import com.jedox.etl.core.component.RuntimeException;
import com.sun.mail.util.MailSSLSocketFactory;

public class MailUtil {
	private String SMTP_HOST_NAME;
	private String SMTP_AUTH_USER;
	private String SMTP_AUTH_PWD;

	private String emailMsgTxt;
	private String emailSubjectTxt;
	private String emailFromAddress;
	private boolean debug = false;
	private boolean tls = true;
	private boolean trustAllServers = true;

	/* Add List of Email address to who email needs to be sent to */
	private List<String> emailList = new ArrayList<String>();
	
	private static final Log log = LogFactory.getLog(MailUtil.class);

	/*
	  To use this program, change values for the following three constants,

	    SMTP_HOST_NAME -- Has your SMTP Host Name
	    SMTP_AUTH_USER -- Has your SMTP Authentication UserName
	    SMTP_AUTH_PWD -- Has your SMTP Authentication Password

	  Next change values for fields

	  emailMsgTxt -- Message Text for the Email
	  emailSubjectTxt -- Subject for email
	  emailFromAddress -- Email Address whose name will appears as "from" address

	  Next change value for "emailList".
	  This String array has List of all Email Addresses to Email Email needs to be sent to.

	 */
	
	/**
	 * SimpleAuthenticator is used to do simple authentication
	 * when the SMTP server requires it.
	 */
	private class SMTPAuthenticator extends javax.mail.Authenticator
	{

		public PasswordAuthentication getPasswordAuthentication()
		{
			String username = SMTP_AUTH_USER;
			String password = SMTP_AUTH_PWD;
			return new PasswordAuthentication(username, password);
		}
	}

	private void postMail( List<String> recipients, String subject, String message , String from) throws MessagingException
	{
		
		/*Set the host smtp address*/
		Properties props = new Properties();
		props.put("mail.smtp.host", SMTP_HOST_NAME);
		props.put("mail.smtp.port", 587);
		props.put("mail.smtp.auth", "true");
		props.put("mail.smtp.starttls.enable",String.valueOf(tls));
		if (trustAllServers) {
			props.put("mail.smtp.socketFactory.fallback", "true");
			try {
				MailSSLSocketFactory sf = new MailSSLSocketFactory();
				sf.setTrustAllHosts(true);
				props.put("mail.smtp.ssl.socketFactory", sf);
			}
			catch (Exception e) {
				log.warn("Problem with smtp socket factory: "+e.getMessage());
			}
		}

		Authenticator auth = new SMTPAuthenticator();
		Session session = Session.getDefaultInstance(props, auth);

		session.setDebug(debug);

		/* create a message */
		Message msg = new MimeMessage(session);

		/* set the from and to address */
		InternetAddress addressFrom = new InternetAddress(from);
		msg.setFrom(addressFrom);

		InternetAddress[] addressTo = new InternetAddress[recipients.size()];
		for (int i = 0; i < recipients.size(); i++)
		{
			addressTo[i] = new InternetAddress(recipients.get(i));
		};
		msg.setRecipients(Message.RecipientType.TO, addressTo);


		/* Setting the Subject and Content Type */
		msg.setSubject(subject);
		msg.setContent(message, "text/plain");
		Transport.send(msg);
	}
	
	public void setServer(String host, String user, String password) {
		SMTP_HOST_NAME = host;
		SMTP_AUTH_USER = user;
		SMTP_AUTH_PWD = password;
		if (emailFromAddress == null)
			emailFromAddress = user;
	}
	
	public void setSender(String sender) {
		emailFromAddress = sender;
	}
	
	public void addRecipient(String recepient) {
		emailList.add(recepient);
	}
	
	public void setSubject(String subject) {
		emailSubjectTxt = subject;
	}
	
	public void setMessage(String message) {
		emailMsgTxt = message;
	}
	
	public void enableDebug(boolean debug) {
		this.debug = debug;
	}
	
	public void enableTLS(boolean tls) {
		this.tls = tls;
	}
	
	public void trustAllServers(boolean trustAllServers) {
		this.trustAllServers = trustAllServers;
	}
	
			
	public void send() throws RuntimeException {
   		try {
			postMail( emailList, emailSubjectTxt, emailMsgTxt, emailFromAddress);
		} catch (MessagingException e) {
			throw new RuntimeException("Unable to send mail: "+e.getMessage());
		}
		log.info("Successfully sent mail.");
	}

}
