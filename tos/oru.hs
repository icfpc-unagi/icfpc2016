{-# LANGUAGE ScopedTypeVariables #-}
import Control.Applicative
import Control.Exception.Base
import Control.Monad
import Control.Monad.State
import Control.DeepSeq
import Data.Array
import Data.Bits
import Data.Char
import Data.Functor.Identity
import Data.List
import Data.Maybe
import Data.Monoid
import Data.Ord
import Data.Ratio
import Data.Tree
import Data.Tuple
import qualified System.IO
import qualified Data.Map as M
import qualified Data.Set as S
import qualified Data.Sequence as Q
import qualified Data.ByteString.Char8 as B

main = do
  nv :: Z <- readLn
  vs <- replicateM nv $ parseV <$> getLine
  np :: Z <- readLn
  ps <- replicateM np $ parseP <$> getLine
  let
    pixs = zip ps [0..]
    padj =
      [(ixp,ixq) | (p,ixp) <- pixs, (q,ixq) <- pixs,
        not . null $ commonEdge p q]
      -- any (\(v,w) -> (w,v)`elem`(boundary q)) (boundary p)]
    theTree = bfsEdges . rooted 0 $ spanningForest [0..np-1] padj
    -- parents = [fst <$> find ((== ix) . snd) theTree | ix<-[0..np-1]]
  -- mapM_ print vs
  -- mapM_ print ps
  -- mapM_ print pixs
  -- mapM_ print padj
  -- mapM_ print $ spanningForest [0..np-1] padj
  -- print theTree
  -- print parents
  let
    -- linear0 = Lin (Cpx (4/5) (3/5)) (Cpx (1/3) (1/2))
    -- x^2 + y^2 should be 1
    linear0 = mempty
    linears = array (0,np-1) $
      (0, linear0) :
      [(j, (linears!i) <> mirror (vs!!v) (vs!!w))
      | (i,j) <- theTree, let [(v,w)] = commonEdge (ps!!i) (ps!!j)]
  -- print linears
  let
    vsNew =
      [linApply f v
      | (v,i) <- zip vs [0..]
      , let f = head [linears!j | (p,j) <- zip ps [0..], i`elem`p]
      ]
    vsNew2 = let
      mx = minimum $ map real vsNew
      my = minimum $ map imag vsNew
      in  map (subtract $ Cpx mx my) vsNew
  --
  print nv
  mapM_ (putStrLn . prettyV) vs
  print np
  mapM_ (putStrLn . (\ xs -> unwords $ (show $ length xs) : map show xs)) ps
  mapM_ (putStrLn . prettyV) vsNew2

type Qi = Cpx Q

data Lin = Lin Qi Qi | AntiLin Qi Qi
  deriving Show

linApply (Lin a b) z = a * z + b
linApply (AntiLin a b) z = a * conj z + b

instance Monoid Lin  where
  mempty = Lin (Cpx 1 0) (Cpx 0 0)
  mappend (Lin a1 b1) (Lin a2 b2) = Lin (a1*a2) (a1*b2 + b1)
  mappend (Lin a1 b1) (AntiLin a2 b2) = AntiLin (a1*a2) (a1*b2 + b1)
  mappend (AntiLin a1 b1) (Lin a2 b2) = AntiLin (a1*conj a2) (a1*conj b2 + b1)
  mappend (AntiLin a1 b1) (AntiLin a2 b2) = Lin (a1*conj a2) (a1*conj b2 + b1)

mirror z w = AntiLin a b  where
  a = normalizedSquare (w-z)
  b = Cpx 2 0 * (z - projection (w-z) z)
  normalizedSquare z = z * z * Cpx (1 / abs2 z) 0

innerProduct z w = real (z * conj w)
projection z w = z * Cpx (innerProduct z w / abs2 z) 0

    

-- p: polygon, return: edges
boundary :: [Z] -> [(Z,Z)]
boundary p = zip p (tail p ++ [head p])

-- p,q: polygon, return: [] or [common edge] (at most unique)
commonEdge p q = flip filter (boundary p) $ \(v,w) -> (w,v)`elem`(boundary q)

----- read / write
parseV :: S -> Qi
parseV str = let
  (xstr,(_:ystr)) = break (== ',') str
  in
    Cpx (parseQ xstr) (parseQ ystr)

parseQ :: S -> Q
parseQ str = let
  (nstr,(_:dstr)) = break (== '/') str
  in
    read nstr % read dstr

-- Polygon (list of Vertex IDs)
parseP :: S -> [Z]
parseP str = let
  (nstr:istrs) = words str
  is = map read istrs
  in
    assert (read nstr == length is) $ is

prettyV :: Qi -> S
prettyV (Cpx x y) = prettyQ x ++ ", " ++ prettyQ y

prettyQ :: Q -> S
prettyQ x
  | d == 1    = show n
  | otherwise = show n ++ "/" ++ show d
  where
    n = numerator x
    d = denominator x
  

-- tree -> rooted tree
rooted root es = foo root (-1)  where
  foo v p = Node v [foo w v | w <- [b | (a,b) <- es, a == v] ++ [a | (a,b) <- es, b == v], w /= p]

bfsEdges (Node v cs) = [(v,w) | w <- map rootLabel cs] ++ concatMap bfsEdges cs

type Z = Int
type Q = Rational
type R = Double
type S = String

spanningForest :: (Ord v) => [v] -> [(v,v)] -> [(v,v)]
spanningForest vs es =
  runUnionFind $ do
    mapM ufFresh vs
    es' <- forM es $ \ (v,w) -> do
      unified <- ufUnify v w
      return $ if unified then Just (v,w) else Nothing
    return $ catMaybes es'

-----  Complex
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

-----  Union-find
type UnionFindT v m a = StateT (M.Map v (UnionFindVal v)) m a
newtype UnionFindVal v = UnionFindVal v

runUnionFindT :: (Monad m) => UnionFindT v m a -> m a
runUnionFindT = flip evalStateT $ M.empty

runUnionFind = runIdentity . runUnionFindT

ufFresh :: (Monad m, Ord v) => v -> UnionFindT v m ()
ufFresh v = modify $ M.insert v (UnionFindVal v)

ufClass :: (Monad m, Ord v) => v -> UnionFindT v m v
ufClass v = do
  (UnionFindVal pv) <- gets (M.! v)
  if v == pv
    then return v
    else do
      c <- ufClass pv
      modify $ M.insert v (UnionFindVal c)
      return c

ufUnify v w = do
  cv <- ufClass v
  cw <- ufClass w
  modify $ M.insert cw (UnionFindVal cv)
  return $ cv /= cw
