/* 
 * File:   PlotForm.h
 * Author: Ivan
 *
 * Created on 27 Октябрь 2011 г., 17:14
 */

#ifndef _PLOTFORM_H
#define	_PLOTFORM_H

#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

#include "ui_PlotForm.h"

class PlotCurve: public QwtPlotCurve {
public:
    PlotCurve(const QString &title = "ololo"):
        QwtPlotCurve(title)
    {
        setRenderHint(QwtPlotItem::RenderAntialiased);
        setStyle(QwtPlotCurve::Lines);
    }

    void setColor(const QColor &color)
    {
        QColor c = color;
//        c.setAlpha(32);
        setPen(c);
//        setBrush(c);
    }
};

class PlotForm : public QWidget {
    Q_OBJECT
public:
    PlotForm();
    virtual ~PlotForm();
    void setData(float *data, int len);
    void setDataLen(int value);
    inline int dataLen() { return m_data_len; }
    void setAxisXType(int type);
    static const int AxisXDefault   = 0;
    static const int AxisXFrequency = 1;
    static const int AxisXTime      = 2;
private:
    void noise();
    Ui::PlotForm widget;
    PlotCurve curve;
    QwtPlotMarker mr_center, mr_left, mr_right;
    double *m_data[2];
    int m_data_len;
    int x_axis_type;
    QDoubleValidator *dvalidator;
private slots:
    void setScaleEngineY(bool checked);
    void setAutoScale(bool checked);
    void rescale(double value);
};

#endif	/* _PLOTFORM_H */
