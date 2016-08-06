package tc.wata.util;

import static java.lang.Math.*;

import java.lang.reflect.*;
import java.util.*;

/**
 * 乱数生成用ユーティリティ
 */
public class Rand {
	
	private Rand() {
	}
	
	private static Random rand = new Random(64327896);
	
	/**
	 * シードをランダムに設定
	 */
	public static void randomSeed() {
		rand = new Random();
	}
	
	/**
	 * 値が[min,max]のランダムint値を作成する (max - min < 2^31)
	 */
	public static int random(int min, int max) {
		return rand.nextInt(max - min + 1) + min;
	}
	
	/**
	 * 値が[min,max]のランダムlong値を作成する (若干一様でない)
	 */
	public static long random(long min, long max) {
		return abs(rand.nextLong()) % (max - min + 1) + min;
	}
	
	/**
	 * 値が[min,max]で長さnのランダムint配列を作成する
	 */
	public static int[] randomArray(int min, int max, int n) {
		int[] is = new int[n];
		for (int i = 0; i < n; i++) is[i] = random(min, max);
		return is;
	}
	
	/**
	 * 値が[min,max]で長さnのランダムlong配列を作成する (若干一様でない)
	 */
	public static long[] randomArray(long min, long max, int n) {
		long[] is = new long[n];
		for (int i = 0; i < n; i++) is[i] = random(min, max);
		return is;
	}
	
	/**
	 * 文字列sをもとに長さnのランダム文字列を作成する
	 */
	public static String randomString(String s, int n) {
		char[] cs = new char[n];
		for (int i = 0; i < n; i++) cs[i] = s.charAt(rand.nextInt(s.length()));
		return new String(cs);
	}
	
	/**
	 * 文字列sをもとにn×mのランダム文字列を作成する
	 */
	public static String[] randomArray(String s, int n, int m) {
		String[] ss = new String[n];
		for (int i = 0; i < n; i++) ss[i] = randomString(s, m);
		return ss;
	}
	
	/**
	 * ランダムに並び替える
	 */
	public static void shuffle(Object os) {
		int n = Array.getLength(os);
		for (int i = 0; i < n; i++) {
			int j = i + rand.nextInt(n - i);
			Object t = Array.get(os, i);
			Array.set(os, i, Array.get(os, j));
			Array.set(os, j, t);
		}
	}
	
}
