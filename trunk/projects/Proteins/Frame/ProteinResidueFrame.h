/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueFrame.h          | Protein Residue Frame           |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2011 14:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_RESIDUE_FRAME_H_
# define LBCPP_PROTEIN_RESIDUE_FRAME_H_

# include "ProteinFrame.h"

namespace lbcpp
{

extern FrameClassPtr defaultProteinSingleResidueFrameClass(ExecutionContext& context);
extern VectorPtr createProteinSingleResidueFrames(ExecutionContext& context, const FramePtr& proteinFrame, FrameClassPtr residueFrameClass);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_RESIDUE_FRAME_H_

