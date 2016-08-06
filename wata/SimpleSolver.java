import static java.util.Arrays.*;

import java.awt.*;
import java.awt.geom.*;
import java.math.*;
import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;

public class SimpleSolver extends Solver {
	
	@Override
	public boolean solve() {
		ArrayList<State> ss = new ArrayList<SimpleSolver.State>();
		for (int i = 0; i < polySkeleton.length; i++) {
			for (int j = 0; j < polySkeleton[i].length; j++) {
				State s = next(new State(), new P(0, 0), new P(1, 0), i, j, false);
				if (s != null) {
					ss.add(s);
				}
				R d = psSkeleton[Utils.get(polySkeleton[i], j + 1)].sub(psSkeleton[polySkeleton[i][j]]).abs2().sqrt();
				if (d != null) {
					s = next(new State(), new P(d, R.ZERO), new P(0, 0), i, j, true);
					if (s != null) {
						ss.add(s);
					}
				}
			}
		}
		Collections.shuffle(ss, rand);
		for (State s : ss) if (rec(s)) return true;
		return false;
	}
	
	public boolean rec(State s) {
		if (debug) s.vis();
//		if (debug && s.usedArea().compareTo(new R(BigInteger.valueOf(9),BigInteger.valueOf(10))) > 0) s.vis();
		if (s.usedArea().compareTo(R.ONE) == 0) {
			System.out.print(s);
//			if (debug) s.vis();
			return true;
		}
		Map<Long, Long> edges = createEdgeMap(s.poly);
		double maxScore = -1;
		State t1 = null, t2 = null;
		for (int i = 0; i < s.poly.length; i++) {
			for (int j = 0; j < s.poly[i].length; j++) {
				int p1 = s.poly[i][j], p2 = Utils.get(s.poly[i], j + 1);
				Long e = edges.get(Utils.pair(p2, p1));
				if (e == null) {
					if (s.ps[p1].y.signum() == 0 && s.ps[p2].y.signum() == 0) continue;
					if (s.ps[p1].x.compareTo(R.ONE) == 0 && s.ps[p2].x.compareTo(R.ONE) == 0) continue;
					if (s.ps[p1].y.compareTo(R.ONE) == 0 && s.ps[p2].y.compareTo(R.ONE) == 0) continue;
					if (s.ps[p1].x.signum() == 0 && s.ps[p2].x.signum() == 0) continue;
					int v1 = s.cor[p1], v2 = s.cor[p2];
					Long f1 = edgesSkeleton.get(Utils.pair(v2, v1)), f2 = edgesSkeleton.get(Utils.pair(v1, v2));
//						Debug.print(i, j, v1, v2);
					State s1 = null, s2 = null;
					if (f1 != null) {
						s1 = next(s, s.ps[p2], s.ps[p1], (int)(f1 >> 32), f1.intValue(), false);
//							Debug.print("f1", (int)(f1 >> 32), f1.intValue(), s1 != null);
					}
					if (f2 != null) {
						s2 = next(s, s.ps[p1], s.ps[p2], (int)(f2 >> 32), f2.intValue(), true);
//							Debug.print("f2", (int)(f2 >> 32), f2.intValue(), s2 != null);
					}
					if (s1 == null && s2 == null) return false;
					if (s1 == null) return rec(s2);
					if (s2 == null) return rec(s1);
					if (maxScore < Math.max(s1.score(), s2.score())) {
						maxScore = Math.max(s1.score(), s2.score());
						t1 = s1;
						t2 = s2;
					}
				}
			}
		}
		if (t1 == null) return false;
		if (t1.score() > t2.score()) return rec(t1) || rec(t2);
		return rec(t2) || rec(t1);
	}
	
