# Print table of large (according to comments we've recorded) subreddits to their associated tags and tag categories

SELECT A.user, A.tag, c.name as 'category'
FROM category c
RIGHT JOIN (
    SELECT B.user, B.tag, t2c.category_id
    FROM tag2category t2c
    RIGHT JOIN (
        SELECT r.name as 'user', S2T.name as 'tag', S2T.tag_id
        FROM subreddit r
        LEFT JOIN (
            SELECT s2t.subreddit_id, T.name, s2t.tag_id
            FROM subreddit2tag s2t
            JOIN (
                SELECT t.id, t.name
                FROM tag t
            ) T on T.id = s2t.tag_id
        ) S2T on S2T.subreddit_id = r.id
        WHERE r.id IN (SELECT subreddit_id FROM user2subreddit_cmnt_count GROUP BY subreddit_id HAVING COUNT(*) > 1000)
    ) B ON t2c.tag_id = B.tag_id
) A ON c.id = A.category_id
;

# Print table of large (according to comments we've recorded) subreddits to their associated tags

SELECT r.name, S2T.name
FROM subreddit r
LEFT JOIN (
    SELECT s2t.subreddit_id, T.name
    FROM subreddit2tag s2t
    JOIN (
        SELECT t.id, t.name
        FROM tag t
    ) T on T.id = s2t.tag_id
) S2T on S2T.subreddit_id = r.id
WHERE r.id IN (SELECT subreddit_id FROM user2subreddit_cmnt_count GROUP BY subreddit_id HAVING COUNT(*) > 1000)
;




# Find subreddits that have multiple tags of the same category

SELECT r.name, r.id as 'sub_id', B.category_id, B.str
FROM subreddit r
JOIN (
    SELECT t2c.category_id, A.subreddit_id, COUNT(t2c.category_id) as c, GROUP_CONCAT(A.tag_name, ":", A.tag_id) as str
    FROM tag2category t2c
    JOIN (
        SELECT name as 'tag_name', tag_id, subreddit_id
        FROM subreddit2tag
        JOIN (
            SELECT name, id
            FROM tag
        ) C ON C.id = tag_id
    ) A ON t2c.tag_id = A.tag_id
    GROUP BY t2c.category_id, A.subreddit_id
    HAVING c > 1
) B ON B.subreddit_id = r.id
;

# Disassociate tag from subreddits, if the subreddit has other tags from the same category

DELETE FROM subreddit2tag
WHERE tag_id IN (SELECT id FROM tag WHERE name = 'TAG')
  AND subreddit_id IN (
    SELECT A.subreddit_id # ,t2c.category_id, A.tag_name, A.tag_id
    FROM tag2category t2c
    JOIN (
        SELECT name as 'tag_name', tag_id, subreddit_id
        FROM subreddit2tag
        JOIN (
            SELECT name, id
            FROM tag
        ) C ON C.id = tag_id
    ) A ON t2c.tag_id = A.tag_id
    WHERE t2c.category_id IN (
        SELECT category_id
        FROM tag2category
        WHERE tag_id IN (SELECT id FROM tag WHERE name = 'TAG')
    )
    AND t2c.tag_id NOT IN (SELECT id FROM tag WHERE name = 'TAG')
    GROUP BY A.subreddit_id
)
;







# Tag subreddit SUBREDDIT_NAME with tag TAG_NAME (assuming TAG_NAME exists in tag table)

INSERT INTO subreddit2tag (tag_id, subreddit_id)
SELECT t.id, s.id
FROM tag t JOIN subreddit s
WHERE (t.name = 'TAG_NAME') AND (s.name = 'SUBREDDIT_NAME')
;

# Remove tag TAG_NAME from subreddit SUBREDDIT_NAME

DELETE FROM subreddit2tag s2t
WHERE s2t.tag_id = (SELECT id FROM tag WHERE name = "TAG_NAME")
  AND s2t.subreddit_id = (SELECT id FROM subreddit WHERE name = "SUBREDDIT_NAME")
