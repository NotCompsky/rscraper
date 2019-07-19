#!/usr/bin/env python3


import ctypes


# One query
def query(userids:bytes):
	rtagger.csv2cls(userids, b"", b"")
	s:bytes = ctypes.c_char_p.in_dll(rtagger, "DST")
	print(s.value)


if __name__ == "__main__":
	rtagger = ctypes.cdll.LoadLibrary("librscraper-tagger.so")
	rtagger.init()

	query(b"id-t2_6l4z3,id-t2_32duad4")

	rtagger.exit_mysql()
