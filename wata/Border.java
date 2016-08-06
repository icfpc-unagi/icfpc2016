
public class Border {
	
	public P p1, p2;
	public BB bb;
	
	public Poly[] poly = new Poly[2];
	
	public Border(P p1, P p2) {
		this.p1 = p1;
		this.p2 = p2;
		bb = new BB();
		bb.update(p1.x.getDouble(), p1.y.getDouble());
		bb.update(p2.x.getDouble(), p2.y.getDouble());
	}
	
	public Border(Border b) {
		this.p1 = b.p1;
		this.p2 = b.p2;
		bb = b.bb;
		poly = b.poly.clone();
	}
	
}
