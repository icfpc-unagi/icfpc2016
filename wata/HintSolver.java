import static java.util.Arrays.*;

import java.awt.*;
import java.awt.geom.*;
import java.math.*;
import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;

public class HintSolver extends Solver {
	
	Vis vis;
	
	@Override
	public boolean solve() {
		int hintN = sc.hasNext() ? sc.nextInt() : 0;
		int[] hints = new int[hintN];
		for (int i = 0; i < hintN; i++) hints[i] = sc.nextInt();
		State s = new State();
		if (hintN > 0) {
			Map<Long, R> length = new TreeMap<Long, R>();
			for (int i = 0; i < polySkeleton.length; i++) {
				for (int j = 0; j < polySkeleton[i].length; j++) {
					int a = polySkeleton[i][j], b = Utils.get(polySkeleton[i], j + 1);
					R d = psSkeleton[b].sub(psSkeleton[a]).abs2().sqrt();
					if (d != null) length.put(Utils.pair(a, b), d);
				}
			}
			s.ps = new P[hintN];
			s.cor = hints.clone();
			s.border = new Border[hintN - 1];
			R total = R.ZERO;
			P dir = new P(0, 1);
			s.ps[0] = new P(0, 0);
			for (int i = 0; i + 1 < hintN; i++) {
				int a = hints[i], b = hints[i + 1];
				R d = length.get(Utils.pair(a, b));
				if (d == null) d = length.get(Utils.pair(b, a));
				if (d == null) {
					System.err.printf("Error: illegal length %d-%d%n", a, b);
					return false;
				}
				s.ps[i + 1] = s.ps[i].add(dir.mul(d));
				s.border[i] = createBorder(s, i, i + 1, -1);
				int num = total.num.divide(total.den).intValue();
				total = total.add(d);
				int num2 = total.num.divide(total.den).intValue();
				Debug.check(total.compareTo(new R(4)) < 0);
				if (num != num2) {
					Debug.check(num + 1 == num2);
					Debug.check(total.num.remainder(total.den).signum() == 0);
					dir = new P(dir.y, dir.x.neg());
				}
			}
			System.err.printf("Total length: %s%n", total);
			s.pid = null;
			s.setPid();
		}
		if (hintN <= 1) {
			ArrayList<Poly> plist = new ArrayList<Poly>();
			for (int i = 0; i < polySkeleton.length; i++) {
				for (int j = 0; j < polySkeleton[i].length; j++) {
					Poly p = placePoly(new P(lx, ly), new P(ux, ly), i, j, false);
					if (p != null && canPlace(s, p)) plist.add(p);
					R d = psSkeleton[Utils.get(polySkeleton[i], j + 1)].sub(psSkeleton[polySkeleton[i][j]]).abs2().sqrt();
					if (d != null) {
						p = placePoly(new P(d, R.ZERO), new P(lx, ly), i, j, true);
						if (p != null && canPlace(s, p)) plist.add(p);
					}
				}
			}
			Collections.shuffle(plist, rand);
			int count = 0;
			for (Poly p : plist) {
				System.err.printf("Solving: %d / %d%n", count++, plist.size());
				if (rec(place(s, p))) return true;
			}
			return false;
		} else {
			s.last = new BB();
			s.last.minX = s.last.minY = -1;
			s.last.maxX = s.last.maxY = 2;
			return rec(s);
		}
	}
	
	R lx = R.ZERO, ux = R.ONE, ly = R.ZERO, uy = R.ONE, AREA = R.ONE;
	
	R maxArea = R.ZERO;
	
