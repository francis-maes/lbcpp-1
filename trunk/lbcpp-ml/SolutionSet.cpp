/*-----------------------------------------.---------------------------------.
| Filename: SolutionSet.cpp                | Set of solutions                |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/SolutionSet.h>
#include <lbcpp-ml/Comparator.h>
#include <MOO-EALib/Hypervolume.h>
#include <algorithm>
using namespace lbcpp;

/*
** MOOSolutionSet
*/
MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits, const std::vector<MOOSolutionPtr>& solutions, MOOSolutionComparatorPtr comparator)
  : limits(limits), solutions(solutions), comparator(comparator)
{
}

MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits)
  : limits(limits)
{
}

void MOOSolutionSet::addSolution(const MOOSolutionPtr& solution)
{
  solutions.push_back(solution);
  comparator = MOOSolutionComparatorPtr(); // mark as unsorted
}

void MOOSolutionSet::addSolution(const ObjectPtr& object, const MOOFitnessPtr& fitness)
  {addSolution(new MOOSolution(object, fitness));}

void MOOSolutionSet::addSolutions(const MOOSolutionSetPtr& otherSolutions)
{
  jassert(otherSolutions);
  size_t offset = solutions.size();
  size_t n = otherSolutions->getNumSolutions();
  solutions.resize(offset + n);
  for (size_t i = 0; i < n; ++i)
    solutions[i + offset] = otherSolutions->solutions[i];
  comparator = MOOSolutionComparatorPtr(); // mark as unsorted
}