;



# Print table of subreddit, username, comment

SELECT r.name AS 'subreddit', R.name AS 'user', R.content
FROM subreddit r
JOIN (
    SELECT s.subreddit_id, P.name, P.content
    FROM submission s
    JOIN (
        SELECT u.name, T.content, T.submission_id
        FROM user u
        JOIN (
            SELECT c.author_id, c.content, c.submission_id
            FROM comment c
        ) T ON T.author_id = u.id
    ) P on P.submission_id = s.id
) R on R.subreddit_id = r.id
;



# Print table of submissions in subreddits tagged with TAG

SELECT R.name, s.id
FROM submission s
JOIN (
    SELECT r.id, r.name
    FROM subreddit r
    JOIN (
        SELECT s2t.subreddit_id
        FROM subreddit2tag s2t
        JOIN (
            SELECT t.id
            FROM tag t
            WHERE t.name = 'TAG'
        ) T on T.id = s2t.tag_id
    ) S2T on S2T.subreddit_id = r.id
) R on R.id = s.subreddit_id
;


# Print table of subreddit, username, comment in subreddits tagged with TAG

SELECT C.name, u.name, C.content
FROM user u
JOIN (
    SELECT S.name, c.author_id, c.content
    FROM comment c
    JOIN (
        SELECT R.name, s.id
        FROM submission s
        JOIN (
            SELECT r.id, r.name
            FROM subreddit r
            JOIN (
                SELECT s2t.subreddit_id
                FROM subreddit2tag s2t
                JOIN (
                    SELECT t.id
                    FROM tag t
                    WHERE t.name = 'TAG'
                ) T on T.id = s2t.tag_id
            ) S2T on S2T.subreddit_id = r.id
        ) R on R.id = s.subreddit_id
    ) S on S.id = c.submission_id
) C on C.author_id = u.id
;


# Print table of permalink, comment in subreddits tagged with TAG1 or TAG2

SELECT CONCAT("https://www.reddit.com/r/", S.name, "/comments/"), c.id, c.content
FROM comment c
JOIN (
    SELECT R.name, s.id
    FROM submission s
    JOIN (
        SELECT r.id, r.name
        FROM subreddit r
        JOIN (
            SELECT s2t.subreddit_id
            FROM subreddit2tag s2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name IN ('TAG1','TAG2')
            ) T on T.id = s2t.tag_id
        ) S2T on S2T.subreddit_id = r.id
    ) R on R.id = s.subreddit_id
) S on S.id = c.submission_id
;



















# Print table of subreddit, username, comment, reason_matched

Optionally for a list of users

SELECT R.reason, r.name AS 'subreddit', R.name AS 'user', R.content
FROM subreddit r
JOIN (
    SELECT s.subreddit_id, P.name, P.content, P.reason
    FROM submission s
    JOIN (
        SELECT u.name, T.content, T.submission_id, T.reason
        FROM user u
        JOIN (
            SELECT c.author_id, c.content, c.submission_id, RAISON.name as reason
            FROM comment c
            JOIN (
                SELECT id, name
                FROM reason_matched
                # WHERE name IN ('REASON1')
            ) RAISON ON RAISON.id = c.reason_matched
        ) T ON T.author_id = u.id
        # WHERE u.name IN ('USERNAME1','USERNAME2')
    ) P on P.submission_id = s.id
) R on R.subreddit_id = r.id
;







# Print table of subreddit, username, comment, reason_matched for users with more than 2 comments recorded, ordered by username

Optionally for a list of users

