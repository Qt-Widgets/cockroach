/* 
 * File:   MainForm.h
 * Author: Ivan
 *
 * Created on 24 Октябрь 2011 г., 14:57
 */

#ifndef _MAINFORM_H
#define	_MAINFORM_H

#include <windows.h>
#include <QLibrary>
#include <QTimer>
#include <QSettings>
#include <QStandardItemModel>
#include <QValidator>
#include <QRegExp>
#include <QRegExpValidator>
#include <QByteArray>
#include <QMessageBox>

#include "PlotForm.h"
#include "ui_MainForm.h"

class MainForm : public QWidget {
    Q_OBJECT
public:
    MainForm();
    virtual ~MainForm();
    
private:
    typedef struct {
        float data[5];
    } Measure;
    static const int ChannelCount = 8;
    Ui::MainForm widget;
    QLibrary lib;
    void deviceOpen();
    void deviceClose();
    void loadLibrary();
    void writeBuffer(uint8_t *buffer, unsigned long len);
    void cmdRead(uint32_t addr, char sender = 0);
    void cmdWrite(uint32_t addr, uint32_t ldata, uint32_t hdata = 0, uint64_t mask = 0xFFFFFFFFFFFFFFFF);
    void cmdSet(uint32_t addr, uint8_t bit, bool value);
    void cmdDdsWrite(uint8_t index, uint8_t addr, uint64_t data);
    void checkBuffer();
    void checkCs(char *pointer, int len, char cs);
    void indicateIfcPrs(char *pointer, int len, int type);
    void indicateIfcFreq(char *pointer, int len, int type);
    void reciveIfcPrs(char *pointer, int len, int type);
    void reciveIfcFreq(char *pointer, int len, int type);
    void reciveInfSa(char *pointer, int len);
    void reciveInfHa(char *pointer, int len);
    void reciveTime(char *pointer, int len);
    void setValueToLineEdit(QComboBox *cb, QLineEdit *le, uint64_t value);
    void reciveMeasure(QVector<double *> *m, char *pointer);
    void reciveMeasureFreq(QVector<double *> *m, char *pointer);
    void saveMeasure(QVector<double *> *m, QString fileName);
    void saveInf(QByteArray *inf, QString fileName);
    uint64_t getValueFromLineEdit(QComboBox *cb, QLineEdit *le);
    uint64_t getMaskValue(QComboBox *cb);
    PlotForm plotForm, plotSpectrum;
    void *handle_device;
    QTimer timer;
    uint64_t dds_data[3][32];
    uint64_t dds2_step_freq;
    uint64_t dds2_start_freq;
    uint64_t dds2_end_freq;
    uint64_t dds2_current_freq;
    int cs_error_counter;
    QByteArray bfr;
    QRegExpValidator *rval;
    QDoubleValidator *dval;
    QIntValidator *ival;
    QMap<uint32_t, QString> addr_map;
    QVector<double *> measure_sa[16];
    QVector<double *> measure_ha[16];
    QVector<double *> measure_fsa[16];
    QVector<double *> measure_fha[16];
    QByteArray inf_sa[16];
    QByteArray inf_ha[16];
    
private slots:
    void cmdReadA();
    void deviceCheck();
    void loadSettings(QString name = "");
    void saveSettings();
    void showPlot();
    void openIni();
    void deviceRead0();
    void deviceRead1();
    void deviceRead2();
    void deviceRead3();
    void deviceRead4();
    void deviceRead5();
    void deviceRead6();
    void deviceRead7();
    void deviceWrite0();
    void deviceWrite1();
    void deviceWrite2();
    void deviceWrite3();
    void deviceWrite4();
    void deviceWrite5();
    void deviceWrite6();
    void deviceWrite7();
    void deviceUpdate();
    void selectMap0(QString value);
    void selectMap1(QString value);
    void selectMap2(QString value);
    void selectMap3(QString value);
    void selectMap4(QString value);
    void selectMap5(QString value);
    void selectMap6(QString value);
    void selectMap7(QString value);
    void selectDbgPlotIndex(int value);
    void selectChannel(int value);
    void writeGain(int value);
    void writeFreqReset();
    void writeFrequency();
    void writeDiscriminator();
    void readScCode();
    void saveMeasure();
    void clearMeasure();
};

#endif	/* _MAINFORM_H */
