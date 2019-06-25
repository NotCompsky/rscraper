#!/usr/bin/env python3

'''
A utility (that will likely not be maintained) for creating the 'comment body regexp', as an alternative to the regex editor in rscraper-hub (which will be maintained).

It has been left here as it may be more useful to customise for verifying your regex - for instance, checking matches are what you expect.
'''

s = (
"(?:^|[^a-zA-Z0-9_])(?:"
    "(?P<RScraper>"
        "[Rr]scraper"
    ")|"
    "(?P<Mr Compsky>"
        "(?:[Nn]ot)?[Cc]ompsky|"
    ")"
")s?[^a-zA-Z0-9]|"

"/r/(?P<Subreddit Link>" # Group 8: Subreddit
    "(?:[Ll]earn)?(?:"
        "cpp|"
        "[Pp]rogramming|"
        "[Pp]ython|"
        "[Jj]ava(?:[Ss]cript)|"
        "[Hh]askell|"
        "SQL|sql|"
        "[Rr]ust"
    ")|"
    "netsec|"
    "[Rr]everse[Ee]ngineering"
")"
)


group_names = []


def replacer_fnct(match):
    # Python regex has greater restrictions on group names, and does not allow multiple groups to share the same name
    group_name = re.sub("[^A-Za-z0-9_]", "_", match.group(1))
    while group_name in group_names:
        group_name += '_'
    group_names.append(group_name)
    return "(?P<" + group_name + ">"


if __name__ == "__main__":
    import argparse
    import os
    import re
    
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--search")
    parser.add_argument("-e", "--export", default=False, action="store_true")
    args = parser.parse_args()
    
    if args.search:
        t = re.sub("\(\?P<([^>]*)>", replacer_fnct, s)
        
        match = re.search(t, args.search)
        if (match is not None):
            for i, x in enumerate(match.groups()):
                if x is not None:
                    print(i, '\t', x)
            for x,y in match.groupdict().items():
                if y is not None:
                    print('\t', x)
    
    if args.export:
        open(os.environ["RSCRAPER_REGEX_FILE"], "w").write(s)