std::vector<ObjectPtr> MOOSolutionSet::getObjects() const
{
  std::vector<ObjectPtr> res(solutions.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = solutions[i]->getObject();
  return res;
}

struct WrapSolutionComparator
{
  WrapSolutionComparator(const MOOSolutionComparatorPtr& comparator)
    : comparator(comparator) {}

  MOOSolutionComparatorPtr comparator;

  bool operator()(size_t a, size_t b) const
    {return comparator->compare(a, b) < 0;}
};

MOOSolutionSetPtr MOOSolutionSet::sort(const MOOSolutionComparatorPtr& comparator, std::vector<size_t>* mapping) const
{
  if (this->comparator == comparator)
    return refCountedPointerFromThis(this);
  size_t n = solutions.size();
  
  // compute ordered indices
  std::vector<size_t> localMapping;
  if (!mapping)
    mapping = &localMapping;
  mapping->resize(n);
  for (size_t i = 0; i < n; ++i)
    (*mapping)[i] = i;
  comparator->initialize(refCountedPointerFromThis(this));
  std::sort(mapping->begin(), mapping->end(), WrapSolutionComparator(comparator));

  // copy and re-order
  MOOSolutionSetPtr res = new MOOSolutionSet(limits);
  res->solutions.resize(n);
  for (size_t i = 0; i < n; ++i)
    res->solutions[i] = solutions[(*mapping)[i]];
  res->comparator = comparator; // mark as sorted
  return res;
}

int MOOSolutionSet::findBestSolution(const MOOSolutionComparatorPtr& comparator) const
{
  if (solutions.empty())
    return -1;
  if (this->comparator == comparator)
    return 0; // already sorted: the best is at first position

  comparator->initialize(refCountedPointerFromThis(this));
  size_t bestSolutionIndex = 0;
  for (size_t i = 1; i < solutions.size(); ++i)
    if (comparator->compare(i, bestSolutionIndex) == -1)
      bestSolutionIndex = i;

  return (int)bestSolutionIndex;
}

MOOSolutionPtr MOOSolutionSet::getBestSolution(const MOOSolutionComparatorPtr& comparator) const
{
  int index = findBestSolution(comparator);
  return index >= 0 ? solutions[index] : MOOSolutionPtr();
}

MOOSolutionSetPtr MOOSolutionSet::selectNBests(const MOOSolutionComparatorPtr& comparator, size_t n) const
{
  if (n >= solutions.size())
    return refCountedPointerFromThis(this);

  MOOSolutionSetPtr res = sort(comparator);
  res->solutions.resize(n);
  return res;
}

MOOFitnessLimitsPtr MOOSolutionSet::getEmpiricalLimits() const
{
  size_t n = limits->getNumDimensions();
  std::vector< std::pair<double, double> > res(n, std::make_pair(DBL_MAX, -DBL_MAX));

  for (size_t i = 0; i < solutions.size(); ++i)
  {
    MOOFitnessPtr fitness = solutions[i]->getFitness();
    for (size_t j = 0; j < n; ++j)
    {
      double value = fitness->getValue(j);
      if (value < res[j].first)
        res[j].first = value;
      if (value > res[j].second)
        res[j].second = value;
    }
  }
  return new MOOFitnessLimits(res);
}

bool MOOSolutionSet::strictlyDominates(const MOOFitnessPtr& fitness) const
{
  for (size_t i = 0; i < solutions.size(); ++i)
    if (solutions[i]->getFitness()->strictlyDominates(fitness))
      return true;
  return false;
}

MOOParetoFrontPtr MOOSolutionSet::getParetoFront() const
{
  std::vector<MOOSolutionPtr> res;
  for (size_t i = 0; i < solutions.size(); ++i)
    if (!strictlyDominates(solutions[i]->getFitness()))
      res.push_back(solutions[i]);
  return new MOOParetoFront(limits, res, comparator);
}

void MOOSolutionSet::computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const
{
  size_t n = solutions.size();
  if (!n)
    return;

  std::vector<size_t> dominationCounter(n, 0); // by how many others am I dominated?
  std::vector< std::vector<size_t> > dominationIndices(n); // who do I dominate?

  mapping.resize(n);

  std::vector<size_t> currentFrontIndices;
  MOOSolutionComparatorPtr dom = dominanceComparator();
  dom->initialize(refCountedPointerFromThis(this));
  for (size_t i = 0; i < n; ++i)
  {
    for (size_t j = 0; j < n; ++j)
    {
      int c = dom->compare(i, j);
      if (c == -1)
        dominationIndices[i].push_back(j);
      else if (c == 1)
        dominationCounter[i]++;
    }
    if (dominationCounter[i] == 0)
    {
      mapping[i] = std::make_pair(0, currentFrontIndices.size());
      currentFrontIndices.push_back(i);
    }
  }

  jassert(currentFrontIndices.size() > 0);
  countPerRank.push_back(currentFrontIndices.size());
  
  for (size_t currentRank = 1; true; ++currentRank)
  {
    std::vector<size_t> nextFrontIndices;
    for (size_t i = 0; i < currentFrontIndices.size(); ++i)
    {
      const std::vector<size_t>& indices = dominationIndices[currentFrontIndices[i]];
      for (size_t j = 0; j < indices.size(); ++j)
      {
        size_t index = indices[j];
        size_t& counter = dominationCounter[index];
        jassert(counter > 0);
        --counter;
        if (counter == 0)
        {
          mapping[index] = std::make_pair(currentRank, nextFrontIndices.size());
          nextFrontIndices.push_back(index);
        }
      }
    }
    currentFrontIndices.swap(nextFrontIndices);

    if (currentFrontIndices.size())
      countPerRank.push_back(currentFrontIndices.size());
    else
      break;
  }

#ifdef JUCE_DEBUG
  size_t totalSize = 0;
  for (size_t i = 0; i < countPerRank.size(); ++i)
    totalSize += countPerRank[i];
  jassert(totalSize == n);
#endif // JUCE_DEBUG
}

std::vector<MOOParetoFrontPtr> MOOSolutionSet::nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping) const
{
  std::vector< std::pair<size_t, size_t> > localMapping;
  if (!mapping)
    mapping = &localMapping;
  std::vector<size_t> countPerRank;
  computeParetoRanks(*mapping, countPerRank);

  std::vector<MOOParetoFrontPtr> res(countPerRank.size());
  for (size_t i = 0; i < res.size(); ++i)
  {
    MOOParetoFrontPtr front = new MOOParetoFront(limits);
    front->solutions.reserve(countPerRank[i]);
    front->comparator = comparator;
    res[i] = front;
  }

  for (size_t i = 0; i < mapping->size(); ++i)
    res[(*mapping)[i].first]->solutions.push_back(solutions[i]);
  return res;
}

