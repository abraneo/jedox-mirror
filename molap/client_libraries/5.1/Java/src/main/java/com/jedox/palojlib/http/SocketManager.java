package com.jedox.palojlib.http;

import java.io.IOException;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import org.apache.log4j.Logger;
import com.jedox.palojlib.managers.HttpHandlerInfo;
import com.jedox.palojlib.managers.LoggerManager;

public class SocketManager {
	
	private HashMap<HttpHandlerInfo,HashMap<Long,Socket>> sockets = new HashMap<HttpHandlerInfo,HashMap<Long,Socket>>();
	private static Logger log = LoggerManager.getInstance().getLogger(SocketManager.class.getSimpleName());
	private boolean activated = true;
	private static SocketManager manager;
	final Lock lock = new ReentrantLock();
	
	private SocketManager(){}
	
	public static SocketManager getInstance(){
		if(manager == null)
			manager = new SocketManager();
		
		return manager;
	}
	
	public synchronized Socket getSocketConnection(HttpHandlerInfo info) throws UnknownHostException, IOException{
		
		HashMap<Long,Socket> socketThreadMap = sockets.get(info);

		if(socketThreadMap==null){
			socketThreadMap = new HashMap<Long, Socket>();
			sockets.put(info, socketThreadMap);
			return createSocket(socketThreadMap,info);
		}else{
			Socket threadSocket = socketThreadMap.get(Thread.currentThread().getId());
			if(threadSocket==null)
				return createSocket(socketThreadMap,info);
			else
				return threadSocket;
		}

	}

	/**
	 * @param info
	 * @throws IOException
	 * @throws UnknownHostException
	 * @throws SocketException
	 */
	private Socket createSocket(HashMap<Long, Socket> socketThreadMap,HttpHandlerInfo info) {

		
		try{
			lock.lock();
			if(info.useSsl){
				SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
				SSLSocket srvSslConnection = (SSLSocket) factory.createSocket(info.getHost(),info.sslPort);
				srvSslConnection.setSoTimeout(info.getTimeout());
				srvSslConnection.setReuseAddress(true);
				srvSslConnection.setSoLinger(true, 0);
				if(activated){
					socketThreadMap.put(Thread.currentThread().getId(), srvSslConnection);			
				}
				log.debug("Creating a new SSL Socket at host \"" + info.getHost() + "\" and port \"" + info.sslPort + "\", number of sockets for thread  "+ Thread.currentThread().getName() + " : " +  socketThreadMap.size());
				return srvSslConnection;
			}
			else{
				Socket srvConnection = new Socket(info.getHost(), Integer.parseInt(info.getPort()));
				srvConnection.setSoTimeout(info.getTimeout());
				srvConnection.setReuseAddress(true);
				srvConnection.setSoLinger(true, 0);
				if(activated){
					socketThreadMap.put(Thread.currentThread().getId(), srvConnection);
				}
				log.debug("Creating a new Socket at host \"" + info.getHost() + "\" and port \"" + info.getPort() + "\", number of sockets for thread  "+ Thread.currentThread().getName() + " : " +  socketThreadMap.size());
				return srvConnection;
			}		
		}catch(Exception e){
			throw new RuntimeException(e.getMessage());
		}finally{
			lock.unlock();
		}
	}
	
	public synchronized void removeSocketConnection(HttpHandlerInfo info, boolean allThreads){		
		try {
			lock.lock();
			HashMap<Long, Socket> map = sockets.get(info);
			if(map!=null){
				for(Long threadId: map.keySet()){
					if(allThreads || threadId.equals(Thread.currentThread().getId())){
						log.debug("Socket removed at host \"" + info.getHost() + "\" and port \"" + (info.useSsl?info.sslPort:info.getPort()) + "\", number of sockets for thread  "+ Thread.currentThread().getName() + " : " +  map.size());
						Socket socket = map.remove(threadId);
						//try to clean
						socket.getOutputStream().flush();
						socket.getOutputStream().close();
						socket.close();
					}
				}
			}
		} catch (Exception e) {
			log.debug("Error occurred while trying to clean the socket resources: " + e.getMessage());
			// do nothing
		}finally{
			if(allThreads)
				sockets.remove(info);
			lock.unlock();
		}
		
	}
	
	public void setActive(boolean activated){
		this.activated = activated;
	}
	
	public boolean isActive(){
		return activated;
	}
}

