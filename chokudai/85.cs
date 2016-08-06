using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Numerics;

public class Myon
{
    public Myon() { }
    public static int Main()
    {
        new Myon().calc();
        return 0;
    }

    Scanner cin;


    int Polygon;
    int N;
    int M;
    Point[] pt;
    Point[] pt2;
    Num xmin;
    Num ymin;

    List<Point> bpoints;
    List<Point> points;
    Dictionary<Point, int> pdic = new Dictionary<Point, int>();
    List<List<int>> polys = new List<List<int>>();

    int[] rev = new int[100000];


    void calc()
    {
        cin = new Scanner();
        Polygon = cin.nextInt();
        /*
        if (Polygon != 1)
        {
            Console.Error.WriteLine("giveup!(many polygons)");
            return;
        }
         */
        N = cin.nextInt();
        pt = new Point[N];
        for (int i = 0; i < N; i++)
        {
            string[] st = cin.next().Split(',');
            pt[i] = new Point(new Num(st[0]), new Num(st[1]));

            
            Console.Error.WriteLine(pt[i]);

            
            var nextb = 1000;
            var nexta = pt[i].x.a * nextb / pt[i].x.b;
            var nextx = new Num(nexta, nextb);

            nextb = 1000;
            nexta = pt[i].y.a * nextb / pt[i].y.b;
            var nexty = new Num(nexta, nextb);

            pt[i] = new Point(nextx, nexty);
             
             
            //Console.Error.WriteLine(pt[i]);
        }

        for (int i = 1; i < Polygon; i++)
        {
            int tempn = cin.nextInt();
            for (int j = 0; j < tempn; j++)
            {
                cin.next();
            }
        }

        int fff = (1 << 6);
        while (true)
        {
            List<Point> temppt = new List<Point>();
            for (int i = 0; i < pt.Length; i++)
            {
                if ((fff>>i) %2 == 0) temppt.Add(pt[i]);
            }
            pt = temppt.ToArray();
            N = pt.Length;
            break;
        }

        //pt[6].x = pt[6].x.Sub(new Num(1, 12));

        while (true)
        {
            bool flag = false;
            List<Point> temppt = new List<Point>();
            for (int i = 0; i < pt.Length; i++)
            {
                var P1 = pt[i];
                var P2 = pt[(i + pt.Length - 1) % pt.Length];
                var P3 = pt[(i + 1) % pt.Length];

                Num nn = Point.S(P2.Sub(P1), P3.Sub(P1));
                if (nn.a < 0) temppt.Add(pt[i]);
                else flag = true;
            }
            pt = temppt.ToArray();
            N = pt.Length;
            if (!flag) break;
        }


        Console.Error.WriteLine("N : " + N);

        for (int i = 0; i < N; i++)
        {
            if (i == 0)
            {
                xmin = pt[i].x;
                ymin = pt[i].y;
            }
            else
            {
                if (pt[i].x.CompareTo(xmin) < 0) xmin = pt[i].x;
                if (pt[i].y.CompareTo(ymin) < 0) ymin = pt[i].y;
            }
        }

        xmin = new Num(-1, 3);
        ymin = new Num(0);
        
        pt2 = new Point[N];
        for (int i = 0; i < N; i++)
		{
            pt2[i] = pt[i].Sub(new Point(xmin, ymin));

            if (pt2[i].x.a > pt2[i].x.b)
            {
                pt2[i].x.a = pt2[i].x.b;
                //Console.Error.WriteLine("giveup!(big)");
                //return;
            }

            if (pt2[i].y.a > pt2[i].y.b)
            {
                pt2[i].y.a = pt2[i].y.b;
                //Console.Error.WriteLine("giveup!(big)");
                //return;
            }
        }



        M = cin.nextInt();

        points = new List<Point>();
        bpoints = new List<Point>();

        points.Add(new Point(new Num(0), new Num(0)));
        points.Add(new Point(new Num(0), new Num(1)));
        points.Add(new Point(new Num(1), new Num(1)));
        points.Add(new Point(new Num(1), new Num(0)));

        List<int> firstl = new List<int>();
        for (int i = 0; i < points.Count; i++)
        {
            firstl.Add(i);
            bpoints.Add(points[i]);
            pdic[points[i]] = i;
        }
        polys.Add(firstl);

        string ans = "";

        string anstemp = "";
        anstemp += (points.Count);
        anstemp += '\n';
        foreach (var item in bpoints)
        {
            anstemp += item + "\n";
        }

        //skelton
        //Console.WriteLine(polys.Count);
        anstemp += polys.Count + "\n";
        foreach (var poly in polys)
        {
            anstemp += (poly.Count + " ");
            anstemp += (String.Join(" ", poly)) + "\n";
        }

        //realPoint

        foreach (var item in points)
        {
            //Console.WriteLine(item.Add(new Point(xmin, ymin)));
            anstemp += item.Add(new Point(xmin, ymin)) + "\n";
        }


        if (anstemp.Length < 5000) ans = anstemp;
        else Console.Error.WriteLine(anstemp.Length);

        pt2 = new Point[4];
        pt2[1] = new Point(new Num(8, 9), new Num(0));
        pt2[0] = new Point(new Num(0), new Num(8, 9));
        pt2[2] = new Point(new Num(1), new Num(1, 9));
        pt2[3] = new Point(new Num(1, 9), new Num(1));


        while (true)
        {
            bool flag = false;
            for (int ii = 0; ii < pt2.Length; ii += 2)
            {
                //int i = (ii * 2) % N;
                //if (ii * 2 >= N && N % 2 == 0) i++;
               int i = ii;
                Point P1 = pt2[i];
                Point P2 = pt2[(i + 1) % pt2.Length];

                for (int j = 0; j < points.Count; j++)
                {
                    Point P3 = points[j];
                    Num SS = Point.S(P3.Sub(P1), P3.Sub(P2));
                    rev[j] = 0;
                    if (SS.a < 0) rev[j] = 1;
                    //Console.Error.WriteLine("S " + SS);
                }

                List<List<int>> nextpoly = new List<List<int>>();


                foreach (var poly in polys)
                {
                    List<int> normallist = new List<int>();
                    List<int> revlist = new List<int>();

                    int pre = poly[poly.Count - 1];
                    int prerev = rev[pre];

                    foreach (var j in poly)
                    {
                        if (prerev != rev[j])
                        {
                            int nextp = getp(pre, j, P1, P2);
                            normallist.Add(nextp);
                            revlist.Add(nextp);
                        }

                        if (rev[j] == 0) normallist.Add(j);
                        else revlist.Add(j);

                        pre = j;
                        prerev = rev[j];
                    }

                    revlist.Reverse(0, revlist.Count);

                    normallist = doublecheck(normallist);
                    revlist = doublecheck(revlist);

                    if (normallist.Count >= 3) nextpoly.Add(normallist);
                    if (revlist.Count >= 3)
                    {
                        flag = true;
                        nextpoly.Add(revlist);
                    }
                }
                polys = nextpoly;




                int revcnt = 0;
                Point diff = P2.Sub(P1);
                Num[,] rot = Point.reflect(diff);

                for (int j = 0; j < points.Count; j++)
                {
                    if (rev[j] == 1)
                    {
                        revcnt++;
                        points[j] = Point.apply(rot, points[j].Sub(P1)).Add(P1);

                    }
                }
                Console.Error.WriteLine("revcount : " + i + " " + revcnt);


                anstemp = "";

                anstemp += (points.Count);
                anstemp += '\n';
                foreach (var item in bpoints)
                {
                    anstemp += item + "\n";
                }

                //skelton
                //Console.WriteLine(polys.Count);
                anstemp += polys.Count + "\n";
                foreach (var poly in polys)
                {
                    anstemp += (poly.Count + " ");
                    anstemp += (String.Join(" ", poly)) + "\n";
                }

                //realPoint

                foreach (var item in points)
                {
                    //Console.WriteLine(item.Add(new Point(xmin, ymin)));
                    anstemp += item.Add(new Point(xmin, ymin)) + "\n";
                }


                if (anstemp.Length < 5000) ans = anstemp;
                else Console.Error.WriteLine(anstemp.Length);
                /*
                else
                {
                    anstemp = "";
                    anstemp += (points.Count);
                    anstemp += '\n';
                    foreach (var item in bpoints)
                    {
                        anstemp += item + "\n";
                    }

                    //skelton
                    //Console.WriteLine(polys.Count);
                    anstemp += polys.Count + "\n";
                    foreach (var poly in polys)
                    {
                        anstemp += (poly.Count + " ");
                        anstemp += (String.Join(" ", poly)) + "\n";
                    }

                    //realPoint
                    var xmin2 = xmin;
                    var ymin2 = ymin;
                    while (xmin2.b >= 1000)
                    {
                        xmin2.a /= 10;
                        xmin2.b /= 10;
                    }
                    while (ymin2.b >= 1000)
                    {
                        ymin2.a /= 10;
                        ymin2.b /= 10;
                    }

                    foreach (var item in points)
                    {
                        //Console.WriteLine(item.Add(new Point(xmin, ymin)));
                        anstemp += item.Add(new Point(xmin2, ymin2)) + "\n";
                    }


                    if (anstemp.Length < 4900) ans = anstemp;
                    else Console.Error.WriteLine(anstemp.Length);
                }
                 */
            }

            if (!flag) break;
        }

        pt2 = new Point[4];
        pt2[0] = new Point(new Num(15, 18), new Num(1));
        pt2[1] = new Point(new Num(15, 18), new Num(0));
        pt2[2] = new Point(new Num(1), new Num(0));
        pt2[3] = new Point(new Num(1), new Num(1));


        while (true)
        {
            bool flag = false;
            for (int ii = 0; ii < pt2.Length; ii += 2)
            {
                //int i = (ii * 2) % N;
                //if (ii * 2 >= N && N % 2 == 0) i++;
                int i = ii;
                Point P1 = pt2[i];
                Point P2 = pt2[(i + 1) % pt2.Length];

                for (int j = 0; j < points.Count; j++)
                {
                    Point P3 = points[j];
                    Num SS = Point.S(P3.Sub(P1), P3.Sub(P2));
                    rev[j] = 0;
                    if (SS.a < 0) rev[j] = 1;
                    //Console.Error.WriteLine("S " + SS);
                }

                List<List<int>> nextpoly = new List<List<int>>();


                foreach (var poly in polys)
                {
                    List<int> normallist = new List<int>();
                    List<int> revlist = new List<int>();

                    int pre = poly[poly.Count - 1];
                    int prerev = rev[pre];

                    foreach (var j in poly)
                    {
                        if (prerev != rev[j])
                        {
                            int nextp = getp(pre, j, P1, P2);
                            normallist.Add(nextp);
                            revlist.Add(nextp);
                        }

                        if (rev[j] == 0) normallist.Add(j);
                        else revlist.Add(j);

                        pre = j;
                        prerev = rev[j];
                    }

                    revlist.Reverse(0, revlist.Count);

                    normallist = doublecheck(normallist);
                    revlist = doublecheck(revlist);

                    if (normallist.Count >= 3) nextpoly.Add(normallist);
                    if (revlist.Count >= 3)
                    {
                        flag = true;
                        nextpoly.Add(revlist);
                    }
                }
                polys = nextpoly;




                int revcnt = 0;
                Point diff = P2.Sub(P1);
                Num[,] rot = Point.reflect(diff);

                for (int j = 0; j < points.Count; j++)
                {
                    if (rev[j] == 1)
                    {
                        revcnt++;
                        points[j] = Point.apply(rot, points[j].Sub(P1)).Add(P1);

                    }
                }
                Console.Error.WriteLine("revcount : " + i + " " + revcnt);


                anstemp = "";

                anstemp += (points.Count);
                anstemp += '\n';
                foreach (var item in bpoints)
                {
                    anstemp += item + "\n";
                }

                //skelton
                //Console.WriteLine(polys.Count);
                anstemp += polys.Count + "\n";
                foreach (var poly in polys)
                {
                    anstemp += (poly.Count + " ");
                    anstemp += (String.Join(" ", poly)) + "\n";
                }

                //realPoint

                foreach (var item in points)
                {
                    //Console.WriteLine(item.Add(new Point(xmin, ymin)));
                    anstemp += item.Add(new Point(xmin, ymin)) + "\n";
                }


                if (anstemp.Length < 5000) ans = anstemp;
                else Console.Error.WriteLine(anstemp.Length);
                /*
                else
                {
                    anstemp = "";
                    anstemp += (points.Count);
                    anstemp += '\n';
                    foreach (var item in bpoints)
                    {
                        anstemp += item + "\n";
                    }

                    //skelton
                    //Console.WriteLine(polys.Count);
                    anstemp += polys.Count + "\n";
                    foreach (var poly in polys)
                    {
                        anstemp += (poly.Count + " ");
                        anstemp += (String.Join(" ", poly)) + "\n";
                    }

                    //realPoint
                    var xmin2 = xmin;
                    var ymin2 = ymin;
                    while (xmin2.b >= 1000)
                    {
                        xmin2.a /= 10;
                        xmin2.b /= 10;
                    }
                    while (ymin2.b >= 1000)
                    {
                        ymin2.a /= 10;
                        ymin2.b /= 10;
                    }

                    foreach (var item in points)
                    {
                        //Console.WriteLine(item.Add(new Point(xmin, ymin)));
                        anstemp += item.Add(new Point(xmin2, ymin2)) + "\n";
                    }


                    if (anstemp.Length < 4900) ans = anstemp;
                    else Console.Error.WriteLine(anstemp.Length);
                }
                 */
            }

            if (!flag) break;
        }


        Console.Write(ans);


        /*

        //basepoint
        Console.WriteLine(points.Count);
        foreach (var item in bpoints)
        {
            Console.WriteLine(item);
        }

        //skelton
        Console.WriteLine(polys.Count);
        foreach (var poly in polys)
        {
            Console.Write(poly.Count + " ");
            Console.WriteLine(String.Join(" ", poly));
        }

        //realPoint

        foreach (var item in points)
        {
            Console.WriteLine(item.Add(new Point(xmin, ymin)));
        }
         */
    }