SELECT R.reason, r.name AS 'subreddit', R.name AS 'user', R.content
FROM subreddit r
JOIN (
    SELECT s.subreddit_id, P.name, P.content, P.reason
    FROM submission s
    JOIN (
        SELECT u.name, T.content, T.submission_id, T.reason
        FROM user u
        JOIN (
            SELECT rm.name as reason, C_COUNTED.author_id, C_COUNTED.content, C_COUNTED.submission_id
            FROM reason_matched rm
            JOIN (
                SELECT c.author_id, c.content, c.submission_id, c.reason_matched
                FROM comment c
                JOIN (
                    SELECT c_counter.author_id
                    FROM comment c_counter
                    GROUP BY c_counter.author_id
                    HAVING COUNT(c_counter.author_id) > 2
                ) C_COUNTER ON C_COUNTER.author_id = c.author_id
            ) C_COUNTED ON C_COUNTED.reason_matched = rm.id
        ) T ON T.author_id = u.id
        # WHERE u.name IN ('USERNAME1','USERNAME2')
    ) P on P.submission_id = s.id
) R on R.subreddit_id = r.id
ORDER BY user
;









# Print table of tagged subreddits and their tags, sorted by tag name

SELECT s.name, B.name as 'tag'
FROM subreddit s
JOIN (
    SELECT A.subreddit_id, t.name
    FROM tag t
    JOIN (
        SELECT s2t.subreddit_id, s2t.tag_id
        FROM subreddit2tag s2t
    ) A ON A.tag_id = t.id
) B ON B.subreddit_id = s.id
ORDER BY tag
;


# Print table of untagged subreddits

SELECT s.name as 'tag'
FROM subreddit s
WHERE s.id NOT IN (
    SELECT A.subreddit_id
    FROM tag t
    JOIN (
        SELECT s2t.subreddit_id, s2t.tag_id
        FROM subreddit2tag s2t
    ) A ON A.tag_id = t.id
)
ORDER BY s.name
;



# Sort the most commented-in subreddits

SELECT r.name, r.id, C.c
FROM subreddit r
JOIN (
    SELECT SUM(count) as c, subreddit_id
    FROM user2subreddit_cmnt_count
    GROUP BY subreddit_id
) C ON C.subreddit_id = r.id
ORDER BY C.c
;




# Sort the most commented-in subreddits, and show percentages

SELECT r.name, r.id, C.c, ROUND(
    100.0*C.c/(SELECT SUM(count) FROM user2subreddit_cmnt_count)
    , 2
) as '%'
FROM subreddit r
JOIN (
    SELECT SUM(count) as c, subreddit_id
    FROM user2subreddit_cmnt_count
    GROUP BY subreddit_id
) C ON C.subreddit_id = r.id
WHERE C.c > 100
ORDER BY C.c
;




# List comments per subreddit for list of usernames

Optionally for list of users

SELECT UU.user, s.name as 'subreddit', UU.count
FROM subreddit s
JOIN (
    SELECT U.name as 'user', u2scc.subreddit_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    JOIN (
        SELECT u.name, u.id
        FROM user u
        # WHERE u.name IN ('USERNAME1','USERNAME2')
    ) U ON U.id = u2scc.user_id
) UU ON UU.subreddit_id = s.id
;






# Table of user_id, tag_id, subreddit_id, count for a select list of user_ids

SELECT U2SCC.user_id, s2t.tag_id, U2SCC.subreddit_id, U2SCC.count
FROM subreddit2tag s2t
JOIN (
    SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    WHERE u2scc.user_id IN (11063919, 34494503, 59635894, 6674527480, 180053962953)
) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id
ORDER BY U2SCC.user_id
;


# Table of user_id, tag_id, subreddit, count for a select list of user_ids

SELECT A.user_id, A.tag_id, s.name, A.count
FROM subreddit s
JOIN (
    SELECT U2SCC.user_id, s2t.tag_id, U2SCC.subreddit_id, U2SCC.count
    FROM subreddit2tag s2t
    JOIN (
        SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
        FROM user2subreddit_cmnt_count u2scc
        WHERE u2scc.user_id IN (11063919, 34494503, 59635894, 6674527480, 180053962953)
    ) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id
) A ON A.subreddit_id = s.id
ORDER BY A.user_id
;