	public boolean rec(State s) {
		if (maxArea.compareTo(s.usedArea()) < 0) {
			maxArea = s.usedArea;
			System.err.printf("%.2f%% filled%n", maxArea.getDouble2() * 100.0);
		}
		if (debug > 0) {
//			R used = s.usedArea();
//			if (maxArea.compareTo(used) < 0) {
//				maxArea = used;
				s.vis();
//			}
		}
		if (s.usedArea().compareTo(AREA) == 0) {
			Debug.check(s.remainingArea().signum() == 0);
			System.out.print(s);
//			s.vis();
			return true;
		}
		for (Border b : s.border) {
			for (int i = 0; i < 2; i++) {
				if (b.poly[i] != null && b.poly[i].bb.crs(s.last)) {
					if (!canPlace(s, b.poly[i])) {
						b.poly[i] = null;
					}
				}
			}
			if (b.poly[0] == null && b.poly[1] == null) return false;
		}
		for (Border b : s.border) {
			for (int i = 0; i < 2; i++) {
				if (b.poly[1 - i] == null) {
					return canPlace(s, b.poly[i]) && rec(place(s, b.poly[i]));
				}
			}
		}
		double maxScore = -1;
		Poly p1 = null, p2 = null;
		for (Border b : s.border) {
//			Debug.check(canPlace(s, b.poly[0]));
//			Debug.check(canPlace(s, b.poly[1]));
			double score = score(s, b.poly[0]);
			if (maxScore < score) {
				maxScore = score;
				p1 = b.poly[0];
				p2 = b.poly[1];
			}
			score = score(s, b.poly[1]);
			if (maxScore < score) {
				maxScore = score;
				p1 = b.poly[1];
				p2 = b.poly[0];
			}
		}
		if (canPlace(s, p1) && rec(place(s, p1))) return true;
		return canPlace(s, p2) && rec(place(s, p2));
	}
	
	boolean canPlace(State s, Poly poly) {
		for (P p : poly.ps) {
			if (p.x.compareTo(lx) < 0 || p.x.compareTo(ux) > 0) return false;
			if (p.y.compareTo(ly) < 0 || p.y.compareTo(uy) > 0) return false;
		}
		for (int i = 0; i < poly.ps.length; i++) {
			Integer v = s.pid.get(poly.ps[i]);
			if (v != null && s.cor[v] != poly.sid[i]) return false;
		}
		for (Border b : s.border) if (b.bb.crs(poly.bb)) {
			for (int i = 0; i < poly.ps.length; i++) {
				P p1 = poly.ps[i], p2 = poly.ps[(i + 1) % poly.ps.length];
				if (P.crsSS2(s.ps[b.p1], s.ps[b.p2], p1, p2)) return false;
			}
		}
		R remain = s.remainingArea();
		if (s.used[poly.pid] == 0) remain = remain.sub(areas[poly.pid]);
		if (AREA.sub(s.usedArea().add(areas[poly.pid])).compareTo(remain) < 0) return false;
		return true;
	}
	
	State place(State s, Poly poly) {
		State t = new State(s);
		t.used[poly.pid]++;
		t.ps = copyOf(s.ps, s.ps.length + poly.ps.length);
		int pn = s.ps.length;
		for (P p : poly.ps) if (!s.pid.containsKey(p)) t.ps[pn++] = p;
		t.ps = copyOf(t.ps, pn);
		t.setPid();
		t.poly[t.poly.length - 1] = new int[poly.ps.length];
		t.cor = copyOf(s.cor, t.ps.length);
		for (int i = s.cor.length; i < t.cor.length; i++) t.cor[i] = -1;
		t.last = new BB();
		for (int i = 0; i < poly.ps.length; i++) {
			int id = t.pid.get(poly.ps[i]);
			t.poly[t.poly.length - 1][i] = id;
			if (t.cor[id] >= 0) Debug.check(t.cor[id] == poly.sid[i]);
			t.cor[id] = poly.sid[i];
			t.last.update(poly.ps[i].x.getDouble(), poly.ps[i].y.getDouble());
		}
		Set<Long> set = new TreeSet<Long>();
		for (int i = 0; i < poly.ps.length; i++) {
			int p1 = t.poly[t.poly.length - 1][i], p2 = Utils.get(t.poly[t.poly.length - 1], i + 1);
			set.add(Utils.pair(p1, p2));
		}
		ArrayList<Border> bs = new ArrayList<Border>();
		for (Border b : s.border) {
			if (!set.remove(Utils.pair(b.p2, b.p1))) {
				bs.add(new Border(b));
			}
		}
		for (int i = 0; i < poly.ps.length; i++) {
			int p1 = t.poly[t.poly.length - 1][i], p2 = Utils.get(t.poly[t.poly.length - 1], i + 1);
			if (set.contains(Utils.pair(p1, p2))) {
				if (t.ps[p1].x.compareTo(lx) == 0 && t.ps[p2].x.compareTo(lx) == 0) continue;
				if (t.ps[p1].y.compareTo(ly) == 0 && t.ps[p2].y.compareTo(ly) == 0) continue;
				if (t.ps[p1].x.compareTo(ux) == 0 && t.ps[p2].x.compareTo(ux) == 0) continue;
				if (t.ps[p1].y.compareTo(uy) == 0 && t.ps[p2].y.compareTo(uy) == 0) continue;
				bs.add(createBorder(t, p1, p2, poly.pid));
			}
		}
		t.border = bs.toArray(new Border[0]);
		return t;
	}
	
