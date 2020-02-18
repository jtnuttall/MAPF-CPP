import System.Environment (getArgs)

thd :: (a, b, c) -> c
thd (_, _, c) = c

noPath :: [Int] -> [Int] -> [(Int, Int, Int)]
noPath = (filter ((-1 ==) . thd) .) . zip3 [1 ..]

putExample :: (Show a, Show b, Show c) => (a, b, c) -> IO ()
putExample (i, t, c) =
  putStrLn $
  "Experiment " ++ show i ++ " with time " ++ show t ++ " and cost " ++ show c

putTaskCompare :: (Int, (Int, Int), (Int, Int)) -> IO()
putTaskCompare (i, (t2t, t2c), (t3t, t3c)) = do
    putStr "\ttask2: "
    putExample (i, t2t, t2c)
    putStr "\ttask3: "
    putExample (i, t3t, t3c)
    putStrLn ""

main :: IO ()
main = do
  [task2Times, task2Costs, task3Times, task3Costs] <-
    map (read . ("[" ++) . (++ "]")) <$> getArgs
  let task2Total = zip task2Times task2Costs
      task3Total = zip task3Times task3Costs
      task2NoPath = noPath task2Times task2Costs
      task3NoPath = noPath task3Times task3Costs
      task2Better =
        filter (\(_, t2, t3) -> snd t2 < snd t3 && snd t2 /= -1) $
          zip3 [1 ..] task2Total task3Total
      task2Faster = 
        filter (\(_, t2, t3) -> fst t2 < fst t3) $
          zip3 [1 ..] task2Total task3Total
      task3Faster =
        filter (\(_, t2, t3) -> fst t2 > fst t3) $
          zip3 [1 ..] task2Total task3Total
  putStrLn "Task 2 shows no path:"
  mapM_ ((putStr "\t" >>) . putExample) task2NoPath
  putStrLn "Task 3 shows no path:"
  mapM_ ((putStr "\t" >>) . putExample) task3NoPath
  putStrLn $ "Task 2 shows better results in " 
    ++ show (length task2Better) ++ " example pairs:"
  mapM_ putTaskCompare task2Better
  putStrLn $ "Task 2 shows faster time in " 
    ++ show (length task2Faster) ++ " example pairs:"
  mapM_ putTaskCompare task2Faster
  putStrLn $ "Task 3 shows faster time in " 
    ++ show (length task3Faster) ++ " example pairs:"
  mapM_ putTaskCompare task3Faster