# GHJKHGJHJK: For a list of user IDs, calculate the mean colour based on the count of their comments in tagged subreddits (WARNING: double counts subreddits if they have multiple coloured tags - this is intended behaviour, as the subreddit should have a colour that is the average of its coloured tags)

SELECT S2T.user_id, SUM(S2T.c), SUM(S2T.c*t.r), SUM(S2T.c*t.g), SUM(S2T.c*t.b)
FROM tag t
JOIN (
    SELECT U2SCC.user_id, s2t.tag_id, SUM(U2SCC.count) as c
    FROM subreddit2tag s2t
    JOIN (
        SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
        FROM user2subreddit_cmnt_count u2scc
        WHERE u2scc.user_id IN (SELECT id FROM user)
    ) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id
    GROUP BY U2SCC.user_id, s2t.tag_id
) S2T ON S2T.tag_id = t.id
WHERE t.r IS NOT NULL
GROUP BY S2T.user_id
;


# For a list of user IDs, concatenate the list of count of thier comments in all subreddits (does not double count)

SELECT UU.user_id, GROUP_CONCAT(s.name, " ", UU.count)
FROM subreddit s
JOIN (
    SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    WHERE u2scc.user_id IN (11063919, 34494503, 59635894, 6674527480, 180053962953)
) UU ON UU.subreddit_id = s.id
GROUP BY UU.user_id
;




# For a list of user IDs, concatenate the list of count of thier comments in tagged subreddits (does not double count)

SELECT UU.user_id, GROUP_CONCAT(s.name, " ", UU.count)
FROM subreddit s
JOIN (
    SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    WHERE u2scc.user_id IN (11063919, 34494503, 59635894, 6674527480, 180053962953)
      AND u2scc.subreddit_id IN (
        SELECT subreddit_id
        FROM subreddit2tag
      )
) UU ON UU.subreddit_id = s.id
GROUP BY UU.user_id
;



SELECT UU.user_id, GROUP_CONCAT(s.name, " ", UU.count)
FROM subreddit s
JOIN (
    SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    WHERE u2scc.subreddit_id IN (
        SELECT subreddit_id
        FROM subreddit2tag
      )
) UU ON UU.subreddit_id = s.id
GROUP BY UU.user_id
;










# ASDFSDF: For a list of user IDs, concatenate the list of count of thier comments in subreddits (does not double count) per subreddit tag_id

SELECT UU.user_id, UU.tag_id, GROUP_CONCAT(s.name, " ", UU.count)
FROM subreddit s
JOIN (
    SELECT u2scc.user_id, u2scc.subreddit_id, A.tag_id, u2scc.count
    FROM user2subreddit_cmnt_count u2scc
    JOIN (
        SELECT subreddit_id, tag_id
        FROM subreddit2tag
    ) A ON A.subreddit_id = u2scc.subreddit_id
) UU ON UU.subreddit_id = s.id
GROUP BY UU.user_id, UU.tag_id
;

# ASDFSDF, but using user names and tag names instead of IDs
# NOTE: Currently, only a small fraction of user IDs are linked to a user name (as only those whose comments match the comment filter of rscrape++ are recorded in 'user' table)
# This means that a re-ordering of the query would probably be more performant

SELECT u.name, C.tag_id, C.string
FROM user u
JOIN (
    SELECT UU.user_id, UU.tag_id, GROUP_CONCAT(s.name, " ", UU.count) as string
    FROM subreddit s
    JOIN (
        SELECT u2scc.user_id, u2scc.subreddit_id, A.tag_id, u2scc.count
        FROM user2subreddit_cmnt_count u2scc
        JOIN (
            SELECT subreddit_id, tag_id
            FROM subreddit2tag
        ) A ON A.subreddit_id = u2scc.subreddit_id
        WHERE u2scc.user_id IN (SELECT id FROM user)
    ) UU ON UU.subreddit_id = s.id
    GROUP BY UU.user_id, UU.tag_id
) C ON u.id = C.user_id
;

