/*-----------------------------------------.---------------------------------.
| Filename: BooleanMultiplexerProblem.h    | Boolean Multiplexer Problem     |
| Author  : Francis Maes                   |                                 |
| Started : 11/10/2012 11:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_
# define LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_

# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class BooleanMultiplexerProblem : public Problem
{
public:
  BooleanMultiplexerProblem(size_t numAddressBits) : numAddressBits(numAddressBits)
    {initialize(defaultExecutionContext());}
  BooleanMultiplexerProblem() {}

  virtual void initialize(ExecutionContext& context)
  {
    size_t numDataBits = (1 << numAddressBits);
    size_t numBits = numAddressBits + numDataBits;

    ExpressionDomainPtr domain = new ExpressionDomain();
    for (size_t i = 0; i < numAddressBits; ++i)
		  domain->addInput(newBooleanClass, "a" + String((int)i));
    for (size_t i = 0; i < numDataBits; ++i)
      domain->addInput(newBooleanClass, "d" + String((int)i));

		domain->addFunction(andBooleanFunction());
    domain->addFunction(orBooleanFunction());
    domain->addFunction(notBooleanFunction());
    domain->addFunction(ifThenElseBooleanFunction());

    domain->addTargetType(booleanType);
    setDomain(domain);

    // data
    size_t numCases = (1 << numBits);

    TablePtr data = new Table(numCases);
    for (size_t i = 0; i < domain->getNumInputs(); ++i)
      data->addColumn(domain->getInput(i), newBooleanClass);
    VariableExpressionPtr supervision = domain->createSupervision(newBooleanClass, "y");
    data->addColumn(supervision, newBooleanClass);

		for (size_t i = 0; i < numCases; ++i)
		{
      size_t address = 0;
      for (size_t j = 0; j < numBits; ++j)
      {
        bool isBitActive = (i & (1 << j)) != 0;
        data->setElement(i, j, new NewBoolean(isBitActive));
        if (j < numAddressBits)
        {
          address <<= 1;
          if (isBitActive)
            address |= 1;
        }
      }
      data->setElement(i, numBits, data->getElement(i, numAddressBits + address));
		}

    // objective
    addObjective(binaryAccuracyObjective(data, supervision));
  }

protected:
  friend class BooleanMultiplexerProblemClass;

  size_t numAddressBits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_BOOLEAN_MULTIPLEXER_PROBLEM_H_
