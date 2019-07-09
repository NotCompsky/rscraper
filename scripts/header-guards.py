#!/usr/bin/env python3


import os
import re


def strip_path(fp):
	s = ""
	is_in_project = False
	ls = os.path.normpath(fp).split(os.sep)
	for node in ls:
		if not is_in_project:
			if node == args.project:
				is_in_project = True
			else:
				continue
		if node in args.ignore:
			continue
		s += "_" + re.sub("[^a-zA-Z0-9_]", "_", node)
	if s == "":
		print(f"ERROR:\tCannot find package name '{args.project}' in {fp}")
		exit(1)
	return s[1:]


def guard_from_fp(fp):
	return f"#ifndef {strip_path(fp).upper()}"


def overwrite_header(fp, ls):
	open(fp, "w").write('\n'.join(ls))


def process(fp):
	'''
	Make some sensible assumptions to simplify the problem:
	- #ifndef declarations match the regex ^#ifndef - i.e. start at the line start and have no spaces between "#" and "ifndef"
	- If there are existing #ifndef declarations, the first declaration is the header guard
	'''
	lines = open(fp).read().split('\n')
	for i, line in enumerate(lines):
		if line.startswith("#ifndef ") and lines[i+1].startswith("#define "):
			new_line = guard_from_fp(fp)
			if line == new_line:
				print(f"GOOD:\t{fp}")
				return
			lines[i]   = new_line
			lines[i+1] = new_line.replace("#ifndef", "#define")
			overwrite_header(fp, lines)
			print(f"FIXED:\t{fp}")
			return
	print(f"ERROR:\tNo header found in {fp}")


if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser()
	parser.add_argument("-p", "--project", help="Project name (must have same capitalisation as root directory of project")
	parser.add_argument("-i", "--ignore", action="append", help="Directories to ignore (such as 'src')")
	parser.add_argument("fps", nargs="+")
	args = parser.parse_args()

	for fp in args.fps:
		process(fp)
