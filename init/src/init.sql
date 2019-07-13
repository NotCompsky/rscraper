/*
Prefer VARBINARY to VARCHAR as the latter forces case-insensitivity in searches.
*/

R"=====(

CREATE TABLE IF NOT EXISTS user (
    id BIGINT UNSIGNED NOT NULL,
    name VARBINARY(128),
    created_at BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS subreddit (
    id BIGINT UNSIGNED NOT NULL,
    name VARBINARY(128),
    created_at BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS subreddit2meta (
    id BIGINT UNSIGNED NOT NULL,
    subscribers BIGINT UNSIGNED NOT NULL,
    created_at BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS submission (
    id BIGINT UNSIGNED NOT NULL,
    author_id BIGINT UNSIGNED NOT NULL DEFAULT 0,
    subreddit_id BIGINT UNSIGNED NOT NULL,
    content VARBINARY(40000),
    created_at BIGINT UNSIGNED NOT NULL DEFAULT 0,
    nsfw SMALLINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS comment (
    id BIGINT UNSIGNED NOT NULL,
    parent_id BIGINT UNSIGNED NOT NULL DEFAULT 0,
    author_id BIGINT UNSIGNED NOT NULL,
    submission_id BIGINT UNSIGNED NOT NULL,
    content VARBINARY(40000) NOT NULL,
    created_at BIGINT UNSIGNED NOT NULL,
    reason_matched INT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS subreddit2tag (
    subreddit_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY `subreddit2tag` (`subreddit_id`, `tag_id`)
);

CREATE TABLE IF NOT EXISTS tag (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(128) UNIQUE,
    r FLOAT NOT NULL,
    g FLOAT NOT NULL,
    b FLOAT NOT NULL,
    a FLOAT NOT NULL,
    UNIQUE KEY (`name`),
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS reason_matched (
    id INT UNSIGNED NOT NULL,
    name VARBINARY(128) UNIQUE,
    r FLOAT NOT NULL DEFAULT 0,
    g FLOAT NOT NULL DEFAULT 0,
    b FLOAT NOT NULL DEFAULT 0,
    a FLOAT NOT NULL DEFAULT 0,
    UNIQUE KEY (name),
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS user2subreddit_cmnt_count (
    user_id BIGINT UNSIGNED NOT NULL,
    subreddit_id BIGINT UNSIGNED NOT NULL,
    count INT UNSIGNED NOT NULL,
    PRIMARY KEY `user2subreddit` (`user_id`, `subreddit_id`)
);

CREATE TABLE IF NOT EXISTS tag2category (
    tag_id BIGINT UNSIGNED NOT NULL,
    category_id INT UNSIGNED NOT NULL,
    PRIMARY KEY `tag2category` (`tag_id`, `category_id`)
);

CREATE TABLE IF NOT EXISTS category (
    id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(32),
    UNIQUE KEY (name),
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS moderator (
    user_id BIGINT UNSIGNED NOT NULL,
    subreddit_id BIGINT UNSIGNED NOT NULL,
    permissions BIGINT UNSIGNED NOT NULL,
    added_on BIGINT UNSIGNED NOT NULL,
    removed_by BIGINT UNSIGNED NOT NULL DEFAULT 0,
    last_updated DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    modrank INT UNSIGNED NOT NULL,
    PRIMARY KEY `user2subreddit` (`user_id`, `subreddit_id`)
);


CREATE TABLE IF NOT EXISTS subreddit_count_bl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS user_count_bl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS subreddit_contents_wl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS subreddit_contents_bl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS user_contents_wl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS user_contents_bl (
    id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS reason_subreddit_blacklist (
    reason INT UNSIGNED NOT NULL,
    subreddit BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY `reason2subreddit` (reason, subreddit)
);

CREATE TABLE IF NOT EXISTS reason_subreddit_whitelist (
    reason INT UNSIGNED NOT NULL,
    subreddit BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY `reason2subreddit` (reason, subreddit)
);

CREATE TABLE IF NOT EXISTS regex_vars (
    name VARBINARY(128) NOT NULL,
    type INT UNSIGNED NOT NULL,
    data VARBINARY(40000) DEFAULT NULL,
    PRIMARY KEY (name)
);


INSERT IGNORE INTO reason_matched (id, name)             VALUES (0, "None");
INSERT IGNORE INTO reason_matched (id, name, r, g, b, a) VALUES (1, "Unknown", 0.5, 0.5, 0.5, 1);

)====="
