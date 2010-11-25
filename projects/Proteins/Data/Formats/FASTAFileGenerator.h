/*-----------------------------------------.---------------------------------.
| Filename: FASTAFileGenerator.h           | FASTA File generator            |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_
# define LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_

# include "../Protein.h"
# include <lbcpp/Data/Consumer.h>

namespace lbcpp
{

class FASTAFileGenerator : public TextPrinter
{
public:
  FASTAFileGenerator(ExecutionContext& context, const File& file)
    : TextPrinter(context, file) {}

  virtual void consume(ExecutionContext& context, const Variable& variable)
  {
    const ProteinPtr& protein = variable.getObjectAndCast<Protein>(context);
    jassert(protein);
    print(T(">") + protein->getName(), true);
    VectorPtr primaryStructure = protein->getPrimaryStructure();
    jassert(primaryStructure);
    String aa = primaryStructure->toString();
    jassert((size_t) aa.length() == primaryStructure->getNumElements());
    print(aa, true);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_
