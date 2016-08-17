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

instance (Fractional k) => Fractional (Cpx k)  where
  recip z@(Cpx x y) = let a2 = abs2 z in Cpx (x / a2) (- y / a2)
  fromRational = undefined

abs2 (Cpx x y) = x*x + y*y
conj (Cpx x y) = Cpx x (-y)

-----

innerProduct :: Num k => Cpx k -> Cpx k -> k
innerProduct z w = real (conj z * w)

outerProduct :: Num k => Cpx k -> Cpx k -> k
outerProduct z w = imag (conj z * w)

projection :: Fractional k => Cpx k -> Cpx k -> Cpx k
projection z w = z * Cpx (innerProduct z w / abs2 z) 0
