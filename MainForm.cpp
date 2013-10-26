/*
 * File:   MainForm.cpp
 * Author: Ivan
 *
 * Created on 24 Октябрь 2011 г., 14:57
 */

#include "MainForm.h"

#define FTD2XX_EXPORTS
#include "ftd2xx.h"
#include <stdio.h>
#define MAX_BUFFER_LENGTH 524288
#include <QFileDialog>
#include "structs.h"
#include <QtEndian>
#include <math.h>

MainForm::MainForm() {
    handle_device = 0;
    cs_error_counter = 0;
    bfr.clear();
    widget.setupUi(this);
    
    plotForm.setParent(this);
    plotForm.setWindowFlags(Qt::Window);
    plotForm.setAxisXType(PlotForm::AxisXTime);
    plotSpectrum.setParent(this);
    plotSpectrum.setWindowFlags(Qt::Window);
    plotSpectrum.setDataLen(512);
    plotSpectrum.setAxisXType(PlotForm::AxisXDefault);
    
    loadLibrary();
    loadSettings();
    
    rval = new QRegExpValidator(QRegExp("[0-9a-fA-F]+"), this);
    dval = new QDoubleValidator();
    ival = new QIntValidator();

    widget.leAddr0->setValidator(rval);
    widget.leAddr1->setValidator(rval);
    widget.leAddr2->setValidator(rval);
    widget.leAddr3->setValidator(rval);
    widget.leAddr4->setValidator(rval);
    widget.leAddr5->setValidator(rval);
    widget.leAddr6->setValidator(rval);
    widget.leAddr7->setValidator(rval);
    widget.leData0->setValidator(rval);
    widget.leData1->setValidator(rval);
    widget.leData2->setValidator(rval);
    widget.leData3->setValidator(rval);
    widget.leData4->setValidator(rval);
    widget.leData5->setValidator(rval);
    widget.leData6->setValidator(rval);
    widget.leData7->setValidator(rval);
    
    widget.leDiscriminator->setValidator(rval);
    widget.leFrequency->setValidator(dval);
    
    connect(widget.btnConnect, SIGNAL(clicked()), this, SLOT(deviceCheck()));
    connect(widget.btnRead0  , SIGNAL(clicked()), this, SLOT(deviceRead0()));
    connect(widget.btnRead1  , SIGNAL(clicked()), this, SLOT(deviceRead1()));
    connect(widget.btnRead2  , SIGNAL(clicked()), this, SLOT(deviceRead2()));
    connect(widget.btnRead3  , SIGNAL(clicked()), this, SLOT(deviceRead3()));
    connect(widget.btnRead4  , SIGNAL(clicked()), this, SLOT(deviceRead4()));
    connect(widget.btnRead5  , SIGNAL(clicked()), this, SLOT(deviceRead5()));
    connect(widget.btnRead6  , SIGNAL(clicked()), this, SLOT(deviceRead6()));
    connect(widget.btnRead7  , SIGNAL(clicked()), this, SLOT(deviceRead7()));
    connect(widget.btnWrite0 , SIGNAL(clicked()), this, SLOT(deviceWrite0()));
    connect(widget.btnWrite1 , SIGNAL(clicked()), this, SLOT(deviceWrite1()));
    connect(widget.btnWrite2 , SIGNAL(clicked()), this, SLOT(deviceWrite2()));
    connect(widget.btnWrite3 , SIGNAL(clicked()), this, SLOT(deviceWrite3()));
    connect(widget.btnWrite4 , SIGNAL(clicked()), this, SLOT(deviceWrite4()));
    connect(widget.btnWrite5 , SIGNAL(clicked()), this, SLOT(deviceWrite5()));
    connect(widget.btnWrite6 , SIGNAL(clicked()), this, SLOT(deviceWrite6()));
    connect(widget.btnWrite7 , SIGNAL(clicked()), this, SLOT(deviceWrite7()));

    connect(widget.cbMap0 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap0(QString)));
    connect(widget.cbMap1 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap1(QString)));
    connect(widget.cbMap2 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap2(QString)));
    connect(widget.cbMap3 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap3(QString)));
    connect(widget.cbMap4 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap4(QString)));
    connect(widget.cbMap5 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap5(QString)));
    connect(widget.cbMap6 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap6(QString)));
    connect(widget.cbMap7 , SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMap7(QString)));
    
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(deviceUpdate()));
    connect(widget.btnPlot      , SIGNAL(clicked()), this, SLOT(showPlot()));
    connect(widget.btnReloadIni , SIGNAL(clicked()), this, SLOT(loadSettings()));
    connect(widget.btnSaveIni   , SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(widget.btnOpenIni   , SIGNAL(clicked()), this, SLOT(openIni()));
    
    connect(widget.btnFreqReset, SIGNAL(clicked()), this, SLOT(writeFreqReset()));
    connect(widget.btnFreqWrite, SIGNAL(clicked()), this, SLOT(writeFrequency()));
    connect(widget.btnDscrWrite, SIGNAL(clicked()), this, SLOT(writeDiscriminator()));
    
    connect(widget.btnOpenIni   , SIGNAL(clicked()), this, SLOT(openIni()));
    
    connect(widget.btnSaveMeasure    , SIGNAL(clicked(bool)), this, SLOT(saveMeasure()));
    connect(widget.btnClearMeasure    , SIGNAL(clicked(bool)), this, SLOT(clearMeasure()));
//    connect(widget.btnFapOffset   , SIGNAL(clicked()), this, SLOT(allowFapOffset()));

    
    connect(widget.sbDbgPlotIndex , SIGNAL(valueChanged(int)), this, SLOT(selectDbgPlotIndex(int)));
    connect(widget.sbChannelIndex , SIGNAL(valueChanged(int)), this, SLOT(selectChannel(int)));

    connect(widget.dialGain, SIGNAL(valueChanged(int)), this, SLOT(writeGain(int)));
}

MainForm::~MainForm() {

}