# ASDFSDF, but grouping by category_id instead of tag_id

SELECT C.user_id, t2c.category_id, GROUP_CONCAT(C.tag_id, ":", C.string) as string
FROM tag2category t2c
JOIN (
    SELECT UU.user_id, UU.tag_id, GROUP_CONCAT(s.name, " ", UU.count) as string
    FROM subreddit s
    JOIN (
        SELECT u2scc.user_id, u2scc.subreddit_id, A.tag_id, u2scc.count
        FROM user2subreddit_cmnt_count u2scc
        JOIN (
            SELECT subreddit_id, tag_id
            FROM subreddit2tag
        ) A ON A.subreddit_id = u2scc.subreddit_id
        WHERE u2scc.user_id IN (SELECT id FROM user)
    ) UU ON UU.subreddit_id = s.id
    GROUP BY UU.user_id, UU.tag_id
) C ON t2c.tag_id = C.tag_id
GROUP BY C.user_id, t2c.category_id
;

# Same as above, but using names instead of IDs for users and categories

SELECT E.name, c.name, E.string
FROM category c
JOIN (
    SELECT u.id as user_id, u.name, D.category_id, D.string
    FROM user u
    JOIN (
        SELECT C.user_id, t2c.category_id, GROUP_CONCAT(C.tag_id, ":", C.string) as string
        FROM tag2category t2c
        JOIN (
            SELECT UU.user_id, UU.tag_id, GROUP_CONCAT(s.name, " ", UU.count) as string
            FROM subreddit s
            JOIN (
                SELECT u2scc.user_id, u2scc.subreddit_id, A.tag_id, u2scc.count
                FROM user2subreddit_cmnt_count u2scc
                JOIN (
                    SELECT subreddit_id, tag_id
                    FROM subreddit2tag
                ) A ON A.subreddit_id = u2scc.subreddit_id
                WHERE u2scc.user_id IN (SELECT id FROM user)
            ) UU ON UU.subreddit_id = s.id
            GROUP BY UU.user_id, UU.tag_id
        ) C ON t2c.tag_id = C.tag_id
        GROUP BY C.user_id, t2c.category_id
    ) D ON u.id = D.user_id
) E ON c.id = E.category_id
ORDER BY E.user_id
;

# Same as above, but also using tag names

SELECT E.name, c.name, E.string
FROM category c
JOIN (
    SELECT u.id as user_id, u.name, D.category_id, D.string
    FROM user u
    JOIN (
        SELECT C.user_id, t2c.category_id, GROUP_CONCAT(C.tag_name, ":", C.string) as string
        FROM tag2category t2c
        JOIN (
            SELECT F.user_id, F.tag_id, F.string, t.name as tag_name
            FROM tag t
            JOIN (
                SELECT UU.user_id, UU.tag_id, GROUP_CONCAT(s.name, " ", UU.count) as string
                FROM subreddit s
                JOIN (
                    SELECT u2scc.user_id, u2scc.subreddit_id, A.tag_id, u2scc.count
                    FROM user2subreddit_cmnt_count u2scc
                    JOIN (
                        SELECT subreddit_id, tag_id
                        FROM subreddit2tag
                    ) A ON A.subreddit_id = u2scc.subreddit_id
                    WHERE u2scc.user_id IN (SELECT id FROM user)
                ) UU ON UU.subreddit_id = s.id
                GROUP BY UU.user_id, UU.tag_id
            ) F on t.id = F.tag_id
        ) C ON t2c.tag_id = C.tag_id
        GROUP BY C.user_id, t2c.category_id
    ) D ON u.id = D.user_id
) E ON c.id = E.category_id
ORDER BY E.user_id
;






# GHJKHGJHJK, but calculating a mean colour per category

