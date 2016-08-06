import static java.math.BigInteger.*;

import java.math.*;
import java.util.*;

import tc.wata.debug.*;

//±1/0の比較には注意すること
//-1/0<…<1/0だが、-1/0=1/0となる
//以下の実装では-1/0を1/0に変換している
public class R implements Comparable<R> {
	
	public static final R ZERO = new R(0);
	public static final R ONE = new R(1);
	public static final R TWO = new R(2);
	
	public BigInteger num, den;
	public R(BigInteger num, BigInteger den) {
		this.num = num; this.den = den;
		red();
	}
	public R(int a) {
		this(valueOf(a), BigInteger.ONE);
	}
	public R(String s) {
		if (s.indexOf('/') >= 0) {
			num = new BigInteger(s.split("/")[0].trim());
			den = new BigInteger(s.split("/")[1].trim());
		} else {
			num = new BigInteger(s.trim());
			den = BigInteger.ONE;
		}
	}
	public void red() {
		BigInteger gcd = num.gcd(den);
		if (gcd.signum() != 0) {
			num = num.divide(gcd); den = den.divide(gcd);
		}
		if (den.signum() < 0 || (den.signum() == 0 && num.signum() < 0)) {
			num = num.negate();	den = den.negate();
		}
	}
	public R add(R r) {
		return new R(num.multiply(r.den).add(r.num.multiply(den)), den.multiply(r.den));
	}
	public R sub(R r) {
		return new R(num.multiply(r.den).subtract(r.num.multiply(den)), den.multiply(r.den));
	}
	public R mul(R r) {
		return new R(num.multiply(r.num), den.multiply(r.den));
	}
	public R div(R r) {
		return new R(num.multiply(r.den), den.multiply(r.num));
	}
	public R neg() {
		return new R(num.negate(), den);
	}
	public static BigInteger sqrt(BigInteger a) {
		if (a.signum() == 0 || a.compareTo(BigInteger.ONE) == 0) return a;
		Debug.check(a.signum() > 0);
		BigInteger lb = BigInteger.ZERO, ub = a;
		while (ub.subtract(lb).compareTo(BigInteger.ONE) > 0) {
			BigInteger mid = lb.add(ub).divide(BigInteger.valueOf(2));
			int comp = mid.multiply(mid).compareTo(a);
			if (comp == 0) return mid;
			else if (comp < 0) lb = mid;
			else ub = mid;
		}
		return null;
	}
	public R sqrt() {
		BigInteger num2 = sqrt(num), den2 = sqrt(den);
		if (num2 == null || den2 == null) return null;
		return new R(num2, den2);
	}
	public int signum() {
		return num.signum();
	}
	public int compareTo(R r) {
		return (num.multiply(r.den).compareTo(r.num.multiply(den)));
	}
	public boolean equals(Object o) {
		R r = (R)o;
		return num.equals(r.num) && den.equals(r.den);
	}
	public int hashCode() {
		return num.hashCode() * 31 + den.hashCode();
	}
	public String toString() {
		if (den.equals(ONE)) return num.toString();
		return num + "/" + den;
	}
	public static R[][] mul(R[][] A, R[][] B) {
		R[][] C = new R[A.length][B[0].length];
		for (int i = 0; i < C.length; i++) {
			for (int j = 0; j < C[i].length; j++) {
				C[i][j] = R.ZERO;
				for (int k = 0; k < A[i].length; k++) {
					C[i][j] = C[i][j].add(A[i][k].mul(B[k][j]));
				}
			}
		}
		return C;
	}
	public double getDouble() {
		return num.doubleValue() / den.doubleValue();
	}
}
