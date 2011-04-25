/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateFeatureGenerator.h  | Concatenate Feature Generator   |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class ConcatenateDoubleFeatureGenerator : public FeatureGenerator
{
public:
  ConcatenateDoubleFeatureGenerator(bool lazy = true)
    : FeatureGenerator(lazy) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual String getOutputPostFix() const
    {return T("Concatenated");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    // create enum
    DefaultEnumerationPtr elementsEnumeration = new DefaultEnumeration(T("concatenatedFeatures(") + String((int)inputVariables.size()) + T(" doubles)"));
    elementsType = TypePtr();
    size_t numInputs = inputVariables.size();
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = inputVariables[i];
      elementsEnumeration->addElement(context, inputVariable->getName(), String::empty, inputVariable->getShortName());
      if (i == 0)
        elementsType = inputVariable->getType();
      else
        elementsType = Type::findCommonBaseType(elementsType, inputVariable->getType());
    }
    if (!elementsType->inheritsFrom(doubleType))
    {
      context.errorCallback(T("All elements do not inherit from double"));
      return EnumerationPtr();
    }
    return elementsEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    size_t numInputs = getNumInputs();
    for (size_t i = 0; i < numInputs; ++i)
    {
      double value = inputs[i].getDouble();
      if (value)
        callback.sense(i, value);
    }
  }

private:
  TypePtr elementsType;
};

class ConcatenateDoubleVectorFeatureGenerator : public FeatureGenerator
{
public:
  ConcatenateDoubleVectorFeatureGenerator(bool lazy = true)
    : FeatureGenerator(lazy) {}

  virtual ClassPtr getLazyOutputType(EnumerationPtr featuresEnumeration, TypePtr featuresType) const
    {return compositeDoubleVectorClass(featuresEnumeration, featuresType);}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass();}
  
  virtual String getOutputPostFix() const
    {return T("Concatenated");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    // make enum name
    ConcatenateEnumerationPtr elementsEnumeration = new ConcatenateEnumeration(T("concatenatedFeatures(") + String((int)inputVariables.size()) + T(" double vectors)"));
    elementsType = TypePtr();
    size_t numInputs = inputVariables.size();
    shifts.resize(numInputs);
    elementsEnumeration->reserveSubEnumerations(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = inputVariables[i];

      shifts[i] = elementsEnumeration->getNumElements();
      if (i && !shifts[i])
        jassertfalse;

      EnumerationPtr subElementsEnumeration;
      TypePtr subElementsType;
      if (!DoubleVector::getTemplateParameters(context, inputVariable->getType(), subElementsEnumeration, subElementsType))
        return EnumerationPtr();

      elementsEnumeration->addSubEnumeration(inputVariable->getName(), subElementsEnumeration);
      if (i == 0)
        elementsType = subElementsType;
      else
        elementsType = Type::findCommonBaseType(elementsType, subElementsType);
    }
    if (!elementsType->inheritsFrom(doubleType))
    {
      context.errorCallback(T("All elements do not inherit from double"));
      return EnumerationPtr();
    }
    return elementsEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    for (size_t i = 0; i < shifts.size(); ++i)
    {
      const DoubleVectorPtr& input = inputs[i].getObjectAndCast<DoubleVector>();
      if (input)
        callback.sense(shifts[i], input, 1.0);
    }
  }

  virtual DoubleVectorPtr toLazyVector(const Variable* inputs) const
  {
    CompositeDoubleVectorPtr res = new CompositeDoubleVector(getOutputType());
    res->reserve(shifts.size());
    for (size_t i = 0; i < shifts.size(); ++i)
    {
      const DoubleVectorPtr& input = inputs[i].getObjectAndCast<DoubleVector>();
      if (input)
        res->appendSubVector(shifts[i], input);
    }
    return res;
  }

  virtual DoubleVectorPtr toComputedVector(const Variable* inputs) const
  {
    SparseDoubleVectorPtr res = createEmptySparseVector();
    for (size_t i = 0; i < shifts.size(); ++i)
    {
      const DoubleVectorPtr& input = inputs[i].getObjectAndCast<DoubleVector>();
      if (input)
        input->appendTo(res, shifts[i], 1.0);
    }
    const_cast<ConcatenateDoubleVectorFeatureGenerator* >(this)->pushSparseVectorSize(res->getNumValues());
    return res;
  }

private:
  std::vector<size_t> shifts;
  TypePtr elementsType;
};

