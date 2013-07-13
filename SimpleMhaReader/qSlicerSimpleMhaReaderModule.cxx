/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// SimpleMhaReader Logic includes
#include <vtkSlicerSimpleMhaReaderLogic.h>

// SimpleMhaReader includes
#include "qSlicerSimpleMhaReaderModule.h"
#include "qSlicerSimpleMhaReaderModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSimpleMhaReaderModule, qSlicerSimpleMhaReaderModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSimpleMhaReaderModulePrivate
{
public:
  qSlicerSimpleMhaReaderModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSimpleMhaReaderModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModulePrivate
::qSlicerSimpleMhaReaderModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSimpleMhaReaderModule methods

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModule
::qSlicerSimpleMhaReaderModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSimpleMhaReaderModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModule::~qSlicerSimpleMhaReaderModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSimpleMhaReaderModule::helpText()const
{
  return "This is a loadable module bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerSimpleMhaReaderModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSimpleMhaReaderModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSimpleMhaReaderModule::icon()const
{
  return QIcon(":/Icons/SimpleMhaReader.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSimpleMhaReaderModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSimpleMhaReaderModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMhaReaderModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSimpleMhaReaderModule
::createWidgetRepresentation()
{
  return new qSlicerSimpleMhaReaderModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSimpleMhaReaderModule::createLogic()
{
  return vtkSlicerSimpleMhaReaderLogic::New();
}
