
public class SATBorder {
	
	public int p1, p2;
	public BB bb;
	public int level;
	
	public Poly[] poly = new Poly[2];
	public boolean[] fold;
	public int[][] reason = new int[2][];
	
	public SATBorder(int p1, int p2, P q1, P q2, int level) {
		this.p1 = p1;
		this.p2 = p2;
		bb = new BB();
		bb.update(q1.x.getDouble(), q1.y.getDouble());
		bb.update(q2.x.getDouble(), q2.y.getDouble());
		fold = new boolean[2];
		this.level = level;
	}
	
	public SATBorder(SATBorder b) {
		this.p1 = b.p1;
		this.p2 = b.p2;
		bb = b.bb;
		poly = b.poly.clone();
		fold = b.fold;
		level = b.level;
		reason = b.reason.clone();
	}
	
}