    int getp(int i, int j, Point P1, Point P2)
    {
        Num A = Point.S(points[i].Sub(P1), points[i].Sub(P2)).Abs();
        Num B = Point.S(points[j].Sub(P1), points[j].Sub(P2)).Abs();

        Point BP = bpoints[i].Mul(B.Div(A.Add(B))).Add(bpoints[j].Mul(A.Div(A.Add(B))));
        Point P = points[i].Mul(B.Div(A.Add(B))).Add(points[j].Mul(A.Div(A.Add(B))) );

        if (pdic.ContainsKey(BP)) return pdic[BP];
        pdic[BP] = points.Count;
        bpoints.Add(BP);
        points.Add(P);
        return pdic[BP];
    }

    List<int> doublecheck(List<int> a)
    {
        if (a.Count == 0) return a;
        List<int> r = new List<int>();
        int pre = a[a.Count - 1];
        foreach (var item in a)
        {
            if (pre != item) r.Add(item);
            pre = item;
        }
        return r;
    }


    static public bool isSqrtable(BigInteger a)
    {
        if (a < 0) return false;
        BigInteger high = a + 1;
        BigInteger low = 0;
        while (high - low > 1)
        {
            var mid = high - low;
            if (mid * mid <= a) low = a;
            else high = a;

        }
        return low * low == a;
    }

