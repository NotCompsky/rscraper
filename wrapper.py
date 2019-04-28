#!/usr/bin/env python3


import ctypes


# One query
def query(userids:bytes):
    n = rtagged.n_required_bytes(userids)
    s = ctypes.create_string_buffer(n + 1)
    rtagged.csv2cls(userids, s)
    print(s.value)


def main():
    rtagged = ctypes.cdll.LoadLibrary("build/rtagged.so")
    rtagged.init_mysql(b"unix:///var/run/mysqld/mysqld.sock", b"rscraper++", b"PASSWRD")
    userids:str = b"id-t2_6l4z3,id-t2_32duad4"
