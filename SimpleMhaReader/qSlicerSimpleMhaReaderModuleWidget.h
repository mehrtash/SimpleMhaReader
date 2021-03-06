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

#ifndef __qSlicerSimpleMhaReaderModuleWidget_h
#define __qSlicerSimpleMhaReaderModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSimpleMhaReaderModuleExport.h"

class qSlicerSimpleMhaReaderModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SIMPLEMHAREADER_EXPORT qSlicerSimpleMhaReaderModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSimpleMhaReaderModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSimpleMhaReaderModuleWidget();

public slots:
  void onFileChanged(const QString&);
  void onFrameSliderChanged(int);
  void onNextImage();
  void onPreviousImage();
  void onNextValidFrame();
  void onPreviousValidFrame();
  void onNextInvalidFrame();
  void onPreviousInvalidFrame();
  void updateState();
  void onPlayIntervalChanged(int);
  void onPlayToggle();
  void onPlayModeChanged(const QString&);
  void onPlayNext();
  void onApplyTransformsChanged(int);
  void onSaveToPng();

protected:
  QScopedPointer<qSlicerSimpleMhaReaderModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerSimpleMhaReaderModuleWidget);
  Q_DISABLE_COPY(qSlicerSimpleMhaReaderModuleWidget);
};

#endif
