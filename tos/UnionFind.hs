module UnionFind  where

import Control.Monad.State
import Data.Functor.Identity
import Data.Maybe
import qualified Data.Map as M

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

-----

spanningForest :: (Ord v) => [v] -> [(v,v)] -> [(v,v)]
spanningForest vs es =
  runUnionFind $ do
    mapM ufFresh vs
    es' <- forM es $ \ (v,w) -> do
      unified <- ufUnify v w
      return $ if unified then Just (v,w) else Nothing
    return $ catMaybes es'
