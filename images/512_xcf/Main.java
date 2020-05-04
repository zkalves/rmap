import java.awt.AWTException;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.HeadlessException;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Robot;
//from   ww  w .  j  av  a 2 s.c  om
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

public class Main {
  public static void main(String[] args) {
    JFrame frame = new JFrame();
    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    frame.setLayout(new BorderLayout());
    frame.add(new TestPane());
    frame.pack();
    frame.setVisible(true);
  }
}

class TestPane extends JPanel {
  public TestPane() {
    Thread thread = new Thread(new BackgroundMonitor());
    thread.setDaemon(true);
    thread.start();
  }
  @Override
  public Dimension getPreferredSize() {
    return new Dimension(200, 200);
  }
  @Override
  protected void paintComponent(Graphics g) {
    super.paintComponent(g);
    Color color = getBackground();
    String text = color.getRed() + "x" + color.getGreen() + "x"
        + color.getBlue();

    g.drawString(text, 20, 30);
  }

  public class BackgroundMonitor implements Runnable {

    @Override
    public void run() {
      try {
        Robot robot = new Robot();

        Color previous = null;
        while (true) {
          Point coord = MouseInfo.getPointerInfo().getLocation();
          Color color = robot.getPixelColor((int) coord.getX(),
              (int) coord.getY());
          if (previous == null || !previous.equals(color)) {
            SwingUtilities.invokeLater(new UpdateBackgroud(color));
          }
          try {
            Thread.sleep(250);
          } catch (InterruptedException ex) {
          }
        }
      } catch (AWTException | HeadlessException exp) {
        exp.printStackTrace();
      }
    }
  }

  class UpdateBackgroud implements Runnable {

    private Color color;

    public UpdateBackgroud(Color color) {
      this.color = color;
    }

    @Override
    public void run() {
      setBackground(color);
    }

  }
}