class ConcatenateMixedDoubleDoubleVectorFeatureGenerator : public ConcatenateDoubleVectorFeatureGenerator
{
public:
  ConcatenateMixedDoubleDoubleVectorFeatureGenerator(bool lazy = true)
    : ConcatenateDoubleVectorFeatureGenerator(lazy) {}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return sumType(doubleType, doubleVectorClass());}
 
  virtual ClassPtr getLazyOutputType(EnumerationPtr featuresEnumeration, TypePtr featuresType) const
    {return FeatureGenerator::getLazyOutputType(featuresEnumeration, featuresType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    // make enum name
    ConcatenateEnumerationPtr elementsEnumeration = new ConcatenateEnumeration(T("concatenatedFeatures(") + String((int)inputVariables.size()) + T(" double vectors)"));
    elementsType = TypePtr();
    size_t numInputs = inputVariables.size();
    shifts.resize(numInputs);
    elementsEnumeration->reserveSubEnumerations(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = inputVariables[i];

      shifts[i] = elementsEnumeration->getNumElements();
      if (i && !shifts[i])
        jassertfalse;

      TypePtr subElementsType;

      if (inputVariable->getType()->inheritsFrom(doubleVectorClass()))
      {
        EnumerationPtr subElementsEnumeration;
        if (!DoubleVector::getTemplateParameters(context, inputVariable->getType(), subElementsEnumeration, subElementsType))
          return EnumerationPtr();
        elementsEnumeration->addSubEnumeration(inputVariable->getName(), subElementsEnumeration);
      }
      else if (inputVariable->getType()->inheritsFrom(doubleType))
      {
        subElementsType = inputVariable->getType();
        DefaultEnumerationPtr subElementsEnumeration = new DefaultEnumeration(inputVariable->getName());
        subElementsEnumeration->addElement(context, T("value"));
        elementsEnumeration->addSubEnumeration(inputVariable->getName(), subElementsEnumeration);
      }
      else
      {
        jassert(false);
        continue;
      }

      if (i == 0)
        elementsType = subElementsType;
      else
        elementsType = Type::findCommonBaseType(elementsType, subElementsType);
    }
    if (!elementsType || !elementsType->inheritsFrom(doubleType))
    {
      context.errorCallback(T("All elements do not inherit from double"));
      return EnumerationPtr();
    }
    return elementsEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    for (size_t i = 0; i < shifts.size(); ++i)
      if (inputs[i].exists())
      {
        if (inputs[i].isDouble())
          callback.sense(shifts[i], inputs[i].getDouble());
        else
          callback.sense(shifts[i], inputs[i].getObjectAndCast<DoubleVector>(), 1.0);
      }
  }

  virtual DoubleVectorPtr toComputedVector(const Variable* inputs) const
  {
    SparseDoubleVectorPtr res = createEmptySparseVector();
    for (size_t i = 0; i < shifts.size(); ++i)
      if (inputs[i].exists())
      {
        if (inputs[i].isDouble())
          res->appendValue(shifts[i], inputs[i].getDouble());
        else
          inputs[i].getObjectAndCast<DoubleVector>()->appendTo(res, shifts[i], 1.0);
      }

    const_cast<ConcatenateMixedDoubleDoubleVectorFeatureGenerator* >(this)->pushSparseVectorSize(res->getNumValues());
    return res;
  }

private:
  std::vector<size_t> shifts;
  TypePtr elementsType;
};

class ConcatenateFeatureGenerator : public ProxyFunction
{
public:
  ConcatenateFeatureGenerator(bool lazy = true)
    : lazy(lazy) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return sumType(doubleType, doubleVectorClass());}
  
  virtual String getOutputPostFix() const
    {return T("Concatenated");}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    bool isAllDouble = true;
    bool isAllDoubleVector = true;
    bool isAllDoubleOrDoubleVector = true;
    for (size_t i = 0; i < inputVariables.size(); ++i)
    {
      bool isDouble = inputVariables[i]->getType()->inheritsFrom(doubleType);
      bool isDoubleVector = inputVariables[i]->getType()->inheritsFrom(doubleVectorClass());
      isAllDouble &= isDouble;
      isAllDoubleVector &= isDoubleVector;
      isAllDoubleOrDoubleVector &= (isDouble | isDoubleVector);
    }
    if (isAllDouble)
      return new ConcatenateDoubleFeatureGenerator(lazy);
    if (isAllDoubleVector)
      return new ConcatenateDoubleVectorFeatureGenerator(lazy);
    if (isAllDoubleOrDoubleVector)
      return new ConcatenateMixedDoubleDoubleVectorFeatureGenerator(lazy);
    return FunctionPtr();
  }

protected:
  friend class ConcatenateFeatureGeneratorClass;

  bool lazy;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_