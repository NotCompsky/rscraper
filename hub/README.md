![Displaying tagged subreddits](https://user-images.githubusercontent.com/30552567/60246564-d8c88300-98b6-11e9-85c9-5d88d7a4d89e.png)

![Chart](https://user-images.githubusercontent.com/30552567/60340500-aea0bf00-99a3-11e9-8900-4f5fce4df5e9.png)

# Building

Download the source code of [mainslice.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-h.html), [mainslice.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-mainslice-cpp.html), [donutbreakdownchart.h](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-h.html) and [donutbreakdownchart.cpp](https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-donutbreakdownchart-cpp.html) into the 3rdparty directory:

    mkdir src/3rdparty
    ./qtdl.sh src/3rdparty qtcharts donutbreakdown mainslice.h
    ./qtdl.sh src/3rdparty qtcharts donutbreakdown mainslice.cpp
    ./qtdl.sh src/3rdparty qtcharts donutbreakdown donutbreakdownchart.h
    ./qtdl.sh src/3rdparty qtcharts donutbreakdown donutbreakdownchart.cpp

In `donutbreakdownchart.cpp`, delete the lines under `DonutBreakdownChart::addBreakdownSeries` that act on the `slices` (starting from line 38), and in `donutbreakdownchart.h` move `QPieSeries *m_mainSeries;` to `public`:

    awk '/const auto slices/{a=1};{if(b||!a)print};/\}/{if(a)b=1}' src/3rdparty/donutbreakdownchart.cpp > tmp  &&  mv tmp src/3rdparty/donutbreakdownchart.cpp
    sed -i -r '/private:/,/QPieSeries \*m_mainSeries;/c\public:\n    QPieSeries *m_mainSeries;' src/3rdparty/donutbreakdownchart.h

In `mainslice.cpp`, delete the `#include "moc_mainslice.cpp"` line:

    sed -i 's/#include "moc_mainslice.cpp"//g' src/3rdparty/mainslice.cpp

# Usage

See [guide](../guides/hub.md) and [manual](../man/rscraper-hub).
