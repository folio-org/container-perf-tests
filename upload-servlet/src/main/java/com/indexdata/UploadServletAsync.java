package com.indexdata;

import java.io.IOException;
import javax.servlet.AsyncContext;
import javax.servlet.ReadListener;

import javax.servlet.ServletException;
import javax.servlet.ServletInputStream;
import javax.servlet.ServletOutputStream;
import javax.servlet.WriteListener;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class UploadServletAsync extends HttpServlet {
  private static final long serialVersionUID = 1L;
  
  private class AsyncReadListener implements ReadListener {
    final AsyncContext context;
    byte[] buffer = new byte[65536];
    int count;

    public AsyncReadListener(AsyncContext context) {
      this.context = context;
    }
    
    @Override
    public void onDataAvailable() throws IOException {
      //System.out.println("onDataAvailable");
      ServletInputStream input = context.getRequest().getInputStream();
      int len;
      while (input.isReady() && (len = input.read(buffer)) != -1) {
        //System.out.println("Received "+len+" bytes");
        count += len;
      }
    }

    @Override
    public void onAllDataRead() throws IOException {
      System.out.println("onAllDataRead");
      context.getResponse().getOutputStream().setWriteListener(new AsyncWriteListener(context, count));
    }

    @Override
    public void onError(Throwable thrwbl) {
      System.out.println("There was an error: "+thrwbl.getMessage());
    }
    
  }
  
  private class AsyncWriteListener implements WriteListener {
    private final AsyncContext context;
    private final int count;

    public AsyncWriteListener(AsyncContext ctx, int count) {
      this.context = ctx;
      this.count = count;
    }

    @Override
    public void onWritePossible() throws IOException {
      ServletOutputStream output = context.getResponse().getOutputStream();
      if (output.isReady()) {
        output.print("Read "+count+" bytes");
        context.complete();
      }
    }

    @Override
    public void onError(Throwable thrwbl) {
      System.out.println("There was an error: "+thrwbl.getMessage());
    }
  }

  @Override
    protected void doPost(final HttpServletRequest req,
        final HttpServletResponse res) throws ServletException, IOException {
     AsyncContext ctx = req.startAsync();
     System.out.println("Async handling started");
     req.getInputStream().setReadListener(new AsyncReadListener(ctx));
    }
}
