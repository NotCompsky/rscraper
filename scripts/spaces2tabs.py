#!/usr/bin/env python3


import os


SPACES_STR = None


def overwrite_header(fp, ls):
	if args.simulate:
		return
	open(fp, "w").write('\n'.join(ls))


def process(fp):
	'''
	Make some sensible assumptions to simplify the problem:
	- #ifndef declarations match the regex ^#ifndef - i.e. start at the line start and have no spaces between "#" and "ifndef"
	- If there are existing #ifndef declarations, the first declaration is the header guard
	'''
	lines = open(fp).read().split('\n')
	is_modified = False
	for i, line in enumerate(lines):
		new_line = ""
		while line.startswith(SPACES_STR):
			new_line += '\t'
			line = line[4:]
			is_modified = True
		new_line += line
		lines[i] = new_line
	
	if not is_modified:
		print(f"GOOD:   {fp}")
	else:
		print(f"FIXING: {fp}")
		overwrite_header(fp, lines)


if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser()
	parser.add_argument("-s", "--simulate", default=False, action="store_true", help="Do not modify files")
	parser.add_argument("-n", "--spaces-per-tab", default=4, type=int)
	parser.add_argument("fps", nargs="+")
	args = parser.parse_args()
	
	SPACES_STR = " " * args.spaces_per_tab

	for fp in args.fps:
		process(fp)