void MainForm::loadLibrary() {
    lib.setFileName("ftd2xx");
    if (lib.load()) widget.teLog->appendPlainText("Inf: Driver library ftd2xx loaded");
    else {
        widget.teLog->appendPlainText("Err: Driver library ftd2xx not loaded");
        return;
    }

    FT_Open            = (FT_STATUS (WINAPI *)(int, FT_HANDLE*))lib.resolve("FT_Open");
    FT_OpenEx          = (FT_STATUS (WINAPI *)(PVOID, DWORD, FT_HANDLE*))lib.resolve("FT_OpenEx");
    FT_ListDevices     = (FT_STATUS (WINAPI *)(PVOID, PVOID, DWORD))lib.resolve("FT_ListDevices");
    FT_Close           = (FT_STATUS (WINAPI *)(FT_HANDLE))lib.resolve("FT_Close");
    FT_Read            = (FT_STATUS (WINAPI *)(FT_HANDLE, LPVOID, DWORD, LPDWORD))lib.resolve("FT_Read");
    FT_Write           = (FT_STATUS (WINAPI *)(FT_HANDLE, LPVOID, DWORD, LPDWORD))lib.resolve("FT_Write");
    FT_ResetDevice     = (FT_STATUS (WINAPI *)(FT_HANDLE))lib.resolve("FT_ResetDevice");
    FT_SetTimeouts     = (FT_STATUS (WINAPI *)(FT_HANDLE, ULONG, ULONG))lib.resolve("FT_SetTimeouts");
    FT_GetStatus       = (FT_STATUS (WINAPI *)(FT_HANDLE, DWORD*, DWORD*, DWORD*))lib.resolve("FT_GetStatus");
    FT_GetDeviceInfo   = (FT_STATUS (WINAPI *)(FT_HANDLE, FT_DEVICE*, LPDWORD, PCHAR, PCHAR, LPVOID))lib.resolve("FT_GetDeviceInfo");
    FT_SetEventNotification = (FT_STATUS (WINAPI *)(FT_HANDLE, DWORD, PVOID))lib.resolve("FT_SetEventNotification");

    if (FT_Open == 0 || FT_OpenEx == 0 || FT_OpenEx == 0 || FT_ListDevices == 0 || 
        FT_Close == 0 || FT_Read == 0 || FT_Write == 0 || FT_ResetDevice == 0 ||
        FT_SetTimeouts == 0 || FT_GetStatus == 0 || FT_GetDeviceInfo == 0 || 
        FT_SetEventNotification == 0) {
        widget.teLog->appendPlainText("Err: Entry points not found");
    } else widget.teLog->appendPlainText("Inf: All entry points found");
}

void MainForm::loadSettings(QString name) {
    bool ok;
    uint64_t data;
    uint32_t addr;
    if (name.isEmpty()) name = "cockroach";
    QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "cockroach", name);
	settings.setIniCodec("UTF-8");
    
    settings.beginGroup("Map");
    QStringList list = settings.childKeys();
    addr_map.clear();
    for (int i = 0; i < list.count(); ++i) {
        addr_map.insert(settings.value(list.at(i), 0x80000000).toString().toUInt(&ok, 16), list.at(i));
    }
    
    widget.cbMap0->clear();
    widget.cbMap0->addItem("none");
    widget.cbMap0->addItems(addr_map.values());
    widget.cbMap1->clear();
    widget.cbMap1->addItem("none");
    widget.cbMap1->addItems(addr_map.values());
    widget.cbMap2->clear();
    widget.cbMap2->addItem("none");
    widget.cbMap2->addItems(addr_map.values());
    widget.cbMap3->clear();
    widget.cbMap3->addItem("none");
    widget.cbMap3->addItems(addr_map.values());
    widget.cbMap4->clear();
    widget.cbMap4->addItem("none");
    widget.cbMap4->addItems(addr_map.values());
    widget.cbMap5->clear();
    widget.cbMap5->addItem("none");
    widget.cbMap5->addItems(addr_map.values());
    widget.cbMap6->clear();
    widget.cbMap6->addItem("none");
    widget.cbMap6->addItems(addr_map.values());
    widget.cbMap7->clear();
    widget.cbMap7->addItem("none");
    widget.cbMap7->addItems(addr_map.values());
    settings.endGroup();
    
    addr = settings.value("Address0", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap0->setCurrentIndex(widget.cbMap0->findText(addr_map.value(addr, "none")));
    widget.leAddr0->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address1", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap1->setCurrentIndex(widget.cbMap1->findText(addr_map.value(addr, "none")));
    widget.leAddr1->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address2", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap2->setCurrentIndex(widget.cbMap2->findText(addr_map.value(addr, "none")));
    widget.leAddr2->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address3", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap3->setCurrentIndex(widget.cbMap3->findText(addr_map.value(addr, "none")));
    widget.leAddr3->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address4", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap4->setCurrentIndex(widget.cbMap4->findText(addr_map.value(addr, "none")));
    widget.leAddr4->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address5", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap5->setCurrentIndex(widget.cbMap5->findText(addr_map.value(addr, "none")));
    widget.leAddr5->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address6", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap6->setCurrentIndex(widget.cbMap6->findText(addr_map.value(addr, "none")));
    widget.leAddr6->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    addr = settings.value("Address7", 0x80000000).toString().toUInt(&ok, 16);
    widget.cbMap7->setCurrentIndex(widget.cbMap7->findText(addr_map.value(addr, "none")));
    widget.leAddr7->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
    
    
    data = settings.value("Data0", 0).toString().toULongLong(&ok, 16);
    widget.leData0->setText(QString("%0").arg((uint64_t)data, 16, 16, QChar('0')));
    data = settings.value("Data1", 0).toString().toULongLong(&ok, 16);
    widget.leData1->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data2", 0).toString().toULongLong(&ok, 16);
    widget.leData2->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data3", 0).toString().toULongLong(&ok, 16);
    widget.leData3->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data4", 0).toString().toULongLong(&ok, 16);
    widget.leData4->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data5", 0).toString().toULongLong(&ok, 16);
    widget.leData5->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data6", 0).toString().toULongLong(&ok, 16);
    widget.leData6->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    data = settings.value("Data7", 0).toString().toULongLong(&ok, 16);
    widget.leData7->setText(QString("%0").arg((uint64_t)data        , 16, 16, QChar('0')));
    
}

