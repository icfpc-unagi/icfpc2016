{-# LANGUAGE OverloadedStrings #-}
-- {-# LANGUAGE ScopedTypeVariables #-}
-- {-# LANGUAGE DeriveGeneric #-}


import Control.Applicative
import Control.Exception.Base
import Control.Monad
import Data.Aeson
import Data.Complex
import Data.List
import Data.Ratio
import qualified Data.ByteString.Lazy as B
import System.IO

import Cpx

seeds = ["010", "002", "006", "012", "016", "024", "026", "035", "041", "042", "045", "063", "070", "074", "088", "097", "106", "112"]

scale = 48

-- data Point = Point { x :: R, y :: R } deriving (Show, Generic)
newtype Point = Point C
instance ToJSON Point  where
  toJSON (Point (x :+ y)) = object ["x" .= x, "y" .= y]


main = do
  -- dat <- sequence [main1 $ "./tmp/oru-24-"++show n ++"-33-10.txt" | n<-[0..7]]
  let filess = [["./tmp/oru-24-"++show m ++"-33-"++seed++".txt" | m<-[0..7]] | seed <- seeds]
  putStrLn "data=[];"
  zipWithM_ main' [0..] filess

main' n files = do
  dat <- sequence $ zipWith main2 (head files : files) files  -- (0,0),(0,1),(1,2),(2,3),...
  putStr $ "data["++show n++"]="
  B.putStr . encode . toJSON $ dat
  putStrLn ";"

main1 file = do
  (vs, ps, ws) <- readSolution file
  return $ object
    [ "before" .= drawPolygon ps vs
    , "after" .= drawPolygon ps ws
    ]

main2 file0 file1 = do
  f <- solToMap <$> readSolution file0
  (vs, ps, w1s) <- readSolution file1
  let w0s = map f vs
  return $ object
    [ "oranai" .= drawPolygon ps vs
    , "oru" .= drawPolygon2 ps w0s w1s
    ]

solToMap (vs, ps, ws) v = w  where
  (Just p) = flip find ps $ \ p -> contain (map (vs !!) p) v
  [i0,i1,i2] = map (p!!) [0,1,2]
  [v0,v1,v2] = map (vs!!) [i0,i1,i2]
  [w0,w1,w2] = map (ws!!) [i0,i1,i2]
  -- a = (w1 - w0) / (v1 - v0)
  g = if ccw v0 v1 v2 == ccw w0 w1 w2 then id else conj
  w = g ((v - v0) / (v1 - v0)) * (w1 - w0) + w0

ccw a b c = signum $ outerProduct (b-a) (c-a)

contain xs x = all f $ zip xs (tail xs ++ [head xs])  where
  f (y,z) = ccw y z x >= 0

toPoint = Point . (* scale) . toComplex

drawPolygon ps vs =
  [ map (toPoint . (vs !!)) p
  | p <- ps
  ]

drawPolygon2 ps vs ws =
  [ [object ["t0" .= toPoint (vs !! i), "t1" .= toPoint (ws !! i)] | i <- p]
  | p <- ps
  ]
-- (\z -> Point (realPart z) (imagPart z))

type Z = Int
type Q = Rational
type R = Double
type S = String
type C = Complex R


hReadLn h = hGetLine h >>= readIO

readSolution file = do
  h <- openFile file ReadMode
  nv <- hReadLn h
  vs <- replicateM nv $ parseV <$> hGetLine h
  np <- hReadLn h
  ps <- replicateM np $ parseP <$> hGetLine h
  ws <- replicateM nv $ parseV <$> hGetLine h
  return (vs, ps, ws)


type Qi = Cpx Q

toComplex :: Qi -> C
toComplex (Cpx x y) = (fromRational x) :+ (fromRational y)


parseV :: S -> Qi
parseV str = let
  (xstr,(_:ystr)) = break (== ',') str
  in
    Cpx (parseQ xstr) (parseQ ystr)

parseQ :: S -> Q
parseQ str
 | '/' `elem`str = let
  (nstr,(_:dstr)) = break (== '/') str
  in
    read nstr % read dstr
 | otherwise = read str % 1

-- Polygon (list of Vertex IDs)
parseP :: S -> [Z]
parseP str = let
  (nstr:istrs) = words str
  is = map read istrs
  in
    assert (read nstr == length is) $ is

