package com.jedox.etl.core.util;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Proxy;
import java.net.ProxySelector;
import java.net.SocketAddress;
import java.net.URI;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.jedox.etl.core.logging.MessageHandler;

public class ProxyUtil extends ProxySelector {
	
	private static final Log log = new MessageHandler(LogFactory.getLog(ProxySelector.class));
	
	private interface ProxyWrapper {
		public Proxy toProxy();
	}
	
	 /*
     * Inner class representing a Proxy and a few extra data
     */
    private class InnerHttpProxy implements ProxyWrapper {
    		private Proxy proxy;
            private SocketAddress addr;
            // How many times did we fail to reach this proxy?
            private int failedCount = 0;
            
            public InnerHttpProxy(InetSocketAddress a) {
                    addr = a;
                    proxy = new Proxy(Proxy.Type.HTTP, a);
            }
            
            public SocketAddress address() {
                    return addr;
            }
            
            public Proxy toProxy() {
                    return proxy;
            }
            
            public int failed() {
                    return ++failedCount;
            }
    }
    
    private class InnerSocksProxy implements ProxyWrapper {
		private Proxy proxy;
        private SocketAddress addr;
        // How many times did we fail to reach this proxy?
        private int failedCount = 0;
        
        public InnerSocksProxy(InetSocketAddress a) {
                addr = a;
                proxy = new Proxy(Proxy.Type.SOCKS, a);
        }
        
        public SocketAddress address() {
                return addr;
        }
        
        public Proxy toProxy() {
                return proxy;
        }
        
        public int failed() {
                return ++failedCount;
        }
}
	
	private ProxySelector defsel = null;
	private HashMap<SocketAddress, InnerHttpProxy> httpproxies = new HashMap<SocketAddress, InnerHttpProxy>();
	private HashMap<SocketAddress, InnerSocksProxy> socksproxies = new HashMap<SocketAddress, InnerSocksProxy>();
    
	public ProxyUtil(ProxySelector def) {
            defsel = def;
    }
	
	public void addHttpProxy(String host,int port) {
		InnerHttpProxy p = new InnerHttpProxy(new InetSocketAddress(host, port));
		log.info("adding proxy "+host+" on port "+port+" to internal http proxies.");
		httpproxies.put(p.address(), p);
	}
	
	public void addSocksProxy(String host,int port) {
		InnerSocksProxy p = new InnerSocksProxy(new InetSocketAddress(host, port));
		socksproxies.put(p.address(), p);
	}
    
    public List<Proxy> select(URI uri) {
            if (uri == null) {
                    throw new IllegalArgumentException("URI can't be null.");
            }
            String protocol = uri.getScheme();
            if (!httpproxies.isEmpty() && ("http".equalsIgnoreCase(protocol) || "https".equalsIgnoreCase(protocol))) {
                    ArrayList<Proxy> l = new ArrayList<Proxy>();
                    for (InnerHttpProxy p : httpproxies.values()) {
                    	log.info("using http proxy "+p.address().toString()+" for URL "+uri.toString());
                        l.add(p.toProxy());
                      }
                    return l;
            }
            if (!socksproxies.isEmpty()) {
            	ArrayList<Proxy> l = new ArrayList<Proxy>();
                for (InnerSocksProxy p : socksproxies.values()) {
                	log.info("using socks proxy "+p.address().toString()+" for URL "+uri.toString());
                    l.add(p.toProxy());
                  }
                return l;
            }
            if (defsel != null) {
            		log.info("Using default system proxy for URL "+uri.toString());
                    return defsel.select(uri);
            } else {
                    ArrayList<Proxy> noproxies = new ArrayList<Proxy>();
                    noproxies.add(Proxy.NO_PROXY);
                    log.info("Using no proxy for URL "+uri.toString());
                    return noproxies;
            }
    }

    public void connectFailed(URI uri, SocketAddress sa, IOException ioe) {
        if (uri == null || sa == null || ioe == null) {
                throw new IllegalArgumentException("Arguments can't be null.");
        }
        log.warn("Proxy connection failed for "+uri.toString()+" using proxy "+sa.toString()+" with message:"+ioe.getMessage());
        String protocol = uri.getScheme();
        if ("http".equalsIgnoreCase(protocol) || "https".equalsIgnoreCase(protocol)) {
	        InnerHttpProxy p = httpproxies.get(sa); 
	        if (p != null) {
	                if (p.failed() >= 3) {
	                	httpproxies.remove(sa);
	                	log.info("Removing non functional proxy "+sa.toString()+" from http proxies.");
	                }
	                        
	        } else {
	                if (defsel != null) {
	                    defsel.connectFailed(uri, sa, ioe);
	                }
	        }
        }  
        else {
        	InnerSocksProxy p = socksproxies.get(sa); 
	        if (p != null) {
	                if (p.failed() >= 3) {
	                        socksproxies.remove(sa);
	                        log.info("Removing non functional proxy "+sa.toString()+" from socks proxies.");    
	                }
	        } else {
	                if (defsel != null)
	                        defsel.connectFailed(uri, sa, ioe);
	        }
        }
    }
}
