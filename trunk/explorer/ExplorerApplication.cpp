/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

#include "Components/ObjectGraphAndContentComponent.h"
#include "Components/StringToObjectMapTabbedComponent.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

Component* lbcpp::createComponentForObject(ObjectPtr object, bool topLevelComponent)
{
  if (object.dynamicCast<StringToObjectMap>())
    return new StringToObjectMapTabbedComponent(object.dynamicCast<StringToObjectMap>());

  ObjectGraphPtr graph = object->toGraph();
  if (topLevelComponent && graph)
    return new ObjectGraphAndContentComponent(graph);
  TablePtr table = object->toTable();
  if (table)
    return new TableComponent(table);
  return new StringComponent(object);
}

class ExplorerErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const String& where, const String& what)
    {AlertWindow::showMessageBox(AlertWindow::WarningIcon, JUCE_T("Error in '") + where + JUCE_T("'"), what);}
  
  virtual void warningMessage(const String& where, const String& what)
    {AlertWindow::showMessageBox(AlertWindow::WarningIcon, JUCE_T("Warning in '") + where + JUCE_T("'"), what);}

  juce_UseDebuggingNewOperator
};

static ExplorerErrorHandler explorerErrorHandler;

class ExplorerMainWindow : public DocumentWindow, public MenuBarModel
{
public:
  ExplorerMainWindow() : DocumentWindow("LBC++ Explorer", Colours::whitesmoke, allButtons)
  {
    setMenuBar(this);
    setResizable(true, true);
    centreWithSize(700, 600);
#ifdef JUCE_MAC
//    setUsingNativeTitleBar(true);
#endif // JUCE_MAC

/*    FeatureDictionaryPtr dictionary = loadFeatureDictionary("/Users/francis/Projets/LBC++/trunk/examples/LearningMachine/features.dic");
    jassert(dictionary);
    dictionary->addScope("coucou", new FeatureDictionary());
    dictionary->addScope("pouet", new FeatureDictionary());
   // setContentComponent(new ObjectGraphAndContent(dictionary->toGraph()));      
    setContentComponent(new ObjectGraphComponent(dictionary->toGraph()));*/
    
//    FeatureDictionaryPtr dictionary = createRandomDictionary(5);
    
  /*  GradientBasedClassifierPtr classifier = loadGradientBasedClassifier("/Users/francis/Projets/LBC++/trunk/examples/LearningMachine/classifier.model");
    DenseVectorPtr params = classifier->getParameters();
    jassert(params);
    new ObjectGraphAndContentComponent(params->toGraph())*/
    
    setContentComponent(content = new ObjectComponentContainer());
    //setContentComponent(new TableComponent(params->getDictionary()->toTable()));
  }
  
  virtual ~ExplorerMainWindow()
  {
    setMenuBar(NULL);
  }
    
  virtual void closeButtonPressed()
  {
    JUCEApplication::quit();
  }
  
  virtual const StringArray getMenuBarNames()
  {
    const tchar* const names[] = {JUCE_T("File"), 0 };
    return StringArray((const tchar**) names);
  }

  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName)
  {
    PopupMenu menu;
    enum {iconSizes = 20};

    if (topLevelMenuIndex == 0)
    {
      menu.addItem(1, "Open");
      menu.addItem(2, "Close", content->getObject() != ObjectPtr());
      menu.addSeparator();
      menu.addItem(3, "Quit");
    }
    return menu;
  }

  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex)
  {
    if (menuItemID == 1)
    {
      FileChooser chooser("Please select the file you want to load...",
                               File::getSpecialLocation (File::userHomeDirectory),
                               "*.*");

      if (chooser.browseForFileToOpen())
      {
        ObjectPtr object = loadObject(chooser.getResult());
        if (object)
        {
          content->setObject(object, true);
          setName(JUCE_T("LBC++ Explorer - ") + object->getClassName() + JUCE_T(" ") + object->getName());
        }
      }
    }
    else if (menuItemID == 2)
    {
      content->setObject(ObjectPtr());
      setName(JUCE_T("LBC++ Explorer"));
    }
    else if (menuItemID == 3)
    {
      JUCEApplication::quit();
    }
  }
  
  juce_UseDebuggingNewOperator

private:
  ObjectComponentContainer* content;
  
  FeatureDictionaryPtr createRandomDictionary(size_t maxChildrens)
  {
    FeatureDictionaryPtr res = new FeatureDictionary("random");
    if (maxChildrens)
    {
      size_t n = maxChildrens;//lbcpp::Random::getInstance().sampleSize(maxChildrens);
      for (size_t i = 0; i < n; ++i)
        res->addScope("pouet " + lbcpp::toString(i), createRandomDictionary(maxChildrens - 1));
    }
    return res;
  }
};

extern void declareProteinsClasses();

class ExplorerApplication : public JUCEApplication
{
public:
  ExplorerApplication() : mainWindow(0) {/*_crtBreakAlloc = 3905;*/}

  virtual void initialise(const String& commandLine)
  {    
    ErrorHandler::setInstance(explorerErrorHandler);
    declareProteinsClasses();

    theCommandManager = new ApplicationCommandManager();

    mainWindow = new ExplorerMainWindow();
    mainWindow->setVisible(true);

//    ImageCache::setCacheTimeout(30 * 1000);
  }
  
  virtual void shutdown()
  {
    delete mainWindow;
    mainWindow = 0;
    deleteAndZero(theCommandManager); 
  }

  virtual void systemRequestedQuit()
  {
    if (mainWindow)
    {
      deleteAndZero(mainWindow);
//      resources::icons.clear();
//      StoredSettings::deleteInstance();
//      nieme::uninitialize();
      quit();
    }
  }

  virtual const String getApplicationName()
    {return JUCE_T("LBC++ Explorer");}

  const String getApplicationVersion()
    {return JUCE_T("1.0");}

  virtual bool moreThanOneInstanceAllowed()
  {
#ifndef JUCE_LINUX
    return false;
#else
    return true; //xxx should be false but doesn't work on linux..
#endif
  }

  virtual void anotherInstanceStarted(const String& commandLine)
  {
//    if (mainWindow != 0)
//      mainWindow->openFile(File::getCurrentWorkingDirectory().getChildFile(commandLine.unquoted()));
  }

  juce_UseDebuggingNewOperator

private:
  ExplorerMainWindow* mainWindow;
};

// START_JUCE_APPLICATION(ExplorerApplication)
// pb de link ...

int main (int argc, char* argv[])
{
  return JUCEApplication::main (argc, argv, new ExplorerApplication());
}
