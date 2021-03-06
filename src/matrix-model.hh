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

#ifndef __MATRIX_MODEL_HH__
#define __MATRIX_MODEL_HH__

#include <QAbstractTableModel>
#include <QStringList>
#include <opencv2/opencv.hpp>

/*!
  \file matrix-model.hh
  \class CMatrixModel
  \brief CMatrixModel is a model that represent matrix data
*/

class CMatrixModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /// Constructor.
  CMatrixModel();

  /// Destructor.
  virtual ~CMatrixModel();

  cv::Mat data() const;
  void setData(const cv::Mat & matrix);

  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  Qt::ItemFlags flags(const QModelIndex & index) const;

  void setProfile(const QString & profile);

  // OpenCV wrappers
  int channels() const;
  int type() const;
  QString typeString() const;

private:

  cv::Mat m_data;
  QStringList m_horizontalHeaderLabels;
  QStringList m_verticalHeaderLabels;
};

#endif  // __MATRIX_MODEL_HH__
