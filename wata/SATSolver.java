import static java.lang.Math.*;
import static java.util.Arrays.*;

import java.awt.*;
import java.awt.geom.*;
import java.math.*;
import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;

public class SATSolver extends Solver {
	
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
			s.level = new int[hintN];
			s.border = new SATBorder[hintN - 1];
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
			if (hintN <= 1) {
				ArrayList<Poly> plist = new ArrayList<Poly>();
				for (int i = 0; i < polySkeleton.length; i++) {
					for (int j = 0; j < polySkeleton[i].length; j++) {
						Poly p = placePoly(new P(lx, ly), new P(ux, ly), i, j, false);
						if (p != null && canPlace(s, p) == OK) plist.add(p);
						R d = psSkeleton[Utils.get(polySkeleton[i], j + 1)].sub(psSkeleton[polySkeleton[i][j]]).abs2().sqrt();
						if (d != null) {
							p = placePoly(new P(d, R.ZERO), new P(lx, ly), i, j, true);
							if (p != null && canPlace(s, p) == OK) plist.add(p);
						}
					}
				}
				Collections.shuffle(plist, rand);
				int count = 0;
				for (Poly p : plist) {
					System.err.printf("Solving: %d / %d%n", count++, plist.size());
					if (rec(place(s, p, -1), -1).type == Return.FOUND) return true;
				}
				return false;
			} else {
				s.last = new BB();
				s.last.minX = s.last.minY = -1;
				s.last.maxX = s.last.maxY = 2;
				return (rec(s, -1).type == Return.FOUND);
			}
		}
		
		BigInteger[] bs;
		if (FOLD == 0) {
			R total = R.ZERO;
			for (int i = 0; i < polySkeleton.length; i++) total = total.add(areas[i]);
			TreeSet<BigInteger> set = new TreeSet<BigInteger>();
			set.add(BigInteger.ONE);
			for (int i = 0; i < polySkeleton.length; i++) {
				for (int j = 0; j < polySkeleton[i].length; j++) {
					R d = psSkeleton[Utils.get(polySkeleton[i], j + 1)].sub(psSkeleton[polySkeleton[i][j]]).abs2().sqrt();
					if (d != null && d.num.equals(BigInteger.ONE) && total.compareTo(d) <= 0) {
						set.add(d.den);
					}
				}
			}
			bs = set.toArray(new BigInteger[0]);
			sort(bs);
		} else {
			bs = new BigInteger[]{BigInteger.valueOf(FOLD)};
		}
		for (int i = bs.length - 1; i >= 0; i--) {
			Debug.print("fold", bs[i]);
			ux = AREA = new R(BigInteger.ONE, bs[i]);
			if (solve2()) return true;
		}
		return false;
	}
	
	R lx = R.ZERO, ux = R.ONE, ly = R.ZERO, uy = R.ONE, AREA = R.ONE;
	
	boolean solve2() {
		ArrayList<Poly> plist = new ArrayList<Poly>();
		State s = new State();
		for (int i = 0; i < polySkeleton.length; i++) {
			for (int j = 0; j < polySkeleton[i].length; j++) {
				Poly p = placePoly(new P(lx, ly), new P(ux, ly), i, j, false);
				if (p != null && canPlace(s, p) == OK) plist.add(p);
				R d = psSkeleton[Utils.get(polySkeleton[i], j + 1)].sub(psSkeleton[polySkeleton[i][j]]).abs2().sqrt();
				if (d != null) {
					p = placePoly(new P(d, R.ZERO), new P(lx, ly), i, j, true);
					if (p != null && canPlace(s, p) == OK) plist.add(p);
				}
			}
		}
		Collections.shuffle(plist, rand);
		int count = 0;
		for (Poly p : plist) {
			System.err.printf("%d / %d%n", count++, plist.size());
			if (rec(place(s, p, -1), -1).type == Return.FOUND) return true;
		}
		return false;
	}
	
	R maxArea = R.ZERO;
	
	static class Return {
		static final int CONFLICT = 0, UNIT = 1, FOUND = 2;
		int type; // 0: conflict, 1: unit, 2: found
		int[] reason;
		int p1, p2, i;
		static Return conflict(int[] reason) {
			Return ret = new Return();
			ret.type = 0;
			ret.reason = reason;
			return ret;
		}
		static Return unit(int[] reason, int p1, int p2, int i) {
			Return ret = new Return();
			ret.type = 1;
			ret.reason = reason;
			ret.p1 = p1;
			ret.p2 = p2;
			ret.i = i;
			return ret;
		}
		static Return found() {
			Return ret = new Return();
			ret.type = 2;
			return ret;
		}
	}
	
	public Return rec(State s, int lastDecision) {
		if (s.usedArea().compareTo(AREA) == 0) {
			Debug.check(s.remainingArea().signum() == 0);
			s = s.unfold();
			System.out.print(s);
			if (debug > 0) s.vis();
			return Return.found();
		}
		for (SATBorder b : s.border) {
			for (int i = 0; i < 2; i++) {
				if (b.poly[i] != null && b.poly[i].bb.crs(s.last)) {
					int c = canPlace(s, b.poly[i]);
					if (c != OK) {
						b.poly[i] = null;
						b.reason[i] = (c == NG_AREA) ? null : is(c, b.level);
					}
				}
			}
			if (b.poly[0] == null && b.poly[1] == null) {
				return Return.conflict(resolution(insert(b.reason[0], s.poly.length + 1), insert(b.reason[1], s.poly.length + 1)));
			}
		}
		for (SATBorder b : s.border) {
			for (int i = 0; i < 2; i++) {
				if (b.poly[1 - i] == null) {
					int[] r1 = insert(b.reason[1 - i], s.poly.length + 1);
					int c = canPlace(s, b.poly[i]);
					if (c != OK) {
						return Return.conflict(resolution(r1, c == NG_AREA ? null : is(c, b.level, s.poly.length + 1)));
					}
					Return ret = rec(place(s, b.poly[i], b.fold[i] ? Utils.pair(s.cor[b.p1], s.cor[b.p2]) : -1), lastDecision);
					if (ret.type == Return.FOUND) return ret;
					if (ret.type == Return.CONFLICT) {
						if (!contains(ret.reason, s.poly.length + 1)) return ret;
						return Return.conflict(resolution(r1, ret.reason));
					} else {
						Debug.print("unit");
						return ret;
					}
				}
			}
		}
		double maxScore = -1;
		int bi = -1, bj = -1;
		for (int i = 0; i < s.border.length; i++) {
			SATBorder b = s.border[i];
			double score = score(s, b.poly[0], Utils.pair(s.cor[b.p1], s.cor[b.p2]));
			if (maxScore < score) {
				maxScore = score;
				bi = i;
				bj = 0;
			}
			score = score(s, b.poly[1], Utils.pair(s.cor[b.p1], s.cor[b.p2]));
			if (maxScore < score) {
				maxScore = score;
				bi = i;
				bj = 1;
			}
		}
		if (maxArea.compareTo(s.usedArea()) < 0) {
			maxArea = s.usedArea;
			System.err.printf("%.2f%% filled%n", maxArea.getDouble2() * 100.0);
		}
		if (debug > 0) s.vis();
		SATBorder b = s.border[bi];
		int r1 = canPlace(s, b.poly[bj]);
		Return ret1;
		if (r1 == OK) {
			ret1 = rec(place(s, b.poly[bj], b.fold[bj] ? Utils.pair(s.cor[b.p1], s.cor[b.p2]) : -1), s.poly.length + 1);
			if (ret1.type == Return.FOUND) return ret1;
			if (ret1.type == Return.UNIT) {
				Debug.print("unit");
				if (ret1.reason[ret1.reason.length - 1] >= lastDecision) {
					for (SATBorder a : s.border) if (a.p1 == ret1.p1 && a.p2 == ret1.p2) {
						a.poly[ret1.i] = null;
						a.reason[ret1.i] = ret1.reason;
					}
					return rec(s, lastDecision);
				}
				return ret1;
			}
			if (!contains(ret1.reason, s.poly.length + 1)) return ret1;
			ret1.reason = insert(ret1.reason, b.level);
		} else {
			ret1 = Return.conflict(insert(r1 == NG_AREA ? null : is(r1, b.level), s.poly.length + 1));
		}
		Debug.print(ret1.reason, lastDecision);
		if (ret1.reason != null && ret1.reason[ret1.reason.length - 2] >= 1 && ret1.reason[ret1.reason.length - 2] < lastDecision) {
			return Return.unit(copyOf(ret1.reason, ret1.reason.length - 1), b.p1, b.p2, bj);
		}
		if (debug > 0) s.vis();
		int r2 = canPlace(s, b.poly[1 - bj]);
		Return ret2;
		if (r2 == OK) {
			ret2 = rec(place(s, b.poly[1 - bj], b.fold[1 - bj] ? Utils.pair(s.cor[b.p1], s.cor[b.p2]) : -1), s.poly.length + 1);
			if (ret2.type == Return.FOUND) return ret2;
			if (ret2.type == Return.UNIT) {
				Debug.print("unit");
				if (ret2.reason[ret2.reason.length - 1] >= lastDecision) {
					for (SATBorder a : s.border) if (a.p1 == ret2.p1 && a.p2 == ret2.p2) {
						a.poly[ret2.i] = null;
						a.reason[ret2.i] = ret2.reason;
					}
					return rec(s, lastDecision);
				}
				return ret2;
			}
			if (!contains(ret2.reason, s.poly.length + 1)) return ret2;
			ret2.reason = insert(ret2.reason, b.level);
		} else {
			ret2 = Return.conflict(insert(r2 == NG_AREA ? null : is(r2, b.level), s.poly.length + 1));
		}
		return Return.conflict(resolution(ret1.reason, ret2.reason));
	}
	
	static final int OK = -3;
	static final int NG_AREA = -2;
	static final int NG_OUTSIDE = -1;
	
	int canPlace(State s, Poly poly) {
		if (bit > 0) {
			for (P p : poly.ps) {
				if (p.x.den.bitCount() > bit) return NG_OUTSIDE;
				if (p.y.den.bitCount() > bit) return NG_OUTSIDE;
			}
		}
		for (P p : poly.ps) {
			if (p.x.compareTo(lx) < 0 || p.x.compareTo(ux) > 0) return NG_OUTSIDE;
			if (p.y.compareTo(ly) < 0 || p.y.compareTo(uy) > 0) return NG_OUTSIDE;
		}
		for (int i = 0; i < poly.ps.length; i++) {
			Integer v = s.pid.get(poly.ps[i]);
			if (v != null && s.cor[v] != poly.sid[i]) return s.level[v];
		}
		for (SATBorder b : s.border) if (b.bb.crs(poly.bb)) {
			for (int i = 0; i < poly.ps.length; i++) {
				P p1 = poly.ps[i], p2 = poly.ps[(i + 1) % poly.ps.length];
				if (P.crsSS2(s.ps[b.p1], s.ps[b.p2], p1, p2)) return b.level;
			}
		}
		R remain = s.remainingArea();
		if (s.used[poly.pid] == 0) remain = remain.sub(areas[poly.pid]);
		if (AREA.sub(s.usedArea().add(areas[poly.pid])).compareTo(remain) < 0) return NG_AREA;
		return OK;
	}
	
	int[] is(int...is) {
		sort(is);
		return Utils.unique(is);
	}
	
	boolean contains(int[] reason, int level) {
		if (reason == null) return true;
		if (reason.length == 0) return false;
		return reason[reason.length - 1] == level;
	}
	
	int[] resolution(int[] a, int[] b) {
		if (a == null || b == null) return null;
		Debug.check(a[a.length - 1] == b[b.length - 1]);
		int[] c = Utils.unique(Utils.merge(a, b));
		return copyOf(c, c.length - 1);
	}
	
	int[] insert(int[] a, int v) {
		if (a == null) return null;
		return Utils.unique(Utils.merge(a, new int[]{v}));
	}
	
	State place(State s, Poly poly, long foldE) {
		State t = new State(s);
		t.used[poly.pid]++;
		t.ps = copyOf(s.ps, s.ps.length + poly.ps.length);
		t.level = copyOf(s.level, t.ps.length);
		int pn = s.ps.length;
		for (P p : poly.ps) if (!s.pid.containsKey(p)) {
			t.ps[pn] = p;
			t.level[pn++] = t.poly.length;
		}
		t.ps = copyOf(t.ps, pn);
		t.level = copyOf(t.level, pn);
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
		ArrayList<SATBorder> bs = new ArrayList<SATBorder>();
		for (SATBorder b : s.border) {
			if (!set.remove(Utils.pair(b.p2, b.p1))) {
				bs.add(new SATBorder(b));
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
		t.border = bs.toArray(new SATBorder[0]);
		if (foldE >= 0 && t.folds != null) inc(t.folds, foldE);
		return t;
	}
	
	SATBorder createBorder(State t, int p1, int p2, int pid) {
		SATBorder b = new SATBorder(p1, p2, t.ps[p1], t.ps[p2], t.poly.length);
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
	
	double score(State s, Poly poly, long foldE) {
		if (s.used[poly.pid] == 0) return 1;
		return 0;
	}
	
	int get(Map<Long, Integer> map, long key) {
		Integer v = map.get(key);
		if (v == null) return 0;
		return v;
	}
	
	void inc(Map<Long, Integer> map, long key) {
		map.put(key, get(map, key) + 1);
	}
	
	class State {
		int[] used;
		P[] ps;
		int[] cor;
		int[] level;
		int[][] poly;
		SATBorder[] border;
		BB last;
		Map<P, Integer> pid;
		R usedArea, remainingArea;
		Map<Long, Integer> folds;
		State() {
			used = new int[polySkeleton.length];
			ps = new P[0];
			cor = new int[0];
			level = new int[0];
			poly = new int[0][];
			border = new SATBorder[0];
			pid = new TreeMap<P, Integer>();
//			folds = new TreeMap<Long, Integer>(); //
		}
		State(State s) {
			used = s.used.clone();
			poly = copyOf(s.poly, s.poly.length + 1);
			if (s.folds != null) folds = new TreeMap<Long, Integer>(s.folds);
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
			} else {
				vis.clear();
			}
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
			for (SATBorder b : border) vis.g.draw(vis.segment(ps[b.p1].x.getDouble(), ps[b.p1].y.getDouble(), ps[b.p2].x.getDouble(), ps[b.p2].y.getDouble()));
			vis.vis(debug == 1);
			if (debug == 1) {
				vis.dispose();
				vis = null;
			}
		}
	}
	
}