	State next(State s, P a, P b, int i, int j, boolean reflect) {
		P[] qs = place(a, b, i, j, reflect);
		if (qs == null) return null;
		for (int k = 0; k < qs.length; k++) {
			if (qs[k].x.signum() < 0 || qs[k].x.compareTo(R.ONE) > 0) return null;
			if (qs[k].y.signum() < 0 || qs[k].y.compareTo(R.ONE) > 0) return null;
		}
		BB bb = new BB();
		for (int k = 0; k < qs.length; k++) bb.update(qs[k].x.getDouble(), qs[k].y.getDouble());
		for (int k = 0; k < s.poly.length; k++) if (bb.crs(s.bb[k])) {
			if (P.crsPP(s.getPoly(k), qs)) return null;
		}
		TreeMap<P, Integer> vs = new TreeMap<P, Integer>();
		for (int k = 0; k < s.ps.length; k++) vs.put(s.ps[k], k);
		for (int k = 0; k < qs.length; k++) if (!vs.containsKey(qs[k])) vs.put(qs[k], vs.size());
		Map<Long, Long> es = createEdgeMap(s.poly);
		int dir = reflect ? -1 : 1;
		for (int k = 0; k < qs.length; k++) {
			int v1 = vs.get(qs[k]), v2 = vs.get(qs[(k + 1) % qs.length]);
			Long e = es.get(Utils.pair(v2, v1));
			if (e != null) {
				int i1 = (int)(e >> 32), j1 = e.intValue(), p2 = s.poly[i1][j1], p1 = Utils.get(s.poly[i1], j1 + 1);
				if (s.cor[p1] != Utils.get(polySkeleton[i], j + k * dir)) return null;
				if (s.cor[p2] != Utils.get(polySkeleton[i], j + (k + 1) * dir)) return null;
			}
		}
		State t = new State(s);
		t.used[i]++;
		t.ps = new P[vs.size()];
		for (Map.Entry<P, Integer> e : vs.entrySet()) t.ps[e.getValue()] = e.getKey();
		t.poly[t.poly.length - 1] = new int[polySkeleton[i].length];
		for (int k = 0; k < t.poly[t.poly.length - 1].length; k++) {
			t.poly[t.poly.length - 1][k] = vs.get(qs[k]);
			P p = t.ps[t.poly[t.poly.length - 1][k]];
			t.bb[t.poly.length - 1].update(p.x.getDouble(), p.y.getDouble());
		}
		t.cor = copyOf(s.cor, t.ps.length);
		for (int k = 0; k < qs.length; k++) {
			t.cor[vs.get(qs[k])] = Utils.get(polySkeleton[i], j + k * dir);
		}
		if (R.ONE.sub(t.usedArea()).compareTo(t.remainingArea()) < 0) return null;
		return t;
	}
	
	class State {
		int[] used;
		P[] ps;
		int[][] poly;
		BB[] bb;
		int[] cor;
		double score = -1;
		State() {
			used = new int[polySkeleton.length];
			ps = new P[0];
			poly = new int[0][];
			bb = new BB[0];
			cor = new int[0];
		}
		State(State s) {
			used = s.used.clone();
			poly = copyOf(s.poly, s.poly.length + 1);
			bb = copyOf(s.bb, s.bb.length + 1);
			bb[s.bb.length] = new BB();
		}
		R usedArea() {
			R a = R.ZERO;
			for (int i = 0; i < used.length; i++) if (used[i] > 0) {
				a = a.add(areas[i].mul(new R(used[i])));
			}
			return a;
		}
		R remainingArea() {
			R a = R.ZERO;
			for (int i = 0; i < used.length; i++) if (used[i] == 0) {
				a = a.add(areas[i]);
			}
			return a;
		}
		P[] getPoly(int i) {
			P[] qs = new P[poly[i].length];
			for (int j = 0; j < qs.length; j++) qs[j] = ps[poly[i][j]];
			return qs;
		}
		double score() {
			if (score >= 0) return score;
			int filled = 0;
			for (int i = 0; i < used.length; i++) if (used[i] > 0) filled++;
			score = (double)filled / used.length;
			int numConcave = 0, numConvex = 0;
			Map<Long, Long> edges = createEdgeMap(poly);
			for (int i = 0; i < poly.length; i++) {
				for (int j = 0; j < poly[i].length; j++) {
					if (!edges.containsKey(Utils.pair(Utils.get(poly[i], j + 1), poly[i][j]))) {
						int i2 = i, j2 = j;
						while (edges.containsKey(Utils.pair(poly[i2][j2], Utils.get(poly[i2], j2 - 1)))) {
							long e = edges.get(Utils.pair(poly[i2][j2], Utils.get(poly[i2], j2 - 1)));
							i2 = (int)(e >> 32);
							j2 = (int)e;
						}
						R det = ps[Utils.get(poly[i], j + 1)].sub(ps[poly[i][j]]).det(ps[Utils.get(poly[i2], j2 - 1)].sub(ps[poly[i][j]]));
						if (det.signum() < 0) numConcave++;
						else if (det.signum() > 0) numConvex++;
					}
				}
			}
			score = score * 10000 + 1.0 / (numConcave + 1);
			return score;
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
		public void vis() {
			Vis vis = new Vis();
			vis.setRange(-0.1, -0.1, 1.1, 2.2);
			vis.g.setColor(Color.blue);
			vis.g.draw(vis.segment(0, 0, 1, 0));
			vis.g.draw(vis.segment(1, 0, 1, 1));
			vis.g.draw(vis.segment(1, 1, 0, 1));
			vis.g.draw(vis.segment(0, 1, 0, 0));
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
			vis.vis(true);
			vis.dispose();
		}
	}
	
}
