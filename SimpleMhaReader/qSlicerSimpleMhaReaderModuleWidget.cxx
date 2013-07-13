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
#include <QDebug>

// SlicerQt includes
#include "qSlicerSimpleMhaReaderModuleWidget.h"
#include "ui_qSlicerSimpleMhaReaderModuleWidget.h"

#include "vtkSlicerSimpleMhaReaderLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSimpleMhaReaderModuleWidgetPrivate: public Ui_qSlicerSimpleMhaReaderModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMhaReaderModuleWidget)
protected:
  qSlicerSimpleMhaReaderModuleWidget* const q_ptr;
public:
  ~qSlicerSimpleMhaReaderModuleWidgetPrivate();
  qSlicerSimpleMhaReaderModuleWidgetPrivate(qSlicerSimpleMhaReaderModuleWidget& object);
  vtkSlicerSimpleMhaReaderLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerSimpleMhaReaderModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModuleWidgetPrivate::~qSlicerSimpleMhaReaderModuleWidgetPrivate()
{
}

qSlicerSimpleMhaReaderModuleWidgetPrivate::qSlicerSimpleMhaReaderModuleWidgetPrivate(qSlicerSimpleMhaReaderModuleWidget& object): q_ptr(&object)
{
}

vtkSlicerSimpleMhaReaderLogic* qSlicerSimpleMhaReaderModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSimpleMhaReaderModuleWidget);
  return vtkSlicerSimpleMhaReaderLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerSimpleMhaReaderModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModuleWidget::qSlicerSimpleMhaReaderModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSimpleMhaReaderModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderModuleWidget::~qSlicerSimpleMhaReaderModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSimpleMhaReaderModuleWidget::setup()
{
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect(d->filePathLineEdit, SIGNAL(currentPathChanged(const QString&)), this, SLOT(onFileChanged(const QString&)));
  connect(d->nextPushButton, SIGNAL(clicked()), this, SLOT(onNextImage()));
  connect(d->previousPushButton, SIGNAL(clicked()), this, SLOT(onPreviousImage()));
  
  connect(d->previousValidFrameButton, SIGNAL(clicked()), this, SLOT(onPreviousValidFrame()));
  connect(d->nextValidFrameButton, SIGNAL(clicked()), this, SLOT(onNextValidFrame()));
  connect(d->previousInvalidFrameButton, SIGNAL(clicked()), this, SLOT(onPreviousInvalidFrame()));
  connect(d->nextInvalidFrameButton, SIGNAL(clicked()), this, SLOT(onNextInvalidFrame()));
  
  qvtkConnect(d->logic(), vtkCommand::ModifiedEvent, this, SLOT(updateState()));
}

void qSlicerSimpleMhaReaderModuleWidget::onFileChanged(const QString& path)
{
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  vtkSlicerSimpleMhaReaderLogic* logic = d->logic();
  logic->setMhaPath(path.toStdString());
}

void qSlicerSimpleMhaReaderModuleWidget::updateState()
{
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  vtkSlicerSimpleMhaReaderLogic* logic = d->logic();
  ostringstream oss;
  oss << logic->getCurrentFrame() << "/" << logic->getNumberOfFrames();
  d->currentFrameLabel->setText(oss.str().c_str());
  
  d->transformStatusLabel->setText(logic->getCurrentTransformStatus().c_str());
  
}

SLOTDEF_0(onNextImage, nextImage);
SLOTDEF_0(onPreviousImage, previousImage);
SLOTDEF_0(onPreviousValidFrame, previousValidFrame);
SLOTDEF_0(onNextValidFrame, nextValidFrame);
SLOTDEF_0(onPreviousInvalidFrame, previousInvalidFrame);
SLOTDEF_0(onNextInvalidFrame, nextInvalidFrame);

