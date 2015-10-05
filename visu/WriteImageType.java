import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.GradientPaint;
import java.awt.Graphics2D;
import java.awt.geom.Ellipse2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.*;

import javax.imageio.ImageIO;


public class WriteImageType { 
  static public void main(String args[]) throws Exception {
    try {

      int nb_pix = 1;

      StringBuilder image = new StringBuilder();
      int read,N=1024*8;
      char [] buffer = new char[N];
      FileReader fr = new FileReader(args[0]);
      BufferedReader br = new BufferedReader(fr);
      while(true){
        read = br.read(buffer,0,N);
        image.append(buffer,0,read);
        if(read<N)break;
      }
      br.close();
      String im = image.toString();
      String[] Data = im.split("\n");
      int height = Data.length-3;
      // System.out.println(height);
      int width = Data[2].length()-2;
      // System.out.println(width);

      // int zero = 0xFFFFFFFF;
      // int empty = WriteImageType.getIntFromColor(0,0,0);
      // int trois =WriteImageType.getIntFromColor(0,0,255);
      // int deux = WriteImageType.getIntFromColor(0,0,170);
      // int un = WriteImageType.getIntFromColor(0,0,85);




      // int width = 200, height = 200;

      // TYPE_INT_ARGB specifies the image format: 8-bit RGBA packed
      // into integer pixels
      BufferedImage bi = new BufferedImage(width*nb_pix, height*nb_pix, BufferedImage.TYPE_INT_ARGB);

      for(int j = 0; j < height; j++){
        for(int i = 0; i < width; i++){

                    // System.out.println(i);
          int rgb = 0;
          switch(Data[j+2].charAt(i+1)){
            case ' ':
              rgb =0x00FFFFFF;
              break;
            case '.':
              rgb = 0xFFFFFFFF;
              break;
            case '1':
              rgb = 0xFF000058;
              break;
            case '2':
              rgb = 0xFF0000A8;
              break;
            case '3':
              rgb = 0xFF0000FF;
              break;
            default:
              rgb = 0x00FFFFFF;
              break;
          }
          for(int p1 = 0; p1 < nb_pix; p1++){
            for(int p2 = 0; p2 < nb_pix; p2++){
              bi.setRGB(i*nb_pix+p1,j*nb_pix+p2,rgb);
            }

          }

        }

      }
      // Graphics2D ig2 = bi.createGraphics();
      //
      //
      // Font font = new Font("TimesRoman", Font.BOLD, 20);
      // ig2.setFont(font);
      // String message = "www.java2s.com!";
      // FontMetrics fontMetrics = ig2.getFontMetrics();
      // int stringWidth = fontMetrics.stringWidth(message);
      // int stringHeight = fontMetrics.getAscent();
      // ig2.setPaint(Color.black);
      // ig2.drawString(message, (width - stringWidth) / 2, height / 2 + stringHeight / 4);
      //
      ImageIO.write(bi, "PNG", new File(args[1]+".png"));
      // ImageIO.write(bi, "JPEG", new File("test.JPG"));
      // ImageIO.write(bi, "gif", new File("c:\\yourImageName.GIF"));
      // ImageIO.write(bi, "BMP", new File("c:\\yourImageName.BMP"));

    } catch (IOException ie) {
      ie.printStackTrace();
    }

  }
}
