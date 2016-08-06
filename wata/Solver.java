import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;

public abstract class Solver {
	
	public boolean debug;
	public Random rand;
	
	public P[] psSkeleton;
	public int[][] polySkeleton;
	public R[] areas;
	public Map<Long, Long> edgesSkeleton;
	
	public abstract boolean solve();
	
	// 多角形iをj番目の点をaに合わせ，j+1番目の点をa->b方向に合わせ，更にreflectがtrueならば反転させた上で配置する
	public P[] place(P a, P b, int i, int j, boolean reflect) {
		P[] qs = new P[polySkeleton[i].length];
		for (int k = 0; k < qs.length; k++) {
			qs[k] = psSkeleton[Utils.get(polySkeleton[i], j + k)];
		}
		P r = qs[0];
		for (int k = 0; k < qs.length; k++) qs[k] = qs[k].sub(r);
		R[][] A = P.rotate(qs[1].sub(qs[0]), b.sub(a));
		if (A == null) return null;
		if (reflect) {
			R[][] B = P.reflect(b.sub(a));
			A = R.mul(B, A);
		}
		qs = P.apply(A, qs);
		for (int k = 0; k < qs.length; k++) qs[k] = qs[k].add(a);
		if (reflect) {
			P[] qs2 = new P[qs.length];
			for (int k = 0; k < qs.length; k++) qs2[k] = qs[(qs.length - k) % qs.length];
			qs = qs2;
		}
		Debug.check(P.area(qs).signum() > 0);
		return qs;
	}
	
	// 多角形iをj番目の点をaに合わせ，j+1番目の点をa->b方向に合わせ，更にreflectがtrueならば反転させた上で配置する
	public Poly placePoly(P a, P b, int i, int j, boolean reflect) {
		P[] qs = new P[polySkeleton[i].length];
		for (int k = 0; k < qs.length; k++) {
			qs[k] = psSkeleton[Utils.get(polySkeleton[i], j + k)];
		}
		P r = qs[0];
		for (int k = 0; k < qs.length; k++) qs[k] = qs[k].sub(r);
		R[][] A = P.rotate(qs[1].sub(qs[0]), b.sub(a));
		if (A == null) return null;
		if (reflect) {
			R[][] B = P.reflect(b.sub(a));
			A = R.mul(B, A);
		}
		qs = P.apply(A, qs);
		for (int k = 0; k < qs.length; k++) qs[k] = qs[k].add(a);
		if (reflect) {
			P[] qs2 = new P[qs.length];
			for (int k = 0; k < qs.length; k++) qs2[k] = qs[(qs.length - k) % qs.length];
			qs = qs2;
		}
		Debug.check(P.area(qs).signum() > 0);
		int[] sid = new int[qs.length];
		if (reflect) {
			for (int k = 0; k < qs.length; k++) sid[k] = Utils.get(polySkeleton[i], j - k);
		} else {
			for (int k = 0; k < qs.length; k++) sid[k] = Utils.get(polySkeleton[i], j + k);
		}
		return new Poly(qs, sid, i);
	}
	
	public static Map<Long, Long> createEdgeMap(int[][] poly) {
		Map<Long, Long> edges = new TreeMap<Long, Long>();
		for (int i = 0; i < poly.length; i++) {
			for (int j = 0; j < poly[i].length; j++) {
				edges.put(Utils.pair(poly[i][j], Utils.get(poly[i], j + 1)), Utils.pair(i, j));
			}
		}
		/*
		for (int i = 0; i < poly.length; i++) {
			for (int j = 0; j < poly[i].length; j++) {
				Long e = edges.get(Utils.pair(Utils.get(poly[i], j + 1), poly[i][j]));
				if (e != null) {
					int i2 = (int)(e >> 32), j2 = (int)e.longValue();
					long f = edges.get(Utils.pair(Utils.get(poly[i2], j2 + 1), poly[i2][j2]));
					Debug.check(i == (f >> 32) && j == (int)f);
				}
			}
		}*/
		return edges;
	}
	
}
