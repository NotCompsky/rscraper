#!/usr/bin/env python
# -*- coding: utf-8 -*-

import flask


app = flask.Flask(__name__)


@app.route('/favicon.ico')
def you_dont_need_this():
    # This is just to allow you to navigate to http://localhost:PORT/ directly from a browser without causing a second, unnecessary query
    return ""

@app.route('/<path:path>')
def get_rtags(path:str):
    rtagger.csv2cls(path.encode(), args.tagcondition.encode(), args.matchcondition.encode())
    res:str = ctypes.c_char_p.in_dll(rtagger, "DST").value.decode()
    return res


if __name__ == "__main__":
    import argparse
    import ctypes

    parser = argparse.ArgumentParser()
    parser.add_argument('-p','--port', type=int, default=8080)
    parser.add_argument('-t','--tagcondition', type=string, default="", help="SQL condition that t (tag) must fulfil. If non-empty, must begin with 'AND'. E.g. 'AND t.name=\"Unknown\"'")
    parser.add_argument('-m','--matchcondition', type=string, default="", help="SQL condition that m (reason_matched) must fulfil. If non-empty, must begin with 'AND'. E.g. 'AND m.id>3 AND m.id<10'")
    args = parser.parse_args()

    rtagger = ctypes.cdll.LoadLibrary("librscraper-tagger.so")
    rtagger.init()

    app.run(host='localhost', port=args.port)

    rtagger.exit_mysql()
