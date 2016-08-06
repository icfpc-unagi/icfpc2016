package tc.wata.util;

import java.util.*;

/**
 * java.util.Randomより5倍くらい高速
 */
public class Xorshift extends Random {
	
	int x = 123456789, y = 362436069, z = 521288629, w = 88675123;
	
	public Xorshift(int seed) {
		x = seed = 1812433253 * (seed ^ (seed >>> 30));
		y = seed = 1812433253 * (seed ^ (seed >>> 30)) + 1;
		z = seed = 1812433253 * (seed ^ (seed >>> 30)) + 2;
		w = seed = 1812433253 * (seed ^ (seed >>> 30)) + 3;
	}
	
	public Xorshift() {
		this(new Random().nextInt());
	}
	
	@Override
	protected int next(int bits) {
		int t = (x ^ (x << 11));
		x = y;
		y = z;
		z = w;
		return (w = (w ^ (w >>> 19)) ^ (t ^ (t >>> 8))) >>> (32 - bits);
	}
	
}
