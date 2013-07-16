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
#include <QTimer>

// SlicerQt includes
#include "qSlicerSimpleMhaReaderModuleWidget.h"
#include "ui_qSlicerSimpleMhaReaderModuleWidget.h"

#include "vtkSlicerSimpleMhaReaderLogic.h"

// STL includes
#include <set>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSimpleMhaReaderModuleWidgetPrivate: public Ui_qSlicerSimpleMhaReaderModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMhaReaderModuleWidget)
protected:
  qSlicerSimpleMhaReaderModuleWidget* const q_ptr;
  QTimer* timer;
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
  delete timer;
}

qSlicerSimpleMhaReaderModuleWidgetPrivate::qSlicerSimpleMhaReaderModuleWidgetPrivate(qSlicerSimpleMhaReaderModuleWidget& object): q_ptr(&object)
{
  timer = new QTimer;
  timer->setInterval(100);
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
  connect(d->playPushButton, SIGNAL(clicked()), this, SLOT(onPlayToggle()));
  connect(d->timer, SIGNAL(timeout()), this, SLOT(onPlayNext()));
  connect(d->playIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPlayIntervalChanged(int)));
  connect(d->playModeComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onPlayModeChanged(const QString&)));
  connect(d->applyTransformsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onApplyTransformsChanged(int)));
  
  connect(d->frameSlider, SIGNAL(valueChanged(int)), this, SLOT(onFrameSliderChanged(int)));
  
  d->logic()->setConsole(d->consoleTextEdit);
  
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
  oss.clear(); oss.str("");
  oss << logic->getImageWidth() << "x" << logic->getImageHeight();
  d->imageDimensionsLabel->setText(oss.str().c_str());
  d->frameSlider->blockSignals(true);
  d->frameSlider->setMaximum(logic->getNumberOfFrames());
  d->frameSlider->setValue(logic->getCurrentFrame());
  d->frameSlider->blockSignals(false);
  std::set<std::string> availableTransforms = logic->getAvailableTransforms();
  std::string avTransText;
  for(std::set<std::string>::iterator it=availableTransforms.begin(); it!=availableTransforms.end(); it++)
  {
    avTransText+=*it + ", ";
  }
  d->availableTransformsLabel->setText(avTransText.c_str());
}

// SLOTDEF_0(onNextImage, nextImage);

void qSlicerSimpleMhaReaderModuleWidget::onNextImage(){
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  d->logic()->nextImage();
}

void qSlicerSimpleMhaReaderModuleWidget::onPlayToggle() {
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  if(d->timer->isActive()){
    d->playPushButton->setText(QString("Start Playing"));
    d->timer->stop();
  }
  else {
    d->playPushButton->setText(QString("Stop Playing"));
    d->timer->start();
  }
}

void qSlicerSimpleMhaReaderModuleWidget::onPlayIntervalChanged(int value)
{
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  d->timer->setInterval(value);
}

void qSlicerSimpleMhaReaderModuleWidget::onPlayModeChanged(const QString& text){
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  d->logic()->setPlayMode(text.toStdString());
}

void qSlicerSimpleMhaReaderModuleWidget::onApplyTransformsChanged(int state){
  Q_D(qSlicerSimpleMhaReaderModuleWidget);
  if(state == Qt::Checked)
    d->logic()->setApplyTransforms(true);
  else if(state == Qt::Unchecked)
    d->logic()->setApplyTransforms(false);
}

SLOTDEF_0(onPreviousImage, previousImage);
SLOTDEF_0(onPreviousValidFrame, previousValidFrame);
SLOTDEF_0(onNextValidFrame, nextValidFrame);
SLOTDEF_0(onPreviousInvalidFrame, previousInvalidFrame);
SLOTDEF_0(onNextInvalidFrame, nextInvalidFrame);
SLOTDEF_0(onPlayNext, playNext);
SLOTDEF_1(int, onFrameSliderChanged, goToFrame);

