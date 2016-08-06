import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;
import tc.wata.util.SetOpt.*;

public class Main {
	
	@Option(abbr = 'a')
	public String algo = "FoldedSolver";
	
	@Option(abbr = 'd')
	public boolean debug = false;
	
	@Option(abbr = 's')
	public long seed = 743943;
	
	void run() throws Exception {
		Scanner sc = new Scanner(System.in);
		int n = sc.nextInt();
		P[] ps = new P[n];
		for (int i = 0; i < ps.length; i++) {
			ps[i] = new P(new R(sc.next()), new R(sc.next()));
		}
		int m = sc.nextInt();
		int[][] poly = new int[m][];
		for (int i = 0; i < poly.length; i++) {
			poly[i] = new int[sc.nextInt()];
			for (int j = 0; j < poly[i].length; j++) {
				poly[i][j] = sc.nextInt();
			}
		}
		Solver solver = (Solver) Class.forName(algo).newInstance();
		solver.polySkeleton = poly;
		solver.psSkeleton = ps;
		solver.areas = new R[poly.length];
		for (int i = 0; i < poly.length; i++) {
			P[] qs = new P[poly[i].length];
			for (int j = 0; j < qs.length; j++) qs[j] = ps[poly[i][j]];
			solver.areas[i] = P.area(qs);
		}
		solver.edgesSkeleton = Solver.createEdgeMap(poly);
		solver.debug = debug;
		if (seed == 0) {
			seed = new Random().nextLong();
			System.err.printf("Seed = %d%n", seed);
		}
		solver.rand = new Random(seed);
		Stat.start("solve");
		boolean res = solver.solve();
		Stat.end("solve");
		Debug.check(res);
		System.err.println("Succeeded!");
	}
	
	public static void main(String[] args) {
		Stat.setShutdownHook();
		Main main = new Main();
		SetOpt.setOpt(main, args);
		try {
			main.run();
		} catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
	}
	
}
