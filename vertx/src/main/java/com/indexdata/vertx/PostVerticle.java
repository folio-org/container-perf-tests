package com.indexdata.vertx;
import io.vertx.core.AbstractVerticle;
import io.vertx.core.buffer.Buffer;
import io.vertx.core.http.HttpServer;

public class PostVerticle extends AbstractVerticle {
  static class Counter {
    int x;
    public Counter(int x) {
      this.x = x;
    }
   }
  @Override
  public void start() throws Exception {
    HttpServer h = vertx.createHttpServer();
    h.requestHandler(req -> {
      Buffer totalBuffer = Buffer.buffer();
      final Counter sz = new Counter(0);
      String s = req.getHeader("Content-Type");
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