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
| Filename: predeclarations.h              | Data Predeclarations            |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 00:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_PREDECLARATIONS_H_
# define LBCPP_CORE_PREDECLARATIONS_H_

# include "ReferenceCountedObject.h"

namespace lbcpp
{

struct ApplicationContext;

class Variable;
class XmlExporter;
class XmlImporter;

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;

class VariableSignature;
typedef ReferenceCountedObjectPtr<VariableSignature> VariableSignaturePtr;

class Type;
typedef ReferenceCountedObjectPtr<Type> TypePtr;

class Class;
typedef ReferenceCountedObjectPtr<Class> ClassPtr;

class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class DefaultEnumeration;
typedef ReferenceCountedObjectPtr<DefaultEnumeration> DefaultEnumerationPtr;

class TemplateType;
typedef ReferenceCountedObjectPtr<TemplateType> TemplateTypePtr;

class Library;
typedef ReferenceCountedObjectPtr<Library> LibraryPtr;

class Pair;
typedef ReferenceCountedObjectPtr<Pair> PairPtr;

class Container;
typedef ReferenceCountedObjectPtr<Container> ContainerPtr;

class Vector;
typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

class ExecutionContext;
typedef ReferenceCountedObjectPtr<ExecutionContext> ExecutionContextPtr;

extern ExecutionContext& defaultExecutionContext();

class DynamicClass;
typedef ReferenceCountedObjectPtr<DynamicClass> DynamicClassPtr;

class SparseDoubleObject;
typedef ReferenceCountedObjectPtr<SparseDoubleObject> SparseDoubleObjectPtr;

class DenseObjectObject;
typedef ReferenceCountedObjectPtr<DenseObjectObject> DenseObjectObjectPtr;

class DenseDoubleObject;
typedef ReferenceCountedObjectPtr<DenseDoubleObject> DenseDoubleObjectPtr;

class DenseGenericObject;
typedef ReferenceCountedObjectPtr<DenseGenericObject> DenseGenericObjectPtr;

class Function;
typedef ReferenceCountedObjectPtr<Function> FunctionPtr;

class CompositeFunction;
typedef ReferenceCountedObjectPtr<CompositeFunction> CompositeFunctionPtr;

class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_PREDECLARATIONS_H_