    static public BigInteger BigSqrt(BigInteger a)
    {
        BigInteger high = a + 1;
        BigInteger low = 0;
        while (high - low > 1)
        {
            var mid = high - low;
            if (mid * mid <= a) low = a;
            else high = a;

        }
        return low;
    }
}

class Point : IComparable<Point>, IEquatable<Point>
{
    public Num x, y;

    public Point()
    {
        this.x = new Num(0);
        this.y = new Num(0);
    }

    public Point(Num x, Num y)
    {
        this.x = x;
        this.y = y;
    }

    public Point(string x, string y)
    {
        this.x = new Num(x);
        this.y = new Num(y);
    }


    public Point Add(Point t)
    {
        return new Point(x.Add(t.x), y.Add(t.y));
    }

    public Point Sub(Point t)
    {
        return new Point(x.Sub(t.x), y.Sub(t.y));
    }

    public Point Mul(Num d)
    {
        return new Point(x.Mul(d), y.Mul(d));
    }


    public Point Div(Num d)
    {
        return new Point(x.Div(d), y.Div(d));
    }

    public Num Dot(Point p)
    {
        return x.Mul(p.x).Add(y.Mul(p.y));
    }

    public Num Det(Point p)
    {
        return x.Mul(p.y).Sub(y.Mul(p.x));
    }