void MainForm::saveSettings() {
    bool ok;
    uint64_t data;
    QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "cockroach", "cockroach");
	settings.setIniCodec("UTF-8");
    settings.setValue("Address0", widget.leAddr0->text());
    settings.setValue("Address1", widget.leAddr1->text());
    settings.setValue("Address2", widget.leAddr2->text());
    settings.setValue("Address3", widget.leAddr3->text());
    settings.setValue("Address4", widget.leAddr4->text());
    settings.setValue("Address5", widget.leAddr5->text());
    settings.setValue("Address6", widget.leAddr6->text());
    settings.setValue("Address7", widget.leAddr7->text());

    data = (widget.leData0->text().toULongLong(&ok, 16));
    settings.setValue("Data0", QString::number(data, 16));
    data = (widget.leData1->text().toULongLong(&ok, 16));
    settings.setValue("Data1", QString::number(data, 16));
    data = (widget.leData2->text().toULongLong(&ok, 16));
    settings.setValue("Data2", QString::number(data, 16));
    data = (widget.leData3->text().toULongLong(&ok, 16));
    settings.setValue("Data3", QString::number(data, 16));
    data = (widget.leData4->text().toULongLong(&ok, 16));
    settings.setValue("Data4", QString::number(data, 16));
    data = (widget.leData5->text().toULongLong(&ok, 16));
    settings.setValue("Data5", QString::number(data, 16));
    data = (widget.leData6->text().toULongLong(&ok, 16));
    settings.setValue("Data6", QString::number(data, 16));
    data = (widget.leData7->text().toULongLong(&ok, 16));
    settings.setValue("Data7", QString::number(data, 16));

}

void MainForm::deviceCheck() {
    widget.teLog->clear();
    if (handle_device == 0)  deviceOpen();
    else deviceClose();
    if (handle_device != 0) {
        widget.btnConnect->setText("Disconnect");
        widget.btnConnect->setChecked(true);
    }
    else {
        widget.btnConnect->setText("Connect");
        widget.btnConnect->setChecked(false);
    }
    
}

void event_listener(DWORD a, DWORD b) {
    puts("event!!!");
    puts("event!!!");
    puts("event!!!");
    puts("event!!!");
    puts("event!!!");
    puts("event!!!");
    puts("event!!!");
}

void MainForm::deviceOpen() {
    uint number = 0;
    uint32_t res = FT_ListDevices(&number, 0, FT_LIST_NUMBER_ONLY);
    if (res != FT_OK) {
        widget.teLog->appendPlainText(QString("Err: FT_ListDevices return %0").arg(res));
        return;
    } else
        widget.teLog->appendPlainText(QString("Inf: Found %0 devices").arg(number));
    if (number == 0) return;

    char name[60];
	for (uint32_t i = 0; i < number; ++i) {
        res = FT_ListDevices((void *)i, name, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
		if (res != FT_OK) {
            widget.teLog->appendPlainText(QString("Err: FT_ListDevices return %0").arg(res));
//			return;
		} else 
            widget.teLog->appendPlainText(QString("Inf: Name of %0 device is '%1'").arg(i + 1).arg(name));
	}
    
    handle_device = 0;
    res = FT_Open(widget.sbDevice->value(), &handle_device);
	if (res != FT_OK) {
        widget.teLog->appendPlainText(QString("Err: FT_Open return %0").arg(res));
		return;
	}
    if (handle_device == 0) {
        widget.teLog->appendPlainText(QString("Err: Can't create handle device"));
        return;
    } else
        widget.teLog->appendPlainText(QString("Inf: First device is opened"));
    res = FT_ResetDevice(handle_device);
    if (res != FT_OK) {
        widget.teLog->appendPlainText(QString("Err: FT_ResetDevice return %0").arg(res));
		FT_Close(handle_device);
		return;
	}
    res = FT_SetTimeouts(handle_device, 5000, 2000);
    if (res != FT_OK) {
        widget.teLog->appendPlainText(QString("Err: FT_SetTimeouts return %0").arg(res));
		FT_Close(handle_device);
		return;
	}
    timer.start(20);
    
//    PFT_EVENT_HANDLER event_handler = &event_listener;
//    res = FT_SetEventNotification(handle_device, FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS, (void *)&event_handler);
//    if (res != FT_OK) {
//        widget.teLog->appendPlainText(QString("Err: FT_SetEventNotification return %0").arg(res));
//		return;
//	}
}

void MainForm::deviceClose() {
    if (handle_device == 0) return;
    uint32_t res = FT_Close(handle_device);
    if (res != FT_OK) 
        widget.teLog->appendPlainText(QString("Err: FT_Close return %0").arg(res));
    else 
        widget.teLog->appendPlainText(QString("Inf: Device closed").arg(res));
    timer.stop();
    handle_device = 0;
}

void MainForm::writeBuffer(uint8_t *buffer, unsigned long len) {
    unsigned long c = 0;
    uint32_t res = FT_Write(handle_device, buffer, len, &c);
	if (res != FT_OK || c != len) {
        widget.teLog->appendPlainText(QString("Err: FT_Write return %0. %1 bytes of 10 written").arg(res).arg(c));
		return;
	}
    if (widget.btnShowInf->isChecked())
        widget.teLog->appendPlainText(QString("Inf: %0 bytes written").arg(c));
}

void MainForm::cmdRead(uint32_t addr, char sender) {
    if (handle_device == 0) {
        widget.teLog->appendPlainText(QString("Err: Device not connected"));
        return;
    }
    const unsigned long blen = 12;
    uint32_t len = blen - 6;
    uint32_t index = 0;
    uint32_t cs_index;
    uint8_t buffer[blen];
    
    buffer[index++] = 0xFF;
    buffer[index++] = 0xAA;
    buffer[index++] = 0xBB;
    buffer[index++] = len & 0xFF;
    buffer[index++] = (len >> 8) & 0xFF;
    cs_index = index++;
    buffer[index++] = 0x02;
    buffer[index++] = sender;
    
    *((uint32_t *)&buffer[index]) = addr;
    
    buffer[cs_index] = 0;
	for (uint32_t i = cs_index + 1; i < blen; ++i) {
		buffer[cs_index] += buffer[i];
	}
    writeBuffer(buffer, blen);
}

void MainForm::cmdReadA() {
    uint32_t addr = 0x80000026;
    if (handle_device == 0) {
        widget.teLog->appendPlainText(QString("Err: Device not connected"));
        return;
    }
    const unsigned long blen = 12;
    uint32_t len = blen - 6;
    uint32_t index = 0;
    uint32_t cs_index;
    uint8_t buffer[blen];
    
    buffer[index++] = 0xFF;
    buffer[index++] = 0xAA;
    buffer[index++] = 0xBB;
    buffer[index++] = len & 0xFF;
    buffer[index++] = (len >> 8) & 0xFF;
    cs_index = index++;
    buffer[index++] = 0x0A;
    buffer[index++] = 0x0A;
    
    *((uint32_t *)&buffer[index]) = addr;
    
    buffer[cs_index] = 0;
	for (uint32_t i = cs_index + 1; i < blen; ++i) {
		buffer[cs_index] += buffer[i];
	}
    writeBuffer(buffer, blen);
}

void MainForm::cmdWrite(uint32_t addr, uint32_t ldata, uint32_t hdata, uint64_t mask) {
    if (handle_device == 0) {
        widget.teLog->appendPlainText(QString("Err: Device not connected"));
        return;
    }

    const uint32_t blen = 27;
    uint32_t len = blen - 6;
    uint32_t index = 0;
    uint32_t cs_index;
    uint8_t buffer[blen];

    buffer[index++] = 0xFF;
    buffer[index++] = 0xAA;
    buffer[index++] = 0xBB;
    buffer[index++] = len & 0xFF;
    buffer[index++] = (len >> 8) & 0xFF;
    cs_index = index++;
    buffer[index++] = 0x01;

    *((uint32_t *)&buffer[index]) = addr; // 4
    index += 4;
    *((uint32_t *)&buffer[index]) = ldata; // 4
    index += 4;
    *((uint32_t *)&buffer[index]) = hdata; // 4
    index += 4;
    *((uint64_t *)&buffer[index]) = mask;  // 8

    buffer[cs_index] = 0;
	for (uint32_t i = cs_index + 1; i < blen; ++i) {
		buffer[cs_index] += buffer[i];
	}

    writeBuffer(buffer, blen);
}

void MainForm::cmdSet(uint32_t addr, uint8_t bit, bool value) {
    if (handle_device == 0) {
        widget.teLog->appendPlainText(QString("Err: Device not connected"));
        return;
    }

    const unsigned long blen = 13;
    uint32_t len = blen - 6;
    uint32_t index = 0;
    uint32_t cs_index;
    uint8_t buffer[blen];
    
    buffer[index++] = 0xFF;
    buffer[index++] = 0xAA;
    buffer[index++] = 0xBB;
    buffer[index++] = len & 0xFF;
    buffer[index++] = (len >> 8) & 0xFF;
    cs_index = index++;
    buffer[index++] = 0x03;

    *((uint32_t *)&buffer[4]) = addr ;
    index += 4;
    buffer[index++] = bit;
    buffer[index++] = (uint8_t)value;

    buffer[cs_index] = 0;
	for (uint32_t i = cs_index + 1; i < blen; ++i) {
		buffer[cs_index] += buffer[i];
	}
    writeBuffer(buffer, blen);
}

void MainForm::deviceRead0() {
    bool ok = false;
    uint32_t addr = widget.leAddr0->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 0);
}