SELECT A.user_id, SUM(A.c), SUM(A.r), SUM(A.g), SUM(A.b)
FROM tag2category t2c
JOIN (
    SELECT S2T.user_id, S2T.tag_id, S2T.c, S2T.c*t.r as r, S2T.c*t.g as g, S2T.c*t.b as b
    FROM tag t
    JOIN (
        SELECT U2SCC.user_id, s2t.tag_id, SUM(U2SCC.count) as c
        FROM subreddit2tag s2t
        JOIN (
            SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count
            FROM user2subreddit_cmnt_count u2scc
            WHERE u2scc.user_id < 10000000
        ) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id
        GROUP BY U2SCC.user_id, s2t.tag_id
    ) S2T ON S2T.tag_id = t.id
) A ON t2c.tag_id = A.tag_id
GROUP BY A.user_id
;



# WARNING: Above queries of this type might be slightly wrong somehow

# Calculate average colour per category, and print string of the relevant subreddit names

# Alpha is treated as a fourth channel, since we want a transparent colour to make the result more transparent, not more black

SELECT A.user_id, t2c.category_id, SUM(A.c), SUM(A.r), SUM(A.g), SUM(A.b), SUM(A.a), GROUP_CONCAT(A.string)
FROM tag2category t2c
JOIN (
    SELECT S2T.user_id, S2T.tag_id, S2T.c, S2T.c*t.r as r, S2T.c*t.g as g, S2T.c*t.b as b, S2T.c*t.a as a, string
    FROM tag t
    RIGHT JOIN (
        SELECT U2SCC.user_id, s2t.tag_id, SUM(U2SCC.count) as c, GROUP_CONCAT(U2SCC.name, " ", U2SCC.count) as string
        FROM subreddit2tag s2t
        JOIN (
            SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count, S.name
            FROM user2subreddit_cmnt_count u2scc
            JOIN (
                SELECT id, name
                FROM subreddit
            ) S ON S.id = u2scc.subreddit_id
            WHERE u2scc.user_id IN (SELECT id FROM user)
        ) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id
        GROUP BY U2SCC.user_id, s2t.tag_id
    ) S2T ON S2T.tag_id = t.id
) A ON t2c.tag_id = A.tag_id
GROUP BY A.user_id, t2c.category_id
;









# For a list of subreddits, calculate their similarity based on users in common

SELECT user_id, subreddit_id, count
FROM user2subreddit_cmnt_count
WHERE user_id IN (
    SELECT user_id
    FROM user2subreddit_cmnt_count
    WHERE subreddit_id IN (SELECT id FROM subreddit WHERE name LIKE '%programming%')
    AND count > 1
    GROUP BY user_id 
    HAVING COUNT(user_id) > 1
)
  AND count > 1
GROUP BY subreddit_id

# Table of subreddits to the number of users in common with given subreddits

SELECT r.name, A.c
FROM subreddit r
JOIN (
    SELECT subreddit_id, COUNT(subreddit_id) as c
    FROM user2subreddit_cmnt_count
    WHERE user_id IN (
        SELECT user_id
        FROM user2subreddit_cmnt_count
        WHERE subreddit_id IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
        AND count > 1
    )
      AND count > 1
      AND subreddit_id NOT IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
    GROUP BY subreddit_id
) A ON r.id = A.subreddit_id
ORDER BY A.c

# Table of subreddits to (user/comment)s in common with given subreddits

SELECT r.name, A.c
FROM subreddit r
JOIN (
    SELECT subreddit_id, SUM(count) as c
    FROM user2subreddit_cmnt_count
    WHERE user_id IN (
        SELECT user_id
        FROM user2subreddit_cmnt_count
        WHERE subreddit_id IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
        AND count > 1
    )
      AND count > 1
      AND subreddit_id NOT IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
    GROUP BY subreddit_id
) A ON r.id = A.subreddit_id
ORDER BY A.c


# Table of subreddits to (user/comment)s in common with given subreddits, normalised by the total number of comments by the user

