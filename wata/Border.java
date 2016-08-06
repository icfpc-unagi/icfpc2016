
public class Border {
	
	public int p1, p2;
	public BB bb;
	
	public Poly[] poly = new Poly[2];
	public boolean[] fold;
	
	public Border(int p1, int p2, P q1, P q2) {
		this.p1 = p1;
		this.p2 = p2;
		bb = new BB();
		bb.update(q1.x.getDouble(), q1.y.getDouble());
		bb.update(q2.x.getDouble(), q2.y.getDouble());
		fold = new boolean[2];
	}
	
	public Border(Border b) {
		this.p1 = b.p1;
		this.p2 = b.p2;
		bb = b.bb;
		poly = b.poly.clone();
		fold = b.fold;
	}
	
}