void MainForm::deviceRead1() {
    bool ok = false;
    uint32_t addr = widget.leAddr1->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 1);
}

void MainForm::deviceRead2() {
    bool ok = false;
    uint32_t addr = widget.leAddr2->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 2);
}

void MainForm::deviceRead3() {
    bool ok = false;
    uint32_t addr = widget.leAddr3->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 3);
}

void MainForm::deviceRead4() {
    bool ok = false;
    uint32_t addr = widget.leAddr4->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 4);
}

void MainForm::deviceRead5() {
    bool ok = false;
    uint32_t addr = widget.leAddr5->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 5);
}

void MainForm::deviceRead6() {
    bool ok = false;
    uint32_t addr = widget.leAddr6->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 6);
}

void MainForm::deviceRead7() {
    bool ok = false;
    uint32_t addr = widget.leAddr7->text().toUInt(&ok, 16);
    if (ok) cmdRead(addr, 7);
}

void MainForm::readScCode() {
    for (int i = 0; i < 6; ++i) {
        cmdWrite(0x80000026, (i << 24) & 0xF000000, 0x0, 0xF000000);
        cmdRead(0x8000003A, 0x50 + i);
    }

    for (int i = 0; i < 6; ++i) {
        cmdWrite(0x80000026, ((i + 7) << 24) & 0xF000000, 0x0, 0xF000000);
        cmdRead(0x8000003A, 0x56 + i);
    }
}

void MainForm::deviceWrite0() {
    bool ok = false;
    uint32_t addr  = widget.leAddr0->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType0, widget.leData0);
    uint64_t mask = getMaskValue(widget.cbType0);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite1() {
    bool ok = false;
    uint32_t addr  = widget.leAddr1->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType1, widget.leData1);
    uint64_t mask = getMaskValue(widget.cbType1);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite2() {
    bool ok = false;
    uint32_t addr  = widget.leAddr2->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType2, widget.leData2);
    uint64_t mask = getMaskValue(widget.cbType2);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite3() {
    bool ok = false;
    uint32_t addr  = widget.leAddr3->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType3, widget.leData3);
    uint64_t mask = getMaskValue(widget.cbType3);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite4() {
    bool ok = false;
    uint32_t addr  = widget.leAddr4->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType4, widget.leData4);
    uint64_t mask = getMaskValue(widget.cbType4);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite5() {
    bool ok = false;
    uint32_t addr  = widget.leAddr5->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType5, widget.leData5);
    uint64_t mask = getMaskValue(widget.cbType5);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite6() {
    bool ok = false;
    uint32_t addr  = widget.leAddr6->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType6, widget.leData6);
    uint64_t mask = getMaskValue(widget.cbType6);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::deviceWrite7() {
    bool ok = false;
    uint32_t addr  = widget.leAddr7->text().toUInt(&ok, 16);
    uint64_t data = getValueFromLineEdit(widget.cbType7, widget.leData7);
    uint64_t mask = getMaskValue(widget.cbType7);
    if (widget.btnWriteAll->isChecked()) {
        for (int i = 0; i < ChannelCount; ++i) {
            cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
            addr += 0x4000;
        }
    }
    else {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), mask);
    }
}

