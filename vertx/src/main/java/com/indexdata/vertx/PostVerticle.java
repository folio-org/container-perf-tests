package com.indexdata.vertx;
import io.vertx.core.AbstractVerticle;
import io.vertx.core.buffer.Buffer;
import io.vertx.core.http.HttpServer;
import io.vertx.core.http.HttpServerOptions;

public class PostVerticle extends AbstractVerticle {
  static class Counter {
    int x;
    public Counter(int x) {
      this.x = x;
    }
   }
  @Override
  public void start() throws Exception {
    HttpServerOptions o = new HttpServerOptions();
    o.setHandle100ContinueAutomatically(false);

    HttpServer h = vertx.createHttpServer(o);
    h.requestHandler(req -> {
      Buffer totalBuffer = Buffer.buffer();
      final Counter sz = new Counter(0);
      String s = req.getHeader("Content-Type");
      if ("100-continue".equals(req.getHeader("Expect"))) {
        req.response().writeContinue();
      }
      req.handler(buffer -> {
        sz.x += buffer.length();
      });
      req.endHandler(v -> {
        req.response().end("Got buffer of length " + sz.x + "\n");
      });
    });
    System.out.println("PostVerticle started\n");
    h.listen(8080);
  }
}