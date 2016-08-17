module Cpx  where

data Cpx k = Cpx {real, imag :: k}
  deriving (Show, Eq)

instance (Num k) => Num (Cpx k)  where
  (Cpx x1 y1) + (Cpx x2 y2) = Cpx (x1+x2) (y1+y2)
  (Cpx x1 y1) - (Cpx x2 y2) = Cpx (x1-x2) (y1-y2)
  (Cpx x1 y1) * (Cpx x2 y2) = Cpx (x1*x2 - y1*y2) (x1*y2 + y1*x2)
  abs = undefined
  signum = undefined
  fromInteger = undefined

abs2 (Cpx x y) = x*x + y*y
conj (Cpx x y) = Cpx x (-y)

-----

innerProduct :: Num k => Cpx k -> Cpx k -> k
innerProduct z w = real (z * conj w)

projection :: Fractional k => Cpx k -> Cpx k -> Cpx k
projection z w = z * Cpx (innerProduct z w / abs2 z) 0