SELECT r.name, A.c
FROM subreddit r
JOIN (
    SELECT s2scc.subreddit_id, SUM(s2scc.count/B.c) as c
    FROM user2subreddit_cmnt_count s2scc
    JOIN (
        SELECT user_id, SUM(count) as c
        FROM user2subreddit_cmnt_count s2scc
        GROUP BY user_id
    ) B ON B.user_id = s2scc.user_id
    WHERE s2scc.user_id IN (
        SELECT user_id
        FROM user2subreddit_cmnt_count
        WHERE subreddit_id IN (SELECT id FROM subreddit WHERE name IN ('%programming%'))
        AND count > 1
    )
      AND count > 1
      AND subreddit_id NOT IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
    GROUP BY subreddit_id
) A ON r.id = A.subreddit_id
ORDER BY A.c
;











# Table of subreddits to (user/comment)s in common with subreddits of a given tag, normalised by the total number of comments by the user

SELECT r.name, A.c
FROM subreddit r
JOIN (
    SELECT s2scc.subreddit_id, SUM(s2scc.count/B.c) as c
    FROM user2subreddit_cmnt_count s2scc
    JOIN (
        SELECT user_id, SUM(count) as c
        FROM user2subreddit_cmnt_count s2scc
        GROUP BY user_id
    ) B ON B.user_id = s2scc.user_id
    WHERE s2scc.user_id IN (
        SELECT user_id
        FROM user2subreddit_cmnt_count
        WHERE subreddit_id IN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id IN (SELECT id FROM tag WHERE name IN ('TAG')))
        AND count > 1
    )
      AND count > 1
      AND subreddit_id NOT IN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id IN (SELECT id FROM tag WHERE name IN ('TAG')))
      #AND subreddit_id NOT IN (SELECT id FROM subreddit WHERE name LIKE '%programming%'))
    GROUP BY subreddit_id
) A ON r.id = A.subreddit_id
ORDER BY A.c
;






# Count comments per subreddit

SELECT r.name, A.c
FROM subreddit r
JOIN (
    SELECT subreddit_id, SUM(count) as c
    FROM user2subreddit_cmnt_count
    GROUP BY subreddit_id
    ORDER BY c DESC
    LIMIT 100
) A ON r.id = A.subreddit_id
;





# Usernames to subreddits moderated

SELECT u.name, B.name
FROM user u
JOIN (
    SELECT A.user_id, r.name
    FROM subreddit r
    JOIN (
        SELECT user_id, subreddit_id
        FROM moderator
    ) A ON A.subreddit_id = r.id
) B ON B.user_id = u.id
ORDER BY u.id
;

# The above, but limited to users who moderate 'SUBREDDIT'

SELECT A.user_id, r.name
FROM subreddit r
JOIN (
    SELECT user_id, subreddit_id
    FROM moderator
    WHERE user_id IN (
        SELECT user_id
        FROM moderator
        WHERE subreddit_id IN (SELECT id FROM subreddit WHERE name = 'SUBREDDIT')
    )
) A ON A.subreddit_id = r.id
ORDER BY A.user_id
;

# Usernames to number of subreddits moderated

SELECT u.name, COUNT(B.subreddit_id) AS c
FROM user u
JOIN (
    SELECT user_id, subreddit_id
    FROM moderator
) B ON B.user_id = u.id
GROUP BY u.name
ORDER BY c
;

# Subreddits with mods in common with 'SUBREDDIT'

SELECT r.name, COUNT(r.id) as c, GROUP_CONCAT(A.name, ":", A.rank)
FROM subreddit r
JOIN (
    SELECT u.name, B.subreddit_id, B.rank
    FROM user u
    JOIN (
        SELECT user_id, subreddit_id, rank
        FROM moderator
        WHERE user_id IN (
            SELECT user_id
            FROM moderator
            WHERE user_id != 11063919
              AND subreddit_id IN (SELECT id FROM subreddit WHERE name = 'SUBREDDIT')
        )
    ) B ON B.user_id = u.id
) A ON A.subreddit_id = r.id
GROUP BY r.id
ORDER BY c ASC
;

