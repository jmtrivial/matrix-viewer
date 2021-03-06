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
#include "matrix-converter.hh"

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QSettings>
#include <QDebug>

#include "config.hh"

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif

CMatrixConverter::CMatrixConverter()
  : QObject()
  , m_data()
  , m_format(Format_Unknown)
{
}

CMatrixConverter::CMatrixConverter(const QString & filename)
  : QObject()
  , m_data()
{
  if (!load(filename))
    qWarning() << "Can't load file: " << filename;
}


CMatrixConverter::~CMatrixConverter()
{
}

bool CMatrixConverter::load(const QString & filename)
{
  if (filename.endsWith(".xml"))
    return loadFromXml(filename);
  else if (filename.endsWith(".txt"))
    return loadFromTxt(filename);
  else if (filename.endsWith(".raw"))
    return loadFromRaw(filename);
  else
    return loadFromImage(filename);

  return false;
}

bool CMatrixConverter::save(const QString & filename)
{
  if (filename.endsWith(".xml"))
    return saveToXml(filename);
  else if (filename.endsWith(".txt"))
    return saveToTxt(filename);
  else if (filename.endsWith(".raw"))
    return saveToRaw(filename);
  else if (filename.endsWith(".bmp"))
    return saveToImage(filename);
  else
    qWarning() << tr("Unsupported save format for file:") << filename;

  return false;
}

cv::Mat CMatrixConverter::data() const
{
  return m_data;
}

void CMatrixConverter::setData(const cv::Mat & matrix)
{
  m_data = matrix;
}

bool CMatrixConverter::isFormatData() const
{
  return m_format == Format_Xml || m_format == Format_Txt;
}

bool CMatrixConverter::isFormatImage() const
{
  return (m_format == Format_Bmp ||
          m_format == Format_Jpg ||
          m_format == Format_Png ||
          m_format == Format_Raw);
}

bool CMatrixConverter::loadFromTxt(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream stream(&file);

      // first line contains the number of columns and rows of the matrix
      QStringList values = stream.readLine().split(" ");
      const uint rows = values[1].toInt(), cols = values[0].toInt();
      if (rows == 0 || cols == 0)
        {
          qWarning() << tr("CMatrixConverter::loadFromTxt invalid dimensions for matrix: ") << filename;
          return false;
        }

      m_data.create(rows, cols, CV_64FC1);

      // second line contains matrix values
      values = stream.readLine().split(" ");

#pragma omp parallel for schedule(static)
      for (uint v = 0; v < rows * cols; ++v)
	{
	  int row = v / cols;
	  int col = v % cols;
	  m_data.at< double >(row, col) = values[v].toDouble();
	}

      file.close();

      // Set file format
      m_format = Format_Txt;

      return true;
    }
  else
    {
      qWarning() << tr("CMatrixConverter::loadFromTxt unable to open: ") << filename;
      return false;
    }
}

bool CMatrixConverter::saveToTxt(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream(&file);
      stream << m_data.cols << " " << m_data.rows << "\n";

      for (int j = 0; j < m_data.rows; ++j)
	for (int i = 0; i < m_data.cols; ++i)
	  stream << m_data.at< double >(j, i) << " ";

      file.close();
      return true;
    }
  else
    {
      qWarning() << tr("CMatrixConverter::saveToTxt unable to open: ") << filename;
      return false;
    }
}

bool CMatrixConverter::loadFromXml(const QString & filename)
{
  cv::FileStorage fs;
  if (!fs.open(filename.toStdString(), cv::FileStorage::READ))
    return false;

  try
    {
      fs["matrix"] >> m_data;
      fs.release();
    }
  catch (cv::Exception & e)
    {
      qWarning() << tr("CMatrixConverter::loadFromXml invalid matrix: ") << filename;
      return false;
    }

  // Set file format
  m_format = Format_Xml;

  return true;
}

bool CMatrixConverter::saveToXml(const QString & filename)
{
  cv::FileStorage fs;
  if (!fs.open(filename.toStdString(), cv::FileStorage::WRITE))
    return false;

  fs << "matrix" << m_data;
  fs.release();

  return true;

}

bool CMatrixConverter::loadFromImage(const QString & filename)
{
  try
    {
      m_data = cv::imread(filename.toStdString());
    }
  catch (cv::Exception & e)
    {
      qWarning() << tr("CMatrixConverter::loadFromImage invalid matrix: ") << filename;
      return false;
    }

  // Set file format according to extension
  if (filename.endsWith(".bmp", Qt::CaseInsensitive))
    m_format = Format_Bmp;
  else if (filename.endsWith(".jpg", Qt::CaseInsensitive))
    m_format = Format_Jpg;
  else if (filename.endsWith(".png", Qt::CaseInsensitive))
    m_format = Format_Png;

  return true;
}

bool CMatrixConverter::saveToImage(const QString & filename)
{
  bool ret;
  try
    {
      ret = cv::imwrite(filename.toStdString(), m_data);
    }
  catch (cv::Exception & e)
    {
      qWarning() << tr("CMatrixConverter::saveToImage invalid matrix: ") << filename;
      return false;
    }
  return ret;
}

bool CMatrixConverter::loadFromRaw(const QString & filename)
{
  QSettings settings;
  settings.beginGroup("raw");
  int type = settings.value("type", 0).toInt();
  int width = settings.value("width", 2592).toInt();
  int height = settings.value("height", 1944).toInt();
  settings.endGroup();

  if (type == 0)
    {
      unsigned short *buffer = new unsigned short[width * height];
      if (FILE* fd = fopen(filename.toStdString().c_str(), "rb"))
        {
          fread(buffer, sizeof(unsigned short), width * height, fd);
        }
      else
        {
          qWarning() << tr("Can't open raw image file in read mode: %1").arg(filename);
          delete [] buffer;
          return false;
        }

      cv::Mat data(cv::Size(width, height), CV_16U, buffer);

      qDebug() << data.type() << data.channels();
      const double ratio = 127.5 / 511.5; // convertion from 10bits to 8 bits
      m_data = data * ratio;
      m_data.convertTo(m_data, CV_8UC1);
      delete [] buffer;
    }
  else
    {
      qWarning() << "CMatrixConverter::loadFromRaw format not supported yet";
      return false;
    }

  m_format = Format_Raw;

  return true;
}

bool CMatrixConverter::saveToRaw(const QString & filename)
{
  Q_UNUSED(filename);
  qWarning() << "CMatrixConverter::saveToRaw not implemented yet";
  return false;
}

void CMatrixConverter::print() const
{
  for (int i = 0; i < m_data.rows; ++i)
    {
      for (int j = 0; j < m_data.cols; ++j)
	{
	  std::cout << m_data.at< double >(i, j);
	}
      std::cout << std::endl;
    }
}