void MOOSolutionSet::computeCrowdingDistances(std::vector<double>& res) const
{
  size_t n = solutions.size();

  res.resize(n, 0.0);
  if (n <= 2)
  {
    for (size_t i = 0; i < n; ++i)
      res[i] = DBL_MAX;
    return;
  }
  
  std::vector<size_t> order(n);
  for (size_t i = 0; i < n; ++i)
    order[i] = i;

  for (size_t i = 0; i < limits->getNumObjectives(); ++i)
  {
    std::vector<size_t> mapping;
    MOOSolutionSetPtr ordered = sort(objectiveComparator(i), &mapping);
    double bestValue = ordered->getFitness(0)->getValue(i);
    double worstValue = ordered->getFitness(n - 1)->getValue(i);
    double invRange = 1.0 / (worstValue - bestValue);
    res[mapping[0]] = DBL_MAX;
    res[mapping[n-1]] = DBL_MAX;
    for (size_t j = 1; j < n - 1; ++j)
      res[mapping[j]] += (ordered->getFitness(j+1)->getValue(i) - ordered->getFitness(j-1)->getValue(i)) * invRange;
  }
}

void MOOSolutionSet::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const MOOSolutionSetPtr& target = t.staticCast<MOOSolutionSet>();
  target->limits = limits;
  target->solutions = solutions;
  target->comparator = comparator;
}

/*
** MOOParetoFront
*/
void MOOParetoFront::addSolutionAndUpdateFront(const ObjectPtr& object, const MOOFitnessPtr& fitness)
{
  std::vector<MOOSolutionPtr> newSolutions;
  newSolutions.reserve(solutions.size());
  for (size_t i = 0; i < solutions.size(); ++i)
  {
    MOOFitnessPtr solutionFitness = solutions[i]->getFitness();
    if (solutionFitness->strictlyDominates(fitness))
      return; // dominated
    if (!fitness->strictlyDominates(solutionFitness))
      newSolutions.push_back(solutions[i]);
  }
  newSolutions.push_back(new MOOSolution(object, fitness));
  solutions.swap(newSolutions);
}

double MOOParetoFront::computeHyperVolume(const MOOFitnessPtr& referenceFitness) const
{
  if (isEmpty())
    return 0.0;

  size_t numObjectives = limits->getNumObjectives();
  if (numObjectives == 1)
  {
    double bestValue = getBestSolution(objectiveComparator(0))->getFitness()->getValue(0);
    double res = limits->getObjectiveSign(0) * (bestValue - referenceFitness->getValue(0));
    return res > 0.0 ? res : 0.0;
  }
  else
  {
    // remove points that fall outside the hypervolume and convert to minimization problem
    std::vector<double> points(solutions.size() * numObjectives);
    size_t numPoints = 0;
    for (size_t i = 0; i < solutions.size(); ++i)
    {
      MOOFitnessPtr fitness = solutions[i]->getFitness();
      if (!referenceFitness->isBetterForAtLeastOneObjectiveThan(fitness))
      {
        std::vector<double> val = fitness->getValuesToBeMinimized();
        memcpy(&points[numPoints * numObjectives], &val[0], sizeof (double) * numObjectives);
        ++numPoints;
      }
    }

    if (!numPoints)
      return 0.0;

    std::vector<double> ref = referenceFitness->getValuesToBeMinimized();

    // shark implementation
    return hypervolume(&points[0], &ref[0], (unsigned int)numObjectives, (unsigned int)numPoints);
  }
}
