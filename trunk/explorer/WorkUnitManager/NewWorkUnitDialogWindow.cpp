/*-----------------------------------------.---------------------------------.
| Filename: NewWorkUnitDialogWindow.cp     | The dialog window to start a    |
| Author  : Francis Maes                   |  new work unit                  |
| Started : 25/01/2011 18:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NewWorkUnitDialogWindow.h"
using namespace lbcpp;

namespace lbcpp {

class WorkUnitSelectorComboxBox : public juce::ComboBox, public juce::ComboBoxListener
{
public:
  WorkUnitSelectorComboxBox(RecentWorkUnitsConfigurationPtr recent, String& workUnitName)
    : juce::ComboBox(T("Toto")), recent(recent), workUnitName(workUnitName)
  {
    setEditableText(true);
    setSize(600, 22);
    setName(T("WorkUnit"));
    addListener(this);

    int id = 1;

    size_t numRecents = recent->getNumRecentWorkUnits();
    if (numRecents)
    {
      addSectionHeading(T("Recent"));
      if (numRecents > 8) numRecents = 8;
      for (size_t i = 0; i < numRecents; ++i)
      {
        String str = recent->getRecentWorkUnit(i)->getWorkUnitName();
        addItem(str, id++);
      }
    }

    size_t numLibraries = lbcpp::getNumLibraries();
    for (size_t i = 0; i < numLibraries; ++i)
    {
      LibraryPtr library = lbcpp::getLibrary(i);
      if (library->getName() == T("LBCpp"))
        continue;
      std::vector<TypePtr> workUnits = library->getTypesInheritingFrom(workUnitClass);
      if (workUnits.size())
      {
        addSectionHeading(library->getName());
        for (size_t j = 0; j < workUnits.size(); ++j)
          if (workUnits[j] != workUnitClass)
            addItem(workUnits[j]->getName(), id++);
      }
    }
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == this)
    {
      workUnitName = getText();
      recent->addRecentWorkUnit(workUnitName);
    }
  }

  RecentWorkUnitsConfigurationPtr recent;
  String& workUnitName;

  juce_UseDebuggingNewOperator
};

using juce::ComboBox;
using juce::Label;
using juce::Font;

class WorkUnitArgumentComponent : public Component
{
public:
  WorkUnitArgumentComponent(ClassPtr workUnitClass, size_t variableIndex)
    : name(NULL), shortName(NULL), description(NULL)
  {
    String nameValue = workUnitClass->getObjectVariableName(variableIndex);
    String shortNameValue = workUnitClass->getObjectVariableShortName(variableIndex);
    String descriptionValue = workUnitClass->getObjectVariableDescription(variableIndex);

    int desiredHeight = namesHeight;
    addAndMakeVisible(name = new Label(T("name"), nameValue));
    name->setFont(Font(16, Font::bold));
    if (shortNameValue != nameValue)
    {
      addAndMakeVisible(shortName = new Label(T("shortName"), T("-") + shortNameValue));
      shortName->setJustificationType(Justification::centredRight);
      shortName->setFont(Font(14, Font::bold));
    }
    
    if (descriptionValue.isNotEmpty())
    {
      addAndMakeVisible(description = new Label(T("desc"), descriptionValue));
      description->setFont(Font(12, Font::italic));
      desiredHeight += descriptionHeightPerLine; // todo: * numLines
    }

    setSize(600, desiredHeight);
  }

  virtual ~WorkUnitArgumentComponent()
    {deleteAllChildren();}

  enum
  {
    namesHeight = 18,
    descriptionHeightPerLine = 15,
  };

  virtual void resized()
  {
    name->setBounds(0, 0, getWidth(), namesHeight);
    if (shortName)
      shortName->setBounds(0, 0, getWidth(), namesHeight);
    if (description)
      description->setBounds(0, namesHeight, getWidth(), descriptionHeightPerLine);
  }

private:
  Label* name;
  Label* shortName;
  Label* description;
};

class WorkUnitArgumentsComponent : public Component, public juce::ComboBoxListener
{
public:
  WorkUnitArgumentsComponent(RecentWorkUnitConfigurationPtr workUnit, String& resultString)
    : workUnit(workUnit), resultString(resultString)
  {
    addAndMakeVisible(recentArguments = new ComboBox(T("recentArgs")));
    recentArguments->setEditableText(true);
    std::vector<String> recents = workUnit->getArguments();
    for (size_t i = 0; i < recents.size(); ++i)
      if (recents[i].trim().isNotEmpty())
        recentArguments->addItem(recents[i], (int)i + 1);
    if (recentArguments->getNumItems())
      recentArguments->setSelectedItemIndex(0);
    recentArguments->addListener(this);

    int desiredHeight = 22;
    ClassPtr workUnitClass = getType(workUnit->getWorkUnitName());
    jassert(workUnitClass);
    arguments.resize(workUnitClass->getObjectNumVariables());
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      addAndMakeVisible(arguments[i] = new WorkUnitArgumentComponent(workUnitClass, i));
      desiredHeight += arguments[i]->getHeight() + 2;
    }
    setSize(600, desiredHeight);
  }

  virtual ~WorkUnitArgumentsComponent()
    {deleteAllChildren();}

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == recentArguments)
      resultString = recentArguments->getText();
  }

  virtual void resized()
  {
    recentArguments->setBounds(0, 0, getWidth(), 20);
    int y = 22;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      int h = arguments[i]->getHeight();
      arguments[i]->setBounds(0, y, getWidth(), h);
      y += h + 2;
    }
  }

protected:
  RecentWorkUnitConfigurationPtr workUnit;
  ComboBox* recentArguments;
  std::vector<WorkUnitArgumentComponent* > arguments;
  String& resultString;
};

class WorkUnitArgumentsViewport : public Viewport, public juce::ComboBoxListener
{
public:
  WorkUnitArgumentsViewport(RecentWorkUnitsConfigurationPtr recent, String& resultString)
    : content(NULL), recent(recent), resultString(resultString)
  {
    setSize(600, 400);
    setScrollBarsShown(true, false);
  }
  
  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
    {
      RecentWorkUnitConfigurationPtr workUnit = recent->getWorkUnit(comboBoxThatHasChanged->getText());
      content = new WorkUnitArgumentsComponent(workUnit, resultString);
      setViewedComponent(content);
      content->setSize(getMaximumVisibleWidth(), content->getHeight());
    }
  }

  juce_UseDebuggingNewOperator

private:
  WorkUnitArgumentsComponent* content;
  RecentWorkUnitsConfigurationPtr recent;
  String& resultString;
};
/*
class WorkUnitArgumentsComponent : public juce::ComboBox, public juce::ComboBoxListener
{
public:
  WorkUnitArgumentsComponent(RecentWorkUnitsConfigurationPtr recent, String& arguments)
    : juce::ComboBox(T("Toto")), recent(recent), arguments(arguments)
  {
    setEditableText(true);
    setSize(600, 22);
    addListener(this);
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == this)
      arguments = getText();
    if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
    {
      clear();
      RecentWorkUnitConfigurationPtr workUnit = recent->getWorkUnit(comboBoxThatHasChanged->getText());
      if (workUnit)
      {
        std::vector<String> arguments = workUnit->getArguments();
        for (size_t i = 0; i < arguments.size(); ++i)
          addItem(arguments[i].isEmpty() ? T(" ") : arguments[i], (int)i + 1);
        if (arguments.size())
          setSelectedItemIndex(0);
      }
    }
  }

  RecentWorkUnitsConfigurationPtr recent;
  String& arguments;

  juce_UseDebuggingNewOperator
};*/

}; /* namespace lbcpp */