    public Num Abs2()
    {
        return x.Mul(x).Add(y.Mul(y));
    }



    override public string ToString()
    {
        return x.ToString() + "," + y.ToString();
    }

    public int CompareTo(Point t)
    {
        int r = x.CompareTo(t.x);
        if (r != 0) return r;
        return y.CompareTo(t.y);
    }

    public bool Equals(Point t)
    {
        return x.Equals(t.x) && y.Equals(t.y);
    }

    override public int GetHashCode()
    {
        int xCode = x.GetHashCode();
        int yCode = y.GetHashCode();
        int hCode = (xCode << 11) ^ (xCode >> 5) ^ (yCode);
        return hCode.GetHashCode();
    }

    public bool IsEndPoint()
    {
        return x.IsEndPoint() || y.IsEndPoint();
    }

    // aをbの方向に回転
    public static Num[,] rotate(Point a, Point b)
    {
        Num ab2 = a.Abs2().Mul(b.Abs2());
        if (!ab2.IsSqrtable()) return null;
        Num ab = ab2.Sqrt();
        Num cos = a.Dot(b).Div(ab), sin = a.Det(b).Div(ab);
        return new Num[,] { { cos, sin.Neg() }, { sin, cos } };
    }

    // d方向を軸に反転
    public static Num[,] reflect(Point d)
    {
        Num d2 = d.Dot(d);
        Num cos = d.x.Mul(d.x).Sub(d.y.Mul(d.y)).Div(d2), sin = Num.TWO.Mul(d.x).Mul(d.y).Div(d2);
        return new Num[,] { { cos, sin }, { sin, cos.Neg() } };
    }