void MainForm::writeFreqReset() {
    cmdWrite(0x80000010, 1, 0, 1);
    cmdWrite(0x80000010, 0, 0, 1);
}

void MainForm::writeFrequency() {
    bool ok = false;
    
    uint32_t addr  = 0x3008;
    double freq = widget.leFrequency->text().toDouble(&ok);
    uint64_t data = *((uint64_t *)(&freq));
    
    for (int i = 0; i < ChannelCount; ++i) {
        cmdWrite(addr, (uint32_t)data, (uint32_t)(data >> 32), 0xFFFFFFFFFFFFFFFFULL);
        addr += 0x4000;
    }
    addr = 0x1004;
    for (int i = 0; i < ChannelCount; ++i) {
        cmdWrite(addr, 1, 0, 0xFF);
        addr += 0x4000;
    }
}

void MainForm::writeDiscriminator() {
    bool ok = false;
    
    uint32_t addr  = 0x8000006a;
    uint64_t data = widget.leDiscriminator->text().toULongLong(&ok, 16);
    
    for (int i = 0; i < ChannelCount; ++i) {
        cmdWrite(addr, (uint32_t)data, 0, 0xFFF);
        addr += 0x40;
    }
}

void MainForm::cmdDdsWrite(uint8_t dds_index, uint8_t addr, uint64_t data) {
    if (handle_device == 0) return;

    const uint32_t blen = 15;
    uint32_t len = blen - 6;
    uint32_t index = 0;
    uint32_t cs_index;
    uint8_t buffer[blen];

	buffer[index++] = 0xFF;
	buffer[index++] = 0xAA;
	buffer[index++] = 0xBB;
    buffer[index++] = len & 0xFF;
    buffer[index++] = (len >> 8) & 0xFF;
    cs_index = index++;

	buffer[index++] = 0x09;
	buffer[index++] = dds_index;
	buffer[index++] = addr;

	buffer[index++] = ((uint8_t *)(&data))[0];
	buffer[index++] = ((uint8_t *)(&data))[1];
	buffer[index++] = ((uint8_t *)(&data))[2];
	buffer[index++] = ((uint8_t *)(&data))[3];
	buffer[index++] = ((uint8_t *)(&data))[4];
	buffer[index++] = ((uint8_t *)(&data))[5];

    buffer[cs_index] = 0;
	for (uint32_t i = cs_index + 1; i < blen; ++i) {
		buffer[cs_index] += buffer[i];
	}
    writeBuffer(buffer, blen);

}

void MainForm::deviceUpdate() {
    if (handle_device == 0) return;

    unsigned long rxc  = 0;
	unsigned long txc  = 0;
	unsigned long stat = 0;
    
	uint32_t res = FT_GetStatus(handle_device, &rxc, &txc, &stat);
    if (res != FT_OK) {
        widget.teLog->appendPlainText(QString("Err: FT_GetStatus return %0").arg(res));
        deviceCheck();
        return;
    }
    
    if (rxc > 0) {
        if (widget.progressBar->value() == widget.progressBar->maximum()) widget.progressBar->setInvertedAppearance(true);
        if (widget.progressBar->value() == widget.progressBar->minimum()) widget.progressBar->setInvertedAppearance(false);
        if (widget.progressBar->invertedAppearance()) widget.progressBar->setValue(widget.progressBar->value() - 1);
        else widget.progressBar->setValue(widget.progressBar->value() + 1);
            
//        widget.teLog->appendPlainText(QString("Inf: Aviable %0 bytes").arg(rxc));
		char *buffer = new char[rxc];
        
        FT_Read(handle_device, buffer, rxc, &txc);
        bfr.append(buffer, txc);
        delete buffer;
        if (bfr.length() > MAX_BUFFER_LENGTH) {
            bfr.clear();
            puts("buffer ovewflow");
        }
        if ((bfr.count() > 0) && (bfr.indexOf("\xFF\xAA\xBB") >= 0)) 
            checkBuffer();
    }
}

void MainForm::checkCs(char *pointer, int len, char cs) {
    char value = 0;
    for (int i = 0; i < len; ++i)
        value += pointer[i];
    if (value != cs) {
        if (widget.btnShowInf->isChecked()) 
            widget.teLog->appendPlainText(QString("cs error: %0 != %1, len = %2")
                .arg(cs & 0xFF, 2, 16, QChar('0'))
                .arg(value & 0xFF, 2, 16, QChar('0'))
                .arg(len)
            );
        cs_error_counter++;
        widget.labelCsError->setText(QString::number(cs_error_counter));
    } 
}

