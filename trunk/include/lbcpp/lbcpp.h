/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: lbcpp.h                        | global include file for lbcpp   |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2009 20:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LBCPP_H_
# define LBCPP_LBCPP_H_

# include "Data/Utilities.h"
# include "Data/Object.h"
# include "Data/Type.h"
# include "Data/Variable.h"
# include "Data/Pair.h"
# include "Data/XmlSerialisation.h"
# include "Data/DynamicObject.h"
# include "Data/Stream.h"
# include "Data/Consumer.h"
# include "Data/Vector.h"
# include "Data/SymmetricMatrix.h"
# include "Data/ProbabilityDistribution.h"
# include "Data/RandomGenerator.h"
# include "Data/RandomVariable.h"
# include "Data/Cache.h"

# include "Function/Function.h"
# include "Function/ScalarFunction.h"
# include "Function/ScalarObjectFunction.h"
# include "Function/Predicate.h"
# include "Function/IterationFunction.h"
# include "Function/StoppingCriterion.h"
# include "Function/Evaluator.h"

# include "Perception/Perception.h"
# include "Perception/PerceptionRewriter.h"

# include "Inference/Inference.h"
# include "Inference/InferenceStack.h"
# include "Inference/InferenceResultCache.h"
# include "Inference/InferenceContext.h"
# include "Inference/InferenceCallback.h"
# include "Inference/DecoratorInference.h"
# include "Inference/SequentialInference.h"
# include "Inference/ParallelInference.h"
# include "Inference/InferenceBatchLearner.h"
# include "Inference/InferenceOnlineLearner.h"

# include "NumericalLearning/NumericalLearning.h"

# include "DecisionTree/DecisionTree.h"

#endif // !LBCPP_LBCPP_H_
