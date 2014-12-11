package com.jedox.etl.core.util;
import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import javax.mail.Authenticator;
import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.Multipart;
import javax.mail.PasswordAuthentication;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeBodyPart;
import javax.mail.internet.MimeMessage;
import javax.mail.internet.MimeMultipart;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;


import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import com.jedox.etl.core.component.RuntimeException;
import com.jedox.etl.core.config.Settings;
import com.jedox.palojlib.interfaces.IConnectionConfiguration;
import com.jedox.palojlib.interfaces.IDatabase;
import com.jedox.palojlib.interfaces.IDimension;
import com.jedox.palojlib.interfaces.IElement;
import com.jedox.palojlib.premium.interfaces.IConnection;
import com.jedox.palojlib.premium.main.ConnectionManager;
import com.sun.mail.util.MailSSLSocketFactory;

public class MailUtil {
	private String SMTP_HOST_NAME;
	private String SMTP_PORT="587";	
	private String SMTP_AUTH_USER;
	private String SMTP_AUTH_PWD;

	private String emailMsgTxt;
	private String emailSubjectTxt;
	private String emailFromAddress;
	private boolean debug = false;
	private boolean tls = true;
	private boolean auth = true;
	private boolean trustAllServers = true;

	/* Add List of Email address to who email needs to be sent to */
	private List<String> emailList = new ArrayList<String>();
	private List<File> attachmentsList = new ArrayList<File>();
	
	private Properties mailProps = null;
	
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
		props.put("mail.smtp.port", SMTP_PORT);
		props.put("mail.smtp.auth", auth);
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
		// Create - Text Part
		MimeBodyPart messageBodyPart = new MimeBodyPart();
		messageBodyPart.setText(emailMsgTxt,"UTF-8");

		// Create - Multi Part
		Multipart multipart = new MimeMultipart();
		multipart.addBodyPart(messageBodyPart);

		// PDF Attachment
		for(int i=0;i<attachmentsList.size();i++){
			messageBodyPart = new MimeBodyPart();
			DataSource source = new FileDataSource(attachmentsList.get(i));
			messageBodyPart.setDataHandler(new DataHandler(source));
			//System.setProperty("mail.mime.encodefilename", "true");
			messageBodyPart.setFileName(attachmentsList.get(i).getName());
			multipart.addBodyPart(messageBodyPart);
		}

		// Put parts in message and send
		msg.setContent(multipart);
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
	
	public void addAttachment(String filename) throws Exception{
		File file = new File(getFilePath(filename));
		if(!file.exists()){
			throw new Exception("File " + filename + " does not exist and therefor can be attached.");
		}
		attachmentsList.add(file);
	}
	
	protected String getDataDir() {
		return Settings.getInstance().getDataDir();
	}

	public String getFilePath(String filename) {
		if (FileUtil.isRelativ(filename)) {
			String dir = getDataDir();
			filename = dir + File.separator + filename;
			filename = filename.replace("/", File.separator);
			filename = filename.replace("\\", File.separator);
		}
		return filename;
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
	
	public void enableAuth(boolean auth) {
		this.auth = auth;
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
	
	private Properties getMailSettingsInConfig() {
		IConnection palosuiteConnection=null;
		try{
			IConnectionConfiguration connectionConfiguration = Settings.getInstance().getSuiteConnectionConfiguration();
			palosuiteConnection = ConnectionManager.getInstance().getConnection(connectionConfiguration);
			palosuiteConnection.openInternal();
			
			IDatabase configdb = palosuiteConnection.getDatabaseByName("Config");
			if(configdb==null){
				throw new Exception("Config database does not exist.");
			}

			IDimension configdim = configdb.getDimensionByName("config");
			Properties props = new Properties();
			IElement[] configElements = configdim.getElements(true);
			for(IElement e:configElements) {
				if(e.getName().startsWith("scheduler")) {
					String key = e.getName().replaceAll("_", "\\.").replace("scheduler", "mail");
					String value = e.getAttributeValue("value").toString();
					if (key.contains("password"))
						value=Settings.getInstance().decrypt(value);
					props.setProperty(key, value);
				}
			}

			palosuiteConnection.close();

			return props;
		}catch(Exception e){
			log.error("An error occurred while trying to read the Mail settings:" + e.getMessage());
			return null;
		}finally{
			if(palosuiteConnection!= null && palosuiteConnection.isConnected()){
				palosuiteConnection.close();
			}
		}
	}
	
	private Boolean parseBoolean (String s) {
		if (s.equals("0"))
			return false;
		if (s.equals("1"))
			return true;
		log.error("Invalid input "+s+" for boolean Mail parameter. Only 0 (false) and 1 (true) allowed.");
		return null;
	}
	
	/* Use the SMTP Settings from central Config database */ 
	public void setServer() {
		if (mailProps == null) {
			mailProps=getMailSettingsInConfig();
		}
		SMTP_HOST_NAME = mailProps.getProperty("mail.smtp.host");
		SMTP_PORT = mailProps.getProperty("mail.smtp.port");		
		SMTP_AUTH_USER = mailProps.getProperty("mail.smtp.user");
		SMTP_AUTH_PWD = mailProps.getProperty("mail.smtp.password");	
		emailFromAddress = SMTP_AUTH_USER;
				
		enableTLS(parseBoolean(mailProps.getProperty("mail.smtp.starttls.enable")));		
		enableAuth(parseBoolean(mailProps.getProperty("mail.smtp.auth")));		
	}
	
}
