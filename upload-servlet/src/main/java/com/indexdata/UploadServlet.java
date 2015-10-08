package com.indexdata;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class UploadServlet extends HttpServlet {
  private static final long serialVersionUID = 1L;

  @Override
    protected void doPost(final HttpServletRequest req,
        final HttpServletResponse res) throws ServletException, IOException {
      long skipped = 0;
      long count = 0;
      while ((skipped = req.getInputStream().skip(1024)) != 0) {
        count += skipped;
      }
      res.getWriter().write("Skipped over " + count);
    }
}
