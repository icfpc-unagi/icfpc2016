
public class Poly {
	
	public P[] ps;
	public int[] sid;
	public int pid;
	public BB bb;
	
	public Poly(P[] ps, int[] sid, int pid) {
		this.ps = ps;
		this.sid = sid;
		this.pid = pid;
		bb = new BB();
		for (P p : ps) bb.update(p.x.getDouble(), p.y.getDouble());
	}
	
}
