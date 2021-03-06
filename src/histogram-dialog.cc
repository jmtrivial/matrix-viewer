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
#include "histogram-dialog.hh"

#include <QDialogButtonBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>

#include "main-window.hh"
#include "histogram.hh"

const QColor CHistogramDialog::_red(239, 41, 41);
const QColor CHistogramDialog::_green(138, 226, 52);
const QColor CHistogramDialog::_blue(114, 159, 207);

CHistogramDialog::CHistogramDialog(QWidget *p)
  : QDialog(p)
  , m_parent(qobject_cast<CMainWindow*>(p))
  , m_redHistogram(new CHistogram)
  , m_greenHistogram(new CHistogram)
  , m_blueHistogram(new CHistogram)
{
  setWindowTitle(tr("Histogram"));

  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));

  QBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(m_redHistogram);
  layout->addLayout(makeAxisLayout(_red));

  layout->addWidget(m_greenHistogram);
  layout->addLayout(makeAxisLayout(_green));

  layout->addWidget(m_blueHistogram);
  layout->addLayout(makeAxisLayout(_blue));

  layout->addStretch();
  layout->addWidget(buttons);
  setLayout(layout);

  resize(QSize(400, 100));
}

CHistogramDialog::~CHistogramDialog()
{
}

void CHistogramDialog::setImage(QImage *image)
{
  QImage copy = image->convertToFormat(QImage::Format_ARGB32);
  QVector<int> redValues(256, 0), greenValues(256, 0), blueValues(256, 0);
  for (int j = 0; j < image->height(); ++j)
    {
      QRgb *row = (QRgb *)copy.scanLine(j);
      for (int i = 0; i < image->width(); ++i)
	{
	  const QColor color = QColor(row[i]);
	  ++redValues[color.red()];
	  ++greenValues[color.green()];
	  ++blueValues[color.blue()];
	}
    }

  m_redHistogram->setValues(redValues, Qt::NoPen, QBrush(_red));
  m_greenHistogram->setValues(greenValues, Qt::NoPen, QBrush(_green));
  m_blueHistogram->setValues(blueValues, Qt::NoPen, QBrush(_blue));
}

QBoxLayout * CHistogramDialog::makeAxisLayout(const QColor & color)
{
  QString css = QString("QLabel{background-color: "
			"qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, "
			"stop: 0 black, stop: 1 %1);}").arg(color.name());

  QLabel *begin = new QLabel("0");
  QLabel *end = new QLabel("255");

  QLabel *gradient = new QLabel;
  gradient->setStyleSheet(css);

  QBoxLayout * layout = new QHBoxLayout;
  layout->addWidget(begin);
  layout->addWidget(gradient, 1);
  layout->addWidget(end);

  return layout;
}

CMainWindow* CHistogramDialog::parent() const
{
  if (!m_parent)
    qWarning() << "CHistogramDialog::parent invalid parent";
  return m_parent;
}
