
public class BB {
	
	public static double INF = Double.MAX_VALUE;
	public static double EPS = 1e-8;
	
	public double minX = INF, maxX = -INF, minY = INF, maxY = -INF;
	
	public boolean crs(BB b) {
		if (maxX + EPS <= b.minX) return false;
		if (b.maxX + EPS <= minX) return false;
		if (maxY + EPS <= b.minY) return false;
		if (b.maxY + EPS <= minY) return false;
		return true;
	}
	
	public void update(double x, double y) {
		if (minX > x) minX = x;
		if (maxX < x) maxX = x;
		if (minY > y) minY = y;
		if (maxY < y) maxY = y;
	}
	
}