void MainForm::checkBuffer() {
    const int hlen = 6;
    uint64_t data = 0;
    int frame_size = 0;
    char cs = 0;
    char *data_ptr;
    int pos = bfr.indexOf("\xFF\xAA\xBB");
    if (pos < 0) return;
    if (pos > 0) bfr.remove(0, pos);
    if (bfr.count() < hlen) return;
    frame_size = bfr.at(3) | (bfr.at(4) << 8);
    cs = bfr.at(5);
    
    if (frame_size + hlen > MAX_BUFFER_LENGTH) {
        widget.teLog->appendPlainText("bad header packet received");
        bfr.remove(0, hlen);
        return;
    }
    
    if (frame_size + hlen > bfr.count()) return;

    data_ptr = bfr.data() + hlen;
    
    if (bfr.count() > 8 + hlen)
        for (int i = 0; i < 8; ++i)
            ((char *)(&data))[i] = data_ptr[1 + i];
    
    checkCs(data_ptr, frame_size, cs);
    
    switch (data_ptr[0]) {
    case 0x00:
        setValueToLineEdit(widget.cbType0, widget.leData0, data);
        break;
    case 0x01:
        setValueToLineEdit(widget.cbType1, widget.leData1, data);
        break;
    case 0x02:
        setValueToLineEdit(widget.cbType2, widget.leData2, data);
        break;
    case 0x03:
        setValueToLineEdit(widget.cbType3, widget.leData3, data);
        break;
    case 0x04:
        setValueToLineEdit(widget.cbType4, widget.leData4, data);
        break;
    case 0x05:
        setValueToLineEdit(widget.cbType5, widget.leData5, data);
        break;
    case 0x06:
        setValueToLineEdit(widget.cbType6, widget.leData6, data);
        break;
    case 0x07:
        setValueToLineEdit(widget.cbType7, widget.leData7, data);
        break;
        
        
    case 0x10:
        plotForm.setData((float *)(&(data_ptr[1])), (frame_size - 1)/sizeof(float));
        break;
    case 0x11:
        plotSpectrum.setData((float *)(&(data_ptr[1])), (frame_size - 1)/sizeof(float));
        break;
    case 0x12:
        reciveIfcPrs(&data_ptr[1], frame_size - 1, 0);
        break;
    case 0x13:
        reciveIfcFreq(&data_ptr[1], frame_size - 1, 0);
        break;
    case 0x14:
        reciveIfcPrs(&data_ptr[1], frame_size - 1, 1);
        break;
    case 0x15:
        reciveIfcFreq(&data_ptr[1], frame_size - 1, 1);
        break;
    case 0x16:
        reciveInfSa(&data_ptr[1], frame_size - 1);
        break;
    case 0x17:
        reciveInfHa(&data_ptr[1], frame_size - 1);
        break;
    case 0x18:
        reciveTime(&data_ptr[1], frame_size - 1);
        break;
    default:
        if (widget.btnShowInf->isChecked())
            widget.teLog->appendPlainText(QString("unknown data received [type: %0 size: %1]").arg((int)(data_ptr[0])).arg((int)frame_size));
        if ((frame_size <= bfr.count()) && (frame_size > 0)) {
            bfr.remove(0, frame_size);
            if (widget.btnShowInf->isChecked()) widget.teLog->appendPlainText("removed");
        }
        else {
            if (frame_size != 0) {
                bfr.clear();
                if (widget.btnShowInf->isChecked()) widget.teLog->appendPlainText("buffer cleared");
            }
        }
        return;
    }
    
    if ((frame_size + hlen <= bfr.count()) && (frame_size > 0)) bfr.remove(0, frame_size + hlen);
    else {
        widget.teLog->appendPlainText("bad packet received");
        bfr.clear();
        return;
    };

    if ((bfr.count() > 0) && (bfr.indexOf("\xFF\xAA\xBB") >= 0)) {
        qApp->processEvents();
        checkBuffer();
    }
}

void MainForm::reciveTime(char *pointer, int len) {
    u32 *mseconds = (u32 *)pointer;
    widget.labelTime->setText(QString::number(*mseconds));
}


void MainForm::reciveMeasure(QVector<double *> *m, char *pointer) {
    struct fap_prs * fap = (struct fap_prs *)pointer;
    
    if (!m[fap->id].isEmpty()) {
        double * last = m[fap->id].last();
        if (last[0] == fap->measure_index) return;
    }
    
    if (!fap->locked) return;
    
    double * data = new double[6];
    data[0] = (double)fap->measure_index;
    data[1] = (double)fap->mseconds;
    data[2] = (double)fap->measure;
    data[3] = (double)fap->range;
    data[4] = (double)fap->phase;
    data[5] = (double)fap->freq_offset;
    
    m[fap->id].append(data);
}

void MainForm::reciveMeasureFreq(QVector<double *> *m, char *pointer) {
    struct fap_freq * fap = (struct fap_freq *)pointer;
    
    if (!m[fap->id].isEmpty()) {
        double * last = m[fap->id].last();
        if (last[0] == fap->measure_index) return;
    }

    if (!fap->locked) return;
    
    double * data = new double[6];
    data[0] = (double)fap->measure_index;
    data[1] = (double)fap->mseconds;
    data[2] = (double)fap->measure;
    data[3] = (double)fap->freq_nominal;
    data[4] = (double)fap->snr;
    data[5] = (double)(fap->measure_phase); // % (1ULL << 48);
    
    m[fap->id].append(data);
}

void MainForm::saveMeasure(QVector<double *> *m, QString fileName) {
    
    for (int i = 0; i < 16; ++i) {
        if (m[i].isEmpty()) continue;
        
        QFile f(QString(fileName).arg(i));
        f.open(QFile::WriteOnly);

        QVector<double *>::iterator value = m[i].begin();

        while (value != m[i].end()) {
            QString s = QString("%0 %1 %2 %3 %4 %5\n")
                        .arg((*value)[0], 0, 'g', 22)
                        .arg((*value)[1], 0, 'g', 22)
                        .arg((*value)[2], 0, 'g', 22)
                        .arg((*value)[3], 0, 'g', 22)
                        .arg((*value)[4], 0, 'g', 22)
                        .arg((*value)[5], 0, 'g', 22);
            f.write(s.toLocal8Bit().data());
            value++;
        }
        f.close();
    }
}

void MainForm::saveInf(QByteArray *inf, QString fileName) {
    
    for (int i = 0; i < 16; ++i) {
        if (inf[i].isEmpty()) continue;

        QFile f(QString(fileName).arg(i));
        f.open(QFile::WriteOnly);
        f.write(inf[i]);
        f.close();
    }
}

