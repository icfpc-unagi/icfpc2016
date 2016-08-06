

import static java.lang.Math.*;
import static java.util.Arrays.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.*;
import javax.swing.*;

class Vis extends JFrame {
	int SIZE = 500;
	double minX = -100, maxX = 100, minY = -100, maxY = 100;
	BufferedImage img;
	Graphics2D g;
	boolean stop;
	Vis() {
		img = new BufferedImage(SIZE, SIZE, BufferedImage.TYPE_INT_RGB);
		g = (Graphics2D)img.getGraphics();
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		setRange(minX, minY, maxX, maxY);
		clear();
		JComponent c = new JComponent() {
			protected void paintComponent(Graphics g) {
				super.paintComponent(g);
				g.drawImage(img, 0, 0, null);
			}
		};
		c.setPreferredSize(new Dimension(SIZE, SIZE));
		c.addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent e) {
				stop = false;
			}
		});
		c.addMouseMotionListener(new MouseMotionAdapter() {
			public void mouseMoved(MouseEvent e) {
				double x = (double)e.getX() * (maxX - minX) / SIZE + minX;
				double y = -((double)e.getY() * (maxY - minY) / SIZE - maxY);
				setTitle(String.format("(%.2f, %.2f)", x, y));
			}
		});
		add(c);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
		pack();
		setVisible(true);
	}
	void setRange(double minX, double minY, double maxX, double maxY) {
		double sX = maxX - minX, sY = maxY - minY;
		double size = max(sX, sY);
		minX -= (size - sX) / 2;
		maxX += (size - sX) / 2;
		minY -= (size - sY) / 2;
		maxY += (size - sY) / 2;
		this.minX = minX;
		this.minY = minY;
		this.maxX = maxX;
		this.maxY = maxY;
		g.setTransform(new AffineTransform());
		g.scale(SIZE / size, SIZE / size);
		g.translate(-minX, maxY);
		g.transform(new AffineTransform(1, 0, 0, -1, 0, 0));
		g.setStroke(new BasicStroke((float)size / SIZE));
	}
	void vis(boolean stop) {
		repaint();
		this.stop = stop;
		while (this.stop) {
			sleep(10);
		}
	}
	void sleep(long time) {
		try {
			Thread.sleep(time);
		} catch (Exception e) {
		}
	}
	void clear() {
		int[] rgb = new int[SIZE * SIZE];
		fill(rgb, -1);
		img.setRGB(0, 0, SIZE, SIZE, rgb, 0, SIZE);
		g.setColor(Color.BLACK);
	}
	void drawAxis() {
		g.draw(line(0, 0, 1, 0));
		g.draw(line(0, 0, 0, 1));
	}
	Shape point(double x, double y) {
		double s = (maxX - minX) / SIZE * 2;
		Path2D path = new Path2D.Double();
		path.moveTo(x - s, y - s);
		path.lineTo(x + s, y + s);
		path.moveTo(x - s, y + s);
		path.lineTo(x + s, y - s);
		return path;
	}
	Shape segment(double x1, double y1, double x2, double y2) {
		return new Line2D.Double(x1, y1, x2, y2);
	}
	Shape line(double x1, double y1, double x2, double y2) {
		if (minX < x1 && x1 < maxX && minY < y1 && y1 < maxY) {
			double dx = x2 - x1, dy = y2 - y1;
			double d = max(min((maxX - x1) / dx, (minX - x1) / dx), min((maxY - y1) / dy, (minY - y1) / dy));
			x1 += dx * d;
			y1 += dy * d;
		}
		if (minX < x2 && x2 < maxX && minY < y2 && y2 < maxY) {
			double dx = x1 - x2, dy = y1 - y2;
			double d = max(min((maxX - x2) / dx, (minX - x2) / dx), min((maxY - y2) / dy, (minY - y2) / dy));
			x2 += dx * d;
			y2 += dy * d;
		}
		return new Line2D.Double(x1, y1, x2, y2);
	}
	Shape circle(double x, double y, double r) {
		return new Ellipse2D.Double(x - r, y - r, 2 * r, 2 * r);
	}
}
