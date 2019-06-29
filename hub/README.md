![Displaying tagged subreddits](https://user-images.githubusercontent.com/30552567/60246564-d8c88300-98b6-11e9-85c9-5d88d7a4d89e.png)

![Chart](https://user-images.githubusercontent.com/30552567/60340500-aea0bf00-99a3-11e9-8900-4f5fce4df5e9.png)

# Building

Download the source code of [mainslice.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-h.html), [mainslice.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-cpp.html), [donutbreakdownchart.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-h.html) and [donutbreakdownchart.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-cpp.html) into the 3rdparty directory. In `donutbreakdownchart.cpp`, delete the lines under `DonutBreakdownChart::addBreakdownSeries` that act on the `slices` (starting from line 38), and in `donutbreakdownchart.h` move `QPieSeries *m_mainSeries;` to `public`.

# Usage

See [guide](../guides/hub.md) and [manual](../man/rscraper-hub).