void MainForm::indicateIfcPrs(char *pointer, int len, int type) {
    struct fap_prs *fap = (struct fap_prs *)pointer;
    if (type) {
        widget.navWidget->setEnabledLed3((bool)fap->locked);
    }
    else {
        widget.navWidget->setValuePointer(360*(((float)fap->range)/100000.0));
        widget.navWidget->setEnabledLed0((bool)fap->locked);
        
        widget.pbRange->setValue((int)(((float)fap->range)/1000.0));
        QPalette pal = widget.labelPrsLocked->palette();
        if (fap->locked)
            pal.setColor(QPalette::Background, QColor("green"));
        else
            pal.setColor(QPalette::Background, QColor("yellow"));
        widget.labelPrsLocked->setPalette(pal);
    }
}

void MainForm::indicateIfcFreq(char *pointer, int len, int type) {
    struct fap_freq *fap = (struct fap_freq *)pointer;
    float snr = 0;
    if (type) {
        widget.navWidget->setEnabledLed4((bool)fap->locked);
        snr = fap->snr;
        if (snr > 0) snr = 10*log10f(snr);
        if (snr < 0) snr = 0;
        widget.navWidget->setValueAmpR(snr/30);
    }
    else {
        widget.navWidget->setEnabledLed1((bool)fap->locked);
        
        snr = fap->snr;
        if (snr > 0) snr = 10*log10f(snr);
        widget.pbSnr->setValue((int)snr);
        if (snr < 0) snr = 0;
        widget.navWidget->setValueAmpL(snr/30);
        QPalette pal = widget.labelFreqLocked->palette();
        if (fap->locked)
            pal.setColor(QPalette::Background, QColor("green"));
        else
            pal.setColor(QPalette::Background, QColor("yellow"));
        widget.labelFreqLocked->setPalette(pal);
    }
}

void MainForm::reciveIfcPrs(char *pointer, int len, int type) {
    struct fap_prs *fap = (struct fap_prs *)pointer;
    QPlainTextEdit *te = type ? widget.teIfcPrsHa : widget.teIfcPrs;
    
    int channel = widget.sbChannelIndex->value();
    
    if (type) reciveMeasure(measure_ha, pointer);
    else reciveMeasure(measure_sa, pointer);
    
    if (fap->id != channel) return;
    
    te->clear();
    te->appendPlainText(QString("id = %0").arg(fap->id));
    te->appendPlainText(QString("enabled = %0").arg(fap->enabled));
    te->appendPlainText(QString("symbol_locked = %0").arg(fap->symbol_locked));
    te->appendPlainText(QString("locked = %0").arg(fap->locked));
    te->appendPlainText(QString("reset = %0").arg(fap->reset));
    te->appendPlainText(QString("noise_ready = %0").arg(fap->noise_ready));
    te->appendPlainText(QString("delay = %0").arg(fap->delay));
    te->appendPlainText(QString("freq_offset = %0").arg(fap->freq_offset));
    te->appendPlainText(QString("freq_nominal = %0").arg(fap->freq_nominal));
    te->appendPlainText(QString("freq_step = %0").arg(fap->freq_step));
    te->appendPlainText(QString("range = %0").arg(fap->range));
    te->appendPlainText(QString("measure_index = %0").arg(fap->measure_index));
    te->appendPlainText(QString("k0 = %0").arg(fap->k0));
    te->appendPlainText(QString("k1 = %0").arg(fap->k1));
    te->appendPlainText(QString("z0 = %0").arg(fap->z0));
    te->appendPlainText(QString("z1 = %0").arg(fap->z1));
    te->appendPlainText(QString("s = %0").arg(fap->s));
    te->appendPlainText(QString("c = %0").arg(fap->c));
    te->appendPlainText(QString("power = %0").arg(fap->power));
    te->appendPlainText(QString("power_locked = %0").arg(fap->power_locked));
    te->appendPlainText(QString("noise = %0").arg(fap->noise));
    te->appendPlainText(QString("noise_tmp = %0").arg(fap->noise_tmp));
    te->appendPlainText(QString("level = %0").arg(fap->level));
    te->appendPlainText(QString("phase = %0").arg(fap->phase));
    te->appendPlainText(QString("measure = %0").arg(fap->measure));
    te->appendPlainText(QString("mseconds = %0").arg(fap->mseconds));
    
    indicateIfcPrs(pointer, len, type);
}

void MainForm::reciveIfcFreq(char *pointer, int len, int type) {
    struct fap_freq *fap = (struct fap_freq *)pointer;
    QPlainTextEdit *te = type ? widget.teIfcFreqHa : widget.teIfcFreq;

    int channel = widget.sbChannelIndex->value();
    
    if (type) reciveMeasureFreq(measure_fha, pointer);
    else reciveMeasureFreq(measure_fsa, pointer);

    if (fap->id != channel) return;
    
    te->clear();
    te->appendPlainText(QString("id = %0").arg(fap->id));
    te->appendPlainText(QString("enabled = %0").arg(fap->enabled));
    te->appendPlainText(QString("current_locked = %0").arg(fap->current_locked));
    te->appendPlainText(QString("locked = %0").arg(fap->locked));
    te->appendPlainText(QString("dropped = %0").arg(fap->dropped));
    te->appendPlainText(QString("reset = %0").arg(fap->reset));
    te->appendPlainText(QString("freq_offset = %0").arg(fap->freq_offset));
    te->appendPlainText(QString("freq_nominal = %0").arg(fap->freq_nominal));
    te->appendPlainText(QString("drop_timeout = %0").arg(fap->drop_timeout));
    te->appendPlainText(QString("lock_timeout = %0").arg(fap->lock_timeout));
    te->appendPlainText(QString("k0 = %0").arg(fap->k0));
    te->appendPlainText(QString("k1 = %0").arg(fap->k1));
    te->appendPlainText(QString("z0 = %0").arg(fap->z0));
    te->appendPlainText(QString("z1 = %0").arg(fap->z1));
    te->appendPlainText(QString("s = %0").arg(fap->s));
    te->appendPlainText(QString("c = %0").arg(fap->c));
    te->appendPlainText(QString("si = %0").arg(fap->si));
    te->appendPlainText(QString("ci = %0").arg(fap->ci));
    te->appendPlainText(QString("power = %0").arg(fap->power));
    te->appendPlainText(QString("level = %0").arg(fap->level));
    te->appendPlainText(QString("snr = %0").arg(10*log10f(fap->snr)));
    te->appendPlainText(QString("snr = %0").arg(fap->snr));
    te->appendPlainText(QString("measure_index = %0").arg(fap->measure_index));
    te->appendPlainText(QString("measure = %0").arg(fap->measure));
    te->appendPlainText(QString("mseconds = %0").arg(fap->mseconds));
    te->appendPlainText(QString("measure_phase = %0").arg(fap->measure_phase));
    te->appendPlainText(QString("phase_nominal = %0").arg(fap->phase_nominal));

    indicateIfcFreq(pointer, len, type);
}

