package tc.wata.util;

import static java.util.Arrays.*;

import java.util.*;

/**
 * 全列挙
 */
public class Enumerate {
	
	private Enumerate() {
	}
	
	/**
	 * {0,...,N-1}の部分集合
	 */
	public static int[][] subset(int N) {
		int[][] res = new int[1 << N][];
		res[0] = new int[0];
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < 1 << i; j++) {
				res[j | 1 << i] = copyOf(res[j], res[j].length + 1);
				res[j | 1 << i][res[j].length] = i;
			}
		}
		return res;
	}
	
	/**
	 * {0,1,...,N-1}^M
	 */
	public static int[][] pow(int N, int M) {
		if (M == 0) return new int[1][0];
		int[][] ps = pow(N, M - 1);
		int[][] res = new int[ps.length * N][];
		for (int i = 0; i < ps.length; i++) {
			for (int j = 0; j < N; j++) {
				res[i * N + j] = copyOf(ps[i], M);
				res[i * N + j][M - 1] = j;
			}
		}
		return res;
	}
	
	/**
	 * {0,...,N-1}の順列
	 */
	public static int[][] permutation(int N) {
		int[] is = new int[N];
		for (int i = 0; i < N; i++) is[i] = i;
		ArrayList<int[]> list = new ArrayList<>();
		do {
			list.add(is.clone());
		} while (Utils.nextPermutation(is));
		return list.toArray(new int[0][]);
	}
	
}
