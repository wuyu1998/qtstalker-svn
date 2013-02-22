/*
 *  Qtstalker stock charter
 *
 *  Copyright (C) 2001-2010 Stefan S. Stratigakos
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include "STOCHSObject.h"
#include "ta_libc.h"
#include "STOCHSDialog.h"
#include "Util.h"

#include <QtDebug>


STOCHSObject::STOCHSObject (QString profile, QString name)
{
  TA_RetCode rc = TA_Initialize();
  if (rc != TA_SUCCESS)
    qDebug() << "STOCHSObject::STOCHSObject: error on TA_Initialize";
  
  _profile = profile;
  _name = name;
  _plugin = QString("STOCHS");
  _type = QString("indicator");
  _kmaType = QString("EMA");
  _dmaType = QString("EMA");
  _period = 5;
  _kperiod = 3;
  _dperiod = 3;
  _outputKKey = QString("k");
  _outputDKey = QString("d");
  _hasOutput = TRUE;
  _inputObject = QString("symbol");
  _highKey = QString("H");
  _lowKey = QString("L");
  _closeKey = QString("C");
  
  _commandList << QString("update");
  _commandList << QString("dialog");
  _commandList << QString("output");
  _commandList << QString("load");
  _commandList << QString("save");
  _commandList << QString("output_keys");

  Util util;
  Object *o = util.object(QString("MA"), QString(), QString());
  if (o)
  {
    QString key("types");
    ObjectCommand toc(key);
    if (o->message(&toc))
      _maList = toc.getList(key);
    delete o;
  }
}

STOCHSObject::~STOCHSObject ()
{
  clear();
}

void
STOCHSObject::clear ()
{
  qDeleteAll(_bars);
  _bars.clear();
}

int
STOCHSObject::message (ObjectCommand *oc)
{
  int rc = 0;

  switch (_commandList.indexOf(oc->command()))
  {
    case 0:
      rc = update(oc);
      break;
    case 1:
      rc = dialog(oc);
      break;
    case 2:
      rc = output(oc);
      break;
    case 3:
      rc = load(oc);
      break;
    case 4:
      rc = save(oc);
      break;
    case 5:
      rc = outputKeys(oc);
      break;
    default:
      break;
  }
  
  return rc;
}

int
STOCHSObject::update (ObjectCommand *oc)
{
  clear();
  
  // input object
  Object *io = (Object *) oc->getObject(_inputObject);
  if (! io)
  {
    qDebug() << "STOCHSObject::update: invalid input" << _inputObject;
    return 0;
  }
  
  // get input bars
  ObjectCommand toc(QString("output"));
  if (! io->message(&toc))
  {
    qDebug() << "STOCHSObject::update: message error" << io->plugin() << toc.command();
    return 0;
  }

  QMap<int, Data *> data = toc.map();
  
  int size = data.size();
  TA_Real high[size];
  TA_Real low[size];
  TA_Real close[size];
  TA_Real out[size];
  TA_Real out2[size];
  TA_Integer outBeg;
  TA_Integer outNb;
  int dpos = 0;
  QMapIterator<int, Data *> it(data);
  while (it.hasNext())
  {
    it.next();
    Data *d = it.value();
    
    if (! d->contains(_highKey))
      continue;
    if (! d->contains(_lowKey))
      continue;
    if (! d->contains(_closeKey))
      continue;
    
    high[dpos] = (TA_Real) d->value(_highKey).toDouble();
    low[dpos] = (TA_Real) d->value(_lowKey).toDouble();
    close[dpos++] = (TA_Real) d->value(_closeKey).toDouble();
  }
  
  TA_RetCode rc = TA_STOCH(0,
                            dpos - 1,
                            &high[0],
                            &low[0],
                            &close[0],
                            _period,
                            _kperiod,
                            (TA_MAType) _maList.indexOf(_kmaType),
                            _dperiod,
                            (TA_MAType) _maList.indexOf(_dmaType),
                            &outBeg,
                            &outNb,
                            &out[0],
                            &out2[0]);
  
  if (rc != TA_SUCCESS)
  {
    qDebug() << "STOCHSObject::update: TA-Lib error" << rc;
    return 0;
  }

  int outLoop = outNb - 1;
  it.toBack();
  while (it.hasPrevious() && outLoop > -1)
  {
    it.previous();
    Data *b = new Data;
    b->insert(_outputKKey, out[outLoop]);
    b->insert(_outputDKey, out2[outLoop--]);
    _bars.insert(it.key(), b);
  }
  
  return 1;
}

int
STOCHSObject::dialog (ObjectCommand *oc)
{
  STOCHSDialog *dialog = new STOCHSDialog(oc->getObjects(), _name);
  dialog->setSettings(_inputObject, _highKey, _lowKey, _closeKey, _kmaType, _dmaType, _period, _kperiod, _dperiod);
  connect(dialog, SIGNAL(signalDone(void *)), this, SLOT(dialogDone(void *)));
  dialog->setModified(FALSE);
  dialog->show();
  return 1;
}

void
STOCHSObject::dialogDone (void *dialog)
{
  STOCHSDialog *d = (STOCHSDialog *) dialog;
  d->settings(_inputObject, _highKey, _lowKey, _closeKey, _kmaType, _dmaType, _period, _kperiod, _dperiod);
  
  ObjectCommand oc(QString("modified"));
  emit signalMessage(oc);
}

int
STOCHSObject::outputKeys (ObjectCommand *oc)
{
  QStringList keys;
  keys << _outputKKey << _outputDKey;
  oc->setValue(QString("output_keys"), keys);
  return 1;
}

int
STOCHSObject::output (ObjectCommand *oc)
{
  outputKeys(oc);
  oc->setMap(_bars);
  return 1;
}

int
STOCHSObject::load (ObjectCommand *oc)
{
  QSettings *settings = (QSettings *) oc->getObject(QString("QSettings"));
  if (! settings)
  {
    qDebug() << "STOCHSObject::load: invalid QSettings";
    return 0;
  }
  
  _inputObject = settings->value(QString("input_object")).toString();
  _highKey = settings->value(QString("high_key")).toString();
  _lowKey = settings->value(QString("low_key")).toString();
  _closeKey = settings->value(QString("close_key")).toString();
  _kmaType = settings->value(QString("ktype")).toString();
  _dmaType = settings->value(QString("dtype")).toString();
  _period = settings->value(QString("period")).toInt();
  _kperiod = settings->value(QString("kperiod")).toInt();
  _dperiod = settings->value(QString("dperiod")).toInt();

  return 1;
}

int
STOCHSObject::save (ObjectCommand *oc)
{
  QSettings *settings = (QSettings *) oc->getObject(QString("QSettings"));
  if (! settings)
  {
    qDebug() << "STOCHSObject::save: invalid QSettings";
    return 0;
  }
  
  settings->setValue(QString("plugin"), _plugin);
  settings->setValue(QString("input_object"), _inputObject);
  settings->setValue(QString("high_key"), _highKey);
  settings->setValue(QString("low_key"), _lowKey);
  settings->setValue(QString("close_key"), _closeKey);
  settings->setValue(QString("ktype"), _kmaType);
  settings->setValue(QString("dtype"), _dmaType);
  settings->setValue(QString("period"), _period);
  settings->setValue(QString("kperiod"), _kperiod);
  settings->setValue(QString("dperiod"), _dperiod);

  return 1;
}