/*
** NewWorkUnitDialogWindow
*/
NewWorkUnitDialogWindow::NewWorkUnitDialogWindow(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory)
  : AlertWindow(T("New Work Unit"), T("Select a work unit and its arguments"), QuestionIcon),
    workUnitSelector(new WorkUnitSelectorComboxBox(recent, workUnitName)),
    argumentsSelector(new WorkUnitArgumentsViewport(recent, arguments)),
    workingDirectorySelector(recent, workingDirectory)
{
  addTextBlock(T("Work Unit:"));
  addCustomComponent(workUnitSelector);
  addTextBlock(T("Arguments:"));
  addCustomComponent(argumentsSelector);
  //addTextBlock(T("Working directory:"));
  //addCustomComponent(&workingDirectorySelector);
  addButton(T("OK"), 1, juce::KeyPress::returnKey);
  addButton(T("Cancel"), 0, juce::KeyPress::escapeKey);

  workUnitSelector->addListener(argumentsSelector);
  workUnitSelector->addListener(&workingDirectorySelector);
  if (workUnitSelector->getNumItems())
    workUnitSelector->setSelectedItemIndex(0);
}

NewWorkUnitDialogWindow::~NewWorkUnitDialogWindow()
{
  deleteAndZero(workUnitSelector);
  deleteAndZero(argumentsSelector);
}

bool NewWorkUnitDialogWindow::run(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory)
{
  NewWorkUnitDialogWindow* window = new NewWorkUnitDialogWindow(recent, workUnitName, arguments, workingDirectory);
  const int result = window->runModalLoop();
  delete window;
  return result == 1;
}

void NewWorkUnitDialogWindow::resized()
{
  //argumentsSelector->setSize(argumentsSelector->getWidth(), getHeight() - 100);
  AlertWindow::resized();
}