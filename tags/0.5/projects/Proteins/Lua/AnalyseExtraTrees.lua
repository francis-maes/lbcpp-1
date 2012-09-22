require 'Statistics'
require 'Context'

local function getAttributes(node)
  local res = {}
  local n = #(node.results)
  for i=1,n do
    local pair = node.results[i]
    res[pair.first] = pair.second
  end
  return res
end

local function findSubNode(node, description)
  local n = #(node.subItems)
  for i=1,n do
    if node.subItems[i]:toShortString() == description then
      return node.subItems[i]
    end
  end
  return nil
end

local function getScoreToMinimize(scoreObject)
  return scoreObject:getScore()
end

local function getScoreToMaximize(scoreObject)
  return 1 - scoreObject:getScore()
end

local function getSensitivity(scoreObject)
  return 1 - (scoreObject.truePositive / (scoreObject.truePositive + scoreObject.falseNegative))
end

local function getSpecificity(scoreObject)
  return 1 - (scoreObject.trueNegative / (scoreObject.trueNegative + scoreObject.falsePositive))
end

local function getBestSensitivity(scoreObject)
  return getSensitivity(scoreObject.bestConfusionMatrix)
end

local function getBestSpecificity(scoreObject)
  return getSpecificity(scoreObject.bestConfusionMatrix)
end

local function getScores(traces, nodeName, scoresOfInterest)
  -- Init. results
  local res = {}
  for scoreName in pairs(scoresOfInterest) do
    res[scoreName] = Statistics.meanAndVariance()
  end
  -- Read scores
  for fold, trace in pairs(traces) do
    if trace ~= nil then
      local node = findSubNode(trace.root.subItems[1], nodeName)
      if node == nil then
        if nodeName == "EvaluateTrain" then
          return res
        else
          node = trace.root.subItems[1].subItems[4]
        end
      end
      -- Observe scores
      scoresNode = node.results[1].second
      for scoreName, scoreInfo in pairs(scoresOfInterest) do
        res[scoreName]:observe(scoreInfo.getScore(scoresNode.scores[scoreInfo.index]))
      end
    end
  end
  return res
end

local function getComplexity(traces)
  local res = Statistics.meanAndVariance()
  for fold, trace in pairs(traces) do
    if trace ~= nil then
      local complexity = findSubNode(trace.root.subItems[1], "Training").subItems[4].subItems[1].results[5].second
      res:observe(complexity)
    end
  end
  return res
end

-- file name: filePrefix .. varValues[.] .. filePostfix .. "_Foldx.trace"
local function main(varName, varValues, filePrefix, filePostfix, numFolds)
  local scoresOfInterest = {}
--  scoresOfInterest["1 - Qp (Greedy 6)"] = {index = 5, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Qp"] = {index = 6, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Qp (Greedy 20)"] = {index = 26, getScore = getScoreToMinimize}
--  for L = 1, 24 do
--    scoresOfInterest["Greedy " .. L] = {index = 6 + L, getScore = getScoreToMinimize}
--  end
--  scoresOfInterest["1 - Q2"] = {index = 1, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Q2 (Bias form test)"] = {index = 2, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Sensitivity"] = {index = 1, getScore = getSensitivity}
--  scoresOfInterest["1 - Specificity"] = {index = 1, getScore = getSpecificity}
--  scoresOfInterest["1 - Sensitivity (Bias form test)"] = {index = 3, getScore = getBestSensitivity}
--  scoresOfInterest["1 - Specificity (Bias from test)"] = {index = 3, getScore = getBestSpecificity}

  scoresOfInterest["CBS"] = {index = 1, getScore = getScoreToMaximize}
--  scoresOfInterest["CBS S&S"] = {index = 3, getScore = getScoreToMaximize}
  scoresOfInterest["Qp (Perfect)"] = {index = 8, getScore = getScoreToMaximize}
--  scoresOfInterest["Q2"] = {index = 4, getScore = getScoreToMaximize}
  scoresOfInterest["OxyDSB Qp (Perfect)"] = {index = 10, getScore = getScoreToMaximize}
--  scoresOfInterest["Q2 (Bias form test)"] = {index = 5, getScore = getScoreToMaximize}
  for i = 1,#varValues do
    context:enter(varName .. ": " .. varValues[i])
    -- Load traces
    local traces = {}
    for fold = 0, numFolds do
      local fileName = filePrefix .. varValues[i] .. filePostfix .. "_Fold" .. fold .. ".trace"
      traces[fold] = lbcpp.Object.fromFile(fileName)
      if traces[fold] == nil then
        context:warning("Missing file: " .. fileName)
      end
    end

    context:result(varName, varValues[i])
    trainScoreStat = {} --getScores(traces, "EvaluateTrain", scoresOfInterest)
    testScoreStat = getScores(traces, "EvaluateTest", scoresOfInterest)
--    complexityStat = getComplexity(traces)

    -- Results
    for scoreName, scoreStat in pairs(trainScoreStat) do
      context:result("Train: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Train: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

    for scoreName, scoreStat in pairs(testScoreStat) do
      context:result("Test: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Test: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

--    context:result("Complexity (Mean)", complexityStat:getMean())
--    context:result("Complexity (StdDev)", complexityStat:getStandardDeviation())

    context:leave()
  end
end

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/1203XX-DSBWithExtraTreesAndPerfectMatching/"
local numFolds = 9

--local winSizes = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35}--, 37, 39, 41, 43, 45}


dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/120619-CBSBasedDSB/"
main("ODSB", {""}, dir .. "x3_pssm15_csp17_K0_1000T_NMIN1_Baseline", "", numFolds)


-- ----- SP39 ----- --

numFolds = 3
--main("SP39", {"500T"}, dir .. "Test/x3_Win19_K2000_", "_NMIN1", numFolds)