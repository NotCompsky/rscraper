struct cmnt_meta {
    const char* author_name;
    const char* subreddit_name;
    
    // ID comparison should be faster for filtering based on exact matches
    const unsigned long int author_id;
    const unsigned long int subreddit_id;
};
