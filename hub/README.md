# Building

Download the source code of [mainslice.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-h.html), [mainslice.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-cpp.html), [donutbreakdownchart.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-h.html) and [donutbreakdownchart.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-cpp.html) into the 3rdparty directory. In `donutbreakdownchart.cpp`, delete the lines under `DonutBreakdownChart::addBreakdownSeries` that act on the `slices` (starting from line 38), and in `donutbreakdownchart.h` move `QPieSeries *m_mainSeries;` to `public`.

# Usage

See [guide](../guide/hub.md) and [manual](../man/rscraper-hub).