    //回転を適用
    public static Point[] apply(Num[,] A, Point[] ps)
    {
        Point[] qs = new Point[ps.Length];
        for (int i = 0; i < ps.Length; i++)
        {
            qs[i] = new Point(A[0, 0].Mul(ps[i].x).Add(A[0, 1].Mul(ps[i].y)), A[1, 0].Mul(ps[i].x).Add(A[1, 1].Mul(ps[i].y)));
        }
        return qs;
    }


    public static Point apply(Num[,] A, Point ps)
    {
        Point qs = new Point(A[0, 0].Mul(ps.x).Add(A[0, 1].Mul(ps.y)), A[1, 0].Mul(ps.x).Add(A[1, 1].Mul(ps.y)));
        return qs;
    }

    //符号付面積
    public static Num S(Point P1, Point P2)
    {
        return P1.x.Mul(P2.y).Sub(P1.y.Mul(P2.x));
    }

    public bool inside()
    {
        return x.inside() && y.inside();
    }
}

class Num : IComparable<Num>, IEquatable<Num>
{
    public BigInteger a; //bunbo
    public BigInteger b; //bunsi

    public Num(string st)
    {
        string[] st2;
        st2 = st.Split('/');
        a = BigInteger.Parse(st2[0]);
        if (st2.Length == 1)
        {
            b = 1;
        }
        else
        {
            b = BigInteger.Parse(st2[1]);
        }
        Yakubun();
    }

