#!/usr/bin/env python3


import ctypes


# One query
def query(userids:bytes):
    rtagged.csv2cls(userids)
    s:bytes = ctypes.c_char_p.in_dll(rtagged, "DST")
    print(s.value)
    rtagged.free_dst() # Must be called after every call to csv2cls


if __name__ == "__main__":
    rtagged = ctypes.cdll.LoadLibrary("build/rtagged.so")
    rtagged.init_mysql(b"unix:///var/run/mysqld/mysqld.sock", b"rscraper++", b"PASSWRD")
    
    query(b"id-t2_6l4z3,id-t2_32duad4")
