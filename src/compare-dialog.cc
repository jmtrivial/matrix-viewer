// Copyright (C) 2013, Romain Goffe <romain.goffe@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************
#include "compare-dialog.hh"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QSettings>
#include <QFormLayout>
#include <QBoxLayout>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

#include "file-chooser.hh"
#include "matrix-converter.hh"
#include "matrix-model.hh"
#include "matrix-view.hh"
#include "main-window.hh"
#include "tab-widget.hh"
#include "delegate.hh"


CCompareDialog::CCompareDialog(QWidget *parent)
  : QDialog(parent)
  , m_parent(qobject_cast<CMainWindow*>(parent))
  , m_fileChooser(new CFileChooser)
  , m_absoluteThreshold(0)
  , m_percentageThreshold(0)
{
  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
  connect(buttons, SIGNAL(accepted()), this, SLOT(compare()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(close()));

  m_fileChooser->setCaption(tr("Open comparison data file"));
  m_fileChooser->setFilter(tr("Data files (*.xml *.txt)"));

  QGroupBox *compareParametersGroupBox = new QGroupBox(tr("Thresholds"));

  QSpinBox *absoluteThresholdSpinBox = new QSpinBox;
  QSpinBox *percentageThresholdSpinBox = new QSpinBox;

  QFormLayout *parametersLayout = new QFormLayout;
  parametersLayout->addRow(tr("Absolute threshold:"), absoluteThresholdSpinBox);
  parametersLayout->addRow(tr("Percentage threshold:"), percentageThresholdSpinBox);
  compareParametersGroupBox->setLayout(parametersLayout);

  QBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(m_fileChooser);
  layout->addWidget(compareParametersGroupBox);
  layout->addWidget(buttons);

  setLayout(layout);

  readSettings();
}

CCompareDialog::~CCompareDialog()
{
  writeSettings();
}


void CCompareDialog::readSettings()
{
  QSettings settings;
  settings.beginGroup("compare");
  m_absoluteThreshold = settings.value("absoluteThreshold", true).toDouble();
  m_percentageThreshold = settings.value("percentageThreshold", true).toDouble();
  settings.endGroup();
}

void CCompareDialog::writeSettings()
{
}

void CCompareDialog::compare()
{
  if (m_fileChooser->path().isEmpty())
    {
      qWarning() << tr("Empty filename");
      return;
    }

  CMatrixConverter converter(m_fileChooser->path());
  cv::Mat originalData = parent()->currentData();
  cv::Mat newData = converter.data();

  if (converter.data().rows != originalData.rows ||
      converter.data().cols != originalData.cols)
    {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(tr("The two matrix cannot be compared because they have different sizes."));
      msgBox.setInformativeText(tr("First matrix: %1 rows x %2 cols\nSecond matrix: %3 rows x %4 cols.")
				.arg(originalData.rows).arg(originalData.cols)
				.arg(newData.rows).arg(newData.cols));
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
      close();
      return;
    }

  // New model/view
  CMatrixModel *model = new CMatrixModel();
  model->setData(newData);

  CMatrixView *view = new CMatrixView;
  view->setModel(model);
  parent()->currentWidget()->addWidget(view);

  // Diff model/view
  cv::Mat diffData;
  cv::absdiff(originalData, newData, diffData);

  CMatrixModel *diffModel = new CMatrixModel();
  diffModel->setData(diffData);

  CMatrixView *diffView = new CMatrixView;
  diffView->setModel(diffModel);
  diffView->setItemDelegate(new CDelegate);
  parent()->currentWidget()->addWidget(diffView);

  accept();
}

CMainWindow* CCompareDialog::parent() const
{
  if (!m_parent)
    qWarning() << "CCompareDialog::parent invalid parent";
  return m_parent;
}