void MainForm::reciveInfSa(char *pointer, int len) {
    struct chnl_inf *inf = (struct chnl_inf *)pointer;
    int channel = widget.sbChannelIndex->value();

    inf_sa[inf->id].append((char *)(&(inf->inf)), sizeof(u64));
    
    if (inf->id != channel) return;
    
    if (((inf->inf & 0xFF) != 0x25) && ((inf->inf & 0xFF) != 0x8D))
        widget.labelInfSa->setText(QString("Index Sa: %0").arg((inf->inf >> 45) & 0xF, 1, 16, QChar('0')));
}

void MainForm::reciveInfHa(char *pointer, int len) {
    struct chnl_inf *inf = (struct chnl_inf *)pointer;
    int channel = widget.sbChannelIndex->value();
    inf_ha[inf->id].append((char *)(&(inf->inf)), sizeof(u64));
    
    if (inf->id != channel) return;
    
    widget.labelInfHa->setText(QString("Index Ha: %0").arg((inf->inf >> 46) & 0xF, 1, 16, QChar('0')));
}

void MainForm::showPlot() {
    plotForm.show();
    plotSpectrum.show();
}

void MainForm::openIni() {
    QString name = QFileDialog::getOpenFileName(this, "Open Profile", "", "*.ini");
    QFileInfo fi(name);
    name = fi.baseName();
    if (name.isEmpty()) return;
    loadSettings(name);
}

void MainForm::selectDbgPlotIndex(int value) {
    cmdWrite(1, (uint32_t)value, 0);
}

void MainForm::selectMap0(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr0->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap1(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr1->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap2(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr2->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap3(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr3->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap4(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr4->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap5(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr5->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap6(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr6->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::selectMap7(QString value) {
    uint32_t addr = addr_map.key(value);
    widget.leAddr7->setText(QString("%0").arg(addr, 8, 16, QChar('0')));
}

void MainForm::writeGain(int value) {
    if (widget.btnGain->isChecked()) value |= 0x80;
    else value &= 0x7F;
    cmdWrite(0x80000004, value, 0, 0xFF);
    cmdWrite(20, value, 0, 0xFF);
    widget.lcdGain->display(value);
}

void MainForm::selectChannel(int value) {
    cmdWrite(4, value);
}

void MainForm::setValueToLineEdit(QComboBox *cb, QLineEdit *le, uint64_t value) {
    int type = cb->currentIndex();
    switch (type) {
        case 1: 
            if (le->validator() != rval) le->setValidator(rval);
            le->setText(QString("%0").arg((bool)value, 16, 16, QChar('0')));
            break;
        case 4: 
            if (le->validator() != rval) le->setValidator(rval);
            le->setText(QString("%0").arg((uint32_t)value, 16, 16, QChar('0')));
            break;
        case 8:
            if (le->validator() != dval) le->setValidator(dval);
            le->setText(QString("%0").arg(*((float *)&value), 0, 'g', 12));
            break;
        case 9:
            if (le->validator() != dval) le->setValidator(dval);
            le->setText(QString("%0").arg(*((double *)&value), 0, 'g', 12));
            break;
        default:
            if (le->validator() != rval) le->setValidator(rval);
            le->setText(QString("%0").arg(value, 16, 16, QChar('0')));
            break;
    }
}

uint64_t MainForm::getValueFromLineEdit(QComboBox *cb, QLineEdit *le) {
    bool ok;
    uint64_t data = 0;
    float *fdata = (float *)&data;
    double *ddata = (double *)&data;
    int type = cb->currentIndex();
    switch (type) {
        case 1:
            data = (uint64_t)le->text().toUInt(&ok, 16);
            break;
        case 4:
            data = (uint64_t)le->text().toUInt(&ok, 16);
            break;
        case 8:
            *fdata = le->text().toFloat(&ok);
            break;
        case 9:
            *ddata = le->text().toDouble(&ok);
            break;
        default:
            data = le->text().toULongLong(&ok, 16);
            break;
    }
    return data;
}

uint64_t MainForm::getMaskValue(QComboBox *cb) {
    uint64_t mask = 0;
    int type = cb->currentIndex();
    switch (type) {
        case 1:
            mask = 0x1;
            break;
        case 2:
        case 3:
            mask = 0xFF;
            break;
        case 4:
        case 5:
        case 8:
            mask = 0xFFFFFFFF;
            break;
        default:
            mask = 0xFFFFFFFFFFFFFFFF;
            break;
    }
    return mask;
}

void MainForm::saveMeasure() {
    saveMeasure(measure_sa, qApp->applicationDirPath() + "/ch%0/rsa.txt");
    saveMeasure(measure_ha, qApp->applicationDirPath() + "/ch%0/rha.txt");
    saveMeasure(measure_fsa, qApp->applicationDirPath() + "/ch%0/fsa.txt");
    saveMeasure(measure_fha, qApp->applicationDirPath() + "/ch%0/fha.txt");
    saveInf(inf_sa, qApp->applicationDirPath() + "/ch%0/inf_sa.bin");
    saveInf(inf_ha, qApp->applicationDirPath() + "/ch%0/inf_ha.bin");
}

void MainForm::clearMeasure() {
    for (int i = 0; i < 16; ++i) {
        measure_sa[i].clear();
        measure_ha[i].clear();
        measure_fsa[i].clear();
        measure_fha[i].clear();
        inf_sa[i].clear();
        inf_ha[i].clear();
    }
}