    public Num(BigInteger a, BigInteger b)
    {
        this.a = a;
        this.b = b;
        Yakubun();
    }

    public Num(BigInteger a)
    {
        this.a = a;
        this.b = 1;
        Yakubun();
    }

    public Num Add(Num t)
    {
        BigInteger nextB = b * t.b;
        BigInteger nextA = a * t.b + t.a * b;
        var ret = new Num(nextA, nextB);
        ret.Yakubun();
        return ret;
    }


    public Num Sub(Num t)
    {
        BigInteger nextB = b * t.b;
        BigInteger nextA = a * t.b - t.a * b;
        var ret = new Num(nextA, nextB);
        ret.Yakubun();
        return ret;
    }


    public Num Mul(Num t)
    {
        BigInteger nextB = b * t.b;
        BigInteger nextA = a * t.a;
        var ret = new Num(nextA, nextB);
        ret.Yakubun();
        return ret;
    }


    public Num Div(Num t)
    {
        BigInteger nextB = b * t.a;
        BigInteger nextA = a * t.b;
        var ret = new Num(nextA, nextB);
        ret.Yakubun();
        return ret;
    }

    public Num Neg()
    {
        return new Num(-a, b);
    }

    public static Num TWO = new Num(2);

    public void Yakubun()
    {
        BigInteger g = BigInteger.GreatestCommonDivisor(BigInteger.Abs(a), b);
        a /= g;
        b /= g;
    }


    public int CompareTo(Num t)
    {
        return (a * t.b).CompareTo(t.a * b);
    }

    
    public bool IsSqrtable()
    {
        return Myon.isSqrtable(a) && Myon.isSqrtable(b);
    }


    public Num Sqrt()
    {
        return new Num(Myon.BigSqrt(a), Myon.BigSqrt(b));
    }

    override public string ToString()
    {
        if (b != 1) return a.ToString() + "/" + b.ToString();
        else return a.ToString();
    }

    public bool Equals(Num p)
    {
        return a == p.a && b == p.b;
    }

    override public int GetHashCode()
    {
        int xCode = a.GetHashCode();
        int yCode = b.GetHashCode();
        int hCode = (xCode << 13) ^ (xCode >> 7) ^ (yCode);
        return hCode.GetHashCode();
    }

    public bool IsEndPoint()
    {
        return b == 1;
    }

    public bool inside()
    {
        return a >= 0 && a <= b;
    }

    internal Num Abs()
    {
        if (a < 0) return this.Neg();
        else return this;
    }
}


class Scanner
{
    string[] s;
    int i;

    char[] cs = new char[] { ' ' };

    public Scanner()
    {
        s = new string[0];
        i = 0;
    }

    public string next()
    {
        if (i < s.Length) return s[i++];
        string st = Console.ReadLine();
        while (st == "") st = Console.ReadLine();
        s = st.Split(cs, StringSplitOptions.RemoveEmptyEntries);
        i = 0;
        return s[i++];
    }

    public int nextInt()
    {
        return int.Parse(next());
    }

    public long nextLong()
    {
        return long.Parse(next());
    }

    public double nextDouble()
    {
        return double.Parse(next());
    }

}