	Border createBorder(State t, int p1, int p2, int pid) {
		Border b = new Border(p1, p2, t.ps[p1], t.ps[p2]);
		Long e1 = edgesSkeleton.get(Utils.pair(t.cor[p2], t.cor[p1]));
		Long e2 = edgesSkeleton.get(Utils.pair(t.cor[p1], t.cor[p2]));
		if (e1 != null) {
			b.poly[0] = placePoly(t.ps[p2], t.ps[p1], (int)(e1 >> 32), e1.intValue(), false);
			if (b.poly[0] != null && b.poly[0].pid == pid) b.fold[0] = true;
		}
		if (e2 != null) {
			b.poly[1] = placePoly(t.ps[p1], t.ps[p2], (int)(e2 >> 32), e2.intValue(), true);
			if (b.poly[1] != null && b.poly[1].pid == pid) b.fold[1] = true;
		}
		return b;
	}
	
	double score(State s, Poly poly) {
		if (s.used[poly.pid] == 0) return 1;
		return 0;
	}
	
	class State {
		int[] used;
		P[] ps;
		int[] cor;
		int[][] poly;
		Border[] border;
		BB last;
		Map<P, Integer> pid;
		R usedArea, remainingArea;
		State() {
			used = new int[polySkeleton.length];
			ps = new P[0];
			cor = new int[0];
			poly = new int[0][];
			border = new Border[0];
			pid = new TreeMap<P, Integer>();
		}
		State(State s) {
			used = s.used.clone();
			poly = copyOf(s.poly, s.poly.length + 1);
		}
		R usedArea() {
			if (usedArea != null) return usedArea;
			R a = R.ZERO;
			for (int i = 0; i < used.length; i++) if (used[i] > 0) {
				a = a.add(areas[i].mul(new R(used[i])));
			}
			return usedArea = a;
		}
		R remainingArea() {
			if (remainingArea != null) return remainingArea;
			R a = R.ZERO;
			for (int i = 0; i < used.length; i++) if (used[i] == 0) {
				a = a.add(areas[i]);
			}
			return remainingArea = a;
		}
		void setPid() {
			if (pid != null) return;
			pid = new TreeMap<P, Integer>();
			for (int i = 0; i < ps.length; i++) pid.put(ps[i], i);
		}
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder();
			sb.append(ps.length).append('\n');
			for (int i = 0; i < ps.length; i++) {
				sb.append(ps[i].x).append(',').append(ps[i].y).append('\n');
			}
			sb.append(poly.length).append('\n');
			for (int i = 0; i < poly.length; i++) {
				sb.append(poly[i].length);
				for (int j = 0; j < poly[i].length; j++) {
					sb.append(' ').append(poly[i][j]);
				}
				sb.append('\n');
			}
			for (int i = 0; i < ps.length; i++) {
				P p = psSkeleton[cor[i]];
				sb.append(p.x).append(',').append(p.y).append('\n');
			}
			return sb.toString();
		}
		public State unfold() {
			int fx = ux.den.intValue();
			TreeMap<P, Integer> vs = new TreeMap<P, Integer>();
			for (int i = 0; i < ps.length; i++) vs.put(ps[i], i);
			ArrayList<int[]> poly2 = new ArrayList<int[]>();
			ArrayList<Integer> cor2 = new ArrayList<Integer>();
			for (int[] a : poly) poly2.add(a);
			for (int i = 0; i < ps.length; i++) cor2.add(cor[i]);
			for (int x = 1; x < fx; x++) {
				for (int i = 0; i < poly.length; i++) {
					int[] a = new int[poly[i].length];
					for (int j = 0; j < poly[i].length; j++) {
						P p = ps[poly[i][j]];
						if (x % 2 == 0) {
							p = p.add(new P(ux.mul(new R(x)), R.ZERO));
						} else {
							p = new P(ux.mul(new R(x + 1)).sub(p.x), p.y);
						}
						if (!vs.containsKey(p)) {
							vs.put(p, vs.size());
							cor2.add(cor[poly[i][j]]);
						}
						if (x % 2 == 0) a[j] = vs.get(p);
						else a[a.length - 1 - j] = vs.get(p);
					}
					poly2.add(a);
				}
			}
			State s = new State();
			s.ps = new P[vs.size()];
			for (Map.Entry<P, Integer> e : vs.entrySet()) s.ps[e.getValue()] = e.getKey();
			s.poly = poly2.toArray(new int[0][0]);
			s.cor = Utils.toi(cor2);
			return s;
		}
		public void vis() {
			if (vis == null) {
				vis = new Vis();
				vis.setRange(-0.1, -0.1, 1.1, 2.2);
			}
			vis.clear();
			vis.g.setColor(Color.blue);
			double x1 = lx.getDouble(), x2 = ux.getDouble(), y1 = ly.getDouble(), y2 = uy.getDouble();
			vis.g.draw(vis.segment(x1, y1, x2, y1));
			vis.g.draw(vis.segment(x2, y1, x2, y2));
			vis.g.draw(vis.segment(x2, y2, x1, y2));
			vis.g.draw(vis.segment(x1, y2, x1, y1));
			vis.g.draw(vis.segment(0, 1.1, 1, 1.1));
			vis.g.draw(vis.segment(1, 1.1, 1, 2.1));
			vis.g.draw(vis.segment(1, 2.1, 0, 2.1));
			vis.g.draw(vis.segment(0, 2.1, 0, 1.1));
			for (int i = 0; i < poly.length; i++) {
				Path2D.Double path = new Path2D.Double();
				path.moveTo(ps[poly[i][0]].x.getDouble(), ps[poly[i][0]].y.getDouble());
				R area = R.ZERO;
				for (int j = 0; j < poly[i].length; j++) {
					int b = Utils.get(poly[i], j + 1);
					path.lineTo(ps[b].x.getDouble(), ps[b].y.getDouble());
					area = area.add(psSkeleton[cor[poly[i][j]]].det(psSkeleton[cor[b]]));
				}
				if (area.signum() < 0) vis.g.setColor(Color.lightGray);
				else vis.g.setColor(Color.pink);
				vis.g.fill(path);
				vis.g.setColor(Color.red);
				vis.g.draw(path);
			}
			R minX = psSkeleton[0].x, maxX = minX;
			R minY = psSkeleton[0].y, maxY = minY;
			for (int i = 1; i < psSkeleton.length; i++) {
				P p = psSkeleton[i];
				if (minX.compareTo(p.x) > 0) minX = p.x;
				if (maxX.compareTo(p.x) < 0) maxX = p.x;
				if (minY.compareTo(p.y) > 0) minY = p.y;
				if (maxY.compareTo(p.y) < 0) maxY = p.y;
			}
			minX = minX.sub(R.ONE.sub(maxX.sub(minX)).div(R.TWO));
			minY = minY.sub(R.ONE.sub(maxY.sub(minY)).div(R.TWO));
			for (int i = 0; i < polySkeleton.length; i++) {
				Path2D.Double path = new Path2D.Double();
				path.moveTo(psSkeleton[polySkeleton[i][0]].x.sub(minX).getDouble(), psSkeleton[polySkeleton[i][0]].y.sub(minY).getDouble() + 1.1);
				for (int j = 0; j < polySkeleton[i].length; j++) {
					int b = Utils.get(polySkeleton[i], j + 1);
					path.lineTo(psSkeleton[b].x.sub(minX).getDouble(), psSkeleton[b].y.sub(minY).getDouble() + 1.1);
				}
				vis.g.setColor(Color.lightGray);
				if (used[i] > 0) vis.g.fill(path);
				vis.g.setColor(Color.red);
				vis.g.draw(path);
			}
			vis.g.setColor(Color.GREEN);
			for (Border b : border) vis.g.draw(vis.segment(ps[b.p1].x.getDouble(), ps[b.p1].y.getDouble(), ps[b.p2].x.getDouble(), ps[b.p2].y.getDouble()));
			vis.vis(false);
//			vis.dispose();
		}
	}
	
}
