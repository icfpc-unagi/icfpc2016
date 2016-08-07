import java.util.*;

import tc.wata.debug.*;
import tc.wata.util.*;
import tc.wata.util.SetOpt.*;

public class Main {
	
	@Option(abbr = 'a')
	public String algo = "FastSolver";
	
	@Option(abbr = 'd')
	public int debug = 0;
	
	@Option(abbr = 's')
	public long seed = 743943;
	
	@Option(abbr = 'f', usage = "0: auto, k: k-fold")
	public int fold = 0;
	
	@Option(abbr = 't')
	public int strategy = 0;
	
	@Option(abbr = 'b')
	public int bit;
	
	void run() throws Exception {
		System.err.println("Algo = " + algo);
		System.err.println("Strategy = " + strategy);
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
		solver.areasDouble = new double[poly.length];
		for (int i = 0; i < poly.length; i++) {
			P[] qs = new P[poly[i].length];
			for (int j = 0; j < qs.length; j++) qs[j] = ps[poly[i][j]];
			solver.areas[i] = P.area(qs);
			solver.areasDouble[i] = solver.areas[i].getDouble();
		}
		solver.edgesSkeleton = Solver.createEdgeMap(poly);
		solver.debug = debug;
		solver.FOLD = fold;
		solver.strategy = strategy;
		solver.bit = bit;
		if (seed == 0) {
			seed = new Random().nextLong();
			System.err.printf("Seed = %d%n", seed);
		}
		solver.rand = new Random(seed);
		solver.sc = sc;
		Stat.start("solve");
		boolean res = solver.solve();
		Stat.end("solve");
		if (!res) {
			System.err.println("No solution found.");
			Debug.check(res);
		}
		System.err.println("Succeeded!");
	}
	
	public static void main(String[] args) {
		Stat.setShutdownHook();
		Main main = new Main();
		SetOpt.setOpt(main, args);
		try {
			main.run();
		} catch (Throwable e) {
			e.printStackTrace();
			System.err.println("Error!!!!!");
			System.exit(1);
		}
	}
	
}
