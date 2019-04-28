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
    rtagger.csv2cls(path.encode())
    res:str = ctypes.c_char_p.in_dll(rtagger, "DST").value.decode()
    rtagger.free_dst() # Must be called after every call to csv2cls
    return res


if __name__ == '__main__':
    import argparse
    import ctypes
    
    parser = argparse.ArgumentParser()
    parser.add_argument('-p','--port', required=True, type=int, default=8080)
    parser.add_argument('-a','--auth', required=True, help="Path to MySQL config file containing url, username, password on separate lines delineated by \\n (not \\r\\n)")
    args = parser.parse_args()
    
    rtagger = ctypes.cdll.LoadLibrary("build/rtagger.so")
    rtagger.init_mysql_from_file(args.auth.encode())
    
    app.run(host='localhost', port=args.port)
