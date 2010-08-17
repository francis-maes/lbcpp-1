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

# include "Traits.h"
# include "ContainerTraits.h"

// new
# include "Data/Object.h"
# include "Data/Type.h"
# include "Data/Variable.h"
# include "Data/Function.h"
# include "Data/Stream.h"
# include "Data/Consumer.h"
# include "Data/Vector.h"
# include "Data/SymmetricMatrix.h"
# include "Data/Predicate.h"
# include "Data/ProbabilityDistribution.h"
# include "Data/Perception.h"

// old
# include "Object/ObjectContainer.h"
# include "Object/ObjectStream.h"
# include "Object/ObjectConsumer.h"
# include "Object/ObjectGraph.h"
# include "Object/Table.h"
# include "Object/StringToObjectMap.h"
// -

# include "Utilities/IterationFunction.h"
# include "Utilities/RandomGenerator.h"
# include "Utilities/RandomVariable.h"
# include "Utilities/ProgressCallback.h"
# include "Utilities/StoppingCriterion.h"

# include "FeatureGenerator/FeatureGenerator.h"
# include "FeatureGenerator/FeatureVisitor.h"
# include "FeatureGenerator/EditableFeatureGenerator.h"
# include "FeatureGenerator/SparseVector.h"
# include "FeatureGenerator/DenseVector.h"
# include "FeatureGenerator/ContinuousFunction.h"
# include "FeatureGenerator/Optimizer.h"
# include "FeatureGenerator/impl/DoubleVector.hpp"
# include "FeatureGenerator/impl/FeatureGenerator.hpp"
# include "FeatureGenerator/impl/FeatureGeneratorDefaultImplementations.hpp"

# include "Inference/Inference.h"
# include "Inference/InferenceStack.h"
# include "Inference/InferenceContext.h"
# include "Inference/InferenceCallback.h"
# include "Inference/DecoratorInference.h"
# include "Inference/SequentialInference.h"
# include "Inference/ParallelInference.h"
# include "Inference/InferenceOnlineLearner.h"
# include "Inference/InferenceResultCache.h"
# include "Inference/Evaluator.h"

#endif // !LBCPP_LBCPP_H_
