/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerSimpleMhaReaderFooBarWidget.h"
#include "ui_qSlicerSimpleMhaReaderFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SimpleMhaReader
class qSlicerSimpleMhaReaderFooBarWidgetPrivate
  : public Ui_qSlicerSimpleMhaReaderFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMhaReaderFooBarWidget);
protected:
  qSlicerSimpleMhaReaderFooBarWidget* const q_ptr;

public:
  qSlicerSimpleMhaReaderFooBarWidgetPrivate(
    qSlicerSimpleMhaReaderFooBarWidget& object);
  virtual void setupUi(qSlicerSimpleMhaReaderFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerSimpleMhaReaderFooBarWidgetPrivate
::qSlicerSimpleMhaReaderFooBarWidgetPrivate(
  qSlicerSimpleMhaReaderFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSimpleMhaReaderFooBarWidgetPrivate
::setupUi(qSlicerSimpleMhaReaderFooBarWidget* widget)
{
  this->Ui_qSlicerSimpleMhaReaderFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerSimpleMhaReaderFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderFooBarWidget
::qSlicerSimpleMhaReaderFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSimpleMhaReaderFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerSimpleMhaReaderFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerSimpleMhaReaderFooBarWidget
::~qSlicerSimpleMhaReaderFooBarWidget()
{
}
