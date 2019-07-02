Run `rscraper-hub` to open the `rscraper-hub` program.

# Main Tab

![rscraper-hub opened from terminal](https://user-images.githubusercontent.com/30552567/60246088-e2052000-98b5-11e9-82be-e259d6f30f9e.png)

You can generate a doughnut chart of the number of comments per category, broken down by each tag.

You first `bake` the chart (which currently freezes the application for ~1 minute), then `show` it.

For instance (with only one category):

![region cat doughnut](https://user-images.githubusercontent.com/30552567/60340500-aea0bf00-99a3-11e9-8900-4f5fce4df5e9.png)

# Scraper Tab

![l](https://user-images.githubusercontent.com/30552567/60397047-94501800-9b40-11e9-9023-31b79795a20d.png)

You can edit the comment body matching regex here. It includes a preprocessor that converts the more human-readable regex here into the regex that is used by the scraper. As it uses `boost::regex`'s Perl regex internally, all features of that are supported here. Additionally, named groups are supported too. If you see a `reason` field on other tabs, it is talking about the named capture groups (that are pre-processed out of the regex and into the database).

You can `test` the regex for correctness.

![Regex Editor](https://user-images.githubusercontent.com/30552567/60394879-c18dcd80-9b22-11e9-97c8-c997013d6d21.png)

# Comments Overview Tab

You can filter and cycle through the scraped comments. Comments which were captured by non-capturing capture groups (i.e. reasons beginning with `!`) will have all the metadata, but empty comment contents.

You can also compare the comment's contents to the current regex, with `Details`. It will show which substrings matched which regex capture groups.

![cmnts tab](https://user-images.githubusercontent.com/30552567/60394734-db2e1580-9b20-11e9-8107-4c619c871adf.png)

# Category Tabs

![Example](https://user-images.githubusercontent.com/30552567/60396903-0e7f9d00-9b3f-11e9-845e-28f36e39e2d9.png)

* NOTE: the package is often being updated. Some screenshots below may be slightly outdated and omit the new buttons, though the general idea is the same.

You can right-click on any tag's label to see the associated subreddits. For instance, here I clicked on `Dutch`.

![subreddit2tag summary for Dutch](https://user-images.githubusercontent.com/30552567/60246564-d8c88300-98b6-11e9-85c9-5d88d7a4d89e.png)

You can rename tags by left-clicking on its label. For instance, here I clicked on `programming`.

![Rename programming](https://user-images.githubusercontent.com/30552567/60246614-f564bb00-98b6-11e9-8b55-ced5d50ff741.png)

You can colour tags by left-clicking on the space between the label and the `+Subreddits` button. Once you have selected a colour, the space will appear that colour too.

![Screenshot from 2019-06-26 22-42-58](https://user-images.githubusercontent.com/30552567/60246656-0a414e80-98b7-11e9-98fb-ceb0b829cb61.png)

You can add tags with the `+Tag` button.

![Screenshot from 2019-06-26 22-43-17](https://user-images.githubusercontent.com/30552567/60246685-1e854b80-98b7-11e9-85cb-4d10f203bd93.png)

The `stats` button next to each tag generates a pie chart of the number of comments per subreddit for that tag.

![Subreddits tagged US](https://i.imgur.com/zVB4f5M.png)
