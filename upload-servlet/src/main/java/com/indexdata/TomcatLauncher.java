package com.indexdata;

import java.io.File;
import org.apache.catalina.Context;
import org.apache.catalina.Wrapper;
import org.apache.catalina.connector.Connector;
import org.apache.catalina.startup.Tomcat;

public class TomcatLauncher {

  public static void main(String[] args) throws Exception {
    Tomcat tomcat = new Tomcat();
    //Connector c = new Connector("HTTP/1.1"); //default connector
    Connector c = new Connector(System.getProperty("connector", "org.apache.coyote.http11.Http11Nio2Protocol")); //force NIO2/epoll connector
    c.setPort(8080);
    tomcat.setConnector(c);
    tomcat.getService().addConnector(c); //stupid API
    tomcat.setPort(8080);
    File base = new File(System.getProperty("java.io.tmpdir"));
    Context rootCtx = tomcat.addContext("/app", base.getAbsolutePath());
    Tomcat.addServlet(rootCtx, "uploadServlet", new UploadServlet());
    rootCtx.addServletMapping("/upload", "uploadServlet");
    Wrapper async = Tomcat.addServlet(rootCtx, "uploadServletAsync", new UploadServletAsync());
    async.setAsyncSupported(true);
    rootCtx.addServletMapping("/upload-async", "uploadServletAsync");
    tomcat.start();
    tomcat.getServer().await();
  }
}
