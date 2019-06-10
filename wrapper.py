#!/usr/bin/env python3


import ctypes


# One query
def query(userids:bytes):
    rtagged.csv2cls(userids)
    s:bytes = ctypes.c_char_p.in_dll(rtagged, "DST")
    print(s.value)


if __name__ == "__main__":
    rtagged = ctypes.cdll.LoadLibrary("librscraper-tagger.so")
    rtagged.init()
    
    query(b"id-t2_6l4z3,id-t2_32duad4")
