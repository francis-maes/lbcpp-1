/*-----------------------------------------.---------------------------------.
| Filename: ProteinComponent.h             | Components for proteins         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_
# define LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_

# include "MultiProtein1DComponent.h"
# include "MultiProtein2DComponent.h"
# include "ProteinPerceptionComponent.h"
# include <lbcpp/UserInterface/VariableSelector.h>
# include "../Frame/ProteinFrame.h"

namespace lbcpp
{

class MultiProteinComponent : public TabbedVariableSelectorComponent
{
public:
  MultiProteinComponent(const std::vector<ProteinPtr>& proteins, const std::vector<String>& names)
    : TabbedVariableSelectorComponent(proteins.size() == 1 ? proteins[0] : Variable()), proteins(proteins), names(names)
    {initializeTabs();}
  
  MultiProteinComponent(ContainerPtr container, const String& name)
    : TabbedVariableSelectorComponent(container)
  {
    jassert(container->getElementsType()->inheritsFrom(proteinClass));
    size_t n = container->getNumElements();
    proteins.reserve(n);
    names.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr object = container->getElement(i).getObject();
      ProteinPtr protein = object.dynamicCast<Protein>();
      jassert(protein);
      proteins.push_back(protein);
      names.push_back(String((int)i) + T(" - ") + protein->getName());
    }
    initializeTabs();
  }
      
  void initializeTabs()
  {
    if (proteins.size() == 1)
    {
      addTab(T("Data"), Colours::white);
      addTab(T("Perception"), Colours::white);
    }
    addTab(T("Protein 1D"), Colours::white);
    addTab(T("Protein 2D"), Colours::white);
    addTab(T("Protein Frame"), Colours::white);
  }

  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& tabName)
  {
    ClassPtr proteinClass = lbcpp::proteinClass;

    if (tabName == T("Data"))
      return userInterfaceManager().createVariableTreeView(context, proteins[0], names[0]);
    else if (tabName == T("Perception"))
      return new ProteinPerceptionComponent(proteins[0]);
    else if (tabName == T("Protein Frame"))
    {
      ProteinFrameFactory factory;
      FrameClassPtr frameClass = factory.createProteinFrameClass(context);
      FramePtr proteinFrame = factory.createFrame(proteins[0]);
      return userInterfaceManager().createVariableTreeView(context, proteinFrame);
    }
    else if (tabName == T("Protein 1D"))
    {
      std::vector< std::pair<String, size_t> > sequenceIndex;
      size_t n = proteinClass->getNumMemberVariables();
      for (size_t i = 0; i < n; ++i)
      {
        TypePtr type = proteinClass->getMemberVariableType(i);
        if (type->inheritsFrom(genericVectorClass(anyType))
         || type->inheritsFrom(objectVectorClass(enumerationDistributionClass(anyType))))
        {
          String friendlyName = proteinClass->getMemberVariableDescription(i);
          addObjectNameIfExists(friendlyName, i, sequenceIndex);
        }
      }

      MultiProtein1DConfigurationPtr configuration = new MultiProtein1DConfiguration(names, sequenceIndex);
      return new MultiProtein1DComponent(proteins, configuration);
    }
    else if (tabName == T("Protein 2D"))
    {
      std::vector< std::pair<String, size_t> > mapIndex;
      addObjectNameIfExists(T("Ca 8 angstrom"), proteinClass->findMemberVariable(T("contactMap8Ca")), mapIndex);
      addObjectNameIfExists(T("Cb 8 angstrom"), proteinClass->findMemberVariable(T("contactMap8Cb")), mapIndex);
      
      MultiProtein2DConfigurationPtr configuration = new MultiProtein2DConfiguration(names, mapIndex);
      return new MultiProtein2DComponent(proteins, configuration);
    }

    jassert(false);
    return NULL;
  }

  juce_UseDebuggingNewOperator

protected:
  std::vector<ProteinPtr> proteins;
  std::vector<String> names;

  void addObjectNameIfExists(const String& friendlyName, size_t variableIndex, std::vector< std::pair<String, size_t> >& res)
  {
    for (size_t i = 0; i < proteins.size(); ++i)
      if (proteins[i]->getTargetOrComputeIfMissing(variableIndex).exists())
      {
        res.push_back(std::make_pair(friendlyName, variableIndex));
        return;
      }
  }  
};

class ProteinComponent : public MultiProteinComponent
{
public:
  ProteinComponent(ProteinPtr protein, const String& name = String::empty)
    : MultiProteinComponent(std::vector<ProteinPtr>(1, protein), std::vector<String>(1, name.isEmpty() ? protein->getName() : name)) {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_USER_INTERFACE_PROTEIN_H_
